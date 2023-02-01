# @file Makefile
# @author 
# @date 15.01.2023
#
# @brief The makefile for compiling client.c
# 

all: client
 
compile_flags = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SCID_SOURCE -D_POSIX_C_SOURCE=200809L -g

client.o: client.c
	gcc $(compile_flags) -c client.c

client: client.o
	gcc -o client client.o

	
clean:
	rm -f *.o
	rm -f client