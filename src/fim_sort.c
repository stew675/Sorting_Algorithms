#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "swap.h"

extern	void	print_array(void *a, size_t n);

static	const	int	(*is_lt)(const void *, const void *);
static	int	swaptype;
static	size_t	es;
static	size_t	rsd = 0, mrsd = 0;

#define	INSERT_SORT_MAX		  9
#define	SWAP_BLOCK_MIN		256
#define	SKEW			8

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


// Swaps two contiguous blocks of differing lengths in place efficiently
static void
_swab(char *a, char *b, char *e)
{
	size_t	gapa = b - a, gapb = e - b;
	WORD	t;

	while (gapa && gapb) {
		if (gapa < gapb) {
			for (char *src = a, *dst = a + gapb; dst != e; src += es, dst += es)
				swap(src, dst);
			e -= gapa;
			gapb = e - b;
		} else {
			for (char *src = b, *dst = a; src != e; src += es, dst += es)
				swap(src, dst);
			a += gapb;
			gapa = b - a;
		}
	}
} // _swab


static void
fim_insert_sort(char *pa, const size_t n)
{
	char	*pe = pa + n * es, *ta, *tb;
	WORD	t;

	if (n == 1)
		return;

	if (n == 2) {
		if (is_lt(pa + es, pa))
			swap(pa + es, pa);
		return;
	}

	// Do an insertion sort of tiny arrays
#if 1
	for (ta = pe - es, pe = ta; ta != pa; )
		for (tb = ta - es, ta = tb; (tb != pe) && is_lt(tb + es, tb); tb = tb + es)
			swap(tb + es, tb);
#else
	for (ta = pa + es; ta != pe; ta += es)
		for (tb = ta; tb != pa && is_lt(tb, tb - es); tb -= es)
			swap(tb, tb - es);
#endif
} // fim_insert_sort


// Merge two sorted sub-arrays together using insertion sort
static void
insertion_merge_in_place(char * restrict pa, char * restrict pb, char * restrict pe)
{
	char	*tb = pe;
	WORD	t;

	do {
		pe = tb - es;  tb = pb - es;  pb = tb;
		do {
			swap(tb + es, tb);
			tb = tb + es;
		} while ((tb != pe) && is_lt(tb + es, tb));
	} while ((pb != pa) && is_lt(pb, pb - es));
} // insert_merge_in_place


#if 0

#define	RIPPLE_STACK_SIZE	360

#define	RIPPLE_STACK_PUSH(s1, s2, s3) 	\
	{ *stack++ = s1; *stack++ = s2; *stack++ = s3; }

#define	RIPPLE_STACK_POP(s1, s2, s3) \
	{ s3 = *--stack; s2 = *--stack; s1 = *--stack; }

