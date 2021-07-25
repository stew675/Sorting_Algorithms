#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

#define DATA_SET_REVERSED	0x04
#define DATA_SET_UNIQUE		0x08

static int disorder_factor = 100;
static int data_set_ops = 0;
static uint32_t data_set_limit = UINT32_MAX;
static unsigned int random_seed = 1;
static bool verbose = false;

static uint64_t numcmps = 0, numkeys = 0;
uint64_t numswaps = 0, numcopies = 0;

// Declarations of all the sort functions we support
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


// Used by functions that just want a key itself (eg. for radix sorts)
static uint32_t
get_uint32_key(register const void *a)
{
	numkeys++;
	return *((uint32_t *)a);
} // get_uint32_key


// Used to compare two uint32_t values pointed at by the pointers given
static int
compare_uint32(register const void *p1, register const void *p2)
{
	register const uint32_t *a = (const uint32_t *)p1;
	register const uint32_t *b = (const uint32_t *)p2;

	numcmps++;
	return (*a == *b) ? 0 : (*a < *b) ? -1 : 1;
} // compare_uint32


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
	fprintf(stderr, "\nUsage: %s [options] <sort_type> numels\n", prog);
	fprintf(stderr, "\n[options] is zero or more of the following options\n");
	fprintf(stderr, "\t-a seed     A random number generator seed value to use (default=1)\n");
	fprintf(stderr, "\t            A value of 0 will use a randomly generated seed\n");
	fprintf(stderr, "\t-d <0..100> Disorder the generated set by the percentage given (default=100)\n");
	fprintf(stderr, "\t-f          Data set keys/values range from 0..UINT32_MAX (default)\n");
	fprintf(stderr, "\t-l num      Data set keys/values limited in range from 0..num\n");
	fprintf(stderr, "\t-o          Use a fully ordered data set (sets disorder factor to 0)\n");
	fprintf(stderr, "\t-r          Reverse the data set order after generating it\n");
	fprintf(stderr, "\t-u          Data set keys/values must all be unique\n");
	fprintf(stderr, "\t-v          Verbose.  Display the data set after generating it\n");
	fprintf(stderr, "\n<sort_type> is one only of the following options\n");
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


void
(*parse_sort_type(char *opt, char **sortname))()
{
	if (strcmp(opt, "-bb") == 0) {
		*sortname = "Bidirectional Bubble Sort";
		return bidir_bubble_sort;
	}

	if (strcmp(opt, "-bs") == 0) {
		*sortname = "Bidirectional Bubble Shell Sort";
		return bishubble_sort;
	}

	if (strcmp(opt, "-bu") == 0) {
		*sortname = "Bubble Sort";
		return bubble_sort;
	}

	if(strcmp(opt, "-co") == 0) {
		*sortname = "Comb Sort";
		return comb_sort;
	}

	if(strcmp(opt, "-gq") == 0) {
		*sortname = "GLibC QuickSort";
		return qsort;
	}

	if(strcmp(opt, "-gs") == 0) {
		*sortname = "Grail Sort";
		return GrailSort;
	}

	if(strcmp(opt, "-hm") == 0) {
		*sortname = "Heap Merge Sort";
		return heap_merge;
	}

	if (strcmp(opt, "-hs") == 0) {
		*sortname = "Heap Sort";
		return heap_sort;
	}

	if(strcmp(opt, "-me") == 0) {
		*sortname = "Merge Sort";
		return merge_sort;
	}

	if(strcmp(opt, "-nq") == 0) {
		*sortname = "New QuickSort";
		return nqsort;
	}

	if(strcmp(opt, "-qr") == 0) {
		*sortname = "Quick Radix Sort";
		return qrsort;
	}

	if(strcmp(opt, "-ra") == 0) {
		*sortname = "Rattle Sort";
		return rattle_sort;
	}

	if(strcmp(opt, "-sh") == 0) {
		*sortname = "Shell Sort";
		return shell_sort;
	}

	if(strcmp(opt, "-sm") == 0) {
		*sortname = "Smooth Sort";
		return smooth_sort;
	}

	if (strcmp(opt, "-wh") == 0) {
		*sortname = "Weak Heap Sort";
		return weak_heap;
	}

	return NULL;
} // parse_sort_type


static int
parse_control_opt(char *argv[])
{
	// The srandom seed value
	if (!strcmp(argv[0], "-a")) {
		random_seed = atoi(argv[1]);
		return 2;	// We grabbed 2 options
	}

	if (!strcmp(argv[0], "-d")) {
		int factor;

		factor = atoi(argv[1]);
		if (factor < 0 || factor > 100) {
			fprintf(stderr, "Please specify a disorder factor between [0..100] (inclusive)\n");
			return 2;
		}
		disorder_factor = factor;
		return 2;
	}
	if (!strcmp(argv[0], "-f")) {
		data_set_limit = UINT32_MAX;
		return 1;
	}
	if (!strcmp(argv[0], "-l")) {
		size_t limit;
		limit = atol(argv[1]);
		if (limit > UINT32_MAX)
			limit = UINT32_MAX;
		if (limit == 0) {
			fprintf(stderr, "Bad value specified for the key/value limit\n");
			return 2;
		}
		data_set_limit = limit;
		return 2;	// We grabbed 2 options
	}
	if (!strcmp(argv[0], "-o")) {
		disorder_factor = 3;
		return 1;
	}
	if (!strcmp(argv[0], "-r")) {
		data_set_ops |= DATA_SET_REVERSED;
		return 1;
	}
	if (!strcmp(argv[0], "-u")) {
		data_set_ops |= DATA_SET_UNIQUE;
		return 1;
	}
	if (!strcmp(argv[0], "-v")) {
		verbose = true;
		return 1;
	}
} // parse_control_opt


