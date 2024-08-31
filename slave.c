#include <stdio.h>

void processMd5(char * file);

int main(int argc, char * argv[]) { 

    for(int i = 0; i < argc -1; i++) {
        processMd5(argv[i]);
    }

    char * line = NULL;
    size_t len;
    while(getline(&line, &len, stdin) > 0) {
        
    }
}