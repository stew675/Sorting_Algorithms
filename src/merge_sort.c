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
#include <stdio.h>
#include "swap.h"

static void
_ms(register char *a, size_t n, size_t es, register const int (*is_less_than)(const void *, const void *), register int swaptype, register char *c)
{
	if (n < 2)
		return;

	register WORD t;

	// Split array into 2
	size_t an = (n + 1) / 2;
	register char *b = a + (an * es), *be = a + (n * es), *ce = c + an * es;

	_ms(a, an, es, is_less_than, swaptype, c);
	_ms(b, n - an, es, is_less_than, swaptype, c);

	// Now merge the 2 sorted sub-arrays back into the original array

	// Copy a to c
	memmove(c, a, ce-c);
	
	// Now merge b and c into a
	for (; b < be && c < ce; a+=es) {
		if (is_less_than(b, c)) {
			copy(a, b); b+=es;
		} else {
			copy(a, c); c+=es;
		}
	}

	// Copy any leftovers from c into a
	if (ce-c > 127) {
		memmove(a, c, ce-c);
	} else {
		for (; c<ce; a+=es, c+=es)
			copy(a, c);
	}
} // _ms


void
merge_sort(char *a, size_t n, size_t es, const int (*is_less_than)(const void *, const void *))
{
	char *c = NULL;

	// Allocate helper array (needs to be at least half of n*es)
	if ((c = malloc(((n+1)/2)*es)) == NULL) {
		return;
	}

	register int swaptype;

	SWAPINIT(a, es);

	_ms(a, n, es, is_less_than, swaptype, c);

	free(c);
} // merge_sort
