#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define PROCESSING 1
#define MULTIPLY 2
#define TRANSFER 3
#define BRANCH 4
#define SPECIAL 5
#define LABEL 6
#define MAX_ITEMS 16384
#define MAX_CHARS 511

typedef struct _label *LabelNode;
typedef struct _ass *Ass;
typedef struct _process *Process;

typedef struct _ass {
    uint32_t memory[MAX_ITEMS];
    LabelNode head;
} ass;

typedef struct _label {
  char *l;
  int p;
  LabelNode next;
} label;

typedef struct _process {
    uint32_t Cond;
    uint32_t I;
    uint32_t Opcode;
    uint32_t S;
    uint32_t Rn;
    uint32_t Rd;
    uint32_t Operand2;
} process;

// helper function to add a new label to the symbol table(using linked list)
void push(LabelNode head, char *label, int position) {
    LabelNode current = head;
    while (current->next != NULL) {
        current = current->next;
    }

    /* now we can add a new variable */
    current->next = malloc(sizeof(LabelNode));
    current->next->l = label;
    current->next->p = position;
    current->next->next = NULL;
}
//First pass, going through the list to create symbol table
LabelNode createTable(FILE *fr) {
  LabelNode head;
  char *next;
  int position = 0;
  while (fgets(next, MAX_CHARS, fr)) {
    char temp[strlen(next) + 1]; //duplicating next to used for comparing, 1 for NUL
    strcpy(temp, next);
    char *current = strtok(next,":");
    if(*current != temp) {
      if(head == NULL) {
        head = malloc(sizeof (LabelNode));
        head->l = current;
        head->p = position;
      } else {
        push(head, current, position);
      }
    } else {
      position += 4;
    }

  }
  return head;
}

//This function choose with translation functino will be called
int identify(char *instruction) {
  if((strcmp(instruction,"add") == 0) || (strcmp(instruction,"sub") == 0)
  || (strcmp(instruction,"rsb") == 0) || (strcmp(instruction,"and") == 0)
  || (strcmp(instruction,"eor") == 0) || (strcmp(instruction,"orr") == 0)
  || (strcmp(instruction,"mov") == 0) || (strcmp(instruction,"tst") == 0)
  || (strcmp(instruction,"teq") == 0) || (strcmp(instruction,"cmp") == 0)) {
    return PROCESSING;
  } else if ((strcmp(instruction,"mul") == 0)
          || (strcmp(instruction,"mla") == 0)) {
    return MULTIPLY;
  } else if ((strcmp(instruction,"ldr") == 0)
          || (strcmp(instruction,"str") == 0)) {
    return TRANSFER;
  } else if ((strcmp(instruction,"lsl") == 0)
          || (strcmp(instruction,"andeq") == 0)) {
    return SPECIAL;
  } else if ((strcmp(instruction,"beq") == 0)
      || (strcmp(instruction,"bne") == 0) || (strcmp(instruction,"bge") == 0)
      || (strcmp(instruction,"blt") == 0) || (strcmp(instruction,"bgt") == 0)
      || (strcmp(instruction,"ble") == 0) || (strcmp(instruction,"b") == 0)) {
    return BRANCH;
  } else {
    return LABEL;
  }
}

//input "r1" "r2" "r3" and output 1,2,3 in binary
uint32_t regTrans(char *reg) {
    if (reg[0] != '\n') { //remove the letter 'r' from r1, r2, ...
            memmove(reg, reg+1, strlen(reg));
    } else {
        fprintf(stderr,"ERROR: EMPTY STRING!");
    }
    int num = atoi(reg);//get the number in integer
    return (uint32_t) num;
}

uint32_t convertImm(char *Imm) {
    if (Imm[0] != '\n') { //remove the character '#' from #341, #0x0ff, ...
            memmove(Imm, Imm+1, strlen(Imm));
    } else {
        fprintf(stderr,"ERROR: EMPTY STRING!");
    }
    if (Imm[0] == '0') {
        return (uint32_t) strtol(Imm, NULL, 0);
    } else {
        return (uint32_t) atoi(Imm);
    }

}
// reorder the instruction to little-endian
uint32_t reorder(uint32_t ins) {
    uint32_t first = ins >> 24;                         //take the first byte
    uint32_t second = ((ins >> 16) & ~(1<<8)) << 8;     //take the second byte
    uint32_t third = ((ins >> 8) & ~(1<<8)) << 16;      //take the third byte
    uint32_t fourth = ((ins) & ~(1<<8)) << 24;
    return (fourth | third | second | first);

}

