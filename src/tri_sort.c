#include <stddef.h>
#include <stdint.h>
#include "swap.h"


void tri_sort(register char *a, size_t n, register const size_t es, register const int (*is_lt)(const void *, const void *))
{
	register char	*b, *c, *d, *s, *e = a + n * es;
	register size_t step;
	register int    swaptype;
	register WORD	t;
	char		tmp[es];
	double		factor = 1.638;

	if (n < 2)
		return;

	SWAPINIT(a, es);

	for (step = n / 3; step > 1; step = step / factor) {
		for (s = a + step * es, b = a, c = s; b < s; b+=es, c+=es) {
			if (is_lt(c, b)) {
				swap(b, c);
			}
		}

		for (c = s, d = s + step * es; d < e; c+=es, d+=es) {
			if (is_lt(d, c)) {
				b = c - step * es;
				swap(c, d);
				if (is_lt(c, b)) {
					swap(b, c);
				}
			}
		}

		if ((step = step / factor) < 2) {
			break;
		}

		for (s = e - (step + 1) * es, d = e - es, c = s; d > s; c-=es, d-=es) {
			if (is_lt(d, c)) {
				swap(c, d)
			}
		}

		for (b = s - step * es, c = s, s = a + (step - 1) * es; c > s; b-=es, c-=es) {
			if (is_lt(c, b)) {
				d = c + step * es;
				swap(b, c);
				if (is_lt(d, c)) {
					swap(c, d);
				}
			}
		}
	}

	// At this point everything should be <10 positions of where it needs to be,
	// and most typically within 0-2 positions.  It so happens that insertion sort
	// is actually very efficient for sorting such really quickly, so we do that
	for (s=a, b=a+es; b<e; s=b, b+=es)
		for (c=b; c>a && is_lt(c, s); c=s, s-=es)
			swap(c, s);
} // tri_sort
