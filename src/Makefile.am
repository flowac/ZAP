PRG = test

# for windows os
if CHECK_OS
#LIBS += -lrt
#RM    = rm -f
#CC    = g++
#CCX   = gcc
SSL   = /usr/include/openssl
BOOST = /usr/include/boost
else
#PRG   = test.exe
#RM    = del
#CCDIR = C:\MinGW\bin
#CC    = $(CCDIR)\g++ -DWINDOWS
#CCX   = $(CCDIR)\gcc -DWINDOWS
#SSL   = winextern\openssl
#BOOST = winextern\boost
#BOOST_ROOT = $(BOOST)\boost
endif

# include dir
AM_CPPFLAGS = \
-I$(top_srcdir)/include \
-I$(top_srcdir)/extern/7z \
-I$(BOOST) \
-I$(SSL)/include

AM_CPPFLAGS += -Wall -Wno-format

bin_PROGRAMS = test

test_SOURCES = \
alib.cpp \
alibio.cpp \
log.cpp \
lzma_wrapper.cpp \
main.cpp \
ssl_fn.cpp \
time_fn.cpp

test_LDADD = ../extern/7z/lib7z.a
