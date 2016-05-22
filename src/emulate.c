#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define PROCESSING 1
#define MULTIPLY 2
#define TRANSFER 3
#define BRANCH 4
#define CPSRth 16
#define Nth = 31
#define Zth = 30
#define Cth = 29
#define Vth = 28

typedef struct _arm *Arm;
typedef struct _process *Process;
typedef struct _multiply *Multiply;
typedef struct _transfer *Transfer;
typedef struct _branch *Branch;
typedef struct _instruction *Instruction;

typedef struct _arm {
    uint32_t registers[17];
    //register 15 is PC


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

typedef struct Transfer {
    uint32_t I;
    uint32_t P;
    uint32_t U;
    uint32_t L;
    uint32_t Rn;
    uint32_t Rd;
    uint32_t Offset;
}

typedef struct _branch {
    uint32_t Offset;
} branch;

FEcycle() {
    Instruction components = malloc (sizeof (struct _instruction));
    Instruction -> cond = (0xF0000000 & instruction) >> 28;
    if (checkCond (a, cond)) {
        int type = decode(components, fetch (a)); //initialises relevant values

        switch(type) {
            1: executeP(components -> p);
            2: executeM(components -> m);
            3: executeT(components -> t);
            4: executeB(components -> b);
        }
    } else {
        //next instruction
    }

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
        uint32_t Rm = 0x0000000F & (p->Operand2); //address of value to be shifted
        uint32_t value = a -> registers[Rm];
        uint32_t shift = (0x00000FF0 & (p->Operand2)) >> 4;
        uint32_t bit4 = 0x00000001 & shift;
        uint32_t shiftType = 0x00000006 & shift; >> 1;

        if (bit4 == 0) {
            //shift by a constant amount
            uint32_t shiftAmount = 0x000000F8 & shift;
        } else {
            //shift by a register
            uint32_t Rs = (0x000000F0 & shift) >> 4;
            uint32_t shiftAmount = 0x0000000F & (a->registers[Rs]);
        }

        switch (shiftType) {
            0:
                op2Value = value << shiftAmount - 1;
                carry = (0x80000000 & op2Value) >> 31;
                op2Value = op2Value << 1;

            1:
                op2Value = value >> shiftAmount - 1;
                carry = 0x00000001 & op2Value;
                op2Value = op2Value >> 1;

            2:
                op2Value = asr(value, shiftAmount - 1);
                carry = 0x00000001 & op2Value;
                op2Value = asr(value, 1);
            3:
                op2Value = ror(value, shiftAmount - 1);
                carry = 0x00000001 & op2Value;
                op2Value = ror (op2Value, 1);

        }
    }

    uint32_t op1Value = a->registers[p->Rn];
    switch (p -> Opcode) {
        0:
            a->registers[p->Rd] = op1Value & op2Value;
            if (p->S == 1) {
                a -> registers[CPSR] = a->registers[CPSR] & ~(1<<30) | (carry << 30); //set the C flag in CPSR
            }
        1:
        2:
        3:
        4:
        8:
        9:
        10:
        12:
        13:
    }

    if (p -> S == 1) {

    } else {
        //CPSR is unaffected
    }

}

uint32_t Op2Register (uint32_t Operand2) {

}

uint32_t ror (uint32_t value, int shift) {
    if ((shift &= 31) == 0) {
        return value;
    } else {
        return (value >> shift) | (value << (32 - shift));
    }
}

uint32_t asr (uint32_t value, int shift) {
    if (0x80000000 & value >> 31 == 1) {
        uint32_t mask = (pow(2, shift) - 1) << (32 - shift);
        return (value >> shift) | mask;
    } else {
        return value >> shift;
    }

}

