#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include "swap.h"

// Classical comb sort implementation using a 1.30 divider
// So simple...so fast
void
comb_sort(char *a, register size_t n, register const size_t es, register const int (*is_lt)(const void *, const void *))
{
	register char	*b, *c, *s, *e = a + (n * es);
	register bool	swapped = true;
	register int	swaptype;
	register WORD	t;

	if (n < 2)
		return;

	SWAPINIT(a, es);

	for (n=(n*10)/13; n>1 && (swapped || n>1); n=(n>1)?((n*10)/13):1)
		for (b=a, c=b+(n*es), swapped=false; c<e; b+=es, c+=es)
			if (is_lt(c, b) && (swapped = true))
				swap(b, c);

	// Use an insertion sort for the final step size of 1 as it's faster
        for (b=a, c=a+es; c<e; b=c, c+=es)
                for (s=c; s>a && is_lt(s, b); s=b, b-=es)
                        swap(b, s);
} // comb_sort
