#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "swap.h"

extern uint64_t numcmps;

#if 0

#define throptf						\
{							\
	if (is_lt(p2, p1)) {				\
		/* p2 < p1 */				\
		if (is_lt(p3, p1)) {			\
			/* p2/p3 < p1 */		\
			swap(p1, p3);			\
			if (is_lt(p2, p1)) {		\
				swap(p1, p2);		\
			}				\
		} else {				\
			swap(p1, p2);			\
		}					\
	} else {					\
		/* p1 < p2 */				\
		if (is_lt(p3, p2)) {			\
			/* p1/p3 < p2 */		\
			swap(p2, p3);			\
			if (is_lt(p2, p1)) {		\
				swap(p1, p2);		\
			}				\
		}					\
	}						\
}



#define throptb						\
{							\
	if (is_lt(p3, p2)) {				\
		/* p3 < p2 */				\
		if (is_lt(p3, p1)) {			\
			/* p3 < p1/p2 */		\
			swap(p1, p3);			\
			if (is_lt(p3, p2)) {		\
				swap(p2, p3);		\
			}				\
		} else {				\
			swap(p2, p3);			\
		}					\
	} else {					\
		/* p2 < p3 */				\
		if (is_lt(p2, p1)) {			\
			/* p2 < p1/p3 */		\
			swap(p1, p2);			\
			if (is_lt(p3, p2)) {		\
				/* p2 < p3 < p1 */	\
				swap(p2, p3);		\
			}				\
		}					\
	}						\
}

#endif


void tri_sort(register char *a, size_t n, register const size_t es, register const int (*is_lt)(const void *, const void *))
{
	register char	*p1, *p2, *p3, *stop, *fullstop, *e = a + n * es;
	register size_t gap, step;
	register int    swaptype;
	register WORD	t;
	double		factor = 1.40; // 1.4

	if (n < 2)
		return;

	SWAPINIT(a, es);

	for (step = (n + 2) / 3; step > 1; step = step / factor) {
		gap = step * es;
		fullstop = e - (gap << 1);
		p1 = a;
		p2 = a + gap;
		p3 = a + (gap << 1);
		for (; p1 < fullstop; p1 = p2, p2 = p3, p3 += gap) {
			stop = p2 < fullstop ? p2 : fullstop;
			for (; p1 < stop; p1 += es, p2 += es, p3 += es) {
				if (is_lt(p2, p1))
					swap(p2, p1);
				if (is_lt(p3, p2)) {
					swap(p3, p2);
					if (is_lt(p2, p1))
						swap(p2, p1);
				}
			}
		}

		if ((step = step / factor) < 2)
			break;

		gap = step * es;
		fullstop = a + (gap << 1) - es;
		p3 = e - es;
		p2 = p3 - gap;
		p1 = p3 - (gap << 1);
		for (; p3 > fullstop; p3 = p2, p2 = p1, p1 -= gap) {
			stop = p2 > fullstop ? p2 : fullstop;
			for(; p3 > stop; p1 -= es, p2 -= es, p3 -= es) {
				if (is_lt(p3, p2))
					swap(p3, p2);
				if (is_lt(p2, p1)) {
					swap(p2, p1);
					if (is_lt(p3, p2))
						swap(p3, p2);
				}
			}
		}
	}

	printf("NumCmps = %lu, NumSwaps = %lu\n", numcmps, numswaps);

	// At this point everything should be <10 positions of where it needs to be,
	// and most typically within 0-2 positions.  It so happens that insertion sort
	// is actually very efficient for sorting such really quickly, so we do that
	for (p1=a, p2=a+es; p2<e; p1=p2, p2+=es)
		for (p3=p2; p3>a && is_lt(p3, p1); p3=p1, p1-=es)
			swap(p3, p1);
} // tri_sort
