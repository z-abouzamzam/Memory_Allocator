#include <stdio.h>
#include <stdlib.h>

#include "myalloc.h"


/*!
 * These variables are used to specify the size and address of the memory pool
 * that the simple allocator works against.  The memory pool is allocated within
 * init_myalloc(), and then myalloc() and free() work against this pool of
 * memory that mem points to.
 */
int MEMORY_SIZE;
unsigned char *mem;

/*
 *
 */
static unsigned char *freeptr;

int header;
int footer;
int currSize;

/*
 * This function initializes both the allocator state, and the memory pool.  It
 * must be called before myalloc() or myfree() will work at all.
 *
 * Note that we allocate the entire memory pool using malloc().  This is so we
 * can create different memory-pool sizes for testing.  Obviously, in a real
 * allocator, this memory pool would either be a fixed memory region, or the
 * allocator would request a memory region from the operating system (see the
 * C standard function sbrk(), for example).
 *
 *
 * For my memory allocator, I initialize each block with a header and a footer.
 * These are simply integer values which allows me to keep track of the start
 * and end of each block by being stored in the memory making accessing of the
 * blocks in the future easier. Positive values indicate free blocks while
 * negatives indicate used memory.
 *
 * Note: I did not use structs in my implemenation. I felt that they were
 * unnessary in the way I planned my implemenation, as I simply only have one
 * value correspond to header and footer, and the sign tell me more information
 * about each.
 */
void init_myalloc() {
    /*
     * Allocate the entire memory pool, from which our simple allocator will
     * serve allocation requests.
     */
    mem = (unsigned char *) malloc(MEMORY_SIZE);
    if (mem == 0) {
        fprintf(stderr,
                "init_myalloc: could not get %d bytes from the system\n",
		MEMORY_SIZE);
        abort();
    }

    /* You can initialize the initial state of your memory pool here. */
    int header, footer;

    /* initializing a header and footer for the starting block of memory
     * The initial memory block has a size of MEMORY_SIZE - 8 bytes for the
     * header and footer. Positive values of header and footer indicate
     * open memory block, negative indicate filled block.
     */

    freeptr = mem;
    header = MEMORY_SIZE - 8;
    footer = header;

    // adds the header and footer to actual locations within the memory block
    *((int *) freeptr) = header;
    *((int *) (freeptr + MEMORY_SIZE - 5)) = footer;

}

/*!
 * Attempt to allocate a chunk of memory of "size" bytes.  Return 0 if
 * allocation fails.
 *
 * This function consists of two parts: the first part traverses the
 * block of memory and implements a best fit algorithm to find the best
 * block of memory for the given size.
 *
 * The second part then using this block, uses only the necessary space
 * within this block to allocate memory for the given size. In other
 * words it splits the block into two parts, the first part for the object
 * and the second for free space. This is done in constant time.
 *
 * The best fit algorithm runs in linear time, as all it does is iterate
 * through the entire memory and find the best result. The strategy is
 * good when there are free blocks all across the memory, but not as
 * efficient when memory is available at the beginning and not at the
 * end of the memory pool.
 */
unsigned char *myalloc(int size) {
    /* Initializes 2 headers and footers for the 2 blocks it will split into,
     * h4 as a temp header for the best fit algorithm, closest for the
     * best fit algorithm, and newHeader as the header to use from the
     * best fit algorithm.
     */
    int h1, h2, f1, f2, h4, newHeader, closest;

    /* best fit algorithm */
    /* traverse the memory from mem to MEMORY_SIZE - 1 */
    /* iterator to traverse the memory */
    unsigned char* iter;

    /* iter equal to start of memory, h4 to initial header closest as */
    /* large value, and newheader arbitrary neg value */
    iter = mem;
    h4 = *(int *) (iter);
    closest = MEMORY_SIZE;
    newHeader = -1;

    /* traverse through memory */
    while (iter < mem + MEMORY_SIZE)
    {
        /* if the header has enough space for the size plus header and footer */
        if(h4 > size + 8)
        {
            /* if this is the smallest usable size, save it */
            if(h4 - size < closest)
            {
                closest = (h4 - size);
                freeptr = iter;
                newHeader = h4;
            }
        }

        /* increment the iterator to the next header */
        iter += (abs(h4) + 8);

        /* increment header to next header */
        h4 = *(int *) iter;

    }

    /* code to split the header */
    if(newHeader > size)
    {
        /* set the pointer to be returned to the start of the memory */
        unsigned char *resultptr = freeptr + 4;

        /* set first header, footer - neg value indicates used */
        /* these indicate the header and footer of the first block */
        h1 = (-size);
        f1 = (-size);

        /* set actual memory locations of header and footer */
        *((int *) (freeptr)) = h1;
        *((int *) (freeptr + size + 4)) = f1;

        /* increment freeptr to next block */
        freeptr += 8 + size;

        /* now set the 2nd headers and footers, and set the memory locations */
        h2 = newHeader - 8 - size;
        f2 = h2;

        *((int *) freeptr) = h2;
        *((int *) (freeptr + h2 + 4)) = f2;

        return resultptr;
    }

    /* if the memory doesn't fit in the block, return 0 */
    else {
        fprintf(stderr, "myalloc: cannot service request of size %d with"
                " %lx bytes allocated\n", size, (freeptr - mem));
        return (unsigned char *) 0;
    }
}

