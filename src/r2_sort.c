#include <stdlib.h>
#include <stdio.h>
#include "swap.h"

static const size_t steps[] = {16, 50, 90, 200, 1000, UINT32_MAX};
// 24 = 0.019457599
// 30 = 0.017951861
// 35 = 0.016949199
// 50 = 0.015863800
// 16 = 0.011807349

void
r2_sort(register char *a, size_t n, register const size_t es, register const int (*is_lt)(const void *, const void *))
{
	register char	*b, *c, *d, *e = a + n * es, *s;
	register int	swaptype;
	register WORD	t;
	size_t		step = 0, gap;
	int		pos = 0;
	char		temp[es];

	SWAPINIT(a, es);

	for (step = steps[pos]; step < n/3; step = steps[++pos]) {
printf("Gap1=%lu\n", step);
		gap = step * es;
		for (register char *q = a, *qe = a + gap; q < e; q = qe, qe += gap) {
			if (qe > e)
				qe = e;
			for (s=q, b=q+es; b<qe; s=b, b+=es)
				for (c=b; c>q && is_lt(c, s); c=s, s-=es)
					swap(c, s);
		}

		gap = (n/step);
		if (gap < step)
			gap = step;
		else
			gap -= gap % step;
		gap++;
printf("Gap2=%lu\n", gap);
		gap *= es;
		for (s = a + gap, b = s, d = a; b < e; b += es, d = b - gap)
                        if (is_lt(b, d)) {
				if (d < s) {
					swap(b, d);
					continue;
				}
                                copy(temp, b);
                                copy(b, d);
					for (c = d, d -= gap; (c >= s) && is_lt(temp, d); c = d, d -= gap)
						copy(c, d);
                                copy(c, temp);
                        }

/*
			for (c = b; c >= s && is_lt(c, d); c = d, d -= gap)
				swap(c, d);
*/
	}

	// At this point everything should be <10 positions of where it needs to be,
	// and most typically within 1-2 positions.  It so happens that insertion sort
	// is actually very efficient for sorting such really quickly, so we do that
printf("numswaps at final = %lu\n", numswaps);
	for (s=a, b=a+es; b<e; s=b, b+=es)
		for (c=b; c>a && is_lt(c, s); c=s, s-=es)
			swap(c, s);
} // r2_sort
