CC := gcc
LD := $(CC)

CFLAGS := -ansi -Wall -Wextra -Wwrite-strings -Wno-variadic-macros -O1 -Iinclude/
LDFLAGS :=

CSRC := src/main.c src/solver/cnf.c src/solver/solve.c
COBJ := $(CSRC:.c=.o)

all: hal
hal: $(COBJ)
	$(LD) -o $@ $^

clean:
	rm -f hal
	rm -f $(COBJ)

.PHONY: clean all