/* Forward coalescing function.
 * takes two arguments, pointer to the current header, and the value of the
 * header.
 *
 * Time of this function is constant as it simply modifies adjacent memory
 * it has access to through the header and footer values. It only looks
 * at one other chunk of memory, so time here remains constant.
 *
 * Works by simply combining the current block with one to its right, by
 * resetting the enclosing header and footer to new values and removing the
 * middle header and footer.
 */
void coalesceForward(unsigned char* i, int h1)
{
    /* initial headers and footers which are calculated from arguments */
    int h2, f2, f1;
    f1 = h1;
    h2 = *((int *) (i + h1 + 8));

    /* Sets the first header and the 2nd footer equal to entire memory block*/
    f2 = h1 + h2 + 8;
    h1 = f2;

    /* set the memory location of the header and footer */
    *((int *) i) = h1;
    *((int *) (i + h1 + 4)) = f2;

    /* remove h2, f1  by setting the values equal to 0 */
    *((int *) (i + f1 + 4)) = 0;
    *((int *) (i + f1 + 8)) = 0;
}

/* Backward coalescing function.
 * Very similar to coalesceForward, except now works in opposite direction
 * takes two arguments, a pointer to the current header, and the value of the
 * preceding footer
 *
 * Time of this function is also constant. Because we keep track of footer
 * values, we know that the footer of the previous memory is adjacent to the
 * header of the current memory, so the function just looks at one other entry,
 * so time is constant.
 *
 * Works by simply combining the current block with one to its left. It
 * can do this efficiently because of footers marking the end of preceding
 * memory blocks, and because it works in the same way as forward, just backward
 *
 */
void coalesceBackward(unsigned char* i, int f1)
{
    /* initial headers and footers for both blocks, calculated using the */
    /* function arguments */
    int h1, h2, f2;
    h1 = *((int *) (i - 8 - f1));
    h2 = *((int *) (i));

    /* calc the new size of the memory block with h1 and f2 */
    h1 = h1 + 8 + h2;
    f2 = h1;

    /* Now set the new blocks in memory and clear the old ones */
    *((int *) (i - 8 - f1)) = h1;
    *((int *) (i + h2 + 4)) = f2;
    *((int *) (i)) = 0;
    *((int *) (i - 4)) = 0;

}


/*!
 * Free a previously allocated pointer.  oldptr should be an address returned by
 * myalloc().
 */
void myfree(unsigned char *oldptr) {
    /*
     * The free function works by simply setting the header and footer of the
     * current pointer to positive values, and then checking the adjacent
     * blocks to see if coalescing is necessary. This function runs in constant
     * time as it only looks at one memory block and calls coalesceForward and
     * coalesceBackward, which are constant time. See respective functions for
     * details on that.
     */

     /* ptr points to the start of the current header, and temp is used */
     /* when calling coalescing funcitons. We also have h and f which are */
     /* the header and footer values of the current block */
     unsigned char * ptr, *temp;
     int h, f;

     /* set ptr to the start of header not start of memory */
     ptr = oldptr - 4;

     /* set header and footer */
     h = *((int *) ptr);
     f = *(int *) (ptr - 4);

     /* make the header and footer values pos instead of neg*/
     *((int *) ptr) = abs(h);
     *((int *) (ptr + abs(h) + 4)) = abs(h);

     /* use temp as argument when calling coalescing */
     temp = ptr;

     /* calls this if forward coalescing possible */
     if(*((int *) (ptr + abs(h) + 8)) > 0
        && ptr + abs(h) + 8 < mem + MEMORY_SIZE)
     {
         coalesceForward(temp, abs(h));
     }

     /* reset temp to the ptr when calling backwards */
     temp = ptr;

     /* calls this if backwards coalescing possible */
     if(*(int *) (ptr - 4) > 0 && ptr - 4 > mem)
     {
         coalesceBackward(temp, f);
     }
}


/*!
 * Clean up the allocator state.
 * All this really has to do is free the user memory pool. This function mostly
 * ensures that the test program doesn't leak memory, so it's easy to check
 * if the allocator does.
 */
void close_myalloc() {
    free(mem);
}
