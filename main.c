#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>

extern void qrsort(char *a, size_t n, size_t es, uint32_t (*getkey)(const void *));

static uint32_t
get_key(const void *a)
{
        return *((uint32_t *)a);
} // get_key

int
compar(const void *p1, const void *p2)
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


int
main(int argc, char **argv)
{
	int		numels, i;
	uint32_t	*data;
	struct	timeval start, end;
	double	tim;
	int sort_type = 0;

	if(argc != 3) {
		fprintf(stderr, "Usage: %s <-q|-r> numels\n", argv[0]);
		exit(-1);
	}

	if(strcmp(argv[1], "-q") && strcmp(argv[1], "-r") && strcmp(argv[1], "-i")) {
		fprintf(stderr, "Usage: %s <-q|-r> numels\n", argv[0]);
		exit(-1);
	}

	if(strcmp(argv[1], "-q") == 0)
		sort_type = 0;
	if(strcmp(argv[1], "-r") == 0)
		sort_type = 1;

	numels = atoi(argv[2]);
	if (numels < 2) {
		fprintf(stderr, "Please use values of 2 or greater for the number of elements\n");
		exit(-1);
	}

	if((data = (uint32_t *)calloc(numels, sizeof(*data))) == NULL)
	{
		fprintf(stderr, "calloc failed - out of memory\n");
		exit(-1);
	}

	printf("Populating array\n");
	srandom(0);
	for(i = 0; i < numels; i++) {
		data[i] = random() % UINT32_MAX;
	}

	switch(sort_type) {
	case 0:
		printf("Using glibc qsort\n");
		gettimeofday(&start, NULL);
		qsort(data, numels, sizeof(*data), compar);
		gettimeofday(&end, NULL);
		break;
	case 1:
		printf("Using quick-radix-sort\n");
		gettimeofday(&start, NULL);
		qrsort((char *)data, numels, sizeof(*data), get_key);
		gettimeofday(&end, NULL);
		break;
	default:
		fprintf(stderr, "Invalid sort type specified\n");
		exit(-1);
		break;
	}


	tim = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
	printf("Time taken : %.6fs\n", tim);

	testsort(data, numels);

	free(data);
} /* main */
