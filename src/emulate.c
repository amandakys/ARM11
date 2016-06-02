#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>
#include <byteswap.h>

#define PROCESSING 1
#define MULTIPLY 2
#define TRANSFER 3
#define BRANCH 4
#define INVALID 5
#define CPSRth 16
#define Nth 31
#define Zth 30
#define Cth 29
#define Vth 28
#define MAX_ITEMS 16384
#define MAX_BYTES 65536

typedef struct _arm *Arm;
typedef struct _process *Process;
typedef struct _multiply *Multiply;
typedef struct _transfer *Transfer;
typedef struct _branch *Branch;
typedef struct _instruction *Instruction;

typedef struct _arm {
    uint32_t registers[17];
    uint32_t memory[MAX_ITEMS];
} arm;

typedef struct _instruction {
    uint32_t Cond;
    Process p;
    Multiply m;
    Transfer t;
    Branch b;
} instruction;

typedef struct _process {
    uint32_t I;
    uint32_t Opcode;
    uint32_t S;
    uint32_t Rn;
    uint32_t Rd;
    uint32_t Operand2;
} process;

typedef struct _multiply {
    uint32_t A;
    uint32_t S;
    uint32_t Rd;
    uint32_t Rn;
    uint32_t Rs;
    uint32_t Rm;
} multiply;

typedef struct _transfer {
    uint32_t I;
    uint32_t P;
    uint32_t U;
    uint32_t L;
    uint32_t Rn;
    uint32_t Rd;
    uint32_t Offset;
} transfer;

typedef struct _branch {
    uint32_t Offset;
} branch;

typedef enum {NEXT, HALT, FLUSH} executeRes;

bool checkCond(Arm a, uint32_t cond);
int decode(Arm a, Instruction components, uint32_t instruction);
uint32_t fetch(Arm a);


uint32_t add (uint32_t x, uint32_t y) {
    return x + y;
}

uint32_t addCarry(uint32_t x, uint32_t y) {
    return x & y;
}

uint32_t sub(uint32_t x, uint32_t y) {
    return x - y;
}

uint32_t subBorrow(uint32_t x, uint32_t y) {
    if (x >= y) {
        return 0; //no borrow
    } else {
        return 1; //borrow
    }
}

void setC(Arm a, uint32_t carry) {
    a -> registers[CPSRth] = (a->registers[CPSRth] & ~(1<<Cth)) | (carry << Cth); //set the C flag in CPSR
}

void setZP(Arm a, uint32_t result) {
    if (result == 0) {
        a->registers[CPSRth] |= 1 << Zth;
    } else {
        a->registers[CPSRth] &= ~(1 << Zth);
    }
}

void setNP(Arm a, uint32_t result) {
    int bit31 =  result >> Nth;
    a->registers[CPSRth] = (a->registers[CPSRth] & ~(1 << Nth)) | (bit31 << Nth);
}

void setCPSR(Arm a, uint32_t carry, uint32_t result) {
    setC(a, carry);
    setZP(a, result);
    setNP(a, result);
}

void setResult(Arm a, Process p, uint32_t result) {
    a->registers[p->Rd] = result;

}

uint32_t calculateShiftAmount(Arm a, uint32_t Operand2) {
    uint32_t shift = (0x00000FF0 & (Operand2)) >> 4;
    uint32_t bit4 = 0x00000001 & shift;
    uint32_t shiftAmount;
    if (bit4 == 0) {
            //shift by a constant amount
            shiftAmount = (0x000000F8 & shift) >> 3;
    } else {
        //shift by a register
        uint32_t Rs = (0x000000F0 & shift) >> 4;
        shiftAmount = 0x0000000F & (a->registers[Rs]);
    }

    return shiftAmount;

}

uint32_t ror(uint32_t value, int shift) {
    if ((shift &= 31) == 0) {
        return value;
    } else {
        return (value >> shift) | (value << (32 - shift));
    }
}

