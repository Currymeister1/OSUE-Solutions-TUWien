/**
  * @file forkFFT.c
  * @author 
  * @date 11.12.2022
  * @brief Program for doing the Fast Fourir Transform
  * @details Using Cooley-Tukey algorithm we can find FFT
*/

#include "forkFFT.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

char *progname = NULL;

/**
  * Usage function
  * @brief explains how the program should work
  * @details If the user provides wrong argument, show usage message and 
  * exit on EXIT_FAILURE
*/
void usage(void){
    fprintf(stderr, "Usage: %s [-p] \n", progname);
    exit(EXIT_FAILURE);
}


/**
  * Error exit function
  * @brief Exit on error
  * @details If some part of program crashes
  * then the program error exists
  * @param msg containg the error message.
*/
void error_exit(char *msg){
    fprintf(stderr, "Error occured: %s \n", msg);
    exit(EXIT_FAILURE);
}

/**
  * Spawn children function
  * @brief use for spawning child processes
  * @details Pipes and the child process is generated
  * @param child the pid and pipes are stored in this sturcture
*/

void spawn_children(child_t *child){
    int pipes[2][2];

    if((pipe(pipes[0]) == -1) || (pipe(pipes[1]) == -1)){
        error_exit("Pipe creation failed!");
    }


    child->pid = fork();

    if(child->pid == -1){
        error_exit("Forking failed!");
    }

    if(child->pid == 0){
        close(pipes[0][1]);
        close(pipes[1][0]);
        
        if((dup2(pipes[0][0], STDIN_FILENO) == -1) || (dup2(pipes[1][1],STDOUT_FILENO) == -1)){
            error_exit("Duplicating file failed");
        }

        close(pipes[0][0]);
        close(pipes[1][1]);

        if(execlp(progname,progname, NULL) == -1){
            error_exit("Excelp failedd");
        }
    }
    else{
    close(pipes[0][0]);
    close(pipes[1][1]);

    child->write = pipes[0][1];
    child->read = pipes[1][0];
    }  
   
    
   
}

/**
  * Write to children function
  * @brief writes to the children stdin
  * @details The numbers at odd and even indices of input are written children responsible for odd and even.
  * @param child_e child responsible for even index numbers
  * @param child_o child responsible for odd index numbers
  * @param evens array containing even index numbers
  * @param odds array containing odd index numbers
  * @param size size of both arrays
*/
void write_to_childern(child_t *child_e, child_t *child_o, comNum_t *evens, comNum_t *odds, int size){
    char oddAsString[MAXLENGTH];
    char evenAsString[MAXLENGTH];
   
    for(int i = 0; i < size; i++){
        snprintf(oddAsString, sizeof(oddAsString), "%f %f*i\n", odds[i].real, odds[i].imaginery);
        snprintf(evenAsString, sizeof(evenAsString), "%f %f*i\n", evens[i].real, evens[i].imaginery);

        write(child_o->write, oddAsString, strlen(oddAsString));
        write(child_e->write, evenAsString, strlen(evenAsString));
    } 
    
  
}

/**
  * Wait for children to finish function
  * @brief Waiting for children to finish
  * @details Wait for children process to finish and
  * if something goes wrong error exit
  * @param child_e child reponsible for even index numbers 
  * @param child_o child responsible for odd index numbers
*/
void wait_for_children_to_finish(child_t *child_e, child_t *child_o){
    int state;
   
    waitpid(child_e->pid, &state, 0);
    
    if(WEXITSTATUS(state) != 0){
        error_exit("WEXITSTATUS failed");
    }

    waitpid(child_o->pid, &state, 0);
    if(WEXITSTATUS(state) != 0){
        error_exit("WEXITSTATUS failed");
    }


}

/**
  * Addition function
  * @brief addition of two complex numbers
  * @param c1 complex number one
  * @param c2 complex number two
  * @param result complex where the addition result is saved
*/
void addition(comNum_t *c1, comNum_t *c2, comNum_t *result){
    float real = c1->real + c2->real;
    float imgainary = c1->imaginery + c2->imaginery;

    result->real = real;
    result->imaginery = imgainary;
}

