PROG = lib7z
AR = ar rcs

CXX = g++ -O2
CXX_C = gcc -O2 -Wall -Werror
CFLAGS = -c -DDEBUG

ODIR = ./obj
SDIR = ./C
IDIR = ./C

OBJS = \
  Alloc.o \
  LzFind.o \
  LzmaDec.o \
  LzmaLib.o \
  LzFind.o \
  LzmaEnc.o \

all: $(PROG)

$(PROG): $(OBJS)
	$(AR) ../../lib/$(PROG).a $(OBJS)

Alloc.o: C/Alloc.c
	$(CXX_C) $(CFLAGS) C/Alloc.c

LzFind.o: C/LzFind.c
	$(CXX_C) $(CFLAGS) C/LzFind.c

LzmaDec.o: C/LzmaDec.c
	$(CXX_C) $(CFLAGS) C/LzmaDec.c

LzmaLib.o: C/LzmaLib.c
	$(CXX_C) $(CFLAGS) C/LzmaLib.c

LzFind.o: C/LzFind.c
	$(CXX_C) $(CFLAGS) C/LzFind.c

LzmaEnc.o: C/LzmaEnc.c
	$(CXX_C) $(CFLAGS) C/LzmaEnc.c

clean:
	-$(RM) $(OBJS)
