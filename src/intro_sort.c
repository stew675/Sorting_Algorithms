#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "swap.h"

static __thread int depth_limit, depth;

// The get index of the most significant bit of a 64 bit value
static int
msb64(uint64_t v)
{
	static const uint64_t dbm64 = (uint64_t)0x03f79d71b4cb0a89;
	static const uint8_t dbi64[64] = {
		 0, 47,  1, 56, 48, 27,  2, 60, 57, 49, 41, 37, 28, 16,  3, 61,
		54, 58, 35, 52, 50, 42, 21, 44, 38, 32, 29, 23, 17, 11,  4, 62,
		46, 55, 26, 59, 40, 36, 15, 53, 34, 51, 20, 43, 31, 22, 10, 45,
		25, 39, 14, 33, 19, 30,  9, 24, 13, 18,  8, 12,  7,  6,  5, 63
	};

	if (!v)
		return -1;
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
	register char	*e=a+n*es, *max, *l, *r, *root;
	register WORD	t;

#define heapify(p)							\
	for (l = p + (p - a) + es, max = p; l < e;) {			\
		root = max;						\
		r = l + es;						\
		if (is_less_than(max, l)) max = l;			\
		if ((r < e) && is_less_than(max, r)) max = r;		\
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


#define med3(a, b, c)  (is_less_than(a, b) ?                                   \
                       (is_less_than(b, c) ? b : is_less_than(a, c) ? c : a) : \
                       (is_less_than(c, b) ? b : is_less_than(c, a) ? c : a)) 

static char *
partition(register char *a, size_t n, register const size_t es, register const int (*is_less_than)(const void *, const void *), register int swaptype)
{
	register char	*e=a+(n-1)*es, *p=a+(n/2)*es;  // e should point AT the last element in the array a
	register WORD	t;

	// Select a pivot point using median of 3
	p = med3(a, p, e);

	// Do a pseudo median-of-9 for larger partitions
	if (n > 63) {
		register size_t ne = (n/8)*es;
		register char  *pl = med3(a+ne*2,  a+ne,  a+ne*3);
		register char  *pr = med3(a+ne*5, a+ne*6, a+ne*7);

		p = med3(pl, p, pr);
	}

	// Move the pivot value to the last element in the array
	if (p != e)
		swap(p, e);

	// Now partition the array around the pivot point's value
	// Remember: e contains the pivot value
	for (p = e; a < p; )
		if (is_less_than(e, a)) {
			p-=es;
			swap(a, p);
		} else
			a+=es;

	// Move the pivot point into position
	if (p != e)
		swap(p, e);

	// Return a pointer to the partition point
	return p;
} // partition


extern void print_array(void *, size_t);

static void
_intro_sort(register char *a, size_t n, register const size_t es, register const int (*is_less_than)(const void *, const void *), register int swaptype)
{
	register char	*p;

	for (depth++;;) {
		// Insertion Sort
		if (n < 19) {
			register char	*s, *e = a + n * es;
			register WORD	t;

			for (p = a+es; p < e; p+=es)
				for(s=p; (s>a) && is_less_than(s, s-es); s-=es)
					swap(s, s-es);
			depth--;
			return;
		}

		// Heap Sort
		if (depth == depth_limit) {
			_hs(a, n, es, is_less_than, swaptype);
			depth--;
			return;
		}

		// Quick Sort
		p = partition(a, n, es, is_less_than, swaptype);
		_intro_sort(a, (p-a)/es, es, is_less_than, swaptype);

		// Rather than recurse here, just restart the loop.  This is the same
		// same as doing the following, just without the actual function call
		//   _intro_sort(p+es, n - (((p+es)-a)/es), es, is_less_than, swaptype);
		p += es;
		n -= (p-a)/es;
		a = p;
	}
} // _intro_sort


// Implementation of intro_sort
void
intro_sort(register char *a, size_t n, register const size_t es, register const int (*is_less_than)(const void *, const void *))
{
	int swaptype;

	if (n < 2)
		return;

	SWAPINIT(a, es);

	depth = 0;
	depth_limit = msb64((uint64_t)n);

	// Perform a recursive intro_sort
	_intro_sort(a, n, es, is_less_than, swaptype);
} // intro_sort
