//				Bidirectional Bubble Sort
//
// Author: Stew Forster (stew675@gmail.com)	Date: 19th July 2021
//
// This implements my version of the https://en.wikipedia.org/wiki/Cocktail_shaker_sort
// variant on Bubble Sort
//
// The start and end pointers are also brought together for sorted sections to reduce
// the size of the sorting partition over time

#include <stddef.h>
#include "oldswap.h"

void
bidir_bubble_sort(register char *a, size_t n, register const size_t es, register const int (*is_less_than)(const void *, const void *))
{
	register char	*b, *c, *e = a + n * es, *s;
	register int	swaptype;

	SWAPINIT(a, es);

	for (;;) {
		for (b=a, c=a+es, s=a; c<e; b+=es, c+=es)
			if (is_less_than(c, b)) { swap(b, c); a = c; }
		if (s == a) return;
		e = a; a = s;

		for (b=e-es, c=b-es, s=e; c>=a; b-=es, c-=es)
			if (is_less_than(b, c)) { swap(b, c); e = c; }
		if (s == e) return;
		a = e; e = s;
	}
} // bidir_bubble_sort
