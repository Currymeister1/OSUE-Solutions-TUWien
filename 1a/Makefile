# @file Makefile
# @author 
# @date 13.11.2022
#
# @brief Makefile for mygrep

all: mygrep


CC = gcc

DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = mygrep.o

.PHONY: all clean

all: mygrep

mygrep: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

mygrep.o: mygrep.c 

clean:
	rm -rf *.o mygrep