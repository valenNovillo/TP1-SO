#include  <semaphore.h>

#define MAX_BUF 500000

typedef struct {
    char buf[MAX_BUF];
    int pendingFiles;
    sem_t sem;
} SharedMemory;