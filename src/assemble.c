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
