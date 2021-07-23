######################################################################################
# DEP = all the objects that we want watched for changes, which will trigger a rebuild
# SRC = all source objects we want included in the final executable
######################################################################################

DEP=	swap.h oldswap.h
SRC=	bidir_bubble.c \
	bishubble.c \
	bubble_sort.c \
	comb_sort.c \
	heap_sort.c \
	main.c \
	nqsort.c \
	qrsort.c \
	rattle_sort.c \
	merge_sort.c \
	smooth_sort.c \
	shell_sort.c

INCDIR= include
SRCDIR= src
OBJDIR= obj

######################################################################################
# What we want the final executable to be called
######################################################################################

BIN=ts

######################################################################################
# COMPILE TIME OPTION FLAGS
######################################################################################

CC= gcc
OPT_FLAGS= -O2 -march=native -mtune=native
DEBUG_FLAGS= 
LIBS=

######################################################################################
# The rules to make it all work.  Should rarely need to edit anything below this line
######################################################################################

CFLAGS= -I$(INCDIR) $(DEBUG_FLAGS) $(OPT_FLAGS)
LDFLAGS= $(DEBUG_FLAGS) $(OPT_FLAGS)

DEPS= $(patsubst %,$(INCDIR)/%,$(DEP)) Makefile

_OBJ=$(SRC:.c=.o)
OBJ= $(patsubst %,$(OBJDIR)/%,$(_OBJ))

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS) | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJDIR):
	mkdir -p $@

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o $(SRCDIR)/*~ core $(INCDIR)/*~ $(BIN)
	(test -d $(OBJDIR) && rmdir $(OBJDIR)) || true
