all:
	gcc -g -Wall -pedantic -o wsptest libwhisper.c

test: all
	valgrind ./wsptest
