#include "table.h"

#define LINE_TOO_LONG -2 /* read_line returns -2 if the line is too long */
#define STARTING_INDEX_CODE 100 /* this is where we start storing information in memory */
#define INSTANT 0 /* instant addressing */
#define DIRECT 1 /* direct addressing */
#define CONST_IDX 2 /* constant inex addressing */
#define REGISTER 3 /* register addressing */





/*
defining PSW
*/
typedef struct {
  unsigned int LABEL_DEFINITION: 1;
  unsigned int HAS_ERROR: 1;
} PSW;

/*
Unit of memory in the code section
it can either function as a space to encode a command line or it can function to encode an operand of a command.
is_command tells is 1 if a command is encoded in the cell and 0 if it's an operand.

Bit structure for command:
  0 - 1 (ARE): external, relocateable or absolute
  2 - 3 (dest_op) : addressing type of the destination operand
  4 - 5 (source_op) : addressing type of the source operand
  6 - 9 (opcode) : number representing the command
  10 - 13 (unused) : unused bits

Bit structure for operand:
  0 - 1 (ARE): external, relocateable or absolute
  2 - 13 (operand) : encoding the data of the operand

*/
typedef struct {
  unsigned int unused: 4;
  unsigned int opcode: 4;
  unsigned int source_op: 2;
  unsigned int dest_op: 2;
  unsigned int ARE: 2;
  unsigned int is_command: 1;
  unsigned int operand: 12;
} code_memory;

/*
Unit of memory in the data section
it's simply 14 bits of storage
*/
typedef struct {
  unsigned int data: 14;
} data_memory;

/*
checks if given line is a macro

Arguments:
  line - pointer to the line

Returns:
  0 - it is not a macro
  1 - it is a macro
*/
int is_macro(char *line) {
  char *temp;
  char is_macro[] = ".define"; /* this is how a macro definition line starts */

  line = skip_white_space(line); /* skip white space in start of line */
  temp = is_macro; /* set temp to point to the start of ".define" */
  while (!isspace(*line)) { /* while we the word isn't over yet */
    if (*line++ != *temp++) return 0; /* if the chars don't match it's not a macro */
  }

  return 1; /* if it is a macro */
}

/*
gets the macro and it's value from the given line

Arguments:
  line - pointer to the line
  name - where the name will be stored
  val - where the value will be stored

Returns:
  pointer to the line after the value of the macro
*/
char *get_macro(char *line, char *name, char *val) {
  line = skip_white_space(line); /* skip white space at the start of the line */
  while(*line == '.' || isalpha(*line)) line++; /* skip the .define part of the line */

  line = skip_white_space(line); /* skip white space between ".define" and the name of the macro */
  while(isalpha(*line)) *name++ = *line++; /* copy the name of the macro */
  *name = '\0'; /* add the '\0' to signal the end of the string */

  line = skip_white_space(line); /* skip white space between name of macro to '=' */
  line++; /* skip the '=' */
  line = skip_white_space(line); /* skip white space between '=' and value of macro */
  while(isalnum(*line)) *val++ = *line++; /* copy value */
  *val = '\0'; /* signal end of string */

  return line;
}

/*
checks if the line has a label attched to it

Arguments:
  line - pointer to the line

Returns:
  1 - if line has a label
  0 - if line has no label
*/
int has_label(char *line) {
  line = skip_white_space(line); /* skip white space at the start of the line */
  while(isalnum(*line)) line++; /* we don't care what the label is yet we are just checking if it exists */
  if (*line == ':') return 1; /* a label looks like "string:" where string is any string, we are cheking for the ':' after skipping the string, if it has a ':' it's a label so we return 1 */

  return 0; /* no label */
}

/*
gets the label from a line

Arguments:
  line - pointer to the line
  label - where the label will be stored

Returns:
  pointer to line after label (also after the ':' the is right after the label)
*/
char *get_label(char *line, char *label) {
  line = skip_white_space(line); /* skip white space at start of line */
  while(isalnum(*line)) *label++ = *line++; /* copy label */
  *line = '\0'; /* signal end of string */

  return ++line; /* skip the ':' */
}

