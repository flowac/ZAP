PROG = lib7z

CC   = gcc -O2 -Wall -Werror -IC -c -o

ifeq ($(OS),Windows_NT)
RM   = del
CC  += -D_WIN32
else
RM   = rm -f
endif

all:
	$(CC) f1.o C/Alloc.c
	$(CC) f2.o C/LzFind.c
	$(CC) f3.o C/LzmaDec.c
	$(CC) f4.o C/LzmaLib.c
	$(CC) f5.o C/LzmaEnc.c
	ar rcs ../../lib/$(PROG).a f1.o f2.o f3.o f4.o f5.o

clean:
	$(RM) *.o
