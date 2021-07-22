CC= gcc

OPT_FLAGS= -O3

DEBUG_FLAGS =

INCLUDES= include

CFLAGS= -I$(INCLUDES) $(DEBUG_FLAGS) $(OPT_FLAGS)
LDFLAGS= $(DEBUG_FLAGS) $(OPT_FLAGS)

_DEPS= swap.h oldswap.h
DEPS= Makefile $(patsubst %,$(INCLUDES)/%,$(_DEPS))

SRCDIR= src

OBJDIR= obj
_OBJ= rattle_sort.o bidir_bubble.o bubble_sort.o comb_sort.o nqsort.o qrsort.o shell_sort.o main.o
OBJ= $(patsubst %,$(OBJDIR)/%,$(_OBJ))

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

ts: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCLUDES)/*~
