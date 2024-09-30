# Compilador
GCC = gcc

# Flags de compilación
CFLAGS = -g -Wall -lrt -pthread -fsanitize=address

# Flags de compilación para correr con valgrind
CFLAGSFORVALGRIND = -g -Wall -lrt -pthread

