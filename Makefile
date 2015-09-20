GIT_SHA = $(shell git rev-parse --short HEAD)
LIBS =
CC = gcc
CFLAGS = -g -Wall -O3 -Wno-strict-aliasing -Isrc -DGIT_SHA=\"$(GIT_SHA)\"

BIN_SRC = $(wildcard *.c)
BIN_OBJ = $(patsubst %.c, %.o, $(BIN_SRC))
PROGRAMS = $(patsubst %.c, %, $(BIN_SRC))

.PHONY: default all clean

default: $(PROGRAMS)
all: default

SOURCES = \
	src/libwhisper.c

OBJECTS = $(patsubst %.c, %.o, $(SOURCES))
HEADERS = $(wildcard src/*.h)

TEST_SRC = $(wildcard tests/*.c)
TEST_OBJ = $(patsubst %.c, %.o, $(TEST_SRC))

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(PROGRAMS): $(OBJECTS) $(BIN_OBJ)
	mkdir -p bin/
	$(CC) -flto $@.o $(OBJECTS) $(LIBS) -o $@.new
	mv $@.new bin/$@

whisper_test: $(OBJECTS) $(TEST_OBJ)
	$(CC) $(OBJECTS) $(TEST_OBJ) $(LIBS) -o $@

test: whisper_test
	./whisper_test
	#valgrind ./whisper_test

test: all

clean:
	-rm -f $(OBJECTS)
	-rm -f $(BIN_OBJ)
	-rm -f $(TEST_OBJ)
	-rm -f $(PROGRAMS) whisper_test