// Assumes NA and NB are greater than zero
static void
ripple_merge_in_place(char *pa, char *pb, char *pe)
{
	__attribute__((aligned(64))) char *_stack[RIPPLE_STACK_SIZE];
	char	**stack = _stack;
	char	*rp, *sp;	// Ripple-Pointer, and Split Pointer
	size_t	bs;		// Byte-wise block size of pa->pb
	WORD	t;		// Temporary variable for swapping

	// For whoever calls us, check if we need to do anything at all
	if (!is_lt(pb, pb - es))
		goto ripple_pop;

ripple_again:
	bs = pb - pa;

	// Just insert merge single items. We already know that *PB < *PA
	if (bs == es) {
		do {
			swap(pa, pb);
			pa = pb;
			pb += es;
		} while (pb != pe && is_lt(pb, pa));
		goto ripple_pop;
	}

	if ((pb + es) == pe) {
		do {
			swap(pb, pb - es);
			pb -= es;
		} while ((pb != pa) && is_lt(pb, pb - es));
		goto ripple_pop;
	}

	// Insertion MIP is slightly faster for very small sorted array pairs
	if ((pe - pa) < (es << 3)) {
		insertion_merge_in_place(pa, pb, pe);
		goto ripple_pop;
	}

	// Ripple all of A up as far as we can
	for (rp = pb + bs; rp <= pe && is_lt(rp - es, pa); rp += bs) {
		if (bs >= SWAP_BLOCK_MIN) {
			swap_blk(pa, pb, bs);
			pa = pb;
			pb = rp;
		} else {
			for ( ; pb != rp; pa += es, pb += es)
				swap (pa, pb);
		}
	}

	// We couldn't ripple the full PA->PB block up any further
	// Split the block up, and keep trying with the remainders

	// Handle scenario where our block cannot fit within what remains
	if (rp > pe) {
		if (pb == pe)
			goto ripple_pop;

		// Swap the remainder with our block if it's too small
		// This prevents a stack runaway condition
		if (bs > ((pe - pb) << 2)) {
			_swab(pa, pb, pe);
			pb = pa + (pe - pb);
			// Check if the swab sorted everything or not
			if (is_lt(pb, pb - es))
				goto ripple_again;
			goto ripple_pop;
		}

		// Adjust the pointers for the new limits
		rp = pe;
		bs = rp - pb;
	}
	// Find spot within PA->PB to split it at
	if (bs > (es << 3)) {	// Binary search on larger sets
		size_t	min = 0, max = bs / es;
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
	} else {	// Linear scan is faster for smaller sets
		sp = pb - bs;
		for ( ; (sp != pb) && !is_lt(rp - es, sp); sp += es, rp -= es);
	}

	if (!(bs = pb - sp))	  // Determine the byte-wise size of the split
		goto ripple_pop;  // If nothing to swap, we're done here

	// Do a single ripple at the split point
	if (bs >= SWAP_BLOCK_MIN) {
		swap_blk(sp, pb, bs);
	} else {
		for (char *ta = sp, *tb = pb; ta < pb; ta += es, tb += es)
			swap(ta, tb);
	}

	// PB->RP is the top part of A that was split, and  RP->PE is the rest
	// of the array we're merging into.
	// PA->SP is one sorted array, and SP->PB is another. PB is a hard upper
	// limit on the search space for this merge, so it's used as the new PE
	if (is_lt(sp, sp - es)) {
		if (rp != pe)
			RIPPLE_STACK_PUSH(pb, rp, pe);
		pe = pb;
		pb = sp;
		goto ripple_again;
	} else {
		bs = ((rp != pe) && is_lt(rp, rp - es));
	}
	if (bs) {
		pa = pb;
		pb = rp;
		goto ripple_again;
	}

ripple_pop:
	while (stack != _stack) {
		RIPPLE_STACK_POP(pa, pb, pe);
		if (is_lt(pb, pb - es))
			goto ripple_again;
	}
} // ripple_merge_in_place

#else

#define	RIPPLE_STACK_SIZE	256

// Assumes NA and NB are greater than zero
static void
ripple_merge_in_place(char *pa, char *pb, char *pe)
{
	__attribute__((aligned(64))) char *_stack[RIPPLE_STACK_SIZE];
	char	**stack = _stack;
	size_t	bs, can_coalesce = 0;
	WORD	t;		// Temporary variable for swapping

	// For whoever calls us, check if we need to do anything at all
	if (!is_lt(pb, pb - es))
		goto ripple_pop;

ripple_again:
	bs = pb - pa;	// Determine the byte-wise size of A

	// Just insert merge single items. We already know that *PB < *PA
	if (bs == es) {
		do {
			swap(pa, pb);  pa = pb;  pb += es;
		} while (pb != pe && is_lt(pb, pa));
		goto ripple_pop;
	}

	// Ripple the PA->PB block up as far as we can
	{	char	*rp = pb + bs;
		size_t	val = ((rp < pe) && is_lt(rp - es, pa));

		while (val) {
			if (bs < SWAP_BLOCK_MIN) {
				for ( ; pb < rp; pa += es, pb += es)
					swap(pa, pb);
			} else {
				swap_blk(pa, pb, bs);
				pa = pb;     pb = rp;
			}
			rp += bs;
			val = ((rp < pe) && is_lt(rp - es, pa));
		}
	}

	// Split the A block into two, and keep trying with remainder
	// The imbalanced split here improves algorithmic performance.
	// Division is slow.  Calculate the following ahead of time
#if 1
	bs = (bs / (es * 8)) + 1;
	can_coalesce = ((stack != _stack) && (*(stack - 1) == pa));
#else
	bs = (bs / (es * 5)) + 1;
#endif
	if (is_lt(pb, pb - es)) {
		char	*spa = pa + (bs * es);

		// Coalesce any fragmentation if possible
		if (can_coalesce) {
			*(stack - 1) = spa;
		} else {
			*stack++ = pa;  *stack++ = spa;
		}
		pa = spa;
		goto ripple_again;
	}

ripple_pop:
	while (stack != _stack) {
		pb = *--stack;  pa = *--stack;

		if (is_lt(pb, pb - es))
			goto ripple_again;
	}
} // ripple_merge_in_place
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

