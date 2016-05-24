#include <stdlib.h>
typedef struct _lable *LabelNode;
typedef struct _ass *Ass;

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
  } else if ((strcmp(instruction,"beg") == 0)
      || (strcmp(instruction,"bne") == 0) || (strcmp(instruction,"bge") == 0)
      || (strcmp(instruction,"blt") == 0) || (strcmp(instruction,"bgt") == 0)
      || (strcmp(instruction,"ble") == 0) || (strcmp(instruction,"b") == 0) {
    return BRANCH;
  } else {
    return LABEL;
  }
}

uint32_t regTrans(char *reg) {
    if (reg[0] == '\n') { //remove the letter 'r' from r1, r2, ...
            memmove(reg, reg+1, strlen(reg));
    }
    int num = atoi(reg);//get the number in integer
    return (uint32_t) num;
}

uint32_t reorder(uint32_t ins) {
    uint32_t first = ins >> 24;//take the first byte
    uint32_t second = ((ins >> 16) & ~(1<<8)) << 8;//take the second byte
    uint32_t third = ((ins >> 8) & ~(1<<8)) << 16;//take the third byte
    uint32_t fourth = ((ins) & ~(1<<8)) << 24;
    return (fourth | third | second | first);

}

void translateP(char *ins, Ass a, int pos) {

}

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

void translateT(char *ins, Ass a, int pos) {

}

void translateB(char *ins, Ass a, int pos) {

}

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
