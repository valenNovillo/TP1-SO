// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is a personal academic project. Dear PVS-Studio, please check it.
#include <sys/time.h>
#include "sharedMemory.h"
#include "include.h"

#define MAX_LENGTH_NAME 50

// PROTOTYPES ------------------------------------------------------------------------------------------------------------------------------------------
SharedMemory * open_shared_memory_and_sem(char * shm_name);
// ---------------------------------------------------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0); 
    char shm_name[MAX_LENGTH_NAME] = {0};

    if(argc > 1) {
        strncpy(shm_name, argv[1], MAX_LENGTH_NAME - 1);
        shm_name[MAX_LENGTH_NAME - 1] = '\0';
    } else {
        if(read(STDIN_FILENO, &shm_name, MAX_LENGTH_NAME) == ERROR_VALUE) {
            perror("Failed reading shm name\n");
            exit(errno);
        }
    }

    SharedMemory * shm = open_shared_memory_and_sem(shm_name);

    int charsRead = 0;
    char * line_end;

    while((shm->total_files) > 0) {
        sem_wait(&(shm->available_files));
        line_end = strchr(shm->buf + charsRead, '\n');
        if (line_end == NULL) {
            perror("Error during buffer parsing");
            exit(ERROR_VALUE);
        }
        charsRead += printf("%.*s\n", (line_end - (shm->buf + charsRead)),shm->buf + charsRead);
        (shm->total_files)--;
    }

    if(munmap(shm, sizeof(SharedMemory)) == ERROR_VALUE) {
        perror("Error unmapping shared memory\n");
        exit(errno);
    }
    return 0;    
}

SharedMemory * open_shared_memory_and_sem(char * shm_name) {
    SharedMemory * shm;
    int shm_fd;

    if((shm_fd = shm_open(shm_name, O_RDWR, 0644)) == ERROR_VALUE) {
        perror("Error opening share memory\n");
        exit(errno);
    }

    if((shm = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)) == MAP_FAILED) {
        perror("Error mapping shared memory\n");
        exit(errno);
    }

    if(close(shm_fd) == ERROR_VALUE) {
        perror("Error closing fd of shared memory\n");
        exit(errno);
    }
    return shm;
}


