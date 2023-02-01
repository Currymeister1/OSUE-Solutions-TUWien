/**
  * @file forkFFT.h
  * @author 
  * @date 11.12.2022

  * @brief Header file for FORKFTT
*/
#include <stdlib.h>


#define PI 3.141592654
#define MAXLENGTH 100000


/**
  * Structure for complex numbers
  * @brief The representation of complex numbers
  * @details A complex number has a real and imaginary part
*/
typedef struct ComplexNumber{
    float real;
    float imaginery;
} comNum_t;


/**
  * Structure for child process
  * @brief The representation of a child process.
  * @details pid for process id, write to write to child's stdin and read to read from child's stdout
*/
typedef struct Child{
    pid_t pid;
    int write;
    int read;
} child_t;
