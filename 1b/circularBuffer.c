/**
  * @file circularBuffer.c
  * @author 
  * @date 13.11.2022
  * @brief Implementation of buffer.h
**/

#include <sys/mman.h>
#include <sys/stat.h>        
#include <fcntl.h>           
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>

#include "circularBuffer.h"

//Semaphores
sem_t *free_sem;
sem_t *use_sem;
sem_t *write_sem;

//Share mem file descriptor
int shmfd;

//Circular buffer
struct Buffer *buffer;


void set_state(bool state){
     buffer->stop = state;
}

bool get_state(void){
    return buffer->stop;
}


void clean_exit(bool isSupervisor){
    if(isSupervisor){
        fprintf(stdout, "Shutting down everything. \n");
        sem_post(free_sem);
        set_state(true);
        close_buffer(isSupervisor);
        exit(EXIT_SUCCESS);
    }
    else{
        fprintf(stdout, "Shutting down generator. \n");
        sem_post(free_sem);
        close_buffer(isSupervisor);
        exit(EXIT_SUCCESS);
    }
   
}

void failed_exit(char *err_msg){
    fprintf(stderr, err_msg);
    exit(EXIT_FAILURE);
}

/**
  * Open Semaphores Supervisor function
  * @brief Open semaphores for the supervisor
**/
void open_semaphones_supervisor(void){
     free_sem = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, BUFFER_SIZE);
     use_sem = sem_open(SEM_USE, O_CREAT | O_EXCL, 0600, 0);
     write_sem = sem_open(SEM_WRITE, O_CREAT | O_EXCL, 0600, 1);
       
        
    if(free_sem == SEM_FAILED || use_sem == SEM_FAILED || write_sem == SEM_FAILED){
        close_buffer(true);
        failed_exit("Couldn't open the semaphones. \n");
    }
}


/**
  * Open Semaphores Generator function
  * @brief Open semaphores for the generators
**/
void open_semaphones_generator(void){
     free_sem = sem_open(SEM_FREE, BUFFER_SIZE);
     use_sem = sem_open(SEM_USE, 0);
     write_sem = sem_open(SEM_WRITE, 1);
       
} 

/**
  * Close Semaphores function
  * @brief Close semaphores for the generators and the supervisor
  * @param isSupervisor different behaviour for the generators and the supervisor
**/
void close_semaphones(bool isSupervisor){
    if(sem_close(write_sem) == -1 || sem_close(free_sem) == -1 || sem_close(use_sem) == -1){
        failed_exit("Couldn't able to close semaphones properly. \n");
    }
    
    if(isSupervisor){
        if(sem_unlink(SEM_WRITE) == -1 || sem_unlink(SEM_FREE) == -1 || sem_unlink(SEM_USE) == -1){
            failed_exit("Couldn't unlink semaphones properly. \n");
        }
    }  
}


void open_buffer(bool isSupervisor){
    shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
   
    if (shmfd == -1){
        close_buffer(isSupervisor);
        failed_exit("Couldn't open share memory. \n");
    }

    if(isSupervisor){
        if(ftruncate(shmfd, sizeof(*buffer)) < 0){
            close_buffer(isSupervisor);
            failed_exit("Ftruncate failed. \n");
        }
    }

   
    buffer = mmap(NULL, sizeof(*buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (buffer == MAP_FAILED){
        close_buffer(isSupervisor);
        failed_exit("mmap faile. \n");
    }

    if(isSupervisor){
        open_semaphones_supervisor();
        set_state(false);
    }
    else{
        open_semaphones_generator();
    }
    
}




void close_buffer(bool isSupervisor){
    if(munmap(buffer, sizeof(*buffer)) == -1){
        failed_exit("Couldn't unmap shared memory properly. \n");
    }

    if(close(shmfd) == -1){
        failed_exit("Couldn't close the shared memory file descriptor properply. \n");
    }

    if(isSupervisor){
        if(shm_unlink(SHM_NAME) == -1){
            failed_exit("Couldn't unlink shared memory. \n");
        }
    }
   
    close_semaphones(isSupervisor);


}

void write_to_buffer(struct Result *result){
    if(sem_wait(write_sem) == -1){
        if(errno == EINTR){
            clean_exit(false);
        }
        else{
            failed_exit("Sem wait for write semaphone failed. \n");
        }
    }

    if(sem_wait(free_sem) == -1){
        if(errno == EINTR){
            clean_exit(false);
        }
        else{
            failed_exit("Sem wait for free semaphone failed. \n");
        }
    }

    buffer->results[buffer->write_index] = *result;
    buffer->write_index += 1;
    buffer->write_index %= BUFFER_SIZE;

    if(sem_post(use_sem) == -1){
        failed_exit("Sem post for use semaphone failed. \n");
    }
    
    if(sem_post(write_sem) == -1){
        failed_exit("Sem post for free semaphone failed. \n");
    }
}

void read_from_buffer(struct Result *result){
    if(sem_wait(use_sem)==-1){
        if(errno == EINTR){
            clean_exit(true);
        }
        else{
           failed_exit("Sem wait for use semaphone failed. \n");
        }
    }


    *result = buffer->results[buffer->read_index];
    buffer->read_index+=1;
    buffer->read_index%=BUFFER_SIZE;
    
    if(sem_post(free_sem) == -1){
        failed_exit("Sem post for free semaphone failed. \n");
    }        
    
}


void print_result(struct Result *result){
    fprintf(stdout, "Soultion with removed %d edges(s): ", result->amount);
    for(int i = 0; i < result->amount; i++){
        fprintf(stdout, "%d-%d ", result->edges[i].n1.value, result->edges[i].n2.value);
    }
    fprintf(stdout, "\n");
    
}

