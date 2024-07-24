/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your information in the following struct.
 ********************************************************/
team_t team = {
    /* Your student ID */
    "20191264",
    /* Your full name*/
    "Sungmin Yoon",
    /* Your email address */
    "zserty12@sogang.ac.kr",
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define WSIZE 4 //word, header, footer size
#define DSIZE 8 //double word size
#define CHUNKSIZE (1<<12)

#define MAX(x,y) (x > y ? x : y)
#define PACK(size, alloc) ((size) | (alloc)) //header 생성
#define GET(p) (*(unsigned int*) (p))
#define PUT(p, val) (*(unsigned int*)(p) = val)
#define GET_SIZE(p) (GET(p) & ~0x7) //8byte alignment
#define GET_ALLOC(p) (GET(p) & 0x1) //LSB is alloc/free bit
#define HDRP(bp) ((char*)((bp)) - WSIZE) //header의 위치
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE) //footer의 위치

#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(((char*)(bp) - WSIZE))) 
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE(((char*)(bp) - DSIZE)))

//free block list for explicit list method
#define NEXT_FREEP(bp) (*(char **)((char*)(bp) + WSIZE))
#define PREV_FREEP(bp) (*(char **)(bp))

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

static void *extend_heap(size_t words);
static void *find_fit(size_t size);
static void place(void* bp, size_t asize);
static void *coalesce(void* bp);
static void remove_block(void *bp);
static void insert_block(void *bp);
static char *free_listp;

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    //create the initial empty heap
    if ((free_listp = mem_sbrk(6*WSIZE)) == (void *)-1)
        return -1;
    PUT(free_listp, PACK(0,0)); //unused padding
    //prologue header & footer
    //PUT(free_listp + WSIZE, PACK(DSIZE, 1)); 
    //PUT(free_listp + 2*WSIZE, PACK(DSIZE, 1)); 
    //1st free block
    PUT(free_listp + WSIZE, PACK(4*WSIZE, 1)); 
    PUT(free_listp + 2*WSIZE, 0);
    PUT(free_listp + 3*WSIZE, 0);
    PUT(free_listp + 4*WSIZE, PACK(4*WSIZE, 1));    
    //epilouge header
    PUT(free_listp + 5*WSIZE, PACK(0,1));
    free_listp += 2*WSIZE;

    //CHUNKSIZE만큼 free block을 갖게 heap extend
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}
/*
*extend heap - extend heap with free blocks under alignment regs
*/
static void *extend_heap(size_t words) {
    char *bp;
    size_t size;
    
    //allocate an even number of word for maintaing alignment
    size = (words % 2) ? (words+1) * WSIZE : words*WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    //initialize free block header&footer and epilogue header
    PUT(HDRP(bp), PACK(size,0));
    PUT(FTRP(bp), PACK(size,0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1));
    //coalesce if the prev block is free
    return coalesce(bp);
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize; //adjusted block size
    size_t extendsize; //amount to extend heap if no fit
    char* bp;

    //ignore spurious request
    if (size == 0) return NULL;
    //adjust block size to include overhead and alignment regs
    if (size <= DSIZE) asize = 2*DSIZE;
    else asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
    //search the free list for a fit
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }
    //No fit -> extend heap
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

/*
*find fit - first fit method
*/
static void *find_fit(size_t size) {
    void *bp;
    /*while (bp) {
        if (!GET_ALLOC(HDRP(bp)) && size <= GET_SIZE(HDRP(bp)))
            return bp;
        bp = NEXT_FREEP(bp);
    }*/
    for (bp = free_listp; GET_ALLOC(HDRP(bp)) != 1; bp = NEXT_FREEP(bp)) {
        if (size <= GET_SIZE(HDRP(bp)))
            return bp;
    }
    return NULL;
}

/*
*coalesce - coalescing if prev or next block is also free block
*/
static void *coalesce(void *bp) {
    if (bp == NULL) return NULL;
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    //Case 1 - prev & alloc allocated
    if (prev_alloc && next_alloc) {
        insert_block(bp);
        return bp;
    }
    //Case 2 - prev allocated & next freed
    else if (prev_alloc && !next_alloc) {
        remove_block(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size,0));
        PUT(FTRP(bp), PACK(size,0));
    }
    //Case 3 - prev freed & next allocated
    else if (!prev_alloc && next_alloc) {
        remove_block(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        bp = PREV_BLKP(bp);
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));
        PUT(HDRP(bp), PACK(size,0));
        
    }
    //Case 4 - prev & next freed
    else {
        remove_block(PREV_BLKP(bp));
        remove_block(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        bp = PREV_BLKP(bp);
        PUT(HDRP(bp), PACK(size,0));
        PUT(FTRP(bp), PACK(size,0));
    }
    insert_block(bp);
    return bp;
}

/*
*place - place block of asize bytes and split if remainder is at least min block size
*/
static void place(void* bp, size_t asize) {
    size_t csize = GET_SIZE(HDRP(bp));
    //remove_block(bp);

    //remainder space is left enough -> split
    if ((csize - asize) >= (2*DSIZE)) {
        remove_block(bp);
        PUT(HDRP(bp), PACK(asize,1));
        PUT(FTRP(bp), PACK(asize,1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK((csize-asize),0));
        PUT(FTRP(bp), PACK((csize-asize),0));
        insert_block(bp);
    }
    else {
        remove_block(bp);
        PUT(HDRP(bp), PACK(csize,1));
        PUT(FTRP(bp), PACK(csize,1));
    }

}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    if (bp == 0) return;
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size,0));
    PUT(FTRP(bp), PACK(size,0));
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
*/
void *mm_realloc(void *ptr, size_t size)
{
    if (!ptr) return mm_malloc(size);
    if (size ==0) {
        mm_free(ptr);
        return 0;
    }
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = GET_SIZE(HDRP(ptr)) - DSIZE;
    if (size < copySize)
      copySize = size;
    memcpy(newptr, ptr, copySize);
    mm_free(ptr);
    return newptr;
}

/*
*insert_block - insert a new free block into the free list using LIFO
*/
static void insert_block(void *bp) {
    if (bp == NULL) return;
    NEXT_FREEP(bp) = free_listp;
    PREV_FREEP(bp) = NULL;
    if (free_listp != NULL)
        PREV_FREEP(free_listp) = bp;
    free_listp = bp; 
}

/*
*remove_block - remove a block from the free list
*/
static void remove_block(void *bp) {
    if (bp == NULL) return;
    if (PREV_FREEP(bp))
        NEXT_FREEP(PREV_FREEP(bp)) = NEXT_FREEP(bp);
    else 
        free_listp = NEXT_FREEP(bp);
    if (NEXT_FREEP(bp))
        PREV_FREEP(NEXT_FREEP(bp)) = PREV_FREEP(bp);
}