void opCodeP(char *mnemonic, Process p) {
    p->S = 0 << 20;
    p->Cond = 14 << 28;
    if (strcmp(mnemonic,"and") == 0) {
        p->Opcode = 0;
    } else if (strcmp(mnemonic,"eor") == 0) {
        p->Opcode = 1;
    } else if (strcmp(mnemonic,"sub") == 0) {
        p->Opcode = 2;
    } else if (strcmp(mnemonic,"rsb") == 0) {
        p->Opcode = 3;
    } else if (strcmp(mnemonic,"add") == 0) {
        p->Opcode = 4;
    } else if (strcmp(mnemonic,"orr") == 0) {
        p->Opcode = 12;
    } else if (strcmp(mnemonic,"mov") == 0) {
        p->Opcode = 13;
    } else if (strcmp(mnemonic,"tst") == 0) {
        p->Opcode = 8;
        p->S = 1;
    } else if (strcmp(mnemonic,"teq") == 0) {
        p->Opcode = 9;
        p->S = 1 << 20;
    } else {
        p->Opcode = 10;
        p->S = 1 << 20;
    }
    p->Opcode <<= 21;
}

//rotate right
uint32_t ror(uint32_t value, int shift) {
    if ((shift &= 31) == 0) {                                   //divisible by 32
        return value;
    } else {
        return (value >> shift) | (value << (32 - shift));
    }
}

//rotate left
uint32_t rol(uint32_t value, int shift) {
    if ((shift &= 31) == 0) {                                   //divisible by 32
        return value;
    } else {
        return (value << shift) | (value >> (32 - shift));
    }
}

//Find Shift type
int findShiftType(char *shift) {
    if(strcmp(shift,"lsl")) {
        return 0;
    } else if(strcmp(shift,"lsr")) {
        return 1;
    } else if(strcmp(shift,"asr")) {
        return 2;
    } else if(strcmp(shift,"ror")) {
        return 3;
    } else {
        fprintf(stderr,"ERROR: CAN NOT FIND SHIFT TYPE");
    }
}

//Translate Operand2
void operand2Handler(char *operand2, char *shType, char *rest, Process p) {
    if (operand2[0] == 'r' ) {
        p->I = 0;                                               //Operand2 is register
        uint32_t Rm = regTrans(operand2);
        if(shType == NULL) {                                    //No shift involved :D
            p->Operand2 = Rm;

        } else {                                                //Yes shift involved :<
            int shiftType = findShiftType(shType);
            uint32_t Code;                                      //Opcode of shift type
            switch (shiftType) {
                case 0: Code = 0; break;
                case 1: Code = 1 << 5;
                case 2: Code = 2 << 5;
                case 3: Code = 3 << 5;
            }

            uint32_t Shift;

            if (rest[0] == 'r') {                               //shift by a register
                uint32_t Rs = regTrans(rest) << 8;
                Shift = (1<<4) | Code | Rs;
            } else if (rest[0] == '#') {                        //shift by a constant
                uint32_t num = convertImm(rest) << 7;
                Shift = num | Code;
            }

            p->Operand2 = Rm | Shift;
        }

    } else if (operand2[0] == '#') {
        p->I = 1 << 25;                                         //Operand2 is constant
        uint32_t value = convertImm(operand2);
        for (int i =0; i < 16; i ++) {
            uint32_t shifted = rol(value, i*2) & 0x000000FF;    // rotate left i*2 times, keep the first 8 bits
            if(ror(shifted, i*2) == value) {                    //rotate right i*2 times with the first 8 bits
                uint32_t Imm = shifted;
                uint32_t rotate = (uint32_t) (i*2) << 8;
                p->Operand2 = shifted | rotate;
            }
        }
        if (p->Operand2 == NULL) {
            fprintf(stderr,"ERROR: CAN NOT REPRESENT NUMBER!");
        }

    } else {
        fprintf(stderr,"ERROR: CAN NOT MATCH SYNTAX");
    }

}

