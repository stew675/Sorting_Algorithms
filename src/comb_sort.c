#include <stddef.h>
#include <stdbool.h>
#include "newswap.h"

// Classical comb sort implementation using a 1.30 divider
// So simple...so fast
void
comb_sort(char *a, register size_t n, register const size_t es, register const int (*cmp)(const void *, const void *))
{
	register char	*b, *c, *e = a + (n * es);
	register bool	swapped;

	do {
		for (n=(n>1)?((n*10)/13):1, b=a, c=b+(n*es), swapped=false; c<e; b+=es, c+=es)
			if ((cmp(b, c) > 0) && (swapped=true)) { swap(b, c, es); }

//		if (swapped || (n > 1))
//			for (n=(n>1)?((n*13)/17):1, b=e-es, c=b-(n*es), swapped=false; c>=a; b-=es, c-=es)
//				if ((cmp(b, c) < 0) && (swapped=true)) { swap(b, c, es); }
	} while (swapped || (n > 1));
} // comb_sort

