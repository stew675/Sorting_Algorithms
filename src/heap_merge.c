//					MERGE SORT
//
// Author: Stew Forster (stew675@gmail.com)		Date: 23 July 2021
//
// My implementation of merge sort using N/2 extra space as required
//
// This version runs merge sort down to SORT_THRESH partition sizes, and
// under those sizes, it will kick off an alternative sort algorithm.  For
// this implementation, that is my version of Heap Sort, however any
// general purpose sorting algorithm can be used.
//
// If it is desired to have the algorithm run as a pure merge sort, then
// lower the SORT_THRESH value to 1 and it shall be so
//
// TODO: Investigate if using insertion or bubble sort is faster for small
// values of SORT_THRESH

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "newswap.h"

#define heapify(p)							\
	for (l = p + (p - a) + es, max = p; l < e;) {			\
		root = max;						\
		r = l + es;						\
		is_less_than(max, l) && (max = l);			\
		(r < e) && is_less_than(max, r) && (max = r);		\
		if (max == root) break;					\
		l = max + (max - a) + es;				\
		swap(root, max, es);					\
	}

// High speed heap sort
static void
_hs(register char *a, size_t n, register size_t es, register const int (*is_less_than)(const void *, const void *))
{
	register char *e=a+n*es, *max, *l, *r, *root;

	// Build the heap
	for (register char *b=a+(n/2-1)*es; b>=a; b-=es)
		heapify(b);
 
	// The first element will always be the current maximum
	// Swap it to the end and bring the end in by one element
	// until we end up completely draining the heap
	for (e-=es; e>a; e-=es) {
		swap(a, e, es);
		heapify(a);
	}
} // _hs

// For partition sizes equal to or smaller than this, we
// invoke heap_sort to sort the rest of any partition
#define SORT_THRESH	3500

static void
_ms(register char *a, size_t n, size_t es, register const int (*is_less_than)(const void *, const void *), register char *c)
{
	if (n < 2)
		return;

	// Split array into 2
	size_t an = (n + 1) / 2;
	register char *b = a + (an * es), *be = a + (n * es), *ce = c + an * es;

	// Merge sort each sub-array
	if (an > SORT_THRESH)
		_ms(a, an, es, is_less_than, c);
	else
		_hs(a, an, es, is_less_than);

	if (n - an > SORT_THRESH)
		_ms(b, n - an, es, is_less_than, c);
	else
		_hs(b, n - an, es, is_less_than);

	// Now merge the 2 sorted sub-arrays back into the original array

	// Copy a to c
	memmove(c, a, ce-c);
//	for (register char *d = c, *s = a; d < ce; d+=es, s+=es)
//		copy(d, s, es);
	
	// Now merge b and c into a
	for (; b < be && c < ce; a+=es) {
		if (is_less_than(b, c)) {
			copy(a, b, es); b+=es;
		} else {
			copy(a, c, es); c+=es;
		}
	}

	// Copy any leftovers in c into a
	for (; c<ce; a+=es, c+=es)
		copy(a, c, es);
} // _ms


void
heap_merge(char *a, size_t n, size_t es, const int (*is_less_than)(const void *, const void *))
{
	if (n <= SORT_THRESH)
		return _hs(a, n, es, is_less_than);

	char *c = NULL;

	// Allocate helper array (needs to be at least half of n*es)
	if ((c = malloc(((n+1)/2)*es)) == NULL) {
		return;
	}

	_ms(a, n, es, is_less_than, c);

	free(c);
} // heap_merge