uint32_t asr(uint32_t value, int shift) {
    if (((0x80000000 & value) >> 31) == 1) {
        uint32_t mask = (((uint32_t) pow(2.0, (double) shift)) - 1) << (32 - shift);
        return (value >> shift) | mask;
    } else {
        return value >> shift;
    }

}

int numDigits (int num) {
    if (num < 0) {
      return (int) (log10(-num) + 2);
    } else if (num == 0) {
        return 1;
    } else {
        return (int) (log10(num) + 1);
    }
}
void printState(Arm a) {
    printf("Registers:\n");
    for (int i = 0; i < 10; i++) {
        printf ("$%i  : ", i);
        for (int j = 0; j < (10-numDigits((int)a->registers[i])); j++) {
            printf(" ");
        }
        printf ("%i (0x%08x)\n", a->registers[i], a->registers[i]);
    }

    for (int i = 10; i < 13; i++) {
        printf ("$%i : ", i);
        for (int j = 0; j < (10-numDigits(a->registers[i])); j++) {
            printf(" ");
        }
        printf ("%i (0x%08x)\n", a->registers[i], a->registers[i]);
    }

    printf("PC  : ");
    for (int j = 0; j < (10-numDigits(a->registers[15] * 4)); j++) {
            printf(" ");
    }
    printf("%i (0x%08x)\n", a->registers[15] * 4, a->registers[15] * 4);

    printf("CPSR: ");
    for (int j = 0; j < (10-numDigits(a->registers[16])); j++) {
            printf(" ");
    }
    printf("%i (0x%08x)\n", a->registers[16], a->registers[16]);

     printf("Non-zero memory:\n");
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (a->memory[i] != 0) {
            printf("0x%08x: 0x%08x\n", i*4, __bswap_32(a->memory[i]));
        }
    }
}


uint32_t separateByte(int byteNumber, uint32_t instruction){

    switch(byteNumber){
        case 0 : return (0xFF000000 & instruction) >> 24;
        case 1 : return (0x00FF0000 & instruction) >> 16;
        case 2 : return (0x0000FF00 & instruction) >> 8;
        case 3 : return (0x000000FF & instruction);
        default: return instruction;

    }
}

uint32_t calculateCarry(Arm a, uint32_t Operand2) {
    uint32_t Rm = 0x0000000F & (Operand2);
    uint32_t value = a -> registers[Rm];
    uint32_t shift = (0x00000FF0 & (Operand2)) >> 4;
    uint32_t shiftType = (0x00000006 & shift) >> 1;
    uint32_t shiftAmount = calculateShiftAmount(a, Operand2);
    uint32_t carry = 0;

    switch (shiftType) {
            case 0: carry = (0x80000000 & (value << (shiftAmount - 1))) >> 31; break;

            case 1: carry = 0x00000001 & (value >> (shiftAmount - 1)); break;

            case 2: carry = 0x00000001 & (asr(value, shiftAmount - 1)); break;

            case 3: carry = 0x00000001 & (ror(value, shiftAmount - 1)); break;

    }

    return carry;
}

uint32_t Op2Register(Arm a, uint32_t Operand2) { //given p->Operand2 it returns the value of op2
    uint32_t Rm = 0x0000000F & (Operand2);
    uint32_t value = a -> registers[Rm];
    uint32_t shift = (0x00000FF0 & (Operand2)) >> 4;
    uint32_t shiftType = (0x00000006 & shift) >> 1;
    uint32_t shiftAmount = calculateShiftAmount(a, Operand2);
    uint32_t op2Value = 0;

    switch (shiftType) {
        case 0: op2Value = value << shiftAmount; break;
        case 1: op2Value = value >> shiftAmount; break;
        case 2: op2Value = asr(value, shiftAmount); break;
        case 3: op2Value = ror(value, shiftAmount); break;
    }

    return op2Value;
}


