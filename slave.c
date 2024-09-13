#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h> 

#define COMMAND_CHARS 7 //Cantidad de caracteres para el comando de la shell
#define MD5 32

void processMd5(const char * file);

int main(int argc, char * argv[]) { 
    setvbuf(stdout,NULL,_IONBF,0);

    char * line = NULL;
    size_t len = 0;

   while(getline(&line, &len, stdin) > 0) {
        line[strcspn(line, "\n")] = '\0';

        char *token = strtok(line, " ");
        while (token != NULL) {
            processMd5(token);
            token = strtok(NULL, " ");
        }
   }

    free(line);

    return 0;
}


void processMd5(const char * file) {
        char * shellInstruction;
        size_t instructionLen = 0;

        if(file != NULL && (instructionLen = strlen(file)) != 0) {
            shellInstruction = calloc(instructionLen + COMMAND_CHARS + 1, sizeof(char));

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

                printf("File: %.*s - MD5: %.*s - PID: %d\n", (int)instructionLen, file, MD5, readLine, pid);                            

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