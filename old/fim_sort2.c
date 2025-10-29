#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "swap.h"

extern	uint64_t numcmps, numswaps;
extern	void	print_array(void *a, size_t n);

static	const	int	(*is_lt)(const void *, const void *);
static	int	swaptype;
static	size_t	es;
static	WORD	t;
static	size_t	n1 = 0, n2 = 0, nn = 0;
static	size_t	depth = 0, maxdepth = 0;

#define	SKEW			7	// ~7 appear to be best general purpose value
#define	INSERT_SORT_MAX		10
#define	SWAP_BLK_MIN		192

// Takes advantage of any vectorization in the optimized memcpy library functions
__attribute__((noinline))
static void
swap_blk(char *a, char *b, size_t n)
{
	__attribute__((aligned(64))) char sbt[1024];

	do {
		size_t	tc = n > 1024 ? 1024 : n;
		memcpy(sbt, a, tc);
		memcpy(a, b, tc);
		memcpy(b, sbt, tc);
		a+=tc;
		b+=tc;
		n-=tc;
	} while (n > 0);
} // swap_blk

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

#if 0
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

#else

// A modified form of insertion sort that averages less swaps
__attribute__((noinline))
static void
fim_insert_sort(char *pa, const size_t n)
{
	char	*pb = pa + es;

	if (n < 2) return;

	if (is_lt(pb, pa)) swap(pb, pa);

	if (n == 2) return;

	char	*pc = pa + (es << 1);

	if (n == 3) {
		if (is_lt(pc, pb)) {
			swap(pc, pb);
			if (is_lt(pb, pa))
				swap(pb, pa);
		}
	} else {
		char	*pd = pb + (es << 1);
		char	*pe = pa + (n * es);

		if (is_lt(pd, pc)) swap(pd, pc);

		if (is_lt(pc, pb)) {
			if (is_lt(pd, pa)) {
				swap(pa, pc);
				swap(pb, pd);
			} else {
				swap(pb, pc);
				if (is_lt(pb, pa)) swap(pb, pa);
				if (is_lt(pd, pc)) swap(pd, pc);
			}
		}

		// Right now, the first 4 are sorted
		for (pd += es; pd < pe; pd += es)
			for (pc = pd; (pc != pa) && is_lt(pc, pc - es); pc -= es)
				swap(pc, pc - es);
	}
} // fim_insert_sort
#endif

#if 1

