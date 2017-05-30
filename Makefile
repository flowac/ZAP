# default rule and rule shortcuts
all: extern torrent
ca: c cleanExtern
c: clean clearLog
debug: FLAGS += -g
debug: extern_d torrent

.PHONY: extern cleanExtern
CC  = g++
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

# compile flags and libraries
FLAGS += -Wall -I$(IDIR) -I/usr/include/openssl -I$(IDIR_EXTERN)
LIBS = -L/usr/include/boost/ -lssl -lcrypto
# statically linked libraries
SLIB = $(LDIR)/lib7z.a

# executable
PRG = test


# create executable
torrent: $(OBJ)
	$(CC) $(OBJ) $(SLIB) -o $(PRG) $(LIBS) $(FLAGS)

# compile 7zip
extern:
	$(MAKE) -C extern/7z all

extern_d:
	$(MAKE) -C extern/7z debug

# compile src files into objects
$(ODIR)/%.o: $(SDIR)/%.cpp $(INCLUDE) $(INCLUDE_EXTERN)
	$(CC) -c -o $@ $< $(FLAGS)

clean:
	rm -f $(PRG) $(ODIR)/*.o

cleanExtern:
	$(MAKE) -C extern/7z clean
	rm -f $(LDIR)/*.a

clearLog:
	rm -f log
