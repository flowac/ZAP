PROG = lib7z
CC   = gcc -O2 -Wall -Werror -Wdangling-pointer=0

SOURCES = $(wildcard *.c)
OBJ := $(SOURCES:%.c=%.o)

all: $(OBJ) lib7z.a

%.o: %.cpp
	$(CC) -o $@ $<

lib7z.a:
	ar rcs $(PROG).a $(OBJ)

clean:
	rm -f *.o *.a
