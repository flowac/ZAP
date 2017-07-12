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
C/7zTypes.h \
C/Alloc.c \
C/Alloc.h \
C/LzFind.c \
C/LzFind.h \
C/LzmaDec.c \
C/LzmaDec.h \
C/LzmaEnc.c \
C/LzmaEnc.h \
C/LzmaLib.c \
C/LzmaLib.h
