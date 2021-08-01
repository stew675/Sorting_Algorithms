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
						/* p4 < p2 < p1 < p3 */	\
						swap(p1, p3);		\
						swap(p1, p4);		\
						swp = true;		\
					} else {			\
						/* p2 < p4 < p1 < p3 */	\
						swap(p1, p3);		\
						swap(p1, p4);		\
						swap(p1, p2);		\
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
