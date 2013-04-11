# Makefile for gigapan program 
# Author: Sergei Radutnuy

# gcc for compiler
CC= gcc

# debugging symbols in object file and all warnings on
CCFLAGS= -g -Wall

# link math lib, include debugging symbols in exec
LDFLAGS= -lm -g

# executable will be called gigapan
gigapan: gigapan.o

gigapan.o: gigapan.h

# make clean gets rid of old executable and all object files
clean:
	rm -f gigapan *.o

