# default rule and rule shortcuts
all: extern torrent
ca: c cleanExtern
c: clean clearLog

.PHONY: extern cleanExtern
CC  = g++
# directory structure
IDIR = ./include
ODIR = ./obj
SDIR = ./src

# objects, src and user libs
SOURCES = $(wildcard $(SDIR)/*.cpp)
INCLUDE = $(wildcard $(IDIR)/*.h)
OBJ := $(SOURCES:$(SDIR)/%.cpp=$(ODIR)/%.o)

# compile flags and libraries
FLAGS = -Wall -I$(IDIR) -I/usr/include/openssl
LIBS = -L/usr/include/boost/ -L./lib -lssl -lcrypto -l7z

# executable
PRG = test

# create executable
torrent: $(OBJ)
	$(CC) $(OBJ) -o $(PRG) $(LIBS) $(FLAGS)

# compile 7zip
extern:
	$(MAKE) -C extern/7z all

# compile src files into objects
$(ODIR)/%.o: $(SDIR)/%.cpp $(INCLUDE)
	$(CC) -c -o $@ $< $(FLAGS)

clean:
	rm -f $(PRG) $(ODIR)/*.o

cleanExtern:
	$(MAKE) -C extern/7z clean

clearLog:
	rm -f log
