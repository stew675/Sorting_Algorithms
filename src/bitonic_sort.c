//				Bidirectional Bubble Sort
//
// Author: Stew Forster (stew675@gmail.com)	Date: 19th July 2021
//
// This implements my version of the https://en.wikipedia.org/wiki/Cocktail_shaker_sort
// variant on Bubble Sort
//
// The start and end pointers are also brought together for sorted sections to reduce
// the size of the sorting partition over time

#include <stddef.h>
#include "swap.h"

/*

static inline void
merge_up(register char *a, size_t n, register const size_t es, register const int (*is_lt)(const void *, const void *), const register int swaptype)
{
	register size_t step=n/2, i, j, k;
	register char	*jp, *js;
	register WORD	t;

	while (step > 0) {
		for (i=0; i < n; i+=step*2) {
			for (j=i,k=0;k < step;j++,k++) {
				jp = a + j * es;
				js = a + (j+step) * es;
				if (is_lt(js, jp))
					swap(js, jp)
			}
		}
		step >>= 1;
	}
} // merge_up


static inline void
merge_down(register char *a, size_t n, register const size_t es, register const int (*is_lt)(const void *, const void *), const register int swaptype)
{
	register size_t step=n/2, i, j, k;
	register char	*jp, *js;
	register WORD	t;

	while (step > 0) {
		for (i=0; i < n; i+=step*2) {
			for (j=i,k=0;k < step;j++,k++) {
				jp = a + j * es;
				js = a + (j+step) * es;
				if (is_lt(jp, js))
					swap(js, jp)
			}
		}
		step >>= 1;
	}
} // merge_down

void
bitonic_sort(register char *a, size_t n, register const size_t es, register const int (*is_lt)(const void *, const void *))
{
	int	swaptype;

	SWAPINIT(a, es);
	for (register size_t s=2; s <= n; s*=2) {
		for (register size_t i=0; i < n; i+=(s<<1)) {
			merge_up(a+i*es, s, es, is_lt, swaptype);
			merge_down((a+(i+s)*es), s, es, is_lt, swaptype);
		}
	}
} // bitonic_sort
*/


// Only works on power of 2 sized values of n
void
bitonic_sort(register char *a, size_t n, register const size_t es, register const int (*is_lt)(const void *, const void *))
{
	register size_t	i, j, k, ij;
	register char	*b, *c;
	register int	swaptype;
	register WORD	t;

	SWAPINIT(a, es);

	for (k = 2; k <= n; k<<=1) // k is doubled every iteration
		for (j = k>>1; j > 0; j>>=1) // j is halved at every iteration, with truncation of fractional parts
			for (i = 0; i < n; i++) {
				b=a+i*es;
				ij=i^j;
				if (ij > i) {
					c = a+ij*es;
					if ((i&k)==0 && is_lt(c, b))
						swap(b, c);
					if ((i&k)!=0 && is_lt(b, c))
						swap(b, c);
				}
			}
} // bitonic_sort
