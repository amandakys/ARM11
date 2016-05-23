#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>

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

typedef struct _arm *Arm;
typedef struct _process *Process;
typedef struct _multiply *Multiply;
typedef struct _transfer *Transfer;
typedef struct _branch *Branch;
typedef struct _instruction *Instruction;

typedef struct _arm {
    uint32_t registers[17];
    uint32_t memory[2048];
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

bool checkCond(Arm a, uint32_t cond);
int decode(Arm a, Instruction components, uint32_t instruction);
uint32_t fetch(Arm a);


uint32_t add (uint32_t x, uint32_t y) {
    return x ^ y;
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
    a -> registers[CPSRth] = (a->registers[CPSRth] & ~(1<<30)) | (carry << 30); //set the C flag in CPSR
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
            shiftAmount = 0x000000F8 & shift;
    } else {
        //shift by a register
        uint32_t Rs = (0x000000F0 & shift) >> 4;
        shiftAmount = 0x0000000F & (a->registers[Rs]);
    }

    return shiftAmount;

}

uint32_t ror (uint32_t value, int shift) {
    if ((shift &= 31) == 0) {
        return value;
    } else {
        return (value >> shift) | (value << (32 - shift));
    }
}

uint32_t asr (uint32_t value, int shift) {
    if (((0x80000000 & value) >> 31) == 1) {
        uint32_t mask = (((uint32_t) pow(2.0, (double) shift)) - 1) << (32 - shift);
        return (value >> shift) | mask;
    } else {
        return value >> shift;
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
            case 0: carry = (0x80000000 & (value << (shiftAmount - 1))) >> 31;

            case 1: carry = 0x00000001 & (value >> (shiftAmount - 1));

            case 2: carry = 0x00000001 & (asr(value, shiftAmount - 1));

            case 3: carry = 0x00000001 & (ror(value, shiftAmount - 1));

    }

    return carry;
}

uint32_t Op2Register (Arm a, uint32_t Operand2) { //given p->Operand2 it returns the value of op2
    uint32_t Rm = 0x0000000F & (Operand2);
    uint32_t value = a -> registers[Rm];
    uint32_t shift = (0x00000FF0 & (Operand2)) >> 4;
    uint32_t shiftType = (0x00000006 & shift) >> 1;
    uint32_t shiftAmount = calculateShiftAmount(a, Operand2);
    uint32_t op2Value = NULL; 

    switch (shiftType) {
        case 0: op2Value = value << shiftAmount;
        case 1: op2Value = value >> shiftAmount;
        case 2: op2Value = asr(value, shiftAmount);
        case 3: op2Value = ror(value, shiftAmount);
    }

    return op2Value;
}


