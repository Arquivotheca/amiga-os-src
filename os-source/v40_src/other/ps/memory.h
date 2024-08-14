#ifndef MEMORY_H
#define MEMORY_H

/* All memory pool blocks are fetched in lumps of at least this size */
#define VMBLOCKSIZE 1024

/* Allocation is rounded up to nearest ALLOCSIZE bytes. This size is */
/* special because it's sizeof(freeblk|alocblk) and will ensure that */
/* all allocations are longword aligned for fast copies and clearing */
#define ALLOCSIZE 8

/* Macro to round up a size to the nearest ALLOCSIZE bytes */
#define ROUNDUP(n) (((n)+(ALLOCSIZE-1)) & ~(ALLOCSIZE-1))

/* Header for block of free memory.  Used for high and low overhead mem pools */
typedef struct __freeblk {
	struct __freeblk *next;	/* the next free block in the list */
	ulong	size;		/* the size of this block (including the header */
} freeblk;

/* Header for a block of allocated memory.  Only for high overhead memory pool. */
typedef struct __alocblk {
	struct __alocblk *next;	/* next allocated memory block */
	uword	size;		/* size of this block (including the header) */
	uword	access_count;	/* access count of this block */
} alocblk;

/* Root structure for a particular memory pool.  Contains a list of allocation  */
/* blocks and a free list.  Also keeps track of the first allocation block and  */
/* stats for total and remaining free memory.					*/
typedef struct __memhdr {
	freeblk	*blocks;	/* memory pool, a list of allocation blocks   */
	freeblk *first;		/* first allocation block in the memory pool  */
	freeblk	*free;		/* list of free memory fragments (all blocks) */
} memhdr;

/* Root header for a VM pool.  Contains root of free lists for both the high and */
/* low overhead pools as well as links and nestcount for PostScript save levels. */
typedef struct __vmem {
	struct __vmem *prev;	/* previous vmem for restore */
	struct __vmem *next;	/* next vmem after save */
	ulong	savelevel;	/* save/restore nest count */
	memhdr	lo;		/* low overhead memory pool */
	memhdr	hi;		/* high overhead memory pool */
	alocblk *alloc;		/* allocated memory in high overhead pool */
} vmem;

/* prototypes for public functions in memory.c */

vmem *CreateVM(void);			/* create a new vmem structure */
void  DestroyVM(vmem *);		/* free vmem and all associated memory */

void *AllocVM(vmem *,ulong);		/* allocate some low overhead VM */
void  FreeVM(vmem *, void *, ulong);	/* free some low overhead VM */
void *ReAllocVM(vmem *,void *,ulong,ulong); /* re-allocate low overhead VM at new size */

void *AllocTrackedVM(vmem *,ulong);	/* allocate some high overhead VM */
void  FreeTrackedVM(vmem *, void *);	/* free some high overhead VM */
void  ReUseTrackedVM(vmem *,void *);	/* bump use count on high overhead VM */

void *ConvertVM(vmem *, void *, ulong);	/* convert lo overhead to hi overhead VM */ 

int GetTotalVM(vmem *);	/* return total memory allocated (available) in pool */
int GetFreeVM(vmem *);	/* return total free memory in allocated pool */

#ifdef DEBUG
void ShowVM(vmem *);		/* print out low overhead memory lists */
void ShowTrackedVM(vmem *);	/* print out high overhead memory lists */
#endif

#endif MEMORY_H
