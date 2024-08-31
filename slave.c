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
        printf("%s\n", line);
        free(line);
        line = NULL; //ues free no modifica la dirección del puntero line
        
    }


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
                FILE *stream = poepen(shellInstruction, 'r');

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
                char buf_pid[30];
                if(sprintf(buf_pid, "Slave ID: %d -> ", pid) <= 0){//Formatea el PID del proceso en una cadena con un mensaje Slave ID: ... -> .
                perror("Failed in sprintf");//Muestra un mensaje de error si sprintf falla.
                free(shellInstruction);//Libera la memoria de command.
                return;
            }
            printf("%s %s\t", buf_pid, readLine);
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