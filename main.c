#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <dlfcn.h>

extern void qrsort(char *a, size_t n, size_t es, uint32_t (*getkey)(const void *));
extern void rattle_sort(void *a, size_t n, uint32_t es, int (*cmp)());
extern void nqsort(void *a, size_t n, size_t es, int (*cmp)());

static uint32_t
get_key(register const void *a)
{
	return *((uint32_t *)a);
} // get_key

int
compar(register const void *p1, register const void *p2)
{
	register const uint32_t *a = (const uint32_t *)p1;
	register const uint32_t *b = (const uint32_t *)p2;

	return (*a == *b) ? 0 : ((*a < *b) ? -1 : 1);
}/*compar*/


void
testsort(uint32_t a[], int numels)
{
	int	i;

	numels--;
	for(i = 0; i < numels; i++)
	{
		if(a[i] > a[i+1])
			break;
	}
	if(i < numels) {
		fprintf(stderr, "Didn't sort data correctly\n");
		exit(-1);
	}
} /* testsort */


#if 0
void *
load_rattle_sort()
{
	void *handle;
	double (*cosine)(double);
	void (*rattle_sort)(void *a, size_t n, size_t es, int (*cmp)());
	char *error;

	handle = dlopen("./rattle_sort.so", RTLD_NOW | RTLD_LOCAL);
	if (!handle) {
		fprintf(stderr, "%s\n", dlerror());
		exit(EXIT_FAILURE);
	}

	dlerror();    /* Clear any existing error */

	*(void **)(&rattle_sort) = dlsym(handle, "rattle_sort");

	/*
	According to the ISO C standard, casting between function
	pointers and 'void *', as done above, produces undefined results.
	POSIX.1-2003 and POSIX.1-2008 accepted this state of affairs and
	roposed the following workaround:

		  *(void **) (&cosine) = dlsym(handle, "cos");

	This (clumsy) cast conforms with the ISO C standard and will
	avoid any compiler warnings.

	The 2013 Technical Corrigendum to POSIX.1-2008 (a.k.a.
	POSIX.1-2013) improved matters by requiring that conforming
	implementations support casting 'void *' to a function pointer.
	Nevertheless, some compilers (e.g., gcc with the '-pedantic'
	option) may complain about the cast used in this program.
	*/

	error = dlerror();
	if (error != NULL) {
		fprintf(stderr, "%s\n", error);
		exit(EXIT_FAILURE);
	}
	return rattle_sort;
}
#endif

void
usage(char *prog)
{
	fprintf(stderr, "Usage: %s <-nq|-qs|-qr|-rs> numels\n", prog);
	exit(-1);
} // usage


int
main(int argc, char **argv)
{
	int		numels, i;
	uint32_t	*data;
	struct timespec start, end;
	double	tim;
	int sort_type = -1;

	if(argc != 3) {
		fprintf(stderr, "Incorrect number of argument\n");
		usage(argv[0]);
	}

	if(strcmp(argv[1], "-qs") == 0)
		sort_type = 0;
	if(strcmp(argv[1], "-qr") == 0)
		sort_type = 1;
	if(strcmp(argv[1], "-rs") == 0)
		sort_type = 3;
	if(strcmp(argv[1], "-nq") == 0)
		sort_type = 4;

	if (sort_type < 0) {
		fprintf(stderr, "Unsupported sort type\n");
		usage(argv[0]);
	}

	numels = atoi(argv[2]);
	if (numels < 2) {
		fprintf(stderr, "Please use values of 2 or greater for the number of elements\n");
		exit(-1);
	}

	data = (uint32_t *)aligned_alloc(4096, numels * sizeof(*data));
	if(data == NULL) {
		fprintf(stderr, "calloc failed - out of memory\n");
		exit(-1);
	}
	memset(data, 0, numels * sizeof(*data));

	printf("Populating array\n");
	srandom(0);
	for(i = 0; i < numels; i++) {
//		data[i] = numels - i;
		data[i] = random() % UINT32_MAX;
	}

	switch(sort_type) {
	case 0:
		printf("Using glibc qsort\n");

		clock_gettime(CLOCK_MONOTONIC, &start);
		qsort(data, numels, sizeof(*data), compar);
		clock_gettime(CLOCK_MONOTONIC, &end);

		break;
	case 1:
		printf("Using quick-radix-sort\n");

		clock_gettime(CLOCK_MONOTONIC, &start);
		qrsort((char *)data, numels, sizeof(*data), get_key);
		clock_gettime(CLOCK_MONOTONIC, &end);

		break;
	case 3:
		printf("Using rattle sort\n");

		clock_gettime(CLOCK_MONOTONIC, &start);
		rattle_sort(data, numels, (uint32_t)sizeof(*data), compar);
		clock_gettime(CLOCK_MONOTONIC, &end);

		break;
	case 4:
		printf("Using new quick sort\n");

		clock_gettime(CLOCK_MONOTONIC, &start);
		nqsort(data, numels, sizeof(*data), compar);
		clock_gettime(CLOCK_MONOTONIC, &end);

		break;
	default:
		fprintf(stderr, "Invalid sort type specified\n");
		exit(-1);
		break;
	}


	tim = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
	printf("Time taken : %.9fs\n", tim);

	testsort(data, numels);

	free(data);
} /* main */
