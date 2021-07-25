//		Adaptive Iterative Merge Sort
//
// Author: Stew Forster (stew675@gmail.com)	Date: 25 July 2021
//
// Implements a bucketed adaptive stable interative merge sort
// Is fast.  Really, really fast. O(n) space overhead though

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "newswap.h"

#define STEP 20

static bool
is_sorted(char *a, register char *e, register size_t es, register size_t step, register const int (*cmp)(const void *, const void *))
{
	for (register char *s=a+step; s < e; s+=step)
		if (cmp(s, s-es) < 0)
			return false;
	return true;
} // is_sorted

void
aim_sort(register char *a, size_t n, register const size_t es, register const int (*cmp)(const void *, const void *))
{
	register char	*se = a + (n * es);	// se means Source End
	register size_t	step = STEP*es;

	// First pass over a, doing insertion sorts every STEP intervals
	do {
		for (register char *b = a, *be = a + step; b < se; b = be, be+=step) {
			if (be > se) {
				be = se;
			}
			for (register char *s, *p = b+es; p < be; p+=es)
				for(s=p; (s>b) && (cmp(s, s-es)<0); s-=es)
					swap(s, s-es, es);
		}
	} while (0);

	// Check if we're done
	if (is_sorted(a, se, es, step, cmp)) {
		return;
	}

	register char *wrk;

	if ((wrk = (char *)aligned_alloc(4096, n * es)) == NULL) {
		return;		// Out of memory
	}

	register char *src = a, *dp = wrk;
	register char *de = dp + n * es;	// Destination End

	for (;;) {
		register char *b1p, *b1e;	// bucket 1 position, bucket 1 end

		// Loop over bucket pairs in src, merging them together into dst
		for (b1p = src, b1e = b1p + step; b1e < se; b1p+=step, b1e=b1p+step) {
			register char *b2p = b1e, *b2e = b1e + step;	// Bucket 2 position and end

			if (b2e > se)
				b2e = se;

			// Merge both buckets into the destination.  I think that this
			// code looks ugly, but it appears to be the fastest way to run
			// these loops, so it is what it is for that reason
			for (;; dp+=es) {
				if (cmp(b1p, b2p) > 0) {
					copy(dp, b2p, es);
					if ((b2p += es) == b2e)
						goto copy_b1_remainder;
				} else {
					copy(dp, b1p, es);
					if ((b1p += es) == b1e)
						goto copy_b2_remainder;
				}
			}

copy_b1_remainder:
			// Copy any remainder in b1 over to the destination
			// Increment dp 'cos we didn't do it before goto's above
			dp += es;
			do {
				copy(dp, b1p, es);
				b1p += es;
				dp += es;
			} while (b1p < b1e);
			continue;

copy_b2_remainder:
			// Copy any remainder in b2 over to the destination
			// Increment dp 'cos we didn't do it before goto's above
			dp += es;
			do {
				copy(dp, b2p, es);
				b2p += es;
				dp += es;
			} while (b2p < b2e);
			continue;
		}

		// Copy any remainder in b1 over to the destination
		// b2 can't have any remainder by this stage
		if (b1e > se)
			b1e = se;

		while (b1p < b1e) {
			copy(dp, b1p, es);
			dp += es;
			b1p += es;
		}

		// Double our bucket step size before checking if we're done
		step += step;

		// Check if we're done
		if (is_sorted((src == a ? wrk : a), de, es, step, cmp))
			break;

		// Swap src with dst and restart the loop
		if (src == a) {
			src = wrk;
			dp = a;
		} else {
			src = a;
			dp = wrk;
		}
		se = src + n * es;
		de = dp + n * es;
	}

	if (src == a) {
		de = a + n * es;
		// Copy wrk back to a
		for (src = wrk, dp = a; dp < de; src+=es, dp+=es) {
			copy(dp, src, es);
		}
	}

	// Release our work space
	free(wrk);
} // aim_sort
