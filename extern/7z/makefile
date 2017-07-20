PROG = lib7z
CC   = gcc -O2 -Wall -Werror -IC -c

all:
	$(CC) -o f1.o C/Alloc.c
	$(CC) -o f2.o C/LzFind.c
	$(CC) -o f3.o C/LzmaDec.c
	$(CC) -o f4.o C/LzmaLib.c
	$(CC) -o f5.o C/LzmaEnc.c
	ar rcs ../../lib/$(PROG).a f1.o f2.o f3.o f4.o f5.o

clean:
	rm -f *.o