void executeP(Arm a, Process p) {
    uint32_t op2Value = 0;
    uint32_t carry = 0;
    if (p-> I == 1) {
        //Operand 2 is immediate constant
        uint32_t value = 0x000000FF & (p -> Operand2);
        uint32_t rotate = (0x00000F00 & (p -> Operand2)) >> 8;
        int numRotations = rotate * 2;


        if (numRotations > 1){
            op2Value = ror(value, numRotations-1);
            carry = 0x00000001 & op2Value;
            op2Value = ror (op2Value, 1);
        } else if (numRotations == 1){
            carry = 0x00000001 & op2Value;
            op2Value = ror(value,1);
        } else {
            carry = 0;
            op2Value = value;
            }

  //  setResult(a, p, op2Value);


    } else {
        //Operand 2 is a shift register
        op2Value = Op2Register(a, p->Operand2);
        carry = calculateCarry(a, p->Operand2);
    }

    uint32_t op1Value = a->registers[p->Rn];
    uint32_t result;

    switch (p -> Opcode) {
        case 0:
                result = op1Value & op2Value;
                setResult(a, p, result);

                if (p->S == 1) {
                    setCPSR(a, carry, result);
                }

                break;
        case 1:
                result = op1Value ^ op2Value;
                setResult(a, p, result);

                if (p->S == 1) {
                    setCPSR(a, carry, result);
                }
                break;

        case 2:
                result = sub (op1Value, op2Value);
                setResult(a, p, result);

                if (p->S == 1) {
                    carry = subBorrow (op1Value, op2Value);
                    if (carry == 1) {
                        carry = 0;
                    } else {
                        carry = 1;
                    }
                    setCPSR(a, carry, result);
                }

                break;
        case 3:
                result = sub (op2Value, op1Value);
                setResult(a, p, result);

                if (p->S == 1) {
                    carry = subBorrow (op2Value, op1Value);
                    if (carry == 1) {
                        carry = 0;
                    } else {
                        carry = 1;
                    }
                    setCPSR(a, carry, result);
                }
                break;
        case 4:
                result = add(op1Value, op2Value);
                setResult(a, p, result);
                if (p ->S == 1) {
                    carry = addCarry(op1Value, op2Value);
                    setCPSR(a, carry, result);
                }
                break;
        case 8:
                result = op1Value & op2Value;

                if (p->S == 1) {
                    setCPSR(a, carry, result);
                }
                break;

        case 9:
                result = op1Value ^ op2Value;

                if (p->S == 1) {
                    setCPSR(a, carry, result);
                }
                break;

        case 10:
                result = sub (op1Value, op2Value);
                //setResult(a, p, result);

                if (p->S == 1) {
                    carry = subBorrow (op1Value, op2Value);
                    if (carry == 1) {
                        carry = 0;
                    } else {
                        carry = 1;
                    }
                    setCPSR(a, carry, result);
                }
                break;

        case 12:
                result = op1Value | op2Value;
                setResult(a, p, result);
                if (p->S == 1) {
                    setCPSR(a, carry, result);
                }
                break;
        case 13:
                result = op2Value;
                setResult(a, p, result);
                if (p->S == 1) {
                    setCPSR(a, carry, result);
                }
                break;
    }
}

void setZ(Arm a, Multiply m) {
    if (a->registers[m->Rd] == 0) {
        a->registers[CPSRth] |= 1 << Zth;
    } else {
        a->registers[CPSRth] &= ~(1 << Zth);
    }
}

void setN(Arm a, Multiply m) {
    int bit31 =  a->registers[m->Rd] >> Nth;
    a->registers[CPSRth] = (a->registers[CPSRth] & ~(1 << Nth))| (bit31 << Nth);
}

void mul(Arm a, Multiply m) {
    uint64_t result = (uint64_t) (a->registers[m->Rm] * a->registers[m->Rs]);
    a->registers[m->Rd] = (uint32_t) (result & 0xFFFFFFFF);

}

void executeM(Arm a, Multiply m) {
    mul(a, m);
    if (m->A == 1) {
        a->registers[m->Rd] += a->registers[m->Rn];
    }
    if (m->S == 1) {
        setN(a,m);
        setZ(a,m);
    }

}

