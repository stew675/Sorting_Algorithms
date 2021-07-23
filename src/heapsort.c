//					HEAP SORT
//
// Author: Stew Forster (stew675@gmail.com)
//
// My purely iterative implementation of the class heap sort algorithm

#include <stddef.h>
#include "oldswap.h"

#define heapify(p)								\
	for (l = p + (p - a) + es, max = p; l < e; l = max + (max - a) + es) {	\
		root = max;							\
		r = l + es;							\
		(cmp(l, max) > 0) && (max = l);					\
		(r < e) && (cmp(r, max) > 0) && (max = r);			\
		if (max == root) break;						\
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
