//			Insertion Sort
//
// Author: Stew Forster (stew675@gmail.com)	Date: 25 July 2021
//
// My implementation of the classic Insertion Sort algorithm
//

#include <stddef.h>
#include <string.h>
#include "swap.h"

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
insertion_sort(register char *a, size_t n, register const size_t es, register const int (*is_lt)(const void *, const void *))
{
	register int	swaptype;
	char		tmp[es];

	if (n < 2)
		return;

	SWAPINIT(a, es);

        for (register char *s=a, *p=a+es, *e=a+n*es; p!=e; s=p, p+=es) 
		if (is_lt(p, s)) {
			for (; s!=a && is_lt(p, s-es); s-=es);
			copy(tmp, p);
			memmove(s+es, s, p-s);
			copy(s, tmp);
		}
//	do {
//		for (p -= es, s = p; s!=e && is_lt(s+es, s); s += es)
//			swap(s, s+es);
//	} while (p != a);
} // insertion_sort
