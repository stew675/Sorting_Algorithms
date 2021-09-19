#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "swap.h"

extern uint64_t numcmps;

typedef const int (*ilt)(const void *, const void *);

// Life Sort Pair Plus Two
#define lsppt { 								\
	if (pair && is_lt(p2, p1))						\
		swap(p1, p2);							\
	/* p1 < p2 */								\
	if (is_lt(p3, p2)) {							\
		/* p1/p3 < p2 */						\
		if (is_lt(p3, p1)) {						\
			/* p3 < p1 < p2 */					\
			if (is_lt(p4, p2)) {					\
				if (is_lt(p4, p1)) {				\
					/* p3/p4 < p1 < p2 */			\
					if (is_lt(p4, p3)) {			\
						/* p4 < p3 < p1 < p2) */	\
						swap(p1, p4);			\
						swap(p2, p3);			\
						swap(p3, p4);			\
					} else {				\
						/* p3 < p4 < p1 < p2) */	\
						swap(p1, p3);			\
						swap(p2, p4);			\
					}					\
				} else {					\
					/* p3 < p1 < p4 < p2 */			\
					swap(p1, p3);				\
					swap(p2, p3);				\
					swap(p3, p4);				\
				}						\
			} else {						\
				/* p3 < p1 < p2 < p4 */				\
				swap(p1, p3);					\
				swap(p2, p3);					\
			}							\
		} else {							\
			/* p1 < p3 < p2 */					\
			if (is_lt(p4, p2)) {					\
				/* p1 < p3 < p2, p4 < p2 */			\
				if (is_lt(p4, p3)) {				\
					/* p1/p4 < p3 < p2 */			\
					if (is_lt(p4, p1)) {			\
						/* p4 < p1 < p3 < p2 */		\
						swap(p1, p4);			\
						swap(p2, p4);			\
					} else {				\
						/* p1 < p4 < p3 < p2 */		\
						swap(p2, p4);			\
					}					\
				} else {					\
					/* p1 < p3 < p4 < p2 */			\
					swap(p2, p3);				\
					swap(p3, p4);				\
				}						\
			} else {						\
				/* p1 < p3 < p2 < p4 */				\
				swap(p2, p3);					\
			}							\
		}								\
	} else {								\
		/* p1 < p2 < p3 */						\
		if (is_lt(p4, p3)) {						\
			/* p1 < p2 < p3, p4 < p3 */				\
			if (is_lt(p4, p2)) {					\
				/* p1/p4 < p2 < p3 */				\
				if (is_lt(p4, p1)) {				\
					/* p4 < p1 < p2 < p3 */			\
					swap(p1, p4);				\
					swap(p2, p4);				\
					swap(p3, p4);				\
				} else {					\
					/* p1 < p4 < p2 < p3 */			\
					swap(p2, p4);				\
					swap(p3, p4);				\
				}						\
			} else {						\
				/* p1 < p2 < p4 < p3 */				\
				swap(p3, p4);					\
			}							\
		} else {							\
			/* p1 < p2 < p3 < p4 */					\
			/* SORTED */						\
		}								\
	}									\
}