void
reverse_set(uint32_t *a, size_t n)
{
	uint32_t t;

	for (size_t i = 0; i < (n / 2); i++) {
		t = a[(n - i) - 1];
		a[(n - i) - 1] = a[i];
		a[i] = t;
	}
} // reverse_set


// Perform a disorder on the set
void
disorder_pass(uint32_t *a, size_t n)
{
	for (size_t i = 0; i < (n - 1); i++) {
		size_t target, range;
		uint32_t t;

		// Determine if we will disorder this element
		if ((random() % 100) >= ((disorder_factor + 1) / 2))
			continue;

		// Establish the range will be for the disorder
		range = n - i;
		range *= disorder_factor;
		range /= 100;
		range += 1;
		do {
			target = i + (random() % range) + 1;
		} while (target >= n);

		// We have our target. Swap ourselves with that
		t = a[i];
		a[i] = a[target];
		a[target] = t;
	}
} // disorder_pass


void
disorder_set(uint32_t *a, size_t n)
{
	if (disorder_factor <= 0)
		return;
	if (disorder_factor > 100)
		disorder_factor = 100;

	disorder_pass(a, n);
	reverse_set(a, n);
	disorder_pass(a, n);
	reverse_set(a, n);
} // disorder_set


uint32_t
get_next_val(uint32_t val, size_t pos, size_t n)
{
	uint32_t vals_left = data_set_limit - val;
	size_t togo = n - pos;
	double step = vals_left;

	step /= togo;

	// Bump step by 1 if it's >1, and if we're not generating
	// unique numbers. This reduces the chance we'll generate
	// masses of 0-value increments if step is, for example, 1.1
	// at this point. If it was, then 90% of the time it would
	// not increment val, whereas if we do this, then we will
	// at least ~53% of the time
	if (data_set_ops & DATA_SET_UNIQUE) {
		if (step < 1) {
			fprintf(stderr, "Data set requested is larger than the unique\n");
			fprintf(stderr, "set of values that can be generated. Aborting\n");
			exit(1);
		}
	} else if (step > 1) {
		step += 1;
	}

	// Generate a decimal value in the range 0..1
	double r = (double)random();
	r /= RAND_MAX;

	step *= r;

	if (pos == 0) {
		if (data_set_ops & DATA_SET_UNIQUE) {
			if (step < 1) {
				return 1;
			}
		}
		return (uint32_t)step;
	}

	if (data_set_ops & DATA_SET_UNIQUE) {
		if (step < 1)
			step = 1;
		if (step > 1)
			step += 1;
	}

	return val + (uint32_t)step;
} // get_next_val


void
fillset(uint32_t *a, size_t n)
{
	// First fill the set in an ordered manner
	a[0] = get_next_val(0, 0, n);
	for (size_t i = 1; i < n; i++) {
		a[i] = get_next_val(a[i-1], i, n);
	}

	// Disorder the set according to the disorder factor
	disorder_set(a, n);

	// Reverse the data set if asked to
	if (data_set_ops & DATA_SET_REVERSED)
		reverse_set(a, n);
} // fillset


int
main(int argc, char *argv[])
{
	size_t		n;
	uint32_t	*a;
	void		(*sort)() = NULL;
	char		*sortname;
	int		optpos = 1;

	if(argc < 3) {
		fprintf(stderr, "Incorrect number of arguments\n");
		usage(argv[0]);
	}

	while (strlen(argv[optpos]) == 2) {
		optpos += parse_control_opt(argv + optpos);
	}

	// Determine the sort type
	if ((sort = parse_sort_type(argv[optpos++], &sortname)) == NULL) {
		fprintf(stderr, "Unsupported sort type\n");
		usage(argv[0]);
	}

	// Determine the size of the array we'll be sorting
	if ((n = atol(argv[optpos++])) < 1) {
		fprintf(stderr, "Please use values of 1 or greater for the number of elements\n");
		exit(-1);
	}
	if (n >= INT32_MAX) {
		fprintf(stderr, "Please use values less than %d for the number of elements\n", INT32_MAX);
		exit(-1);
	}

	// Set up the random number generator
	if (random_seed > 0) {
		printf("Seeding Random Generator with:  %u\n", random_seed);
	} else {
		struct timespec now;

		clock_gettime(CLOCK_MONOTONIC, &now);
		random_seed = (uint32_t)((now.tv_sec ^ now.tv_nsec) & 0xffffffff);
		printf("Using randomly generated seed value: %u\n", random_seed);
	}
	srandom(random_seed);

	// Allocate up the array to sort
	if ((a = (uint32_t *)aligned_alloc(4096, n * sizeof(*a))) == NULL) {
		fprintf(stderr, "alloc failed - out of memory\n");
		exit(-1);
	}
	memset(a, 0, n * sizeof(*a));

	// Now populate the array according to the command line options
	printf("\nPopulating array of size: %lu\n\n", n);

	fillset(a, n);
	if (verbose) {
		print_array(a, n);
	}

	// Let's finally do this thing!
	struct timespec start, end;
	printf("Starting %s\n", sortname);
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

	// Did it sort correctly?
	test_sort(a, n);

	// Stats time!
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
