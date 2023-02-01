/**
  * @file circularBuffer.h
  * @author 
  * @date 13.11.2022 
  * @brief The module containing functions for opening/closing the shared buffer and writing/reading from the shared buffer.
  * @details Furthermore, print result, get/set state, clean exit and failed exit functions are provided. 
  * These functions are used by the supervisor and generator programms. 
**/
#include <stdbool.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdbool.h>


#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H


#define SHM_NAME "/11937894_shm"
#define SEM_FREE "/11937894_sem_free"
#define SEM_USE "/11937894_sem_use"
#define SEM_WRITE "/11937894_sem_write"

extern int shfmd;
extern struct Buffer *buffer;
extern sem_t *free_sem;
extern sem_t *use_sem;
extern sem_t *write_sem;

#define MAX_EDGES (8)
#define BUFFER_SIZE (5)


/**
  * Structure for the node
  * @brief The representation of a node
  * @details A node has a value and a colour.
**/
struct Node{
    int value;
    int colour;
};

/**
  * Structure for the edge
  * @brief The representation of an edge
  * @details An edge contains two nodes
**/
struct Edge{
    struct Node n1;
    struct Node n2;
};

/**
  * Structure for the result
  * @brief The representation of a result
  * @details An optimal result produced by the generator and read by the supervisor.
  * A result has the number of removed edges and those edges. 
**/
struct Result{
    int amount;
    struct Edge edges[MAX_EDGES];
};

/**
  * Structure for the circular buffer
  * @brief The representation of a buffer
  * @details The shared circular buffer containing read- write-index, the results and state telling the generators to stop or not.
  * A generator writes to the shared ciruclar buffer the results and the supervisor reads them from the shared circular buffer.
**/
struct Buffer{
    int read_index;
    int write_index;
    struct Result results[BUFFER_SIZE];
    bool stop;
};


/**
  * Open Buffer function
  * @brief Open the circular buffer as a supervisor or generator.
  * @details Setup the shared memory and semaphores for the supervisor and the generators. 
  * The supervisor must first opens the shared buffer. And then the generator opens it. 
  * Else the program will crash. 
  * @param isSupervisor checking if the supervisor called the function or not. Based on the param 
  * function will do slightly different things.
**/
void open_buffer(bool isSupervisor);

/**
  * Close Buffer function
  * @brief Close the circular buffer as a supervisor or generator.
  * @details Shared memory and all semaphores are closed for the supervisor and generator.
  * @param isSupervisor checking if the supervisor called the function or not. Based on the param 
  * function will do slightly different things.
**/
void close_buffer(bool isSupervisor);

/**
  * Write To Buffer function
  * @brief Write to the circular buffer
  * @details A generator writes to the shared memory a result
  * which it found using the 3 colouring algorithm.
  * @param result The found result
**/
void write_to_buffer(struct Result *result);

/**
  * Read From Buffer function
  * @brief Read from the circular buffer
  * @details The supervisor will read the result
  * from the shared memory, which was produced by
  * a generator.
  * @param result The found result will be written to the memory address of this variable 
**/
void read_from_buffer(struct Result *result);

/**
  * Print Result function
  * @brief Print the result
  * @details Print the result with the number of removed edges
  * and the removed edges.
  * @param result the result to print
**/
void print_result(struct Result *result);

/**
  * Get State function
  * @brief Get the state 
  * @details Used by generators to know if they should stop or not.
  * @return true for stopping, else for continuing 
**/
bool get_state(void);

/**
  * Set State function
  * @brief Set the state
  * @details Used by the supervisor to let generators know if they should
  * produce results or not.
  * @param state State to set
**/
void set_state(bool state);

/**
  * Clean Exit function
  * @brief Function for clean exiting with EXIT SUCCESS and closing the resources.
  * @param isSupervisor Different behaviours for generators and supervisor.
**/
void clean_exit(bool isSupervisor);

/**
  * Failed Exit function
  * @brief Function for failed exiting with EXIT FAILURE and closing the resources.
  * @param isSupervisor Different behaviours for generators and supervisor.
**/
void failed_exit(char* err_msg);

#endif