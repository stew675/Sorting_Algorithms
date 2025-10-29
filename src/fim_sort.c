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
#define	BULK_SWAP_MIN		256

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


// Swaps two contiguous blocks in place efficiently
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
	char	*pe = pa + n * es;
	WORD	t;

	if (n == 1)
		return;

	if (n == 2) {
		if (is_lt(pa + es, pa))
			swap(pa + es, pa);
		return;
	}

	// Do an insertion sort of tiny arrays
	for (char *ta = pa + es, *tb; ta != pe; ta += es)
		for (tb = ta; tb != pa && is_lt(tb, tb - es); tb -= es)
			swap(tb, tb - es);
} // fim_insert_sort


#define	RIPPLE_STACK_SIZE	360

#define	RIPPLE_STACK_PUSH(s1, s2, s3) 	\
		{ *stack++ = s1; *stack++ = s2; *stack++ = s3; }

#define	RIPPLE_STACK_POP		\
	{				\
		if (stack == _stack)	\
			return;		\
		pe = *--stack;		\
		pb = *--stack;		\
		pa = *--stack;		\
		goto ripple_again;	\
	}

#if 1
// Assumes NA and NB are greater than zero
static void
ripple_merge_in_place(char *pa, char *pb, char *pe)
{
	char	*_stack[RIPPLE_STACK_SIZE], **stack = _stack;
	char	*rp, *sp;	// Ripple-Pointer, and Split Pointer
	size_t	bs;		// Byte-wise block size of pa->pb
	WORD	t;		// Temporary variable for swapping

	// For whoever calls us, check if we need to do anything at all
	if (!is_lt(pb, pb - es))
		goto ripple_pop;

ripple_again:
	bs = pb - pa;
	if (bs == es) {
		// Just insert merge single items.
		// We already know that *PB < *PA
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

	// Ripple all of A up as far as we can
	for (rp = pb + bs; rp <= pe && is_lt(rp - es, pa); rp += bs) {
		if (bs >= BULK_SWAP_MIN) {
			swap_blk(pa, pb, bs);
			pa = pb;
			pb = rp;
		} else {
			for ( ; pb != rp; pa += es, pb += es)
				swap (pa, pb);
		}
	}

	// Okay, we couldn't ripple the full PA->PB up block any further
	// Split the A block up, and keep trying with the remainders

	// Handle scenario where our block cannot fit within what remains
	if (rp > pe) {
		if (pb == pe)
			goto ripple_pop;

		// Swap the remainder with our block if it's too small
		// This prevents a stack runaway condition
		if (((pe - pb) << 2) < bs) {
			_swab(pa, pb, pe);
			pb = pa + (pe - pb);
			// Check if the swab sorted everything or not
			if (is_lt(pb, pb - es))
				goto ripple_again;
			goto ripple_pop;
		}

		// Just adjust the pointers for the new limits
		rp = pe;
		bs = rp - pb;
	}

	// Find spot within A to split it at
	if (bs >= (es << 3)) {	// Binary search on larger A sets
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
	} else {	// Linear scan is faster for smaller A sets
		sp = pb - bs;
		for ( ; (sp != pb) && !is_lt(rp - es, sp); sp += es, rp -= es);
	}

	// Determine the byte-wise size of A
	if (!(bs = pb - sp))	
		goto ripple_pop;	// Nothing to swap.  We're done here

	// Do a single ripple at the split point
	if (bs >= BULK_SWAP_MIN) {
		swap_blk(sp, pb, bs);
	} else {
		for (char *ta = sp, *tb = pb; ta < pb; ta += es, tb += es)
			swap(ta, tb);
	}

	// PB->RP is the top part of A that was split, and  RP->PE is the rest
	// of the array we're merging into.  Capture the decision on if there's
	// more above to work on.  *NOTE* - We're being naughty by over-loading
	// the use of `bs` here for speed, so just be aware of that
	bs = ((rp != pe) && is_lt(rp, rp - es));

	// PA->SP is one sorted array, and SP->PB is another. PB is a hard upper
	// limit on the search space for this merge, so it's used as the new PE
	if (is_lt(sp, sp - es)) {
		if (bs) RIPPLE_STACK_PUSH(pb, rp, pe);
		pe = pb;
		pb = sp;
		goto ripple_again;
	}
	if (bs) {
		pa = pb;
		pb = rp;
		goto ripple_again;
	}

ripple_pop:
	RIPPLE_STACK_POP;
} // ripple_merge_in_place

#else

// Assumes NA and NB are greater than zero
static void
ripple_merge_in_place(char *pa, size_t na, char *pb, char *pe)
{
	size_t	bs;
	WORD	t;		// Temporary variable for swapping

ripple_again:
	if (na == 1) {
		// Just insert merge single items. We already know that *PB < *PA
		do {
			swap(pa, pb);
			pa = pb;   pb += es;
		} while (pb < pe && is_lt(pb, pa));
		return;
	}

	// Ripple A up as far as we can
	// Technically we can test up to fp <= pe, but this occurs extremely
	// rarely and would require an extra if statement below that messes
	// with branch prediction, which slows things down
	bs = pb - pa;		// Determine the byte-wise size of A
	for (char *fp = pb + bs ; fp < pe && is_lt(fp - es, pa); fp += bs) {
		if (bs < BULK_SWAP_MIN) {
			for ( ; pb < fp; pa += es, pb += es)
				swap (pa, pb);
		} else {
			swap_blk(pa, pb, bs);
			pa = pb;     pb = fp;
		}
	}

	// If we get here, we couldn't roll the full A block any further
	// Split the A block into two, and keep trying with remainder
	// The imbalanced split here improves algorithmic performance.
//	size_t	hna = (((pb - pa) / es) >> 2) + 1;
	size_t	hna = (na >> 2) + 1;
	char	*hpa = pa + hna * es;

	if (is_lt(pb, pb - es))
		ripple_merge_in_place(hpa, na - hna, pb, pe);

	if (is_lt(hpa, hpa - es)) {
		na = hna;
		pb = hpa;
		goto ripple_again;
		// ripple_merge_in_place(pa, na, hpa, pe);
	}
} // ripple_merge_in_place
#endif


// Merges A and B together using workspace W
// Assumes both NA and NB are > zero on entry
static void
fim_merge_with_workspace(char *w, char *a, const size_t na, char *b, const size_t nb)
{
	WORD	t;

	// Skip initial part of A as required
	for ( ; (a != b) && !is_lt(b, a); a += es);

	if (a == b)	// Nothing to merge
		return;

	char	*e = b + (nb * es);
	char	*pw = w;

	// Now copy everything remaining from A to W
	if ((b - a) < BULK_SWAP_MIN) {
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
} // fim_merge_with_workspace


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
		fim_merge_with_workspace(w, pb, nb, pc, nc);

	if (is_lt(pb, pb - es))
		fim_merge_with_workspace(w, pa, na, pb, nb + nc);
} // fim_sort_with_workspace


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

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
		fim_merge_with_workspace(ws, pb, nb, pc, nc);

	// Now finally sort A
	fim_sort_main(pa, na);

	// Now use the Ripple Merge In Place algorithm to merge A into BC
	// Ripple Merge doesn't need a workspace, but it's half the speed
	// of doing a workspace based merge, so we invoke it sparingly
//	if (is_lt(pb, pb - es))
		ripple_merge_in_place(pa, pb, pa + n * es);
} // fim_sort_main


static void
simplest(char *pa, const size_t n)
{
	// Handle small array size inputs with insertion sort
	if (n <= 7)
		return fim_insert_sort(pa, n);

	size_t	na = (n >> 2) + 1, nb = n - na;
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