// Merges A + B into W
__attribute__((noinline))
static void
fim_merge_into_workspace(char *ws, char *pa, char *pb, char *pe)
{
	char	*ta = pa, *tb = pb;
	WORD	t;

	for ( ; (ta != pb) && (tb != pe); ws += es) {
		if (is_lt(tb, ta)) {
			swap(ws, tb);
			tb += es;
		} else {
			swap(ws, ta);
			ta += es;
		}
	}

	for ( ; ta != pb; ta += es, ws += es)
		swap(ws, ta);

	for ( ; tb != pe; tb += es, ws += es)
		swap(ws, tb);
} // fim_merge_into_workspace


// Merges a pre-loaded workspace and a sorted B, into A + B
// Requirements: A + B are contiguous
__attribute__((noinline))
static void
fim_merge_from_workspace(char *ws, char *we, char *pa, char *pb, char *pe)
{
	WORD	t;

	// Now merge rest of W into B
	for ( ; (pb != pe) && (ws != we); pa += es)
		if(is_lt(pb, ws)) {
			swap(pa, pb);
			pb += es;
		} else {
			swap(pa, ws);
			ws += es;
		}

	// Merge any remainder
	if ((we - ws) < SWAP_BLOCK_MIN) {
		for ( ; ws != we; ws += es, pa += es)
			swap(pa, ws);
	} else {
		swap_blk(ws, pa, we - ws);
	}
} // from_merge_from_workspace


// Merges A and B together using workspace W
// Assumes both NA and NB are > zero on entry
static void
fim_merge_using_workspace(char *w, char *a, const size_t na, char *b, const size_t nb)
{
	WORD	t;

	// Skip initial part of A as required
	for ( ; (a != b) && !is_lt(b, a); a += es);

	if (a == b)	// Nothing to merge
		return;

	char	*e = b + (nb * es);
	char	*pw = w;

	// Now copy everything remaining from A to W
	if ((b - a) < SWAP_BLOCK_MIN) {
		for (char *ta = a; ta != b; pw += es, ta += es)
			swap(pw, ta);
	} else {
		// Use bulk swaps for greater speed when we can
		swap_blk(a, w, (b - a));
		pw += (b - a);
	}

	// We already know that the first B is smaller
	swap(a, b);
	a += es;
	b += es;

	// Now merge rest of W into B
	for ( ; (b != e) && (w != pw); a += es)
		if(is_lt(b, w)) {
			swap(a, b);
			b += es;
		} else {
			swap(a, w);
			w += es;
		}

	// Merge any remainder
	for ( ; w != pw; w += es, a += es)
		swap(a, w);
} // fim_merge_using_workspace

#if 1

static void
fim_sort_with_workspace(char *w, char *pa, const size_t n)
{
	// Handle small array size inputs with insertion sort
	if (n <= INSERT_SORT_MAX)
		return fim_insert_sort(pa, n);

	// Split A into three mostly equal parts
	size_t	na = n / 4.5;
	size_t	nb = na;
	size_t	nc = n - (na + nb);		// It's important that pc gets any spill-over as
						// doing so avoids possible workspace overflow
	char	*pb = pa + na * es;
	char	*pc = pb + nb * es;

	// Keep recursing until we hit an insertion sort
	fim_sort_with_workspace(w, pa, na);
	fim_sort_with_workspace(w, pb, nb);
	fim_sort_with_workspace(w, pc, nc);

	// Merge A and B together using W as the workspace
	if (is_lt(pc, pc - es))
		fim_merge_using_workspace(w, pb, nb, pc, nc);

	if (is_lt(pb, pb - es))
		fim_merge_using_workspace(w, pa, na, pb, nb + nc);
} // fim_sort_with_workspace


static void
fim_sort_main(char *pa, const size_t n)
{
	// Handle small array size inputs with insertion sort
	if (n <= INSERT_SORT_MAX)
		return fim_insert_sort(pa, n);

	// Split array to be sorted into 3 parts.
	// A becomes our merging workspace.  It's important for A to be >= B in size

	size_t	nc = (n / 5) * 3;	// Roughly 60% of the total array
	size_t	nb = n / 5;		// Roughly 20% of the total array
	size_t	na = n - (nb + nc);	// A Must be >= B and >= C/3

	char	*ws = pa;		// First quarter becomes our work space WS
	char	*pb = pa + na * es;
	char	*pc = pb + nb * es;

	assert(na >= nb);
	assert(na >= ((nc + 2) / 3));
	assert((na + nb + nc) == n);

	// Sort B and C using A as a workspace
	fim_sort_with_workspace(ws, pb, nb);
	fim_sort_with_workspace(ws, pc, nc);

	// Merge B and C together using A as a workspace
	if (is_lt(pc, pc - es))
		fim_merge_using_workspace(ws, pb, nb, pc, nc);

	// Now finally sort A
	fim_sort_main(pa, na);

	// Now use the Ripple Merge In Place algorithm to merge A into BC
	// Ripple Merge doesn't need a workspace, but it's half the speed
	// of doing a workspace based merge, so we invoke it sparingly
	ripple_merge_in_place(pa, pb, pa + n * es);
} // fim_sort_main

