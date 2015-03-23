TARGET = all

GIT_SHA = $(shell git rev-parse HEAD)

WHISPER_CFLAGS += -g -Wall -Werror -Wextra -Wno-unused-parameter \
		  -Wno-missing-field-initializers -fPIC -D_GNU_SOURCE \
		  -Wno-deprecated-declarations -std=gnu99 -Os \
		  -DGIT_SHA='"${GIT_SHA}"'
WHISPER_INCLUDES += -Isrc

all: whisper-info

whisper-info:
	$(CC) $(WHISPER_CFLAGS) $(WHISPER_INCLUDES) src/whisper-info.c whisper.c -o bin/$@

#test: all
#	valgrind ./wsptest

clean:
	-rm -rf bin/*
