#include	<sys/types.h>
#include        <stdint.h>

#define	SMULT	(0.467)	// Small es

#define swapcode(TYPE, parmi, parmj, n) { \
	long i = (n) / sizeof (TYPE); \
	register TYPE *pi = (TYPE *) (parmi); \
	register TYPE *pj = (TYPE *) (parmj); \
	do { \
		register TYPE	t = *pi;	\
		*pi++ = *pj;			\
		*pj++ = t;			\
        } while (--i > 0);	\
}

#define SWAPINIT(a, es) swaptype = ((char *)a - (char *)0) % sizeof(long) || \
	es % sizeof(long) ? 2 : es == sizeof(long)? 0 : 1;

static void
swapfunc(a, b, n, swaptype)
char *a, *b;
int n, swaptype;
{
	if(swaptype <= 1) 
		swapcode(long, a, b, n)
	else
		swapcode(char, a, b, n)
}

#define swap(a, b) \
	if (swaptype == 0) { \
		long t = *(long *)(a); \
		*(long *)(a) = *(long *)(b); \
		*(long *)(b) = t; \
	} else { \
		swapfunc(a, b, es, swaptype); \
	}

void
_shaker_sort(char a[], int n, int es, int (*cmp)())
{
	register	char	*b, *c, *d, *e, *f, *hi;
	register	int	stepsize, swaptype;

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
