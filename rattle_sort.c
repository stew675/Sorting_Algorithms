#include <stddef.h>
#include "oldswap.h"

void
rattle_sort(register char *a, size_t n, register const size_t es, register const int (*cmp)(const void *, const void *))
{
	register char	*b, *c, *e = a + (n - 1) * es;
	register int	swaptype;

	if (n < 2) return;

	SWAPINIT(a, es);

	for (;;) {
		// Forward sift
		if ((n = (n * 13) / 17) < 2) break;
		for (b = a, c = a + (n * es); c <= e; b+=es, c+=es)
			if (cmp(b, c) > 0)
				swap(b, c);

		// Backward sift
		if ((n = (n * 13) / 17) < 2) break;
		for (b = e, c = e - (n * es); c >= a; b-=es, c-=es)
			if (cmp(b, c) < 0)
				swap(b, c);
	}

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
