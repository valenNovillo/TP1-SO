#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

//Cantidad de caracteres para el comando de la shell
#define BLOCK 7

void processMd5(const char * file);

int main(int argc, char * argv[]) { 

    for(int i = 0; i < argc -1; i++) {
        processMd5(argv[i]);
    }

    char * line = NULL;
    size_t len = 0;
    
    while((len = getline(&line, &len, stdin)) > 0) {
        processMd5(line);
        free(line);
        line = NULL; //Pues free no modifica la dirección del puntero line
        
    }

    return 0;
    //TODO Enviar señal a proceso padre que se libero
}


void processMd5(const char * file) {
        char * shellInstruction;
        size_t instructionLen = 0;

        if(file != NULL && (instructionLen = strlen(file)) != 0) {
            shellInstruction = calloc(instructionLen + BLOCK + 1, sizeof(char));

            if(shellInstruction != NULL)
            {
                strcat(shellInstruction, "md5sum ");
                strcat(shellInstruction, file);
                FILE *stream = popen(shellInstruction, "r");

                if(stream == NULL)
                {
                    perror("Error al generar el md5sum\n");
                    free(shellInstruction);
                    return;
                }

                char *readLine = NULL;
                size_t rta_len = 0;

                if(getline(&readLine, &rta_len, stream) < 0)
                {
                    perror("Error leyendo la rta de md5sum\n");
                    free(shellInstruction);
                    return;
                }

                pid_t pid =  getpid();

                printf("File: %.*s - MD5: %.*s - PID: %d\n", (int)instructionLen, file, 32, readLine, pid);                            

                free(readLine);
                pclose(stream);
            }
            free(shellInstruction);
        } 
        else {
            perror("Empty line");
            return;
        }
}