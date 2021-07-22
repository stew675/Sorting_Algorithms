#####################################################################################
# All our dependent objects we want built/watched for changes
#####################################################################################

_DEPS=	swap.h oldswap.h
_OBJ=	rattle_sort.o bidir_bubble.o bubble_sort.o comb_sort.o \
	nqsort.o qrsort.o shell_sort.o main.o
INCDIR= include
SRCDIR= src
OBJDIR= obj

#####################################################################################
# What we want the final executable to be called
#####################################################################################

BIN=ts

#####################################################################################
# COMPILE TIME OPTION FLAGS
#####################################################################################

CC= gcc
OPT_FLAGS= -O3
DEBUG_FLAGS=
LIBS=

#####################################################################################
# The rules to make it all work.  Should rarely need to edit anything below this line
#####################################################################################

CFLAGS= -I$(INCDIR) $(DEBUG_FLAGS) $(OPT_FLAGS)
LDFLAGS= $(DEBUG_FLAGS) $(OPT_FLAGS)

DEPS= $(patsubst %,$(INCDIR)/%,$(_DEPS))

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
	rmdir $(OBJDIR)
