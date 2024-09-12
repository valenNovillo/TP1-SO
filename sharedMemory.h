#include  <semaphore.h>

#define MAX_BUF 500000
#define LEN_NAME_SEM 17

typedef struct {
    char buf[MAX_BUF];
    int total_files;
    sem_t * available_files;
    char sem_path[LEN_NAME_SEM];
} SharedMemory;