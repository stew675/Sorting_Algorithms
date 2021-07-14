# Details

Copyright: stew675@gmail.com

Author: stew675@gmail.com

Date: 11 July 2021

Email stew675@gmail.com for permission to use for commercial purposes

Free for use for educational purposes

# Introduction

**ts** is a sorting testing framework program designed to generate repeatable data sets of abitrary sizes
which may be given to a selected sorting algorithm to sort.  The result is tested for correctness.
The time taken to complete the sort is reporting down to a nanosecond resolution.

# QuickRadixSort

Time Complexity: *Expected* **O(kn)** *Observed* **O(n logn)**
Space Complexity: **O(k)**  where *k* is the highest bit set in all keys

I whipped this up in day I spent doodling about, refreshing self on implementing radix sorts and then deciding that
a binary radix sort was the best implementation for such.  Using 32-bit keys only, I then set about seeing if
I could make it run faster than glibc qsort for which I succeeded.  It could be expanded to use 64-bit keys easily.

The only drawback to implementing a pure binary radix sort is that the sort becomes unstable (meaning that the
original order of elements with equal keys is not preserved).

Just for fun I implemented both recursive and interative variants although they're near identical in practise.

For sorting unsigned 32-bit integers, the interative variant appears to be uniformly faster than glibc qsort by 20%
regardless of the array size used. The recursive variant appears to 2-5% slower than the iterative variant.
Of course like all radix sorts it has the restriction that keys must be of a fixed width, in this case being an unsigned 32-bit width.

It could be expanded to handle 64 bit keys fairly easily

# New QuickSort

Time Complexity:  *Average* **O(n logn)** *Worst Case* **O(n^2)**
Space Complexity: **O(logn)**

Copied wholesale (with some minor modernisation) from: https://cs.fit.edu/~pkc/classes/writing/papers/bentley93engineering.pdf
All rights and kudos go to those authors.  This is the classic recursive median-of-3 qsort implementation
that forms the basic of many modern qsort implementations

# glibc qsort

Time Complexity: *Average* **O(n logn)** *Worst Case* **O(n^2)**
Space Complexity: **O(k)**   where *k* is number of bits in *size_t*

Source can be found here: https://code.woboq.org/userspace/glibc/stdlib/qsort.c.html

glibc *qsort* is derived from New Quicksort above, but modified to be wholly iterative, with a fixed space complexity.
Personally I found it to consistently run slower than the original paper's algorithm that it is based upon.

# rattle sort

Time Complexity: *Observed* **O(n logn)** *Worst Case* **O(n^2)**
Space Complexity: **0**   No extra space is needed, all operational variables are assigned to registers

I wouldn't even want to try to mathematically analyse the time complexity of this dark horse.  It's basically
a bi-directional bubble-sort but with (initially) large offsets that decrease in size with each pass down to a
true bubble sort.  In observable practise it appears to follow a near perfect **O(n logn)** time complexity
progression.  It is a remarkably simple algorithm, that follows a logarithmic progression to reducing the
step sizes for each pass, such that by the time the step sizes reach 1 (effectively acting as a pure bubble sort)
that very few (typically 1 to 4) 1-step passes are needed to complete the sorting of the data.

It is unstable (in sorting terms)

It is wholly in-place, iterative, and with zero space overhead

For arrays of 1000 elements or less, it is typically faster than *glibc qsort* but for data sets of 10K+ it is
almost consistently 20-25% slower than *glibc qsort*

# Sample Runtimes

A stock-clocked AMD Ryzen 9 5950X was used.  Memory set to 3600MHz at 14-15-14-14 1T CMD timings.

```
./ts -qr 50000000
Populating array
Using quick-radix-sort
Time taken : 4.392748111s


./ts -nq 50000000
Populating array
Using new quick sort
Time taken : 5.054120625s


./ts -qs 50000000
Populating array
Using glibc qsort
Time taken : 5.479669791s


./ts -rs 50000000
Populating array
Using rattle sort
Time taken : 6.826960173s
```
