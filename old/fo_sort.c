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

static void __mip_sort2(char *a, const size_t n, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype);
static void __mip_sort3(char *a, const size_t n, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype);

// Algorithm below relies on SPLIT_MIN never being less than 4
#define SPLIT_MIN	4

// Merges two sorted sub-arrays in place
// Assumes that arrays of zero length will not be passed in
// Assumes that A and B are byte-wise continguous as a whole
// Assumes that A is before B in the address space
// Assumes that number of items in B >= the number of items in A
static void
__mip_sub3(char *a, size_t na, char *b, size_t nb, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
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
	for (pa -= es, pb += es; pa >= a && pb < e && is_lt(pb, pa); pa -= es, pb += es);
	pa += es;

	// We now have 4 sub-sets
	// 1. A  -> PA
	// 2. PA -> B
	// 3. B  -> PB
	// 4. PB -> E
	// Now merge PA->B into B->E.
	// When done PA->B will be loosely unsorted with both A->PA, and B->E fully sorted
	
	for (char *ta = pa, *tb = pb, *tc = b; ta < b; tc += es)
		if (tb < e && is_lt(tb, ta)) {
			swap(tb, tc);
			tb += es;
		} else {
			swap(ta, tc);
			ta += es;
		}
	// Now sort the PA -> B subset
	nb = (b - pa) / es;
	na = (pa - a) / es;
	for (char *ta = pa, *tb = pa + es; tb < b; ta = tb, tb += es)
		for (char *tc = tb; tc > pa && is_lt(tc, ta); tc = ta, ta -= es)
			swap(tc, ta);
//	if (nb > 1)
//		__mip_sort3(pa, nb, es, is_lt, swaptype);

	// We now have two unmerged sorted subsets, A->PA, and PA->B.  Let's merge them now
	if (na > 0 && nb > 0)
		__mip_sub3(a, na, pa, nb, es, is_lt, swaptype);
} // __mip_sub3

