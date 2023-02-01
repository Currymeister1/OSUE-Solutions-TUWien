# @file Makefile
# @author 
# @date 11.12.2022

# @brief Make file for forkFFT

CC = gcc
DEFS =  -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lm

OBJECTS = forkFFT.o

.PHONY: all clean
all: forkFFT

forkFFT: $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

forkFFT.o: forkFFT.c forkFFT.h

clean:
	rm -rf *.o forkFFT
	