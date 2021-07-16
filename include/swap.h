#ifndef SWAP_H
#define SWAP_H

// swap.h
//
// Set of handy macros for swapping things
//

#define min(a,b)		\
   ({ __typeof__ (a) _a = (a);	\
       __typeof__ (b) _b = (b);	\
     _a < _b ? _a : _b; })

typedef int WORD;

#define W sizeof(WORD) /* must be a power of 2 */
#define SWAPINIT(a, es)					\
	swaptype = (a - (char*)0 | es) % W ? 2 : es > W ? 1 : 0

#define exch(a, b, t)					\
	(t = a, a = b, b = t)

#define swap(a, b)					\
	swaptype != 0 ? swapfunc(a, b, es, swaptype) :	\
	(void)exch(*(WORD*)(a), *(WORD*)(b), t)

static void
swapfunc(char *a, char *b, size_t n, int swaptype)
{
	if (swaptype <= 1) {
		WORD t;
		for( ; n > 0; a += W, b += W, n -= W) {
			exch(*(WORD*)a, *(WORD*)b, t);
		}
	} else {
		char t;
		for( ; n > 0; a += 1, b += 1, n -= 1) {
			exch(*a, *b, t);
		}
	}
} // swapfunc

#define vecswap(a, b, n)			\
	if (n > 0) {				\
		swapfunc(a, b, n, swaptype);	\
	}

#define PVINIT(pv, pm)				\
	if (swaptype != 0) {			\
		pv = a;				\
		swap(pv, pm);			\
	} else {				\
		pv = (char*)&v;			\
		v = *(WORD*)pm;			\
	}

#endif
