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

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0); 
    char shm_name[MAX_LENGTH_NAME] = {0};

    if(argc > 1)
    {
        strcpy(shm_name, argv[1]);
    }
    else
    {
        if(read(STDIN_FILENO, &shm_name, MAX_LENGTH_NAME) == -1)
        {
            perror("Failed reading shm name\n");
            exit(errno);
        }
    }

    SharedMemory *shm_p;

    int shm_fd;
    if((shm_fd = shm_open(shm_name, O_RDONLY | O_TRUNC, S_IRUSR)) == -1)
    {
        perror("Error openning share memory\n");
        exit(errno);
    }
    if((shm_p = mmap(NULL, sizeof(SharedMemory), PROT_READ, MAP_SHARED, shm_fd, 0)) == MAP_FAILED)
    {
        perror("Error mapping shared memory\n");
        exit(errno);
    }

    if(close(shm_fd) == -1)
    {
        perror("Error closing shared memory\n");
        exit(errno);
    }

    int line_length;
    int charsRead = 0;

    while((shm_p->total_files) > 0)
    {
        sem_wait(&(shm_p->available_files));
        line_length = printf("%s", shm_p->buf + charsRead);
        (shm_p->total_files)--;
        charsRead += line_length + 1;
    }

    if(sem_destroy(&(shm_p->available_files)) == -1)
    {
        perror("Error destroying semaphore\n");
        exit(errno);
    }

    if(munmap(shm_p, sizeof(SharedMemory)) == -1)
    {
        perror("Error unmapping shared memory\n");
        exit(errno);
    }

    return 0;    


}