int index (Arm a, Transfer t, int i) {
    if(t -> P == 0) {//post indexing
        if(t -> U == 1) {
            a -> registers[t -> Rn] += (t -> Offset);
            //i += (t->Offset);
        } else {
            a -> registers[t -> Rn] -= (t -> Offset);
            //i -= (t->Offset);
        }
    } else {
        //Pre-indexing, no changes to Rn (according to specs)
        if(t -> U == 1) {
            i += (t -> Offset);
        } else {
            i -= (t -> Offset);
        }
    }
    return i;
}

uint32_t changeByte (int loc, uint32_t byte, uint32_t memory) {
    switch(loc){

    case 0: return (memory & 0x00FFFFFF) | (byte <<24);
    case 1: return (memory & 0xFF00FFFF) | (byte <<16);
    case 2: return (memory & 0xFFFF00FF) | (byte <<8);
    case 3: return (memory & 0xFFFFFF00) |  byte;
    default: return memory;


    }
}
//lololol talk about a massive over complication
/*void gpio(uint32_t i, uint32_t value) {
    int pin = 0;
    if (i == 0x20200008) {
        printf("One GPIO pin from 20 to 29 has been accessed");
        pin = 20;
    } else if (i == 0x20200004) {
        printf("One GPIO pin from 10 to 19 has been accessed");
        GPIO = true;
        pin = 10;
    } else if (i == 0x20200000) {
        printf("One GPIO pin from 0 to 9 has been accessed");
        GPIO = true;
    }

    //we have to make sure that value doesn't also set some pin as an input pin
    //ie change a 001 to a 000

    if ((value | 0) != 0) {
        //value changes some pin into an output pin
        //pin needs to be cleared then set

        //determine which pin needs to be cleared
        uint32_t mask1 = 0x00000007;
        uint32_t mask2 = 0x00000018;
        uint32_t mask3 = 0x000000D0;

        for (int i = 0; i < 10; i+3) {
            if ((value & mask1) == 1) {
                pin += i;
                break;
            }
            mask1 = mask1 << 9;
        }

        for (int i = 1; i < 10; i + 3) {
            if ((value & mask2) == 1) {
                pin += i;
                break;
            }
            mask2 = mask2 << 9;
        }

         for (int i = 2; i < 10; i + 3) {
            if ((value & mask3) == 1) {
                pin += i;
                break;
            }
            mask3 = mask3 << 9;
        }

        //set bit ? at memory address 0x20200028 to 1 = CLEAR
        uint32_t pinbit = 0x00000001 << pin;
        memory[0x20200028] |= pinbit;
        printf("PIN OFF");

        //set bit ? at memory address 0x2020001C to 1 = ON
        uint32_t pinbit = 0x00000001 << pin;
        memory[0x2020001C] |= pinbit;
        printf("PIN ON");

    }
}
}*/
void executeT(Arm a, Transfer t) {

    int i; // start of memory position that we want to transfer

    //Check if immediate offset or as shifted register
    if(t -> I == 1) {
        //Offset as shifted register(Op2Register is helper func of executeP)
        t->Offset = Op2Register(a, t -> Offset);
    }

    //Check if load from or to memory
    if(t -> L == 1) {
        //Load to a register
        i = a -> registers[t->Rn];
        // if the memory address is given by the PC
        // multiply by 4 to fix up our dodgy PC storing method
        if (t->Rn == 15) {
            i *=4;
        }

        i = index(a, t, i);

        if ( i + 3 <= MAX_ITEMS*4 ) {

             // takes the appropriate 4byte value from memory position i
            uint32_t byte0 = separateByte(3-(i%4), a->memory[i/4]) << 24; i++;
            uint32_t byte1 = separateByte(3-(i%4), a->memory[i/4]) << 16; i++;
            uint32_t byte2 = separateByte(3-(i%4), a->memory[i/4]) << 8; i++;
            uint32_t byte3 = separateByte(3-(i%4), a->memory[i/4]);

            uint32_t backwardValue = (byte0 | byte1 | byte2 | byte3);
            uint32_t value = __bswap_32(backwardValue);

            a -> registers[t -> Rd] = value;
        }

    } else {
        //Load to memory
        i = a -> registers[t->Rn];
        i = index (a, t, i); //which address in memory we want to take from

        uint32_t value =  __bswap_32(a->registers[t->Rd]);

        if (i == 0x20200008) {
            printf("One GPIO pin from 20 to 29 has been accessed");
        } else if (i == 0x20200004) {
            printf("One GPIO pin from 10 to 19 has been accessed");
        } else if (i == 0x20200000) {
            printf("One GPIO pin from 0 to 9 has been accessed");
        } else if (i == 0x2020001C) {
            printf("PIN ON");
        } else if (i == 0x20200028) {
            printf("PIN OFF");
        }

        if ( i + 3 <= MAX_ITEMS*4 ) {
            uint32_t byte0 = separateByte(0, value);
            uint32_t byte1 = separateByte(1, value);
            uint32_t byte2 = separateByte(2, value);
            uint32_t byte3 = separateByte(3, value);

            a->memory[i/4] = changeByte(3 - i%4, byte0, a->memory[i/4]);
            a->memory[(i+1)/4] = changeByte(3 - (i+1)%4, byte1, a->memory[(i+1)/4]);
            a->memory[(i+2)/4] = changeByte(3 - (i+2)%4, byte2, a->memory[(i+2)/4]);
            a->memory[(i+3)/4] = changeByte(3 - (i+3)%4, byte3, a->memory[(i+3)/4]);
        }
    }

    if (i > MAX_BYTES){
        printf("Error: Out of bounds memory access at address 0x%08x\n", i );
    }
}

