#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "swap.h"

static __thread int depth_limit, depth;

// The get index of the most significant bit of a 64 bit value
static uint8_t
msb64(uint64_t v)
{
	static const uint64_t dbm64 = (uint64_t)0x03f79d71b4cb0a89;
	static const uint8_t dbi64[64] = {
		 0, 47,  1, 56, 48, 27,  2, 60, 57, 49, 41, 37, 28, 16,  3, 61,
		54, 58, 35, 52, 50, 42, 21, 44, 38, 32, 29, 23, 17, 11,  4, 62,
		46, 55, 26, 59, 40, 36, 15, 53, 34, 51, 20, 43, 31, 22, 10, 45,
		25, 39, 14, 33, 19, 30,  9, 24, 13, 18,  8, 12,  7,  6,  5, 63
	};

	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v |= v >> 32;
	return dbi64[(v * dbm64) >> 58];
}


// Heap Sort Implementation
static void
_hs(register char *a, size_t n, register size_t es, register const int (*is_less_than)(const void *, const void *), register int swaptype)
{
	register char *e=a+n*es, *max, *l, *r, *root;
	register WORD t;

#define heapify(p)							\
	for (l = p + (p - a) + es, max = p; l < e;) {			\
		root = max;						\
		r = l + es;						\
		is_less_than(max, l) && (max = l);			\
		(r < e) && is_less_than(max, r) && (max = r);		\
		if (max == root) break;					\
		l = max + (max - a) + es;				\
		swap(root, max);					\
	}

	// Build the heap
	for (register char *b=a+(n/2-1)*es; b>=a; b-=es)
		heapify(b);

	// The first element will always be the current maximum
	// Swap it to the end and bring the end in by one element
	// until we end up completely draining the heap
	for (e-=es; e>a; e-=es) {
		swap(a, e);
		heapify(a);
	}
#undef heapify
} // _hs


static char *
partition(register char *a, size_t n, register const size_t es, register const int (*is_less_than)(const void *, const void *), register int swaptype)
{
	register char *e=a+(n-1)*es, *s;  // e should point AT the last element in the array a
	register WORD t;

	// Select a pivot point using median of 3 and move it to e
	// Here we choose the last element (e) plus 2 random ones
	do {
		register char *m = a +(random()%(n-1))*es;

		do {
			s = a +(random()%(n-1))*es;
		} while (m == s);

		if (is_less_than(e, m)) {
			// e < m
			if (is_less_than(s, e)) {
				// s < e < m
				break;
			}
			if (is_less_than(s, m)) {
				// e < s < m
				swap(s, e);
				break;
			}
			// e < m < s
			swap(m, e);
			break;
		}
		// m < e
		if (is_less_than(e, s)) {
			// m < e < s
			break;
		}
		if (is_less_than(s, m)) {
			// s < m < e
			swap(m, e);
			break;
		}
		// m < s < e
		swap(s, e);
	} while (0);


	// Now partition the array around the pivot point's value
	s = e;
	while (a < s)
		if (is_less_than(e, a)) {
			s-=es;
			swap(a, s);
		} else {
			a+=es;
		}

	// Move the pivot point into position
	if (s != e)
		swap(s, e);

	// Return a pointer to the partition point
	return s;
} // partition


extern void print_array(void *, size_t);

static void
intro_sort_util(register char *a, size_t n, register const size_t es, register const int (*is_less_than)(const void *, const void *), register int swaptype)
{
	register char *p;

	for (depth++;;) {

		// Insertion Sort
		if (n < 19) {
			register char   *s, *e = a + n * es;
			register WORD t;

			for (p = a+es; p < e; p+=es)
				for(s=p; (s>a) && is_less_than(s, s-es); s-=es)
					swap(s, s-es);
			depth--;
			return;
		}

		// Heap Sort
		if (n < 7000 || depth >= depth_limit ) {
			_hs(a, n, es, is_less_than, swaptype);
			depth--;
			return;
		}

		// Quick Sort
		p = partition(a, n, es, is_less_than, swaptype);
		intro_sort_util(a, (p-a)/es, es, is_less_than, swaptype);

		// Rather than recurse here, just restart the loop.  This is the same
		// same as doing the following, just without the actual function call
		//   intro_sort_util(p, n - ((p-a)/es), es, is_less_than, swaptype, dl - 1);
		p += es;
		n -= (p-a)/es;
		a = p;
	}
} // intro_sort_util


// Implementation of intro_sort
void
intro_sort(register char *a, size_t n, register const size_t es, register const int (*is_less_than)(const void *, const void *))
{
	int swaptype;

	if (n < 2)
		return;

	SWAPINIT(a, es);

	depth = 0;
	depth_limit = (int)msb64((uint64_t)n);
 
	// Perform a recursive intro_sort
	intro_sort_util(a, n, es, is_less_than, swaptype);
} // intro_sort
