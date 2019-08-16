#include "first_pass.h"


int has_ext_in_symbol_table(symbol_table table[]) {
  int i;
  symbol_table curr;
  for (i = 0; i < HASHSIZE; i++) {
    curr = table[i];
    while (curr.next != NULL) {
      if (curr.type == DOT_EXT) return 1;
      curr = *curr.next;
    }
  }
  return 0;
}

void make_ext_file(char *name, symbol_table table[]) {
  int i;
  symbol_table curr;
  char file_name[MAX_STRING_LEN];
  FILE *f;

  strcpy(file_name, name);
  strcpy(file_name, ".ext"); /* add a .ext to end of file name */
  f = fopen(file_name, "w");
  if (has_ext_in_symbol_table(table)) { /* case when we need to build a file */
    for (i = 0; i < HASHSIZE; i++) {
      curr = table[i];
      while (curr.next != NULL) {
        if (curr.type == DOT_EXT) {
          fprintf(f, curr.name);
          fprintf(f, " ");
          if (curr.value < 1000) { /* print an extra 0 so it's in the same pattern as other lines */
            fprintf(f, "0");
          }
          fprintf(f, "%d", curr.value);
        }
        curr = *curr.next;
      }
  }
}
}

int has_ent_in_symbol_table(symbol_table table[]) {
  int i;
  symbol_table curr;

  for (i = 0; i < HASHSIZE; i++) {
    curr = table[i];
    while (curr.next != NULL) {
      if (curr.type == DOT_ENTRY) return 1;
      curr = *curr.next;
    }
  }
  return 0;
}

void make_ent_file(char *name, symbol_table table[]) {
  int i;
  symbol_table curr;
  char file_name[MAX_STRING_LEN];
  FILE *f;

  strcpy(file_name, name);
  strcpy(file_name, ".ent"); /* add a .ext to end of file name */
  f = fopen(file_name, "w");
  if (has_ext_in_symbol_table(table)) { /* case when we need to build a file */
    for (i = 0; i < HASHSIZE; i++) {
      curr = table[i];
      while (curr.next != NULL) {
        if (curr.type == DOT_EXT) {
          fprintf(f, curr.name);
          fprintf(f, " ");
          if (curr.value < 1000) { /* print an extra 0 so it's in the same pattern as other lines */
            fprintf(f, "0");
          }
          fprintf(f, "%d", curr.value);
        }
        curr = *curr.next;
      }
  }
}
}

/*
converts given number to out special base 4
*/
char get_special_base_4(int pair_value) {
  if (pair_value == 0) return '*';
  if (pair_value == 1) return '#';
  if (pair_value == 2) return '%';
  return '!';
}

/*
writes an operand to the file (only the data part)

Arguments:
  operand_code - code cell where the operand is encoded
  f - file pointer
*/
void write_operand(code_memory *operand_code, FILE *f) {
  int i, operand, pair_value;
  char special_base_4;

  operand = operand_code->operand; /* get the coded operand */
  for (i = 5; i >= 0; i--) { /* there are twelve bits in the data section of the operand so there are six pairs */
    if ((operand >> (2*i + 1)) & 01) pair_value += 2; /* we are going over the pairs from last to first, we first check the leftmost bit in the pair, this bit's value is 2 so we add 2 if it's on */
    if ((operand >> (2*i)) & 01) pair_value += 1;

    special_base_4 = get_special_base_4(pair_value);
    fprintf(f, "%c", special_base_4);
  }
}

/*
writes the opcode of a command into the object file

Arguments:
  command_code - pointer to where the command is encoded
  f - pointer to the file
*/
void write_opcode(code_memory *command_code, FILE *f) {
  int i, command, pair_value;
  char special_base_4;

  command = command_code->opcode; /* get the coded operand */
  for (i = 1; i >= 0; i--) { /* there are twelve bits in the data section of the operand so there are six pairs */
    if ((command >> (2*i + 1)) & 01) pair_value += 2; /* we are going over the pairs from last to first, we first check the leftmost bit in the pair, this bit's value is 2 so we add 2 if it's on */
    if ((command >> (2*i)) & 01) pair_value += 1;

    special_base_4 = get_special_base_4(pair_value);
    fprintf(f, "%c", special_base_4);
  }
}

void make_ob_file(char *name, code_memory memory[], int ic) {
  int i;
  FILE *f;
  char file_name[MAX_STRING_LEN];

  strcpy(file_name, name);
  strcpy(file_name, ".ob");
  f = fopen(file_name, "w");
  for (i = 0; i < ic; i++) {
    if (memory[i].op_or_command) { /* case of an operand */
      if (ic < 1000) { /* print an extra 0 to address (at start) */
        fprintf(f, "0");
      }
      fprintf(f, "%d ", ic); /* print address and space */
      write_operand(&memory[i], f); /* write data part of operand to file */
      fprintf(f, "%c\n", get_special_base_4(memory[i].ARE)); /* write ARE part of operand to file */
    }

    else { /* case of command */
      if (ic < 1000) { { /* print an extra 0 to address (at start) */
        fprintf(f, "0");
      }
      fprintf(f, "%d ", ic); /* print address */
      fprintf(f, "**"); /* write unused bits to file */
      write_opcode(&memory[i], f);
      fprintf(f, "%c\n", get_special_base_4(memory[i].sourceop)); /* write source operand addressing type to file */
      fprintf(f, "%c\n", get_special_base_4(memory[i].destop)); /* write destination operand addresing type to file */
      fprintf(f, "%c\n", get_special_base_4(memory[i].ARE)); /* write ARE part of operand to file */
    }
  }
}
}
