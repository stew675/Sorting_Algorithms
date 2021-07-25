#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "newswap.h"

// Macros for bit operations on an unsigned 32-bit integer bit array
#define setbit(R,K)	  ( *(R+((K)>>5)) |=  (1<<((K)&0x1f)) )
#define clrbit(R,K)	  ( *(R+((K)>>5)) &= ~(1<<((K)&0x1f)) )
#define tstbit(R,K)	!!( *(R+((K)>>5)) &   (1<<((K)&0x1f)) )
#define flpbit(R,K)	  ( *(R+((K)>>5)) ^=  (1<<((K)&0x1f)) )

void
weak_heap(register char *a, register size_t n, register size_t es, register const int (*is_less_than)(const void *, const void *))
{
	register size_t k;
	register uint32_t *r;

	if (n < 2)
		return;

	// Allocate up a fully zeroed node marking registry
	if ((r = calloc(sizeof(*r), ((n-1)/(sizeof(*r)*8))+1)) == NULL)
		return;

	// Weak heapify the array
	for (k = n - 1; k > 0; k--) {
		register size_t i;
		// Find ancestor of k
		for (i = k; (i & 1) == tstbit(r, i>>1); i>>=1);
		i >>= 1;
		// Swap as needed
		if (is_less_than(a+i*es, a+k*es)) {
			swap(a+i*es, a+k*es, es);
			flpbit(r, k);
		}
	}

	// Now extract the greatest repeatedly
	for (register char *e = a + (n-1)*es; e > a; e-=es) {
		swap(a, e, es);

		if (n-- < 3)
			continue;

		// Sift down to restore weak heap ordering
		for (k = !tstbit(r, 0); (k + k + tstbit(r, k)) < n; k += k + tstbit(r, k));
		for (; k; k>>=1) {
			if (is_less_than(a, a+k*es)) {
				swap(a, a+k*es, es);
				flpbit(r, k);
			}
		}
	}

	// Release the node marking registry
	free(r);
} // weak_heap