#else

static void
fim_sort_using_workspace(char * const ws, char * const pa, const size_t n)
{
	if (n == 1)
		return;

	// Handle small array size inputs with insertion sort
	if ((n <= INSERT_SORT_MAX) || (n < (SKEW + 2)))
		return fim_insert_sort(pa, n);

	// Split A into three with ratios of (1:1:SKEW) in size
	size_t	nb = n / (SKEW + 2);
	size_t	nc = nb * SKEW;
	size_t	na = n - (nb + nc);

	char	*pb = pa + na * es;
	char	*pc = pb + nb * es;

	// Keep recursing until we hit an insertion sort
	fim_sort_using_workspace(ws, pc, nc);
	fim_sort_using_workspace(ws, pb, nb);
	fim_sort_using_workspace(ws, pa, na);

	if (is_lt(pb, pb - es)) {
		char	*we = ws + (na + nb) * es;
		char	*pe = pa + n * es;

		fim_merge_into_workspace(ws, pa, pb, pc);
		fim_merge_from_workspace(ws, we, pa, pc, pe);
	} else if (is_lt(pc, pc - es)) {
		fim_merge_using_workspace(ws, pa, na + nb, pc, nc);
	}
} // fim_sort_using_workspace

static void
fim_sort_main(char * const pa, const size_t n)
{
	// Handle small array size inputs with insertion sort
	if ((n <= INSERT_SORT_MAX) || (n < 8))
		return fim_insert_sort(pa, n);

	// Split A into four parts with ratios of (2:1:1:4) in size.  These
	// values appear to be close to optimal for the RippleSort Algorithm
	// which works best when rippling in arrays that are 1/4 the size of
	// the target array.
	size_t	nb = n >> 3;
	size_t	nc = nb;
	size_t	nd = nb << 2;
	size_t	na = n - (nb + nc + nd);

	char	*pb = pa + na * es;
	char	*pc = pb + nb * es;
	char	*pd = pc + nc * es;

	// Now Sort B, C, D, using A as workspace

	fim_sort_using_workspace(pa, pd, nd);
	fim_sort_using_workspace(pa, pc, nc);
	fim_sort_using_workspace(pa, pb, nb);

	if (is_lt(pc, pc - es)) {
		char	*we = pa + (nb + nc) * es;
		char	*pe = pa + n * es;

		fim_merge_into_workspace(pa, pb, pc, pd);
		fim_merge_from_workspace(pa, we, pb, pd, pe);
	} else if (is_lt(pd, pd - es)) {
		fim_merge_using_workspace(pa, pb, nb+nc, pd, nd);
	}

	// Now finally recurse to sort A
	fim_sort_main(pa, na);

//	printf("After Merge: Num Compares = %ld, Num Swaps = %ld\n\n", numcmps, numswaps);

	// Now use the Ripple Merge In Place algorithm to merge A into B
	// Ripple Merge doesn't need a workspace, but it's half the speed
	// of doing a workspace based merge, so we invoke it sparingly
	ripple_merge_in_place(pa, pb, pa + n * es);
} // fim_sort_main

#endif

static void
simplest(char *pa, const size_t n)
{
	// Handle small array size inputs with insertion sort
	if (n <= 7)
		return fim_insert_sort(pa, n);

//	size_t	na = (n >> 2) + 1
	size_t	na = (n / 4.6) + 1;
	size_t	nb = n - na;
	char	*pb = pa + na * es;

	if (na > 1)
		simplest(pa, na);

	if (nb > 1)
		simplest(pb, nb);

	ripple_merge_in_place(pa, pb, pa + (n * es));
} // simplest

#pragma GCC diagnostic pop


void
fim_sort(char *a, const size_t n, const size_t _es, const int (*_is_lt)(const void *, const void *))
{
	es = _es;
	is_lt = _is_lt;

	SWAPINIT(a, es);

	fim_sort_main(a, n);
//	simplest(a, n);
//	print_array(a, n);
} // fim_sort
