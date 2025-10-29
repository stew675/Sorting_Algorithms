#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "swap.h"

#define SPLIT_DIVISOR	4

#define bulkswap(_pa, _pb, _nb)				\
do {							\
	char	_t[1024];				\
	char	*_a = _pa;				\
	char	*_b = _pb;				\
	size_t	_n = _nb;				\
							\
	while (_n > 0) {				\
		size_t _nc = _n > 1024 ? 1024 : _n;	\
							\
		memcpy(_t, _a, _nc);			\
		memcpy(_a, _b, _nc);			\
		memcpy(_b, _t, _nc);			\
							\
		_a += _nc;				\
		_b += _nc;				\
		_n -= _nc;				\
	}						\
} while(0);


static void
dumpa(char *a, const char *e, const size_t es)
{
	size_t n = 20;
	while (a < e) {
		if (n == 0) {
			printf("\n");
			n = 20;
		}
		n--;
		printf("%4d ", *((int *)a));
		a += es;
	}
	printf("\n\n");
} // dumpa


static inline void
__is(char *a, const char *e, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype, size_t hop)
{
	size_t	gap = hop * es;
	WORD	t;

	for (char *s = a, *b = a + gap; b < e; s = b, b += gap)
		for (char *c = b; c > a && is_lt(c, s); c = s, s -= gap)
			swap(c, s);
} // __is

static void
do_is(char *a, const size_t n, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype, size_t d)
{
	if ((d == n) || (d < (n / 5))) {
		char	*e = a + (n * es);
		size_t	pos, hop = n / d;

		for (pos = 0; pos < hop; pos++)
			__is(a + (pos * es), e, es, is_lt, swaptype, hop);
	}
}


// Quickly nudges values closer to where they need to be
// Takes 1.25n compares
// Averages approx 0.9n swaps on purely random inputs
static void
nudge(char *a, const size_t n, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	WORD	t;				// Temporary variable for swapping

	if (n < 4) {
		if (n < 2)
			return;

		char *p1 = a, *p2 = a + es;

		// If n == 2, just do a compare, swap if needed, and return
		if (n == 2) {
			if (is_lt(p2, p1))
				swap(p2, p1);
			return;
		}

		// n == 3 here
		char *p3 = p2 + es;

		if (is_lt(p2, p1))
			swap(p2, p1);
		if (is_lt(p3, p2)) {
			swap(p3, p2);
			if (is_lt(p2, p1))
				swap(p2, p1);
		}
		return;
	}

	char	*e = a + (n * es);
	size_t	step = n / 4;
	char	*p1 = a;
	char	*p2 = a + step * es;
	char	*p3 = p2 + step * es;
	char	*p4 = p3 + step * es;

	while (p4 < e) {
		if (is_lt(p2, p1))
			swap(p2, p1);
		if (is_lt(p4, p3))
			swap(p4, p3);
		if (is_lt(p3, p1))
			swap(p3, p1);
		if (is_lt(p4, p2))
			swap(p4, p2);
		if (is_lt(p3, p2))
			swap(p3, p2);

		p1 += es;
		p2 += es;
		p3 += es;
		p4 += es;
	}
} // nudge

static void
sanity(char *a, char *e, size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	for(char *s = a + es; s < e; s += es)
		if (is_lt(s, s - es))
			exit(1);
} // sanity

