#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>       /* For mode constants */
#include <fcntl.h>          /* For O_* constants */
#include <errno.h>
#include <sys/select.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "sharedMemory.h"

#define SHM_NAME "/shm"
#define ERROR_VALUE -1
#define SEM_NAME "/available_files"
#define ALL_READ_WRITE 00666
#define MAX_SLAVES 10
#define PERCENTAGE_FILES_PER_SLAVES 0.1
#define WAIT_VIEW 2
#define PIPE_BORDERS 2
#define READ 0
#define WRITE 1
#define BUFF_LEN 5000

int clean_pipes(int slave_count,int (*pipe_slave_app_fd)[PIPE_BORDERS], int (*pipe_app_slave_fd)[PIPE_BORDERS]);
void super_exit(int slave_count,int (*pipe_slave_app_fd)[PIPE_BORDERS], int (*pipe_app_slave_fd)[PIPE_BORDERS], SharedMemory * shm, FILE* resultsText);
int get_max_fd(fd_set * file_descriptors, int (*pipe_slave_app_fd)[PIPE_BORDERS], const int slave_count);
SharedMemory * create_shared_memory(int total_files);
int destroy_shared_memory_and_sem(SharedMemory * shm);
int get_answer(int fd, char * answer);
int initial_distribution(char * argv[], int * pending_processes, int slave_count, int (*pipe_app_slave_fd)[PIPE_BORDERS]);
int new_baby_slaves(const int slave_count, int (*pipe_slave_app_fd)[PIPE_BORDERS], int (*pipe_app_slave_fd)[PIPE_BORDERS], int * children_pids);
void print_conection_info(const char * bufName, const int time);


int main(int argc, char * argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);

    if(argc == 1){
        perror("There must be at least one file to process");
        return 1;
    }

    int pending_processes = argc - 1;
    int slave_count = (pending_processes < MAX_SLAVES) ? pending_processes : MAX_SLAVES;

    FILE* resultsText = fopen("resultados.txt", "w");
    if(resultsText == NULL){
        perror("Error opnening file results.txt\n");
        exit(errno);
    }

    SharedMemory * shm;
    if((shm = create_shared_memory(pending_processes)) == NULL) {
        fclose(resultsText);
        exit(errno);
    }
    
    print_conection_info(SHM_NAME, WAIT_VIEW);

    int children_pids[slave_count];
    int pipe_slave_app_fd[slave_count][PIPE_BORDERS];
    int pipe_app_slave_fd[slave_count][PIPE_BORDERS];

    if (initial_distribution(argv, &pending_processes, slave_count, pipe_app_slave_fd) == ERROR_VALUE) {
        super_exit(slave_count, pipe_slave_app_fd, pipe_app_slave_fd, shm, resultsText);
    }
    
    if(new_baby_slaves(slave_count, pipe_slave_app_fd, pipe_app_slave_fd, children_pids) == ERROR_VALUE) {
        super_exit(slave_count, pipe_slave_app_fd, pipe_app_slave_fd, shm, resultsText);
    }

    int ready_to_read;
    fd_set read_fds;
    int offset_buf_shm = 0;
    int sended = 0;
    
    while(sended < (argc - 1))
    {
        int max_fd = get_max_fd(&read_fds, pipe_slave_app_fd, slave_count);
        ready_to_read = select(max_fd, &read_fds,NULL, NULL, NULL);

        for (int i = 0; i < slave_count && ready_to_read > 0; i++)
        {
            if (FD_ISSET(pipe_slave_app_fd[i][READ], &read_fds)) {
                
                char answer[MAX_BUF];
                int char_read = get_answer(pipe_slave_app_fd[i][READ], answer);
                if(char_read == ERROR_VALUE) {
                   super_exit(slave_count, pipe_slave_app_fd, pipe_app_slave_fd, shm, resultsText);
                } 
                ready_to_read--;

                answer[char_read] = '\0';
                char * token = strtok(answer, "\n");
                
                while(token != NULL) {
                    fprintf(resultsText, "%s\n", token);
                    offset_buf_shm +=  snprintf((shm->buf) + offset_buf_shm, MAX_BUF, "%s", token);
                    sem_post(shm->available_files);
                    sended++;
                    token = strtok(NULL, "\n");
                }

                if(pending_processes > 0) {
                    dprintf(pipe_app_slave_fd[i][WRITE], "%s\n", argv[argc - pending_processes]);
                    pending_processes--;
                }
            }
        }   
    }

    if(destroy_shared_memory_and_sem(shm) == ERROR_VALUE) {
        fclose(resultsText);
        exit(errno);
    }

    fclose(resultsText);

}

