# QuickRadixSort

Copyright: stew675@gmail.com

Author: stew675@gmail.com

Date: 11 July 2021

Email stew675@gmail.com for permission to use for commercial purposes

Free for use for educational purposes

Just a day spent doodling about, refreshing self on implementing radix sorts and then seeing if
I could make it run faster than glibc qsort (which it is, with certain restrictions)

Just for fun I implemented both recursive and interative variants.

For sorting unsigned 32-bit integers, the interative variant appears to be uniformly faster than glibc qsort by 10-15%
regardless of the array size used. The recursive variant appears to 2-5% slower than the iterative variant.
Of course like all radix sorts it has the restriction that keys must be of a fixed width, in this case being an unsigned 32-bit width.

It could expanded to 64 bit keys fairly easily

