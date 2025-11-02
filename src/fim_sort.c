#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>

//				TUNING KNOBS!
// INSERT_SORT_MAX defines the number of items below which we'll resort to
// simply using Insertion Sort.  Anything from 8-30 seems reasonable, with
// values around 20+ giving best speeds at the expense of more swaps/compares
#define	INSERT_SORT_MAX		24

// SKEW defines the split ratio when doing top-down division of the array
// A SKEW of 2 is classic merge sort, which is likely best for larger element sizes
// A SKEW of 4 appears to be cache optimal for smaller (<16 bytes) element sizes
// SKEW values higher than 5 appear to degrade performance regardless
#define	SKEW			4

// WSRATIO defines the split ratio when choosing how much of the array to
// use as a makeshift workspace when no workspace is provided
// Experimentally anything from 3-20 works okay, but 9 appears optimal
// Using 3 would mirror a closest approximation of classic merge sort
#define	WSRATIO			9

// STABLE_WSRATIO controls the behaviour of the stable sorting "front end" to
// the main algorithm.  It has to dig out unique values from the sort space
// to use as a workspace for the main algorithm.  Since doing so isn't "free"
// there's a trade-off between spending more time digging out uniques, as
// opposed to just using what we can find.  A good value appears to be anywhere
// from 1.5x to 3x of what WSRATIO is set to,
#define	STABLE_WSRATIO		19


#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

extern	uint64_t	numcmps, numswaps;
size_t	n1 = 0, n2 = 0, n3 = 0;


#if 1
// Handy defines to keep code looking cleaner

#define	COMMON_ARGS	es, swaptype, is_lt
#define	COMMON_PARAMS	const size_t es, int swaptype, const int (*is_lt)(const void *, const void *)


static inline void
__memswap(void * restrict p1, void * restrict p2, size_t n)
{
	enum { SWAP_GENERIC_SIZE = 32 };

	unsigned char tmp[SWAP_GENERIC_SIZE];

	while (n > SWAP_GENERIC_SIZE) {
		mempcpy(tmp, p1, SWAP_GENERIC_SIZE);
		p1 = __mempcpy(p1, p2, SWAP_GENERIC_SIZE);
		p2 = __mempcpy(p2, tmp, SWAP_GENERIC_SIZE);
		n -= SWAP_GENERIC_SIZE;
	}
	while (n > 0) {
		unsigned char t = ((unsigned char *)p1)[--n];
		((unsigned char *)p1)[n] = ((unsigned char *)p2)[n];
		((unsigned char *)p2)[n] = t;
	}
} // __memswap

enum swap_type_t {
	SWAP_WORDS_64 = 0,
	SWAP_WORDS_32,
	SWAP_VOID_ARG,
	SWAP_BYTES
};

static inline void
swapfunc(void * restrict va, void * restrict vb, size_t size, int swaptype)
{
	numswaps++;
	if (swaptype == SWAP_WORDS_64) {
		uint64_t t = *(uint64_t *)va;
		*(uint64_t *)va = *(uint64_t *)vb;
		*(uint64_t *)vb = t;
	} else if (likely(swaptype == SWAP_WORDS_32)) {
		uint32_t t = *(uint32_t *)va;
		*(uint32_t *)va = *(uint32_t *)vb;
		*(uint32_t *)vb = t;
	} else {
		__memswap (va, vb, size);
	}
} // swapfunc

#define	swap(a, b)	swapfunc(a, b, es, swaptype)

static enum swap_type_t
get_swap_type (void *const pbase, size_t size)
{
	if ((size & (sizeof(uint32_t) - 1)) == 0) {
		if (((uintptr_t)pbase) % __alignof__(uint32_t) == 0) {
			if (size == sizeof(uint32_t)) {
				return SWAP_WORDS_32;
			} else if (size == sizeof(uint64_t)) {
				if (((uintptr_t)pbase) % __alignof__(uint64_t) == 0) {
					return SWAP_WORDS_64;
				}
			}
		}
	}
	return SWAP_BYTES;
} // get_swap_type

#define	TMPVAR

#else
#if 1
#include "swap.h"
#define	COMMON_ARGS	es, swaptype, is_lt
#define	COMMON_PARAMS	const size_t es, const int swaptype, const int (*is_lt)(const void *, const void *)
#define	TMPVAR		WORD t;
#else
#include "glibc_swap.h"
#define	COMMON_PARAMS	const size_t es, enum swap_type_t swaptype, const int (*is_lt)(const void *, const void *)
#define	SWAPINIT(a, b)	get_swap_type(a, b)
#define	swap(a, b)	do_swap(a, b, es, swaptype)
typedef	uint64_t	WORD;
#endif
#endif

