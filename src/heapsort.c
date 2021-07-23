//					HEAP SORT
//
// Author: Stew Forster (stew675@gmail.com)
//
// My purely iterative implementation of the classic heap sort algorithm
//
// The ordering of everything as it appears in the macro is absolutely
// critical to achieve the greatest speed.  I deliberately interleaved
// various calculations to occur while unrelated operations take place
// which helps the CPU instruction scheduler to order the operations in
// the fastest way through the CPU pipeline
//
// This version of hash-sort absolutely flies on my AMD 5950x.  For set
// sizes of 100000 items or less, it is the fastest of any algorithm in
// this repository.  Even up to 400000 set sizes it is still faster than
// any other general purpose sorting algorithm on the Zen 3 CPU
//
// Once we pass 1M elements though, it starts to fall behind every other
// O(n log n) general purpose sort.  The inherently poor spatial locality
// of the algorithm starts to thrash the CPU caches with such large data
// sets

#include <stddef.h>
#include "oldswap.h"

#define heapify(p)								\
	for (l = p + (p - a) + es, max = p; l < e;) {				\
		root = max;							\
		r = l + es;							\
		(cmp(l, max) > 0) && (max = l);					\
		(r < e) && (cmp(r, max) > 0) && (max = r);			\
		if (max == root) break;						\
		l = max + (max - a) + es;					\
		swap(root, max);						\
	}

void
heap_sort(char *a, size_t n, register size_t es, register const int (*cmp)(const void *, const void *))
{
	register char *e=a+n*es, *max, *l, *r, *root;
	register int swaptype;

	SWAPINIT(a, es);

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
}
