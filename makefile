# Makefile for Assignment 3
CC=gcc
CFLAGS= -g -c -Wall `sdl-config --cflags`
LDFLAGS= `sdl-config --libs`
SOURCES=dpcm.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=dpcm

all: $(SOURCES) $(EXECUTABLE)
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
.c.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm *.o
	rm $(EXECUTABLE)
