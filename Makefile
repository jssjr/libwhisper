all:
	gcc -g -Wall -ansi -pedantic -o wsptest whisper.c

test: all
	valgrind ./wsptest
