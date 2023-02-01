# @file Makefile
# @author 
# @date 13.11.2022
#
# @brief Makefile for genertor and supervisor 
all: generator supervisor

CC = gcc

DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

LDFLAGS = -lpthread -lrt

generator.o: generator.c
	$(CC) $(CFLAGS) -c generator.c

generator: generator.o circularBuffer.o
	$(CC) -o generator generator.o circularBuffer.o $(LDFLAGS)

supervisor.o: supervisor.c
	$(CC) $(CFLAGS) -c supervisor.c

supervisor: supervisor.o circularBuffer.o
	$(CC) -o supervisor supervisor.o circularBuffer.o $(LDFLAGS)

circularBuffer.o: circularBuffer.c
	$(CC) $(CFLAGS) -c circularBuffer.c
	
clean:
	rm -f *.o
	rm -f generator
	rm -f supervisor
