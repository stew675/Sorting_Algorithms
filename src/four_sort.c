#include <stddef.h>
#include <stdbool.h>
#include "swap.h"

typedef const int (*ilt)(const void *, const void *);

#define sort2(p1, p2, is_lt, swp)	\
	if (is_lt(p2, p1)) {		\
		swap(p1, p2);		\
		swp = true;		\
	}				\

#define sort3(p1, p2, p3, is_lt, swp)			\
{							\
	if (is_lt(p3, p2)) {				\
		/* p3 < p2 */				\
		if (is_lt(p2, p1)) {			\
			/* p3 < p2 < p1 */		\
			swap(p1, p3);			\
			swp = true;			\
		} else {				\
			/* p1/p3 < p2 */		\
			if (is_lt(p3, p1)) {		\
				/* p3 < p1 < p2 */	\
				swap(p1, p2);		\
				swap(p1, p3);		\
				swp = true;		\
			} else {			\
				/* p1 < p3 < p2 */	\
				swap(p2, p3);		\
				swp = true;		\
			}				\
		}					\
	} else {					\
		/* p2 < p3 */				\
		if (is_lt(p2, p1)) {			\
			/* p2 < p1/p3 */		\
			if (is_lt(p3, p1)) {		\
				/* p2 < p3 < p1 */	\
				swap(p1, p3);		\
				swap(p1, p2);		\
				swp = true;		\
			} else {			\
				/* p2 < p1 < p3 */	\
				swap(p1, p2);		\
				swp = true;		\
			}				\
		} else {				\
			/* p1 < p2 < p3 */		\
		}					\
	}						\
}

#define sort4(p1, p2, p3, p4, is_lt, swp)				\
	if (is_lt(p3, p2)) {						\
		/* p3 < p2 */						\
		if (is_lt(p4, p3)) {					\
			/* p4 < p3 < p2	*/				\
			if (is_lt(p3, p1)) {				\
				/* p4 < p3 < p1/p2 */			\
				if (is_lt(p2, p1)) {			\
					/* p4 < p3 < p2 < p1 */		\
					swap(p1, p4);			\
					swap(p2, p3);			\
					swp = true;			\
				} else {				\
					/* p4 < p3 < p1 < p2 */		\
					swap(p1, p4);			\
					swap(p2, p4);			\
					swap(p2, p3);			\
					swp = true;			\
				}					\
			} else {					\
				/* p1/p4 < p3 < p2 */			\
				if (is_lt(p4, p1)) {			\
					/* p4 < p1 < p3 < p2 */		\
					swap(p1, p2);			\
					swap(p1, p4);			\
					swp = true;			\
				} else {				\
					/* p1 < p4 < p3 < p2 */		\
					swap(p2, p4);			\
					swp = true;			\
				}					\
			}						\
		} else {						\
			/* p3 < p2/p4 */				\
			if (is_lt(p4, p2)) {				\
				/* p3 < p4 < p2 */			\
				if (is_lt(p4, p1)) {			\
					/* p3 < p4 < p1/p2 */		\
					if (is_lt(p2, p1)) {		\
						/* p3 < p4 < p2 < p1 */	\
						swap(p1, p3);		\
						swap(p2, p3);		\
						swap(p2, p4);		\
						swp = true;		\
					} else {			\
						/* p3 < p4 < p1 < p2 */	\
						swap(p1, p3);		\
						swap(p2, p4);		\
						swp = true;		\
					}				\
				} else {				\
					/* p1/p3 < p4 < p2 */		\
					if (is_lt(p3, p1)) {		\
						/* p3 < p1 < p4 < p2 */	\
						swap(p1, p3);		\
						swap(p2, p4);		\
						swap(p2, p3);		\
						swp = true;		\
					} else {			\
						/* p1 < p3 < p4 < p2 */	\
						swap(p3, p2);		\
						swap(p3, p4);		\
						swp = true;		\
					}				\
				}					\
			} else {					\
				/* p3 < p2 < p4 */			\
				if (is_lt(p2, p1)) {			\
					/* p3 < p2 < p1/p4 */		\
					if (is_lt(p4, p1)) {		\
						/* p3 < p2 < p4 < p1 */	\
						swap(p3, p1);		\
						swap(p3, p4);		\
						swp = true;		\
					} else {			\
						/* p3 < p2 < p1 < p4 */	\
						swap(p1, p3);		\
						swp = true;		\
					}				\
				} else {				\
					/* p1/p3 < p2 < p4 */		\
					if (is_lt(p3, p1)) {		\
						/* p3 < p1 < p2 < p4 */	\
						swap(p1, p2);		\
						swap(p1, p3);		\
						swp = true;		\
					} else {			\
						/* p1 < p3 < p2 < p4 */	\
						swap(p2, p3);		\
						swp = true;		\
					}				\
				}					\
			}						\
		}							\
	} else {							\
		/* p2 < p3 */						\
		if (is_lt(p2, p1)) {					\
			/* p2 < p1/p3 */				\
			if (is_lt(p3, p1)) {				\
				/* p2 < p3 < p1 */			\
				if (is_lt(p4, p3)) {			\
					/* p2/p4 < p3 < p1 */		\
					if (is_lt(p4, p2)) {		\
						/* p4 < p2 < p3 < p1 */	\
						swap(p1, p4);		\
						swp = true;		\
					} else {			\
						/* p2 < p4 < p3 < p1 */	\
						swap(p1, p4);		\
						swap(p1, p2);		\
						swp = true;		\
					}				\
				} else {				\
					/* p2 < p3 < p1/p4 */		\
					if (is_lt(p4, p1)) {		\
						/* p2 < p3 < p4 < p1 */	\
						swap(p1, p2);		\
						swap(p2, p4);		\
						swap(p2, p3);		\
						swp = true;		\
					} else {			\
						/* p2 < p3 < p1 < p4 */	\
						swap(p1, p3);		\
						swap(p1, p2);		\
						swp = true;		\
					}				\
				}					\
			} else {					\
				/* p2 < p1 < p3 */			\
				if (is_lt(p4, p1)) {			\
					/* p2/p4 < p1 < p3 */		\
					if (is_lt(p4, p2)) {		\
						/* p2 < p4 < p1 < p3 */	\
						swap(p1, p3);		\
						swap(p1, p4);		\
						swap(p1, p2);		\
						swp = true;		\
					} else {			\
						/* p4 < p2 < p1 < p3 */	\
						swap(p1, p3);		\
						swap(p1, p4);		\
						swp = true;		\
					}				\
				} else {				\
					/* p2 < p1 < p3/p4 */		\
					if (is_lt(p4, p3)) {		\
						/* p2 < p1 < p4 < p3 */	\
						swap(p1, p2);		\
						swap(p3, p4);		\
						swp = true;		\
					} else {			\
						/* p2 < p1 < p3 < p4 */	\
						swap(p1, p2);		\
						swp = true;		\
					}				\
				}					\
			}						\
		} else {						\
			/* p1 < p2 < p3 */				\
			if (is_lt(p4, p2)) {				\
				/* p1/p4 < p2 < p3 */			\
				if (is_lt(p4, p1)) {			\
					/* p4 < p1 < p2 < p3 */		\
					swap(p1, p2);			\
					swap(p1, p4);			\
					swap(p3, p4);			\
					swp = true;			\
				} else {				\
					/* p1 < p4 < p2 < p3 */		\
					swap(p2, p3);			\
					swap(p2, p4);			\
					swp = true;			\
				}					\
			} else {					\
				/* p1 < p2 < p3/p4 */			\
				if (is_lt(p4, p3)) {			\
					/* p1 < p2 < p4 < p3 */		\
					swap(p3, p4);			\
					swp = true;			\
				} else {				\
					/* p1 < p2 < p3 < p4 */		\
					/* IS SORTED - DO NOTHING */	\
				}					\
			}						\
		}							\
	}

