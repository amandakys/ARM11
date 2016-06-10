CC      = gcc
CFLAGS  = -Wall -Werror -g -D_POSIX_SOURCE -D_BSD_SOURCE -std=c99 -pedantic

.SUFFIXES: .c .o

.PHONY: all clean

all: assemble emulate

assemble: assemble.o

emulate: emulate.c
	$(CC) $(CFLAGS) emulate.c -lm -o emulate

clean:
	rm -f $(wildcard *.o)
	rm -f assemble
	rm -f emulate
