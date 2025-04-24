CC := gcc
LD := $(CC)

CFLAGS := -ansi -Wall -Wextra -Wwrite-strings -Wno-variadic-macros -O1 -Iinclude/ -g -D_PERF # -D_DEBUG

LDFLAGS :=

CSRC := src/main.c src/solver/cnf.c src/solver/solve.c src/parser.c
COBJ := $(CSRC:.c=.o)

all: hal
hal: $(COBJ)
	$(LD) -o $@ $^

clean:
	rm -f hal
	rm -f $(COBJ)

.PHONY: clean all
