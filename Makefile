CC=gcc
INCLUDES=include
CFLAGS=-I$(INCLUDES) -O3
LDFLAGS=-O3
OBJDIR=obj

_DEPS = swap.h oldswap.h
DEPS = $(patsubst %,$(INCLUDES)/%,$(_DEPS))

_OBJ = qrsort.o nqsort.o rattle_sort.o main.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

ts: $(OBJ)
	$(CC) $(LDLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