void super_exit(int slave_count,int (*pipe_slave_app_fd)[PIPE_BORDERS], int (*pipe_app_slave_fd)[PIPE_BORDERS], SharedMemory * shm, FILE* resultsText)
{
    clean_pipes(slave_count, pipe_slave_app_fd, pipe_app_slave_fd);
    destroy_shared_memory_and_sem(shm);
    fclose(resultsText);
    exit(errno); 
}

int clean_pipes(int slave_count,int (*pipe_slave_app_fd)[PIPE_BORDERS], int (*pipe_app_slave_fd)[PIPE_BORDERS])
{
    int i = 0;
    while(i < slave_count){
        if(close(pipe_app_slave_fd[i][WRITE]) == ERROR_VALUE){
            perror("Error closing one of the pipe_app_slave fd\n");
            return ERROR_VALUE;
        }

         if(close(pipe_slave_app_fd[i][READ]) == ERROR_VALUE){
            perror("Error closing one of the pipe_app_slave fd\n");
            return ERROR_VALUE;
        }
        
        i++;
    }

    return i;
}

/*En esta funcion ademas de obtener el maximo fd, se completa el vector que recibe por parametro con TODOS los fd*/
int get_max_fd(fd_set * file_descriptors, int (*pipe_slave_app_fd)[PIPE_BORDERS], const int slave_count){
    FD_ZERO(file_descriptors);
    int max_fd = -1;
    for(int i = 0; i < slave_count; i++)
    {
        FD_SET(pipe_slave_app_fd[i][READ], file_descriptors);
        if(pipe_slave_app_fd[i][READ] > max_fd){
            max_fd = pipe_slave_app_fd[i][READ];
        }
    }

    return max_fd; 
}


int initial_distribution(char * argv[], int * pending_processes, int slave_count, int (*pipe_app_slave_fd)[PIPE_BORDERS]) {
    int proportion_files_per_slave = (PERCENTAGE_FILES_PER_SLAVES * (*pending_processes) / slave_count);
    int files_per_slaves = (proportion_files_per_slave < 1) ? 1 : proportion_files_per_slave;

    for (int i = 0; i < slave_count; i++) {
        char path_files[BUFF_LEN];
        path_files[0] = '\0';
        for(int j = i * files_per_slaves + 1; j <= i * files_per_slaves + files_per_slaves; j++){
            strcat(path_files, argv[j]);
            if (j < i * files_per_slaves + files_per_slaves - 1)
                strcat(path_files, " ");
        }
        if (dprintf(pipe_app_slave_fd[i][WRITE], "%s\n", path_files) < 0) {
            perror("Error printing throw pipe");
            return ERROR_VALUE;
        }
    }
    
    *pending_processes = *pending_processes - files_per_slaves * slave_count;
    return 0;
}

