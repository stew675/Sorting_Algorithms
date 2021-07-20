#include <stddef.h>
#include "oldswap.h"

// 701..1 is the Marcin Ciura gap sequence
// Gaps preceding 701 follow gap[n] = gap[n+1] * 7 / 3
//static size_t gaps[] = {8906, 3817, 1636, 701, 301, 132, 57, 23, 10, 4, 1};

//static size_t gaps[] = {772019, 330859, 141803, 60773, 26041, 11161, 4783, 2053, 877, 379, 71, 29, 13, 5, 2, 1};
//static size_t gaps[] = {567209, 243091, 104179, 44647, 19139, 8209, 3517, 1511, 647, 277, 113, 53, 23, 11, 4, 1};
static size_t gaps[] = {919189, 344719, 129263, 48479, 18181, 6823, 2557, 953, 359, 137, 53, 19, 7, 2, 1};
//953, 2557, 6823, 18181, 48479, 129263, 344719, 919189

// Sort an array a[0...n-1].
void
shell_sort(char *a, size_t n, register const size_t es, register const int (*cmp)(const void *, const void *))
{
	register char *b, *c, *d, *e = a + n * es, *s;	// e points at the end of the array
	register size_t gap;
	register int swaptype;
	char temp[es];

	SWAPINIT(a, es);
	
	// Start with the largest gap and work down to a gap of 1
	for (int pos = 0; pos < (sizeof(gaps) / sizeof(*gaps)); pos++) {
		for (gap = gaps[pos] * es, s = a + gap, b = s, d = s - gap; b < e; b += es, d = b - gap) {
			if (cmp(d, b) > 0) {
				copy(temp, b);
				copy(b, d);
				for (c = d, d -= gap; (c >= s) && (cmp(d, temp) > 0); c = d, d -= gap) {
					copy(c, d);
				}
				copy(c, temp);
			}
		}
	}
} // shell_sort
