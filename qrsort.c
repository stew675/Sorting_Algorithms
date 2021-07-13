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

#include <stddef.h>
#include <stdint.h>
#include "swap.h"

#define __SELECT_THRESH	 7

// Iterative is faster than recursive by about 2%.  Not a huge difference, but it is consistent
#define __ITERATIVE

#ifdef __ITERATIVE

struct stack_node {
	char	 *ps;
	char	 *pe;
	uint32_t msb;
};

// We only add to the stack when absolutely necessary, otherwise we try to remain iterative where possible
static void
_qrsort(register char *ps, register char *pe, register const size_t es, register const uint32_t (*getkey)(), uint32_t msb)
{
	register uint32_t mask, stkpos = 0;
	register char *sps, *epe;
	register int swaptype;
	register WORD t;
	struct stack_node stk[32];

	SWAPINIT(ps, es);

	// We could push our starting parameters onto the stack, and pop them off again, OR we can just get straight to it!
	goto _qrsort_restart;

	while (stkpos > 0) {
		// Pop our loop parameters from the stack
		--stkpos;
		ps = stk[stkpos].ps;
		pe = stk[stkpos].pe;
		msb = stk[stkpos].msb;

_qrsort_restart:	// Allows us to restart the loop without popping values from the stack
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
					swap(smp, ps);
				}
			}
			continue;
		}

		// Radix sort the current partition
		mask = ((uint32_t)1) << msb;
		for (;;) {
			// Sort based upon the current radix mask
			while (!(getkey(ps) & mask))
				if ((ps += es) == pe) goto _qrsort_stop_inner;
			while (getkey(pe) & mask)
				if ((pe -= es) == ps) goto _qrsort_stop_inner;

			swap (ps, pe);

			if ((ps += es) == pe) break;
			if ((pe -= es) == ps) break;
		}

_qrsort_stop_inner:
		// Pop off the next item on stack if this partition is now fully sorted
		if (msb == 0) { continue; }
		msb--;

		// ps == pe so adjust their position according to what they're pointing at
		if (getkey(ps) & mask)  {
			if (ps > sps) ps -= es;
		} else {
			if (pe < epe) pe += es;
		}

		if (pe < epe) {			// Only do right partition if there's 2 or more elements
			if (ps > sps) {		// Only do left partition if there's 2 or more elements
				stk[stkpos].ps = pe;
				stk[stkpos].pe = epe;
				stk[stkpos].msb = msb;
				stkpos++;

				// _rqsort(sps, ps, es, getkey, scanbits, msb);
				pe = ps;
				ps = sps;
			} else {
				// _rqsort(pe, epe, es, getkey, scanbits, msb);
				ps = pe;
				pe = epe;
			}
			goto _qrsort_restart;
		} else if (ps > sps) {		// Only do left partition if there's 2 or more elements
			// _rqsort(sps, ps, es, getkey, scanbits, msb);
			pe = ps;
			ps = sps;
			goto _qrsort_restart;
		}
	}
} // _qrsort

#else

static void
_qrsort(register char *ps, register char *pe, register const size_t es, register const uint32_t (*getkey)(), uint32_t msb)
{
	register uint32_t mask;
	register char *sps, *epe;
	register int swaptype;
	register WORD t;

	SWAPINIT(ps, es);

_qrsort_restart:	// Allows us to restart the loop without popping values from the stack
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
				swap(smp, ps);
			}
		}
		return;
	}

	// Radix sort the current partition
	mask = ((uint32_t)1) << msb;
	for (;;) {
		// Sort based upon the current radix mask
		while (!(getkey(ps) & mask))
			if ((ps += es) == pe) goto _qrsort_stop_inner;
		while (getkey(pe) & mask)
			if ((pe -= es) == ps) goto _qrsort_stop_inner;

		swap (ps, pe);

		if ((ps += es) == pe) break;
		if ((pe -= es) == ps) break;
	}

_qrsort_stop_inner:
	// Return now if this partition is fully sorted
	if (msb == 0) { return; }
	msb--;

	// ps == pe so adjust their position according to what they're pointing at
	if (getkey(ps) & mask)  {
		if (ps > sps) ps -= es;
	} else {
		if (pe < epe) pe += es;
	}

	if (ps > sps) {			// Only do left partition if there's 2 or more elements
		if (pe < epe) {		// Only do right partition if there's 2 or more elements
			_qrsort(sps, ps, es, getkey, msb);
			ps = pe;
			pe = epe;
		} else {
			pe = ps;
			ps = sps;
		}
		goto _qrsort_restart;
	} else if (pe < epe) {		// Only do right partition if there's 2 or more elements
		ps = pe;
		pe = epe;
		goto _qrsort_restart;
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