// Merges two sorted sub-arrays , A and B, in place
// Assumes that arrays of zero length will not be passed in
// Assumes that A and B are byte-wise contiguous as a whole
// Assumes that A is before B in the address space
static void
__mip_sub2(char *a, const size_t na, char *b, const size_t nb, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	char	*e = b + (nb * es);		// E points at immediately AFTER the last elemnent in B
	char	*la = b - es;			// LA points at the last element of A
//	char	*lb = e - es;			// LE points at the last element of B
	char	*pa, *pb;			// PA and PB are floating pointers into A and B
	WORD	t;				// Temporary variable for swapping

	if (!is_lt(b, la))			// Already sorted
		goto __mip_sub2_done;
	
	if (na == 1) {
		swap(a, b);
		if (nb > 1) {
			for (pa = b + es; pa < e && is_lt(pa, pa - es); pa += es)
				swap(pa, pa - es);
		}
		goto __mip_sub2_done;
	}

	if (nb == 1) {
		swap(b, la);
		for (pb = la; pb > a && is_lt(pb, pb - es); pb -= es)
			swap(pb, pb - es);
		goto __mip_sub2_done;
	}

#if 0
	if (nb < 4 && nb < na && is_lt(lb, la)) {

		// Special case - all of B moves to within A

		// Find where the last element of B needs to move to within A
		for (pa = la - es; pa >= a && is_lt(lb, pa); pa -= es);
		pa += es;

		// Now move all of B before PA.  Handle overlapping
		// Afterwards:
		// - From the end of PB to E is sorted and in place
		// - From PB to end of PB is sorted, but not in place
		// - From A to PB is sorted, but not in place

		size_t tnb = nb;
		do {
			size_t c = tnb;

			do {
				--c;
				swap(la, lb);
				la -= es;
				lb -= es;
			} while (c && la >= pa);

			if (c) {
				tnb = c;
				la = lb - (tnb * es);
			}
		} while (la >= pa);

		// Merge the two arrays that are not in place

		if (pa > a)
			__mip_sub2(a, (pa - a) / es, pa, nb - 1, es, is_lt, swaptype);

		goto __mip_sub2_done;
	}

	if (na < 4 && na < nb && is_lt(b, a)) {
		size_t	tna = na;
		char	*fa = a, *fb = b;

		// Special case - all of A moves to within B

		// Find where the first element of A needs to move to within B
		if (nb > 31) {
			size_t min = 1;
			size_t max = nb;
			size_t pos = (nb + 1 ) / 2;
			while (min < max) {
				char *tb = b + (pos * es);

				if (is_lt(tb, a))
					min = pos + 1;
				else
					max = pos;
				pos = (min + max) / 2;
			}
			pb = b + pos * es;
			pb -= es;
		} else {
			for (pb = b + es; pb < e && is_lt(pb, a); pb += es);
			pb -= es;
		}

		// Now move all of A after PB.  Handle overlapping
		// Afterwards:
		// From A to PA is sorted and in place
		// From PA to the end of PA is sorted, but not in place
		// From end of PA to E is sorted, but not in place
		do {
			size_t	c = tna;

			do {
				--c;
				swap(fa, fb);
				fa += es;
				fb += es;
			} while(c && fb <= pb);

			if (c) {
				tna = c;
				fb = fa + (tna * es);
			}
		} while (fb <= pb);

		la = pb + es;
		fa = la - (na * es);

		// Merge the two arrays that are not in place
		if (la < e)
			__mip_sub2(fa + es, na - 1, la, (e - la) / es, es, is_lt, swaptype);

		goto __mip_sub2_done;
	}
#endif

	// Find the pivot points in A and B, such that when swapped at that point,
	// all elements in A will be less than, or equal to, any element in B.
	if (na > 31 && nb > 31) {
		// Do binary search if sets are large enough
		size_t min = 1;
		size_t max = (na > nb) ? nb : na;
		size_t pos = (min + max ) / 2;

		while (min < max) {
			char *ta = b - (pos * es);
			char *tb = b + (pos * es);

			if (is_lt(tb, ta - es))
				min = pos + 1;
			else
				max = pos;
			pos = (min + max) / 2;
		}
		pa = b - pos * es;
		pb = b + pos * es;
	} else {
		pa = la - es;
		pb = b + es;

		// For smaller sets, a tight linear search is faster
		if (na < nb)
			for ( ; pa >= a && is_lt(pb, pa); pa -= es, pb += es);
		else
			for ( ; pb != e && is_lt(pb, pa); pa -= es, pb += es);
		pa += es;
	}

	// Now swap the A and B sub-arrays at the pivot points
	size_t ns = (b - pa);

	if (ns > 255) {
		bulkswap(pa, b, ns);
	} else {
		for (char *ta = pa, *tb = b; ta != b; ta += es, tb += es)
			swap(ta, tb);
	}

	ns /= es;

	// Now merge-in-place the four sub-arrays as required
	// Ensure that we don't pass in 0 length sub-arrays
	if (pa != a)
		__mip_sub2(a, na - ns, pa, ns, es, is_lt, swaptype);

	if (pb != e)
		__mip_sub2(b, ns, pb, nb - ns, es, is_lt, swaptype);

__mip_sub2_done:
} // __mip_sub2