/*
checks if the line is a ".string" line

Arguments:
  line - pointer to the line

Returns:
  1 - if it is a '.string' line
  0 - if it is not a '.string' line
*/
int is_dot_string(char *line) {
  char *temp; /* temp will be used to point to the helper strings */
  char dot_string[] = ".string"; /* pattern for a .string line */

  line = skip_white_space(line); /* skip white space before ".string" */

  temp = dot_string; /* temp now points to start of dot_string */
  while(!isspace(*line)) { /* while the current string in the line is not over */
    if(*line++ != *temp++) return 0; /* if it's different from the pattern it's not a '.string' line */
  }
  return 1; /* it is a '.string' line */
}

/*
copies the string from the line into string

Argumenst:
  line - pointer t the line
  data - data memory section of the program
  dc - pointer to data counter
*/
char *get_string_data(char *line, data_memory data[], int *dc) {
  int last_qoute, i = 0;
  char *line_ptr;
  line = skip_white_space(line); /* skip white space before '.string' part of line */
  while (*line == '.' || isalpha(*line)) line++; /* skip '.string' part of the line */
  line = skip_white_space(line); /* skip white space between '.string' and argument */
  if (*line != '\"') return NULL; /* if we are missing an opening " return NULL */
  line++; /* skip the " */
  line_ptr = line;
  while (*line_ptr != '\0') { /* find last qoute of line */
    if (*line_ptr == '\"') last_qoute = i; /* if we found another qoute we set it to be the new last one */
    i++; /* increment i (i just counts how many chars have passed) */
    line_ptr++; /* get to the next char in line */
  }

  while (last_qoute > 0) { /* copy string */
    data[(*dc)++].data = *line++;
    last_qoute--;
  }
  data[*dc++].data = '\0'; /* signal end of string */

  return ++line; /* skip the last qoute */
}

/*
checks if the line is a ".data" line

Arguments:
  line - pointer to the line

Returns:
  1 - if it is a '.data' line
  0 - if it is not a '.data' line
*/
int is_dot_data(char *line) {
  char *temp; /* temp will be used to point to the helper strings */
  char dot_data[] = ".data"; /* pattern for a .data line */

  line = skip_white_space(line); /* skip white space before ".data" */

  temp = dot_data; /* temp now points to start of dot_data */
  while(!isspace(*line)) { /* while the current string in the line is not over */
    if(*line++ != *temp++) return 0; /* if it's different from the pattern it's not a '.data' line */
  }
  return 1; /* it is a '.data' line */
}

/*
get 2's complement negative of the number

Arguments:
  num - number to get negative of

Returns:
  2's complement negative of num
*/
int get_negative(int num) {
  return (~num) + 1;
}

/*
gets the numbers from a '.data' line

Argumenst:
  line - pointer t the line
  data - the data memory section of the program
  dc - data counter
  table - the symbol table
*/
char *get_number_data(char *line, data_memory data[], int *dc, symbol_table table[]) {
  int need_comma = 0, sign, num;
  char number_text[MAX_STRING_LEN], *temp;
  symbol_table *node; /* will be used to lookup macro in the symbol table */

  line = skip_white_space(line); /* skip white space before '.data' */
  while (isalpha(*line) || *line == '.') line++; /* skip the '.data' part of the line */

  while (*line != '\0') {
    temp = number_text;
    sign = 1;
    line = skip_white_space(line);
    if (*line == '\0') break; /* if we reached the end of the line break out of loop */
    if (need_comma) {
      if(*line != ',') return NULL; /* case of a missing comma, return NULL to signify error */
      line++; /* skip comma */
      line = skip_white_space(line); /* skip white space to next element */
    }
    if (*line == '-') { /* negative number */
      sign = -1;
      line++;
    }
    if (*line == '+') { /* positive number */
      sign = 1;
      line++;
    }
    if (isalpha(*line)) { /* case of a macro */
      while(isalpha(*line)) {
        *temp++ = *line++; /* copy name of macro */
      }
      *temp = '\0'; /* signal end of string */
      node = lookup(temp, table);
      if (node == NULL) return NULL; /* macro not found */
      if (node->type != DOT_MACRO) return NULL; /* not a macro */
      data[*dc++].data = node->value; /* copy the value of the macro into data */
    }
    else { /* literal number */
      while(isdigit(*line)) {
        *temp++ = *line++; /* copy number */
      }
      *temp = '\0'; /* signal end of string */
      num = atoi(number_text);
      if (num == 0 && number_text[0] != '0') return NULL; /* case of atoi failing, note that we need to make sure that the value of the string isn't truly 0 */
      if (sign == -1) {
        data[*dc++].data = get_negative(num); /* get 2's complement negative since it's not surely the same as computer negative */
      }
      else { /* sign == 1 case */
        data[*dc++].data = num;
      }
    }
  }
  return line;
}