void executeB(Arm a, Branch b) {
    // The offset was already sign-extended and shifted by 2 positions
    // to the right when passed to this function. If the offset is positive
    // it simply adds it to the PC and if it's negative it changes it
    // to the respective positive value and then decrements it from PC

    uint32_t MSB = (b -> Offset & 0x80000000) >> 31;
    if (MSB == 0) {
        a -> registers[15] += (b -> Offset)/4;
    } else {
        uint32_t unSigned = (~(b -> Offset) + 1);
        a -> registers[15] -= unSigned/4;
    }

}


bool checkCond(Arm a, uint32_t cond) {
    uint32_t mask = 0xF0000000;
    uint32_t CPSR = (a -> registers[16] & mask) >> 28;

    uint32_t n = (0x00000008 & CPSR) >> 3;
    uint32_t z = (0x00000004 & CPSR) >> 2;
    //uint32_t c = (0x00000002 & CPSR) >> 1;
    uint32_t v = 0x00000001 & CPSR;

    switch(cond) {
        case 0: return z == 1; break;
        case 1: return z == 0; break;
        case 10: return n == v; break;
        case 11: return n != v; break;
        case 12: return z == 0 && n == v; break;
        case 13: return z == 1 || n != v; break;
        case 14: return true;   break;
        default: return false;  break;
    }
}

uint32_t fetch(Arm a) { //returns instruction
    //increment PC
    uint32_t PC = a -> registers[15];
    //convert PC from binary to integer rep
    //get value at memory
    a -> registers[15] ++;
    return a->memory[(int) (PC)];
}

void decodeT(Transfer t, uint32_t instruction) {
    t -> I      = (0x02000000 & instruction) >> 25;
    t -> P      = (0x01000000 & instruction) >> 24;
    t -> U      = (0x00800000 & instruction) >> 23;
    t -> L      = (0x00100000 & instruction) >> 20;
    t -> Rn     = (0x000F0000 & instruction) >> 16;
    t -> Rd     = (0x0000F000 & instruction) >> 12;
    t -> Offset = (0x00000FFF & instruction);
}

void decodeP(Process p, uint32_t instruction) {

    p -> I        = (0x02000000 & instruction) >> 25;
    p -> Opcode   = (0x01E00000 & instruction) >> 21;
    p -> S        = (0x00100000 & instruction) >> 20;
    p -> Rn       = (0x000F0000 & instruction) >> 16;
    p -> Rd       = (0x0000F000 & instruction) >> 12;
    p -> Operand2 = (0x00000FFF & instruction);
}

