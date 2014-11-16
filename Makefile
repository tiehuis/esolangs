CC       := gcc
CFLAGS   := -march=native -O2
CLIBS	 := -ltcc -ldl

CXX      := g++
CXXFLAGS := -std=c++11 -march=native -O2

all: bfi bft

bft: src/bft.cpp
	$(CXX) $(CXXFLAGS) src/bft.cpp -o bft

bfi: src/bfi.c
	$(CC) $(CFLAGS) src/bfi.c -o bfi $(CLIBS)

clean: 
	@rm -f bft bfi

.PHONY: all clean
