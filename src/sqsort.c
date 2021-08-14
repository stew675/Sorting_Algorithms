//			Stew's optimised quick sort implementation
//
// Author: Stew Forster (stew675@gmail.com)			Date: 9th Aug 2021
//
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "swap.h"

#define med3(a, b, c)	(is_lt(a, b) ?                            \
			(is_lt(b, c) ? b : is_lt(a, c) ? c : a) : \
			(is_lt(c, b) ? b : is_lt(c, a) ? c : a)) 

static inline char *
partition(register char *a, size_t n, register const size_t es,
          register const int (*is_lt)(const void *, const void *), register int swaptype)
{
	// e should point AT the last element in the array a
	register char	*e=a+(n-1)*es, *p=a+(n/2)*es;
	register WORD	t;

	// Select a pivot point using median of 3
	p = med3(a, p, e);

	// Do a pseudo median-of-9 for larger partitions
	if (n > 63) {
		register size_t ne = (n/8)*es;
		register char  *pl = med3(a+ne*2,  a+ne,  a+ne*3);
		register char  *pr = med3(a+ne*5, a+ne*6, a+ne*7);

		p = med3(pl, p, pr);
	}

	// Move the pivot value to the last element in the array
	// so it doesn't move about
	if (p != e)
		swap(p, e);

	// Now partition the array around the pivot point's value
	// Remember: e contains the pivot value
	for (p=e; a<p; a+=es)
		if (is_lt(e, a))
			for (p-=es; p!=a; p-=es)
				if (is_lt(p, e)) {
					swap(a, p);
					break;
				}

	// Move the pivot point into position
	if (p != e)
		swap(p, e);

	// Return a pointer to the partition point
	return p;
} // partition


static void
_sqsort(register char *a, size_t n, register const size_t es,
        register const int (*is_lt)(const void *, const void *), register int swaptype)
{
	for (;;) {
		register char	*e = a + n * es;

		// Insertion Sort
		if (n < 19) {
			register char	*p, *s, *v;
			register WORD	t;
			char tmp[es];

			for (s=a, p = a+es; p < e; s=p, p+=es)
				if (is_lt(p, s)) {
					copy(tmp, p);
					copy(p, s);
					for (v = s, s-=es; v > a && is_lt(tmp, s); v = s, s-=es)
						copy(v, s);
					copy(v, tmp);
				}
			return;
		}

		// Quick Sort
		register char *p = partition(a, n, es, is_lt, swaptype);

		// We only recurse on the smaller of the 2 partitions, and just restart the
		// loop on the larger of the two.  This keeps recursion depth very minimal
		if (p-a > e-p) {
			_sqsort(p+es, n - (((p+es)-a)/es), es, is_lt, swaptype);

			// Don't recurse, just restart the loop with the updated value for n
			n = (p-a)/es;
		} else {
			_sqsort(a, (p-a)/es, es, is_lt, swaptype);

			// Rather than recurse here, just restart the loop.  This is the same
			// same as doing the following, just without the actual function call
			//   _sqsort(p+es, n - (((p+es)-a)/es), es, is_lt, swaptype);
			p += es;
			n -= (p-a)/es;
			a = p;
		}

	}
} // _sqsort


// My Implementation of qsort
void
sqsort(register char *a, size_t n, register const size_t es,
       register const int (*is_lt)(const void *, const void *))
{
	int swaptype;

	if (n < 2)
		return;

	SWAPINIT(a, es);

	// Perform a recursive sqsort
	_sqsort(a, n, es, is_lt, swaptype);
} // sqsort
