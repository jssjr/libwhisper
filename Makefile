all:
	gcc -g -o wsptest libwhisper.c

test: all
	valgrind ./wsptest
