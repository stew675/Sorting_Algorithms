#ifndef SWAP_H
#define SWAP_H

#include <stdint.h>
extern uint64_t numswaps, numcopies;

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

// Copies from b to a, and then from c to b, in one operation
static void
copyfunc3(char *a, char *b, char *c, size_t n, int swaptype)
{
	numcopies++;
	if (swaptype <= 1) {
		for( ; n; a += W, b += W, n -= W) {
			*(WORD*)a = *(WORD*)b;
			*(WORD*)b = *(WORD*)c;
		}
	} else {
		for( ; n; n--, *a++ = *b, *b++ = *c++);
	}
} // copyfunc3


// Copies from b to a, and then from c to b, in one operation
#define copy3(a, b, c)					\
	if (swaptype) {					\
		copyfunc3(a, b, c, es, swaptype);	\
	} else {					\
		*(WORD*)(a) = *(WORD*)(b);		\
		*(WORD*)(b) = *(WORD*)(c);		\
		numcopies++;				\
	}

// Copies from b to a
static void
copyfunc(char *a, char *b, size_t n, int swaptype)
{
	numcopies++;
	if (swaptype <= 1) {
		for( ; n; a += W, b += W, n -= W) {
			*(WORD*)a = *(WORD*)b;
		}
	} else {
		for( ; n; n--, *a++ = *b++);
	}
} // copyfunc


// Copies from b to a
#define copy(a, b)					\
	if (swaptype) {					\
		copyfunc(a, b, es, swaptype);		\
	} else {					\
		*(WORD*)(a) = *(WORD*)(b);		\
		numcopies++;				\
	}


#define exch(a, b, t)					\
	(t = a, a = b, b = t)

static void
swapfunc(char *a, char *b, size_t n, int swaptype)
{
	numswaps++;
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

#define swap(a, b)					\
	if (swaptype) {					\
		swapfunc(a, b, es, swaptype);		\
	} else {					\
		(void)exch(*(WORD*)(a), *(WORD*)(b), t);\
		numswaps++;				\
	}

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
