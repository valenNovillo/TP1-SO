#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>       /* For mode constants */
#include <fcntl.h>          /* For O_* constants */
#include <errno.h>
#include <sharedMemory.h>

#define SHM_NAME "/shm"
#define ERROR_VALUE -1

int main(int argc, char * argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);

    createSharedMemory();
    printf("%s\n", SHM_NAME);

    
}

SharedMemory * createSharedMemory() {
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
        exit(errno);
    }
    
    close(shm_fd);
}