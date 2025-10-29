#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "swap.h"

extern uint64_t numswaps, numcmps, numcopies;
static size_t num_rolls = 0;

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
		if (is_lt(p4, p1))
			swap(p4, p1);
		if (is_lt(p3, p2))
			swap(p3, p2);
		if (is_lt(p2, p1))
			swap(p2, p1);
		if (is_lt(p4, p3))
			swap(p4, p3);
		if (is_lt(p3, p2))
			swap(p3, p2);

		p1 += es;
		p2 += es;
		p3 += es;
		p4 += es;
	}
}

static void __new_sort(char *a, const size_t n, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype);

// Algorithm below relies on SPLIT_MIN never being less than 4
#define SPLIT_MIN	4

// Merges two sorted sub-arrays in place
// Assumes that arrays of zero length will not be passed in
// Assumes that A and B are byte-wise continguous as a whole
// Assumes that A is before B in the address space
// Assumes that number of items in B >= the number of items in A
static void
__new_sub(char *a, size_t na, char *b, size_t nb, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	size_t	n = na + nb;
	char	*e = a + n * es;		// E points at immediately AFTER the last elemnent in B
	char	*pa = b - es;
	char	*pb = b;
	WORD	t;				// Temporary variable for swapping
	
	if ((na == 0) || (nb == 0))
		return;

	if (!is_lt(b, pa))			// Already sorted
		return;
	
#if 0
	if (n < 8) {
		// Do an insertion sort of tiny arrays
		char *e = a + n * es;
		for (char *s = a, *b = a + es; b < e; s = b, b += es)
			for (char *c = b; c > a && is_lt(c, s); c = s, s -= es)
				swap(c, s);
		return;
	}
#endif

	// Find the pivot points in A and B, such that when swapped at that point,
	// all elements in A will be less than, or equal to, any element in B.
	if (na > 15 && nb > 15) {
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
		for (pa -= es, pb += es; pa >= a && pb < e && is_lt(pb, pa); pa -= es, pb += es);
		pa += es;
	}

	// We now have 4 sub-sets
	// 1. A  -> PA
	// 2. PA -> B
	// 3. B  -> PB
	// 4. PB -> E
	// Now merge PA->B into B->E.
	// When done PA->B will be loosely unsorted with both A->PA, and B->E fully sorted
	
	for (char *ta = pa, *tb = pb, *tc = b; ta < b; tc += es) {
		if (tb < e && is_lt(tb, ta)) {
			swap(tb, tc);
			tb += es;
		} else {
			swap(ta, tc);
			for (char *ts = ta; ts > pa && is_lt(ts, ts - es); ts -= es)
				swap(ts, ts - es);
			ta += es;
		}
	}

//	dumpa(pa, b, es);

	// Now sort the PA -> B subset
//	for (char *ta = pa, *tb = pa + es; tb < b; ta = tb, tb += es)
//		for (char *tc = tb; tc > pa && is_lt(tc, ta); tc = ta, ta -= es)
//			swap(tc, ta);

	// We now have two unmerged sorted subsets, A->PA, and PA->B.  Let's merge them now
	__new_sub(a, (pa - a) / es, pa, (b - pa) / es, es, is_lt, swaptype);
} // __new_sub


// Requires A and B to be of equal size, and C to be of non-zero size
static void
__new_merge(char *a, size_t na, char *b, size_t nb, char *c, size_t nc, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	char	*ec = c + nc * es;
	char	*ea = a;
	WORD	t;

	// na MUST be greater than or equal to nb 
	assert(na >= nb);

	// Now swap all of B into A.  Afterwards, ea points after the end of a
	for (char *tb = b; tb < c; ea += es, tb += es)
		swap(tb, ea);

	// Now merge A into B+C, with A picking up the unsorted remainder
	for (char *ta = a, *tb = b, *tc = c; ta < ea; tb += es)
		if (tc < ec && is_lt(tc, ta)) {
			swap(tc, tb);
			tc += es;
		} else {
			swap(ta, tb);
			ta += es;
		}
} // __new_merge


static void
__new_sort(char *a, const size_t n, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	char	*e = a + n * es;
	WORD	t;

	// Handle small array size inputs efficiently
	if (n < 7) {
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

	nudge(a, n, es, is_lt, swaptype);

	// Split into 3 parts

	size_t	nc = n / 3;
	size_t	nb = (n - nc) / 2;
	size_t	na = n - (nb + nc);

//	printf("n = %ld, na = %ld, nb = %ld, nc = %ld\n", n, na, nb, nc);
	char	*b = a + (na * es);
	char	*c = b + (nb * es);

	// Now sort sub-arrays B and C
	__new_sort(b, nb, es, is_lt, swaptype);
	__new_sort(c, nc, es, is_lt, swaptype);

	__new_merge(a, na, b, nb, c, nc, es, is_lt, swaptype);

	// Here we keep looping and merging
	while (na > 3) {
		// Merge B & C.  A will return to us unsorted
		c = b;
		nc += nb;

		nb = na / 2;
		na -= nb;
		b = a + (na * es);

		__new_sort(b, nb, es, is_lt, swaptype);
		__new_merge(a, na, b, nb, c, nc, es, is_lt, swaptype);
	}

	if (na > 2)
		for (char *ta = a + es + es, *tb = ta + es; tb < e && is_lt(tb, ta); ta = tb, tb += es)
			swap(ta, tb);

	if (na > 1)
		for (char *ta = a + es, *tb = ta + es; tb < e && is_lt(tb, ta); ta = tb, tb += es)
			swap(ta, tb);

	for (char *ta = a, *tb = ta + es; tb < e && is_lt(tb, ta); ta = tb, tb += es)
		swap(ta, tb);
} // __new_sort


void
new_sort(char *a, const size_t n, const size_t es, const int (*is_lt)(const void *, const void *))
{
	char	*s, *e = a + (n * es);
	size_t	pos;
	int	swaptype;

	SWAPINIT(a, es);

	// Do a quick pass to nudge values roughly where they need to be
//	do_is(a, n, es, is_lt, swaptype, 6);		// Step of 8
//	do_is(a, n, es, is_lt, swaptype, 23);		// Step of 23
	__new_sort(a, n, es, is_lt, swaptype);
//	__is(a, e, es, is_lt, swaptype, 1);
//	dumpa(a, e, es);
//	printf("num_rolls = %ld\n", num_rolls);
} // new_sort
