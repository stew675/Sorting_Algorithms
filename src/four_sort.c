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

#define sort3(p1, p2, p3, is_lt, swp)			\
{							\
	if (is_lt(p3, p2)) {				\
		/* p3 < p2 */				\
		if (is_lt(p2, p1)) {			\
			/* p3 < p2 < p1 */		\
			swap(p1, p3);			\
			swp = true;			\
		} else {				\
			/* p1/p3 < p2 */		\
			if (is_lt(p3, p1)) {		\
				/* p3 < p1 < p2 */	\
				swap(p1, p2);		\
				swap(p1, p3);		\
				swp = true;		\
			} else {			\
				/* p1 < p3 < p2 */	\
				swap(p2, p3);		\
				swp = true;		\
			}				\
		}					\
	} else {					\
		/* p2 < p3 */				\
		if (is_lt(p2, p1)) {			\
			/* p2 < p1/p3 */		\
			if (is_lt(p3, p1)) {		\
				/* p2 < p3 < p1 */	\
				swap(p1, p3);		\
				swap(p1, p2);		\
				swp = true;		\
			} else {			\
				/* p2 < p1 < p3 */	\
				swap(p1, p2);		\
				swp = true;		\
			}				\
		} else {				\
			/* p1 < p2 < p3 */		\
		}					\
	}						\
}

