all:
	gcc -g -Wall -ansi -pedantic -o wsptest libwhisper.c

test: all
	valgrind ./wsptest
