PROG = lib7z
CC   = gcc -O2 -Wall -Werror -IC -c

all:
	$(CC) -o f1.o Alloc.c
	$(CC) -o f2.o LzFind.c
	$(CC) -o f3.o LzmaDec.c
	$(CC) -o f4.o LzmaLib.c
	$(CC) -o f5.o LzmaEnc.c
	ar rcs ../../lib/$(PROG).a f1.o f2.o f3.o f4.o f5.o

clean:
	rm -f *.o
