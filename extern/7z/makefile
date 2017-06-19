debug: CFLAGS += -g
debug: all
PROG = lib7z

ifneq ($(OS),Windows_NT)
RM    = rm -f
AR    = ar rcs
CXX   = g++
CXX_C = gcc
else
RM    = del
CCDIR = C:\MinGW\bin
AR    = $(CCDIR)\ar rcs
CXX   = $(CCDIR)\g++
CXX_C = $(CCDIR)\gcc
endif

CXX  += -O2
CXX_C+= -O2 -Wall -Werror -I$(IDIR)

ODIR = ./obj
SDIR = ./C
IDIR = ./C

CFLAGS += -c -DDEBUG

OBJS = \
  Alloc.o \
  LzFind.o \
  LzmaDec.o \
  LzmaLib.o \
  LzmaEnc.o \
#Threads.o \
#Lzma2Enc.o \
#MtCoder.o \

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

#Threads.o: C/Threads.c
#	$(CXX_C) $(CFLAGS) C/Threads.c
#
#Lzma2Enc.o: C/Lzma2Enc.c
#	$(CXX_C) $(CFLAGS) C/Lzma2Enc.c
#
#MtCoder.o: C/MtCoder.c
#	$(CXX_C) $(CFLAGS) C/MtCoder.c

clean:
	$(RM) Alloc.o
	$(RM) LzFind.o
	$(RM) LzmaDec.o
	$(RM) LzmaLib.o
	$(RM) LzmaEnc.o
#	$(RM) Threads.o
#	$(RM) Lzma2Enc.o
#	$(RM) MtCoder.o
