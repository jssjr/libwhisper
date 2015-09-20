GIT_SHA = $(shell git rev-parse --short HEAD)
TARGET = whisper
LIBS =
CC = gcc
CFLAGS = -g -Wall -O3 -Wno-strict-aliasing -Isrc -DGIT_SHA=\"$(GIT_SHA)\"

.PHONY: default all clean

default: $(TARGET)
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

$(TARGET): $(OBJECTS) whisper.o
	$(CC) -flto whisper.o $(OBJECTS) $(LIBS) -o $@.new
	mv $@.new $@

$(TARGET)_test: $(OBJECTS) $(TEST_OBJ)
	$(CC) $(OBJECTS) $(TEST_OBJ) $(LIBS) -o $@

test: $(TARGET)_test
	./$(TARGET)_test
	#valgrind ./wsptest

test: all

clean:
	-rm -f $(OBJECTS) whisper.o
	-rm -f $(TEST_OBJ)
	-rm -f $(TARGET) $(TARGET)_test