void executeP(Arm a, Process p) {
    uint32_t op2Value;
    uint32_t carry;
    if (p-> I == 1) {
        //Operand 2 is immediate constant
        uint32_t value = 0x000000FF & (p -> Operand2);
        uint32_t rotate = (0x00000F00 & (p -> Operand2)) >> 8;
        int numRotations = rotate * 2;

        op2Value = ror(value, numRotations - 1);
        carry = 0x00000001 & op2Value;
        op2Value = ror (op2Value, 1);


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
        case 1:
            result = op1Value ^ op2Value;
            setResult(a, p, result);

            if (p->S == 1) {
                setCPSR(a, carry, result);
            }

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
        case 4:
            result = add(op1Value, op2Value);
            setResult(a, p, result);
            if (p ->S == 1) {
                carry = addCarry(op1Value, op2Value);
                setCPSR(a, carry, result);
            }
        case 8:  result = op1Value & op2Value;

            if (p->S == 1) {
                setCPSR(a, carry, result);
            }

        case 9:  result = op1Value ^ op2Value;

            if (p->S == 1) {
                setCPSR(a, carry, result);
            }

        case 10: result = sub (op1Value, op2Value);
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

        case 12:
            result = op1Value | op2Value;
            setResult(a, p, result);
            if (p->S == 1) {
                setCPSR(a, carry, result);
            }
        case 13:
            result = op2Value;
            setResult(a, p, result);
            if (p->S == 1) {
                setCPSR(a, carry, result);
            }
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

void executeT(Arm a, Transfer t) {
    //Check if immediate offset or as shifted register
    if(t -> I == 1) {
        //Offset as shifted register(Op2Register is helper func of executeP)
        t -> Offset = Op2Register(a, t -> Offset);
    } else {
        //Immediate Offset, Do nothing
    }
    //Check if load from or to memory
    if(t -> L == 1) {
        //Load from memory
        a -> registers[t -> Rd] = a -> memory[t -> Rn];
    } else {
        //Load to memory
        a -> memory[t -> Rn] = a -> registers[t -> Rd];
    }
    //Check for Post-indexing after transfer
    if(t -> P == 0) {
        if(t -> U == 1) {
            a -> registers[t -> Rn] += t -> Offset;
        } else {
            a -> registers[t -> Rn] -= t -> Offset;
        }
    } else {
        //Pre-indexing, no changes to Rn (according to specs)
    }
}

void executeB(Arm a, Branch b) {
    // The offset was already sign-extended and shifted by 2 positions
    // to the right when passed to this function. If the offset is positive
    // it simply adds it to the PC and if it's negative it changes it
    // to the respective positive value and then decrements it from PC

    uint32_t MSB = (b -> Offset & 0x80000000) >> 31;
    if (MSB == 0) {
        a -> registers[15] += b -> Offset;
    } else {
        uint32_t unSigned = (~(b -> Offset) + 1);
        a -> registers[15] -= unSigned;
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
        case 0: return z == 1;
        case 1: return z == 0;
        case 10: return n == v;
        case 11: return n != v;
        case 12: return z == 0 && n == v;
        case 13: return z == 1 || n != v;
        case 14: return true;
        default: return false;
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
        b -> Offset = (0x00FFFFFF & instruction) << 2;
    }
}

int decode(Arm a, Instruction components, uint32_t instruction) {
    //take 2
    uint32_t mask = 0x0C000000;
    if (checkCond (a, components -> Cond)) {
        return INVALID;
    } else if (((instruction & mask) >> 26) == 1) {
        //single data transfer
        components -> t = malloc (sizeof (struct _transfer));
        decodeT(components -> t, instruction);
        return TRANSFER;
    } else if (((instruction & mask) >> 26) == 2) {
        //branch
        components -> b = malloc (sizeof (struct _branch));
        decodeB(components -> b, instruction);
        return BRANCH;
    } else {
        mask = 0x02000000;
        uint32_t mask2 = 0x000000F0;
        if ((instruction & mask) == 1 || (instruction & mask2) != 9) {
            //data processing
            components -> p = malloc (sizeof (struct _process));
            decodeP(components -> p, instruction);
            return PROCESSING;
        } else {
            components -> m = malloc (sizeof (struct _multiply));
            decodeM(components -> m, instruction);
            return MULTIPLY;
        }
    }
}

void execute (Arm a, Instruction components, int type) {
    switch(type) {
            case PROCESSING: executeP(a, components -> p);
            case MULTIPLY:   executeM(a, components -> m);
            case TRANSFER:   executeT(a, components -> t);
            case BRANCH:     executeB(a, components -> b);
    }
}


void FEcycle (Arm a) {
    Instruction components = malloc (sizeof (struct _instruction));

    uint32_t instruction = fetch(a); //PC = 0
    int type = decode (a, components, instruction);
    instruction = fetch(a); //PC = 1
    execute(a, components, type);
    type = decode (a, components, instruction);
    instruction = fetch(a); //PC = 2

    while (instruction != 0) {
        execute(a, components, type);
        type = decode (a, components, instruction);
        instruction = fetch(a);
    }

    execute(a, components, type);
}

/*void FEcycle(Arm a) {
    //take current instruction 

    do: 

    uint32_t instruction = fetch(a);
    //make instruction struct
    Instruction components = malloc (sizeof (struct _instruction));
    //initialise cond 
    components -> Cond = (0xF0000000 & instruction) >> 28;

    int type = decode(a, components, instruction);

    execute(a, components, type);

    while (instruction != 0) {

    }

    

    //make instruction struct
    Instruction components = malloc (sizeof (struct _instruction));
    //initialise cond 
    components -> Cond = (0xF0000000 & instruction) >> 28;

    if (checkCond (a, components -> Cond)) { //check instruction condition is satisfied 
        int type = decode(components, fetch (a)); //decode initialises relevant values in components 
        //type = the type of instruction 

        switch(type) {
            case 1: executeP(a, components -> p);
            case 2: executeM(a, components -> m);
            case 3: executeT(a, components -> t);
            case 4: executeB(a, components -> b);
        }
    } else {
        //next instruction
    }

}*/

void printState(Arm a) {
    for (int i = 0; i < 13; i++) {
        printf ("$%i: %i", i, a->registers[i]);
    }

    printf("PC: %i", (a->registers[15] - 1) * 4);
    printf("CPSR: %i", a->registers[16]);
}
/*Son trial
void FEcycle(Arm a) {
  uint32_t stageFetch;//instruction currently fetch
  uint32_t fetched = 0;//the one that fetched in the last loop iteration
  unin32_t stageDecode = 0;//instruction currently decode
  uint32_t decoded = 0;//the one decoded last loop iteration
  uint32_t PC = a->registers[15];

  while (a->memory[PC] != \0){//when memory is not empty
    // The order of reading is fetch first then decode then execute
    // so that it will satisfy the spec : the one being fetched
    // is 2 instructions further than the one being executed.
    // Since we have to fetch new one before decode old one, decode new one
    // before execute old one I use fetched and decoded variable to
    // save the values of old stageFetch and old stageDecode as they will
    // be overided by the new values
    stageFetch = fetch(a);//fetch the next one
    Instruction components = malloc (sizeof (struct _instruction));//initialize stuff
    if (fetched != 0) {//if there is a thing fetched before then decode it
      components -> Cond = (0xF0000000 & fetched) >> 28;
      uint32_t stageDecode = decode(components, Fetched)
    }
    if (decoded != 0) {//if there is a thing decoded before then execute it
      if(checkCond (a, components -> Cond)) {//condition is checked right before the execution
        switch(decoded) {
            case 1: executeP(a, components -> p);
            case 2: executeM(a, components -> m);
            case 3: executeT(a, components -> t);
            case 4: executeB(a, components -> b);
        }
      }
    }
    fetched = stageFetch;//save the instruction fetched in this loop iteration
    decoded = stageDecode;//save the instruction decoded in this loop iteration
  }

}*/

int main (int argc, char** argv) {
    //read the file name from command line scanf
    //fopen to create a file pointer
    //fread to create
    //create memory array with 65536 bits

    Arm a = malloc (sizeof (struct _arm));

    for (int i = 0; i < 2048; i++) {
        a -> memory[i] = 0;
    }

    FILE* fp;
    fp = fopen(argv[0], "rb");
    int i = fread(a -> memory, 4, 2048, fp); //adjust parameters
    assert (i == 2048);

    //initialise registers to 0

    for (int i = 0; i < 17; i++) {
        a -> registers[i] = 0;
    }

    //print initial state
    printState(a);

    FEcycle(a);

    //print final state 

    printState(a);

}
