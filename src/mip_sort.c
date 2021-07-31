//			Stable Merge In Place Sort
//
// Author: Stew Forster (stew675@gmail.com)		Date: 29 July 2021
//
// Implements a stable in-place merge sort

#include <stddef.h>
#include "swap.h"

// The following values are arbitrary, but appear to be optimal
// Choose values of 1 to run the sort without insertion sorts
#define MII_THRESH	16
#define MSI_THRESH	24

typedef const int (*ilt)(const void *, const void *);

// Both sorted sub-arrays must be adjacent in 'a'
// Assumes that both 'an' and 'bn' are always non-zero
// 'an' is the length of the first sorted section in 'a', referred to as A
// 'bn' is the length of the second sorted section in 'a', referred to as B
static void
merge_inplace(register char *a, register size_t an, size_t bn, register size_t es, register ilt is_less_than, register int swaptype)
{
	register char	*b = a+an*es, *pa = b-es;

	// If the first element of B is not less then the last element
	// of A, then since A and B are in order, it naturally follows
	// that [A, B] must also all be in order and we're done here
	if (!is_less_than(b, pa))
		return;

	register char 	*e = b+bn*es, *pb=b+es, *s, *v;
	register WORD   t;

	// Do insertion sort to merge if the collective size of the sub-arrays are small enough
	if ((an + bn) < MII_THRESH) {
		if (bn < an) {
			for (; b<e; b+=es)	// Insert Sort B into A
				for (s=b, v=b-es; s>a && is_less_than(s, v); s=v, v-=es)
					swap(s, v);
		} else {
			for (; b>a; b-=es)	// Insert Sort A into B
				for(v=b, s=b-es; v<e && is_less_than(v, s); s=v, v+=es)
					swap(s, v);
		}
		return;
	}

	// Find the portion to swap.  We already know that the first element
	// of B is less than the last element of A, so we skip that comparison
	for(; pa>a && pb<e && is_less_than(pb, pa-es); pa-=es, pb+=es);

	// Swap last part of a with first part of b
	for (s=pa, v=b; s<b; s+=es, v+=es)
		swap(s, v);

	// Now merge the two sub-array pairings.  We know that (pb-b) > 0 but we
	// need to check that either array didn't wholly swap out the other and
	// cause the remaining portion to be zero
	if (pa>a)
		merge_inplace(a, (pa-a)/es, (pb-b)/es, es, is_less_than, swaptype);

	if (e>pb)
		merge_inplace(b, (pb-b)/es, (e-pb)/es, es, is_less_than, swaptype);
} // merge_inplace

// Implements a recursive merge-sort algorithm with an optional
// insertion sort for when the splits get too small.  'n' must
// ALWAYS be 2 or more.  It enforces this when calling itself
static void
merge_sort(register char *a, size_t n, register size_t es, register ilt is_less_than, register int swaptype)
{
	size_t m = n/2;

	// Do an insertion sort on the rest if 'n' is small enough
	if (n < MSI_THRESH) {
		register char *p, *s, *v, *e=a+n*es;
		register WORD   t;

		for (p=a+es; p<e; p+=es)
			for(s=p, v=p-es; s>a && is_less_than(s, v); s=v, v-=es)
				swap(s, v);
		return;
	}

	// Sort first and second halves only if the target 'n' will be > 1
	if (m > 1)
		merge_sort(a, m, es, is_less_than, swaptype);

	if ((n-m)>1)
		merge_sort(a+m*es, n-m, es, is_less_than, swaptype);

	// Now merge the two sorted sub-arrays together. We know that since
	// n > 1, then both m and n-m MUST be non-zero, and so we will never
	// violate the condition of not passing in zero length sub-arrays
	merge_inplace(a, m, n-m, es, is_less_than, swaptype);
} // merge_sort

// Really just a wrapper for merge_sort() to sanity check input, and set up
// the swap functionality
void
mip_sort(register char *a, size_t n, register const size_t es, register ilt is_less_than)
{
	int	swaptype;

	if (( a== NULL) || (n < 2) || (es == 0))
		return;

	SWAPINIT(a, es);

	merge_sort(a, n, es, is_less_than, swaptype);
} // mip_sort
