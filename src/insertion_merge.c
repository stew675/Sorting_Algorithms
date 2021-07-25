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
#include "newswap.h"

__attribute__((noinline))
static void
_is(register char *a, size_t n, register const size_t es, register const int (*cmp)(const void *, const void *))
{
	register char	*p, *s, *e = a + n * es;

	for (p = a+es; p < e; p+=es)
		for(s=p; (s>a) && (cmp(s, s-es)<0); s-=es)
			swap(s, s-es, es);
} // insertion_sort

// For partition sizes equal to or smaller than this, we
// invoke insertion_sort to sort the rest of any partition
#define SORT_THRESH	20

static void
_ms(register char *a, size_t n, size_t es, register const int (*cmp)(const void *, const void *), register char *c)
{
	if (n < 2)
		return;

	// Split array into 2
	size_t an = (n + 1) / 2;
	register char *b = a + (an * es), *be = a + (n * es), *ce = c + an * es;

	// Merge sort each sub-array
	if (an > SORT_THRESH)
		_ms(a, an, es, cmp, c);
	else
		_is(a, an, es, cmp);

	if (n - an > SORT_THRESH)
		_ms(b, n - an, es, cmp, c);
	else
		_is(b, n - an, es, cmp);

	// Now merge the 2 sorted sub-arrays back into the original array

	// Copy a to c
	for (register char *d = c, *s = a; d < ce; d+=es, s+=es)
		copy(d, s, es);
	
	// Now merge b and c into a
	for (; b < be && c < ce; a+=es) {
		if (cmp(b, c) < 0) {
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
insertion_merge(char *a, size_t n, size_t es, const int (*cmp)(const void *, const void *))
{
	if (n <= SORT_THRESH) {
		return _is(a, n, es, cmp);
	}

	char *c = NULL;

	// Allocate helper array (needs to be at least half of n*es)
	if ((c = malloc(((n+1)/2)*es)) == NULL) {
		return;
	}

	_ms(a, n, es, cmp, c);

	free(c);
} // heap_merge