// Merges two sorted sub-arrays in place
// Assumes that arrays of zero length will not be passed in
// Assumes that A and B are byte-wise contiguous as a whole
// Assumes that A is before B in the address space
static void
__mip_sub(char *a, const size_t na, char *b, const size_t nb, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	char	*e = b + (nb * es);		// E points at immediately AFTER the last elemnent in B
	char	*pb = b;
	char	*pa = b - es;
	WORD	t;				// Temporary variable for swapping
	
	if (!is_lt(pb, pa))			// Already sorted
		return;

	if (na == 1) {
		if (nb == 1) {
			swap(pb, pa);
			return;
		}
		do {
			swap(pb, pa);
			pa = pb;
			pb += es;
		} while (pb != e && is_lt(pb, pa));
		return;
	}

	if (nb == 1) {
		do {
			swap(pb, pa);
			pb = pa;
			pa -= es;
		} while (pb != a && is_lt(pb, pa));
		return;
	}
	       
	// Find the pivot points in A and B, such that when swapped at that point,
	// all elements in A will be less than, or equal to, any element in B.
	if (na > 31 && nb > 31) {
		// Do binary search if sets are large enough
		size_t min = 1;
		size_t max = (na > nb) ? nb : na;
		size_t pos = (min + max ) / 2;
		while (min < max) {
			char *ta = b - (pos * es);
			char *tb = b + (pos * es);

			if (is_lt(tb, ta - es))
				min = pos + 1;
			else
				max = pos;
			pos = (min + max) / 2;
		}
		pa = b - pos * es;
		pb = b + pos * es;
	} else {
		// Otherwise a tight linear search is faster for small sets
		for (pa -= es, pb += es; pa >= a && pb < e && is_lt(pb, pa); pa -= es, pb += es);
		pa += es;
	}

	size_t ns = b - pa;

	// Now swap the A and B sub-arrays at the pivot points
	if (ns > 255) {
		bulkswap(pa, b, ns);
	} else {
		for (char *ta = pa, *tb = b; ta < b; ta += es, tb += es)
			swap(ta, tb);
	}

	ns /= es;

	// Now merge-in-place the two sub-arrays as required
	// Ensure that we don't pass in 0 length sub-arrays
	if (pa != a)
		__mip_sub(a, na - ns, pa, ns, es, is_lt, swaptype);

	if (pb != e)
		__mip_sub(b, ns, pb, nb - ns, es, is_lt, swaptype);
} // __mip_sub


static void
__mip_sort3(char *pa, const size_t n, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	char	*pe = pa + n * es;	// Points at first byte after the end of pa
	WORD	t;

	// Handle small array size inputs efficiently
	if (n < 25) {
		// If 0 or 1 items, then nothing to sort, just return
		if (n < 2)
			return;

		// If n == 2, just do a compare, swap if needed, and return
		if (n == 2) {
			char *pb = pa + es;

			if (is_lt(pb, pa))
				swap(pa, pb);
			return;
		}

		// Do an insertion sort of tiny arrays
		for (char *ta = pa, *tb = pa + es; tb < pe; ta = tb, tb += es)
			for (char *tc = tb; tc > pa && is_lt(tc, ta); tc = ta, ta -= es)
				swap(tc, ta);
		return;
	}

	// Split into 3 parts. NA and NB must be equal, with NC equal to NA or larger

	size_t	na = n / 5;		// 5 appears to be near optimal but uses more compares and swaps
	size_t	nb = na;
	size_t	nc = n - (na + nb);

	char	*pb = pa + na * es;
	char	*pc = pb + nb * es;

	// Sort B and C
	
	__mip_sort3(pb, nb, es, is_lt, swaptype);
	__mip_sort3(pc, nc, es, is_lt, swaptype);

	// Merge B and C together if required. Afterwards A and C
	// will be sorted with B being unsorted in the middle
	if (is_lt(pc, pc - es)) {
		char	*ta = pa, *tb = pb, *tc = pc;

		// Merge to the end of A.  A will always fill before C
		// so there is no need to check for C boundary here
		for (ta = pa; ta < pb; ta += es)
			if (is_lt(tc, tb)) {
				swap(tc, ta);
				tc += es;
			} else {
				swap(tb, ta);
				tb += es;
			}
		
		// A is full.  Advance TA to C then continue
		// Make sure to check for C boundary now
		for (ta = pc; tb < pc; ta += es)
			if (tc < pe && is_lt(tc, tb)) {
				swap(tc, ta);
				tc += es;
			} else {
				swap(tb, ta);
				tb += es;
			}

		// Sort B
		__mip_sort3(pb, nb, es, is_lt, swaptype);
	} else {
		// Sort A
		__mip_sort3(pa, na, es, is_lt, swaptype);
	}

	// Now merge A and B together.
	__mip_sub2(pa, na, pb, nb, es, is_lt, swaptype);

	// Now merge AB and C together
	__mip_sub2(pa, na + nb, pc, nc, es, is_lt, swaptype);
} // __mip_sort3


