#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "swap.h"

#define bulkswap(_pa, _pb, _nb)				\
do {							\
	char	_t[4096];				\
	char	*_a = _pa;				\
	char	*_b = _pb;				\
	size_t	_n = _nb;				\
							\
	while (_n > 0) {				\
		size_t _nc = _n > 4096 ? 4096 : _n;	\
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


static void
sanity(char *a, char *e, size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	for(char *s = a + es; s < e; s += es)
		if (is_lt(s, s - es))
			exit(1);
} // sanity

// Finds the position in B to insert A at
static inline char *
find_pos(char *a, char *b, const size_t nb, size_t es, const int (*is_lt)(const void *, const void *))
{
	char *pe = b + nb * es;
	char *pb;

	if (nb < 21) {
		// For smaller sets, a tight linear search is faster
		if (a > b) {
			// Linear scan backwards
			for (pb = pe, pe = pe - es; pb > b && !is_lt(pe, a); pb = pe, pe -= es);
		} else {
			// Linear scan forwards
			for (pb = b; pb < pe && is_lt(pb, a); pb += es);
		}
	} else {
		// Do binary search if sets are large enough
		size_t min = 0, max = nb;
		size_t pos = (min + max ) / 2;

		while (min < max) {
			pb = b + (pos * es);

			if (is_lt(pb, a))
				min = pos + 1;
			else
				max = pos;

			pos = (min + max) / 2;
		}

		pb = b + pos * es;
	}
	return pb;
} // find_pos


// Swaps two blocks in place efficiently
static inline void
_swab(char *a, size_t na, char *b, size_t nb, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	char	*e = b + nb * es;
	WORD	t;				// Temporary variable for swapping

	while (na > 0 && nb > 0) {
		if (na == 1 && nb == 1) {
			swap(a, b);
			break;
		}

		if (na > nb) {
			for (char *from = b, *to = a; from < e; from += es, to += es)
				swap(from, to);

			a += (nb * es);
			na -= nb;
		} else {
			for (char *from = a, *to = a + nb * es; to < e; from += es, to += es)
				swap(from, to);

			nb -= na;
			e = b + nb * es;
		}
	}
} // _swab


static inline char *
merge_to_right(char *pa, char *pb, char *pe, size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	for (WORD t; pb < pe && is_lt(pb, pa); pa = pb, pb += es)
		swap(pa, pb);
	return pa;
} // merge_to_right


static inline char *
merge_to_left(char *pa, char *la, char *pb, size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	for (WORD t; pb > pa && is_lt(pb, la); pb = la, la -= es)
		swap(pb, la);
	return pb;
} // merge_to_right


// Merges two sorted sub-arrays , A and B, in place
// Assumes that arrays of zero length will not be passed in
// Assumes that A and B are byte-wise contiguous as a whole
// Assumes that A is before B in the address space
static void
_fim_sub(char *a, const size_t na, char *b, const size_t nb, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	char	*e = b + (nb * es);		// E points at immediately AFTER the last elemnent in B
	char	*la = b - es;			// LA points at the last element of A
	char	*lb = e - es;			// LE points at the last element of B
	char	*pa, *pb;			// PA and PB are floating pointers into A and B
	WORD	t;				// Temporary variable for swapping

	if (na == 0 || nb == 0)
		return;

	if (!is_lt(b, la))			// Already sorted
		return;

	if (na == 1 && nb == 1) {
		swap(a, b);
		return;
	}

	if (na == 1) {
		swap(a, b);
		merge_to_right(a + es, b + es, e, es, is_lt, swaptype);
		return;
	}

	if (nb == 1) {
		swap(b, la);
		merge_to_left(a, la - es, b - es, es, is_lt, swaptype);
		return;
	}

#if 0

	// Split the larger of the two arrays into two
	
	if (nb < na) {
		// Find mid-point of B
		pb = b + (nb >> 1) * es;

		// Now we need to find the point in A that PB is smaller than
		pa = find_pos(pb, a, na, es, is_lt);
	} else {
		// Find mid-point of A
		pa = a + (na >> 1) * es;

		// Now we need to find the point in B that PA is smaller than
		pb = find_pos(pa, b, nb, es, is_lt);
	}

	// Now swap the blocks AS->B, and B->BS
	size_t	asn = (b - pa) / es;
	size_t	bsn = (pb - b) / es;

//	printf("NA = %ld, ASN = %ld, BSN = %ld, NB = %ld\n", na - asn, asn, bsn, nb - bsn);

	_swab(pa, asn, b, bsn, es, is_lt, swaptype);

	// Now we have 4 sub-arrays.
	// All of A through to end of PA is less than end of PA to E
	// Merge A to end of PA, and end of PA to E

//	if (bsn)
		_fim_sub(a, na - asn, pa, bsn, es, is_lt, swaptype);

//	if (asn)
		_fim_sub(pa + bsn * es, asn, pb, nb - bsn, es, is_lt, swaptype);

#endif

#if 0
	if (is_lt(lb, a)) {			// All of B is less than all of A
		_swab(a, na, b, nb, es, is_lt, swaptype);
		return;
	}
	
	if (nb < 4 && is_lt(lb, la)) {
		// Special case - all of B moves to within A
		// Find where the last element of B needs to move to within A
		pa = find_pos(lb, a, na - 1, es, is_lt);

		// Now move all of B to PA.  Handle overlapping
		// Afterwards:
		// - From the end of PB to E is sorted and in place
		// - From PB to end of PB is sorted, but not in place
		// - From A to PB is sorted, but not in place

		_swab(pa, (b - pa) / es, b, nb, es, is_lt, swaptype);

		// Merge the two arrays that are not in place

		if (pa > a)
			_fim_sub(a, (pa - a) / es, pa, nb - 1, es, is_lt, swaptype);

		return;
	}

	if (na < 4 && is_lt(b, a)) {
		// Special case - all of A moves to within B
		// Find where the first element of A needs to move to within B
		// We already know the first element of B is less than A
		pb = find_pos(a, b + es, nb - 1, es, is_lt);

		// Now move all of A to before PB.  Handle overlapping
		// Afterwards:
		// From A to PA is sorted and in place
		// From PA to the end of PA is sorted, but not in place
		// From end of PA to E is sorted, but not in place
		_swab(a, na, b, (pb - b) / es, es, is_lt, swaptype);

		char *fa = pb - (na * es);

		// Merge the two arrays that are not in place
		if (pb < e)
			_fim_sub(fa + es, na - 1, pb, (e - pb) / es, es, is_lt, swaptype);

		return;
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
		ns /= es;
	} else {
		ns /= es;
		for (char *ta = pa, *tb = b; ta != b; ta += es, tb += es)
			swap(ta, tb);
	}

	// Now merge-in-place the four sub-arrays as required
	// Ensure that we don't pass in 0 length sub-arrays
	if (pa != a)
		_fim_sub(a, na - ns, pa, ns, es, is_lt, swaptype);

	if (pb != e)
		_fim_sub(b, ns, pb, nb - ns, es, is_lt, swaptype);
} // _fim_sub


// Assumes NA and NB are greater than zero
static void
_ripple_merge(char *a, const size_t na, char *b, const size_t nb, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	char	*e = b + (nb * es);		// E points at immediately AFTER the last elemnent in B
	char	*pa = a, *pb = b;		// PA and PB are just floating pointers into A and B
	WORD	t;				// Temporary variable for swapping

	if (!is_lt(b, b - es))			// Already sorted
		return;

	if (na == 1) {
		// Just insert merge single items
		// We already know *PB < *PA
		do {
			swap(pa, pb);
			pa = pb;
			pb += es;
		} while (pb < e && is_lt(pb, pa));
	} else {
		size_t	bs = b - a;		// Determine our block size
		char	*te = pb + bs;

		// Try to ripple all of A up as far as we can
		if (bs < 256) {
			for ( ; te < e && is_lt(te - es, pa); te += bs)
				for ( ; pb < te; pa += es, pb += es)
					swap (pa, pb);
		} else {
			for ( ; te < e && is_lt(te - es, pa); te += bs) {
				bulkswap(pa, pb, bs);
				pa = pb;
				pb = te;
			}
		}

		// If we get here, we couldn't roll the full A block any further
		// Split the A block into half, and keep trying with remainder
		size_t	n1 = na >> 1;
		char	*p2 = pa + n1 * es;

		_ripple_merge(p2, na - n1, pb, (e - pb) / es, es, is_lt, swaptype);

		_ripple_merge(pa, n1, p2, (e - p2) / es, es, is_lt, swaptype);
	}
} // _ripple_merge


// Merges A and B together using workspace W
static void
_fim_merge(char *w, char *a, char *b, char *e, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	size_t	na = (b - a) / es;
	char	*pw = w;
	WORD	t;

	// Skip initial part of A as required
	while (a < b && !is_lt(b, a))
		a += es;

	// Now copy everything remaining from A to W
	// Use bulk swaps for greater speed when we can
	size_t bs = b - a;

	if (bs < 256) {
		for (char *ta = a; ta < b; pw += es, ta += es)
			swap(pw, ta);
	} else {
		bulkswap(a, w, bs);
		pw += bs;
	}

	// Now merge W and B into B
	for ( ; w < pw && b < e; a += es)
		if(is_lt(b, w)) {
			swap(b, a);
			b += es;
		} else {
			swap(w, a);
			w += es;
		}

	// Copy over any remainder
	for ( ; w < pw; w += es, a += es)
		swap(a, w);
} // _fim_merge


static void
_fim_sort_merge(char *w, char *pa, const size_t n, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	char	*pe = pa + n * es;	// Points at first byte after the end of pa
	WORD	t;

	// Handle small array size inputs efficiently
	if (n < 13) {
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

	// Split A into two parts
	size_t	na = n / 2;
	size_t	nb = n - na;
	char	*pb = pa + na * es;

	_fim_sort_merge(w, pa, na, es, is_lt, swaptype);
	_fim_sort_merge(w, pb, nb, es, is_lt, swaptype);
	_fim_merge(w, pa, pb, pe, es, is_lt, swaptype);
} // _fim_sort_merge


static void
_fim_sort(char *pa, const size_t n, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	char	*pe = pa + n * es;	// Points at first byte after the end of pa
	WORD	t;

	// Handle small array size inputs with insertion sort
	if (n < 21) {
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

	// Split into 3 parts
#if 0
	size_t	na;

	for (na = 1; na < n; na += na);
	na >>= 2;
	size_t	nb = na;
	size_t	nc = n - (nb + na);
#endif

	size_t	nb = n / 4;
	size_t	nc = nb + nb;
	size_t	na = n - (nb + nc);

	char	*pb = pa + na * es;
	char	*pc = pb + nb * es;

	// Sort B and C and merge together using A for workspace
	
	_fim_sort_merge(pa, pb, nb, es, is_lt, swaptype);
	_fim_sort_merge(pa, pc, nc, es, is_lt, swaptype);
	_fim_merge(pa, pb, pc, pe, es, is_lt, swaptype);

	// Now sort A
	_fim_sort(pa, na, es, is_lt, swaptype);

	// Now Merge In Place A, and BC
//	_fim_sub(pa, na, pb, nb + nc, es, is_lt, swaptype);
	_ripple_merge(pa, na, pb, nb + nc, es, is_lt, swaptype);
#if 0
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
		_fim_sort(pb, nb, es, is_lt, swaptype);
	} else {
		// Sort A
		_fim_sort(pa, na, es, is_lt, swaptype);
	}

	// Now merge A and B together.
	_fim_sub(pa, na, pb, nb, es, is_lt, swaptype);

	// Now merge AB and C together
	_fim_sub(pa, na + nb, pc, nc, es, is_lt, swaptype);
#endif
} // _fim_sort


void
fim_sort(char *a, const size_t n, const size_t es, const int (*is_lt)(const void *, const void *))
{
	char	*e = a + n * es;
	int	swaptype;

	SWAPINIT(a, es);

	_fim_sort(a, n, es, is_lt, swaptype);
//	dumpa(a, e, es);
} // fo_sort
