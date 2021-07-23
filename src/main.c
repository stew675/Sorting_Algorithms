#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

extern void bidir_bubble_sort(void *a, size_t n, size_t es, int (*cmp)());
extern void bishubble_sort(void *a, size_t n, size_t es, int (*cmp)());
extern void bubble_sort(void *a, size_t n, size_t es, int (*cmp)());
extern void qrsort(char *a, size_t n, size_t es, uint32_t (*getkey)(const void *));
extern void rattle_sort(void *a, size_t n, size_t es, int (*cmp)());
extern void nqsort(void *a, size_t n, size_t es, int (*cmp)());
extern void comb_sort(void *a, size_t n, size_t es, int (*cmp)());
extern void shell_sort(void *a, size_t n, size_t es, int (*cmp)());
extern void heap_sort(void *a, size_t n, size_t es, int (*cmp)());

static uint32_t
get_uint32_key(register const void *a)
{
	return *((uint32_t *)a);
} // get_uint32_key

static int
compare_uint32(register const void *p1, register const void *p2)
{
	register const uint32_t *a = (const uint32_t *)p1;
	register const uint32_t *b = (const uint32_t *)p2;

	return (*a == *b) ? 0 : (*a < *b) ? -1 : 1;
} // compar


static void
testsort(register uint32_t a[], register size_t n)
{
	for(register size_t i = 1; i < n; i++)
		if(a[i-1] > a[i]) {
			fprintf(stderr, "Didn't sort data correctly\n");
			exit(-1);
		}
} // testsort


static void
usage(char *prog)
{
	fprintf(stderr, "Usage: %s <-bb|-bu|-co|-gq|-nq|-qr|-ra|-sh> numels\n", prog);
	fprintf(stderr, "\t-bb\tBidirectional Bubble Sort\n");
	fprintf(stderr, "\t-bi\tBidirectional Bubble Shell Sort\n");
	fprintf(stderr, "\t-bu\tBubble Sort\n");
	fprintf(stderr, "\t-co\tComb Sort\n");
	fprintf(stderr, "\t-gq\tGlibc QuickSort\n");
	fprintf(stderr, "\t-hs\tHeap Sort\n");
	fprintf(stderr, "\t-nq\tNew Quick Sort\n");
	fprintf(stderr, "\t-qr\tQuick Radix Sort\n");
	fprintf(stderr, "\t-ra\tRattle Sort\n");
	fprintf(stderr, "\t-sh\tShell Sort\n");
	exit(-1);
} // usage


int
main(int argc, char **argv)
{
	size_t		n;
	uint32_t	*a;
	void		(*sort)() = NULL;
	char		*sortname;

	if(argc != 3) {
		fprintf(stderr, "Incorrect number of arguments\n");
		usage(argv[0]);
	}

	if(strcmp(argv[1], "-gq") == 0) {
		sort = qsort;
		sortname = "GLibC QuickSort";
	} else if(strcmp(argv[1], "-qr") == 0) {
		sort = qrsort;
		sortname = "Quick Radix Sort";
	} else if(strcmp(argv[1], "-ra") == 0) {
		sort = rattle_sort;
		sortname = "Rattle Sort";
	} else if(strcmp(argv[1], "-nq") == 0) {
		sort = nqsort;
		sortname = "New QuickSort";
	} else if(strcmp(argv[1], "-co") == 0) {
		sort = comb_sort;
		sortname = "Comb Sort";
	} else if(strcmp(argv[1], "-sh") == 0) {
		sort = shell_sort;
		sortname = "Shell Sort";
	} else if (strcmp(argv[1], "-bu") == 0) {
		sort = bubble_sort;
		sortname = "Bubble Sort";
	} else if (strcmp(argv[1], "-bi") == 0) {
		sort = bishubble_sort;
		sortname = "Bidirectional Bubble Shell Sort";
	} else if (strcmp(argv[1], "-hs") == 0) {
		sort = heap_sort;
		sortname = "Heap Sort";
	} else if (strcmp(argv[1], "-bb") == 0) {
		sort = bidir_bubble_sort;
		sortname = "Bidirectional Bubble Sort";
	}

	if (sort == NULL) {
		fprintf(stderr, "Unsupported sort type\n");
		usage(argv[0]);
	}

	n = atol(argv[2]);
	if (n < 1) {
		fprintf(stderr, "Please use values of 1 or greater for the number of elements\n");
		exit(-1);
	}

	a = (uint32_t *)aligned_alloc(4096, n * sizeof(*a));
	if(a == NULL) {
		fprintf(stderr, "calloc failed - out of memory\n");
		exit(-1);
	}
	memset(a, 0, n * sizeof(*a));

	printf("Populating array\n");
	srandom(1);
	for(register size_t i = 0; i < n; i++) {
		a[i] = random() % INT32_MAX;
//		a[i] = i + 1;
//		a[i] = n - i;
	}

	printf("Starting %s\n", sortname);
	struct timespec start, end;

	clock_gettime(CLOCK_MONOTONIC, &start);
	if (sort != qrsort) {
		sort(a, n, sizeof(*a), compare_uint32);
	} else {
		sort(a, n, sizeof(*a), get_uint32_key);
	}
	clock_gettime(CLOCK_MONOTONIC, &end);

	double tim = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
	printf(" ");
	printf(" ");
	printf(" ");
	printf(" ");
	printf(" ");
	printf("\nTime taken : %.9fs\n", tim);

	testsort(a, n);

	free(a);
} /* main */
