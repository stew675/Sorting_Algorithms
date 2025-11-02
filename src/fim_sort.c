#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "swap.h"

extern	void	print_array(void *a, size_t n);
extern	void	print_value(void *a);
extern	uint64_t	numcmps;

#define	COMMON_PARAMS	const size_t es, const int swaptype, const int (*is_lt)(const void *, const void *)
#define	COMMON_ARGS	es, swaptype, is_lt

#define	INSERT_SORT_MAX		  9
#define	SWAP_BLOCK_MIN		256

// Swaps two contiguous blocks of differing lengths in place efficiently
// Basically implements the well known block Rotate() functionality
// Takes advantage of any vectorization in the optimized memcpy library functions
static void
swap_blk(char *a, char *b, size_t n)
{
	_Alignas(64) char t[1024];

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
// Basically implements the well known block Rotate() functionality
static void
_swab(char *a, char *b, char *e, const size_t es, const int swaptype)
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


// Implementation of standard insertion sort
static void
fim_insert_sort(char *pa, const size_t n, COMMON_PARAMS)
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

	for (ta = pa + es; ta != pe; ta += es)
		for (tb = ta; tb != pa && is_lt(tb, tb - es); tb -= es)
			swap(tb, tb - es);
} // fim_insert_sort


// Merge two sorted sub-arrays together using insertion sort
static void
insertion_merge_in_place(char * restrict pa, char * restrict pb,
			 char * restrict pe, COMMON_PARAMS)
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
} // insertion_merge_in_place


// Stack size needs to be 16 * log16(N), where N is the size of the block
// being merged. 80 covers 10^6 items. 160 covers 10^12 items.  240 is 10^18
// 1 work stack position holds 2 pointers (16 bytes on 64-bit machines)
#define	SPLIT_STACK_SIZE	160
#define	SPLIT_SIZE		(((((pb - pa) / es) + 15) >> 4) * es)

static void
split_merge_in_place(char *pa, char *pb, char *pe, COMMON_PARAMS)
{
	_Alignas(64) char *_stack[SPLIT_STACK_SIZE * 2];
	char	**stack = _stack, *rp, *spa;
	size_t	split_size, bs;
	WORD	t;	// Temporary variable for swapping

	// For whoever calls us, check if we need to do anything at all
	if (!is_lt(pb, pb - es))
		goto split_pop;

	// Determine our initial split size.  Ensure a minimum of 1 element
	split_size = SPLIT_SIZE;

split_again:
	bs = pb - pa;		// Determine the byte-wise size of A

	// Just insert merge single items. We already know that *PB < *PA
	if (bs == es) {
		do {
			swap(pa, pb);  pa = pb;  pb += es;
		} while ((pb != pe) && is_lt(pb, pa));
		goto split_pop;
	}

	// Advance the PA->PB block up as far as we can
	for (rp = pb + bs; (rp < pe) && is_lt(rp - es, pa); rp += bs)
		if (bs < SWAP_BLOCK_MIN) {
			for ( ; pb < rp; pa += es, pb += es)
				swap(pa, pb);
		} else {
			swap_blk(pa, pb, bs);
			pa = pb;     pb = rp;
		}

	// Split the A block into two, and keep trying with remainder
	// The imbalanced split here improves algorithmic performance.
	rp = pb - es;  spa = pa + split_size;
	if (is_lt(pb, rp)) {
		// Keep our split point within limits
		spa = (spa > rp) ? rp : spa;

		// Push a new split point to the work stack
		*stack++ = pa;
		*stack++ = spa;

		pa = spa;
		goto split_again;
	}

split_pop:
	while (stack != _stack) {
		pb = *--stack;
		pa = *--stack;

		if (is_lt(pb, pb - es)) {
			split_size = SPLIT_SIZE;
			goto split_again;
		}
	}
} // split_merge_in_place

// 1 work stack position holds 3 pointers (24 bytes on 64-bit machines)
#define	RIPPLE_STACK_SIZE	240

#define	RIPPLE_STACK_PUSH(s1, s2, s3) 	\
	{ *stack++ = s1; *stack++ = s2; *stack++ = s3; }

#define	RIPPLE_STACK_POP(s1, s2, s3) \
	{ s3 = *--stack; s2 = *--stack; s1 = *--stack; }


