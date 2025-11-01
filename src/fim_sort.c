#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "swap.h"

extern	void	print_array(void *a, size_t n);
extern	uint64_t	numcmps;

static	const	int	(*is_lt)(const void *, const void *);
static	int	swaptype;
static	size_t	es;
static	size_t	rsd = 0, mrsd = 0;

#define	INSERT_SORT_MAX		  9
#define	SWAP_BLOCK_MIN		256
#define	SKEW			2

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


// Implementation of standard insertion sort
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

	for (ta = pa + es; ta != pe; ta += es)
		for (tb = ta; tb != pa && is_lt(tb, tb - es); tb -= es)
			swap(tb, tb - es);
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
} // insertion_merge_in_place


// Stack size needs to be 16 * log16(N), where N is the size of the block
// being merged. 80 covers 10^6 items. 160 covers 10^12 items.  240 is 10^18
#define	SPLIT_STACK_SIZE	160

static void
ripple_split_in_place(char *pa, char *pb, char *pe)
{
	_Alignas(64) char *_stack[SPLIT_STACK_SIZE * 2];
	char	**stack = _stack;
	size_t	split_size, bs;
	WORD	t;	// Temporary variable for swapping

	// For whoever calls us, check if we need to do anything at all
	if (!is_lt(pb, pb - es))
		goto split_pop;

	// Determine our initial split size.  Ensure a minimum of 1 element
	split_size = ((((pb - pa) / es) + 15) >> 4) * es;

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
	for (char *rp = pb + bs; (rp < pe) && is_lt(rp - es, pa); rp += bs)
		if (bs < SWAP_BLOCK_MIN) {
			for ( ; pb < rp; pa += es, pb += es)
				swap(pa, pb);
		} else {
			swap_blk(pa, pb, bs);
			pa = pb;     pb = rp;
		}

	// Split the A block into two, and keep trying with remainder
	// The imbalanced split here improves algorithmic performance.
	if (is_lt(pb, pb - es)) {
		char	*spa = pa + split_size;

		// Keep our split point within limits
		spa = (spa > (pb - es)) ? (pb - es) : spa;

		// Push a new split point to the work stack
		*stack++ = pa;  *stack++ = spa;

		pa = spa;
		goto split_again;
	}

split_pop:
	while (stack != _stack) {
		pb = *--stack;  pa = *--stack;
		split_size = ((((pb - pa) / es) + 15) >> 4) * es;

		if (is_lt(pb, pb - es))
			goto split_again;
	}
} // ripple_split_in_place

#define	RIPPLE_STACK_SIZE	240

#define	RIPPLE_STACK_PUSH(s1, s2, s3) 	\
	{ *stack++ = s1; *stack++ = s2; *stack++ = s3; }

#define	RIPPLE_STACK_POP(s1, s2, s3) \
	{ s3 = *--stack; s2 = *--stack; s1 = *--stack; }