// Swaps two contiguous blocks in place efficiently
static void
_swab(char *a, char *b, char *e)
{
	size_t	gapa = b - a, gapb = e - b;

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

#else

// Swaps two contiguous blocks in place efficiently
static void
_swab(char *a, char *b, char *e)
{
	size_t	na = (b - a) / es;
	size_t	nb = (e - b) / es;

	while (na && nb) {
		if (na < nb) {
			for (char *src = a, *dst = a + nb * es; dst < e; src += es, dst += es)
				swap(src, dst);

			nb -= na;
			e = b + nb * es;
		} else {
			for (char *src = b, *dst = a; src < e; src += es, dst += es)
				swap(src, dst);

			if (na == nb)
				return;

			a += (nb * es);
			na -= nb;
		}
	}
} // _swab

#endif

static void
rotate_merge_in_place(char *pa, char *pb, char *pe)
{
	char    *rp;	    // Roaming pointer

	while ((pa < pb) && (pb < pe)) {
		// Quickly roll A up closer to target
		size_t  bs = pb - pa;	   // Determine the byte-wise size of A
		for (rp = pb + bs; rp <= pe && is_lt(rp - es, pa); rp += bs)
			for ( ; pb != rp; pa += es, pb += es)
				swap (pa, pb);

		if (pb == pe)
			return;

		while((pa < pb) && !is_lt(pb, pa))
			pa += es;

		if (pa == pb)
			return;

		for (rp = pb; (rp < pe) && is_lt(rp, pa); rp += es);

		_swab(pa, pb, rp);
		pa += rp - pb;
		pb = rp;
	}
} // rotate_merge_in_place


#pragma GCC diagnostic pop

// Merge two sorted sub-arrays together using insertion sort
static void
insertion_merge_in_place(char * restrict pa, char * restrict pb, char * restrict pe)
{
	// Check if the arrays aren't already sorted
	// This is true about 25% of the time
	if (!is_lt(pb, pb - es))
		return;

	// Check if the arrays aren't just reversed,  This is also
	// true about 25% of the time that this function is called
	if (is_lt(pe - es, pa))
		return _swab(pa, pb, pe);

	pe -= es;
	do {
		char *tb = pb;

		pb -= es;
		swap(pb, tb);
		for ( ; (tb != pe) && is_lt(tb + es, tb); tb += es)
			swap(tb + es, tb);
	} while ((pb != pa) && is_lt(pb, pb - es));
} // insert_merge_in_place


// Inserts a single element, PA, into an array pointed at by PB
// Starts with the following assumption:
// - PB starts immediately after PA (ie. PB == PA + ES)
// - The first item of PB is already known to be less than PA
// - PE points at the location immediately after the end of the
//   array pointed at by PB
static inline void
insert_up(char *pa, char *pb, char *pe)
{
	// Find where we're stopping at. Then ripple the element up
	// This may look odd to break this operation into two
	// separate loops, but doing this keeps the loops tighter
	for ( ; ((pb + es) != pe) && is_lt(pb + es, pa); pb += es);
	for ( ; (pa != pb); pa += es) swap(pa, pa + es);
} // insert_up


// Inserts a single element, PB, into an array pointed at by PA
// Assumes the following:
// - PB is located immediately after the end of PA
// - *PB is already known to be less than the last item of PA
// - PA has at least a single item in it
static inline void
insert_down(char *pa, char *pb)
{
	char *ta = pb - es;

	for ( ; (ta != pa) && is_lt(pb, ta - es); ta -= es);
	for ( ; (ta != pb) ; ta += es) swap(ta, pb);
} // insert down

static void
ripple_merge_smallest(char *pa, char *pb, char *pe)
{
	if (is_lt(pe - es, pa))
		return _swab(pa, pb, pe);

	for (char *spa = pb, *spb = pb; pb != pe; pa = pb, pb = spb, spa = pb) {

		while ((spa != pa) && (spb != pe) && is_lt(spb, spa - es))
			spa -= es, spb += es;

		if (spa == pb)  return;

		for (char *ta = spa, *tb = pb; ta != pb; ta += es, tb += es)
			swap(ta, tb);

		if (spa != pa)
			if ((pb - pa) < (11 * es))
				insertion_merge_in_place(pa, spa, pb);
			else
				ripple_merge_smallest(pa, spa, pb);
	}
} // ripple_merge_smallest

#if 0
    int t, *B = &A[an];
    size_t  pa, pb;     // Swap partition pointers within A and B

    // Find the portion to swap.  We're looking for how much from the
    // start of B can swap with the end of A, such that every element
    // in A is less than or equal to any element in B.  This is quite
    // simple when both sub-arrays come at us pre-sorted
    for(pa = an, pb = 0; pa>0 && pb<bn && B[pb] < A[pa-1]; pa--, pb++);

    // Now swap last part of A with first part of B according to the
    // indicies we found
    for (size_t index=pa; index < an; index++)
	swap(A[index], B[index-pa]);

    // Now merge the two sub-array pairings.  We need to check that either array
    // didn't wholly swap out the other and cause the remaining portion to be zero
    if (pa>0 && (an-pa)>0)
	merge_inplace(A, pa, an-pa);

    if (pb>0 && (bn-pb)>0)
	merge_inplace(B, pb, bn-pb);

#endif

#if 0
// Merges two small arrays together
// Assumes the following:
// - PA->PB->PE is contiguous
// - The last item in *PA is already known to be less than
//   the first item in *PB
static void
merge_two_smalls(char *pa, char *pb, char *pe)
{
	if ((pa + es) == pb)
		return (void)insert_up(pa, pb, pe);
	if ((pb + es) == pe)
		return (void)insert_down(pa, pb);

	// Check which array is smaller
	if ((pb - pa) < (pe - pb)) {
		char *ta = pb;
		do {
			ta -= es;
			pe = insert_up(ta, pb, pe);
			pb -= es;
		} while ((ta != pa) && is_lt(ta, ta - es));
	} else {
		do {
			pa = insert_down(pa, pb) + es;
			pb += es;
		} while ((pb != pe) && is_lt(pb, pb - es));
	}
} // merge_two_smalls
#endif

#if 0
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

			_swab(pa, pb, pe);
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
#endif

#if 1

static void ripple_merge_in_place(char *pa, char *pb, char *pe);

#if 1

#if 0
// Assumes NA and NB are greater than zero
static void
ripple_merge_down(char *pa, char *pb, char *pe)
{
//	if (++depth > maxdepth) {
//		maxdepth = depth;
//		printf("Depth = %ld\n", depth);
//	}

	while(is_lt(pb, pb - es)) {
		if (pa == (pb - es)) {	// Check if just 1 item in A
			do {
				swap(pa, pb);
				pa = pb;  pb += es;
			} while ((pb != pe) && is_lt(pb, pa));
			break;
		}
		if ((pb + es) == pe) {	// Check if just 1 item in B
			do {
				swap(pb, pb - es);
				pb -= es;
			} while ((pb != pa) && is_lt(pb, pb - es));
			break;
		}

		size_t	bs = pe - pb;		// Byte-wise size of B
		char	*asp = pb - bs, *bsp;	// A/B Split Pointers

		// Ripple all of PB->PE down as far as we can
		for ( ; asp >= pa && is_lt(pe - es, asp); asp -= bs) {
			if (bs < SWAP_BLK_MIN) {
				char *ta = asp, *tb = pb;
				while (ta != pb) {
					swap(ta, tb);
					ta += es;
					tb += es;
				}
			} else {
				swap_blk(asp, pb, bs);
			}
			pe = pb; pb = asp;
		}

		// If we went all the way then we're done here
		if (pb == pa)
			break;

		// Handle the special case of when we reach the end (PA)
		// and if what's left is smaller than what we're moving.
		if (asp < pa) {
			if ((pe - pa) < (es * 12))
				insertion_merge_in_place(pa, pb, pe);
			else
				ripple_merge_in_place(pa, pb, pe);
			break;
		}

		// Find spot within A to split it at
		if (bs > (11 * es)) {
			// Binary search on adequately large A sets
			size_t	min = 0, max = (bs / es) - 1;
			size_t	sn = max >> 1;

			for ( ; min < max; sn = (min + max) >> 1) {
				asp = pb - (sn * es);
				bsp = pb + (sn * es);
				if (is_lt(bsp, asp - es))
					min = sn + 1;
				else
					max = sn;
			}

			asp = pb - (sn * es);
			bsp = pb + (sn * es);
		} else {
			// A linear scan on small sets is faster
			asp += es;
			bsp = pb + (pb - asp);
			for ( ; (asp != pb) && !is_lt(bsp - es, asp); asp+=es)
				bsp -= es;
		}

		if (!(pb - asp))	   // Nothing to swap.  We're done here
		       	break;

		// Do a single ripple at the split point
		if ((pb - asp) >= SWAP_BLK_MIN) {
			swap_blk(asp, pb, pb - asp);
		} else {
			for (char *ta=asp, *tb=pb; ta < pb; ta+=es, tb+=es)
				swap(ta, tb);
		}

		// PB->RP is one sorted array, and RP->PE is another.
		// PB is a hard lower limit on the search space for
		// this sub-merge, so it's used as the new PA
		if ((pe - pb) < (es << 3))
			insertion_merge_in_place(pb, bsp, pe);
		else
			ripple_merge_down(pb, bsp, pe);

		// PB->RP is the top part of A that was split
		// RP->PE is the rest of the array we're merging into
		pe = pb;
		pb = asp;
	}
//	depth--;
} // ripple_merge_down
#endif

// Assumes NA and NB are greater than zero
static void
ripple_merge_in_place(char *pa, char *pb, char *pe)
{
//	if (++depth > maxdepth) {
//		maxdepth = depth;
//		printf("Depth = %ld\n", depth);
//	}

	while(is_lt(pb, pb - es)) {
		if (pa == (pb - es)) {	// Check if just 1 item in A
			do {
				swap(pa, pb);
				pa = pb;  pb += es;
			} while ((pb != pe) && is_lt(pb, pa));
			break;
		}
		if ((pb + es) == pe) {	// Check if just 1 item in B
			do {
				swap(pb, pb - es);
				pb -= es;
			} while ((pb != pa) && is_lt(pb, pb - es));
			break;
		}

		size_t	bs = pb - pa;		// Byte-wise size of A
		char	*sp, *rp = pb + bs;	// Split/Ripple pointer

		// Ripple entirety of A->B up as far as we can
		if ((pb - pa) >= SWAP_BLK_MIN) {
			for ( ; (rp <= pe) && is_lt(rp - es, pa); rp += bs) {
				swap_blk(pa, pb, bs);
				pa = pb;  pb = rp;
			}
		} else {
			for ( ; (rp <= pe) && is_lt(rp - es, pa); rp += bs)
				for ( ; pb != rp; pa += es, pb += es)
					swap (pa, pb);
		}
			
		// If we went all the way then we're done here
		if (pb == pe) break;

		// Handle the special case of when we reach the end (PE)
		// and if what's left is smaller than what we're moving.
		// Swap the remainder (PB -> PE) with our block (A -> PB)
		// and make the remainder be the new block we're rippling
		// This avoids a potential recursion run-away scenario
#if 0
		if (rp > pe) {
			if ((pe - pa) < (es * 12))
				insertion_merge_in_place(pa, pb, pe);
			else
				ripple_merge_down(pa, pb, pe);
			break;
		}
#else
		if (rp > pe) {
			_swab(pa, pb, pe);
			pb = pa + (pe - pb);
			continue;
		}
#endif
		// Find spot within A to split it at
		if (bs > (11 * es)) {
			// Binary search on adequately large A sets
			size_t	min = 0, max = (bs / es) - 1;
			size_t	sn = max >> 1;

			for ( ; min < max; sn = (min + max) >> 1) {
				sp = pb - (sn * es);
				rp = pb + (sn * es);
				if (is_lt(rp, sp - es))
					min = sn + 1;
				else
					max = sn;
			}

			sp = pb - (sn * es);
			rp = pb + (sn * es);
		} else {
			// A linear scan on small sets is faster
			rp -= es;
			sp = pb - (rp - pb);
			for ( ; (sp != pb) && !is_lt(rp - es, sp); sp+=es)
				rp -= es;
		}

		if (!(pb - sp))	   // Nothing to swap.  We're done here
		       	break;

		// Do a single ripple at the split point
		if ((pb - sp) >= SWAP_BLK_MIN) {
			swap_blk(sp, pb, pb - sp);
		} else {
			for (char *ta=sp, *tb=pb; ta < pb; ta+=es, tb+=es)
				swap(ta, tb);
		}

		// PA->SP is one sorted array, and SP->PB is another.
		// PB is a hard upper limit on the search space for
		// this sub-merge, so it's used as the new PE
		if ((pb - pa) < (es << 3))
			insertion_merge_in_place(pa, sp, pb);
		else
			ripple_merge_in_place(pa, sp, pb);

		// PB->RP is the top part of A that was split
		// RP->PE is the rest of the array we're merging into
		pa = pb;
		pb = rp;
	}
//	depth--;
} // ripple_merge_in_place

#else

static void ripple_merge_in_place(char *pa, char *pb, char *pe);


static void
ripple_merge_down(char *pa, char *pb, char *pe)
{
	while (is_lt(pb, pb - es)) {
		if ((pb - es) == pa)	// Check if just 1 item in A
			return insert_up(pa, pb, pe);
		if ((pb + es) == pe)	// Check if just 1 item in B
			return insert_down(pa, pb);

		if ((pe - pb) >= (pb - pa))
			return ripple_merge_in_place(pa, pb, pe);

		size_t	bs = pe - pb;	// Get byte-wise size of B
		char	*rp = pb - bs;	// Ripple Pointer
		size_t	nb = bs / es;	// Number of items in B

		// Ripple all of PB->PE down as far as we can
		for ( ; rp >= pa && is_lt(pe - es, rp); rp -= bs) {
			if (bs < SWAP_BLK_MIN) {
				char *ta = rp, *tb = pb;
				while (ta != pb) {
					swap(ta, tb);
					ta += es;
					tb += es;
				}
			} else {
				swap_blk(rp, pb, bs);
			}
			pe = pb; pb = rp;
		}
			  
		// Check if we reached the end.  If so, we're done
		if (pb == pa) return;

		// We couldn't ripple the full B block any further. Split
		// the B block into two, and keep trying with remainder.
		// An imbalanced split here improves algorithmic performance
		// ns = ((n + 4) / 4) OR ns = ((n + 3) / 5) are also good
		// size_t  ns = ((na + 3) << 1) / 9;    // Basically na/4.5
		size_t  ns = (nb >> 2) + 1;
		char    *ps = pe - (ns * es);	     // Points at split

		ripple_merge_down(pa, pb, ps);

		// Restart the loop to merge in the bit we left behind
		pb = ps;
	}
} // ripple_merge_down

// Assumes NA and NB are greater than zero
// An alternative algorithm to above that eschews searching for the
// split point, and instead employs a simple mathematical formula
// Maximum recursion depth is ~2.4 * log2(N)
static void
ripple_merge_in_place(char *pa, char *pb, char *pe)
{
	while (is_lt(pb, pb - es)) {
		if ((pb - es) == pa)	// Check if just 1 item in A
			return insert_up(pa, pb, pe);
		if ((pb + es) == pe)	// Check if just 1 item in B
			return insert_down(pa, pb);

		if ((pe - pb) < (pb - pa))
			return ripple_merge_down(pa, pb, pe);

		size_t	bs = pb - pa;	// Get byte-wise size of A
		char	*rp = pb + bs;	// Ripple Pointer
		size_t	na = bs / es;	// Number of items in A

		// Ripple all of PA->PB up as far as we can
		for ( ; rp <= pe && is_lt(rp - es, pa); rp += bs) {
			if (bs < SWAP_BLK_MIN) {
				for ( ; pb != rp; pa += es, pb += es)
					swap (pa, pb);
			} else {
				swap_blk(pa, pb, bs);
				pa = pb;     pb = rp;
			}
		}
			  
		// Check if we reached the end.  If so, we're done
		if (pb == pe) return;

		// We couldn't ripple the full A block any further. Split
		// the A block into two, and keep trying with remainder.
		// An imbalanced split here improves algorithmic performance
		// ns = ((n + 4) / 4) OR ns = ((n + 3) / 5) are also good
		// size_t  ns = ((na + 3) << 1) / 9;    // Basically na/4.5
		size_t  ns = (na >> 2) + 1;
		char    *ps = pa + ns * es;	     // Points at split

		if ((pe - ps) < (es * 11))
			insertion_merge_in_place(ps, pb, pe);
		else
			ripple_merge_in_place(ps, pb, pe);

		// Restart the loop to merge in the bit we left behind
		pb = ps;
	}
} // ripple_merge_in_place

#endif
#endif

// Merges A + B into W
__attribute__((noinline))
static void
fim_merge_into_workspace(char *ws, char *pa, char *pb, char *pe)
{
	char	*ta = pa, *tb = pb;

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
	if ((we - ws) < SWAP_BLK_MIN) {
		for ( ; ws != we; ws += es, pa += es)
			swap(pa, ws);
	} else {
		swap_blk(ws, pa, we - ws);
	}
} // from_merge_from_workspace


// Merges A and B together using workspace W
// Start of A through to end of B must be contiguous
// Assumes both NA and NB are > zero on entry
__attribute__((noinline))
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
		swap_blk(a, w, b - a);
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
		return swap_blk(wp, a, we - wp);

	for ( ; wp != we; wp += es, a += es)
		swap(a, wp);
} // fim_merge_using_workspace