// Assumes NA and NB are greater than zero
static void
ripple_merge_in_place(char *pa, char *pb, char *pe, COMMON_PARAMS)
{
	_Alignas(64) char *_stack[RIPPLE_STACK_SIZE * 3];
	char	**maxstack, **stack = _stack;
	char	*rp, *sp;	// Ripple-Pointer, and Split Pointer
	size_t	bs;		// Byte-wise block size of pa->pb
	WORD	t;		// Temporary variable for swapping

	maxstack = _stack + (sizeof(_stack) / sizeof(*_stack));

	// For whoever calls us, check if we need to do anything at all
	if (!is_lt(pb, pb - es))
		goto ripple_pop;

ripple_again:
	// If our stack is about to over-flow, move to use the slower, but more
	// resilient, algorithm that handles degenerate scenarios without issue
	if (stack == maxstack) {
		split_merge_in_place(pa, pb, pe, COMMON_ARGS);
		goto ripple_pop;
	}

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
		insertion_merge_in_place(pa, pb, pe, COMMON_ARGS);
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
		bs = pe - pb; // Adjust the block size for the end limit
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
		sp = pb - bs;	rp = pb + bs;
		for ( ; (sp != pb) && !is_lt(rp - es, sp); sp += es, rp -= es);
	}

	if (!(bs = pb - sp))	  // Determine the byte-wise size of the split
		goto ripple_pop;  // If nothing to swap, we're done here

	// Do a single ripple at the split point
	if (bs >= SWAP_BLOCK_MIN) {
		swap_blk(sp, pb, bs);
	} else {
		for (char *ta = sp, *tb = pb; ta != pb; ta += es, tb += es)
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
	}

	if ((rp != pe) && is_lt(rp, rp - es)) {
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


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

// Merges A and B together using workspace W
// Assumes both NA and NB are > zero on entry
// Requires workspace to be at least as large as A
static void
fim_merge_using_workspace(char *a, const size_t na, char *b, const size_t nb,
			  char *w, const size_t nw, COMMON_PARAMS)
{
	WORD	t;

	// Make sure our caller didn't screw up
	assert(nw >= na);

	// Check if we need to do anything at all!
	if (!is_lt(b, b - es))
		return;

	// Skip initial part of A if opportunity presents
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

	// Swap back any remainder
	if (pw > w) {
		if ((pw - w) >= SWAP_BLOCK_MIN) {
			swap_blk(a, w, (pw - w));
		} else {
			for ( ; w != pw; w += es, a += es)
				swap(a, w);
		}
	}
} // fim_merge_using_workspace


// Base merge-sort algorithm - I'm all 'bout that speed baby!
// NOTE - Using a pre-allocated work-space makes this algorithm sort-stable
static void
fim_base_sort(char * const a, const size_t n, char * const ws,
	      const size_t nw, COMMON_PARAMS)
{
	// Handle small array size inputs with insertion sort
	if (n <= 20)
		return fim_insert_sort(a, n, COMMON_ARGS);

	// The RippleSort Algorithm works best when rippling in arrays
	// that are 1/4 the size of the target array.

	if (ws) {
		// Use the work-space given to us

		// 1/4 appears to work best, but feel free to experiment
		char	*pa = a;
		size_t	na = n >> 2;

		// Make sure we don't exceed available workspace
		if (na > nw)
			na = nw;

		size_t	nb = n - na;
		char	*pb = pa + (na * es);

		// First sort A
		fim_base_sort(pa, na, ws, nw, COMMON_ARGS);

		// Now sort B
		fim_base_sort(pb, nb, ws, nw, COMMON_ARGS);

		// Now merge A with B
		fim_merge_using_workspace(pa, na, pb, nb, ws, nw, COMMON_ARGS);
	} else {
		// We have to carve out our own workspace
		char	*pa = a;
		size_t	na = n / 10;

		if (na < 9)
			na = 9;

		size_t	nb = n - na;
		char	*pb = pa + (na * es);

		// Sort B using A as workspace
		fim_base_sort(pb, nb, pa, na, COMMON_ARGS);

		// Now sort the workspace
		fim_base_sort(pa, na, NULL, 0, COMMON_ARGS);

		// Now ripple merge A with B
//		split_merge_in_place(pa, pb, pa + n * es);
		ripple_merge_in_place(pa, pb, pa + n * es, COMMON_ARGS);
	}
} // fim_base_sort


#if 0

// Classic bottom-up merge sort
static void
stable_sort(char *pa, const size_t n, COMMON_PARAMS)
{
#define	STABLE_UNIT_SIZE 10

	// Handle small array size inputs with insertion sort
	if (n < (STABLE_UNIT_SIZE * 2))
		return fim_insert_sort(pa, n, COMMON_ARGS);

	char	*pe = pa + (n * es);
	WORD	t;

	do {
		size_t	bound = n - (n % STABLE_UNIT_SIZE);
		char	*bpe = pa + (bound * es);

		// First just do insert sorts on all with size STABLE_UNIT_SIZE
		for (char *pos = pa; pos != bpe; pos += (es * STABLE_UNIT_SIZE)) {
			char	*stop = pos + (es * STABLE_UNIT_SIZE);
			for (char *ta = pos + es, *tb; ta != stop; ta += es)
				for (tb = ta; tb != pos && is_lt(tb, tb - es); tb -= es)
					swap(tb, tb - es);
		}

		// Insert sort any remainder
		if (n - bound)
			fim_insert_sort(bpe, n - bound, COMMON_ARGS);
	} while (0);

	for (size_t size = STABLE_UNIT_SIZE; size < n; size += size) {
		char	*stop = pa + ((n - size) * es);
		for (char *pos1 = pa; pos1 < stop; pos1 += (size * es * 2)) {
			char *pos2 = pos1 + (size * es);
			char *pos3 = pos1 + (size * es * 2);

			if (pos3 > pe)
				pos3 = pe;

			if (pos2 < pe)
				ripple_merge_in_place(pos1, pos2, pos3, COMMON_ARGS);
		}
	}
#undef STABLE_UNIT_SIZE
} // stable_sort

#else

// Top-down merge sort with a bias to smaller left-side arrays as
// this appears to help the in-place merge algorithm a little bit
// This makes it a hair faster than the bottom-up merge
static void
stable_sort(char *pa, const size_t n, COMMON_PARAMS)
{
	// Handle small array size inputs with insertion sort
	if (n <= 11)
		return fim_insert_sort(pa, n, COMMON_ARGS);

//	size_t	na = n >> 2;
//	size_t	na = (n * 2) / 5;
	size_t	na = ((n + 3) * 3) / 10;
	size_t	nb = n - na;
	char	*pb = pa + na * es;

	if (na > 1)
		stable_sort(pa, na, COMMON_ARGS);

	if (nb > 1)
		stable_sort(pb, nb, COMMON_ARGS);

//	split_merge_in_place(pa, pb, pa + (n * es), COMMON_ARGS);
	ripple_merge_in_place(pa, pb, pa + (n * es), COMMON_ARGS);
} // stable_sort
#endif


// Designed for efficiently processing smallish sets of items
// Note that the last item is always assumed to be unique
// TODO - give hints from caller for existing duplicate runs
static char *
extract_unique_sub(char * const a, char * const pe, char *hints, COMMON_PARAMS)
{
	char	*pu = a;	// Points to list of unique items
	WORD	t;

	if (hints == NULL)
		hints = pe;

	for (char *pa = a + es; pa < pe; pa += es) {
		if (is_lt(pa - es, pa))
			continue;

		// The item before our position is a duplicate
		// Mark it, and then find the end of the run
		char *dp = pa - es;

		// Now find the end of the run of duplicates
		for (pa += es; (pa < pe) && !is_lt(pa - es, pa); pa += es);
		pa -= es;

		// pa now points at the last item of the duplicate run
		// Roll the duplicates down
		if ((pa - dp) > es) {
			// Multiple items.  swab them down
			if (dp > pu)
				_swab(pu, dp, pa, es, swaptype);
			pu += (pa - dp);
		} else {
			// Single item, just ripple it down
			while (dp > pu) {
				swap(dp, dp - es);
				dp -= es;
			}
			pu += es;
		}
		pa += es;
	}

	return pu;
} // extract_unique_sub


// Assumptions:
// - The list we're passed is already sorted
// TODO - give hints from caller for existing duplicate runs
static char *
extract_uniques(char * const a, const size_t n, char *hints, COMMON_PARAMS)
{
	char	*pe = a + (n * es);

	// I'm not sure what a good value should be here, but 40 seems okay
	if (n < 40)
		return extract_unique_sub(a, pe, NULL, COMMON_ARGS);

	if (hints == NULL)
		hints = pe;

	// Divide and conquer!
	char	*pa = a;
	size_t	na = (n + 3) >> 2;	// Looks to be about right
	char	*pb = pa + (na * es);

	// First find where to split at, which basically means, find the
	// end of any duplicate run that we may find ourselves in
	while ((pb < pe) && !is_lt(pb - es, pb))
		pb += es;

	// If we couldn't find a sub-split, just process what we have
	if (pb == pe)
		return extract_unique_sub(a, pe, NULL, COMMON_ARGS);

	// Recalculate our size
	na = (pb - pa) / es;
	size_t	nb = n - na;

	// Note that there is ALWAYS at least one unique to be found
	char	*apu = extract_uniques(pa, na, NULL, COMMON_ARGS);
	char	*bpu = extract_uniques(pb, nb, NULL, COMMON_ARGS);

	// Coalesce non-uniques together
	if (bpu > pb) {
		_swab(apu, pb, bpu, es, swaptype);
	}
	pb = apu + (bpu - pb);

	// PA->BP now contains non-uniques and BP->PE are uniques
	return pb;
} // extract_uniques


// Here we recursively use the work-space to sort itself
static void
fim_re_sort_workspace(char * const pa, const size_t n, COMMON_PARAMS)
{
	// Handle small array size inputs with insertion sort
	if (n <= 11)
		return fim_insert_sort(pa, n, COMMON_ARGS);

	size_t	na = n / 10;
	char	*pb = pa + (na * es);
	size_t	nb = n - na;

	fim_base_sort(pb, nb, pa, na, COMMON_ARGS);

	// Now re-sort what we used as work-space
	fim_re_sort_workspace(pa, na, COMMON_ARGS);

	// Now merge them together
	ripple_merge_in_place(pa, pb, pa + (n * es), COMMON_ARGS);
} // fim_re_sort_workspace


#define	EXTRACT_UNIQUES		1
#define	DO_NOT_EXTRACT_UNIQUES	0
// Workspace ratio is the amount we divide N by.
// Somewhere between 10 and 30 seems best
#define	WORKSPACE_RATIO		16

static void
fim_stable_sort(char * const a, const size_t n, COMMON_PARAMS)
{
	bool	sorted = false;
	int	num_tries = 2;
	size_t	na, nr, nw;
	char	*ws, *pr;

	// Okay, so A->WS is a set of sorted non-uniques
	// 40 items appears to be the cross-over
	if (n <= 40)
		return stable_sort(a, n, COMMON_ARGS);

	na = n / WORKSPACE_RATIO;
	nr = n - na;
	pr = a + (na * es);	// Pointer to rest

	// First sort our candidate work-space chunk
	stable_sort(a, na, COMMON_ARGS);

	ws = extract_uniques(a, na, NULL, COMMON_ARGS);
	nw = (pr - ws) / es;
	na = na - nw;

	// A->WS  is pointing at non-uniques
	// WS->PR is a set of uniques we can use as workspace
	// PR->PE is everything else that we still need to sort

	// If we didn't get enough work-space, we'll try twice more
	// before giving up and falling back to stable_sort
	// We'll give up immediately if we couldn't even find 1%
	while (((nw * WORKSPACE_RATIO * 12) >> 3) < nr) {
		if ((num_tries-- <= 0) || ((nw * 100) < nr)) {
			// Give up and fall back to old faithful
			stable_sort(pr, nr, COMMON_ARGS);
			sorted = true;
			break;
		}

//		printf("Not enough workspace - Trying harder\n");
		size_t	nna = nr / 9;
		char	*nws = pr;
		nr -= nna;
		pr = pr + (nna * es);

		// Sort new work-space candidate
		stable_sort(nws, nna, COMMON_ARGS);

		// Merge old workspace with new
		ripple_merge_in_place(ws, nws, pr, COMMON_ARGS);

		// We may have picked up new duplicates.  Separate them
		nws = extract_uniques(ws, nw + nna, NULL, COMMON_ARGS);

		// Merge original duplicates with new ones
		if ((nws > ws) && (ws > a))
			ripple_merge_in_place(a, ws, nws, COMMON_ARGS);

		ws = nws;
		nw = (pr - ws) / es;
	}

	if (!sorted) {
		// Sort the remainder using the workspace we extracted
		fim_base_sort(pr, nr, ws, nw, COMMON_ARGS);

		// Now re-sort our work-space.
		fim_re_sort_workspace(ws, nw, COMMON_ARGS);
	}

	if (na > nw) {
		// Merge our work-space with the rest
		ripple_merge_in_place(ws, pr, a + (n * es), COMMON_ARGS);

		// Merge our non-uniques with the rest
		if (na > 0)
			ripple_merge_in_place(a, ws, a + (n * es), COMMON_ARGS);
	} else {
		// Merge our non-uniques with our workspace
		if (na > 0)
			ripple_merge_in_place(a, ws, pr, COMMON_ARGS);

		// Now merge the lot together
		ripple_merge_in_place(a, pr, a + (n * es), COMMON_ARGS);
	}
} // fim_stable_sort


#pragma GCC diagnostic pop

void
fim_sort(char *a, const size_t n, const size_t es,
	 const int (*is_lt)(const void *, const void *))
{
	int	swaptype;

	SWAPINIT(a, es);

//	stable_sort(a, n, COMMON_ARGS);
//	fim_base_sort(a, n, NULL, 0, COMMON_ARGS);
	fim_stable_sort(a, n, COMMON_ARGS);
//	print_array(a, n);
} // fim_sort
