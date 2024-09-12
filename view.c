#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/stat.h>
#include <signal.h>
#include "sharedMemory.h"
#define MAX_LENGTH_NAME 50
#define ERROR_VALUE -1

SharedMemory * open_shared_memory_and_sem(char * shm_name, SharedMemory * shm);

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0); 
    char shm_name[MAX_LENGTH_NAME] = {0};

    if(argc > 1) {
        strcpy(shm_name, argv[1]);
    } else {
        if(read(STDIN_FILENO, &shm_name, MAX_LENGTH_NAME) == ERROR_VALUE)
        {
            perror("Failed reading shm name\n");
            exit(errno);
        }
    }

    SharedMemory * shm = open_shared_memory_and_sem(shm_name, shm);

    int line_length;
    int charsRead = 0;

    while((shm->total_files) > 0)
    {
        sem_wait(&(shm->available_files));
        line_length = printf("%s", shm->buf + charsRead);
        (shm->total_files)--;
        charsRead += line_length + 1;
    }

    if(sem_close(&(shm->available_files)) == ERROR_VALUE){
        perror("Error destroying semaphore\n");
        exit(errno);
    }

    if(munmap(shm, sizeof(SharedMemory)) == ERROR_VALUE){
        perror("Error unmapping shared memory\n");
        exit(errno);
    }

    return 0;    

}

SharedMemory * open_shared_memory_and_sem(char * shm_name, SharedMemory * shm) {
    int shm_fd;
    if((shm_fd = shm_open(shm_name, O_RDONLY | O_TRUNC, S_IRUSR)) == ERROR_VALUE) {
        perror("Error openning share memory\n");
        exit(errno);
    }

    if((shm = mmap(NULL, sizeof(SharedMemory), PROT_READ, MAP_SHARED, shm_fd, 0)) == MAP_FAILED) {
        perror("Error mapping shared memory\n");
        exit(errno);
    }

    shm->available_files = sem_open(shm->sem_path, 0);
    
    if(shm->available_files == SEM_FAILED) {
        perror("problem opening a semaphore\n");
        exit(errno);
    }

    if(close(shm_fd) == ERROR_VALUE) {
        perror("Error closing fd of shared memory\n");
        exit(errno);
    }

    return shm;
}