//concatenate stuff in struct _process
uint32_t concatP(Process p) {
    return p->Cond | p->I | p->Opcode | p-> S | p->Rn | p->Rd | p->Operand2;
}
//Translate Processing
void translateP(char *ins, Ass a, int pos) {
    Process p = malloc(sizeof (struct _process));
    p->Cond = 14 << 25;                                         //Initialise struct p
    char *mnemonic = strtok(ins," ,");                          //read the mnemonic
    opCodeP(mnemonic, p);                                       //Set opcode based on mnemonic
    if ((p->Opcode >= 0) & (p->Opcode <= 4) | (p->Opcode == 12)) { //if Opcode is first type
        p->Rn = regTrans(strtok(NULL," ,")) << 16;              //read Rn
        p->Rd = regTrans(strtok(NULL," ,")) << 12;              //Read Rd
        char *operand2 = strtok(NULL," ,");                     //read first part of Operand2: r3, #234
        char *shType = strtok(NULL," ,");                       //read potential second part of Operand2: lsl, ror
        char *rest = strtok(NULL," ,");                         //read potential third part of Operand2: r4, r5
        operand2Handler(operand2, shType, rest, p);             //Magic function that deal with Operand2
        uint32_t result = concatP(p);                           //Concatenate stuff in Process p
        a->memory[pos] = reorder(result);                       //Save result in little-endian
    } else if (p->Opcode = 13) {
        p->Rn = 0;                                              // Rn is ignored
        p->Rd = regTrans(strtok(NULL," ,")) << 12;              //read Rd
        char *operand2 = strtok(NULL," ,");                     //read first part of Operand2: r3, #234
        char *shType = strtok(NULL," ,");                       //read potential second part of Operand2: lsl, ror
        char *rest = strtok(NULL," ,");                         //read potential third part of Operand2: r4, r5
        operand2Handler(operand2, shType, rest, p);             //Magic function that deal with Operand2
        uint32_t result = concatP(p);                           //Concatenate stuff in Process p
        a->memory[pos] = reorder(result);                       //Save result in little-endian
    } else {
        p->S = 1 << 20;                                         // CSPR flags will be updated
        p->Rn = regTrans(strtok(NULL," ,")) << 16;
        p->Rd = 0;                                              //Result is not written
        char *operand2 = strtok(NULL," ,");                     //read first part of Operand2: r3, #234
        char *shType = strtok(NULL," ,");                       //read potential second part of Operand2: lsl, ror
        char *rest = strtok(NULL," ,");                         //read potential third part of Operand2: r4, r5
        operand2Handler(operand2, shType, rest, p);             //Magic function that deal with Operand2
        uint32_t result = concatP(p);                           //Concatenate stuff in Process p
        a->memory[pos] = reorder(result);                       //Save result in little-endian
    }
}

//Translate Multiply
void translateM(char *ins, Ass a, int pos) {
    uint32_t Cond = 14 << 28;                                   // Condition field is 1110
    uint32_t S = 0;                                             // S field is 0
    char *mnemonic = strtok(ins," ,");
    uint32_t A = 0;
    if (strcmp(mnemonic,"mul") != 0) {
        A = 1;
    }
    char *Rds = strtok(NULL," ,");
    uint32_t Rd = regTrans(Rds) << 16;                          //Rd: bit 16 to bit 19
    char *Rns = strtok(NULL, " ,");
    uint32_t Rn = regTrans(Rns) << 12;                          //Rn: bit 12 to bit 15
    char *Rss = strtok(NULL," ,");
    uint32_t Rs = regTrans(Rss) << 8;                           //Rs: bit 8 to bit 11
    uint32_t other = 9 << 4;                                    // the other field is 1001
    char *Rms = strtok(NULL, " ,");
    uint32_t Rm = 0;
    if (Rms != NULL) {
        Rm = regTrans(Rms);
    }
    uint32_t result = Cond | A | S | Rd | Rn | Rs | Rm;
    a->memory[pos] = reorder(result);
}

//Translate Transfer
void translateT(char *ins, Ass a, int pos) {
  uint32_t condb = 0xE4000000; //Cond(Ignored) & 01 part
  //Determine opreration (ldr or str)
  char *op = strtok(ins, " ,[]");
  uint32_t opb = 0; // for ldr
  if(strcmp(op, "str") == 0) {
    opb = 1;  // for str
  }
  //determine Rd
  char *rd = strtok(NULL, " ,[]");
  uint32_t rdb = regTrans(rd);
  //determine offset, Rn, P fields from expression input
  char *expr = strtok(NULL, " ,[]");
  uint32_t offsetb;
  uint32_t rnb;
  uint32_t pb;
  uint32_t ub = 1;
  if(*expr == '=') { // Numeric comstant
    uint32_t expb = convertImm(expr);
    // Add another if here to make use of mov shortcuttttt
    int end = MAX_ITEMS;
    while(1) {
      if(a -> memory[end] == NULL) {
        a -> memory[end] = expb;
        break;
      }
      end -= 1;
    }
    offsetb = (uint32_t) (end - pos); //Calcuate offset
    rnb = (uint32_t) pos; //PC as based register
  } else { //Pre & Post Indexing
    char *rn = strtok(NULL, " ,[]");
    rnb = regTrans(rn);

    char *offset = strtok(NULL, " ,[]");
    if (*offset != NULL) { // have expression after Rn input
      if (*(offset + strlen(offset) - 1) == ']') {
        pb = 1;
        *(offset + strlen(offset) - 1) = '\0';// remove ']' from offset;
      } else {
        pb = 0;
      }
      offsetb = convertImm(offset);//using helper func of translateT
    } else { // no expression
      offsetb = 0;
    }
  }
  //Combining fields & put in pos
  uint32_t result = condb | pb << 24 | ub << 23 | opb << 20 | rnb << 16 |
                    rdb << 12 | offsetb;
  a -> memory[pos] = reorder(result);
}

