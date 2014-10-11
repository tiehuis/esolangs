CC	   = g++
FLAGS  = -std=c++11 -Wextra -Wall
INPUT  = src/bft.cpp
PROG   = bft

all:
	$(CC) $(FLAGS) $(INPUT) -o $(PROG)
