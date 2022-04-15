PROG = lib7z
CC   = gcc -O2 -Wall -Werror

SOURCES = $(wildcard *.c)
OBJ := $(SOURCES:%.c=%.o)

all: $(OBJ) lib7z.a
	rm -f *.o

%.o: %.cpp
	$(CC) -o $@ $<

lib7z.a:
	echo $(SOURCES)
	echo $(OBJ)
	ar rcs $(PROG).a $(OBJ)
