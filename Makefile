# Incluimos el archivo con los flags adicionales
include Makefile.inc

# Definimos las variables
COMPILER = $(GCC)
FLAGS = $(CFLAGS)
FLAGS_VALGRIND = $(CFLAGSFROVALGRINDS)
SRC_APP = app_md5.c
SRC_CHILD = slave.c
SRC_VIEW = view.c

# Objetivos principales
all: app_md5 slave view

# Reglas para compilar cada uno de los binarios
app_md5: $(SRC_APP)
	$(COMPILER) $^ $(FLAGS) -o $@

slave: $(SRC_CHILD)
	$(COMPILER) $^ $(FLAGS) -o $@

view: $(SRC_VIEW)
	$(COMPILER) $^ $(FLAGS) -o $@

# Reglas para Valgrind
checkval: checkval_app_md5 checkval_slave checkval_view

checkval_app_md5: $(SRC_APP)
	$(COMPILER) $^ $(CFLAGSFORVALGRIND) -o app_md5

checkval_slave: $(SRC_CHILD)
	$(COMPILER) $^ $(CFLAGSFORVALGRIND) -o slave

checkval_view: $(SRC_VIEW)
	$(COMPILER) $^ $(CFLAGSFORVALGRIND) -o view

# Limpiar los binarios generados
clean:
	rm -f app_md5 slave view

# Indicamos que estos objetivos no son archivos
.PHONY: all clean app_md5 slave view checkval checkval_app_md5 checkval_slave checkval_view