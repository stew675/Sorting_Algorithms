#include <stddef.h>
#include <stdbool.h>
#include "newswap.h"

// Classical comb sort implementation using a 1.30 divider
// So simple...so fast
void
comb_sort(char *a, register size_t n, register const size_t es, register const int (*is_less_than)(const void *, const void *))
{
	register char	*b, *c, *e = a + (n * es);
	register bool	swapped;

	do {
		for (n=(n>1)?((n*10)/13):1, b=a, c=b+(n*es), swapped=false; c<e; b+=es, c+=es)
			if ((is_less_than(c, b)) && (swapped=true)) { swap(b, c, es); }
	} while (swapped || (n > 1));
} // comb_sort

