#include  <semaphore.h>

#define SHM_NAME "/shm"
#define MAX_BUF 500000

typedef struct {
    char buf[MAX_BUF];
    int total_files;
    sem_t available_files;
} SharedMemory;