#if 0

// Merges A and B together using workspace W
// Assumes both NA and NB are > zero on entry
__attribute__((noinline))
static void
fim_merge_using_workspace(char *w, char *a, const size_t na, char *b, const size_t nb)
{
	// Skip initial part of A, but only if A and B are in sequence
	while ((a != b) && !is_lt(b, a))  a += es;

	if (a == b)	// Nothing to merge
		return;

	char	*e = b + (nb * es);
	char	*pw = w;

	// Now copy everything remaining from A to W
	if ((b - a) < SWAP_BLK_MIN) {
		for (char *ta = a; ta != b; pw += es, ta += es)
			swap(pw, ta);
	} else {
		// Use bulk swaps for greater speed when we can
		swap_blk(a, w, b - a);
		pw += (b - a);
	}

	// We already know that the first B is smaller
	swap(a, b);
	a += es;
	b += es;

	fim_merge_from_workspace(w, pw, a, b, e);
} // fim_merge_using_workspace

//#else

// Merges A and B together using workspace W
// Assumes both NA and NB are > zero on entry
__attribute__((noinline))
static void
fim_merge_using_workspace(char * const w, char * const a, size_t const na, char * const b, size_t const nb)
{
	char	*pe = b + (nb * es);
	char	*pa = a, *pb = b;
	char	*wh = w, *wt = w;		// Workspace Queue Head and Tail

	// Skip initial part of A, but only if A and B are in sequence
	while ((pa != b) && !is_lt(b, pa))  pa += es;

	if (pa == b)	// Nothing to merge
		return;

	// We already know that the first B is smaller
	// Push A onto the queue
	swap(pa, wt);
	wt += es;

	// Now swap B into A
	swap(pa, pb);
	pa += es;
	pb += es;

	// Now process the rest of A
	for ( ; (pa != b); pa += es) {
		swap(pa, wt);
		wt += es;
		if((pb != pe) && is_lt(pb, wh)) {
			swap(pb, pa);
			pb += es;
		} else {
			swap(wh, pa);
			wh += es;
		}
	}

	// Now merge the Workspace queue with B
	for ( ; (pb != pe) && (wh != wt); pa += es) {
		if(is_lt(pb, wh)) {
			swap(pa, pb);
			pb += es;
		} else {
			swap(pa, wh);
			wh += es;
		}
	}

	// Copy in any remainder in the workspace queue
	if ((wt - wh) < SWAP_BLK_MIN) {
		for ( ; wh != wt; wh += es, pa += es)
			swap(pa, wh);
	} else {
		swap_blk(wh, pa, wt - wh);
	}
} // fim_merge_using_workspace

