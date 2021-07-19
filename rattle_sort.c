#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "oldswap.h"

// 4/3 => 1.333333333333		(Best overall balanced performance)
static const uint32_t steps[] = {1, 2, 3, 5, 7, 11, 13, 17, 23, 31, 43, 59, 73, 101, 131, 179, 239, 317, 421, 563, 751, 997, 1327, 1777, 2357, 3137, 4201, 5591, 7459, 9949, 13267, 17707, 23599, 31469, 41953, 55933, 74573, 99439, UINT32_MAX};

int
rattle_sort(register char *a, size_t n, register const size_t es, register const int (*cmp)(const void *, const void *))
{
	register char	*b, *c, *e = a + (n - 1) * es;
	register int	swaptype;

	SWAPINIT(a, es);

	// For arrays of length 12 or less, it's faster to just go straight to bubble sort.  The
	// actual goal of this step here is not to fully sort the data, but rather to move
	// everything closer to where it should be in repeated stages.  I've often thought of it
	// as like rattling a tub of mixed rocks.  The small stones fall to the bottom while the
	// larger rocks rise to the top, hence the name.  The bubble sort below does the finer
	// sifting after everything is close to where it should be.
	if (n > 12) {
		size_t	step = n;
		int	pos = 0;

		#define next_step(_x)	(((_x) > steps[pos+1]) ? (n / steps[++pos]) : steps[--pos])
		for (;;) {
			for (step = next_step(step), b=a, c=a+(step*es); c<=e; b+=es, c+=es)
				if (cmp(b, c) > 0) swap(b, c);
			if (step <= 2) break;

			for (step = next_step(step), b=e, c=e-(step*es); c>=a; b-=es, c-=es)
				if (cmp(b, c) < 0)
					swap(b, c);
			if (step <= 2) break;
		}
		#undef next_step
	}

	// 2-way bubble sort with collapsing ends.  Quickly moves stragglers into position while
	// bringing the start and end pointers in to skip any portions that are already sorted
	do {
		char *saved;
		int passes = 0;

		for (;;) {
			// Forward sift
			for (saved = a, b = a, c = a + es, passes++; c <= e; b=c, c+=es)
				if (cmp(b, c) > 0) { swap(b, c); a = c; }
			if (a == saved) return passes;
			e = a; a = saved;

			// Backward sift
			for (saved = e, b = e, c = e - es, passes++; c >= a; b=c, c-=es)
				if (cmp(b, c) < 0) { swap(b, c); e = c; }
			if (e == saved) return passes;
			a = e; e = saved;
		}
	} while (0);
} // rattle_sort
