#include "files.h"

#define ABSOLUTE 0
#define EXTERNAL 1
#define RELOCATEABLE 2

/*
skips label

Arguments:
  line - pointer to the text of the line
*/
char *skip_label(char *line) {
  line = skip_white_space(line); /* skip white space from start to label */
  while(isalnum(*line)) line++; /* skip the label */
  line++; /* skip the ":" */
  return line;
}

/*
get integer value of instant operand

Arguments:
  operand - pointer to the text of the operand
*/
int get_instant_value(char *operand) {
  int sign = 1;
  char num_text[MAX_STRING_LEN], *temp;
  temp = num_text;
  operand = skip_white_space(operand); /* skip white space before start of operand */
  ++operand; /* skip the "#" */
  if (*operand == '+' || *operand == '-') {
    if (*operand == '-') sign = -1;
    ++operand;
  }
  while(isdigit(*operand)) *temp++ = *operand++;
  *temp = '\0';

  return atoi(num_text) * sign;
}

/*
gets the number of the register

Arguments:
  operand - pointer to the text of the operand
*/
int get_register_number(char *operand) {
  operand = skip_white_space(operand); /* skip white space before register */
  operand++; /* skip the 'r' to get to the number */
  return atoi(operand); /* convert number text to int and return it */
}

/*
gets the argument in a .entry line

Arguments:
  line - pointer to the line
  arg - pointer to where the argument will be stored
*/
char *get_entry_arg(char *line, char *arg) {
  line = skip_white_space(line);
  while(isalpha(*line)) {
    *arg++ = *line++;
  }
  *arg = '\0'; /* signal end of string */

  return line;
}

void code_binary_operands(char *operand1, char *operand2,int *ic, int num_operands, int curr_line, symbol_table table[], code_memory code[], PSW *psw) {
  int val1;
  char string1[MAX_STRING_LEN], *temp;
  symbol_table *node;

  if (num_operands == 0); /* no operand's */
  if (num_operands >= 1) { /* 1 operand */
    if (is_register(operand1)) {
      code[*ic].ARE = ABSOLUTE;
      code[*ic++].operand |= (get_register_number(operand1)) << 5; /* we slide by 5 to get to where the source operand num is encoded */
    }
    else if (is_instant(operand1, table)) {
      code[*ic].ARE = ABSOLUTE;
      val1 = get_instant_value(operand1);
      if (val1 >= 0) {
        code[*ic++].operand = val1;
      }
      else { /* in this case we implement a 2's complement negative independent of how the current computer functions */
        val1 = -1*val1;
        code[*ic++].operand = get_negative(val1);
        }
      }
    else if (is_const_idx(operand1)) {
      operand1 = skip_white_space(operand1);
      temp = string1;
      while(*operand1 != '[') *temp++ = *operand1++; /* copy name of the array into string1 */
      node = lookup(string1, table);
      if (node != NULL) {
        if (node->type == DOT_EXT) code[*ic].ARE = EXTERNAL;
        else code[*ic].ARE = RELOCATEABLE;
        code[*ic++].operand = node->value;
      }
      else {
        printf("Error: label undefined, Line: %d\n", curr_line);
        psw->HAS_ERROR = 1;
      }
      ++operand1; /* skip the '[' */
      temp = string1; /* set temp to the start of string1 again */
      if (isdigit(*operand1)) { /* case of a number */
        while(*operand1 != ']') *temp++ = *operand1++; /* copy what's inside the brackets */
        val1 = atoi(string1);
        /* atoi returns 0 upon failue, this if statement checks if atoi doen't fail or if the value of the number truly is 0 to avoid ambiguity */
        if (val1 != 0 || string1[0] == '0') {
          code[*ic].ARE = ABSOLUTE;
          code[*ic++].operand = atoi(string1);
        }
        else {
          printf("Error: Not a number or a macro index, Line: %d\n", curr_line);
          psw->HAS_ERROR = 1;
        }
      }
      else { /* case of a macro */
        while(*operand1 != ']') *temp++ = *operand1++; /* copy what's inside the brackets */
        if ((node = lookup(string1, table)) == NULL) { /* macro not in symbol table */
          printf("Error: macro undefined %s, Line: %d\n", string1, curr_line);
          psw->HAS_ERROR = 1;
        }
        else {
          code[*ic].ARE = ABSOLUTE;
          code[*ic++].operand = node->value;
        }
      }
      }

    else { /* case of direct addressing */
      operand1 = skip_white_space(operand1);
      temp = string1;
      while(isalnum(*operand1)) *temp++ = *operand1++; /* copy name of the array into string1 */
      node = lookup(string1, table);
      if (node != NULL) {
        code[*ic++].operand = node->value;
      }
      else {
        printf("Error: label undefined, Line: %d\n", curr_line);
        psw->HAS_ERROR = 1;
      }
    }
  }
  if(num_operands >= 2) { /* 2 operands, this does exctly the same as done in the previous if to the second operand */
    if (is_register(operand2)) {
      code[*ic].ARE = ABSOLUTE;
      code[*ic++].operand |= (get_register_number(operand2)) << 5; /* we slide by 5 to get to where the source operand num is encoded */
    }
    else if (is_instant(operand2, table)) {
      code[*ic].ARE = ABSOLUTE;
      val1 = get_instant_value(operand1);
      if (val1 >= 0) {
        code[*ic++].operand = val1;
      }
      else { /* in this case we implement a 2's complement negative independent of how the current computer functions */
        val1 = -1*val1;
        code[*ic++].operand = get_negative(val1);
        }
      }
    else if (is_const_idx(operand2)) {
      operand2 = skip_white_space(operand2);
      temp = string1;
      while(*operand2 != '[') *temp++ = *operand2++; /* copy name of the array into string1 */
      node = lookup(string1, table);
      if (node != NULL) {
        if (node->type == DOT_EXT) code[*ic].ARE = EXTERNAL;
        else code[*ic].ARE = RELOCATEABLE;
        code[*ic++].operand = node->value;
      }
      else {
        printf("Error: label undefined, Line: %d\n", curr_line);
        psw->HAS_ERROR = 1;
      }
      ++operand2; /* skip the '[' */
      temp = string1; /* set temp to the start of string1 again */
      if (isdigit(*operand2)) { /* case of a number */
        while(*operand2 != ']') *temp++ = *operand2++; /* copy what's inside the brackets */
        val1 = atoi(string1);
        /* atoi returns 0 upon failue, this if statement checks if atoi doen't fail or if the value of the number truly is 0 to avoid ambiguity */
        if (val1 != 0 || string1[0] == '0') {
          code[*ic].ARE = ABSOLUTE;
          code[*ic++].operand = atoi(string1);
        }
        else {
          printf("Error: Not a number or a macro index, Line: %d\n", curr_line);
          psw->HAS_ERROR = 1;
        }
      }
      else { /* case of a macro */
        while(*operand2 != ']') *temp++ = *operand2++; /* copy what's inside the brackets */
        if ((node = lookup(string1, table)) == NULL) { /* macro not in symbol table */
          printf("Error: macro undefined %s, Line: %d\n", string1, curr_line);
          psw->HAS_ERROR = 1;
        }
        else {
          code[*ic].ARE = ABSOLUTE;
          code[*ic++].operand = node->value;
        }
      }
      }

    else { /* case of direct addressing */
      operand1 = skip_white_space(operand1);
      temp = string1;
      while(isalnum(*operand1)) *temp++ = *operand1++; /* copy name of the array into string1 */
      node = lookup(string1, table);
      if (node != NULL) {
        code[*ic++].operand = node->value;
      }
      else {
        printf("Error: label undefined, Line: %d\n", curr_line);
        psw->HAS_ERROR = 1;
      }
    }
  }
}

