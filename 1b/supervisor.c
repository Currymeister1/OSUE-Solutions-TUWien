/**
  * @file supervisor.c
  * @author 
  * @date 13.11.2020

  * @brief The supervisor module
  * @details The supervisor reads from the shared memory the results.
**/

#include <signal.h>
#include <stdio.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "circularBuffer.h"

char *progname;

volatile sig_atomic_t quit = 0;

/**
  * Usage function
  * @brief explains how to use the programm
  * @details If the user provides wrong argument, show usage message and 
  * exit on EXIT_FAILURE
**/
void usage(void){
    fprintf(stderr, "Usage: %s doesn't require any arguments\n", progname);
    exit(EXIT_FAILURE);
}

/**
  * Handle Signal function
  * @brief used for sa.sa_handler. 
  * @details set the quit to one.
  * @param signal 
**/
void handle_signal(int signal) { quit = 1; }

/**
  * Main function
  * @brief entry point to the program
  * @details The supervisor reads from the shared memeory and
  * if the current result is worse than the better result,
  * then the current result becomes the better result and
  * it is printed out. If the graph is 3 colourable, 
  * the supervisor tells that and ends the program with
  * EXIT_SUCCESS.
  * @param argc
  * @param argv
  * @return EXIT_SUCCESS or EXIT_FAILURE
**/
int main(int argc, char **argv){
    bool isSupervisor = true;
    progname = argv[0];

   
    if(argc > 1){
        usage();
    }

  
    //Signal Setup
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    //Ctrl+c
    sigaction(SIGINT, &sa, 0);
    //kill command
    sigaction(SIGTERM, &sa, 0);
    
    struct Result currentResult = {.amount = INT_MAX};
    struct Result betterResult = currentResult;

   //Open the buffer
    open_buffer(isSupervisor);

    
    while(!quit){
        read_from_buffer(&currentResult);
        
        //Graph is 3 colourable
        if(currentResult.amount == 0){
            fprintf(stdout,"The given graph is 3-Colourable. \n");
            break;
        }
        
        //better result found
        if(currentResult.amount < betterResult.amount ){
            betterResult = currentResult;
            print_result(&betterResult);
        }
    }


    clean_exit(isSupervisor);

}


