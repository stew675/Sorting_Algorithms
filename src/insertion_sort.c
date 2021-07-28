//			Insertion Sort
//
// Author: Stew Forster (stew675@gmail.com)	Date: 25 July 2021
//
// My implementation of the classic Insertion Sort algorithm
//

#include <stddef.h>

#include "newswap.h"

/*
void
insertion_sort(register char *a, size_t n, register const size_t es, register const int (*is_less_than)(const void *, const void *))
{
        register char   *p, *s, *e = a + n * es;
        char            tmp[es];

        for (p = a+es; p < e; p+=es) 
                if (is_less_than(p, p-es)) {
                        copy(tmp, p, es);
                        copy(p, p-es, es);
                        for (s=p-es; s>a && is_less_than(tmp,s-es); s-=es)
                                copy(s, s-es, es);
                        copy(s, tmp, es);
                }
} // insertion_sort
*/

void
insertion_sort(register char *a, size_t n, register const size_t es, register const int (*is_less_than)(const void *, const void *))
{
	register char	*p, *s, *e = a + n * es;

	for (p = a+es; p < e; p+=es)
		for(s=p; (s>a) && is_less_than(s, s-es); s-=es)
			swap(s, s-es, es);
} // insertion_sort