static void
four_sort1(register char *a, size_t n, register const size_t es, register ilt is_less_than, register int swaptype)
{
        register char *a1, *a2, *a3, *a4;
        register char *e = a+n*es;
        register WORD t;
	bool swapped;

        for (a1=a; a1<e; a1+=(4*es)) {
                a2 = a1 + es;
                a3 = a1 + (es << 1);
                a4 = a1 + (es * 3);
		sort4(a1, a2, a3, a4, is_less_than, swapped);
	}
} // four_sort1


static void
four_sort2(register char *a, size_t n, register const size_t es, register ilt is_less_than, register int swaptype)
{
        register char *s, *p, *v, *u;
        register char *e = a+n*es;
        register WORD t;

        for (s=a+4*es; a<e; a=s, s+=4*es)
                for (p=a+es; p<s; p+=es)
                        for (v=p, u=p-es; v>a && is_less_than(v, u); v=u, u-=es)
                                swap(v, u);
} // four_sort


static void
eight_sort(register char *a, size_t n, register const size_t es, register ilt is_less_than, register int swaptype)
{
        register char *s, *p, *v, *u;
        register char *e = a+n*es;
        register WORD t;

        for (s=a+8*es; a<e; a=s, s+=8*es)
                for (p=a+es; p<s; p+=es)
                        for (v=p, u=p-es; v>a && is_less_than(v, u); v=u, u-=es)
                                swap(v, u);
} // four_sort


void
four_sort(register char *a, size_t n, register const size_t es, register ilt is_less_than)
{
        register char *e = a+(n-4)*es;
        int swaptype;

        SWAPINIT(a, es);

        // Initial pass
        four_sort1(a, n, es, is_less_than, swaptype);

#ifdef SORT1
        for (char *p=a+2*es; p<e; p+=8*es)
                four_sort1(p, 4, es, is_less_than, swaptype);
        four_sort1(a, n, es, is_less_than, swaptype);
        for (char *p=a+2*es; p<e; p+=8*es)
                four_sort1(p, 4, es, is_less_than, swaptype);
// #else
        eight_sort(a, n, es, is_less_than, swaptype);
#endif
} // four_sort