int new_baby_slaves(const int slave_count, int (*pipe_slave_app_fd)[PIPE_BORDERS], int (*pipe_app_slave_fd)[PIPE_BORDERS], int * children_pids) {
    for(int i=0; i < slave_count; i++) {
        if (pipe(pipe_app_slave_fd[i]) == ERROR_VALUE || pipe(pipe_slave_app_fd[i]) == ERROR_VALUE) {
            perror("error while creating pipe");
            return ERROR_VALUE;
        }

        if((children_pids[i] = fork()) == ERROR_VALUE ) {
            perror("error forking new baby slave\n");
            return ERROR_VALUE;
        } else if(children_pids[i] == 0) {
            if(dup2(pipe_app_slave_fd[i][READ], STDIN_FILENO) == ERROR_VALUE|| dup2(pipe_slave_app_fd[i][WRITE], STDOUT_FILENO) == ERROR_VALUE) {
                perror("problem setting fd\n");
                return ERROR_VALUE;
            }
            
            if (close(pipe_app_slave_fd[i][READ]) == ERROR_VALUE || close(pipe_app_slave_fd[i][WRITE] == ERROR_VALUE)
                || close(pipe_slave_app_fd[i][READ]) == ERROR_VALUE || close(pipe_slave_app_fd[i][WRITE]) == ERROR_VALUE) {
                perror("problem closing pipes\n");
                return ERROR_VALUE;
            }

            execve("/slave.c", NULL, NULL);
        }

        if( close(pipe_app_slave_fd[i][READ]) == ERROR_VALUE || close(pipe_slave_app_fd[i][WRITE]) == ERROR_VALUE ) { 
            perror("problem closing border pipe\n");
            return ERROR_VALUE;
        }//OJO!Todavía nos van a faltar cerrar el extremo de escritura del pipe_app_slave_fd y el de lectura del pipe_slave_app_fd (esto lo hacemos al final de nuestro main pues a estos antes los tenemos que utilizaer para que se puedan leer y escribir los resultados creo)
    }
    return 0;
}

void print_conection_info(const char * bufName, const int time) {
    printf("%s\n", bufName);
    sleep(time);
}


SharedMemory * create_shared_memory(int total_files) {
    shm_unlink(SHM_NAME);
       
    int shm_fd;
    shm_fd = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_TRUNC, S_IWUSR);
    
    if(shm_fd == ERROR_VALUE) {
        perror("problem creating a shared memory\n");
        return NULL;
    }
    
    if(ftruncate(shm_fd, sizeof(SharedMemory)) == ERROR_VALUE) {
        perror("problem sizing the memory\n");
        return NULL;
    }

    SharedMemory * shm = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(shm == MAP_FAILED) {
        perror("problem mappping to shared memory\n");
        return NULL;
    }

    sem_unlink(SEM_NAME);
    shm->available_files = sem_open(SEM_NAME, O_CREAT, ALL_READ_WRITE, 0);
    if(shm->available_files == SEM_FAILED) {
        perror("problem creating a semaphore\n");
        return NULL;
    }

    if(close(shm_fd) == ERROR_VALUE) {
        perror("Error closing fd of shared memory\n");
        return NULL;
    }

    shm->total_files = total_files;
    strcpy(shm->sem_path, SEM_NAME);

    return shm;
}

int destroy_shared_memory_and_sem(SharedMemory * shm) {

    if(munmap(shm, sizeof(SharedMemory)) == ERROR_VALUE){
        perror("Error unmapping shared memory\n");
        return ERROR_VALUE;
    }

    if(shm_unlink(SHM_NAME) == ERROR_VALUE){
        perror("Error unlinking Share Memory");
        return ERROR_VALUE;
    }

    if(sem_unlink(SEM_NAME)){
        perror("Error unlinking semaphore");
        return ERROR_VALUE;
    }

}


int get_answer(int fd, char * answer) {
    if(dup2(fd, STDIN_FILENO) != 0){
        perror("Error copying file descriptor\n");
        return ERROR_VALUE;
    }

    int char_read = read(STDIN_FILENO, answer, MAX_BUF);
    if(char_read == ERROR_VALUE){
        perror("Error reading line\n");
        printf("%d",errno);
        return ERROR_VALUE;
    }
    return char_read;
}