//Translate Branch
void translateB (char *ins, Ass a, int pos) {
  //Get the cond
  char *cond = strtok(ins," ");
  uint32_t condb; // Cond field
  if(strcmp(cond,"beq") == 0) {
    condb = 0;
  } else if(strcmp(cond,"bne") == 0) {
    condb = 1 << 28;
  } else if(strcmp(cond,"bge") == 0) {
    condb = 10 << 28;
  } else if(strcmp(cond,"blt") == 0) {
    condb = 11 << 28;
  } else if(strcmp(cond,"bgt") == 0) {
    condb = 12 << 28;
  } else if(strcmp(cond,"ble") == 0) {
    condb = 13 << 28;
  } else {
    condb = 14 << 28;
  }
  //Finding label position
  char *label = strtok(NULL," ");
  int labelposition;
  LabelNode current = a -> head;
  while(current != NULL) {
    if(strcmp(current -> l, label) == 0) {
      labelposition = current -> p;
      break;
    } else {
      current = current -> next;
    }
  }
  //Finding offset field
  int offset = pos - labelposition;
  uint32_t offset24bit = (uint32_t) offset;
  offset24bit = offset24bit >> 2;
  //1010 part
  uint32_t mid = 10 << 24;
  //Combining to get full instruction
  uint32_t result = condb | mid | offset24bit;
  //Reorder bytes & put in memory
  a -> memory[pos] = reorder(result);
}

//Translate Special
void translateS(char *ins, Ass a, int pos) {
  //Get the Mnemonic
  char *mnemonic = strtok(ins," ,");
  uint32_t result;
  if(strcmp(mnemonic, "andeq") == 0) { //All 0 case. Need to check para ???
    result = 0x00000000;
  } else {
    uint32_t condb = 0xE0000000; // Condition field
    uint32_t opb = 13 << 21; // Opcode Field
    char *rn = strtok(NULL, " ,"); // get Rn
    uint32_t rnb = regTrans(rn); // convert Rn to binary(for Rd field)
    uint32_t shiftTypeb = 0 << 4; //shift type & 0 at bit 4
    char *expr = strtok(NULL, " ,");
    uint32_t exprb = (uint32_t) strtol(expr, NULL, 0) << 7; // How TO MAKE SURE THIS FIT IN 5 bit ?
    exprb &= 0x00000F80; //Just to make sure the integer fit in bit 7-11

    result = condb | opb | rnb << 12/*for Rd field*/| exprb | shiftTypeb | rnb;
  }
  a -> memory[pos] = reorder(result);
}


void translate(Ass a, FILE *fr, LabelNode head) {
  char *next;
  int position = 0;
  while (fgets(next, MAX_CHARS, fr)) {
    char *newtemp = calloc(strlen(next) + 1, sizeof(char));
    strcpy(newtemp, next);
    char *mnemonic = strtok(newtemp," ");
    switch(identify(mnemonic)) {
      case 1: translateP(next, a, position); position += 4; break;
      case 2: translateM(next, a, position); position += 4; break;
      case 3: translateT(next, a, position); position += 4; break;
      case 4: translateB(next, a, position); position += 4; break;
      case 5: translateS(next, a, position); position += 4; break;
      case 6: break;
    }
  }
}



int main(int argc, char **argv) {
  //open binary file to write
  Ass a = malloc (sizeof(struct _ass));
  int i;
  for (i = 0; i < 16384; i++) {
      a -> memory[i] = 0;
  }

  FILE *fr;
  fr = fopen(argv[0], "r");//open string file

  LabelNode head = createTable(fr);//list of label with position

  translate(a, fr, head);

  FILE *fw;
  fw = fopen(argv[1], "wb");
  fwrite(a -> memory, 4, sizeof (a -> memory), fw);// binary writer
<<<<<<< HEAD
=======

  printf("Non-zero memory:\n");
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (a->memory[i] != 0) {
            printf("0x%08x: 0x%08x\n", i*4, a->memory[i]);
        }
    }
>>>>>>> 674c606418947d8154832d41460e63b50dea93f2
}
