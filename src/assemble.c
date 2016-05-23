#include <stdlib.h>
typedef struct _lable *LabelNode;
typedef struct _input *Input;


#define PROCESSING 1
#define MULTIPLY 2
#define TRANSFER 3
#define BRANCH 4
#define SPECIAL 5

typedef struct _label {
  char *l;
  int p;
  LabelNode next;
};

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

LabelNode createTable(FILE *fr) {
  LabelNode head;
  char *next;
  int position = 0;
  while (fgets(next, 511, fr)) {
    char *current = strtok(next,":");
    if(current != next) {
      if(head == NULL) {
        head = malloc(sizeof (LabelNode))
        head->l = current;
        head->p = position;
      } else {
        push(head, current, position);
      }
    }
    position += 4;
  }
  return head;
}

int identify(char *instruction) {
  if ((strcmp(instruction,"add") == 0) || (strcmp(instruction,"sub") == 0)
|| (strcmp(instruction,"rsb") == 0) || (strcmp(instruction,"and") == 0)
|| (strcmp(instruction,"eor") == 0) || (strcmp(instruction,"orr") == 0)
|| (strcmp(instruction,"mov") == 0) || (strcmp(instruction,"tst") == 0)
|| (strcmp(instruction,"teq") == 0) || (strcmp(instruction,"cmp") == 0) ) {
    return PROCESSING;
  } else if ((strcmp(instruction,"mul") == 0) || (strcmp(instruction,"mla") == 0)) {
    return MULTIPLY;
  } else if ((strcmp(instruction,"ldr") == 0) || (strcmp(instruction,"str") == 0)) {
    return TRANSFER;
  } else if ((strcmp(instruction,"lsl") == 0) || (strcmp(instruction,"andeq") == 0)) {
    return SPECIAL;
  } else {
    return BRANCH;
  }
}




int main(int argc, char **argv) {
  //open binary file to write

  uint32_t memory[2048];
  for (int i = 0; i < 2048; i++) {
      memory[i] = 0;
  }

  FILE *fr;
  fr = fopen(argv[0], "r");//open string file

  LabelNode head = createTable(fr);//list of label with position


  FILE *fw;
  fw = fopen(argv[1], "wb");
  fwrite(memory, 4, sizeof (memory), fp);// binary writer
}

