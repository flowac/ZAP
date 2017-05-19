# executable
PRG = test

# default rule and rule shortcuts
all: extern torrent
clean: c
c: c3

.PHONY: extern cleanExtern
CC = g++
# directory structure
IDIR = ./include
LDIR = ./lib
ODIR = ./obj
SDIR = ./src

# objects, src and user libs
SOURCES = $(wildcard $(SDIR)/*.cpp)
INCLUDE = $(wildcard $(IDIR)/*.h)
OBJ := $(SOURCES:$(SDIR)/%.cpp=$(ODIR)/%.o)

# compile flags and libraries
FLAGS = -Wall -I$(IDIR) -I/usr/include/openssl
LIBS = -L/usr/include/boost/ -lssl -lcrypto
# statically linked libraries
SLIB = $(LDIR)/lib7z.a

# create executable
torrent: $(OBJ)
	$(CC) $(OBJ) $(SLIB) -o $(PRG) $(LIBS) $(FLAGS)

# compile 7zip
extern:
	$(MAKE) -C extern/7z all

# compile src files into objects
$(ODIR)/%.o: $(SDIR)/%.cpp $(INCLUDE)
	$(CC) -c -o $@ $< $(FLAGS)

c1:
	rm -f $(PRG) $(ODIR)/*.o

c2: c1
	$(MAKE) -C extern/7z clean
	rm -f $(LDIR)/*.a

c3: c2
	rm -f log
