Memory Allocator
================

About
-----
This is an explicit memory allocator I wrote as part of an assignment for one of my CS classes.

How it works
------------
The allocator first creates a large contiguous "pool" of memory, which it then uses to store the allocations. Every time we allocate memory into our pool, we reserve space at the beginning and end of our block to be used as a header and footer, which includes information on how much space each block takes up within our entire memory pool. This then allows us to traverse forward and backward within the pool quite easily using pointer arithmetic.

When an allocation request is made, we use a best fit algorithm when to find the smallest open block that will fit our request. When a deallocation request is made, if the block is adjacent to a free memory block, the allocator will coalesce the blocks in constant time, leaving as large blocks as it can in memory. Allocation requests are denied if no open blocks are large enough for the memory. The best fit allocation and coalescing on deallocation then give us a memory utilization of around 71%.

The code itself is documented, so more specific information can be found in the headers of each function.

**Note**: As this project was originally part of an assignment, some starter code and testing code was given, seen in the sequence and test files. However, the main allocation methods, and nearly all code in the myalloc.c file was in fact written by me.

Usage:
-----
To compile the code, simply run the `make` commands from the Makefile, and use to run the test functions use `./testmyalloc` or `./simpletest`. 
