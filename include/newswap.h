#ifndef __NEW_SWAP_H__
#define __NEW_SWAP_H__

#include <stdint.h>
#include <limits.h>

#if INT_MAX == 32767
#define _SIZE_MASK_	0x1
#elif INT_MAX == 2147483647
#define _SIZE_MASK_	0x3
#elif INT_MAX == 9223372036854775807
#define _SIZE_MASK_	0x7
#else
#define _SIZE_MASK_	UINT_MAX
#endif

extern uint64_t numswaps, numcopies;

#define swapcode(TYPE, parmi, parmj, n) 			\
{								\
	register TYPE *pi = (TYPE *) (parmi);			\
	register TYPE *pj = (TYPE *) (parmj);			\
	register TYPE   t = *pi;				\
	do { t = *pi; *pi++ = *pj; *pj++ = t; } while (--n);	\
}

static inline void
swapfunc(char *a, char *b, register size_t es)
{
	numswaps++;
	if (es & _SIZE_MASK_) {
		swapcode(char, a, b, es)
	} else {
		es /= sizeof(int);
		swapcode(int, a, b, es)
	}
}

#define swap(a, b, c)					\
	if (c & _SIZE_MASK_) {				\
		swapfunc(a, b, c);			\
	} else {					\
		register int ti = *(int *)(a);		\
		*(int *)(a) = *(int *)(b);		\
		*(int *)(b) = ti;			\
		numswaps++;				\
	}

#define copycode(TYPE, parmi, parmj, n) 		\
{							\
	register TYPE *pi = (TYPE *) (parmi);		\
	register TYPE *pj = (TYPE *) (parmj);		\
	do { *pi++ = *pj++; } while (--n);		\
}

static void
copyfunc(char *a, char *b, register size_t es)
{
	numcopies++;
	if (es & _SIZE_MASK_) {
		copycode(char, a, b, es);
	} else {
		es /= sizeof(int);
		copycode(int, a, b, es);
	}
}

#define copy(a, b, c)					\
	if (c & _SIZE_MASK_) {				\
		copyfunc(a, b, c);			\
	} else {					\
		numcopies++;				\
		*(int *)(a) = *(int *)(b);		\
	}

#endif
