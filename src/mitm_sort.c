#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "swap.h"

extern	uint64_t numcmps, numswaps;
extern	void	print_array(void *a, size_t n);

static	const	int	(*is_lt)(const void *, const void *);
static	int	swaptype;
static	size_t	es;
static	size_t	n1 = 0, n2 = 0, nn = 0;

#define	INSERT_SORT_MAX		10
#define	SWAP_BLK_MIN		256

// Takes advantage of any vectorization in the optimized memcpy library functions
static void
swap_blk(char *a, char *b, size_t n)
{
	__attribute__((aligned(64))) char t[1024];

	do {
		size_t	tc = n > 1024 ? 1024 : n;
		memcpy(t, a, tc);
		memcpy(a, b, tc);
		memcpy(b, t, tc);
		a+=tc;
		b+=tc;
		n-=tc;
	} while (n > 0);
} // swap_blk


static void
fim_insert_sort(char *pa, const size_t n)
{
	char	*pe = pa + (n * es);
	WORD	t;

	if (n < 2) return;

	if (is_lt(pa + es, pa)) swap(pa + es, pa);

	if (n == 2) return;

	for (char *ta = pa + es, *tb; ta < pe; ta += es)
		for (tb = ta; (tb > pa) && is_lt(tb, tb - es); tb -= es)
			swap(tb, tb - es);
} // fim_insert_sort


// Swaps two blocks in place efficiently
// This is my variant of a Block Rotate
// Assume N = (na + nb)
// At best N/2 swaps when na == nb
// At worst N-1 swaps when either na or nb = 1
static inline void
_swab(char *pa, size_t na, char * const pb, size_t nb)
{
	char	*pe = pb + nb * es;
	WORD	t;				// Temporary variable for swapping

	while (na && nb) {
		if (na < nb) {
			size_t	gap = pb - pa;
			char	*from = pa, *to = pe - gap;

			for ( ; to != pe; from += es, to += es)
				swap(from, to);

			nb -= na;
			pe -= gap;
		} else {
			char	*from = pb, *to = pa;

			for ( ; from != pe; from += es, to += es)
				swap(from, to);

			na -= nb;
			pa += (pe - pb);
		}
	}
} // _swab


// Merge two sorted sub-arrays together using insertion sort
static void
insertion_merge_in_place(char *pa, char *pb, char *pe)
{
	WORD	t;

	for (char *tb = pb; (pb != pe) && (pa != pb); pa = tb + es, pb += es)
		for (tb = pb; (tb > pa) && is_lt(tb, tb - es); tb -= es)
			swap(tb, tb - es);
} // insert_merge_in_place


// Assumes NA and NB are greater than zero
static void
ripple_merge_in_place(char *pa, size_t na, char *pb, char *pe)
{
	char	*rp, *sp;	// Ripple-Pointer, and Split Pointer
	size_t	bs;		// Byte-wise block size of pa->pb
	WORD	t;		// Temporary variable for swapping

	while (is_lt(pb, pb - es)) {
		if (na == 1) {
			// Just insert merge single items. We already know
			// that *PB < *PA, so we start with a swap.
			// Long term average is 3 compares & swaps
			do {
				swap(pa, pb);
				pa = pb;
				pb += es;
			} while (pb != pe && is_lt(pb, pa));
			return;
		}

		// Ripple all of A up as far as we can
		bs = pb - pa;		// Determine the byte-wise size of A
		for (rp = pb + bs; rp <= pe && is_lt(rp - es, pa); rp += bs)
			for ( ; pb != rp; pa += es, pb += es)
				swap (pa, pb);
			
		if (pb == pe)	// We went all the way!  We're done here
			return;

		// Handle the special case of when we reach the end
		// Swap the remainder (PB -> PE) with our block (A -> PB)
		// and make the remainder be the new block we're rippling
		if (rp > pe) {
			size_t nb = (pe - pb) / es;

			_swab(pa, na, pb, nb);
			ripple_merge_in_place(pa, nb, pa + (nb * es), pe);
			return;
		}

		// Find spot within A to split it at
		if (na > 11) {
			// Binary search on adequately large A sets
			size_t	min = 0, max = na - 1;
			size_t	sn = max >> 1;

			sp = pb - (sn * es);
			rp = pb + (sn * es);

			while (min < max) {
				if (is_lt(rp, sp - es))
					min = sn + 1;
				else
					max = sn;

				sn = (min + max) >> 1;
				sp = pb - (sn * es);
				rp = pb + (sn * es);
			}
		} else {
			// Linear scan on small sets is faster
			rp -= es;
			sp = pb - (rp - pb);
			for ( ; (sp != pb) && !is_lt(rp - es, sp); sp += es)
				rp -= es;
		}

		if (sp == pb)		// Nothing to swap.  We're done here
			return;

		// Do a single ripple at the split point
		bs = pb - sp;		// Byte size of SP->PB (also PB->RP)
		if (bs >= SWAP_BLK_MIN) {
			swap_blk(sp, pb, bs);
		} else {
			for (char *ta = sp, *tb = pb; ta < pb; ta += es, tb += es)
				swap(ta, tb);
		}
		bs /= es;		// Convert BS to an element count now

		// PA->SP is one sorted array, and SP->PB is another.
		// PB is a hard upper limit on the search space for
		// this sub-merge, so it's used as the new PE
		if (na < 10)
			insertion_merge_in_place(pa, sp, pb);
		else
			ripple_merge_in_place(pa, na - bs, sp, pb);

		// PB->RP is the top part of A that was split
		// RP->PE is the rest of the array we're merging into
		na = bs;
		pa = pb;
		pb = rp;
	}
} // ripple_merge_in_place


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