#define sort4(p1, p2, p3, p4, is_lt, swp)				\
	if (is_lt(p3, p2)) {						\
		/* p3 < p2 */						\
		if (is_lt(p4, p3)) {					\
			/* p4 < p3 < p2	*/				\
			if (is_lt(p3, p1)) {				\
				/* p4 < p3 < p1/p2 */			\
				if (is_lt(p2, p1)) {			\
					/* p4 < p3 < p2 < p1 */		\
					swap(p1, p4);			\
					swap(p2, p3);			\
					swp = true;			\
				} else {				\
					/* p4 < p3 < p1 < p2 */		\
					swap(p1, p4);			\
					swap(p2, p4);			\
					swap(p2, p3);			\
					swp = true;			\
				}					\
			} else {					\
				/* p1/p4 < p3 < p2 */			\
				if (is_lt(p4, p1)) {			\
					/* p4 < p1 < p3 < p2 */		\
					swap(p1, p2);			\
					swap(p1, p4);			\
					swp = true;			\
				} else {				\
					/* p1 < p4 < p3 < p2 */		\
					swap(p2, p4);			\
					swp = true;			\
				}					\
			}						\
		} else {						\
			/* p3 < p2/p4 */				\
			if (is_lt(p4, p2)) {				\
				/* p3 < p4 < p2 */			\
				if (is_lt(p4, p1)) {			\
					/* p3 < p4 < p1/p2 */		\
					if (is_lt(p2, p1)) {		\
						/* p3 < p4 < p2 < p1 */	\
						swap(p1, p3);		\
						swap(p2, p3);		\
						swap(p2, p4);		\
						swp = true;		\
					} else {			\
						/* p3 < p4 < p1 < p2 */	\
						swap(p1, p3);		\
						swap(p2, p4);		\
						swp = true;		\
					}				\
				} else {				\
					/* p1/p3 < p4 < p2 */		\
					if (is_lt(p3, p1)) {		\
						/* p3 < p1 < p4 < p2 */	\
						swap(p1, p3);		\
						swap(p2, p4);		\
						swap(p2, p3);		\
						swp = true;		\
					} else {			\
						/* p1 < p3 < p4 < p2 */	\
						swap(p3, p2);		\
						swap(p3, p4);		\
						swp = true;		\
					}				\
				}					\
			} else {					\
				/* p3 < p2 < p4 */			\
				if (is_lt(p2, p1)) {			\
					/* p3 < p2 < p1/p4 */		\
					if (is_lt(p4, p1)) {		\
						/* p3 < p2 < p4 < p1 */	\
						swap(p3, p1);		\
						swap(p3, p4);		\
						swp = true;		\
					} else {			\
						/* p3 < p2 < p1 < p4 */	\
						swap(p1, p3);		\
						swp = true;		\
					}				\
				} else {				\
					/* p1/p3 < p2 < p4 */		\
					if (is_lt(p3, p1)) {		\
						/* p3 < p1 < p2 < p4 */	\
						swap(p1, p2);		\
						swap(p1, p3);		\
						swp = true;		\
					} else {			\
						/* p1 < p3 < p2 < p4 */	\
						swap(p2, p3);		\
						swp = true;		\
					}				\
				}					\
			}						\
		}							\
	} else {							\
		/* p2 < p3 */						\
		if (is_lt(p2, p1)) {					\
			/* p2 < p1/p3 */				\
			if (is_lt(p3, p1)) {				\
				/* p2 < p3 < p1 */			\
				if (is_lt(p4, p3)) {			\
					/* p2/p4 < p3 < p1 */		\
					if (is_lt(p4, p2)) {		\
						/* p4 < p2 < p3 < p1 */	\
						swap(p1, p4);		\
						swp = true;		\
					} else {			\
						/* p2 < p4 < p3 < p1 */	\
						swap(p1, p4);		\
						swap(p1, p2);		\
						swp = true;		\
					}				\
				} else {				\
					/* p2 < p3 < p1/p4 */		\
					if (is_lt(p4, p1)) {		\
						/* p2 < p3 < p4 < p1 */	\
						swap(p1, p2);		\
						swap(p2, p4);		\
						swap(p2, p3);		\
						swp = true;		\
					} else {			\
						/* p2 < p3 < p1 < p4 */	\
						swap(p1, p3);		\
						swap(p1, p2);		\
						swp = true;		\
					}				\
				}					\
			} else {					\
				/* p2 < p1 < p3 */			\
				if (is_lt(p4, p1)) {			\
					/* p2/p4 < p1 < p3 */		\
					if (is_lt(p4, p2)) {		\
						/* p4 < p2 < p1 < p3 */	\
						swap(p1, p3);		\
						swap(p1, p4);		\
						swp = true;		\
					} else {			\
						/* p2 < p4 < p1 < p3 */	\
						swap(p1, p3);		\
						swap(p1, p4);		\
						swap(p1, p2);		\
						swp = true;		\
					}				\
				} else {				\
					/* p2 < p1 < p3/p4 */		\
					if (is_lt(p4, p3)) {		\
						/* p2 < p1 < p4 < p3 */	\
						swap(p1, p2);		\
						swap(p3, p4);		\
						swp = true;		\
					} else {			\
						/* p2 < p1 < p3 < p4 */	\
						swap(p1, p2);		\
						swp = true;		\
					}				\
				}					\
			}						\
		} else {						\
			/* p1 < p2 < p3 */				\
			if (is_lt(p4, p2)) {				\
				/* p1/p4 < p2 < p3 */			\
				if (is_lt(p4, p1)) {			\
					/* p4 < p1 < p2 < p3 */		\
					swap(p1, p2);			\
					swap(p1, p4);			\
					swap(p3, p4);			\
					swp = true;			\
				} else {				\
					/* p1 < p4 < p2 < p3 */		\
					swap(p2, p3);			\
					swap(p2, p4);			\
					swp = true;			\
				}					\
			} else {					\
				/* p1 < p2 < p3/p4 */			\
				if (is_lt(p4, p3)) {			\
					/* p1 < p2 < p4 < p3 */		\
					swap(p3, p4);			\
					swp = true;			\
				} else {				\
					/* p1 < p2 < p3 < p4 */		\
					/* IS SORTED - DO NOTHING */	\
				}					\
			}						\
		}							\
	}

