#include "table.h"

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
  while(isalpha(*line)) line++; /* we don't care what the label is yet we are just checking if it exists */
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
  while(isalpha(*line)) *label++ = *line++; /* copy label */
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
  string - where the string will be stored
*/
char *get_string_data(char *line, char *string) {
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
  while (last_qoute != 0) { /* copy string */
    *string++ = *line++;
    last_qoute--;
  }

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
  
}

/*
gets the numbers from a '.data' line

Argumenst:
  line - pointer t the line
  data - the data memory section of the program
  dc - data counter
*/
char *get_number_data(char *line, data_memory data[], int *dc) {
  int need_comma, sign, num;
  char number_text[MAX_STRING_LEN], *temp;

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
  return line;
}
