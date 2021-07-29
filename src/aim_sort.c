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
#include <alloca.h>
#include <string.h>
#include "newswap.h"

#define STEP 24

static bool
is_sorted(char *a, register char *e, register size_t es, register size_t step, register const int (*is_less_than)(const void *, const void *))
{
	for (register char *s=a+step; s<e; s+=step)
		if (is_less_than(s, s-es))
			return false;
	return true;
} // is_sorted

void
aim_sort(register char *a, size_t n, register const size_t es, register const int (*is_less_than)(const void *, const void *))
{
	register char	*se = a + (n * es);	// se means Source End
	register size_t	step;

	// Choose a step size that divides the problem set as evenly as possible
	for (step = n; step > STEP; step = (step + 1) / 2);
	step *= es;

	// First pass over a, doing insertion sorts every STEP intervals
	do {
		char temp[es];

		for (register char *b = a, *be = a + step; b < se; b = be, be+=step) {
			if (be > se) {
				be = se;
			}
			for (register char *s, *p=b+es; p < be; p+=es) {
				if (!is_less_than(p, p-es))
					continue;

				s = p - es;
				copy(temp, p, es);
				copy(p, s, es);
				while ((s > b) && is_less_than(temp, s-es)) {
					copy(s, s-es, es);
					s -= es;
				}
				copy(s, temp, es);
//				for(s=p-es; (s>b) && is_less_than(s, s-es); s-=es)
//					swap(s, s-es, es);
			}
		}
	} while (0);

	// Check if we're done
	if (is_sorted(a, se, es, step, is_less_than)) {
		return;
	}

	register char *wrk;

	if ((wrk = (char *)malloc(n * es)) == NULL)
		return;		// Out of memory

	register char *src = a, *dst = wrk;
	register char *de = dst + n * es;	// Destination End

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
			for (;; dst+=es) {
				if (is_less_than(b2p, b1p)) {
					copy(dst, b2p, es);
					if ((b2p += es) == b2e)
						goto copy_b1_remainder;
				} else {
					copy(dst, b1p, es);
					if ((b1p += es) == b1e)
						goto copy_b2_remainder;
				}
			}

copy_b1_remainder:
			// Copy any remainder in b1 over to the destination
			// Increment dst 'cos we didn't do it before goto's above
			dst += es;
			do {
				copy(dst, b1p, es);
				b1p += es;
				dst += es;
			} while (b1p < b1e);
			continue;

copy_b2_remainder:
			// Copy any remainder in b2 over to the destination
			// Increment dst 'cos we didn't do it before goto's above
			dst += es;
			do {
				copy(dst, b2p, es);
				b2p += es;
				dst += es;
			} while (b2p < b2e);
			continue;
		}

		// Copy any remainder in b1 over to the destination
		// b2 can't have any remainder by this stage
		if (b1e > se)
			b1e = se;

		while (b1p < b1e) {
			copy(dst, b1p, es);
			dst += es;
			b1p += es;
		}

		// Double our bucket step size before checking if we're done
		step += step;

		// Check if we're done
		if (is_sorted((src == a ? wrk : a), de, es, step, is_less_than))
			break;

		// Swap src with dst and restart the loop
		if (src == a) {
			src = wrk;
			dst = a;
		} else {
			src = a;
			dst = wrk;
		}
		se = src + n * es;
		de = dst + n * es;
	}

	if (src == a) {
		// Copy wrk back to a
		memmove(a, wrk, se-src);
	}

	// Release our work space
	free(wrk);
} // aim_sort
