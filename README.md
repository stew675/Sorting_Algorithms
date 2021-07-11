# QuickRadixSort

Copyright: stew675@gmail.com

Author: stew675@gmail.com

Date: 11 July 2021

Email stew675@gmail.com for permission to use for commercial purposes

Free for use for educational purposes

Just a day spent doodling about, refreshing self on implementing radix sorts and then seeing if
I could make it run faster than glibc qsort (which it is, within certain restrictions)

Just for fun I implemented both recursive and interative variants.

Generally faster than glibc qsort by around 10% or so when sorting large arrays of unsigned 32-bit integers
Of course it has the restriction that keys must be of fixed unsigned 32-bit width
It could expanded to 64 bit keys fairly easily

