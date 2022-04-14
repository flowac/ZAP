# for windows
if CHECK_OS
RM    = rm -f
#AR    = ar rcs
else
#RM    = del
#CCDIR = C:\MinGW\bin
#AR    = $(CCDIR)\ar rcs
#CFLAGS += -D_WIN32
endif

noinst_LIBRARIES=lib7z.a
lib7z_a_SOURCES=\
7zTypes.h \
Alloc.c \
Alloc.h \
LzFind.c \
LzFind.h \
LzmaDec.c \
LzmaDec.h \
LzmaEnc.c \
LzmaEnc.h \
LzmaLib.c \
LzmaLib.h
