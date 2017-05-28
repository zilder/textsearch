CC = g++
CFLAGS = -std=c++11 -g3
SOURCES = main.cpp inverted.cpp

all:
	$(CC) $(SOURCES) -o bin/ts $(CFLAGS)