extern	void		print_array(void *a, size_t n);
extern	void		print_value(void *a);

// Swaps two contiguous blocks of differing lengths in place efficiently
// Basically implements the well known block Rotate() functionality
// Takes advantage of any vectorization in the optimized memcpy library functions
#define	SWAP_BLOCK_MIN		256
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
_swab(char *a, char *b, char *e, COMMON_PARAMS)
{
	size_t	gapa = b - a, gapb = e - b;
	TMPVAR

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
	TMPVAR

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
	TMPVAR

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
	TMPVAR

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
	TMPVAR

	maxstack = _stack + (sizeof(_stack) / sizeof(*_stack));

	// For whoever calls us, check if we need to do anything at all
	if (!is_lt(pb, pb - es))
		goto ripple_pop;

ripple_again:
	// If our stack is about to over-flow, move to use the slower, but more
	// resilient, algorithm that handles degenerate scenarios without issue
	// If the stack is large enough, this should almost never ever happen
	if (unlikely(stack == maxstack)) {
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
	} else if ((pb + es) == pe) {
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
	// This occurs about 3% of the time, hence the use of unlikely
	if (unlikely(rp > pe)) {
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
	bs = (rp != pe) && is_lt(rp, rp - es);	// <- Oddly enough, this helps
	if (is_lt(sp, sp - es)) {
		if (rp != pe)
			RIPPLE_STACK_PUSH(pb, rp, pe);
		pe = pb;
		pb = sp;
		goto ripple_again;
	} else if (bs) {
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
	TMPVAR

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
// It logically follows that if this is given unique items to sort
// then the result will naturally yield a sort-stable result
static void
fim_base_sort(char * const pa, const size_t n, char * const ws,
	      const size_t nw, COMMON_PARAMS)
{
	// Handle small array size inputs with insertion sort
	if ((n <= INSERT_SORT_MAX) || (n < 8))
		return fim_insert_sort(pa, n, COMMON_ARGS);

	if (ws) {
		// The RippleSort Algorithm works best when rippling in arrays
		// that are roughly 1/4 the size of the target array.
		// The ratio is controlled by the SKEW #define
		size_t	na = (n / (SKEW));

		// Enforce a sensible minimum
		if (na < 4)
			na = 4;

		// Make sure we don't exceed the given available workspace
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
		// 9 appears to be close to optimal, but anything from 3-20 works
		size_t	na = n / WSRATIO;

		// Enforce a sensible minimum
		if (na < 4)
			na = 4;

		char	*pe = pa + (n * es);
		char	*pb = pa + (na * es);
		size_t	nb = n - na;

		// Sort B using A as the workspace
		fim_base_sort(pb, nb, pa, na, COMMON_ARGS);

		// Now recursively sort what we used as work-space
		fim_base_sort(pa, na, NULL, 0, COMMON_ARGS);

		// Now merge them together
//		split_merge_in_place(pa, pb, pe);
		ripple_merge_in_place(pa, pb, pe, COMMON_ARGS);
	}
} // fim_base_sort


#if 0
// Classic bottom-up merge sort
static void
stable_sort(char *pa, const size_t n, COMMON_PARAMS)
{
#define	STABLE_UNIT_SIZE 16

	// Handle small array size inputs with insertion sort
	if ((n <= INSERT_SORT_MAX) || (n < (STABLE_UNIT_SIZE * 2)));
		return fim_insert_sort(pa, n, COMMON_ARGS);

	char	*pe = pa + (n * es);
	TMPVAR

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
	// Ensure there's no way na and nb could be zero
	if ((n <= INSERT_SORT_MAX) || (n <= SKEW))
		return fim_insert_sort(pa, n, COMMON_ARGS);

	size_t	na = n / SKEW;
	size_t	nb = n - na;
	char	*pb = pa + na * es;
	char	*pe = pa + (n * es);

	stable_sort(pa, na, COMMON_ARGS);
	stable_sort(pb, nb, COMMON_ARGS);

//	split_merge_in_place(pa, pb, pe, COMMON_ARGS);
	ripple_merge_in_place(pa, pb, pe, COMMON_ARGS);
} // stable_sort
#endif


// Designed for efficiently processing smallish sets of items
// Note that the last item is always assumed to be unique
// Returns a pointer to the list of unique items positioned
// to the right-side of the array.  All duplicates are located
// at the start of the array (a)
//  A -> PU = Duplicates
// PU -> PE = Unique items
static char *
extract_unique_sub(char * const a, char * const pe, char *ph, COMMON_PARAMS)
{
	char	*pu = a;	// Points to list of unique items
	TMPVAR

	// Sanitize our hints pointer
	if (ph == NULL)
		ph = pe;

	// Process everything up to the hints pointer
	for (char *pa = a + es; pa < ph; pa += es) {
		if (is_lt(pa - es, pa))
			continue;

		// The item before our position is a duplicate
		// Mark it, and then find the end of the run
		char *dp = pa - es;

		// Now find the end of the run of duplicates
		for (pa += es; (pa < ph) && !is_lt(pa - es, pa); pa += es);
		pa -= es;

		// pa now points at the last item of the duplicate run
		// Roll the duplicates down
		if ((pa - dp) > es) {
			// Multiple items.  swab them down
			if (dp > pu)
				_swab(pu, dp, pa, COMMON_ARGS);
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

	if (ph < pe) {
		// Everything (ph - es) to (pe - es) is a duplicate
		_swab(pu, ph - es, pe - es, COMMON_ARGS);
		pu += (pe - ph);
	}

	return pu;
} // extract_unique_sub


// Returns a pointer to the list of unique items positioned
// to the right-side of the array.  All duplicates are located
// at the start of the array (a)
//  A -> PU = Duplicates
// PU -> PE = Unique items
//
// Assumptions:
// - The list we're passed is already sorted
static char *
extract_uniques(char * const a, const size_t n, char *hints, COMMON_PARAMS)
{
	char	*pe = a + (n * es);

	// I'm not sure what a good value should be here, but 40 seems okay
	if (n < 40)
		return extract_unique_sub(a, pe, hints, COMMON_ARGS);

	if (hints == NULL)
		hints = pe;

	// Divide and conquer!  This algorithm appears to operate in close
	// to an O(n) time complexity, albeit with a moderately high K factor
	char	*pa = a;
	size_t	na = (n + 3) >> 2;	// Looks to be about right
	char	*pb = pa + (na * es);

	char	*ps = pb;	// Records original intended split point

	// First find where to split at, which basically means, find the
	// end of any duplicate run that we may find ourselves in
	while ((pb < pe) && !is_lt(pb - es, pb))
		pb += es;

	// If we couldn't find a sub-split, just process what we have
	if (pb == pe)
		return extract_unique_sub(a, pe, ps, COMMON_ARGS);

	// Recalculate our size
	na = (pb - pa) / es;
	size_t	nb = n - na;

	if (hints < pb)
		hints = pe;

	// Note that there is ALWAYS at least one unique to be found
	char	*apu = extract_uniques(pa, na, ps, COMMON_ARGS);
	char	*bpu = extract_uniques(pb, nb, hints, COMMON_ARGS);

	// Coalesce non-uniques together
	if (bpu > pb) {
		_swab(apu, pb, bpu, COMMON_ARGS);
	}
	pb = apu + (bpu - pb);

	// PA->BP now contains non-uniques and BP->PE are uniques
	return pb;
} // extract_uniques


// This essentially operates as a "front end" to the main ripple-merge-sort
// sequence.  Its primary role is to extract unique values from the main
// data set, which in turn allows us to use these as a workspace to pass
// to the main algorithm.  Doing so preserves sort stability.  While it is
// generating the set of uniques, it is also still sorting by generating a
// sorted set of duplicates that were disqualified, and this allows it to
// try extra hard to find a working set as doing so isn't "time wasted".
static void
fim_stable_sort(char * const pa, const size_t n, COMMON_PARAMS)
{
	bool	sorted = false;
	int	num_tries = 19;	// This 19 here is experimentally derived
	size_t	na, nr, nw;
	char	*ws, *pr;

	// 40 items appears to be the cross-over
	if (n <= 40)
		return stable_sort(pa, n, COMMON_ARGS);

	// We start with a workspace candidate size that is 1.1x whatever
	// the STABLE_WSRATIO is asking for.  This allows for up to a 10%
	// duplicate ratio on the first attempt before we try harder

	na = ((n * 10) / 9) / STABLE_WSRATIO;
	nr = n - na;
	pr = pa + (na * es);	// Pointer to rest

	size_t	wstarget = nr / STABLE_WSRATIO;

	// First sort our candidate work-space chunk
	stable_sort(pa, na, COMMON_ARGS);

	ws = extract_uniques(pa, na, NULL, COMMON_ARGS);
	nw = (pr - ws) / es;
	na = na - nw;

	// PA->WS is pointing at (sorted) non-uniques
	// WS->PR is a set of uniques we can use as workspace
	// PR->PE is everything else that we still need to sort

	// If we couldn't find enough work-space we'll try up to num_tries
	// more.  Despite this seeming excessive, with each try we're still
	// sorting more of the array any way, so it sort of balances out.
	// It works out that we give up at above a 99.4% duplicate rate
	while (nw < wstarget) {
//		printf("Not enough workspace. Wanted: %ld  Got: %ld  "
//		       "Duplicates: %ld\n", wstarget, nw, (ws - pa) / es);

		if (num_tries-- <= 0) {
			// Give up and fall back to good old stable_sort()
//			printf("Giving up!\n");
			stable_sort(pr, nr, COMMON_ARGS);
			sorted = true;
			break;
		}

		// Use whatever workspace we have, to try to find more
		char	*nws = pr;
		size_t	nna = nw * 8;	// This 8 is experimentally derived

		if (nna > (nr / 4))
			nna = nr / 4;

		nr -= nna;
		pr = pr + (nna * es);

		// Sort new work-space candidate with our current workspace
		fim_base_sort(nws, nna, ws, nw, COMMON_ARGS);

		// Our current work-space is now jumbled, so sort that too
		fim_base_sort(ws, nw, NULL, 0, COMMON_ARGS);

		// Merge current workspace with the new set
		ripple_merge_in_place(ws, nws, pr, COMMON_ARGS);

		// We may have picked up new duplicates.  Separate them out
		nws = extract_uniques(ws, nw + nna, NULL, COMMON_ARGS);

		// Merge original duplicates with new ones.  We're essentially
		// sorting the array by extracting duplicates!
		if ((nws > ws) && (ws > pa))
			ripple_merge_in_place(pa, ws, nws, COMMON_ARGS);

		ws = nws;
		nw = (pr - ws) / es;
		wstarget = nr / STABLE_WSRATIO;
	}

	if (!sorted) {
		// Sort the remainder using the workspace we extracted
		fim_base_sort(pr, nr, ws, nw, COMMON_ARGS);

		// Now re-sort our work-space.  The result will be
		// sort stable because we're sorting all uniques
		fim_base_sort(ws, nw, NULL, 0, COMMON_ARGS);
	}

	char	*pe = pa + (n * es);

	if (na > nw) {
		// Merge our work-space with the rest
		ripple_merge_in_place(ws, pr, pe, COMMON_ARGS);

		// Merge our non-uniques with the rest
		if (na > 0)
			ripple_merge_in_place(pa, ws, pe, COMMON_ARGS);
	} else {
		// Merge our non-uniques with our workspace
		if (na > 0)
			ripple_merge_in_place(pa, ws, pr, COMMON_ARGS);

		// Now merge the lot together
		ripple_merge_in_place(pa, pr, pe, COMMON_ARGS);
	}
} // fim_stable_sort


#pragma GCC diagnostic pop

void
fim_sort(char *a, const size_t n, const size_t es,
	 const int (*is_lt)(const void *, const void *))
{
#if 0
	// 8 appears to be optimal
	size_t	nw = n / 8;
	char	*ws = malloc(nw * es);
#else
	size_t	nw = 0;
	char	*ws = NULL;
#endif

	int	swaptype = get_swap_type(a, es);

	// SWAPINIT(a, es);

	// printf("swaptype = %d\n", swaptype);

//	stable_sort(a, n, COMMON_ARGS);
	if (ws) {
		fim_base_sort(a, n, ws, nw, COMMON_ARGS);
		free(ws);
	} else {
		fim_base_sort(a, n, NULL, 0, COMMON_ARGS);
//		fim_stable_sort(a, n, COMMON_ARGS);
	}
//	printf("n1 = %ld, n2 = %ld, n3 = %ld\n", n1, n2, n3);
//	print_array(a, n);
} // fim_sort
