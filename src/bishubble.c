#include <stddef.h>
#include "oldswap.h"

// The following sequence is calculated from a running exponent of (8/3)^n, n=1..18, and then choosing the closest prime number to each value
// Appears to match or outperform the Marcin Ciura sequence for all array sizes
static size_t steps[] = {46498303, 17436869, 6538817, 2452057, 919519, 344821, 129313, 48491, 18181, 6823, 2557, 953, 359, 137, 53, 19, 7, 2, 1, 0};

// Sort an array a[0...n-1].
void
bishubble_sort(register char *a, size_t n, const size_t es, register const int (*cmp)(const void *, const void *))
{
	register char	*b, *c, *s;
	register int	swaptype;
	register size_t step, i;
	int	pos;
	char	temp[es];

	SWAPINIT(a, es);

	// Start with the largest usable step and work down to a step of 1
	for (pos = 0; n <= steps[pos]; pos++);
	for (step = steps[pos]; step > 0; step = steps[++pos])
		for (i = 0; (i < step) && ((step + i) < n); i++) {
			char *sa = a;	// Remember location of a

			a += (i*es);
			step *= es;
			for (register char *e = a + (n - i) * es;;) {
				for (b=a, c=a+step, s=a; c<e; b+=step, c+=step)
					if (cmp(b, c) > 0) { swap(b, c); a = c; }
				if (s == a) break;
				e = a; a = s;

				for (b=e-step, c=b-step, s=e; c>=a; b-=step, c-=step)
					if (cmp(b, c) < 0) { swap(b, c); e = c; }
				if (s == e) break;
				a = e; e = s;
			}
			step /= es;	// Restore step
			a = sa;		// Restore a
		}
} // shell_sort

