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
// Extensive brute-force empirical analysis found that the 4/3 ratio was the natural cadence
// of RattleSort's step progression
//
// The step-set implementation offers something of a twist to how step-sizes are generated.
// Initially step-sizes are calculated as n/steps[pos] up until step exceeds sqrt(n), after
// which point we reverse direction in the step-set and the step-dividers become the actual
// step-sizes themselves.  This guarantees no set-size degeneracy for pretty much any value
// of n, and therefore the input is almost always near-fully sorted by the time a step-size
// of 1 is reached, after which the algorithm acts as a 2-way bubble sort with collapsing
// ends and quickly mops up any remaining positional stragglers
//
// As such the algorithm is O(N log₁.₃₃N) where the log base is 1.333 and the number of compares
// is uniform for all step sizes >1.  Technically, due to the final stage bubble-sort, the
// algorithm could be claimed to be O(n^2) in the worst-case, but I have difficulty conceiving
// how such a worst-case data set could be constructed given the sheer number of prime-based
// steps involved.  Values would need to remain near invariant with respect to ther locality
// across a mind-boggingly large greatest-common-multiple set space to arrive at a poorly
// sorted set by the time we reach the step=1 bubble-sort phase.  I have yet to personally
// witness anything other than O(n logn) behavior for n>100 with time-variancy mostly occuring
// due to the number of swaps required.  It is my running supposition that the number of
// 1-step passes will never exceed ceil(log₁.₃₃N) or ceil(log(n)/log(4.0/3.0)) and to date
// neither any brute force scenarios, nor have any random data sets ever violated this
// supposition with the peak number of 1-step passes appearing to diminish for large values
// for n
//
// Therefore, it is posited that the absolute worst case consists of log₁.₃₃N steps of step
// sizes greater than 1, and up to log₁.₃₃N 1-step passes.  Since each pass traverses, at
// most, N elements (often much less), this creates a worst case of 2*N*log₁.₃₃N compares
// and the number of swaps is, of course, capped by the number of compares.
//
// The results below demonstrate this effect.  I brute-forced every permutation for array
// sizes up to 14 (15 and 16 still calculating) and the worst-case number of swaps does not
// follow an O(n^2) sequence.  I experimentally hammered the algorithm with ~1 trillion
// input sets for larger values of n, and found that the worst-case number of swaps actually
// approaches the average case as n is raised.
//
// Worst Case Brute-Force results in terms of total SWAPS required to sort
//		 MAX	 MAX	nPERM	 AVG	nWORST	SAMPLE WORST CASE SET
//	 N	1-PASS	SWAPS		SWAPS    SETS
//	16	 6	 61	16!	26.564	 17	[15, 16, 6, 10, 2, 8, 13, 5, 14, 11, 4, 9, 1, 7, 12, 3]
//	15	 6	 52	15!     23.511	 57	[8, 15, 5, 12, 10, 4, 13, 7, 14, 2, 9, 6, 3, 11, 1]
//	14	 6	 48	14!	20.605	  1	[12, 14, 6, 2, 11, 8, 5, 10, 13, 4, 1, 9, 7, 3]
//	13	 7	 43	13!	18.413	 28	[4, 13, 5, 9, 7, 11, 3, 12, 2, 8, 6, 10, 1]
//	12	 7	 41	12!	16.062	  1	[12, 6, 8, 2, 10, 4, 11, 5, 7, 1, 9, 3]
//	11	 5	 29	11!	13.298	  6	[4, 11, 5, 8, 9, 3, 10, 2, 7, 6, 1]
//	10	 4	 26	10!	11.195	  2	[10, 4, 7, 8, 3, 9, 2, 6, 5, 1]
//	 9	 3	 19	 9!	 9.206	  2	[8, 9, 2, 4, 7, 5, 1, 3, 6]
//	 8	 5	 20	 8!	 8.324	  1	[8, 4, 6, 2, 7, 3, 5, 1]
//	 7	 4	 14	 7!	 7.433	  9	[3, 7, 5, 2, 6, 4, 1]
//	 6	 4	 12	720	 5.500	  1	[6, 4, 2, 5, 3, 1]
//	 5	 3	  7	120	 3.800	  4	[3, 5, 2, 4, 1]
//	 4	 3	  5	 24	 2.333	  1	[4, 2, 3, 1]
//	 3	 3	  3	  6	 1.500	  1	[3, 2, 1]
//	 2	 2	  1	  2	 0.500	  1	[2, 1]
//	 1	 1	  0	  1	 0.000	  1	[1]
//
// Since the number of permutations is O(n!) then values for N above 16 become prohibitively
// expensive to computationally brute-force to find the worst case set, I had to resort to
// anecdotal/empirical result gathering, the results of which are presented below
//
// Worst Case for number of 1-step passes seen anecdotally with random data sets.  Most runs
// typically consist of 100M-10B randomly generated data sets of unique elements, which are
// then sorted, and the maximum number of 1-step passes seen is then recorded
//
//		 MAX	 MAX	 AVG
//	  N	1-PASS	SWAPS	SWAPS	   Notes
//	9973	  7	70419	68331	* 100M passes
//	5000	  8	32696	31344	* 100M passes
//	2000	  8	12215	11246	* 100M passes
//	1000	  8	 5593	 4965	* 100M passes
//	 500	  9	 2762	 2171	* 1B passes
//	 200	  9	 1065	  720	* 1B passes
//	 100	 10	  436	  300	* 1B passes
//	  50	  9	  205	  118	* 1B passes
//	  40	  9	  182	   91	* 10B passes
//	  30	  9	  133	 62.4	* 10B passes
//	  20	  8	   74	 31.5	* 10B passes
//	  18	  8	   73	29.494	* 10B passes
//	  16	  6	   59	26.564	* 10B passes
//	  15	  6	   51	23.511	* 100B passes
//	  14	  6	   46	20.605	* Results closely match exhaustive brute force observation
//	  13	  7	   43	18.413	* Results perfectly match exhaustive brute force observation
//	  12	  7	   41   16.062	* Results perfectly match exhaustive brute force observation
//	  11	  5	   29	11.298	* Results perfectly match exhaustive brute force observation

#include <stddef.h>
#include <stdint.h>
#include "newswap.h"

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

#define next_step       ((step > steps[pos+1]) ? (n / steps[++pos]) : (pos > 0 ? steps[--pos] : 1))
	for (;;) {
		for (step = next_step, b=a, c=a+(step*es), s = a; c<e; b+=es, c+=es)
			if (cmp(b, c) > 0) { swap(b, c, es); a = c; }
		if (step == 1) { if (s == a) { return; } else { e = a; } }
		a = s;

		for (step = next_step, b=e-es, c=b-(step*es), s = e; c>=a; b-=es, c-=es)
			if (cmp(b, c) < 0) { swap(b, c, es); e = c; }
		if (step == 1) { if (s == e) { return; } else { a = e; } }
		e = s;
	}
#undef next_step
} // rattle_sort
