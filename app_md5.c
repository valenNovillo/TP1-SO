#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>       /* For mode constants */
#include <fcntl.h>          /* For O_* constants */
#include <errno.h>
#include <sharedMemory.h>

#define SHM_NAME "/shm"
#define ERROR_VALUE -1
#define SEM_NAME "/available_files"
#define ALL_READ_WRITE 00666


int main(int argc, char * argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);

    SharedMemory * shm = create_shared_memory();
    printf("%s\n", SHM_NAME);
}

SharedMemory * create_shared_memory() {
    int shm_fd;
    shm_fd = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_TRUNC, S_IWUSR);
    
    if(shm_fd == ERROR_VALUE) {
        perror("problem creating a shared memory\n");
        exit(errno);
    }
    
    if(ftrucate(shm_fd, sizeof(SharedMemory)) == ERROR_VALUE) {
        perror("problem sizing the memory\n");
        exit(errno);
    }

    SharedMemory * shm = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(shm == MAP_FAILED) {
        perror("problem mappping to shared memory\n");
        exit(errno);
    }

    shm->available_files = sem_open(SEM_NAME, O_CREAT, ALL_READ_WRITE, 0);
    if(shm->available_files == SEM_FAILED) {
        perror("problem creating a semaphore\n");
        exit(errno);
    }

    close(shm_fd);
    return shm;
}