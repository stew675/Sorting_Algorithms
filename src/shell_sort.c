#include <stddef.h>
#include "oldswap.h"

// 701..1 is the Marcin Ciura gap sequence
// Gaps preceding 701 follow gap[n] = gap[n+1] * 7 / 3
//static size_t gaps[] = {8906, 3817, 1636, 701, 301, 132, 57, 23, 10, 4, 1};

// The following sequence is calculated from a running exponent of (8/3)^n, n=1..18, and then choosing the closest prime number to each value
// Appears to match or outperform the Marcin Ciura sequence for all array sizes
static size_t gaps[] = {46498303, 17436869, 6538817, 2452057, 919519, 344821, 129313, 48491, 18181, 6823, 2557, 953, 359, 137, 53, 19, 7, 2, 1, 0};

// Sort an array a[0...n-1].
void
shell_sort(char *a, size_t n, register const size_t es, register const int (*is_less_than)(const void *, const void *))
{
	register char *b, *c, *d, *e = a + n * es, *s;	// e points at the end of the array
	register size_t gap;
	register int swaptype, pos;
	char temp[es];

	SWAPINIT(a, es);
	
	// Start with the largest usable gap and work down to a gap of 1
	for (pos = 0; n < gaps[pos]; pos++);
	for (gap = gaps[pos]; gap > 0; gap = gaps[++pos]) {
		for (gap *= es, s = a + gap, b = s, d = s - gap; b < e; b += es, d = b - gap) {
			if (is_less_than(b, d)) {
				copy(temp, b);
				copy(b, d);
				for (c = d, d -= gap; (c >= s) && is_less_than(temp, d); c = d, d -= gap) {
					copy(c, d);
				}
				copy(c, temp);
			}
		}
	}
} // shell_sort