int second_pass(FILE *file, symbol_table table[], code_memory code[], PSW *psw) {
  int ic = 0, curr_line = 0, val1;
  char line_text[MAX_STRING_LEN], string1[MAX_STRING_LEN], string2[MAX_STRING_LEN], string3[MAX_STRING_LEN], *line;
  symbol_table *node;

  line = line_text;
  while (read_line(file, line) != EOF) { /* while the file isn't over */
    ++curr_line;
    if (has_label(line)) { /* if the line has a label skip it */
      line = skip_white_space(line); /* skip white space before label */
      while(isalnum(*line)) line++; /* skip label */
      line++; /* skip the ":" */
    }
    if ((is_dot_data(line) || is_dot_string(line) || is_extern(line))) continue;
    if (is_entry(line)) {
      line = get_entry_arg(line, string1);
      node = lookup(string1, table);
      if (node != NULL) node->type = DOT_ENTRY;
      else {
        printf("Error: label undefined %s. Line: %d\n", string1, curr_line);
        psw->HAS_ERROR = 1;
      }
      continue;
    }
    /* only case remaining is of a command */

    line = skip_white_space(line); /* skip white space before command */
    line = get_command(line, string1); /* string1 now stores the name of the command */
    val1 = get_number_args(string1); /* val1 now stores the number of operands the command takes as input */
    line = get_operands(line, val1, string2, string3); /* string2 and string3 now contain the operands of the command */

    ++ic; /* skip the cell the command is encoded in */
    code_binary_operands(string2, string3, &ic, val1, curr_line, table, code, psw);
    }
    if (psw->HAS_ERROR == 1) return -1;

    return ic;
}
