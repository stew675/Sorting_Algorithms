static void
four_sort(register char *a, size_t n, register const size_t es, register ilt is_less_than)
{
	char *a1, *a2, *a3, *a4;
	char *e = a+n*es;

	for (a1=a; a1<e; a1+=(4*es)) {
		if (is_less_than(a3, a2)) {
			// a3 < a2
			if (is_less_than(a4, a3)) {
				// a4 < a3 < a2
				if (is_less_than(a3, a1)) {
					// a4 < a3 < a1/a2
					if (is_less_than(a2, a1)) {
						// a4 < a3 < a2 < a1
						swap(a1, a4);
						swap(a2, a3);
					} else {
						// a4 < a3 < a1 < a2
						swap(a1, a4);
						swap(a2, a4);
						swap(a2, a3);
					}
				} else {
					// a1/a4 < a3 < a2
					if (is_less_than(a4, a1)) {
						// a4 < a1 < a3 < a2
						swap(a1, a2);
						swap(a1, a4);
					} else {
						// a1 < a4 < a3 < a2
						swap(a2, a4);
					}
				}
			} else {
				// a3 < a2/a4
				if (is_less_than(a4, a2)) {
					// a3 < a4 < a2
					if (is_less_than(a4, a1)) {
						// a3 < a4 < a1/a2
						if (is_less_than(a2, a1)) {
							// a3 < a4 < a2 < a1
							swap(a1, a3);
							swap(a2, a3);
							swap(a2, a4);
						} else {
							// a3 < a4 < a1 < a2
							swap(a1, a3);
							swap(a2, a4);
						}
					} else {
						// a1/a3 < a4 < a2
						if (is_less_than(a3, a1)) {
							// a3 < a1 < a4 < a2
							swap(a1, a3);
							swap(a2, a4);
							swap(a2, a3);
						} else {
							// a1 < a3 < a4 < a2
							swap(a3, a2);
							swap(a3, a4);
						}
					}
				} else {
					// a3 < a2 < a4
					if (is_less_than(a2, a1)) {
						// a3 < a2 < a1/a4
						if (is_less_than(a4, a1)) {
							// a3 < a2 < a4 < a1
							swap(a3, a1);
							swap(a3, a4);
						} else {
							// a3 < a2 < a1 < a4
							swap(a1, a3);
						}
					} else {
						// a1/a3 < a2 < a4
						if (is_less_than(a3, a1)) {
							// a3 < a1 < a2 < a4
							swap(a1, a2);
							swap(a1, a3);
						} else {
							// a1 < a3 < a2 < a4
							swap(a2, a3);
						}
					}
				}
			}
		} else {
			// a2 < a3
			if (is_less_than(a4, a2)) {
				// a4 < a2 < a3
				if (is_less_than(a2, a1)) {
					// a4 < a2 < a1/a3
					if (is_less_than(a3, a1)) {
						// a4 < a2 < a3 < a1
						swap(a1, a4);
					} else {
						// a4 < a2 < a1 < a3
						swap(a1, a3);
						swap(a1, a4);
					}
				} else {
					// a1/a4 < a2 < a3
					if (is_less_than(a4, a1)) {
						// a4 < a1 < a2 < a3
						swap(a1, a2);
						swap(a1, a4);
						swap(a3, a4);
					} else {
						// a1 < a4 < a2 < a3
						swap(a2, a4);
						swap(a3, a4);
					}
				}
			} else {
				// a2 < a3/a4
				if (is_less_than(a4, a3)) {
					// a2 < a4 < a3
					if (is_less_than(a4, a1)) {
						// a2 < a4 < a1/a3
						if (is_less_than(a3, a1)) {
							// a2 < a4 < a3 < a1
							swap(a1, a4);
							swap(a1, a2);
						} else {
							// a2 < a4 < a1 < a3
							swap(a1, a3);
							swap(a1, a4);
							swap(a1, a2);
						}
					} else {
						// a1/a2 < a4 < a3
						if (is_less_than(a2, a1)) {
							// a2 < a1 < a4 < a3
							swap(a1, a2);
							swap(a3, a4);
						} else {
							// a1 < a2 < a4 < a3
							swap(a3, a4);
						}
					}
				} else {
					// a2 < a3 < a4
					if (is_less_than(a3, a1)) {
						// a2 < a3 < a1/a4
						if (is_less_than(a4, a1)) {
							// a2 < a3 < a4 < a1
							swap(a1, a2);
							swap(a2, a3);
							swap(a3, a4);
						} else {
							// a2 < a3 < a1 < a4
							swap(a1, a3);
							swap(a1, a2);
						}
					} else {
						// a1/a2 < a3 < a4
						if (is_less_than(a2, a1)) {
							// a2 < a1 < a3 < a4
							swap(a1, a2);
						} else {
							// a1 < a2 < a3 < a4
						}
					}
				}
			}
		}
	}
} // four_sort