// Assumes NA and NB are greater than zero
static void
ripple_merge_in_place(char *pa, char *pb, char *pe)
{
	alignas(64) char *_stack[RIPPLE_STACK_SIZE * 3];
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
		ripple_split_in_place(pa, pb, pe);
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
		for (char *ta = sp, *tb = pb; ta < pb; ta += es, tb += es)
			swap(ta, tb);
	}

	// PB->RP is the top part of A that was split, and  RP->PE is the rest
	// of the array we're merging into.
	// PA->SP is one sorted array, and SP->PB is another. PB is a hard upper
	// limit on the search space for this merge, so it's used as the new PE
	if ((sp != pa) && is_lt(sp, sp - es)) {
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



#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

#if 0

// VW points at work space
// NW is the number of bytes in the workspace
// vpa is the start of the first array
// vpb is the start of the second array
// vpe points immediately after the end of the second array
// Assumptions:
// - vpa and vpb are adjacent

// Moves a block from BP->BE such that BE is now at NE (New End)
// Assumes NE is not within BP->BE
static void
move_block_up(char *bp, char *be, char *ne)
{
	while (be > bp) {
		be -= es;
		ne -= es;
		swap(be, ne);
	}
} // move_block_up


// Moves a block from BP->BE such that BP is now at NP (New Pointer)
// Assumes NP is not within BP->BE
static void
move_block_down(char *bp, char *be, char *np)
{
	while (bp < be) {
		swap(bp, np);
		bp += es;
		np += es;
	}
} // move_block_up


static void
merge_retricted_workspace(void *VW, size_t NW, void *vpa, void *vpb, void *vpe)
{
	char	*hwe = (char *)VW + (NW - (NW % es));	// Hard end of workspace
	char	*wa = (char *)VW;		// Start of A's data in workspace
	char	*we = wa;			// End of data from A in workspace

	char	*pa = (char *)vpa;		// Start of A's elements
	char	*ae = pb;			// End of A's elements

	char	*pb = (char *)vpb;		// Start of B's elements
	char	*pe = (char *)vpe;		// End of A + B

	char	*mp = pa;			// Merge pointer within A+B

	// First skip everything in A that we don't need to copy
	for ( ; (pa != pb) && !is_lt(pb, pa); pa += es);

	if (pa == ea)	// Nothing to merge
		return;

	for (int first_time = 1;;) {
		// Move anything in the workspace down to the start
		// (but only if it's not already there)
		if ((wa < we) && (wa > (char *)VW)) {
			move_block_down(wa, we, (char *)VW);
			we = (char *VW) + (we - wa);
			wa = (char *VW);
		}

		// Copy as much of A into the workspace as we can fit
		while((we < hwe) && (pa < ea)) {
			swap(we, pa);
			we += es;
			pa += es;
		}

		// Move A up to be adjacent with B if there's room
		if ((pa < ae) && (ae < pb)) {
			move_block_up(pa, ae, pb);
			pa += (pb - ae);
			ae = pb;
		}

		// We already know first *PB is less than *WA
		if (first_time) {
			swap(mp, pb);
			pb += es;
			mp += es;
			first_time = 0;
		}

		// Now merge from B and workspace back into MP
		while ((wa < we) && (mp < pa) && (pb < pe)) {
			if (is_lt(pb, wa)) {
				swap(mp, pb);
				pb += es;
			} else {
				swap(mp, wa);
				wa += es;
			}
			mp += es;
		}

		// if mp == pa - we merged up as far as we could
		// if wa == we - we ran out of data in the workspace
		// If pb == pe - we've already merged all of B

		// Check if we're done merging
		if (pb < pe)
			continue;

		// B MUST be empty
		// Move anything remaining in A to the end
		if ((pa < ae) && (ae < pb)) {
			move_block_up(pa, ae, pb);
			pa += (pb - ae);
			ae = pb;
		}

		// Just swap back everything from the work-space into mp
		if (wa < we)
			move_block_up(wa, we, pa);

		break;
	}
} // merge_retricted_workspace

#endif


// Merges A and B together using workspace W
// Assumes both NA and NB are > zero on entry
static void
fim_merge_using_workspace(char *w, char *a, const size_t na, char *b, const size_t nb)
{
	WORD	t;

	// Check if we need to do anything at all!
	if (!is_lt(b, b - es))
		return;

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


static char *
extract_unique_sub(char * const a, const size_t n)
{
	char	*pa = a;
	char	*pe = a + ((n - 1) * es);
	char	*pu = a;
	WORD	t;

	fim_insert_sort(a, n);

	// Note that the last item is always desginated
	// as unique, within the listen given
	for (char *ta = a; (ta < pe); ta += es) {
		if (is_lt(ta, ta + es))
			continue;

		// It is a duplicate.  Move it down
		for (char *dp = ta; dp > pu; dp -= es)
			swap(dp, dp - es);

		pu += es;
	}

	return pu;
} // extract_unique_sub


static char *
extract_uniques(char * const a, const size_t n)
{
	WORD	t;

	if (n < 25)
		return extract_unique_sub(a, n);

	size_t	na = n / 5;
	size_t	nb = n - na;
	char	*pa = a;
	char	*pb = a + (na * es);
	char	*pe = a + (n * es);

	char	*upa = extract_uniques(pa, na);
	char	*upb = extract_uniques(pb, nb);
	char	*pu = upa;

#if 0
	printf("PA -> UPA: "); print_array(pa, (upa - pa) / es);
	printf("UPA -> PB: "); print_array(upa, (pb - upa) / es);
	printf("PB -> UPB: "); print_array(pb, (upb - pb) / es);
	printf("UPB -> PE: "); print_array(upb, (pe - upb) / es);
#endif

	// Now extract uniques from UPA->PB, and PB->UPB
	for (char *ta = upa, *tb = pb; ta < pb; ta += es) {
		// Note, convert this to binary scan
		while ((tb < upb) && is_lt(tb, ta))
			tb += es;

		if (tb == upb)
			break;

		if (is_lt(ta, tb))
			continue;

		// ta is a duplicate of what's at tb
		for (char *dp = ta; dp > pu; dp -= es)
			swap(dp, dp - es);

		pu += es;
	}

	char	*puu = pu;

	for (char *ta = pu, *tb = upb; ta < pb; ta += es) {
		while ((tb < pe) && is_lt(tb, ta))
			tb += es;

		if (tb == pe)
			break;

		if (is_lt(ta, tb))
			continue;

		for (char *dp = ta; dp > puu; dp -= es)
			swap(dp, dp - es);

		puu += es;
	}

#if 0
	printf("START -> "); print_array(a, n);
	printf("UPA -> PU: "); print_array(upa, (pu - upa) / es);
	printf("PU -> PUU: "); print_array(pu, (puu - pu) / es);
	printf("PUU -> PB: "); print_array(puu, (pb - puu) / es);
#endif

	// Merge upa -> pu -> puu together
	if ((pu > upa) && (puu > pu))
		ripple_merge_in_place(upa, pu, puu);

	// Move our extracted set up uniques up
	if ((pb > puu) && (upb > pb)) {
		_swab(puu, pb, upb);
	}
	pb = upb - (pb - puu);

	// Merge upa -> puu -> pb together
	if ((puu > upa) && (pb > puu))
		ripple_merge_in_place(upa, puu, pb);

	// Merge pa -> upa -> pb together
	if ((upa > pa) && (pb > upa))
		ripple_merge_in_place(pa, upa, pb);

	// Merge pb->upb->pe together
	if ((upb > pb) && (pe > upb))
		ripple_merge_in_place(pb, upb, pe);

#if 0
	printf("PA -> PB -> "); print_array(pa, (pb - pa) / es);
	printf("PB -> PE -> "); print_array(pb, (pe - pb) / es);
#endif

	return pb;
} // extract_uniques


static void
fim_sort_using_workspace(char * const ws, const size_t wn, char * const pa, const size_t n)
{
	// Handle small array size inputs with insertion sort
	if (n < 20)
		return fim_insert_sort(pa, n);

	size_t	na = n >> 2;	// 1/4 appears to work best

	if (na > wn)		// Make sure we don't exceed available workspace
		na = wn;

	size_t	nb = n - na;
	char	*pb = pa + (na * es);

	// First sort A
	fim_sort_using_workspace(ws, wn, pa, na);

	// Now sort B
	fim_sort_using_workspace(ws, wn, pb, nb);

	// Now merge A with B
	fim_merge_using_workspace(ws, pa, na, pb, nb);
} // fim_sort_using_workspace


static void
fim_sort_main(char * const pa, const size_t n)
{
	// Handle small array size inputs with insertion sort
	if (n <= 10)
		return fim_insert_sort(pa, n);

	// The RippleSort Algorithm works best when rippling in arrays
	// that are 1/4 the size of the target array.
	
	size_t	na = n / 10;

	if (na < 9)
		na = 9;

	size_t	nb = n - na;

	char	*pb = pa + (na * es);

	// Sort using A as workspace
	fim_sort_using_workspace(pa, na, pb, nb);

	// Now sort the workspace
	fim_sort_main(pa, na);

//	printf("After Merge: Num Compares = %ld, Num Swaps = %ld\n\n", numcmps, numswaps);

	// Now ripple merge A with B
//	ripple_split_in_place(pa, pb, pa + n * es);
	ripple_merge_in_place(pa, pb, pa + n * es);
} // fim_sort_main


static void
simplest(char *pa, const size_t n)
{
	// Handle small array size inputs with insertion sort
	if (n <= 7)
		return fim_insert_sort(pa, n);

	size_t	na = (n >> 2) + 1;
	size_t	nb = n - na;
	char	*pb = pa + na * es;

	if (na > 1)
		simplest(pa, na);

	if (nb > 1)
		simplest(pb, nb);

//	ripple_split_in_place(pa, pb, pa + n * es);
	ripple_merge_in_place(pa, pb, pa + (n * es));
} // simplest

#pragma GCC diagnostic pop


static int
cmp(void *va, void *vb)
{
	numcmps++;
	return *(int *)va - *(int *)vb;
} // cmp


static void
merge_test(char *a, const size_t n)
{
	int	mid = n / 2;

	for (int i = 0; i < mid; i++) {
		char *pos = a + (i * es);
		*(int *)pos = (int)(i);
	}

	for (int i = mid; i < n; i++) {
		char *pos = a + (i * es);
		*(int *)pos = (int)(i - mid);
	}

	char	*pa = a;
	char	*pb = a + (mid * es);
	char	*pe = a + (n * es);

//	qsort(a, n, es, cmp);
	ripple_merge_in_place(pa, pb, pe);
} // merge_test

void
fim_sort(char *a, const size_t n, const size_t _es, const int (*_is_lt)(const void *, const void *))
{
	es = _es;
	is_lt = _is_lt;

	SWAPINIT(a, es);

//	print_array(a, n);

//	extract_uniques(a, n);
//	merge_test(a, n);
//	fim_sort_main(a, n);
	simplest(a, n);
//	print_array(a, n);
} // fim_sort
