PROG = lib7z
CC   = gcc -std=c17 -O2 -Wall

SOURCES = $(wildcard *.c)
OBJ := $(SOURCES:%.c=%.o)

all: $(OBJ)
#	ar rcs $(PROG).a $(OBJ)

%.o: %.c
	$(CC) -c -o $@ $<

clean:
	rm -f *.o *.a
