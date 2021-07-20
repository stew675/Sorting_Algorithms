// Rattle Sort
//
// Author: Stew Forster (stew675@gmail.com)	Date: 19th July 2021
//
// Essentially a bi-directional comb-sort but with an algorithmically optimal step progression.
// Comb-sort tops out at a 1.3x ratio, but rattle-sort achieves a 1.333x ratio, meaning that it
// scales better for medium+ (>10K) n-values and is generally slightly faster in all scenarios
// Faster than median-of-3 q-sort for n values up to 1600 when sorting integers/pointers.  It
// gets progressively slower than such qsort implementations for very large n-values but still
// usually stays within a 1.0-1.25x time-to-run range of qsort even up to n=100M
//
// The step-set implementation offers something of a twist to how step-sizes are generated.
// Initially step-sizes are calculated as n/steps[pos] up until step exceeds sqrt(n), after
// which point we reverse direction in the step-set and the step-dividers become the actual
// step-sizes themselves.  This almost guarantees no set-size degeneracy for pretty much any
// value of n, and therefore the input is almost always near-fully sorted by the time a
// step-size of 1 is reached, after which the algorithm acts as a 2-way bubble sort and quickly
// mops up positional stragglers
//
// As such the algorithm is O(n logn) where the log base is 1.333 and the number of compares
// is uniform for all step sizes >1.  Technically, due to the final stage bubble-sort, the
// algorithm is O(n^2) worst-case, but I have difficulty conceiving how such a worst-case
// data set could be constructed given the sheer number of prime-based steps involved.  Values
// would need to remain near invariant with respect to ther locality across a mind-boggingly
// large greatest-common-multiple set to arrive at a poorly sorted set by the time we reach
// the step=1 bubble-sort phase.  I have yet to personally witness anything other than O(n logn)
// behavior for n>100 with time-variancy mostly occuring due to the number of swaps required

#include <stddef.h>
#include "oldswap.h"

// 4/3 => 1.333333333333
// Best overall balanced performance.  The 4294967295 value acts as a sentinel.  This step set supports
// n values up to 10^10.  Higher n values will still work, the algorithm will just start to slow down.
// The step set is expanded with more values, but an n value of 10^10 should satisfy most things
static const size_t steps[] = {1, 2, 3, 5, 7, 11, 13, 17, 23, 31, 43, 59, 73, 101, 131, 179, 239, 317, 421, 563, 751, 997, 1327, 1777,
			       2357, 3137, 4201, 5591, 7459, 9949, 13267, 17707, 23599, 31469, 41953, 55933, 74573, 99439, 4294967295};

void
rattle_sort(register char *a, size_t n, register const size_t es, register const int (*cmp)(const void *, const void *))
{
	register char	*b, *c, *e = a + n * es;
	register int	swaptype, swapped;
	size_t		step = n;
	int		pos = 0;

	SWAPINIT(a, es);

#define next_step       ((step > steps[pos+1]) ? (n / steps[++pos]) : (pos > 0 ? steps[--pos] : 1))
	do {
		for (step = next_step, b=a, c=a+(step*es), swapped = 0; c<e; b+=es, c+=es)
			if ((cmp(b, c) > 0) && (swapped = 1))
				swap(b, c);

		if (swapped || step > 1)
			for (step = next_step, b=e-es, c=b-(step*es), swapped = 0; c>=a; b-=es, c-=es)
				if ((cmp(b, c) < 0) && (swapped = 1))
					swap(b, c);
	} while (swapped || step > 1);
#undef next_step
} // rattle_sort
