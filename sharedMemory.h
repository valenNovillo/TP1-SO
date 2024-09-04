#define MAX_BUF 500000

typedef struct 
{
    char buf[MAX_BUF];
    sem_t ready_to_read;
    sem_t ready_to_write;
    sem_t mutex;
}SharedMemory