//			Stable Merge In Place Sort
//
// Author: Stew Forster (stew675@gmail.com)		Date: 29 July 2021
//
// Implements a stable in-place merge sort

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "swap.h"

// The following values are arbitrary, but appear to be optimal
// Choose values of 1 to run the sort without insertion sorts
#define MII_THRESH	16
#define MSI_THRESH	24

// The minimum number of bytes to swap before we switch to using memcpy
#define BLK_THRESH	128

typedef const int (*ilt)(const void *, const void *);

// Used for larger memory swaps
// Takes advantage of any vectorization in the optimized memcpy library functions
static void
swap_blk(char *a, char *b, size_t n)
{
	size_t tc;
	__attribute__((aligned(64))) char t[1024];

	do {
		tc = n > 1024 ? 1024 : n;
		memcpy(t, a, tc);
		memcpy(a, b, tc);
		memcpy(b, t, tc);
		a+=tc;
		b+=tc;
		n-=tc;
	} while (n > 0);
} // swap_blk


// Both sorted sub-arrays must be adjacent in 'a'
// Assumes that both 'an' and 'bn' are always non-zero upon entry
// 'an' is the length of the first sorted section in 'a', referred to as A
// 'bn' is the length of the second sorted section in 'a', referred to as B
static void
merge_inplace(register char *a, register size_t an, size_t bn, register size_t es, register ilt is_less_than, register int swaptype)
{
	register char	*b = a+an*es;

	// If the first element of B is not less then the last element
	// of A, then since A and B are in order, it naturally follows
	// that [A, B] must also all be in order and we're done here
	if (!is_less_than(b, b-es)) {
		return;
	}

	register char 	*e = b+bn*es;

	// Use insertion sort to merge if the size of the sub-arrays is small enough
	if ((an + bn) < MII_THRESH) {
		register WORD   t;

		if (bn < an) {
			for (register char *s, *v; b<e; b+=es)	// Insert Sort B into A
				for (s=b, v=b-es; s>a && is_less_than(s, v); s=v, v-=es)
					swap(s, v);
		} else {
			for (register char *s, *v; b>a; b-=es)	// Insert Sort A into B
				for (v=b, s=b-es; v<e && is_less_than(v, s); s=v, v+=es)
					swap(s, v);
		}
		return;
	}

	// Find the portion to swap.  We already know that the first element
	// of B is less than the last element of A, so we skip that comparison
	// We do a binary search here for speed as most smaller sizes will
	// have already been mopped up by the insertion sort intercept above
	register char 	*pa, *pb;
	{
		register size_t max = bn > an ? an : bn;
		register size_t min = 1, sn = (max + min) / 2;

		pa = b - sn*es;
		pb = b + sn*es;

		for (; min < max; sn = (max + min) / 2, pa = b - sn*es, pb = b + sn*es)
			if (is_less_than(pb, pa-es))
				min = sn + 1;
			else
				max = sn;
	}

	// Now swap last part of A with first part of B
	if (pb-b < BLK_THRESH) {
		register WORD   t;

		for (register char *s=pa, *v=b; s<b; s+=es, v+=es)
			swap(s, v);
	} else {
		swap_blk(pa, b, pb-b);
	}

	// Now recursively merge the two sub-array pairings.  We know that
	// (pb-b) > 0 but we need to check that either array didn't wholly
	// swap out the other and cause the remaining portion to be zero
	if (an < bn) {
		if (pa>a) merge_inplace(a, (pa-a)/es, (pb-b)/es, es, is_less_than, swaptype);
		if (e>pb) merge_inplace(b, (pb-b)/es, (e-pb)/es, es, is_less_than, swaptype);
	} else {
		if (e>pb) merge_inplace(b, (pb-b)/es, (e-pb)/es, es, is_less_than, swaptype);
		if (pa>a) merge_inplace(a, (pa-a)/es, (pb-b)/es, es, is_less_than, swaptype);
	}
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
mip_sort(char *a, size_t n, const size_t es, ilt is_less_than)
{
	int	swaptype;

	if (( a== NULL) || (n < 2) || (es == 0))
		return;

	SWAPINIT(a, es);

	merge_sort(a, n, es, is_less_than, swaptype);
} // mip_sort
