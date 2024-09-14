# TP1-SO

This project uses various inter process communications to calculate the md5sum of several files by delegating the tasks to child processes and reading their outputs 

## Compiling instructions
  **make all**

  **make checkval** to run the programs with valgrind this compiles the programs without the -fsanitize=address flag

  **make clean** to delete the object files

## Running instructions
  ###slave.c
  ./slave
  reads pathfiles from stdin up to an EOF and prints their 
  
  ### app_md5.c
  expects at least one valid path to a file as a command line argument
  ./app_md5 file1 file2 ...
  it outputs the results to a file named *resultados.txt*

  ### view.c
  expects a valid name of an existing shared memory as a command line argument, to use the one printed by app_md5 it needs to be done before its 2 seconds sleep ends
  ./view /shm_name

  ### piping app_md5.c and view.c
  is not necessary to give a command line arguement to view.c in this format
  ./app_md5 file1 file2 ... | ./view