void executeM(Arm a, Multiply m) {
    mul(a->registers[m->Rd], a->registers[m->Rm], a->registers[m->Rs]);
    if (m->A == 1) {
        a->registers[m->Rd] += a->registers[m->Rn];
    }
    if (m->S == 1) {
        setN(a,m);
        setZ(a,m);
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
    a->registers[CPSRth] = a->registers[CPSRth] & ~(1 << Nth) | (bit31 << Nth);
}

void mul(Arm a, Multiply m) {
    uint64_t result = (uint64_t) (a->registers[m->Rm] * a->registers[m->Rs]);
    a->registers[m->Rd] = (uint32_t) (result & 0xFFFFFFFF);

}

void executeT(Transfer t) {
    //Check if immediate offset or as shifted register
    if(t -> I == 1) {
        //Offset as shifted register(Op2Register is helper func of executeP)
        t -> Offset = Op2Register(t -> Offset);
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
            a -> registers[t -> Rn] += a -> Offset;
        } else {
            a -> registers[t -> Rn] -= a -> Offset;
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
    if (MSB = 0) {
        a -> register[15] += Offset;
    } else {
        uint32_t unSigned = (~(b -> Offset) + 1);
        b - > register[15] -= unSigned;
    }

}

bool checkCond(Arm a, uint32_t cond) {
    uint32_t CPSR = (a -> registers[16] & mask) >> 28;

    uint32_t n = (0x00000008 & CPSR) >> 3;
    uint32_t z = (0x00000004 & CPSR) >> 2;
    uint32_t c = (0x00000002 & CPSR) >> 1;
    uint32_t v = 0x00000001 & CPSR;

    switch(cond) {
        0: return z == 1;
        1: return z == 0;
        10: return n == v;
        11: return n != v;
        12: return z == 0 && n == v;
        13: return z == 1 || n != v;
        14: return true;
    }
}

uint32_t fetch(Arm a) { //returns instruction
    //increment PC
    uint32_t PC = a -> registers[15];
    //convert PC from binary to integer rep
    //get value at memory
    PC++;
    return memory[(int) (PC-1)];
}

int decode(Instruction components, uint32_t instruction) {
    //take 2
    uint32_t mask = 0x0C000000;
    if ((instuction&mask) >> 26 == 1) {
        //single data transfer
        components -> t = malloc (sizeof (struct _transfer));
        decodeT(t, instruction);
        return TRANSFER;
    } else if (instruction&mask >> 26 == 2) {
        //branch
        components -> b = malloc (sizeof (struct _branch));
        decodeB(b, instruction);
        return BRANCH;
    } else {
        mask = 0x02000000;
        mask2 = 0x000000F0;
        if (instruction&mask == 1 || instruction&mask2 != 9) {
            //data processing
            components -> p = malloc (sizeof (struct _processing));
            decodeP(p, instruction);
            return PROCESSING;
        } else {
            components -> m = malloc (sizeof (struct _multiply));
            decodeM(m, instruction);
            return MULTIPLY;
        }
    }
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


    uint32_t MSB = (0x00800000 & Instruction) >> 23;
    if (MSB = 1) {
        b -> Offset = (0xFF000000 | Instruction) << 2;
    } else {
        b -> Offset = (0x00FFFFFF & Instruction) << 2;
    }
}




int destination;
int* dst = &destination;
void getBin (int *dst, uint32_t x) {

}

void printBits (uint32_t x) {
    int i;
    uint32_t mask = 1 << 31;

    for(i = 0; i <32; ++1) {
        printf("%i", (x&mask) != 0);
        x <<= 1;
    }
    printf("\n");
}

int main (int argc, char** argv) {
    //read the file name from command line scanf
    //fopen to create a file pointer
    //fread to create
    //create memory array with 65536 bits

    Arm a = malloc (sizeof (struct _arm));

    for (int i = 0; i < 65536; i++) {
        a -> memory[i] = 0;
    }

    FILE* fp;
    fp = fopen(argv[0], "rb");
    fread(a -> memory, 4, sizeof (a -> memory), fp); //adjust parameters

    //initialise registers to 0
    for (int y = 0; y < 17; y++) {
        for (int x = 0, x < 32; x++) {
            a -> registers[y][x] = 0;
        }
    }

}
do
a = fetch(a)
da = decode a
fetch b
excute a
decode b
fecth c
while ( instructions not empty) {
    excute(da)
    decode things in fecth

    fecth
    b = fetch(b);
}

 while instruction not empty
    fetch
    decode
    execute
