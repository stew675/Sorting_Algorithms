#include "swap.h"

#define	SMULT	(0.467)	// Small es

void
_shaker_sort(char a[], int n, int es, int (*cmp)())
{
	register char	*b, *c, *d, *e, *f, *hi;
	register int	stepsize, swaptype;
	register WORD t;

	if(n <= 1)
		return;

	SWAPINIT(a, es);

	hi = a + (n - 1) * es;
	stepsize = n;

	while(stepsize > 1) {
		if((stepsize = (stepsize * SMULT)) < 1)
			stepsize = 1;
		d = a + (stepsize * es);
		for(b = a, c = d; c <= hi; b+=es, c+=es) {
			if(cmp(b, c) <= 0)
				continue;
			swap(b, c);
			for(f = b; f >= d; f = e) {
				e = f - (stepsize * es);
				if(cmp(e, f) <= 0)
					break;
				swap(e, f);
			}
		}
	}
} // shaker_sort_generic


void
shaker_sort(void *a, size_t n, size_t es, int (*cmp)())
{
	double smult;

	if(n <= 1)
		return;
	_shaker_sort((char *)a, n, es, cmp);
} // shaker_sort
