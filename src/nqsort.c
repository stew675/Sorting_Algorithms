// nqsort.c
//
// Taken from: https://cs.fit.edu/~pkc/classes/writing/papers/bentley93engineering.pdf
//
// It's basically a recursive version of modern glibc qsort

#include <stddef.h>
#include "swap.h"

static char *
med3(char *a, char *b, char *c, int (*cmp)(void *, void *))
{
	return	cmp(a, b) < 0 ?
		(cmp(b, c) < 0 ? b : cmp(a, c) < 0 ? c : a) :
		(cmp(b, c) > 0 ? b : cmp(a, c) > 0 ? c : a);
} // med3


void
nqsort(char *a, size_t n, size_t es, int (*cmp)(void *, void *))
{
	char *pa, *pb, *pc, *pd, *pl, *pm, *pn, *pv;
	int r, swaptype;
	WORD t, v;
	size_t s;

	SWAPINIT(a, es);
	if (n <= 9) { /* Insertion sort on smallest arrays */
		for (pm = a + es; pm < a + n*es; pm += es) {
			for (pl = pm; pl > a && cmp(pl-es, pl) > 0; pl -= es) {
				swap(pl, pl-es);
			}
		}
		return;
	}

	pm = a + (n/2)*es; /* Small arrays, middle element */

	if (n > 9) {
		pl = a;
		pn = a + (n-1)*es;

		if (n > 44) { /* Big arrays, pseudomedian of 9 */
			s = (n/8)*es;
			pl = med3(pl, pl+s, pl+2*s, cmp);
			pm = med3(pm-s, pm, pm+s, cmp);
			pn = med3(pn-2*s, pn-s, pn, cmp);
		}
		pm = med3(pl, pm, pn, cmp); /* Mid-size, med of 3 */
	}

	PVINIT(pv, pm); /* pv points to partition value */
	pa = pb = a;
	pc = pd = a + (n-1)*es;
	for (;;) {
		while (pb <= pc && (r = cmp(pb, pv)) <= 0) {
			if (r == 0) {
				swap(pa, pb);
				pa += es;
			}
			pb += es;
		}

		while (pc >= pb && (r = cmp(pc, pv)) >= 0) {
			if (r == 0) {
				swap(pc, pd);
				pd -= es;
			}
			pc -= es;
		}

		if (pb > pc) {
			break;
		}

		swap(pb, pc);
		pb += es;
		pc -= es;
	}

	pn = a + n*es;

	s = min(pa-a, pb-pa );
	vecswap(a, pb-s, s);

	s = min(pd-pc, pn-pd-es);
	vecswap(pb, pn-s, s);

	if ((s = pb-pa) > es) {
		nqsort(a, s/es, es, cmp);
	}
	if ((s = pd-pc) > es) {
		nqsort(pn-s, s/es, es, cmp);
	}
} // nqsort
