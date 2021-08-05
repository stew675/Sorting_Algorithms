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

// Merging at the top appears to be slower, despite less data being copied about
// I suspect that this is due to the merge-at-bottom's memmove operation leaving
// the CPU data caches warm with the data that we're about to operate on, and so
// merging at the bottom runs slightly faster overall.  Still, uncomment the
// following line if you want to play around with that code path
//#define MERGE_AT_TOP

#define KEYBUFSIZE	16384
//#define KEYBUFSIZE	16384
//#define KEYBUFSIZE	4096
#define STEP 50

extern void print_array(char *a, size_t n);

// The get index of the most significant bit of a 64 bit value
static uint64_t
msb64(uint64_t v)
{
        static const uint64_t dbm64 = (uint64_t)0x03f79d71b4cb0a89;
        static const uint8_t dbi64[64] = {
                 0, 47,  1, 56, 48, 27,  2, 60, 57, 49, 41, 37, 28, 16,  3, 61,
                54, 58, 35, 52, 50, 42, 21, 44, 38, 32, 29, 23, 17, 11,  4, 62,
                46, 55, 26, 59, 40, 36, 15, 53, 34, 51, 20, 43, 31, 22, 10, 45,
                25, 39, 14, 33, 19, 30,  9, 24, 13, 18,  8, 12,  7,  6,  5, 63
        };

        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v |= v >> 32;
        return dbi64[(v * dbm64) >> 58];
} // msb64

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
	return s;
} // binary_search


static inline char *
skip_search(register char *a, register char *e, register size_t es, register const int (*is_lt)(const void *, const void *), register char *key)
{
	register size_t skip = 0;

	if (is_lt(key, a))
		return a;
	a += es;

	for (;;) {
		if (a == e && skip < 2) {
			return a;
		} else if (a >= e) {
			skip = (skip + 1) / 2;
			a -= skip*es;
		} else if(is_lt(key, a)) {
			if (skip > 1)
				for (a-=(skip-1)*es; !is_lt(key, a); a+=es);
			return a;
		} else {
			skip++;
			a += skip*es;
		}
	}
} // skip_search


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
        register char   *e = b+bn*es, *ap = a, *bp = b, *wp = NULL;
	register char	*kb = keybuf, *kp = kb, *ke = kb + KEYBUFSIZE;	// kp = key position
	register WORD	t;
	register size_t gap;

	for (;;) {
		if (ap < b) {
/*
			wp = skip_search(ap, b, es, is_lt, bp);
			if ((ap = wp) == b)
				continue;
*/
			if (!is_lt(bp, ap)) {
				ap += es;
				continue;
			}

			if (bp+es == e || !is_lt(bp+es, ap)) {
				swap(bp, ap);
				ap += es;
				continue;
			}

			// ap > bp+es, so we must copy ap to the keybuf
			// and then copy bp to where ap was
			// copy3 copies from ap to kp, and then from bp to ap
			copy3(kp, ap, bp);
			ap+=es;
			bp+=es;
			kp+=es;

			// Restart the loop if the keybuf can hold more
			if (kp + es <= ke)
				continue;
		}

		if (ap == bp)
			break;

		// If we get here, we either ran out of things to sort in A
		// or the keybuf is full

		// Move A forwards until we reach where the first key will
		// start to merge with it.  This just reduces the amount of
		// data we need to work on for comparisons and to copy about
		a = binary_search(a, ((ap - a) / es) - 1, es, is_lt, kb);

		// Start of keybuf merging with A section
		gap = kp - kb;	// Gap refers to the size of the keybuffer

#ifdef MERGE_AT_TOP
		// Everything from ap onwards is unsorted but won't be touched
		// by the following merge operation, so just move it wholesale
		// forwards by the gap amount
		memmove(ap+gap, ap, b - ap);

		// We'll use the "worker pointer" to track the value within the
		// [a..ap-es] range that we're comparing with the keybuf keys
		for (wp = ap - es; kp > kb; )
			if (is_lt(kp-es, wp)) {
				copy(wp + gap, wp);
				if (wp == a)
					break;
				wp -= es;
			} else {
				kp-=es;
				copy(wp + gap, kp);
				gap -=es;
			}
#else
		// First move everything from A onwards up by the size
		// of the entries in the keybuffer
		// kp MUST be > kb, or we wouldn't even be here
		memmove(a + gap, a, b - a);
		wp = a+gap;

		// Now merge K and A' into A
		// Only use the binary search path if the average number of compares
		// is less when using a binary search, as opposed to linear search
		do {
			register char *ep = ap + gap;
//			if (msb64((ep-wp)/es)+3 < (ep-wp)/gap) {
			if ((ep-wp)/gap > 2) {
				register char *ki;

//				ki = binary_search(wp, (ep-wp)/es, es, is_lt, kb);
				ki = skip_search(wp, ep, es, is_lt, kb);
				for (; kb < kp && wp < ep; a += es)
					if (wp == ki) {
						copy(a, kb);
						kb+=es;
						//ki = binary_search(wp, (ep-wp)/es, es, is_lt, kb);
						ki = skip_search(wp, ep, es, is_lt, kb);
					} else {
						copy(a, wp);
						wp+=es;
					}
			} else {
				for (; kb < kp && wp < ep; a += es)
					if (is_lt(kb, wp)) {
						copy(a, kb);
						kb+=es;
					} else {
						copy(a, wp);
						wp+=es;
					}
			}
		} while(0);
#endif

		// Copy any remainder left in k into A
		for (; kb < kp; a += es, kb += es)
			copy(a, kb);

		// Set-up for next loop
		a = binary_search(ap, ((bp - ap) / es) - 1, es, is_lt, bp);
		kb = keybuf;
		b = bp;
		ap = a;
		kp = kb;
		wp = NULL;
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
			if (be > se)
				be = se;
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
	if (is_sorted(a, se, es, step, is_lt))
		return;

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
