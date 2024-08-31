#include <errno.h>
#include <stdio.h>
#include <string.h>

void processMd5(char * file);

int main(int argc, char * argv[]) { 

    for(int i = 0; i < argc -1; i++) {
        processMd5(argv[i]);
    }

    char * line = NULL;
    size_t len = 0;
    while((len = getline(&line, &len, stdin)) > 0) {
        processMd5(line);
    }

    //Enviar señal a proceso padre que se libero
}


void processMd5(char * file) {
        char * shellInstruction;
        size_t instructionLen = 0;

        if(file != NULL && (instructionLen = strlen(file)) != 0) {
            
        } else {
            fprintf(stderr, "Empty line");
        }
}