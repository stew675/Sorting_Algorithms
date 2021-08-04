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

#define KEYBUFSIZE	16384
//#define KEYBUFSIZE	16384
//#define KEYBUFSIZE	4096
#define STEP 20

extern void print_array(char *a, size_t n);

static inline bool
is_sorted(char *a, register char *e, register size_t es, register size_t step, register const int (*is_lt)(const void *, const void *))
{
	for (register char *s=a+step; s<e; s+=step)
		if (is_lt(s, s-es))
			return false;
	return true;
} // is_sorted


static inline char *
binary_search(register char *a, register size_t n, register size_t es, register const int (*is_lt)(const void *, const void *), register char *key)
{
	register size_t lo = 0, hi = n, mid = hi / 2;
	register char *s=a+mid*es;

	while (lo < hi) {
		if (is_lt(key, s))
			hi = mid;
		else
			lo = mid + 1;
		mid = (lo + hi) / 2;
		s=a+mid*es;
	}
	return a+mid*es;
} // binary_search


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
        register char   *e = b+bn*es, *ap = a, *bp = b, *wp;
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
				for (wp = bp + es ; wp < e && is_lt(wp, wp-es); wp += es)
					swap(wp, wp-es);
				continue;
			}
		}

		if (ap == bp)
			break;

		// Start of keybuf merging with A section

		gap = kp - kb;	// Gap refers to the size of the keybuffer

		// Move A forwards until we reach where the first key will
		// start to merge with it.  This just reduces the amount of
		// data we need to work on for comparisons
		a = binary_search(a, ((ap - a) / es) - 1, es, is_lt, kb);

/*
		// Everything from ap onwards is unsorted but won't be touched
		// by the following merge operation, so just move it wholesale
		// forwards by the gap amount
		memmove(ap+gap, ap, b - ap);

		// We'll use the "worker pointer" to track the value within the
		// [a..ap-es] range that we're comparing with the keybuf keys
		for (wp = ap - es; kp > kb; ) {
			if (is_lt(kp-es, wp)) {
				copy(wp + gap, wp);
				if (wp > a) {
					wp -= es;
				} else {
					break;
				}
			} else {
				kp -= es;
				copy(wp + gap, kp);
				gap -=es;
			}
		}
*/

		// First move everything from A onwards up by the size
		// of the entries in the keybuffer
		// kp MUST be > kb, or we wouldn't even be here
		memmove(a + gap, a, b - a);

		// Now merge K and AP into A
		for (wp = a + gap; kb < kp && wp < bp; a += es)
			if (is_lt(kb, wp)) {
				copy(a, kb);
				kb+=es;
			} else {
				copy(a, wp);
				wp+=es;
			}

		// Copy any remainder left in k into A
		for (; kb < kp; a += es, kb += es)
			copy(a, kb);

		// Set-up for next loop
		a = binary_search(ap, ((bp - ap) / es) - 1, es, is_lt, bp);
		kb = keybuf;
		b = bp;
		ap = a;
		kp = kb;
//printf("Pointing A at %u, B at %u\n", *(uint32_t *)a, *(uint32_t *)b);
	}
} // roller_merge


void
roller_sort(register char *a, size_t n, register const size_t es, register const int (*is_lt)(const void *, const void *))
{
	register char	*se = a + (n * es);	// se means Source End
	register size_t	step;
	int swaptype;
	WORD t;

	// If es is too large, someone needs to adjust the keybufsize
	assert(KEYBUFSIZE >= es);

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