#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"


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
	if (is_lt(pb, pb - es))
//		ripple_merge_smallest(pa, pb, pa + n * es);
		ripple_merge_in_place(pa, pb, pa + n * es);
} // fim_sort_main

static void
skewy_sort(char * const w, char * const a, const size_t n)
{
	if (n < 9)
		return fim_insert_sort(a, n);

	size_t	na = n / 4.66;
	na = (na == 0) ? 1 : na;
	size_t	nb = n - na;
	char	*pb = a + na * es;

	skewy_sort(w, a, na);

	skewy_sort(w, pb, nb);

	fim_merge_using_workspace(w, a, na, pb, nb);
}

static void
skewy_sort_top(char * const a, const size_t n)
{
	if (n < 15)
		return fim_insert_sort(a, n);

	size_t	na = (n * 2) / 7;
	size_t	nb = n - na;
	char	*pb = a + na * es;
	char	*pe = a + n * es;

	skewy_sort(a, pb, nb);

	skewy_sort_top(a, na);

	ripple_merge_in_place(a, pb, pe);
	//ripple_merge_smallest(a, pb, pe);
}

static void
simplest(char *pa, const size_t n)
{
	// Handle small array size inputs with insertion sort
	if (n < 15)
		return fim_insert_sort(pa, n);

	size_t	na = (n >> 2) + 1;		// 1/4 looks to be better
	char	*pb = pa + (na * es);
	size_t	nb = n - na;

	if (na > 1)
		simplest(pa, na);
	if (nb > 1)
		simplest(pb, nb);

	char	*pe = pa + (n * es);

	//insertion_merge_in_place(pa, pb, pe);
//	rotate_merge_in_place(pa, pb, pe);
	//merge_inplace(pa, na, pb, pe);
	ripple_merge_in_place(pa, pb, pe);
	//ripple_merge_smallest(pa, pb, pe);
} // simplest

#pragma GCC diagnostic pop


void
fim_sort(char *a, const size_t n, const size_t _es, const int (*_is_lt)(const void *, const void *))
{
	int	skew = SKEW;

	es = _es;
	is_lt = _is_lt;

	assert((skew >= 0) && (skew <= 10));
	SWAPINIT(a, es);

	fim_sort_main(a, n);
//	skewy_sort_top(a, n);
//	simplest(a, n);

//	for (int i = 0; i < n; i++) {
//		assert(i == *(((int *)a) + i));
//	}
//	printf("n1 = %ld, n2 = %ld, nn = %ld\n", n1, n2, nn);
//	printf("n1 = %ld, n2 = %ld\n", n1, n2);
//	printf("nn = %ld\n", nn);
//	print_array(a, n);
} // fim_sort
