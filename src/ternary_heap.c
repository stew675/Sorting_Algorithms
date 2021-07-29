//				HEAP SORT
//
// Author: Stew Forster (stew675@gmail.com)	Date: 23 July 2021
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
// Once we exceed 1M elements though, it starts to fall behind the other
// O(n log n) general purpose sorts.  The poor spatial locality of the
// algorithm starts to thrash the CPU caches with such large data sets

#include <stddef.h>
#include <stdint.h>
#include "newswap.h"

void
ternary_heap(register char *a, size_t n, register size_t es, register const int (*is_less_than)(const void *, const void *))
{
	register char *e=a+n*es, *max, *l, *llm, *lm, *m, *rm, *rrm, *r, *root;

#define heapify(p)										\
	for (l = p + 6*(p-a) + es, max = p; l < e;) {						\
		root = max;									\
		llm = l + es;									\
		is_less_than(max, l) && (max = l);						\
		if (llm < e) {									\
			lm = l + 2*es;								\
			if (is_less_than(max, llm))						\
		       		max = llm;							\
			if (lm < e) {								\
				m = l + 3*es;							\
				if (is_less_than(max, lm))					\
			       		max = lm;						\
				if (m < e) {							\
					rm = l + 4*es;						\
					if (is_less_than(max, m))				\
						max = m;					\
					if (rm < e) {						\
						rrm = l + 5*es;					\
						if (is_less_than(max, rm))			\
					       		max = rm;				\
						if (rrm < e) {					\
							r = l + 6*es;				\
							if (is_less_than(max, rrm))		\
								max = rrm;			\
							if (r < e)				\
								if (is_less_than(max, r))	\
						       			max = r;		\
						}						\
					}							\
				}								\
			}									\
		}										\
		if (max == root) break;								\
		l = max+6*(max-a)+es;								\
		swap(root, max, es);								\
	}

	// Build the heap
	for (register char *b=a+(n/7-1)*es; b>=a; b-=es) {
		heapify(b);
	}

	// The first element will always be the current maximum
	// Swap it to the end and bring the end in by one element
	// until we end up completely draining the heap
	for (e-=es; e>a; e-=es) {
		swap(a, e, es);
		heapify(a);
	}
#undef heapify
} // ternary_heap

/*
						0
			1			2			3
	 	4	5	6	7	8	9	10	11	12
	    13 14 15|16 17 18| 

*/
