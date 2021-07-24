#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

static uint64_t numcmps = 0, numkeys = 0;
uint64_t numswaps = 0, numcopies = 0;

extern void bidir_bubble_sort(void *a, size_t n, size_t es, int (*cmp)());
extern void bishubble_sort(void *a, size_t n, size_t es, int (*cmp)());
extern void bubble_sort(void *a, size_t n, size_t es, int (*cmp)());
extern void qrsort(char *a, size_t n, size_t es, uint32_t (*getkey)(const void *));
extern void rattle_sort(void *a, size_t n, size_t es, int (*cmp)());
extern void nqsort(void *a, size_t n, size_t es, int (*cmp)());
extern void comb_sort(void *a, size_t n, size_t es, int (*cmp)());
extern void shell_sort(void *a, size_t n, size_t es, int (*cmp)());
extern void heap_sort(void *a, size_t n, size_t es, int (*cmp)());
extern void heap_merge(void *a, size_t n, size_t es, int (*cmp)());
extern void merge_sort(void *a, size_t n, size_t es, int (*cmp)());
extern void smooth_sort(void *a, size_t n, size_t es, int (*cmp)());
extern void GrailSort(uint32_t *arr,int Len);
extern void weak_heap(uint32_t *a, size_t n, size_t es, int (*cmp)());

static uint32_t
get_uint32_key(register const void *a)
{
	numkeys++;
	return *((uint32_t *)a);
} // get_uint32_key

static int
compare_uint32(register const void *p1, register const void *p2)
{
	register const uint32_t *a = (const uint32_t *)p1;
	register const uint32_t *b = (const uint32_t *)p2;

	numcmps++;
	return (*a == *b) ? 0 : (*a < *b) ? -1 : 1;
} // compar


static void
print_array(register uint32_t a[], register size_t n)
{
	printf("%u", a[0]);
	for(register size_t i = 1; i < n; i++)
		printf(", %u", a[i]);
	printf("\n");
} // print_array


static void
test_sort(register uint32_t a[], register size_t n)
{
	for(register size_t i = 1; i < n; i++)
		if(a[i-1] > a[i]) {
			fprintf(stderr, "Didn't sort data correctly\n");
			exit(-1);
		}
} // test_sort


static void
usage(char *prog)
{
	fprintf(stderr, "Usage: %s <-bb|-bu|-co|-gq|-nq|-qr|-ra|-sh> numels\n", prog);
	fprintf(stderr, "\t-bb\tBidirectional Bubble Sort\n");
	fprintf(stderr, "\t-bs\tBidirectional Bubble Shell Sort\n");
	fprintf(stderr, "\t-bu\tBubble Sort\n");
	fprintf(stderr, "\t-co\tComb Sort\n");
	fprintf(stderr, "\t-gq\tGlibc QuickSort\n");
	fprintf(stderr, "\t-gs\tGrail Sort\n");
	fprintf(stderr, "\t-hm\tHeap Merge Sort\n");
	fprintf(stderr, "\t-hs\tHeap Sort\n");
	fprintf(stderr, "\t-me\tMerge Sort\n");
	fprintf(stderr, "\t-nq\tNew Quick Sort\n");
	fprintf(stderr, "\t-qr\tQuick Radix Sort\n");
	fprintf(stderr, "\t-ra\tRattle Sort\n");
	fprintf(stderr, "\t-sh\tShell Sort\n");
	fprintf(stderr, "\t-sm\tSmooth Sort\n");
	fprintf(stderr, "\t-wh\tWeak Heap Sort\n");
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

	if (strcmp(argv[1], "-bb") == 0) {
		sortname = "Bidirectional Bubble Sort";
		sort = bidir_bubble_sort;
	} else if (strcmp(argv[1], "-bs") == 0) {
		sortname = "Bidirectional Bubble Shell Sort";
		sort = bishubble_sort;
	} else if (strcmp(argv[1], "-bu") == 0) {
		sortname = "Bubble Sort";
		sort = bubble_sort;
	} else if(strcmp(argv[1], "-co") == 0) {
		sortname = "Comb Sort";
		sort = comb_sort;
	} else if(strcmp(argv[1], "-gq") == 0) {
		sortname = "GLibC QuickSort";
		sort = qsort;
	} else if(strcmp(argv[1], "-gs") == 0) {
		sortname = "Grail Sort";
		sort = GrailSort;
	} else if(strcmp(argv[1], "-hm") == 0) {
		sortname = "Heap Merge Sort";
		sort = heap_merge;
	} else if (strcmp(argv[1], "-hs") == 0) {
		sortname = "Heap Sort";
		sort = heap_sort;
	} else if(strcmp(argv[1], "-me") == 0) {
		sortname = "Merge Sort";
		sort = merge_sort;
	} else if(strcmp(argv[1], "-nq") == 0) {
		sortname = "New QuickSort";
		sort = nqsort;
	} else if(strcmp(argv[1], "-qr") == 0) {
		sortname = "Quick Radix Sort";
		sort = qrsort;
	} else if(strcmp(argv[1], "-ra") == 0) {
		sortname = "Rattle Sort";
		sort = rattle_sort;
	} else if(strcmp(argv[1], "-sh") == 0) {
		sortname = "Shell Sort";
		sort = shell_sort;
	} else if(strcmp(argv[1], "-sm") == 0) {
		sortname = "Smooth Sort";
		sort = smooth_sort;
	} else if (strcmp(argv[1], "-wh") == 0) {
		sortname = "Weak Heap Sort";
		sort = weak_heap;
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

	printf("\nPopulating array of size: %lu\n\n", n);
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
		if (sort == GrailSort) {
			sort(a, n);
		} else {
			sort(a, n, sizeof(*a), compare_uint32);
		}
	} else {
		sort(a, n, sizeof(*a), get_uint32_key);
	}
	clock_gettime(CLOCK_MONOTONIC, &end);

	test_sort(a, n);

	double tim = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
	printf("\n");
	printf("Time taken to sort: %.9fs\n", tim);
	printf("Number Key Lookups: %lu\n", numkeys);
	printf("Number of Compares: %lu\n", numcmps);
	printf("Number of Swaps   : %lu\n", numswaps);
	printf("Number of Copies  : %lu\n", numcopies);
	printf("\n");

	free(a);
} /* main */