/**
  * Subtraction function
  * @brief substraction of two complex numbers
  * @param c1 complex number one
  * @param c2 complex number two
  * @param result complex where the substraction result is saved
*/
void substraction(comNum_t *c1, comNum_t *c2, comNum_t *result){
    float real = c1->real - c2->real;
    float imgainary = c1->imaginery - c2->imaginery;

    result->real = real;
    result->imaginery = imgainary;
}

/**
  * Multiplication function
  * @brief multiplication of two complex numbers
  * @param c1 complex number one
  * @param c2 complex number two
  * @param result complex where the multiplication result is saved
*/
void multiplication(comNum_t *c1, comNum_t *c2, comNum_t *result){
    //a · c − b · d + i · (a · d + b · c)
    
    float real = (c1->real * c2->real) - (c1->imaginery * c2->imaginery);
    float imaginary = (c1->real * c2->imaginery) + (c1->imaginery * c2->real);

    result->real = real;
    result->imaginery = imaginary;
}


/**
  * Read from file function
  * @brief read the data from the children
  * @param file file opened with fdopen of child
  * @param comNum the values from the file is stored here
*/
void read_from_file(FILE *file, comNum_t *comNum){
    char *line = NULL;
    char *tmp_pointer;
    size_t length = 0;

    getline(&line, &length, file);
    tmp_pointer = line;
    comNum->real = strtof(line, &line);
    comNum->imaginery = strtof(line, NULL);
    free(tmp_pointer);
}

/**
  * Butterfly function
  * @brief Butterfly operation as described in pdf
  * @param e_result even index results stored here
  * @param o_result odd index results stored here
  * @param evens even index numbers from input
  * @param odds odd index nubmers from input
  * @param size size of the input
  * @param i index in evens and odds
*/
void butterfly(comNum_t *e_result, comNum_t *o_result, comNum_t *evens, comNum_t *odds, int size, int i){
    e_result->real = cos((-(2 * PI) / size) * i);
  
    e_result->imaginery = sin((-(2 * PI) / size) * i);
     
    multiplication(e_result,odds ,e_result);
     
    addition(e_result, evens, e_result);
    
  
    o_result->real = cos((-(2 * PI) / size) * i);
    
    o_result->imaginery = sin((-(2 * PI) / size) * i);
 
    multiplication( o_result, odds,o_result);
 
    substraction(evens, o_result, o_result);

}

/**
  * Calculate result
  * @brief calculates the result
  * @param file_even even values to be read
  * @param file_odd odd values to be read
  * @param size size of the odd and even arrays
  * @param result to store results
*/
void calculate_result(FILE *file_even, FILE *file_odd, int size, comNum_t *result){
    int realSize = size*2;
    for(int i = 0; i < size; i++){
        comNum_t even;
        comNum_t odd;

        read_from_file(file_even, &even);
        read_from_file(file_odd, &odd);
        
        comNum_t even_result;
        comNum_t odd_result;

        butterfly(&even_result, &odd_result, &even, &odd, realSize, i);

        result[i] = even_result;
        result[i+size] = odd_result;
        
    }
}

/**
  * Convert input to complex num function
  * @brief converts the input to complex number
  * @param comNum to store complex number
  * @param index use for tracking the size of input
*/
void convert_input_to_complex_num(comNum_t *comNum, int *index){
    char *endptr;

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    while ((nread = getline(&line, &len, stdin)) != -1) {
        comNum[(*index)].real = strtof(line, &endptr);
       
        if(comNum[(*index)].real == 0 && (line == endptr || errno != 0)){
            error_exit("Wrong input (real)");
        }

        if(!(*endptr == ' ' || *endptr == '\n')){
            error_exit("Wrong input (after)");
        }

        if(*endptr == ' '){
            comNum[(*index)].imaginery = strtof(endptr, &endptr);
            if(strcmp("*i\n", endptr) != 0){
            error_exit("Wrong input (imaginary)");
            }
        if(*endptr == '\n'){
            comNum[(*index)].imaginery = 0;
        }
    }
    if(isspace(*endptr)){
        comNum[(*index)].imaginery = 0;
    }
        

        (*index)++;
    }
    free(line);

}


