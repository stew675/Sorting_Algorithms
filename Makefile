CC=gcc

OPT_FLAGS=-O3

DEBUG_FLAGS=

INCLUDES=include

CFLAGS=-I$(INCLUDES) $(DEBUG_FLAGS) $(OPT_FLAGS)
LDFLAGS=$(DEBUG_FLAGS) $(OPT_FLAGS)

_DEPS = swap.h oldswap.h
DEPS = Makefile $(patsubst %,$(INCLUDES)/%,$(_DEPS))

OBJDIR=obj
_OBJ = qrsort.o nqsort.o comb_sort.o shell_sort.o rattle_sort.o main.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

ts: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCLUDES)/*~
