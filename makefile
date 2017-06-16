# executable
PRG = test

WINDOWS = 1
ifndef WINDOWS
CC    = g++
CCX   = gcc
SSL   = /usr/include/openssl
BOOST = /usr/include/boost
else
BOOST_ROOT = C:\Users\acai\Downloads\0share\winextern\boost\boost
CCDIR = C:\PROGRA~1\MINGW-~1\X86_64~1.0-W\mingw64\bin
CC    = $(CCDIR)\g++ -DWINDOWS #x86_64-w64-mingw32-g++.exe
CCX   = $(CCDIR)\gcc -DWINDOWS #x86_64-w64-mingw32-gcc.exe
SSL   = C:\bin\Gnu\src\openssl\0.9.8h\openssl-0.9.8h\include
BOOST = C:\Users\acai\Downloads\0share\winextern\boost
endif

# default rule and rule shortcuts
all: extern torrent
clean: c
c: c3
ARGS_EXTERN = all
debug: ARGS_EXTERN = debug
debug: FLAGS += -g
debug: all

.PHONY: extern cleanExtern
# directory structure
IDIR = ./include
IDIR_EXTERN = ./extern/7z
LDIR = ./lib
ODIR = ./obj
SDIR = ./src

# objects, src and user libs
SOURCES = $(wildcard $(SDIR)/*.cpp)
INCLUDE = $(wildcard $(IDIR)/*.h)
INCLUDE_EXTERN = $(wildcard $(IDIR)/*.h)
OBJ := $(SOURCES:$(SDIR)/%.cpp=$(ODIR)/%.o)

FLAGS += -Wall -I$(IDIR) -I$(SSL) -I$(IDIR_EXTERN) -I$(BOOST)
# remove lrt later my work pc is retarded
LIBS = -L$(BOOST) -lssl -lcrypto -lpthread -lrt
# statically linked libraries
SLIB = $(LDIR)/lib7z.a

# executable
PRG = test

# create executable
torrent: $(OBJ)
	$(CC) $(OBJ) $(SLIB) -o $(PRG) $(LIBS) $(FLAGS)

# compile 7zip
extern:
	$(MAKE) -C extern/7z $(ARGS_EXTERN)

# compile src files into objects
$(ODIR)/%.o: $(SDIR)/%.cpp $(INCLUDE) $(INCLUDE_EXTERN)
	$(CC) -c -o $@ $< $(FLAGS) $(LIBS)

wrap:
	gcc -o wrap $(SDIR)/arg_wrap.c $(SDIR)/arg_test.c -I$(IDIR)

cf:
	rm -f log temp*.file temp*.unc t*.7z

c1:
	rm -f $(PRG) $(ODIR)/*.o wrap

c2: c1
	$(MAKE) -C extern/7z clean
	rm -f $(LDIR)/*.a

c3: c2 cf