// Life Sort Two Plus Pair
#define lstpp { 								\
	if (pair && is_lt(p4, p3))						\
		swap(p3, p4);							\
	/* p3 < p4 */								\
	if (is_lt(p3, p2)) {							\
		/* p3 < p2/p4 */						\
		if (is_lt(p4, p2)) {						\
			/* p3 < p4 < p2 */					\
			if (is_lt(p3, p1)) {					\
				/* p3 < p4 < p2, p3 < p1 */			\
				if (is_lt(p4, p1)) {				\
					/* p3 < p4 < p1/p2 */			\
					if (is_lt(p2, p1)) {			\
						/* p3 < p4 < p2 < p1 */		\
						swap(p1, p3);			\
						swap(p2, p4);			\
						swap(p3, p4);			\
					} else {				\
						/* p3 < p4 < p1 < p2 */		\
						swap(p1, p3);			\
						swap(p2, p4);			\
					}					\
				} else {					\
					/* p3 < p1 < p4 < p2 */			\
					swap(p1, p3);				\
					swap(p2, p3);				\
					swap(p3, p4);				\
				}						\
			} else {						\
				/* p1 < p3 < p4 < p2 */				\
				swap(p2, p3);					\
				swap(p3, p4);					\
			}							\
		} else {							\
			/* p3 < p2 < p4 */					\
			if (is_lt(p3, p1)) {					\
				/* p3 < p2 < p4, p3 < p1 */			\
				if (is_lt(p2, p1)) {				\
					/* p3 < p2 < p1/p4 */			\
					if (is_lt(p4, p1)) {			\
						/* p3 < p2 < p4 < p1 */		\
						swap(p1, p3);			\
						swap(p3, p4);			\
					} else {				\
						/* p3 < p2 < p1 < p4 */		\
						swap(p1, p3);			\
					}					\
				} else {					\
					/* p3 < p1 < p2 < p4 */			\
					swap(p1, p3);				\
					swap(p2, p3);				\
				}						\
			} else {						\
				/* p1 < p3 < p2 < p4 */				\
				swap(p2, p3);					\
			}							\
		}								\
	} else {								\
		/* p2 < p3 < p4 */						\
		if (is_lt(p2, p1)) {						\
			/* p2 < p3 < p4, p2 < p1 */				\
			if (is_lt(p3, p1)) {					\
				/* p2 < p3 < p1/p4 */				\
				if (is_lt(p4, p1)) {				\
					/* p2 < p3 < p4 < p1 */			\
					swap(p1, p2);				\
					swap(p2, p3);				\
					swap(p3, p4);				\
				} else {					\
					/* p2 < p3 < p1 < p4 */			\
					swap(p1, p2);				\
					swap(p2, p3);				\
				}						\
			} else {						\
				/* p2 < p1 < p3 < p4 */				\
				swap(p1, p2);					\
			}							\
		} else {							\
			/* p1 < p2 < p3 < p4 */					\
			/* SORTED */						\
		}								\
	}									\
}

#define CUTOFF 1

void
life_sort(register char *a, size_t n, register const size_t es, register ilt is_lt)
{
	register char	*e = a + (n - 1) * es, *h, *p1, *p2, *p3, *p4, *stop;
	register size_t	step, gap;
	register int	pair, swaptype, c;
	register WORD	t;
	register double	nn = n / 4.0, factor = 1.71;

	SWAPINIT(a, es);

	for (step = nn; step >= CUTOFF; step = (nn /= factor)) {

#if 0
		// Forward Pass
		gap = step * es;
		p1 = a + gap;
		for (p4 = p1; p4 <= e; p4+=es)
			for (c = 0, p3 = p4, p2 = p4 - gap; c < 3 && p3 >= p1 && is_lt(p3, p2); c++, p3 = p2, p2 -= gap)
				swap(p2, p3);

		if ((step = (nn /= factor)) < CUTOFF)
			break;

		// Backwards Pass
		gap = step * es;
		p4 = e - gap;
		for (p1 = p4; p1 >= a; p1-=es)
			for (c = 0, p2 = p1, p3 = p1 + gap; c < 3 && p2 <= p4 && is_lt(p3, p2); c++, p2 = p3, p3 += gap)
				swap(p2, p3);

#endif
		// Forward Pass
		gap = step * es;
		for (h=a, pair=1; h<=e-gap*3; h+=(gap<<1), pair=0) {
			stop = p4 = h+gap*3, p3=h+gap*2, p2=h+gap, p1=h;

			if (stop >= e-gap)
				stop=e-gap+es;

			for (; p3<stop; p1+=es, p2+=es, p3+=es, p4+=es)
				lsppt;
		}

		if ((step = (nn /= factor)) < CUTOFF)
			break;

		// Backwards Pass
		gap = step * es;
		for (h=e, pair=1; h>=a+gap*3; h-=(gap<<1), pair=0) {
			stop = p1 = h-gap*3, p2=h-gap*2, p3=h-gap, p4=h;

			if (stop <= a+gap)
				stop=a+gap-es;

			for (; p2>stop; p1-=es, p2-=es, p3-=es, p4-=es)
				lstpp;
		}
	}

printf("NumCmps = %lu, NumSwaps = %lu\n", numcmps, numswaps);

	for (p1=a, p2=a+es; p1<e; p1=p2, p2+=es)
		for (p3 = p2; p3 > a && is_lt(p3, p1); p3 = p1, p1 -= es)
			swap(p1, p3);
} // life_sort
