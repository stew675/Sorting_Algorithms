#include <stddef.h>
#include "oldswap.h"

// Classical comb sort implementation using a 1.30 divider
// So simple...so fast
void
comb_sort(char *a, register size_t n, register const size_t es, register const int (*cmp)(const void *, const void *))
{
	register char	*b, *c, *e = a + (n * es);
	register int	swaptype, swapped;

	SWAPINIT(a, es);
	do {
		for (n = (n > 1) ? (n / 1.3) : 1, b=a, c=b+(n*es), swapped = 0; c<e; b+=es, c+=es)
			if ((cmp(b, c) > 0) && (swapped = 1))
				swap(b, c);
	} while (swapped || (n > 1));
} // comb_sort

