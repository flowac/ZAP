# executable
PRG = test

# default rule and rule shortcuts
all: extern torrent
clean: c
c: c3
ARGS_EXTERN = all
debug: ARGS_EXTERN = debug
debug: FLAGS += -g
debug: all


.PHONY: extern cleanExtern
CC = g++
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

FLAGS += -Wall -I$(IDIR) -I/usr/include/openssl -I$(IDIR_EXTERN)
# remove lrt later my work pc is retarded
LIBS = -L/usr/include/boost/ -lssl -lcrypto -lpthread -lrt
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
	$(CC) -c -o $@ $< $(FLAGS)

cf:
	rm -f log temp*.file temp*.unc t*.7z

c1:
	rm -f $(PRG) $(ODIR)/*.o

c2: c1
	$(MAKE) -C extern/7z clean
	rm -f $(LDIR)/*.a

c3: c2 cf
