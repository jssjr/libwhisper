GIT_SHA = $(shell git rev-parse --short HEAD)
LIBS =
CC = gcc
CFLAGS = -g -Wall -O3 -Wno-strict-aliasing -Isrc -DGIT_SHA=\"$(GIT_SHA)\"

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  CFLAGS += -D_DARWIN_SOURCE
endif

BIN_SRC = $(wildcard *.c)
BIN_OBJ = $(patsubst %.c, %.o, $(BIN_SRC))
SHELL_SRC = $(wildcard *.sh)

PROGRAMS = $(patsubst %.c, %, $(BIN_SRC))
SCRIPTS = $(patsubst %.sh, %, $(SHELL_SRC))

.PHONY: default all clean

default: $(PROGRAMS) $(SCRIPTS)
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

$(SCRIPTS):
	cp $@.sh $@
	chmod 0755 $@

$(PROGRAMS): $(OBJECTS) $(BIN_OBJ)
	$(CC) -flto $@.o $(OBJECTS) $(LIBS) -o $@.new
	mv $@.new $@

whisper_test: $(OBJECTS) $(TEST_OBJ)
	$(CC) $(OBJECTS) $(TEST_OBJ) $(LIBS) -o $@

test: whisper_test
	./whisper_test

test: all

clean:
	-rm -f $(OBJECTS)
	-rm -f $(BIN_OBJ)
	-rm -f $(TEST_OBJ)
	-rm -f $(PROGRAMS) whisper_test
