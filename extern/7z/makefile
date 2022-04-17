PROG = lib7z
CC   = gcc -std=c17

SOURCES = $(wildcard *.c)
OBJ := $(SOURCES:%.c=%.o)

all: $(OBJ)

%.o: %.c
	$(CC) -O2 -c -o $@ $<

clean:
	rm -f *.o *.a
