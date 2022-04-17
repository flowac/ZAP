# executable
PRG = test

CC    = g++ -std=c++20
RM    = rm -f

# for windows os
ifeq ($(OS),Windows_NT)
SSL   = winextern\openssl
else
LIBS += -lrt
SSL_INC = /usr/include/openssl
endif

# directory structure
7Z_DIR = extern/7z
ODIR = obj
SDIR = src

# objects, src and user libs
SOURCES = $(wildcard $(SDIR)/*.cpp)
OBJ := $(SOURCES:$(SDIR)/%.cpp=$(ODIR)/%.o)

LIBS += -lssl -lcrypto
FLAGS += -O2
FLAGS += -Wall -fpermissive -Iinclude -I$(SSL_INC) -I$(7Z_DIR)
ARGS_EXTERN = all

# default rule and rule shortcuts
all: lib7z.a $(OBJ)
	$(CC) $(OBJ) $(7Z_DIR)/*.o -o $(PRG) $(LIBS) $(FLAGS)

debug: FLAGS += -g
debug: all

# compile 7zip
lib7z.a:
	$(MAKE) -j12 -C $(7Z_DIR) $(ARGS_EXTERN)

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
