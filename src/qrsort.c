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

extern uint64_t numcmps;

// Partition size thresh-hold where we do insertion sorting instead
// Extensive empirical testing arrived at the below value which I
// admit does seem particularly high, but it is what it is
#define __INSERT_THRESH	 20

// Iterative is faster on some architectures, slower on others.  Recursive is a safe bet all
// around, but for absolute speed, choose iterative if it benefits your situation specifically
//#define __ITERATIVE

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
	register char *sps, *spe;	// Saved Partition Start/End
	register int swaptype;
	register WORD t;
	struct stack_node stk[msb];

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

		// Do selection sort on partitions <= __SELECT_THRESH elements in size
		if (((pe - ps) / es) <= __INSERT_THRESH) {
			for (sps=ps+es; sps <= pe; sps+=es) {
				for(spe=sps; (spe>ps) && (getkey(spe) < getkey(spe-es)); spe-=es)
					swap(spe, spe-es);
			}
			continue;
		}

		sps = ps;
		spe = pe;

		// Radix sort the current partition
		mask = ((uint32_t)1) << msb;
		for (;;) {
			// Sort based upon the current radix mask
			while (!(getkey(ps) & mask))
				if ((ps += es) == pe) goto _qrsort_stop_inner;
			while (getkey(pe) & mask)
				if ((pe -= es) == ps) goto _qrsort_stop_inner;

			swap(ps, pe);

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
			if (pe < spe) pe += es;
		}

		if (pe < spe) {			// Only do right partition if there's 2 or more elements
			if (ps > sps) {		// Only do left partition if there's 2 or more elements
				stk[stkpos].ps = pe;
				stk[stkpos].pe = spe;
				stk[stkpos].msb = msb;
				stkpos++;

				// _rqsort(sps, ps, es, getkey, scanbits, msb);
				pe = ps;
				ps = sps;
			} else {
				// _rqsort(pe, spe, es, getkey, scanbits, msb);
				ps = pe;
				pe = spe;
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
_qrsort(register char *ps, register char *pe, register const size_t es, register const uint32_t (*getkey)(void *), uint32_t msb)
{
	register uint32_t mask;
	register char *sps, *spe;	// Saved Partition Start/End
	register int swaptype;
	register WORD t;

_qrsort_restart:	// Allows us to restart the loop without popping values from the stack

	// Do selection sort on partitions <= __SELECT_THRESH elements in size
	if (((pe - ps) / es) <= __INSERT_THRESH) {
		for (sps=ps+es; sps <= pe; sps+=es) {
			for(spe=sps; (spe>ps) && (getkey(spe) < getkey(spe-es)); spe-=es)
				swap(spe, spe-es);
		}
		return;
	}

	sps = ps;
	spe = pe;

	// Radix sort the current partition
	mask = ((uint32_t)1) << msb;
	for (;;) {
		// Sort based upon the current radix mask
		while (!(getkey(ps) & mask))
			if ((ps += es) == pe) goto _qrsort_stop_inner;
		while (getkey(pe) & mask)
			if ((pe -= es) == ps) goto _qrsort_stop_inner;

		swap(ps, pe);

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
		if (pe < spe) pe += es;
	}

	if (ps > sps) {			// Only do left partition if there's 2 or more elements
		if (pe < spe) {		// Only do right partition if there's 2 or more elements
			_qrsort(sps, ps, es, getkey, msb);
			ps = pe;
			pe = spe;
		} else {
			pe = ps;
			ps = sps;
		}
		goto _qrsort_restart;
	} else if (pe < spe) {		// Only do right partition if there's 2 or more elements
		ps = pe;
		pe = spe;
		goto _qrsort_restart;
	}
} // _qrsort
#endif


static const uint32_t deBruijnMagic = 0x06EB14F9;
static const uint8_t deBruijnTable[32] = {
	0,  1, 16,  2, 29, 17,  3, 22, 30, 20, 18, 11, 13,  4,  7, 23,
	31, 15, 28, 21, 19, 10, 12,  6, 14, 27,  9,  5, 26,  8, 25, 24,
};

static uint8_t
getmsb(uint32_t v)
{
	// The following leaves us with just the highest bit set
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v -= (v >> 1);

	// Return the index of the highest bit set
	return deBruijnTable[(v * deBruijnMagic) >> 27];
} // getmsb

void
qrsort(char *a, size_t n, size_t es, const uint32_t (*getkey)(void *))
{
	register char *e = a + (es * (n - 1));
	uint32_t msb;

	// Sanity check our parameters
	if ((a == NULL) || (n < 2) || (es < 1) || (getkey == NULL)) {
		return;
	}

	// Obtain a hint about the msb set of all keys in the data set
	// Only do this if n is above the insertion sort threshold
	if ((n - 1) > __INSERT_THRESH) {
		msb = 0;
		// For small values of n, save the result of the getmsb
		// calculation to reduce the overall key lookups
		// However, for large values of n, we will do, at most
		// 31 extra key lookups, and so it's faster to not save
		// the getmsb(getkey()) result
		if (n < 200) {
			register uint32_t gmsb;
			for (register char *b = a; b <= e; b += es) {
				if ((gmsb = getmsb(getkey(b))) > msb) {
					if ((msb = gmsb) == 31) {
						break;
					}
				}
			}
		} else {
			for (register char *b = a; b <= e; b += es) {
				if (getmsb(getkey(b)) > msb) {
					if ((msb = getmsb(getkey(b))) == 31) {
						break;
					}
				}
			}
		}
	} else {
		msb = 31;
	}

	_qrsort(a, e, es, getkey, msb);
} // qrsort

#ifdef __INSERT_THRESH
#undef __INSERT_THRESH
#endif

#ifdef __ITERATIVE
#undef __ITERATIVE
#endif
