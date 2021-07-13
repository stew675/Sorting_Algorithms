ts: oldswap.h swap.h qrsort.c shaker_sort.c rattle_sort.c nqsort.c main.c
	$(CC) -O3 -o ts qrsort.c shaker_sort.c rattle_sort.c nqsort.c main.c