/*
checks if the line is a .extern line

Argumenst:
  line - pointer to the line

Returns:
  1 - if it is a .extern line
  0 - if it isn't a .extern line
*/
int is_extern(char *line) {
  char pattern[] = ".extern"; /* pattern for an external line */
  char *temp;

  line = skip_white_space(line);
  temp = pattern; /* temp now points to pattern */
  while(!isspace(*line)) { /* while the current word isn't over */
    if (*line++ != *temp++) return 0; /* if they don't match return 0 */
  }
  return 1; /* they match */
}

/*
gets the argument to a .extern line

Arguments:
  line - pointer to the line
  arg - pointer to where the argument will be stored

Returns:
  pointer to the line after the argument
*/
char *get_extern_arg(char *line, char *arg) {
  line = skip_white_space(line); /* skip white space before .extern part of the line */
  while (*line == '.' || isalpha(*line)) line++; /* skip the .extern part */
  line = skip_white_space(line); /* skip white space between .extern part the argument */

  while(isalpha(*line)) { /* while the argument isn't over */
    *arg++ = *line++; /* copy current letter and increment */
  }
  *arg = '\0'; /* end of string */

  return line;
}

/*
checks if the line is a .entry line

Argumenst:
  line - pointer to the line

Returns:
  1 - if it is a .entry line
  0 - if it isn't a .entry line
*/
int is_entry(char *line) {
  char *temp;
  char pattern[] = ".entry"; /* pattern for an entry line */

  line = skip_white_space(line);
  temp = pattern; /* temp now points to pattern */
  while(!isspace(*line)) { /* while the current word isn't over */
    if (*line++ != *temp++) return 0; /* if they don't match return 0 */
  }
  return 1; /* they match */
}

/*
gets the command of the line

Arguments:
  line - pointer to the line
  command - pointer to where the command will be copied

Returns:
  pointer to the line after the command
*/
char *get_command(char *line, char *command) {
  line = skip_white_space(line); /* skip white space before name of command */

  while(isalpha(*line)) { /* while the command isn't over */
    *command++ = *line++; /* copy current letter of command */
  }
  *command = '\0';

  return line;
}

/*
checks if the command is a valid one

Arguments:
  command - pointer to the command

Returns:
  -1 - if it's not valid
  opcode of command if it is valid (opcode is described in the instructions)
*/
int is_valid_command(char *command) {
  if(strcmp(command, "mov") == 0) return 0;
  else if(strcmp(command, "cmp") == 0) return 1;
  else if(strcmp(command, "add") == 0) return 2;
  else if(strcmp(command, "sub") == 0) return 3;
  else if(strcmp(command, "not") == 0) return 4;
  else if(strcmp(command, "clr") == 0) return 5;
  else if(strcmp(command, "lea") == 0) return 6;
  else if(strcmp(command, "inc") == 0) return 7;
  else if(strcmp(command, "dec") == 0) return 8;
  else if(strcmp(command, "jmp") == 0) return 9;
  else if(strcmp(command, "bne") == 0) return 10;
  else if(strcmp(command, "red") == 0) return 11;
  else if(strcmp(command, "prn") == 0) return 12;
  else if(strcmp(command, "jsr") == 0) return 13;
  else if(strcmp(command, "rts") == 0) return 14;
  else if(strcmp(command, "stop") == 0) return 15;
  return -1;
}

/*
checks if the addressing type of the argument is instant addressing

Arguments:
  arg - pointer to the arg
  table - the symbol table

Returns:
  1 - type is instant addressing
  1 - type isn't instant addressing
*/
int is_instant(char *arg, symbol_table table[]) {
  symbol_table *node;

  if (*arg == '#') return 1; /* instant addressing starts like this or is a macro */
  if ((node = lookup(arg, table)) != NULL) { /* if node isn't null check it's a macro */
    if (node->type == DOT_MACRO) return 1; /* if it's a macro */
  }
  return 0;
}

/*
checks if the addressing type of the argument is direct addressing

Arguments:
  arg - pointer to the arg
  table - the symbol table

Returns:
  1 - type is direct addressing
  1 - type isn't direct addressing
*/
int is_direct(char *arg, symbol_table table[]) {
  symbol_table *node;

  if ((node = lookup(arg, table)) != NULL) { /* if node isn't null check it's a data or extern */
    if (node->type == DOT_DATA || node->type == DOT_EXT) return 1; /* if it's a macro */
  }
  return 0;
}