static void
__mip_sort(char *pa, const size_t n, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	char	*pe = pa + n * es;	// Points at first byte after the end of pa
	WORD	t;

	// Handle small array size inputs efficiently
	if (n < 12) {
		// If 0 or 1 items, then nothing to sort, just return
		if (n < 2)
			return;

		// If n == 2, just do a compare, swap if needed, and return
		if (n == 2) {
			char *pb = pa + es;

			if (is_lt(pb, pa))
				swap(pa, pb);
			return;
		}

		// Do an insertion sort of tiny arrays
		for (char *ta = pa, *tb = pa + es; tb < pe; ta = tb, tb += es)
			for (char *tc = tb; tc > pa && is_lt(tc, ta); tc = ta, ta -= es)
				swap(tc, ta);
		return;
	}

	// Split into 3 equal parts

	size_t	na = n / 4;
	size_t	nb = na;
	size_t	nc = n - (na + nb);

	char	*pb = pa + na * es;
	char	*pc = pb + nb * es;

	// Sort B and C
	
	__mip_sort(pb, nb, es, is_lt, swaptype);
	__mip_sort(pc, nc, es, is_lt, swaptype);

	// Merge B and C together.  We get fully sorted B->E back
	if (is_lt(pc, pc - es)) {
		char	*ta = pa, *tb = pb, *tc = pc;

		// Determine how much of B to skip
		while (!is_lt(tc, tb)) 
			tb += es;

		size_t	skb = tb - pb;		// Number of skipped bytes

		for (ta = pa + skb; tb < pc; ta += es, tb += es)
			swap(ta, tb);

		for (ta = pa + skb, tb = pb + skb; ta < pb; tb += es)
			if (tc < pe && is_lt(tc, ta)) {
				swap(tc, tb);
				tc += es;
			} else {
				swap(ta, tb);
				ta += es;
			}
	}

	// Sort A
	__mip_sort(pa, na, es, is_lt, swaptype);

	// Merge A and B together.  We get fully sorted A->C back
	__mip_sub2(pa, na, pb, nb + nc, es, is_lt, swaptype);
} // __mip_sort

#if 0
static void
__mip_sort2(char *a, const size_t n, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	char	*e = a + n * es;
	WORD	t;

	// Handle small array size inputs efficiently
	if (n < 21) {
		// If 0 or 1 items, then nothing to sort, just return
		if (n < 2)
			return;

		// If n == 2, just do a compare, swap if needed, and return
		if (n == 2) {
			char *b = a + es;

			if (is_lt(b, a))
				swap(a, b);
			return;
		}

		// Do an insertion sort of tiny arrays
		for (char *s = a, *b = a + es; b < e; s = b, b += es)
			for (char *c = b; c > a && is_lt(c, s); c = s, s -= es)
				swap(c, s);

		return;
	}


	// Split into 3 equal parts

	size_t	na = n / 3;
	char	*b = a + (na * es);
	char	*c = a + ((na << 1) * es);
	size_t	nc = n - (na << 1);

	__mip_sort2(a, na, es, is_lt, swaptype);
	__mip_sort2(b, na, es, is_lt, swaptype);

	// Transfer all of A to working space C (if required)
	if (is_lt(b, b - es)) {
		char	*ta = a, *tb;
		char	*tc = c, *td;

		// Skip anything in A that we don't need to move
		while (is_lt(ta, b))
			ta += es;

		// Swap the remainder of A with C
		for (tb = ta; tb < b; tb += es, tc += es)
			swap(tb, tc);

		// Merge B and C into the remainder of A
		for (td = tc, tc = c; tc < td; ta += es)
			if (tb < c && is_lt(tb, tc)) {
				swap(tb, ta);
				tb += es;
			} else {
				swap(tc, ta);
				tc += es;
			}
	}

	// Now sort the range C->E
	__mip_sort2(c, nc, es, is_lt, swaptype);

	// Now merge A->C, and C->E together
	__mip_sub(a, (na << 1), c, nc, es, is_lt, swaptype);
} // __mip_sort2
#endif


void
fo_sort(char *a, const size_t n, const size_t es, const int (*is_lt)(const void *, const void *))
{
	char	*s, *e = a + (n * es);
	size_t	pos;
	int	swaptype;

	SWAPINIT(a, es);

	// Do a quick pass to nudge values roughly where they need to be
//	nudge(a, n, es, is_lt, swaptype);
//	do_is(a, n, es, is_lt, swaptype, 8);
	__mip_sort3(a, n, es, is_lt, swaptype);
//	__is(a, e, es, is_lt, swaptype, 1);
//	dumpa(a, e, es);
//	printf("num_rolls = %ld\n", num_rolls);
} // fo_sort
