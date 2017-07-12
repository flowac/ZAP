debug: CFLAGS += -g -DDEBUG
debug: all
PROG = lib7z

AR    = ar rcs
CXX   = g++
CXX_C = gcc

ifneq ($(OS),Windows_NT)
RM    = rm -f
else
RM    = del
CFLAGS += -D_WIN32
endif

CXX  += -O2
CXX_C+= -O2 -Wall -Werror -I$(IDIR)

ODIR = ./obj
SDIR = ./C
IDIR = ./C

CFLAGS += -c

OBJS = \
  Alloc.o \
  LzFind.o \
  LzmaDec.o \
  LzmaLib.o \
  LzmaEnc.o

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

LzmaEnc.o: C/LzmaEnc.c
	$(CXX_C) $(CFLAGS) C/LzmaEnc.c

clean:
	$(RM) Alloc.o
	$(RM) LzFind.o
	$(RM) LzmaDec.o
	$(RM) LzmaLib.o
	$(RM) LzmaEnc.o
