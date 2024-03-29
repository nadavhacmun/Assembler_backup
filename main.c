#include "second_pass.h"

#define DATA_SIZE 4096
#define CODE_SIZE 4096

int main(int argc, char *argv[]) {
  int i, val, ic = 0, dc = 0, is_init[HASHSIZE];
  symbol_table table[HASHSIZE];
  PSW psw;
  data_memory data[DATA_SIZE];
  code_memory code[CODE_SIZE];
  FILE *f;
  char arr[MAX_STRING_LEN], *temp1, *temp2;


  for (i = 0; i < HASHSIZE; i++) {
    is_init[i] = 0;
  }
  if (argc >= 2) {
    for (i = 1; i < argc; i++) {
      temp1 = argv[i];
      temp2 = arr;
      while (*temp1 != '.') *temp2++ = *temp1++; /* copy name of the current file without the extension */
      *temp2 = '\0';
      f = fopen(argv[i], "r");
      val = first_pass(f , table, &psw, data, code, is_init, &ic, &dc);
      if (val == -1) continue; /* case of an error in the first pass */
      fseek(f, 0, SEEK_SET);
      val = second_pass(f, table, code, &psw, is_init); /* val returns the final ic if no error was found and -1 in casw of an error */
      if (val == -1) continue; /* case of an error in the second pass */
      /* make all the files */
      fseek(f, 0, SEEK_SET);
      make_ext_file(arr, code, ic, table, is_init);
      make_ent_file(arr, table, is_init);
      make_ob_file(arr, code, data, ic, dc);
    }
    fclose(f);
  }
  return 0;
}