void decodeM(Multiply m, uint32_t instruction) {

    m -> A  = (0x00200000 & instruction) >> 21;
    m -> S  = (0x00100000 & instruction) >> 20;
    m -> Rd = (0x000F0000 & instruction) >> 16;
    m -> Rn = (0x0000F000 & instruction) >> 12;
    m -> Rs = (0x00000F00 & instruction) >> 8;
    m -> Rm = (0x0000000F & instruction);
}

void decodeB(Branch b, uint32_t instruction) {

    uint32_t MSB = (0x00800000 & instruction) >> 23;
    if (MSB == 1) {
        b -> Offset = (0xFF000000 | instruction) << 2;
    } else {
        b -> Offset = (0x00FFFFFF & instruction) << 2 ;
    }
}

int decode(Arm a, Instruction components, uint32_t instruction) {
    //take 2
    uint32_t mask = 0x0C000000;
    components -> Cond = (0xF0000000 & instruction) >> 28;
    if (instruction == 0) {//instruction is all zero = halt
        return 0;
    } else if (!checkCond (a, components -> Cond)) {
        return INVALID;
    } else if (((instruction & mask) >> 26) == 1) {
        //single data transfer
        //components -> t = malloc (sizeof (struct _transfer));
        decodeT(components -> t, instruction);
        return TRANSFER;
    } else if (((instruction & mask) >> 26) == 2) {
        //branch
        //components -> b = malloc (sizeof (struct _branch));
        decodeB(components -> b, instruction);
        return BRANCH;
    } else {
        mask = 0x02000000;
        uint32_t mask2 = 0x000000F0;
        if (((instruction & mask) >>25) == 1 || ((instruction & mask2) >>4) != 9) {
            //data processing
            //components -> p = malloc (sizeof (struct _process));
            decodeP(components -> p, instruction);
            return PROCESSING;
        } else {
            //components -> m = malloc (sizeof (struct _multiply));
            decodeM(components -> m, instruction);
            return MULTIPLY;
        }
    }
}

executeRes execute (Arm a, Instruction components, int type) {
    switch(type) {
            case 0: return HALT;
            case PROCESSING: executeP(a, components -> p); return NEXT;
            case MULTIPLY:   executeM(a, components -> m); return NEXT;
            case TRANSFER:   executeT(a, components -> t); return NEXT;
            case BRANCH:     executeB(a, components -> b); return FLUSH;
            case INVALID: return NEXT;
    }
    return NEXT;
}

void FEcycle (Arm a) {
    Instruction components = malloc (sizeof (struct _instruction));
    components -> p = malloc (sizeof (struct _process));
    components -> m = malloc (sizeof (struct _multiply));
    components -> t = malloc (sizeof (struct _transfer));
    components -> b = malloc (sizeof (struct _branch));

    uint32_t instruction = fetch(a);
    int type = decode (a, components, instruction);
    instruction = fetch(a);
    executeRes result = execute (a, components, type);
    type = decode (a, components, instruction);
    instruction = fetch(a);

    while (result != HALT) {
        result = execute(a, components, type);
        if (result == FLUSH) { //clear pipeline of previously fetched instructions
            instruction = fetch(a);
        }
        if (result != HALT) {
        type = decode (a, components, instruction);
        instruction = fetch(a);
        }
    }

}



int main (int argc, char** argv) {
    //read the file name from command line scanf
    //fopen to create a file pointer
    //fread to create
    //create memory array with 65536 bitsx
    Arm a = malloc (sizeof (struct _arm ));

    for (int i = 0; i < MAX_ITEMS; i++) {
        a -> memory[i] = 0;
    }

    FILE* fp = NULL;
    fp = fopen(argv[1], "rb");

    fread(a -> memory, sizeof(uint32_t), MAX_ITEMS, fp);

    //initialise registers to 0

    for (int i = 0; i < 17; i++) {
        a -> registers[i] = 0;
    }

    FEcycle(a);

    //print final state

    printState(a);


}