static void
__mip_sort3(char *a, const size_t n, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	char	*e = a + n * es;
	WORD	t;

	// Handle small array size inputs efficiently
	if (n < 8) {
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

	// Split into 2 equal parts

	size_t	na = n / 2;
	size_t	nb = n - na;
	char	*b = a + (na * es);

	// Sort each sub-array
	__mip_sort3(a, na, es, is_lt, swaptype);
	__mip_sort3(b, nb, es, is_lt, swaptype);

	// Now merge the two sub-arrays
	__mip_sub3(a, na, b, nb, es, is_lt, swaptype);
} // __mip_sort3


// Merges two sorted sub-arrays in place
// Assumes that arrays of zero length will not be passed in
// Assumes that A and B are byte-wise continguous as a whole
// Assumes that A is before B in the address space
static void
__mip_sub(char *a, const size_t na, char *b, const size_t nb, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	char	*e = b + (nb * es);		// E points at immediately AFTER the last elemnent in B
	char	*pa = b - es;
	char	*pb = b;
	WORD	t;				// Temporary variable for swapping
	
	if (!is_lt(b, pa))			// Already sorted
		return;

	// Find the pivot points in A and B, such that when swapped at that point,
	// all elements in A will be less than, or equal to, any element in B.
	for (pa -= es, pb += es; pa >= a && pb < e && is_lt(pb, pa); pa -= es, pb += es);
	pa += es;

	// Now swap the A and B sub-arrays at the pivot points
	for (char *ta = pa, *tb = b; ta < b; ta += es, tb += es)
		swap(ta, tb);

	size_t ns = (b - pa) / es;

	// Now merge-in-place the two sub-arrays as required
	// Ensure that we don't pass in 0 length sub-arrays
	if (pa > a)
		__mip_sub(a, na - ns, pa, ns, es, is_lt, swaptype);

	if (pb < e)
		__mip_sub(b, ns, pb, nb - ns, es, is_lt, swaptype);
} // __mip_sub


static void
__mip(char *a, const size_t na, char *b, const size_t nb, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	char	*e = b + (nb * es);		// E points at immediately AFTER the last elemnent in B
	char	*c = b;
	WORD	t;				// Temporary variable for swapping
	
	while (b < e) {
		if (!is_lt(b, b - es))			// Already sorted
			return;

		// We know that at least 1 element in A is greater than the least element in B

		for (; a < c; a += es) {
			if (!is_lt(b, a))
				continue;
			swap(b, a);
			b += es;
			if ((b == e) || is_lt(b - es, b)) {
				// Rest of B is > all of C
				b = c;
				continue;
			}
			if (is_lt(c, b)) {
				swap(c, b);
				for (char *tc = c, *tn = c + es; is_lt(tn, tc); tc = tn, tn += es) {
					num_rolls++;
					swap(tn, tc);
				}
			}
		}
		if (b == c)
			break;
		c = b;
	}
	
#if 0
	while (b < e) {
		while (a < c && !is_lt(b, a))
			a += es;
	swap(b, a);
	b += es;
//	printf("MIP called, na = %ld, nb = %ld\n", na, nb);

	if (!is_lt(b, ea))
		return;				// Already sorted.  All of A is less than first element of B

	// At this point there are multiple elements in both A and B, AND we know that
	// there is at least one element in A greater than the first element of B
	// From here on, both a and b are "floating" and can advance in position
	// A must always remain <= C, and B must always remian <= E
	
	// Advance A to the first item that is greater than the first item in B
	while (!is_lt(b, a))
		a += es;
	swap(a, b);
	a += es;
	b += es;	// Now C points at first swapped element

	while (a < c) {
		if (is_lt(b, a)) {
			swap(a, b);
			b += es;
		}

		if (is_lt(c, a)) {
			char *tc, *tn;

			swap(a, c);
			for (tc = c, tn = c + es; tn < b && is_lt(tn, tc); tc = tn, tn += es)
				swap(tc, tn);
		}

		a += es;	// Advance A
	}

	if (b >= e)
		return;

	// Now ripple up the working set C, into B
	for (char *tc = b - es; tc >= c; tc -= es)
		for (char *s = tc, *n = tc + es; n < e && is_lt(n, s); s = n, n += es)
			swap(s, n);
#endif
} // mip


static void
__mip_sort(char *a, const size_t n, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	if (n < 7) {
		// If 0 or 1 items, then nothing to sort, just return
		if (n < 2)
			return;

		// Temporary variable for swapping
		WORD	t;

		// If n == 2, just do a compare, swap if needed, and return
		if (n == 2) {
			char *b = a + es;

			if (is_lt(b, a))
				swap(a, b);
			return;
		}

		// Do an insertion sort of tiny arrays
		char *e = a + n * es;
		for (char *s = a, *b = a + es; b < e; s = b, b += es)
			for (char *c = b; c > a && is_lt(c, s); c = s, s -= es)
				swap(c, s);

		return;
	}

	// Split the array into 2
	
	size_t	na = n / 2;			// Split array into 2
	size_t	nb = n - na;			// Number of elements in sub-array B
	char 	*b = a + (na * es);		// B points to start of 2nd sub-array

	// Now sort the two sub-arrays
	__mip_sort(a, na, es, is_lt, swaptype);

	__mip_sort(b, nb, es, is_lt, swaptype);

	// Now merge the two sub-arrays in place
//	__mip_sub(a, na, b, nb, es, is_lt, swaptype);
	__mip(a, na, b, nb, es, is_lt, swaptype);
} // __mip_sort



static void
__mip_sort2(char *a, const size_t n, const size_t es, const int (*is_lt)(const void *, const void *), int swaptype)
{
	char	*e = a + n * es;
	WORD	t;

	// Handle small array size inputs efficiently
	if (n < 10) {
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

	// Transfer all of A to working space C if required
	if (is_lt(b, b - es)) {
		char	*ta, *tb, *tc, *td;

		// Skip anything in A that we don't need to move
		for (ta = a; is_lt(ta, b); ta += es);

		// Swap the remainder of A with C
		for (tb = ta, tc = c; tb < b; tb += es, tc += es)
			swap(tb, tc);

		// Merge B and C into the remainder of A
		for (td = tc, tc = c; tb < c && tc < td; ta += es)
			if (is_lt(tc, tb)) {
				swap(tc, ta);
				tc += es;
			} else {
				swap(tb, ta);
				tb += es;
			}

		// Swap back anything leftover from C
		for ( ; tc < td; ta += es, tc += es)
			swap(ta, tc);
	}

	// Now sort the range C->E
	__mip_sort2(c, nc, es, is_lt, swaptype);

	// Now merge A->C, and C->E together
	__mip_sub(a, (na << 1), c, nc, es, is_lt, swaptype);
} // __mip_sort2


void
fo_sort(char *a, const size_t n, const size_t es, const int (*is_lt)(const void *, const void *))
{
	char	*s, *e = a + (n * es);
	size_t	pos;
	int	swaptype;

	SWAPINIT(a, es);

	// 1, 4, 9, 23, 57, 138, 326, 749, 1695, 3785, 8359, 18298, 39744

#if 0
	do_is(a, n, es, is_lt, swaptype, 4);		// Step of 4
	do_is(a, n, es, is_lt, swaptype, 9);		// Step of 9
	do_is(a, n, es, is_lt, swaptype, 23);		// Step of 23
	do_is(a, n, es, is_lt, swaptype, 57);		// Step of 57
	do_is(a, n, es, is_lt, swaptype, 138);		// Step of 138
	do_is(a, n, es, is_lt, swaptype, 326);		// Step of 326
	do_is(a, n, es, is_lt, swaptype, 749);		// Step of 749
	do_is(a, n, es, is_lt, swaptype, 1695);		// Step of 1695
	do_is(a, n, es, is_lt, swaptype, 3785);		// Step of 3785
	do_is(a, n, es, is_lt, swaptype, 8359);		// Step of 8359
	do_is(a, n, es, is_lt, swaptype, 18298);	// Step of 18298
	do_is(a, n, es, is_lt, swaptype, 39744);	// Step of 29744
#endif

	// Do a quick pass to nudge values roughly where they need to be
//	do_is(a, n, es, is_lt, swaptype, 8);		// Step of 8
//	do_is(a, n, es, is_lt, swaptype, 23);		// Step of 23
	__mip_sort2(a, n, es, is_lt, swaptype);
//	__is(a, e, es, is_lt, swaptype, 1);
//	dumpa(a, e, es);
//	printf("num_rolls = %ld\n", num_rolls);
} // rattle_sort
