#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include "swap.h"

typedef const int (*ilt)(const void *, const void *);

#define did_swap(a)	(a)=true

#define sort2(p1, p2, is_lt, swp)	\
	if (is_lt(p2, p1)) {		\
		swap(p1, p2);		\
		swp = true;		\
	}				\

// Sort 1 plus pair
// p2 and p3 arrive at us already sorted
#define sort1pp(p1, p2, p3, is_lt, swp)					\
	/* p2 < p3 */							\
	if (!is_lt(p2, p1)) {						\
		/* p1 < p2 < p3 */					\
	} else if (is_lt(p3, p1)) {					\
		/* p2 < p3 < p1 */					\
		swap(p1, p2);						\
		swap(p2, p3);						\
		swp = true;						\
	} else {							\
		/* p2 < p1 < p3 */					\
		swap(p1, p2);						\
		swp = true;						\
	}

// Sort pair plus 1 more 
// p1 and p2 arrive at us already sorted
#define sortpp1(p1, p2, p3, is_lt, swp)					\
	/* p1 < p2 */							\
	if (!is_lt(p3, p2)) {						\
		/* p1 < p2 < p3 */					\
	} else if (is_lt(p3, p1)) {					\
		/* p3 < p1 < p2 */					\
		swap(p2, p3);						\
		swap(p1, p2);						\
		swp = true;						\
	} else {							\
		/* p1 < p3 < p2 */					\
		swap(p1, p2);						\
		swp = true;						\
	}


// Sort 2 pairs
#define sort2pairs(p1, p2, p3, p4, is_lt, swp)				\
{									\
	/* p1 < p2 and p3 < p4 */					\
	if (!is_lt(p3, p2)) {						\
		/* p1 < p2 < p3 < p4 */					\
	} else if (is_lt(p4, p1)) {					\
		/* p3 < p4 < p1 < p2 */					\
		swap(p1, p3);						\
		swap(p2, p4);						\
		swp = true;						\
	} else {							\
		/* p1 < p2, p3 < p4, p1 < p4, p3 < p2 */		\
		if (is_lt(p4, p2)) {					\
			/* p1/p3 < p4 < p2 */				\
			if (is_lt(p3, p1)) {				\
				/* p3 < p1 < p4 < p2 */			\
				swap(p1, p3);				\
				swap(p2, p4);				\
				swap(p2, p3);				\
				swp = true;				\
			} else {					\
				/* p1 < p3 < p4 < p2 */			\
				swap(p2, p3);				\
				swap(p3, p4);				\
				swp = true;				\
			}						\
		} else {						\
			/* p1/p3 < p2 < p4 */				\
			if (is_lt(p3, p1)) {				\
				/* p3 < p1 < p2 < p4 */			\
				swap(p2, p3);				\
				swap(p1, p2);				\
				swp = true;				\
			} else {					\
				/* p1 < p3 < p2 < p4 */			\
				swap(p2, p3);				\
				swp = true;				\
			}						\
		}							\
	}								\
}


static bool
four_sort_forwards(register char *a, size_t n, register const size_t es, register ilt is_lt, register int swaptype, register size_t step)
{
        register char *e = a+(n-1)*es;

	step *= es;

	if ((((e - a) + 1) / 2) < step)
		return false;

        register char *a1, *a2, *a3, *a4;
        register WORD t;
	register bool swapped = false;

	for (size_t s = 0; s < step; s+=es) {
		if ((e-a) < s)
			break;

		a1 = a + s;

		if ((e - a1) < step)
			break;

		for (;;) {
			a2 = a1 + step;

			sort2(a1, a2, is_lt, swapped);

			if ((e - a2) < step)
				break;

			a3 = a2 + step;

			if ((e - a3) < step) {
				sortpp1(a1, a2, a3, is_lt, swapped);
				break;
			}

			a4 = a3 + step;

			sort2(a3, a4, is_lt, swapped);

			sort2pairs(a1, a2, a3, a4, is_lt, swapped);

			a1 = a4;

			if ((e-a1) < step)
				break;
		}
	}
	return swapped;
} // four_sort_forwards


static bool
four_sort_backwards(register char *a, size_t n, register const size_t es, register ilt is_lt, register int swaptype, register size_t step)
{
        register char *e = a+(n-1)*es;

	step *= es;

	if ((((e - a) + 1) / 2) < step)
		return false;

        register char *a1, *a2, *a3, *a4;
        register WORD t;
	register bool swapped = false;

	for (size_t s = 0; s < step; s+=es) {
		if ((e-a) < s)
			break;

		a1 = e - s;

		if ((a1-a) < step)
			break;

		for (;;) {
			a2 = a1 - step;

			sort2(a2, a1, is_lt, swapped);

			if ((a2-a) < step)
				break;

			a3 = a2 - step;

			if ((a3 - a) < step) {
				sort1pp(a3, a2, a1, is_lt, swapped);
				break;
			}

			a4 = a3 - step;

			sort2(a4, a3, is_lt, swapped);

			sort2pairs(a4, a3, a2, a1, is_lt, swapped);

			a1 = a4;

			if ((a1-a) < step)
				break;
		}
	}
	return swapped;
} // four_sort_backwards


static const size_t steps[] = {1, 2, 3, 5, 7, 11, 13, 17, 23, 31, 43, 59, 73, 101, 131, 179, 239, 317, 421, 563, 751, 997, 1327, 1777,
			       2357, 3137, 4201, 5591, 7459, 9949, 13267, 17707, 23599, 31469, 41953, 55933, 74573, 99439, 4294967295};

//static const size_t steps[] = {1, 2, 5, 7, 13, 23, 37, 59, 101, 167, 277, 461, 769, 1277, 2129, 3547, 5903, 9851, 16411, 27361, 45587, 75979, 126631, UINT32_MAX};

void
four_sort(register char *a, size_t n, register const size_t es, register ilt is_lt)
{
        register char *e = a+(n-1)*es;
        int swaptype;
	int swapped;
	int onepasses = 0;
	int pos = 0;
	size_t step = n;

        SWAPINIT(a, es);

#define next_step       ((step > steps[pos+1]) ? (n / steps[++pos]) : (pos > 0 ? steps[--pos] : 1))
	for (;;) {
		step = next_step;
		swapped = four_sort_forwards(a, n, es, is_lt, swaptype, step);
		if (step == 1) {
			onepasses++;
		 	if (!swapped)
				break;
		}

		step = next_step;
		swapped = four_sort_backwards(a, n, es, is_lt, swaptype, step);
		if (step == 1) {
			onepasses++;
		 	if (!swapped)
				break;
		}
	}
#undef next_step

	printf("Four Sort took %d onepasses\n", onepasses);
} // four_sort


// A1+A2 -> 1C + 0.5S
// A3+A4 -> 1C + 0.5S
// A2+A3 -> 1C + 0.5S
//	If A2+A3 swapped
//		A1+A2 -> 1C + 0.5S
//		A3+A4 -> 1C + 0.5S

// On Average: 4 compares and 3 swaps
