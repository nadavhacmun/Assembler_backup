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

void make_ext_file(char *name, code_memory code[], int ic, symbol_table table[], int is_init[]) {
  int i;
  char file_name[MAX_STRING_LEN];
  FILE *f;
  strcpy(file_name, name);
  strcat(file_name, ".ext"); /* add a .ext to end of file name */
  f = fopen(file_name, "w");
  if (has_ext_in_symbol_table(table)) { /* case when we need to build a file */
    for (i = 0; i < ic; i++) {
      if (code[i].ARE == 1) {
        fprintf(f, "%s %d\n", code[i].name, i + STARTING_INDEX_CODE);
      }
      else continue;
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

void make_ent_file(char *name, symbol_table table[], int is_init[]) {
  int i;
  symbol_table *curr;
  char file_name[MAX_STRING_LEN];
  FILE *f;

  strcpy(file_name, name);
  strcat(file_name, ".ent"); /* add a .ent to end of file name */
  f = fopen(file_name, "w");
  if (has_ext_in_symbol_table(table)) { /* case when we need to build a file */
    for (i = 0; i < HASHSIZE; i++) {
      if (is_init[i] == 1) {
        curr = &table[i];
        while (curr != NULL) {
          if (curr->type == DOT_ENTRY) {
            fprintf(f, "%s", curr->name);
            fprintf(f, " ");
            if (curr->value < 1000) { /* print an extra 0 so it's in the same pattern as other lines */
              fprintf(f, "0");
            }
            fprintf(f, "%d\n", curr->value);
          }
          curr = curr->next;
        }
      }
  }
}
fclose(f);
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
  int i, pair_value;
  char special_base_4;

  for (i = 5; i >= 0; i--) { /* there are twelve bits in the data section of the operand so there are six pairs */
    pair_value = 0;
    if ((operand_code->operand >> (2*i + 1)) & 01) pair_value += 2; /* we are going over the pairs from last to first, we first check the leftmost bit in the pair, this bit's value is 2 so we add 2 if it's on */
    if ((operand_code->operand >> (2*i)) & 01) pair_value += 1;

    special_base_4 = get_special_base_4(pair_value);
    fprintf(f, "%c", special_base_4);
  }
}

void write_data(data_memory *data, FILE *f) {
  int i, pair_value;
  char special_base_4;

  for (i = 6; i >= 0; i--) { /* there are twelve bits in the data section of the operand so there are six pairs */
    pair_value = 0;
    if ((data->data >> (2*i + 1)) & 01) pair_value += 2; /* we are going over the pairs from last to first, we first check the leftmost bit in the pair, this bit's value is 2 so we add 2 if it's on */
    if ((data->data >> (2*i)) & 01) pair_value += 1;

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
    pair_value = 0;
    if ((command >> (2*i + 1)) & 01) pair_value += 2; /* we are going over the pairs from last to first, we first check the leftmost bit in the pair, this bit's value is 2 so we add 2 if it's on */
    if ((command >> (2*i)) & 01) pair_value += 1;

    special_base_4 = get_special_base_4(pair_value);
    fprintf(f, "%c", special_base_4);
  }
}

void make_ob_file(char *name, code_memory code[], data_memory data[], int ic, int dc) {
  int i;
  FILE *f;
  char file_name[MAX_STRING_LEN];
  strcpy(file_name, name);
  strcat(file_name, ".ob");
  f = fopen(file_name, "w");
  fprintf(f, "%d\t%d\n", ic, dc); /* print ic and dc to file */
  for (i = 0; i < ic; i++) {
    if (code[i].is_command == 0) { /* case of an operand */
      if (i < 1000) { /* print an extra 0 to address (at start) */
        fprintf(f, "0");
      }
      fprintf(f, "%d ", i + STARTING_INDEX_CODE); /* print address and space */
      write_operand(&code[i], f); /* write data part of operand to file */
      fprintf(f, "%c\n", get_special_base_4(code[i].ARE)); /* write ARE part of operand to file */
    }

    else { /* case of command */
      if (i < 1000) { /* print an extra 0 to address (at start) */
        fprintf(f, "0");
      }
      fprintf(f, "%d ", i + STARTING_INDEX_CODE); /* print address */
      fprintf(f, "**"); /* write unused bits to file */
      write_opcode(&code[i], f);
      fprintf(f, "%c", get_special_base_4(code[i].source_op)); /* write source operand addressing type to file */
      fprintf(f, "%c", get_special_base_4(code[i].dest_op)); /* write destination operand addresing type to file */
      fprintf(f, "%c\n", get_special_base_4(code[i].ARE)); /* write ARE part of operand to file */
  }
}
  for (i = 0; i < dc; i++) {
    if (ic + i < 1000)  { /* if line number is less then 1000 add another 0 to fit the format */
      fprintf(f, "0");
    }
    fprintf(f, "%d ", i + ic + STARTING_INDEX_CODE); /* print address */
    write_data(&data[i], f);
    fprintf(f, "\n");
  }
}