/**
  * Seperate odd even function
  * @brief Seperates the even and odd numbers from input
  * @param complexNumbers input
  * @param odds to store odd index number
  * @param even to store even index number
  * @param size size of the input
*/
void seperate_odd_even(comNum_t *complexNumbers, comNum_t *odds, comNum_t *evens, int size){
    int odd = 0;
    int even = 0;
    for(int i = 0; i < size; i++){
        if(i % 2 == 0){
            evens[even++] = complexNumbers[i];
        }
        else{
            odds[odd++] = complexNumbers[i];
        }
    }

}


/**
  * Print result
  * @brief prints the result
  * @details to tackle the -0.000 problem, fabs is used
  * @param p_flag to check if the input should be round 3 digits
  * @param comNum result to be printed
*/
void print_result(bool p_flag, comNum_t comNum){
    if(p_flag){
        
        if (fabs(comNum.imaginery) < 0.0001 &&fabs(comNum.real) < 0.0001){
            fprintf(stdout, "%.3f %.3f*i \n", fabs(comNum.real), fabs(comNum.imaginery));
        }

        else if(fabs(comNum.real) < 0.0001){
            fprintf(stdout, "%.3f %.3f*i \n", fabs(comNum.real), comNum.imaginery);
        }
        else if (fabs(comNum.imaginery) < 0.0001){
            fprintf(stdout, "%.3f %.3f*i \n", comNum.real, fabs(comNum.imaginery));
        }
        else{
            fprintf(stdout, "%.3f %.3f*i \n", comNum.real, comNum.imaginery);
        }
        
    }
    else{
       
        fprintf(stdout, "%f %f*i \n", comNum.real, comNum.imaginery);
    }
}

/**
  * Main function
  * @brief entry point to the program
  * @details Input is read from the stdin and FFT is done
  * @param argc
  * @param argv
  * @return EXIT_SUCCESS or EXIT_FAILURE
*/
int main(int argc, char **argv){
    progname = argv[0];


    bool p_flag = false;

    int c = 0;

    while((c = getopt(argc, argv, "p")) != -1){
        switch (c){
            case 'p':
                if(p_flag == true){
                    usage();
                    break;
                }
                p_flag = true;
                break;
            case '?':
                usage();
                break;
            default:
                usage();
                break;
        }
    }

    //Checking the input size
    if((p_flag && argc != 2) || (!p_flag && argc != 1)){
        usage();
    }

    comNum_t complexNumbers[MAXLENGTH];
    int size = 0;

    convert_input_to_complex_num(complexNumbers, &size);


    if(size == 1){
        print_result(p_flag, complexNumbers[0]);
        exit(EXIT_SUCCESS);
    }

   
    if(size%2 != 0){
        error_exit("Wrong input size");
    }


    int halfSize = size/2;
    comNum_t oddNums[halfSize];
    comNum_t evenNums[halfSize];
    seperate_odd_even(complexNumbers, oddNums, evenNums, size);
    
    child_t child_e;
    child_t child_o;

    spawn_children(&child_e);
    spawn_children(&child_o);


    write_to_childern(&child_e, &child_o, evenNums, oddNums, halfSize);
    
   

    close(child_o.write);
    close(child_e.write);
    
    wait_for_children_to_finish(&child_e, &child_o);

   
    FILE *f_e = fdopen(child_e.read, "r");
    FILE *f_o = fdopen(child_o.read, "r");


    comNum_t results[size];
    calculate_result(f_e, f_o, halfSize, results);

    for(int i = 0; i < size; i++){
        print_result(p_flag, results[i]);
    }

    fclose(f_o);
    fclose(f_e);
    exit(EXIT_SUCCESS);
}