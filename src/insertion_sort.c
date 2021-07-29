//			Insertion Sort
//
// Author: Stew Forster (stew675@gmail.com)	Date: 25 July 2021
//
// My implementation of the classic Insertion Sort algorithm
//

#include <stddef.h>
#include "swap.h"

void
insertion_sort(register char *a, size_t n, register const size_t es, register const int (*is_less_than)(const void *, const void *))
{
	register char	*p, *s, *e = a + n * es;
	register int	swaptype;
	register WORD	t;

	SWAPINIT(a, es);
	for (p = a+es; p < e; p+=es)
		for(s=p; (s>a) && is_less_than(s, s-es); s-=es)
			swap(s, s-es);
} // insertion_sort