include Makefile.inc

SRC = slave.c app_md5.c view.c
EXE = $(SRC:.c=)

all: $(EXE)

%: %.c
	$(GCC) $(CFLAGS) $< -o $@

checkval: $(EXE)

%: %.c
	$(GCC) $(CFLAGSFROVALGRINDS) $< -o $@

clean:
	rm -f $(EXE)

.PHONY: all clean