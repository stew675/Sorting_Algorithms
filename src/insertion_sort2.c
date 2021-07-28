//			Insertion Sort 2
//
// Author: Stew Forster (stew675@gmail.com)	Date: 25 July 2021
//
// My implementation of the classic Insertion Sort algorithm
// Here we use a binary search to locate where we need to shift the current
// element to, and use highly memory efficient memmove calls to move blocks

#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "newswap.h"

extern void print_array(void *a, size_t n);

void
insertion_sort2(register char *a, size_t n, register const size_t es, register const int (*is_less_than)(const void *, const void *))
{
	register char	*p, *s, *e = a + n * es;
	char		tmp[es];

	// Now start inserting, using a binary search to find where
	for (p = a+es; p < e; p+=es) {
		if (!is_less_than(p, p-es))
			continue;
		copy(tmp, p, es);

		// For this sort of thing, using integers is a lot faster
		// than doing pointer arithmetic to find pos
		register size_t start = 0, end = ((p-a)/es)-1, pos;
		for (pos = (end+start)>>1; start < end; pos = (end+start)>>1) {
			if (is_less_than(tmp, a+pos*es))
				end=pos;
			else
				start = pos+1;
		}
		s=a+pos*es;
		memmove(s+es, s, p-s);
		copy(s, tmp, es);
	}
} // insertion_sort2
