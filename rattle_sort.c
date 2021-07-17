#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "oldswap.h"

#define MULT(x)    (((x)*13)/17)

static inline size_t
get_next_n(size_t n, int *pass)
{
	// The first value is the trigger point after which the manual step system takes over
	static size_t steps[] = {10700, 6296, 4881, 3426, 2643, 1885, 1439, 903, 583, 420,	// Measured Results
				 293, 229, 160, 125, 93, 65, 42, 26, 17, 11, 7, 5, 3, 2, 1};

	if (n < steps[0])
		return steps[++(*pass)];

	// Not using pre-set steps, just follow a regular geometric progression instead
	return MULT(n);
} // get_next_n


void
rattle_sort(register char *a, size_t n, register const size_t es, register const int (*cmp)(const void *, const void *))
{
	register char	*b, *c, *e = a + (n - 1) * es;
	register int	swaptype;
	int pass = 0;

	if (n < 2)
		return;
	if ((n *= 0.3512) < 2) n = 2;	// Most generally optimal for first pass
	SWAPINIT(a, es);

	// Attempt to move elements fairly close to where they should finally be
	for (;;) {
		// Forward sift
		for (b=a, c=a+(n*es), n = get_next_n(n, &pass); c<=e; b+=es, c+=es)
			if (cmp(b, c) > 0)
				swap(b, c);
		if (n < 2) break;

		// Backward sift
		for (b=e, c=e-(n*es), n = get_next_n(n, &pass); c>=a; b-=es, c-=es)
			if (cmp(b, c) < 0)
				swap(b, c);
		if (n < 2) break;
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
