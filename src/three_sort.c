#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "swap.h"

extern uint64_t numcmps;


void three_sort(register char *a, size_t n, register const size_t es, register const int (*is_lt)(const void *, const void *))
{
	register char	*b, *c, *d, *s, *e = a + n * es;
	register size_t step, gap;
	register int    swaptype, i, max = 5;
	register WORD	t;
//	char		tmp[es];
	double		factor = 1.87;

	if (n < 2)
		return;

	SWAPINIT(a, es);

	for (step = n / max; step > 1; step = step / factor) {
		gap = step * es;
		s = a + gap;
		for (d = a, b = a + gap; b < e; b += es, d = b - gap)
	    		for (c = b, i = 0; i < max && c >= s && is_lt(c, d); i++, c = d, d -= gap)
				swap(c, d);
	}

	printf("NumCmps = %lu, NumSwaps = %lu\n", numcmps, numswaps);

	// At this point everything should be <10 positions of where it needs to be,
	// and most typically within 0-2 positions.  It so happens that insertion sort
	// is actually very efficient for sorting such really quickly, so we do that
	for (s=a, b=a+es; b<e; s=b, b+=es)
		for (c=b; c>a && is_lt(c, s); c=s, s-=es)
			swap(c, s);
} // three_sort
