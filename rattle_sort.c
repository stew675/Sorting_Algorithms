//					Rattle Sort
//
// Altering Bubble Sort to be O(n log n) time complexity for the best, worst, and average cases.
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
// step-sizes themselves.  This guarantees no set-size degeneracy for pretty much any value
// of n, and therefore the input is almost always near-fully sorted by the time a step-size
// of 1 is reached, after which the algorithm acts as a 2-way bubble sort with collapsing
// ends and quickly mops up any remaining positional stragglers
//
// As such the algorithm is O(n logn) where the log base is 1.333 and the number of compares
// is uniform for all step sizes >1.  Technically, due to the final stage bubble-sort, the
// algorithm could be claimed to be O(n^2) in the worst-case, but I have difficulty conceiving
// how such a worst-case data set could be constructed given the sheer number of prime-based
// steps involved.  Values would need to remain near invariant with respect to ther locality
// across a mind-boggingly large greatest-common-multiple set space to arrive at a poorly
// sorted set by the time we reach the step=1 bubble-sort phase.  I have yet to personally
// witness anything other than O(n logn) behavior for n>100 with time-variancy mostly occuring
// due to the number of swaps required
//
// The results below demonstrate this effect.  I brute-forced every permutation for array
// sizes up to 14 (15 and 16 still calculating) and the worst-case number of swaps does not
// follow an O(n^2) sequence.  I experimentally hammered the algorithm with trillions of
// input sets for larger values of n, and found that the worst-case number of swaps actually
// approaches the average case as n is raised.
//
// Worst Case Brute-Force results in terms of total SWAPS required to sort
// N	SWAPS	nPERM	AVG SW	nWORST	SAMPLE WORST CASE SET
// 16		16!
// 15		15!
// 14	48	14!	20.605	  1	[12, 14, 6, 2, 11, 8, 5, 10, 13, 4, 1, 9, 7, 3]
// 13	43	13!	18.413	 28	[4, 13, 5, 9, 7, 11, 3, 12, 2, 8, 6, 10, 1]
// 12	41	12!	16.062	  1	[12, 6, 8, 2, 10, 4, 11, 5, 7, 1, 9, 3]
// 11	29	11!	13.298	  6	[4, 11, 5, 8, 9, 3, 10, 2, 7, 6, 1]
// 10	26	10!	11.195	  2	[10, 4, 7, 8, 3, 9, 2, 6, 5, 1]
// 9	19	 9!	 9.206	  2	[8, 9, 2, 4, 7, 5, 1, 3, 6]
// 8	20	 8!	 8.324	  1	[8, 4, 6, 2, 7, 3, 5, 1]
// 7	14	 7!	 7.433	  9	[3, 7, 5, 2, 6, 4, 1]
// 6	12	720	 5.500	  1	[6, 4, 2, 5, 3, 1]
// 5	7	120	 3.800	  4	[3, 5, 2, 4, 1]
// 4	5	24	 2.333	  1	[4, 2, 3, 1]
// 3	3	6	 1.500	  1	[3, 2, 1]
// 2	1	2	 0.500	  1	[2, 1]
// 1	0	1	 0.000	  1	[1]
//
// Since the number of permutations is O(n!) then values for N above 16 become prohibitively
// expensive to computationally brute-force to find the worst case set

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
	register char	*b, *c, *e = a + n * es, *s;
	register int	swaptype;
	size_t		step = n;
	int		pos = 0;

	SWAPINIT(a, es);

#define next_step       ((step > steps[pos+1]) ? (n / steps[++pos]) : (pos > 0 ? steps[--pos] : 1))
	for (;;) {
		for (step = next_step, b=a, c=a+(step*es), s = a; c<e; b+=es, c+=es)
			if (cmp(b, c) > 0) { swap(b, c); a = b; }
		if (step == 1) { if (s == a) { return; } else { e = a; } }
		a = s;

		for (step = next_step, b=e-es, c=b-(step*es), s = e; c>=a; b-=es, c-=es)
			if (cmp(b, c) < 0) { swap(b, c); e = b; }
		if (step == 1) { if (s == e) { return; } else { a = e; } }
		e = s;
	}
#undef next_step
} // rattle_sort