/*
checks if the addressing type is constant index addressing

Arguments:
  arg - pointer to the argument

Returns:
  1 - if addressing type is constant index addressing
  0 - if addressing type isn't constant index addressing
*/
int is_const_idx(char *arg) {
  while(*arg != '\0') { /* while the argument isn't over */
    if (*arg == '[') return 1; /* if we find a bracket we have this type of indexing */
    arg++;
  }

  return 0;
}

/*
checks if the addressing type of the argument is register addressing

Arguments:
  arg - pointer to the argument

Returns:
  1 - if the addressing type is register addressing
  0 - if the addressing type isn't register addressing
*/
int is_register(char *arg) {
  if(*arg == 'r') { /* a register var is of the form 'rx' where x is a digit, we check for this pattern */
    arg++;
    if (isdigit(*arg)) {
      arg++;
      if (*arg == '\0') return 1; /* matching pattern */
    }
  }
  return 0; /* pattern not matching */
}

/*

*/
char *get_operands(char *line, int num_args, char *op1, char *op2) {
  if (num_args == 0);
  if (num_args == 1) {
    line = skip_white_space(line);
    while(isalpha(*line)) {
      *op1++ = *line++; /* copy argument */
    }
    *op1 = '\0';
  }
  if (num_args == 2) {
    line = skip_white_space(line);
    while(isalpha(*line)) {
      *op1++ = *line++; /* copy argument */
    }
    *op1 = '\0';
    line = skip_white_space(line); /* skip white space before ',' */
    ++line; /* skip ',' */
    line = skip_white_space(line); /* skip white space between ',' and argument */
    while(isalpha(*line)) {
      *op2++ = *line++; /* copy argument */
    }
    *op2 = '\0';
  }

  return line;
}

/*
returns the type of the operand

Arguments:
  operand - pointer to the operand
  table - symbol table of the program

Returns:
  REGISTER - when operand's addressing type is register addressing
  DIRECT - when operand's addressing type is direct addressing
  CONST_IDX - when operand's addressing type is constant index addressing
  INSTANT - when operand's addressing type is instant addressing
*/
int get_operand_type(char *operand, symbol_table table[]) {
  if(is_register(operand) == 1) return REGISTER;
  if(is_direct(operand, table) == 1) return DIRECT;
  if(is_const_idx(operand) == 1) return CONST_IDX;
  return INSTANT;
}

/*
returns the amount of cells required to encode the operand
*/
int get_operand_extra_cells(int operand) {
  if (operand == REGISTER) return 1;
  if (operand == DIRECT) return 1;
  if (operand == CONST_IDX) return 2;
  return 1; /* INSTANT addressing */
}

/*

*/
void build_binary_code(code_memory code[], int *ic, int opcode, int num_args, char *op1, char *op2, symbol_table table[]) {
  int L = 0;
  int type1, type2;

  if (num_args == 0) {
    code[*ic].is_command = 1;
    code[*ic].opcode = opcode;
    L++; /* add one cell to L since we are only coding the command */
  }
  else if (num_args == 1) { /* if the function only has a single argument */
    type1 = get_operand_type(op1, table); /* type1 now has the value of the addressing type */
    code[*ic].source_op = type1;
    L += get_operand_extra_cells(type1); /* returns amount of cells to code operand */
  }
  else {
    type1 = get_operand_type(op1, table);
    type2 = get_operand_type(op2, table);
    if (type1 == REGISTER && type2 == REGISTER) L++; /* special case when both operands are register */
    else {
      L += get_operand_extra_cells(type1); /* amount of cells needed to code source operand */
      L += get_operand_extra_cells(type2); /* amount of cells needed to code destination operand */
      code[*ic].source_op = type1; /* set value of souce operand */
      code[*ic].dest_op = type2; /* set value of destination operand */
    }
  }
  *ic += L; /* increment ic */
}

