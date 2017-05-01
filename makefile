CC  = g++
PRG = test
INC = -L/usr/include/boost/
OBJ = main.o
CMP = -Wall

all: $(OBJ)
	$(CC) $(OBJ) -o$(PRG) $(INC) $(CMP)

main.o:
	$(CC) -c main.cpp -o$@ $(CMP)

clean:
	rm $(PRG) *.o
