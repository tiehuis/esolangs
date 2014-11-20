CC       := gcc
CFLAGS   := -march=native -O2
CLIBS	 := -ltcc -ldl

CXX      := g++
CXXFLAGS := -std=c++11 -march=native -O2

all: befunge bfi bft malbolge

bft: src/bft.cpp
	$(CXX) $(CXXFLAGS) src/bft.cpp -o bft

bfi: src/bfi.c
	$(CC) $(CFLAGS) src/bfi.c -o bfi $(CLIBS)

malbolge: src/malbolge.c
	$(CC) $(CFLAGS) src/malbolge.c -o malbolge

befunge: src/befunge.c
	$(CC) $(CFLAGS) src/befunge.c -o befunge

clean: 
	@rm -f befunge bft bfi malbolge

.PHONY: all clean
