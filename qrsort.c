// Quick Radix Sort
//
// Copyright: stew675@gmail.com
// Author: stew675@gmail.com
// Date: 11 July 2021
//
// Email stew675@gmail.com for permission to use for commercial purposes
// Free for use for educational purposes
//
// Just a day spent doodling about, refreshing self on implementing radix sorts and then seeing if
// I could make it run faster than glibc qsort (which it is, within certain restrictions)
//
// Just for fun I implemented both recursive and interative variants.
//
// Generally faster than glibc qsort by around 10% or so when sorting large arrays of unsigned 32-bit integers
// Of course it has the restriction that keys must be of fixed unsigned 32-bit width
// It could expanded to 64 bit keys fairly easily

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define __do_swap(a, b, tmp, size)	\
	do {				\
		tmp = *a;		\
		*a++ = *b;		\
		*b++ = tmp;		\
	} while (--size > 0)

#define swap(a, b, size)					\
	if (size & 0x11) {					\
		register size_t __size = (size);		\
		register char *__a = (a), *__b = (b);		\
		register char __tmp;				\
		__do_swap(__a, __b, __tmp, __size);		\
	} else {						\
		size_t __size = (size)>>2;			\
		register uint32_t *__a = (uint32_t *)(a);	\
		register uint32_t *__b = (uint32_t *)(b);	\
		register uint32_t __tmp;			\
		__do_swap(__a, __b, __tmp, __size);		\
	}


#define __SELECT_THRESH	 7

// Iterative is faster than recursive by about 2%.  Not a huge difference, but it is consistent
#define __ITERATIVE

#ifdef __ITERATIVE
// We only add to the stack when absolutely necessary, otherwise we try to remain iterative where possible
static void
_qrsort(register char *ps, register char *pe, register size_t es, register uint32_t (*getkey)(), register uint32_t msb)
{
	register uint32_t mask, qpos = 1;
	register char *sps, *epe;
	char *q_ps[32], *q_pe[32];
	uint32_t q_msb[32];

	goto _rqsort_restart;

	for (; qpos > 0; qpos--) {
		ps = q_ps[qpos];
		pe = q_pe[qpos];
		msb = q_msb[qpos];

_rqsort_restart:
		sps = ps;
		epe = pe;

		// Do selection sort on partitions <= __SELECT_THRESH elements in size
		if (((pe - ps) / es) <= __SELECT_THRESH) {
			for (ps = sps; ps < epe; ps += es) {
				register char *smp;	// Smallest Pointer
				for (smp = ps, pe = ps + es; pe <= epe; pe += es) {
					if (getkey(pe) < getkey(smp)) {
						smp = pe;
					}
				}
				if (smp != ps) {
					swap(smp, ps, es);
				}
			}
			continue;
		}

		mask = ((uint32_t)1) << msb;

		// Radix sort the current partition
		for (;;) {
			// Sort based upon the current radix mask
			while (!(getkey(ps) & mask))
				if ((ps += es) == pe) goto _rqsort_stop_inner;
			while (getkey(pe) & mask)
				if ((pe -= es) == ps) goto _rqsort_stop_inner;

			swap (ps, pe, es);

			if ((ps += es) == pe) break;
			if ((pe -= es) == ps) break;
		}
_rqsort_stop_inner:
		// Pop next item on stack if this partition is now fully sorted
		if (msb == 0) {
			continue;
		}
		msb--;

		// ps == pe.  Adjust their position according to what they're pointing at
		if (getkey(ps) & mask)  {
			if (ps > sps) ps -= es;
		} else {
			if (pe < epe) pe += es;
		}

		if (pe < epe) {			// Only do right partition if there's 2 or more elements
			if (ps > sps) {		// Only do left partition if there's 2 or more elements
				q_ps[qpos] = pe;
				q_pe[qpos] = epe;
				q_msb[qpos] = msb;
				qpos++;

				// _rqsort(sps, ps, es, getkey, scanbits, msb);
				pe = ps;
				ps = sps;
			} else {
				// _rqsort(pe, epe, es, getkey, scanbits, msb);
				ps = pe;
				pe = epe;
			}
			goto _rqsort_restart;
		} else if (ps > sps) {		// Only do left partition if there's 2 or more elements
			// _rqsort(sps, ps, es, getkey, scanbits, msb);
			pe = ps;
			ps = sps;
			goto _rqsort_restart;
		}
	}
} // _qrsort

#else

static void
_qrsort(register char *ps, register char *pe, register size_t es, register uint32_t (*getkey)(), uint32_t msb)
{
	register uint32_t mask = ((uint32_t)1) << msb;
	char *sps = ps, *epe = pe;

	// Do selection sort on partitions <= __SELECT_THRESH elements in size
	if (((pe - ps) / es) <= __SELECT_THRESH) {
		for (ps = sps; ps < epe; ps += es) {
			register char *smp;	// Smallest Pointer
			for (smp = ps, pe = ps + es; pe <= epe; pe += es) {
				if (getkey(pe) < getkey(smp)) {
					smp = pe;
				}
			}
			if (smp != ps) {
				swap(smp, ps, es);
			}
		}
		return;
	}

	// Radix sort the current partition
	for (;;) {
		// Sort based upon the current radix mask
		while (!(getkey(ps) & mask))
			if ((ps += es) == pe) goto _rqsort_inner_stop;
		while (getkey(pe) & mask)
			if ((pe -= es) == ps) goto _rqsort_inner_stop;

		swap (ps, pe, es);

		if ((ps += es) == pe) break;
		if ((pe -= es) == ps) break;
	}

_rqsort_inner_stop:

	// Return now if this partition is fully sorted
	if (msb == 0) {
		return;
	}
	msb--;

	// ps == pe.  Adjust their position according to what they're pointing at
	if (getkey(ps) & mask)  {
		if (ps > sps) ps -= es;
	} else {
		if (pe < epe) pe += es;
	}

	// Only do left partition if there's 2 or more elements
	if (ps > sps) {
		_qrsort(sps, ps, es, getkey, msb);
	}

	// Only do right partition if there's 2 or more elements
	if (pe < epe) {
		_qrsort(pe, epe, es, getkey, msb);
	}
} // _qrsort
#endif

void
qrsort(char *a, size_t n, size_t es, uint32_t (*getkey)())
{
	// Sanity check our parameters
	if ((a == NULL) || (n < 2) || (es < 1) || (getkey == NULL)) {
		return;
	}

	_qrsort(a, a + (es * (n - 1)), es, getkey, 31);
} // qrsort

#undef __SELECT_THRESH
#undef __ITERATIVE
#undef __do_swap
#undef swap
