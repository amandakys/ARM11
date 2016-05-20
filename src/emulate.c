#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct Arm {
    uint32_t registers[17];
    //register 15 is PC
};

FEcycle() {
    Cond 
    IO
    int Opcode; //1234
    Sc
    Rn
    Rd
    Rm
    Rs
    O2

    decode(); //initialises relevant values 
    switch(Opcode) {
        1: execute1()
        2: execute2()
        3: execute3()
        4: execute4()
    }
}

uint32_t fetch(Arm a) {
    //increment PC
    uint32_t PC = a.registers[15];
    //convert PC from binary to integer rep 
    //get value at memory 
    PC++;
    return memory[(int) (PC-1)];
}

int decode(uint32_t instruction) {

}

execute (int action) {
    switch (action) {
        
    }
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
  