static bool
four_sort_forwards(register char *a, size_t n, register const size_t es, register ilt is_less_than, register int swaptype, register size_t step)
{
        register char *e = a+(n-1)*es;

	step *= es;

	if ((((e - a) + 1) / 2) < step)
		return false;

        register char *a1, *a2, *a3, *a4;
        register WORD t;
	bool swapped = false;

	for (size_t s = 0; s < step; s+=es) {
		if ((e-a) < s)
			break;

		a1 = a + s;

		if ((e - a1) < step)
			break;

		a2 = a1 + step;

		if ((e - a2) < step) {
			sort2(a1, a2, is_less_than, swapped);
			continue;
		}

		for (;;) {
			a3 = a2 + step;

			if ((e - a3) < step) {
				sort3(a1, a2, a3, is_less_than, swapped);
				break;
			}

			a4 = a3 + step;

			sort4(a1, a2, a3, a4, is_less_than, swapped);

			a1 = a3;
			a2 = a4;

			if ((e - a2) < step)
				break;
		}
	}
	return swapped;
} // four_sort_forwards


static bool
four_sort_backwards(register char *a, size_t n, register const size_t es, register ilt is_less_than, register int swaptype, register size_t step)
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

		a2 = a1 - step;

		if ((a2-a) < step) {
			sort2(a2, a1, is_less_than, swapped);
			continue;
		}

		for (;;) {
			a3 = a2 - step;

			if ((a3 - a) < step) {
				sort3(a3, a2, a1, is_less_than, swapped);
				break;
			}

			a4 = a3 - step;

			sort4(a4, a3, a2, a1, is_less_than, swapped);

			a1 = a3;
			a2 = a4;

			if ((a2-a) < step)
				break;
		}
	}
	return swapped;
} // four_sort_backwards


static void
four_sort2(register char *a, size_t n, register const size_t es, register ilt is_less_than, register int swaptype)
{
        register char *s, *p, *v, *u;
        register char *e = a+n*es;
        register WORD t;

        for (s=a+4*es; a<e; a=s, s+=4*es)
                for (p=a+es; p<s; p+=es)
                        for (v=p, u=p-es; v>a && is_less_than(v, u); v=u, u-=es)
                                swap(v, u);
} // four_sort


static void
eight_sort(register char *a, size_t n, register const size_t es, register ilt is_less_than, register int swaptype)
{
        register char *s, *p, *v, *u;
        register char *e = a+n*es;
        register WORD t;

        for (s=a+8*es; a<e; a=s, s+=8*es)
                for (p=a+es; p<s; p+=es)
                        for (v=p, u=p-es; v>a && is_less_than(v, u); v=u, u-=es)
                                swap(v, u);
} // four_sort


void
four_sort(register char *a, size_t n, register const size_t es, register ilt is_less_than)
{
        register char *e = a+(n-1)*es;
        int swaptype;
	int swapped;
	int onepasses = 0;

        SWAPINIT(a, es);

	four_sort_forwards(a, n, es, is_less_than, swaptype, 1259);
	four_sort_backwards(a, n, es, is_less_than, swaptype, 631);

	four_sort_forwards(a, n, es, is_less_than, swaptype, 317);
	four_sort_backwards(a, n, es, is_less_than, swaptype, 163);

	four_sort_forwards(a, n, es, is_less_than, swaptype, 83);
	four_sort_backwards(a, n, es, is_less_than, swaptype, 43);

	four_sort_forwards(a, n, es, is_less_than, swaptype, 23);
	four_sort_backwards(a, n, es, is_less_than, swaptype, 13);

	four_sort_forwards(a, n, es, is_less_than, swaptype, 7);
	four_sort_backwards(a, n, es, is_less_than, swaptype, 5);

	four_sort_forwards(a, n, es, is_less_than, swaptype, 3);
	four_sort_backwards(a, n, es, is_less_than, swaptype, 2);

	for (;;) {
		onepasses++;
		if (!four_sort_forwards(a, n, es, is_less_than, swaptype, 1))
			break;
		onepasses++;
		if (!four_sort_backwards(a, n, es, is_less_than, swaptype, 1))
			break;
	}

	printf("Four Sort took %d onepasses\n", onepasses);
} // four_sort
