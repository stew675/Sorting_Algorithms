//					Bubble Sort
//
// Author: Stew Forster (stew675@gmail.com)	Date: 19th July 2021
//
// My implementation of the classic Bubble Sort algorithm
//
// The end pointer is brought in for sorted sections to reduce the size of the search
// partition over time.

#include <stddef.h>
#include "oldswap.h"

void
bubble_sort(register char *a, size_t n, register const size_t es, register const int (*cmp)(const void *, const void *))
{
	register char	*b, *c, *e = a + n * es, *s;
	register int	swaptype;

	SWAPINIT(a, es);

	for (;;) {
		for (s=a, b=a, c=a+es; c<e; b+=es, c+=es)
			if (cmp(b, c) > 0) { swap(b, c); a = c; }
		if (s == a) return;
		e = a; a = s;
	}
} // bubble_sort
