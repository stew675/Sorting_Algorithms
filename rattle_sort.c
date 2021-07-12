#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define swapcode(TYPE, parmi, parmj, n) {		\
	register TYPE *pi = (TYPE *) (parmi);		\
	register TYPE *pj = (TYPE *) (parmj);		\
	register int i = (n) / sizeof (TYPE);		\
	do {						\
		register TYPE	t = *pi;		\
		*pi++ = *pj;				\
		*pj++ = t;				\
        } while (--i > 0);				\
}

#define SWAPINIT(a, es) swaptype = ((((char *)a - (char *)0) % sizeof(int)) || (es % sizeof(int))) ? 2 : es == sizeof(int) ? 0 : 1;

static void
swapfunc(char *a, char *b, int n, int swaptype)
{
	if(swaptype <= 1) 
		swapcode(int, a, b, n)
	else
		swapcode(char, a, b, n)
}

#define swap(a, b)					\
	if (swaptype == 0) {				\
		register int t = *(int *)(a);		\
		*(int *)(a) = *(int *)(b);		\
		*(int *)(b) = t;			\
	} else {					\
		swapfunc(a, b, es, swaptype);		\
	}

void
_rattle_sort(register char *a, size_t n, register size_t es, register int (*cmp)())
{
	register int	swaptype = 2;	// A type of 2, means byte-wise, which is always a safe default
	register char	*hi = a + (n - 1) * es;
	register char	*b, *c;

	SWAPINIT(a, es);

	for(;;) {
		// Forward sift
		if ((n = (n * 13) / 17) < 2) break;
		for(b = a, c = a + (n * es); c <= hi; b+=es, c+=es)
			if(cmp(b, c) > 0) swap(b, c);

		// Backward sift
		if ((n = (n * 13) / 17) < 2) break;
		for(b = hi, c = hi - (n * es); c >= a; b-=es, c-=es)
			if(cmp(c, b) > 0) swap(b, c);
	}

	for(;;) {
		register char *pos;

		for(pos = NULL, b = a, c = a + es; c <= hi; b=c, c+=es)
			if(cmp(b, c) > 0) {
				swap(b, c);
				pos = b;
			}
		if (pos == NULL) break;
		hi = pos;

		for(pos = NULL, b = hi, c = hi - es; c >= a; b=c, c-=es)
			if(cmp(c, b) > 0) {
				swap(b, c);
				pos = b;
			}
		if (pos == NULL) break;
		a = pos;
	}
} // _rattle_sort


void
rattle_sort(void *a, size_t n, size_t es, int (*cmp)())
{
	if(n < 2)
		return;

	_rattle_sort((char *)a, n, es, cmp);
} // rattle_sort
