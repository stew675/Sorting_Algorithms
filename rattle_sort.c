#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>

#define swapcode(TYPE, parmi, parmj, n) {	\
	register TYPE *pi = (TYPE *) (parmi);	\
	register TYPE *pj = (TYPE *) (parmj);	\
	register long i = (n) / sizeof (TYPE);	\
	do {					\
		register TYPE	t = *pi;	\
		*pi++ = *pj;			\
		*pj++ = t;			\
        } while (--i > 0);			\
}

#define SWAPINIT(a, es) swaptype = ((char *)a - (char *)0) % sizeof(long) || \
	es % sizeof(long) ? 2 : es == sizeof(long) ? 0 : 1;

static void
swapfunc(char *a, char *b, int n, int swaptype)
{
	if(swaptype <= 1) 
		swapcode(long, a, b, n)
	else
		swapcode(char, a, b, n)
}

#define swap(a, b)				\
	if (swaptype == 0) {			\
		register long t = *(long *)(a);	\
		*(long *)(a) = *(long *)(b);	\
		*(long *)(b) = t;		\
	} else {				\
		swapfunc(a, b, es, swaptype);	\
	}

void
_rattle_sort(char *a, size_t n, register size_t es, register int (*cmp)())
{
	register int	swaptype = 2;	// A type of 2, means byte-wise, which is always a safe default
	register char	*b, *c, *hi;
	size_t		stepsize = n;

	SWAPINIT(a, es);

	hi = a + (n - 1) * es;

	for(;;) {
		register int swaps;

		/* Forward sift */

		stepsize = stepsize > 1 ? (stepsize * 13) / 17 : 1;
		swaps = 0;
		for(b = a, c = a + (stepsize * es); c <= hi; b+=es, c+=es)
			if(cmp(b, c) > 0) {
				swap(b, c);
				swaps++;
			}
		if ((swaps == 0) && (stepsize == 1))
			break;

//		printf("stepsize = %lu\n", stepsize);

		/* Backward sift */

		stepsize = stepsize > 1 ? (stepsize * 13) / 17 : 1;
		swaps = 0;
		for(b = hi, c = hi - (stepsize * es); c >= a; b-=es, c-=es)
			if(cmp(c, b) > 0) {
				swap(b, c);
				swaps++;
			}
		if ((swaps == 0) && (stepsize == 1))
			break;
	}
} // _rattle_sort


void
rattle_sort(void *a, size_t n, size_t es, int (*cmp)())
{
	if(n < 2)
		return;

	_rattle_sort((char *)a, n, es, cmp);
} // rattle_sort
