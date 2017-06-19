# executable
PRG = test

ifneq ($(OS),Windows_NT)
RM    = rm -f
CC    = g++
CCX   = gcc
SSL   = /usr/include/openssl
BOOST = /usr/include/boost
else
PRG   = test.exe
RM    = del
CCDIR = C:\MinGW\bin
CC    = $(CCDIR)\g++ -DWINDOWS
CCX   = $(CCDIR)\gcc -DWINDOWS
SSL   = winextern\openssl
BOOST = winextern\boost
BOOST_ROOT = $(BOOST)\boost
endif

# default rule and rule shortcuts
all: extern torrent
clean: c
c: c3

FLAGS += -O3
ARGS_EXTERN = all
debug: FLAGS:=$(filter-out -O3,$(FLAGS))
debug: ARGS_EXTERN = debug
debug: FLAGS += -g
debug: all

.PHONY: extern cleanExtern
# directory structure
IDIR = include
IDIR_EXTERN = extern/7z
LDIR = lib
ODIR = obj
SDIR = src

# objects, src and user libs
SOURCES = $(wildcard $(SDIR)/*.cpp)
INCLUDE = $(wildcard $(IDIR)/*.h)
INCLUDE_EXTERN = $(wildcard $(IDIR)/*.h)
OBJ := $(SOURCES:$(SDIR)/%.cpp=$(ODIR)/%.o)

FLAGS += -Wall -std=gnu++11 -I$(IDIR) -I$(SSL)/include -I$(IDIR_EXTERN) -I$(BOOST)
# remove lrt later my work pc is retarded
LIBS = -L$(BOOST) -L$(SSL)/lib -lssl -lcrypto -lpthread #-lrt
# statically linked libraries
SLIB = $(LDIR)/lib7z.a

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
	$(CCX) -o wrap $(SDIR)/arg_wrap.c $(SDIR)/arg_test.c -I$(IDIR)

cf:
	$(RM) log
	$(RM) temp*.*

ifneq ($(OS),Windows_NT)
cleanC1 = $(ODIR)/*.o
else
cleanC1 = $(ODIR)\*.o
endif
c1:
	$(RM) $(PRG)
	$(RM) $(cleanC1)
	$(RM) wrap

ifneq ($(OS),Windows_NT)
cleanC2a = extern/7z
cleanC2b = $(LDIR)/*.a
else
cleanC2a = extern\7z
cleanC2b = $(LDIR)\*.a
endif
c2: c1
	$(MAKE) -C $(cleanC2a) clean
	$(RM) $(cleanC2b)

c3: c2 cf
