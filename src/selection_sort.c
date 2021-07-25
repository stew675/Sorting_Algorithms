//			Selection Sort
//
// Author: Stew Forster (stew675@gmail.com)	Date: 25 July 2021
//
// My implementation of the classic Insertion Sort algorithm
//

#include <stddef.h>
#include "swap.h"

void
selection_sort(register char *a, size_t n, register const size_t es, register const int (*is_less_than)(const void *, const void *))
{
	register char	*p, *l, *s, *e = a + n * es;
	register int	swaptype;
	register WORD	t;

	SWAPINIT(a, es);
	for (p = a; p < e; p+=es) {
		for(l=p, s=p+es; s<e; s+=es) {
			if (is_less_than(s, l)) {
				l = s;
			}
		}
		if (l != p) {
			swap(p, l);
		}
	}
} // selection_sort
