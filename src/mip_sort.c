//			Merge In Place Sort
//
// Author: Stew Forster (stew675@gmail.com)	Date: 29 July 2021
//		Adaptive Iterative Merge Sort
//
// Implements a bottom up in-place merge sort

#include <stddef.h>
#include "swap.h"

// The following value is arbitrary, but appears to be optimal
#define INSERT_THRESH	14

typedef const int (*ilt)(const void *, const void *);

// Both sorted sub-arrays must be adjacent in a
// an is the length of the first sorted section in a
// bn is the length of the second sorted section in a
static void
merge_inplace(register char *a, size_t an, size_t bn, register size_t es, register ilt is_less_than, register int swaptype)
{
	register char	*b = a+an*es, *e = b+bn*es, *s;
	register WORD   t;

	// Return right now if we're done
	if (an==0 || bn==0 || !is_less_than(b, b-es))
		return;

	// Do insertion sort to merge if the size of the sub-arrays are small enough
	if (an < INSERT_THRESH && an <= bn && bn < (INSERT_THRESH * 2)) {
		// Insert Sort A into B
		for (register char *p=b, *v; p>a; p-=es)
			for(v=p, s=p-es; v<e && is_less_than(v, s); s=v, v+=es)
				swap(s, v);
		return;
	}

	if (bn < INSERT_THRESH && an < (INSERT_THRESH * 2)) {
		// Insert Sort B into A
		for (register char *p=b, *v; p<e; p+=es)
			for (s=p, v=p-es; s>a && is_less_than(s, v); s=v, v-=es)
				swap(s, v);
		return;
	}

	// Find the pivot points.
	register char *pa=a, *pb=b;

	for (s=a; s<b && pb<e; s+=es)
		if (is_less_than(pb, pa))
			pb+=es;
		else
			pa+=es;
	pa+=(b-s);

	// Swap first part of b with last part of a
	for (register char *la=pa, *fb=b; la<b; la+=es, fb+=es)
		swap(la, fb);

	// Now merge the two sub-array pairings
	if (pa>a)
		merge_inplace(a, (pa-a)/es, (pb-b)/es, es, is_less_than, swaptype);
	merge_inplace(b, (pb-b)/es, (e-pb)/es, es, is_less_than, swaptype);
} // merge_inplace

static void
_mip(register char *a, size_t n, register size_t es, register ilt is_less_than, register int swaptype)
{
	size_t m = (n+1)/2;

	// Sort first and second halves
	if (m > 1)
		_mip(a, m, es, is_less_than, swaptype);

	if (n-m>1)
		_mip(a+m*es, n-m, es, is_less_than, swaptype);

	// Now merge the two sorted sub-arrays together
	merge_inplace(a, m, n-m, es, is_less_than, swaptype);
} // _mip

void
mip_sort(register char *a, size_t n, register const size_t es, register ilt is_less_than)
{
	int	swaptype;

	SWAPINIT(a, es);

	_mip(a, n, es, is_less_than, swaptype);
} // mip_sort
