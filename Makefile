strings: main.c
	gcc -Wall -ansi -pedantic main.c

first: first_pass.h
	gcc -Wall -ansi -pedantic first_pass.h
