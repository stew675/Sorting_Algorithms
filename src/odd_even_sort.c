//					Bubble Sort
//
// Author: Stew Forster (stew675@gmail.com)	Date: 19th July 2021
//
// My implementation of the classic Bubble Sort algorithm
//
// The end pointer is brought in for sorted sections to reduce the size of the search
// partition over time.

#include <stddef.h>
#include "swap.h"

void
odd_even_sort(register char *a, size_t n, register const size_t es, register const int (*is_lt)(const void *, const void *))
{
	register char	*b, *c, *e = a + (n - 1) * es, *s;
	register int	swaptype, unsorted = 1;
	register WORD	t;

	SWAPINIT(a, es);

	while (unsorted) {
		unsorted = 0;
		for (b = a + es; b < e; b += es*2)
			if (is_lt(b+es, b)) {
				unsorted = 1;
				swap(b+es, b);
			}
		for (b = a; b < e; b += es*2)
			if (is_lt(b+es, b)) {
				unsorted = 1;
				swap(b+es, b);
			}
	}
} // odd_even_sort
