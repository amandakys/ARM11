#include <stdlib.h>
typedef struct _lable *LabelNode;
typedef struct _ass *Ass;
typedef struct _process *Process;

#define PROCESSING 1
#define MULTIPLY 2
#define TRANSFER 3
#define BRANCH 4
#define SPECIAL 5
#define LABEL 6
#define MAX_ITEMS 16384
#define MAX_CHARS 511


typedef struct _ass {
    uint32_t memory[MAX_ITEMS];
};

typedef struct _label {
  char *l;
  int p;
  LabelNode next;
};

typedef struct _process {
    uint32_t Cond;
    uint32_t I;
    uint32_t Opcode;
    uint32_t S;
    uint32_t Rn;
    uint32_t Rd;
    uint32_t Operand2;
} ;

// helper function to add a new label to the symbol table(using linked list)
void push(node_t * head, char *label, int position) {
    node_t * current = head;
    while (current->next != NULL) {
        current = current->next;
    }

    /* now we can add a new variable */
    current->next = malloc(sizeof(LabelNode));
    current->next->*l = label;
    current->next->p = position;
    current->next->next = NULL;
}
//First pass, going through the list to create symbol table
LabelNode createTable(FILE *fr) {
  LabelNode head;
  char *next;
  int position = 0;
  while (fgets(next, MAX_CHARS, fr)) {
    char *current = strtok(next,":");
    if(current != next) {
      if(head == NULL) {
        head = malloc(sizeof (LabelNode))
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
      || (strcmp(instruction,"ble") == 0) || (strcmp(instruction,"b") == 0) {
    return BRANCH;
  } else {
    return LABEL;
  }
}

//input "r1" "r2" "r3" and output 1,2,3 in binary
uint32_t regTrans(char *reg) {
    if (reg[0] == '\n') { //remove the letter 'r' from r1, r2, ...
            memmove(reg, reg+1, strlen(reg));
    }
    int num = atoi(reg);//get the number in integer
    return (uint32_t) num;
}
// reorder the instruction to little-endian
uint32_t reorder(uint32_t ins) {
    uint32_t first = ins >> 24;//take the first byte
    uint32_t second = ((ins >> 16) & ~(1<<8)) << 8;//take the second byte
    uint32_t third = ((ins >> 8) & ~(1<<8)) << 16;//take the third byte
    uint32_t fourth = ((ins) & ~(1<<8)) << 24;
    return (fourth | third | second | first);

}

void opCodeP(char *mnemonic, PROCESSING p) {
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

void operand2Handler(char *operand2, char *rest, PROCESSING p) {
    if (rest == NULL) {
        if (operand2[0] == 'r' ) {
            p->Operand2 = regTrans(operand2);
            p->I = 0;
        } else if (operand[0] == '#') {
            p->Operand2 = regTrans(operand2);
            P->I = 1 << 25;
        }
    } else {

    }

}
//Translate Processing
void translateP(char *ins, Ass a, int pos) {
    PROCESSING p = malloc(sizeof (struct _process));
    char *mnemonic = strtok(ins," ");
    opCodeP(mnemonic, p);
    p->Rn = regTrans(strtok(NULL," ")) << 16;
    p->Rd = regTrans(strtok(NULL," ")) << 12;
    char *operand2 = strtok(NULL," ");
    char *rest = strtok(NULL," ");


}

//Translate Multiply
void translateM(char *ins, Ass a, int pos) {
    uint32_t Cond = 14 << 28; // Condition field is 1110
    uint32_t S = 0; // S field is 0
    char *mnemonic = strtok(ins," ");
    uint32_t A = 0;
    if (strcmp(mnemonic,"mul") != 0) {
        A = 1;
    }
    char *Rds = strtok(NULL," ");
    uint32_t Rd = regTrans(Rds) << 16;//Rd: bit 16 to bit 19
    char *Rns = strtok(NULL, " ");
    uint32_t Rn = regTrans(Rns) << 12;//Rn: bit 12 to bit 15
    char *Rss = strtok(NULL," ");
    uint32_t Rs = regTrans(Rss) << 8;//Rs: bit 8 to bit 11
    uint32_t other = 9 << 4; // the other field is 1001
    char *Rms = strtok(NULL, " ");
    uint32_t Rm = 0;
    if (Rms != NULL) {
        Rm = regTrans(Rms);
    }
    uint32_t result = Cond | A | S | Rd | Rn | Rs | Rm;
    a->memory[pos] = reorder(result);
}

//Translate Transfer
void translateT(char *ins, Ass a, int pos) {

}

//Translate Branch
void translateB (char *line, Ass a, int position) {
  //Get the cond
  char *cond = strtok(line," ");
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

  char *label = strtok(NULL," ");
  LabelNode current = a -> head;

  int offset = position

}

//Translate Special
void translateS(char *ins, Ass a, int pos) {

}


void translate(Ass a, FILE fr, LabelNode head) {
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

  for (int i = 0; i < 16384; i++) {
      a -> memory[i] = 0;
  }

  FILE *fr;
  fr = fopen(argv[0], "r");//open string file

  LabelNode head = createTable(fr);//list of label with position

  translate(a, fr, head);

  FILE *fw;
  fw = fopen(argv[1], "wb");
  fwrite(a -> memory, 4, sizeof (a -> memory), fp);// binary writer
}
