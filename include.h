#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>      /* For O_* constants */
#include <errno.h> 
#include <sys/stat.h>   /* For mode constants */  
#include <sys/wait.h> 
#include <signal.h>

#define ERROR_VALUE -1

