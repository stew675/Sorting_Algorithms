#include <stddef.h>
#include "swap.h"

// 701..1 is the Marcin Ciura gap sequence
// Gaps preceding 701 follow gap[n] = gap[n+1] * 7 / 3
//static size_t gaps[] = {8906, 3817, 1636, 701, 301, 132, 57, 23, 10, 4, 1};

// The following sequence is calculated from a running exponent of (8/3)^n, n=1..18, and then choosing the closest prime number to each value
// Appears to match or outperform the Marcin Ciura sequence for all array sizes
static size_t gaps[] = {46498303, 17436869, 6538817, 2452057, 919519, 344821, 129313, 48491, 18181, 6823, 2557, 953, 359, 137, 53, 19, 7, 2, 1, 0};

//static size_t steps[] = {1, 3, 7, 17, 37, 89, 211, 487, 1129, 2633, 6151, 14347, 33487, 78137, UINT32_MAX};
//static size_t steps[] = {1, 2, 5, 13, 29, 71, 163, 379, 877, 2053, 4783, 11161, 26041, 60773, UINT32_MAX};
//static size_t steps[] = {1, 4, 10, 23, 57, 132, 301, 701, 1636, 3817, 8906, UINT32_MAX};	// Marcin
//static size_t steps[] = {1, 5, 11, 23, 53, 113, 277, 647, 1511, UINT32_MAX};
//static size_t steps[] = {1, 2, 7, 19, 53, 137, 359, 953, 2557, 6823, 18181, 48491, 129313, UINT32_MAX}; // (8/3)^n -> closest prime
//static size_t steps[] = {1, 3, 11, 29, 83, 241, 727, 2179, 6563, 19681, 59051, UINT32_MAX}; // 3^n -> closest prime
//static size_t steps[] = {1, 3, 11, 37, 127, 409, 1373, 4567, 15241, 50821, 169343, UINT32_MAX}; // (10/3)^n -> closest prime
//static size_t steps[] = {1, 3, 13, 47, 181, 661, 2437, 8923, 32687, 119797, UINT32_MAX}; // (11/3)^n -> closest prime
static size_t steps[] = {1, 5, 17, 67, 257, 1021, 4099, 16381, 65537, 262147, UINT32_MAX}; // 4^n -> closest prime

// Sort an array a[0...n-1].
void
shell_sort(char *a, size_t n, register const size_t es, register const int (*is_less_than)(const void *, const void *))
{
	register char *b, *c, *d, *e = a + n * es, *s;	// e points at the end of the array
	register int swaptype, pos = 0;
	register WORD t;
	char temp[es];

	SWAPINIT(a, es);
	
	register size_t gap, step = n;
#define next_step       ((step > steps[pos+1]) ? (n / steps[++pos]) : (pos > 0 ? steps[--pos] : 1))

	do {
		step = next_step;
		for (gap = step*es, s = a + gap, b = s, d = s - gap; b < e; b += es, d = b - gap) {
			if (is_less_than(b, d)) {
				copy(temp, b);
				copy(b, d);
				for (c = d, d -= gap; (c >= s) && is_less_than(temp, d); c = d, d -= gap)
					copy(c, d);
				copy(c, temp);
			}
		}
	} while (step > 1);
/*
	// Start with the largest usable gap and work down to a gap of 1
	register size_t gap;
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
*/
#undef next_step
} // shell_sort