/*
function implementing the first pass of the assembler

Arguments:
  f - pointer to the file
  table - symbol table of the program
  psw - pointer to psw
  data - the data memory section of the program
  code - the code memory section of the program
  is_init - an array the same size as hashtab, the ith element is 1 if hashtab[i] is initialized and is 0 otherwise
  ic - pointer to ic
  dc - pointer to dc
*/
int first_pass(FILE *f, symbol_table table[], PSW *psw, data_memory data[], code_memory code[], int is_init[], int *ic, int *dc) {
  char line_arr[MAX_LINE_LEN], *line, string1[MAX_STRING_LEN], string2[MAX_STRING_LEN], string3[MAX_STRING_LEN], label[MAX_STRING_LEN]; /* string1 string2 string3 will be used to store temporary string data */
  int curr_line = 0, val1, val2;


  while ((val1 = read_line(f, line_arr)) != EOF) { /* while file isn't over */
    line = line_arr; /* line now points to where the line will be stored */
    ++curr_line; /* increment line count */
    psw->LABEL_DEFINITION = 0;
    if (*line == ';') continue; /* comment line */
    if (val1 == LINE_TOO_LONG) { /* if the line is too long to be read */
      psw->HAS_ERROR = 1; /* notify we have error */
      printf("Error: line too long, Line number: %d\n", curr_line); /* print error */
      continue; /* continue to next line since this one couldn't be read */
    }

    if(is_macro(line)) { /* case of a macro */
      line = get_macro(line, string1, string2); /* string1 now stores the name of the macro while string2 now stored it's value */
      val1 = atoi(string2); /* convert value to integer from string */
      if (lookup(string1, table) == NULL) { /* if not found */
        install(string1, val1, DOT_MACRO, table, is_init);
      }
      else {
        psw->HAS_ERROR = 1; /* if the name of the macro is already in the table we have an error */
        printf("Error: macro name already in table, Line number: %d\n", curr_line); /* print error */
      }
      continue; /* go to the next line */
    }

    if(has_label(line)) {
      line = get_label(line, label); /* get the label */
      psw->LABEL_DEFINITION = 1; /* notify psw of a label definition */
    }

    if (is_dot_string(line)) {
      if (psw->LABEL_DEFINITION) { /* if we have a label definition */
        if (lookup(label, table) != NULL) { /* if the label is already in the table */
          psw->HAS_ERROR = 1; /* turn on error */
          printf("Error: label already defined, \"%s\", Line number: %d\n", label, curr_line); /* print error */
        }
        else {
          install(label, *dc, DOT_DATA, table, is_init); /* install node in table */
        }
      }
      line = get_string_data(line, data, dc);
      continue; /* go to the next line */
    }

    if (is_dot_data(line)) {
      if (psw->LABEL_DEFINITION) { /* if we have a label definition */
        if (lookup(label, table) != NULL) { /* if the label is already in the table */
          psw->HAS_ERROR = 1; /* turn on error */
          printf("Error: label already defined, \"%s\", Line number: %d\n", label, curr_line); /* print error */
        }
        else {
          install(label, *dc, DOT_DATA, table, is_init); /* install node in table */
        }
    }
    line = get_number_data(line, data, dc, table); /* get all numbers and store them in the data section */
    continue; /* go to the next line */
  }

  if (is_entry(line)) {
    continue; /* we deal with entry lines in the second pass */
  }

  if (is_extern(line)) {
    line = get_extern_arg(line, string1); /* get the argument */
    install(string1, 0, DOT_EXT, table, is_init); /* insert it into the table with no value and .ext as type*/
    continue; /* go to next line */
  }
  /* the only remaining case if of a command, so we are dealing with a command */
  if (psw->LABEL_DEFINITION) { /* if we have a label */
    if (lookup(label, table) != NULL) { /* if the label is already in the table */
      psw->HAS_ERROR = 1;
      printf("Error: Label \"%s\" already defined, Line number: %d\n", label, curr_line);
    }
    else {
      install(label, *ic + STARTING_INDEX_CODE, DOT_CODE, table, is_init);
    }
  }
  line = get_command(line, string1);
  if ((val1 = is_valid_command(string1)) == -1) {
    psw->HAS_ERROR = 1;
    printf("Error: invalid command name, Line number: %d\n", curr_line);
    continue; /* can't do anything more on this line so we go to the next one */
  }
  val2 = get_number_args(string1);
  get_operands(line, val2, string2, string3);
  build_binary_code(code, ic, val1, val2, string2, string3, table);
  }
  if (psw->HAS_ERROR) {
    return -1; /* signal first pass had an error */
  }
  correct_data_index(*ic + STARTING_INDEX_CODE, table, is_init);
  return 0; /* succsess */
}
