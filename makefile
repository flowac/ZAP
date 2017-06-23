# executable
PRG = test

# for windows os
ifneq ($(OS),Windows_NT)
LIBS += -lrt
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

FLAGS += -O3
FLAGS += -Wall -Wno-format -I$(IDIR) -I$(SSL)/include -I$(IDIR_EXTERN) -I$(BOOST)
#FLAGS += std=gnu++11
ARGS_EXTERN = all
#debug: FLAGS:=$(filter-out -O3,$(FLAGS))
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

# remove lrt later my work pc is retarded
LIBS += -L$(BOOST) -L$(SSL)/lib -lssl -lcrypto -lpthread
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

clean_files:
	$(RM) log
	$(RM) temp*.*

ifneq ($(OS),Windows_NT)
clean_ODIR = $(ODIR)/*.o
else
clean_ODIR = $(ODIR)\*.o
endif
clean_local:
	$(RM) $(PRG)
	$(RM) $(clean_ODIR)
	$(RM) wrap

ifneq ($(OS),Windows_NT)
clean_extern_dir = extern/7z
clean_lib = $(LDIR)/*.a
else
clean_extern_dir = extern\7z
clean_lib = $(LDIR)\*.a
endif
# clean local and extern
clean: clean_local
	$(MAKE) -C $(clean_extern_dir) clean
	$(RM) $(clean_lib)

# clean local, extern and extra files
clean_all: clean clean_files
cf: clean_files
c: clean_all
