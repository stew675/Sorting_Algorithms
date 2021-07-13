#include <stddef.h>
#include "oldswap.h"

void
rattle_sort(register char *a, size_t n, register const size_t es, register const int (*cmp)(const void *, const void *))
{
	register char	*b, *c, *hi = a + (n - 1) * es;
	register int	swaptype;

	if (n < 2) return;

	SWAPINIT(a, es);

	for (;;) {
		// Forward sift
		if ((n = (n * 13) / 17) < 2) break;
		for (b = a, c = a + (n * es); c <= hi; b+=es, c+=es)
			if (cmp(b, c) > 0)
				swap(b, c);

		// Backward sift
		if ((n = (n * 13) / 17) < 2) break;
		for (b = hi, c = hi - (n * es); c >= a; b-=es, c-=es)
			if (cmp(b, c) < 0)
				swap(b, c);
	}

	for (register char *pos;;) {
		for (pos = NULL, b = a, c = a + es; c <= hi; b=c, c+=es)
			if (cmp(b, c) > 0) { swap(b, c); pos = b; }
		if ((hi = pos) == NULL) break;

		for (pos = NULL, b = hi, c = hi - es; c >= a; b=c, c-=es)
			if (cmp(b, c) < 0) { swap(b, c); pos = b; }
		if ((a = pos) == NULL) break;
	}
} // rattle_sort
