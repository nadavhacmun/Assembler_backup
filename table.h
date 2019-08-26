#include <stdio.h>
#include <stdlib.h>
#include "string_processing.h"

#define HASHSIZE 101
#define DOT_DATA 0 /* number representing .data in symbol table */
#define DOT_MACRO 1 /* number representing .macro in symbol table */
#define DOT_EXT 2 /* number representing .external in symbol table */
#define DOT_CODE 3 /* number representing .code in symbol table */
#define DOT_ENTRY 4 /* number representing .entry in symbol table */

/*
This section of the code implements a table data structure.
*/

/*
definition of the a single row in "symbol_table".
in practice, we will be using an array of this.

Contains:
  name - the name of the label
  value - address or if it's a macro it's value
  type - the type of the sentence, 0 = .data, 1 = .macro, 2 = .external, 3 = .code, 4 = .entry
  ext - whether or not it's external.
*/
typedef struct nlist{
  struct nlist *next;
  char name[MAX_MACRO_NAME_LEN];
  int value;
  unsigned int type: 3;

} symbol_table;

/*
init symbol_table node, returns -1 on failure and 0 on succsess

Arguments:
  node - node to init
  name - name of node (label)
  val - address or value (if macro)
  type - type of sentence it's attached to
*/
char init_symbol(symbol_table *node, char *name, int val, unsigned int type, symbol_table *next) {
  if (strcpy(node->name, name) == NULL) return -1;
  node->value = val;
  node->type = type;
  node->next = next;

  return 0;
}

/*
created hash value for string
*/
unsigned hash(char *s) {
  unsigned int hashval;

  for (hashval = 0; *s != '\0'; s++)
    hashval = *s + 31 * hashval;

  return hashval % HASHSIZE;
}

/*
looks up a string in dictionary provided. returns NULL if not found and pointer to node if found.

Arguments:
  s - the string we will be searching for
  hashtab - the dictionary in which to look for s
*/
symbol_table *lookup(char *s, symbol_table hashtab[], int is_init[]) {
  symbol_table *np;
  int h = hash(s);
  if (is_init[h] == 0) return NULL;
  for (np = &hashtab[h]; np != NULL; np = np->next) {
    if (!isalpha(np->name[0])) return NULL;
    printf("%s\n", np->name);
    printf("%s\n", s);
    if (strcmp(s, np->name) == 0) return np; /* if found */
  }

  return NULL; /* if not found*/
}

/*
insetrs a value into the table, if the value is alreadt in the chart, returns NULL
otherwise, it returns the pointer to the created object.

Arguments:
  name - name of label
  value - address  if variable value if macro
  type - type of the sentence in which the label appers
  hashtab - the table to insert into
  is_init - an array the same size as hashtab, the ith element is 1 if hashtab[i] is initialized and is 0 otherwise
*/
symbol_table *install(char *name, int value, unsigned int type, symbol_table hashtab[], int is_init[]) {
  symbol_table *np;
  unsigned int hashval;
  if ((np = lookup(name, hashtab, is_init)) == NULL) {
      np = (symbol_table *) malloc(sizeof(*np));
      if (np == NULL) return NULL; /* if no memory can be allocated */
      hashval = hash(name);
      if (is_init[hashval]) {
        if (init_symbol(np, name, value, type, &hashtab[hashval]) == -1) return NULL;
      }
      else {
        if (init_symbol(np, name, value, type, NULL) == -1) return NULL;
      }
      hashtab[hashval] = *np;

      return np;
  }
  /* if the name is already defined assembler should raise an error so we return null */
  return NULL;
}

/*
updates index of .data entries on the symbol table

Arguments:
  offset - how much to add to current index
  table - symbol table
*/
void correct_data_index(int offset, symbol_table table[], int is_init[]) {
  int i;
  symbol_table curr;
  for (i = 0; i < HASHSIZE; i++) {
    if (is_init[i] == 0) continue;
    curr = table[i];
    while (curr.next != NULL) {
      if (curr.type == DOT_DATA) curr.value += offset;
      printf("TYPE IS: %d\n", (*(curr.next)).type);
      printf("hrllodada");
      curr = *(curr.next);
    }
  }
}
