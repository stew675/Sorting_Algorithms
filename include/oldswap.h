#ifndef OLDSWAP_H
#define OLDSWAP_H

#define min(a,b)                \
   ({ __typeof__ (a) _a = (a);  \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

typedef int WORD;

#define copycode(TYPE, parmi, parmj, n) 		\
{							\
	register int i = (n) / sizeof (TYPE);		\
	register TYPE *pi = (TYPE *) (parmi);		\
	register TYPE *pj = (TYPE *) (parmj);		\
	do {						\
		*pi++ = *pj++;				\
	} while (--i > 0);				\
}

#define swapcode(TYPE, parmi, parmj, n) 		\
{							\
	register int i = (n) / sizeof (TYPE);		\
	register TYPE *pi = (TYPE *) (parmi);		\
	register TYPE *pj = (TYPE *) (parmj);		\
	do {						\
		register TYPE   t = *pi;		\
		*pi++ = *pj;				\
		*pj++ = t;				\
	} while (--i > 0);				\
}

#define SWAPINIT(a, es) swaptype = ((char *)a - (char *)0) % sizeof(int) || es % sizeof(int) ? 2 : es == sizeof(int)? 0 : 1;

static void
copyfunc(char *a, char *b, int n, int swaptype)
{
	if(swaptype <= 1)
		copycode(int, a, b, n)
	else
		copycode(char, a, b, n)
}

static void
swapfunc(char *a, char *b, int n, int swaptype)
{
	if(swaptype <= 1)
		swapcode(int, a, b, n)
	else
		swapcode(char, a, b, n)
}

#define copy(a, b)					\
	if (swaptype == 0) {				\
		*(int *)(a) = *(int *)(b);		\
	} else {					\
		copyfunc(a, b, es, swaptype);		\
	}

#define swap(a, b)					\
	if (swaptype == 0) {				\
		register int t = *(int *)(a);		\
		*(int *)(a) = *(int *)(b);		\
		*(int *)(b) = t;			\
	} else {					\
		swapfunc(a, b, es, swaptype);		\
	}

#endif
