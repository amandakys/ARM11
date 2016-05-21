#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define PROCESSING 1
#define MULTIPLY 2
#define TRANSFER 3
#define BRANCH 4
#define CPSR 16
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

void executeP(Process p) {

}

void executeM(Arm a, Multiply m) {
    mul(registers[m->Rd], registers[m->Rm], registers[m->Rs]);
    if (m->A == 1) {
        registers[m->Rd] += registers[m->Rn];
    }
    if (m->S == 1) {
        setN(registers[CPSR], registers[m->Rd]);
        setZ(registers[CPSR], registers[m->Rd]);
    }

}

void setZ(uint32_t cpsr, uint32_t result) {
    if (cpsr == 0) {
        cpsr |= 1 << Zth;
    } else {
        cpsr &= ~(1 << Zth);
    }
}

void setN(uint32_t cpsr, uint32_t result) {
    int bit31 =  RD >> Nth;
    cpsr = cpsr & ~(1 << Nth) | (bit31 << Nth);
}

void mul(uint32_t des, uint32_t first, uint32_t second) {
    uint16_t firstH = first >> 16;
    uint16_t firstL = first & 0xFFFF;
    uint16_t secondH = second >> 16;
    uint16_t secondL = second & 0xFFFF;
    uint64_t result = firstH * secondH;
    result <<= 16;
    result += firstH * secondL;
    result += firstL * secondH;
    result <<= 16;
    result += firstL * secondL;
    des = result & 0xFFFFFFFF;
}

void executeT(Transfer t) {

}

void executeB(Branch b) {

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
        if (instruction&mask == 1) {
            //data processing
            components -> p = malloc (sizeof (struct _processing));
            decodeP(p, instruction);
            return PROCESSING;
        } else { //they are both 000
            mask = 0x000000F0;
            if (instruction&mask == 9) {
                //multiply
                components -> m = malloc (sizeof (struct _multiply));
                decodeM(m, instruction);
                return MULTIPLY;
            } else {
                //data processing
                components -> p = malloc (sizeof (struct _processing));
                decodeP(p, instruction);
                return PROCESSING;
            }
        }
    }
}

void decodeT(Transfer t, uint32_t instruction) {

    t -> Cond = (0xF0000000 & instruction) >> 28;
    t -> I =
    t -> P =
}

void decodeP(Process p, uint32_t instruction) {
    p -> Cond = (0xF0000000 & instruction) >> 28;
}

void decodeM(Multiply m, uint32_t instruction) {
    m -> Cond = (0xF0000000 & instruction) >> 28;
}

void decodeB(Branch b, uint32_t instruction) {
    b -> Cond = (0xF0000000 & instruction) >> 28;
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

    uint32_t memory[2048];

    for (int i = 0; i < 65536; i++) {
        memory[i] = 0;
    }

    FILE* fp;
    fp = fopen(argv[0], "rb");
    fread(memory, 4, sizeof memory, fp); //adjust parameters

    //initialise registers to 0
    struct arm a1;
    for (int y = 0; y < 17; y++) {
        for (int x = 0, x < 32; x++) {
            a1.registers[y][x] = 0;
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






