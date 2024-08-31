include Makefile.inc

SRC = slave.c hello.c
EXE = $(SRC:.c=)

all: $(EXE)

%: %.c
	$(GCC) $(CFLAGS) $< -o $@

clean:
	rm -f $(EXE)

.PHONY: all clean