// Merges A and B together using workspace W
// Start of A through to end of B must be contiguous
// Assumes both NA and NB are > zero on entry
static void
fim_merge_using_workspace(char *w, char *a, const size_t na, char *b, const size_t nb)
{
	WORD	t;

	// First check if we need to do anything at all
	if (!is_lt(b, b - es))
		return;

	// Skip initial part of A
	while ((a != b) && !is_lt(b, a))  a += es;

	if (a == b)	// Nothing to merge
		return;

	char * const pe = b + (nb * es);
	char	*wp = w, *we = w;	// Workspace pointer/end

	// Now copy everything remaining from A to W
	if ((b - a) < SWAP_BLK_MIN) {
		for (char *ta = a; ta != b; we += es, ta += es)
			swap(we, ta);
	} else {
		// Use bulk swaps for greater speed when we can
		swap_blk(a, w, (b - a));
		we += (b - a);
	}

	// We already know that the first B is smaller
	swap(a, b);
	a += es;
	b += es;

	// Now merge rest of W into B
	for ( ; (b != pe) && (wp != we); a += es)
		if(is_lt(b, wp)) {
			swap(a, b);
			b += es;
		} else {
			swap(a, wp);
			wp += es;
		}

	// Merge any remainder
	if ((we - wp) >= SWAP_BLK_MIN)
		return swap_blk(wp, a, (we - wp));

	for ( ; wp != we; wp += es, a += es)
		swap(a, wp);
} // fim_merge_using_workspace

#define	SKEW	4.7

static void
skew_sort_sub(char * const w, char * const a, const size_t n)
{
	if (n < 15)
		return fim_insert_sort(a, n);

	size_t	na = n / (SKEW);
	size_t	nb = n - na;
	char	*pb = a + na * es;

	skew_sort_sub(w, a, na);

	skew_sort_sub(w, pb, nb);

	fim_merge_using_workspace(w, a, na, pb, nb);
}

static void
skew_sort_main(char * const a, const size_t n)
{
	if (n < 15)
		return fim_insert_sort(a, n);

	size_t	na = (n * 2) / 7;
	size_t	nb = n - na;
	char	*pb = a + na * es;
	char	*pe = a + n * es;

	skew_sort_sub(a, pb, nb);

	skew_sort_main(a, na);

	ripple_merge_in_place(a, na, pb, pe);
}

#pragma GCC diagnostic pop

void
mitm_sort(char *a, const size_t n, const size_t _es, const int (*_is_lt)(const void *, const void *))
{
	if (n <= 1)
		return;

	es = _es;
	is_lt = _is_lt;
	SWAPINIT(a, es);

	skew_sort_main(a, n);
//	printf("nn = %ld\n", nn);
//	printf("n1 = %ld, n2 = %ld, nn = %ld\n", n1, n2, nn);
//	for (int i = 0; i < n; i++) {
//		assert(i == *(((int *)a) + i));
//	}
//	print_array(a, n);
} // mitm_sort
