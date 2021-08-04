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
#include <assert.h>
#include "swap.h"

#define KEYBUFSIZE	4096
//#define KEYBUFSIZE	16384
//#define KEYBUFSIZE	4096
#define STEP 20

extern void print_array(char *a, size_t n);

static bool
is_sorted(char *a, register char *e, register size_t es, register size_t step, register const int (*is_lt)(const void *, const void *))
{
	for (register char *s=a+step; s<e; s+=step)
		if (is_lt(s, s-es))
			return false;
	return true;
} // is_sorted

// Both sorted sub-arrays must be adjacent in 'a'
// Assumes that both 'an' and 'bn' are always non-zero upon entry
// 'an' is the length of the first sorted section in 'a', referred to as A
// 'bn' is the length of the second sorted section in 'a', referred to as B
static void
roller_merge(register char *a, register size_t an, size_t bn, register size_t es, register const int (*is_lt)(const void *, const void *), register int swaptype)
{
        register char   *b = a+an*es;

        // If the first element of B is not less then the last element
        // of A, then since A and B are in order, it naturally follows
        // that [A, B] must also all be in order and we're done here
        if (!is_lt(b, b-es))
                return;

	char		keybuf[KEYBUFSIZE];
        register char   *e = b+bn*es, *ap = a, *bp = b, *bw;
	register char	*kb = keybuf, *kp = kb, *ke = kb + KEYBUFSIZE;	// kp = key position
	register WORD	t;
	register size_t gap;

	for (ap = a; ; a = (kp==kb ? ap : a)) {
		if (ap < b) {
			if (is_lt(bp, ap)) {
				swap(bp, ap);
				ap += es;
				if (bp+es == e)
					continue;
				if (!is_lt(bp+es, bp))
					continue;
			} else {
				ap += es;
				continue;
			}

			// Keep going if keybuf not yet full
			if (kp + es <= ke) {
				copy(kp, bp);
				bp+=es;
				kp+=es;
				if (kp < ke)
					continue;
			}

			// keybuf can't hold even one element.
			// Fallback to slow default of just bubbling the values up
			if (kp == kb) {
				for (bw = bp + es ; bw < e && is_lt(bw, bw-es); bw += es)
					swap(bw, bw-es);
				continue;
			}
		}

		if (ap == bp)
			break;

		// First move everything from A onwards up by the size
		// of the entries in the keybuffer
		// kp MUST be > kb, or we wouldn't even be here
		gap = kp - kb;
		if (gap < 20) {
			for (bw = b; bw > a; ) {
				bw-=es;
				copy(bw+gap, bw);
			}
		} else
			memmove(a + gap, a, b - a);

		// Now merge K and AP into A
		for (bw = a + gap; kb < kp && bw < bp; a += es)
			if (is_lt(kb, bw)) {
				copy(a, kb);
				kb+=es;
			} else {
				copy(a, bw);
				bw+=es;
			}

		// Copy any remainder left in k into A
		for (; kb < kp; a += es, kb += es)
			copy(a, kb);

		// Set-up for next loop
		kb = keybuf;
		b += gap;
		a = ap;
		kp = kb;
	}
} // roller_merge


void
roller_sort(register char *a, size_t n, register const size_t es, register const int (*is_lt)(const void *, const void *))
{
	register char	*se = a + (n * es);	// se means Source End
	register size_t	step;
	int swaptype;
	WORD t;

	SWAPINIT(a, es);

	// Choose a step size that divides the problem set as evenly as possible
	for (step = n; step > STEP; step = (step + 1) / 2);
	step = step * es;

	// First pass over a, doing insertion sorts every STEP intervals
	do {
		char temp[es];

		for (register char *b = a, *be = a + step; b < se; b = be, be+=step) {
			if (be > se) {
				be = se;
			}
			for (register char *s, *p=b+es; p < be; p+=es) {
				if (!is_lt(p, p-es))
					continue;

				s = p - es;
				copy(temp, p);
				copy(p, s);
				while ((s > b) && is_lt(temp, s-es)) {
					copy(s, s-es);
					s -= es;
				}
				copy(s, temp);
			}
		}
	} while (0);

	// Check if we're done
	if (is_sorted(a, se, es, step, is_lt)) {
		return;
	}

	for (;;) {
		register char *b1p, *b1e;	// bucket 1 position, bucket 1 end

		// Loop over bucket pairs in src, merging them together into dst
		for (b1p = a, b1e = b1p + step; b1e < se; b1p+=step*2, b1e=b1p+step) {
			register char *b2p = b1e, *b2e = b1e + step;	// Bucket 2 position and end

			if (b2e > se)
				b2e = se;

			roller_merge(b1p, step / es, (b2e - b2p) / es, es, is_lt, swaptype);
		}

		// Double our bucket step size before checking if we're done
		step += step;

		// Check if we're done
		if (is_sorted(a, se, es, step, is_lt))
			break;
	}
} // roller_sort
