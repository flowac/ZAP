# executable
PRG = test

CC    = g++ -std=c++14
CCX   = gcc -std=c11
RM    = rm -f

# for windows os
ifeq ($(OS),Windows_NT)
SSL   = winextern\openssl
BOOST = winextern\boost
#temporary not used flag
FLAGS+= -DBOOST_REGEX
else
LIBS += -lrt
SSL   = /usr/include/openssl
BOOST = /usr/include/boost
endif

# default rule and rule shortcuts
all: extern torrent

FLAGS += -O3
FLAGS += -Wall -Wno-format -I$(IDIR) -I$(SSL)/include -I$(IDIR_EXTERN) -I$(BOOST)
ARGS_EXTERN = all

debug: ARGS_EXTERN = debug
debug: FLAGS += -g
debug: all

.PHONY: extern

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

clean: clean_local clean_files
	$(MAKE) -C extern/7z clean
	$(RM) $(LDIR)/*.a

clean_local:
	$(RM) $(PRG)*
	$(RM) $(ODIR)/*.o

clean_files:
	$(RM) log
	$(RM) temp*.*
	$(RM) orig*.*

c:  clean
cl: clean_local
cf: clean_files
