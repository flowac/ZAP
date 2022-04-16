# executable
PRG = test

CC    = g++ -std=c++14
CCX   = gcc -std=c11
RM    = rm -f

# for windows os
ifeq ($(OS),Windows_NT)
SSL   = winextern\openssl
else
LIBS += -lrt
SSL_INC = /usr/include/openssl
endif

# default rule and rule shortcuts
all: lib7z.a torrent

FLAGS += -O2
FLAGS += -Wall -Wno-format -Iinclude -I$(SSL_INC) -I$(7Z_DIR)
ARGS_EXTERN = all

debug: ARGS_EXTERN = debug
debug: FLAGS += -g
debug: all

# directory structure
7Z_DIR = extern/7z
ODIR = obj
SDIR = src

# objects, src and user libs
SOURCES = $(wildcard $(SDIR)/*.cpp)
OBJ := $(SOURCES:$(SDIR)/%.cpp=$(ODIR)/%.o)

LIBS += -lssl -lcrypto
# statically linked libraries
SLIB = $(7Z_DIR)/lib7z.a

# create executable
torrent: $(OBJ)
	$(CC) $(OBJ) $(SLIB) -o $(PRG) $(LIBS) $(FLAGS)

# compile 7zip
lib7z.a:
	$(MAKE) -C $(7Z_DIR) $(ARGS_EXTERN)

# compile src files into objects
$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CC) -c -o $@ $< $(FLAGS) $(LIBS)

clean: clean_local clean_files
	$(MAKE) -C $(7Z_DIR) $@

clean_local:
	$(RM) $(PRG)*
	$(RM) $(ODIR)/*.o

clean_files:
	$(RM) log
	$(RM) temp*.*

c:  clean
cl: clean_local
cf: clean_files
