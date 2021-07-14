#include <stddef.h>
#include "oldswap.h"

#define MULT(x)    (((x)*13)/17)

void
rattle_sort(register char *a, size_t n, register const size_t es, register const int (*cmp)(const void *, const void *))
{
	register char	*b, *c, *e = a + (n - 1) * es;
	register int	swaptype;

	if (n < 2)
		return;
	n = n/2;

	SWAPINIT(a, es);

	// Attempt to move elements fairly close to where they should finally be
	for(;;) {
		// Forward sift
		for (b=a, c=a+(n*es); c<=e; b+=es, c+=es)
			if (cmp(b, c) > 0)
				swap(b, c);
		if ((n=MULT(n)) < 2) break;

		// Backward sift
		for (b=e, c=e-(n*es); c>=a; b-=es, c-=es)
			if (cmp(b, c) < 0)
				swap(b, c);
		if ((n=MULT(n)) < 2) break;
	}

	// 2-way bubble sort with closing ends to mop up any stragglers
	for (char *saved;;) {
		// Forward sift
		for (saved = a, b = a, c = a + es; c <= e; b=c, c+=es)
			if (cmp(b, c) > 0) { swap(b, c); a = c; }
		if (a == saved) break;
		e = a; a = saved;

		// Backward sift
		for (saved = e, b = e, c = e - es; c >= a; b=c, c-=es)
			if (cmp(b, c) < 0) { swap(b, c); e = c; }
		if (e == saved) break;
		a = e; e = saved;
	}
} // rattle_sort
