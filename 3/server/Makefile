# @file Makefile
# @author 
# @date 15.01.2023
#
# @brief The makefile for compiling server.c
# 
all: server
 
compile_flags = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SCID_SOURCE -D_POSIX_C_SOURCE=200809L -g

server.o: server.c
	gcc $(compile_flags) -c server.c

server: server.o
	gcc -o server server.o
	
clean:
	rm -f *.o
	rm -f server