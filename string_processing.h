#include <stdio.h>
#include <string.h>
#include <ctype.h>
#define MAX_MACRO_NAME_LEN 50
#define MAX_LINE_LEN 80 /* max size of line */
#define MAX_STRING_LEN 81 /* max size needed for arrays since array also include "\0" */
#define NO_MORE_SPACE_ERROR -2

/*
This function reads one line from input file, it returns -1 for EOF and 0 for succsess.

Arguments:
	f - FILE pointer, pointer to the file.
	str - char pointer to where you want the line to be copied.

Returns:
	0 - on succsess
	EOF - if nothing is left to read (reached EOF)
	-2 - if the line is longer than max size
*/
char read_line(FILE *f, char *str) {
	char c;
	int space_left = MAX_LINE_LEN;

	if ((c = fgetc(f)) == EOF) return EOF;
	while(c != '\n') {
		if (c == EOF) break;
		if (space_left == 0) return NO_MORE_SPACE_ERROR; /* no space left */
		*str = c;
		str++;
		c = fgetc(f);
		space_left--;
	}
	*str = '\0';
	return 0;
}

/*
skips white spaces in the provided string, returns the pointer after passing all the white space.

Arguments:
	str - the string
*/
char *skip_white_space(char *str) {
		while(isspace(*str)) str++; /* skip white spaces */

		return str;
}

/*
gets the next word in text, assumes white space has been skipped. returns pointer to line after read word.

Arguments:
	line - pointer to the line
	word - pointer to where the word will be copied
*/
char *get_word(char *line, char *word) {
		while(isalnum(*line)) *word++ = *line++;
		*word = '\0';

		return line;
}

/*
my version of strcmp that only checks for equality, normal version overwrote other stuff for some weird reason
*/
int strcmp_(char *s1, char *s2) {
    while(*s1 == *s2) {
        if (*s1 == '\0' || *s2 == '\0') break;
        s1++;
        s2++;
    }
    if (*s1 == '\0' && *s2 == '\0') return 0;

    return -1;
}

/*
takes the name of the command and returns the number of operands it has

Arguments:
	name - the name of the command

Returns:
	the number of operands if command is valid
	-1 if command is invalid
*/
int get_number_args(char *name) {
	if (strcmp_(name, "mov") == 0) return 2;
	else if (strcmp_(name, "cmp") == 0) return 2;
	else if (strcmp_(name, "add") == 0) return 2;
	else if (strcmp_(name, "sub") == 0) return 2;
	else if (strcmp_(name, "lea") == 0) return 2;
	else if (strcmp_(name, "not") == 0) return 1;
	else if (strcmp_(name, "clr") == 0) return 1;
	else if (strcmp_(name, "inc") == 0) return 1;
	else if (strcmp_(name, "dec") == 0) return 1;
	else if (strcmp_(name, "jmp") == 0) return 1;
	else if (strcmp_(name, "bne") == 0) return 1;
	else if (strcmp_(name, "red") == 0) return 1;
	else if (strcmp_(name, "prn") == 0) return 1;
	else if (strcmp_(name, "jsr") == 0) return 1;
	else if (strcmp_(name, "rts") == 0) return 0;
	else if (strcmp_(name, "stop") == 0) return 0;


	return -1; /* if it's not the name of a valid command */
}

/*
 check there are no more char's in the rest of the line, returns 0 if there are none and -1 if there are.

Arguments:
 	line - pointer to the line
*/
int check_end(char *line) {
	while(*line != '\0') {
		if (!isspace(*line)) return -1;
		line++;
	}
	return 0;
}
