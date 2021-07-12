ts: qrsort.c main.c shaker_sort.c rattle_sort.c
	$(CC) -O3 -o ts main.c qrsort.c shaker_sort.c rattle_sort.c
