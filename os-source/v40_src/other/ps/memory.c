/*
 * Standard memory allocation routines for PostScript.  There are actually two
 * different allocators here.  One is a simple memory allocator/de-allocator
 * that deals with exact pointers to memory and must be given the size of an
 * area that is to be freed.  This is the low overhead virtual memory pool.
 * This pool permits the re-allocation of memory blocks to either shrink or
 * grow the size of an allocated block (copying it if nescessary).
 *
 * The other allocator deals with inexact pointers to memory and maintains
 * a list of allocated blocks of memory and keeps access counts on those
 * blocks.  If the access count of a block is non-zero then that block cannot
 * be freed (because there is more than one pointer to it).  This is the high
 * overhead virtual memory pool used by the PostScript interpreter.
 *
 * Both types of memory pool are associated with a vmem header that represents
 * the current savelevel of an executing postscript context or process.  The
 * actual PostScript objects themselves must keep track of the savelevel at
 * which an object was created, the allocation routines know nothing about it.
 */

#include "exec/types.h"
#include <exec/memory.h>

#include "pstypes.h"
#include "memory.h"

#include <proto/exec.h>
#include <stdio.h>

/* non-public functions that need prototyping */
int   GetMoreVM(memhdr *, ulong);
void  MergeFreeVM(memhdr *, freeblk *, ulong);
void *Alloc(memhdr *, ulong);

/******************************************************************************
*
* vmem = CreateVM()
*  d0
*
* Allocates and initialises a VM pool for use. No pool memory is allocated for
* the low and high overhead memory pools because this is done automatically as
* allocation requests are received.  Could easily make this a macro instead.
*
******************************************************************************/
vmem *CreateVM( void ) {
    return (vmem *)AllocMem(sizeof(vmem),MEMF_PUBLIC|MEMF_CLEAR);
}

/******************************************************************************
*
* DestroyVM( vmem )
*	      a0
*
* Frees up all memory used by a vmem struct and any alloced or free blocks. The
* access counts on high overhead blocks are totally ignored. Everything will be
* freed up regardless of current usage.  Be real careful that nothing is still
* using any of the allocated memory.  Usually only used at context termination
* or when a PostScript restore command is succesfully executed.
*
******************************************************************************/
void DestroyVM( vmem *m ) {
register freeblk *f,*n;

    n=m->lo.blocks;	/* first free low overhead memory pool */
    while(n) { f=n; n=f->next; FreeMem((char *)f,f->size); }

    n=m->hi.blocks;	/* now free high overhead memory pool */
    while(n) { f=n; n=f->next; FreeMem((char *)f,f->size); }

    FreeMem((char *)m,sizeof(vmem));	/* finally free the header itself */
}

/******************************************************************************
*
* memory = AllocVM( vmem, size )
*   d0		     a0    d0
*
* Allocates size bytes of memory from the low overhead memory pool.  Returns a
* pointer to the allocated memory or NULL if the allocation fails.
******************************************************************************/
void *AllocVM(vmem *v, ulong s) { return(Alloc(&v->lo,s)); }

/******************************************************************************
* memory = ReAllocVM( vmem, memory, oldsize, newsize )
*   d0		       a0     a1      d0	d1
*
* Used to shrink or grow allocations.  If the rounded up sizes work out to be
* exactly the same then the original memory pointer is returned.  If, however,
* the new size is larger or shrinking won't allow the allocated block to be
* split into a free block and the original memory then a new area is allocated
* and the data is copied across.  In this case a new pointer is returned.  If
* allocation of a new block fails then the original block will be freed and
* NULL will be returned instead.
******************************************************************************/
void *ReAllocVM(vmem *v, void *m, ulong s, ulong n) {
void *newmem;

    s=ROUNDUP(s); n=ROUNDUP(n); if(s==n) return m;

    if(n==0) {	/* new size of 0 so free the whole lot */
	MergeFreeVM(&v->lo,(freeblk *)m,s);
	return NULL;
    }

    if((n<s) && ((s-n) >= sizeof(freeblk))) { /* room to shrink in place */
	MergeFreeVM(&v->lo,(freeblk *)(n+(ulong)m),s-n); /* free end of block */
	return m;
    }

    /* Can't do allocations in place or there may not be enough space to split */
    /* the allocation into two parts (allocated and free) so we must allocate  */
    /* a new block of memory and copy the data across before freeing original  */
    if(newmem=AllocVM(v,n)) {	/* got the new piece of memory */
	n=n<s ? n : s;		/* copy minimum size across */
	/* because min alloc size is 8, data always longword aligned */
	CopyMemQuick((char *)m,(char *)newmem,(long)n);
    }
    MergeFreeVM(&v->lo,(freeblk *)m,s);	/* free original memory block */
    return(newmem);			/* and return the new block   */
}

/******************************************************************************
*
* FreeVM(vmem, memory, size)
*	  a0	 a1	d0
*
* Frees a chunk of low overhead memory and merges it with adjacent free blocks
******************************************************************************/
void FreeVM(vmem *v, void *m, ulong s) {
    MergeFreeVM(&v->lo,(freeblk *)m,ROUNDUP(s));/* merge into the free pool */
}

/******************************************************************************
*
* memory = AllocTrackedVM( vmem, size )
*   d0			    a0    d0
*
* Allocates size bytes of memory from the high overhead memory pool. Returns a
* pointer to the allocated memory or NULL if the allocation fails.  This memory
* is used when garbage collect is required along with inexact pointers to the
* memory for freeing purposes.  There is no need to keep track of the size for
* this memory because it's held in a prepended structure and linked into a
* list of allocated blocks.  When calling FreeTrackedVM this size is searched
* for and extracted at that point instead.
******************************************************************************/
void *AllocTrackedVM(vmem *v, ulong s) {
alocblk *a;

    if(!s) return NULL;		/* can't allocate 0 memory */
    s+=sizeof(alocblk);		/* add in alocblk overhead */
    if(a=Alloc(&v->hi,s)) {	/* we got the memory */
	a->size=s;		/* stash the allocated size */
	a->access_count=1;
	a->next = v->alloc;	/* link into list of allocated blocks */
	v->alloc = a;
	return(a+1);		/* return pointer to callers memory */
    }
    else return NULL;
}

/******************************************************************************
*
* FreeTrackedVM( vmem, memory )
*		  a0	 a1
*
* Given a pointer to a piece of tracked memory, or (more importantly) a pointer
* to somewhere within a piece of tracked memory, frees that block of memory.
* This allows the PS interpreter to blithely pass around pointers and pointers
* with offsets added in and still have them refer to the same allocation.  If
* the access count of a piece of tracked memory is > one then no freeing will
* take place (because some PS object is still referencing it) but the access
* count will be decremented instead.
******************************************************************************/
void FreeTrackedVM( vmem *v, void *m ) {
alocblk *a,*t,**tptr;

    /* Check simple case where we have the pointer returned by AllocTrackedVM */
    a=((alocblk *)m)-1;	/* cast *void to *alocblk */
    tptr = &v->alloc;	/* point at list of allocated blocks */
    while(*tptr && (*tptr!=a)) tptr = &((*tptr)->next);

    /* if that fails check the case of a pointer to somewhere in the block */
    if(*tptr==0) { /* we didn't find our block of memory */
	tptr = &v->alloc;	/* point at list of allocated blocks */
	while((t=*tptr) && !((t<a)&&(((ulong)a-(ulong)t)<=t->size))) tptr=&t->next;
	a=t;	/* we found our block */
    }

    if( a && !(--(a->access_count))) {/* if access count of this block is now 0 */
	*tptr=a->next;	/* unlink allocated block from list */
	MergeFreeVM(&v->hi,(freeblk *)a,ROUNDUP(a->size));/* and merge into free memory */
    }
}

/******************************************************************************
*
* ReUseTrackedVM( vmem, memory )
*		   a0	  a1
*
* Given a pointer to a piece of tracked memory, or (more importantly) a pointer
* to somewhere within a piece of tracked memory, bumps the access count on that
* piece of memory.  The interpreter uses this routine whenever a composite
* object is duplicated on the stack or by reference from a name lookup.
*
* **NOTE** The block of memory is also moved to the head of the alloc list so
* that a subsequent free of this block is more likely to find it quicker.  This
* really depends on the way the interpreter creates and destroys objects but
* most duplications get consumed immediately so this is a valid optimisation.
******************************************************************************/
void ReUseTrackedVM( vmem *v, void *m ) {
alocblk *a,*t,**tptr;

    /* Check simple case where we have the pointer returned by AllocTrackedVM */
    a=((alocblk *)m)-1;	/* cast *void to *alocblk */
    tptr = &v->alloc;	/* point at list of allocated blocks */
    while(*tptr && (*tptr!=a)) tptr = &((*tptr)->next);

    /* if that fails check the case of a pointer to somewhere in the block */
    if(*tptr==0) { /* we didn't find our block of memory */
	tptr = &v->alloc;	/* point at list of allocated blocks */
	while((t=*tptr) && !((t<a)&&(((ulong)a-(ulong)t)<=t->size))) tptr=&t->next;
	a=t;	/* we found our block */
    }

    if(a) { /* we found our block of memory */
	++(a->access_count);	/* bump the access count */
	*tptr = a->next;	/* and move to head of list */
	a->next = v->alloc;
	v->alloc = a;
    }
}

/******************************************************************************
*
* memory=ConvertVM( vmem, memory, size )
*   d0		     a0	    a1	   d0
*
* Converts the given block of low overhead memory into tracked memory (copying
* data across) and frees the given low overhead memory (even if the tracked
* allocation failed).  NULL return indicates no memory was available.
******************************************************************************/
void *ConvertVM(vmem *v, void *m, ulong s) {
void *n;

    if(n = AllocTrackedVM(v,s)) { /* if we got our memory */
	CopyMemQuick((char *)m,(char *)n,(long)ROUNDUP(s));
    }
    FreeVM(v,m,s);	/* free the original block of memory */
    return(n);		/* and return the new one or NULL */
}

/******************************************************************************
*
* success = GetMoreVM(memhdr, size )
*   d0			a0     d0
*
* Allocates another chunk of memory from the system memory pool for a memhdr's
* free list.  Will attempt to allocate MAX(VMBLOCKSIZE,size+sizeof(freeblk))
* bytes. If successful links the new block into the memhdr.blocks list and then
* links the free portion of that into the free list. If this is the first time
* an allocation was performed for this memhdr then the block address is saved
* in memhdr.first so we don't attempt to free an allocation block when it's
* the last one available.
*
* Returns TRUE for success else FALSE if the allocation fails.  This routine is
* an internal allocator call only, not to be used by higher level functions!
*
******************************************************************************/
int GetMoreVM(memhdr *m, ulong s) {
freeblk *f,**tptr;

    s += sizeof(freeblk);	/* get space needed+freeblk for linkage  */
    s = s>VMBLOCKSIZE ? s : VMBLOCKSIZE;/* always allocate at least VMBLOCKSIZE */
    s = ROUNDUP(s);	/* ensure alignment is correct */
    if(f=(freeblk *)AllocMem(s,MEMF_PUBLIC)) {
	f->size=s;			/* size of this allocation block 	*/
	f->next = m->blocks;		/* link into list of allocation blocks	*/
	m->blocks = f;
	if(m->first==0) m->first=f;	/* first block to be allocated	*/

	s -= sizeof(freeblk);		/* allow for linkage area we just used	*/
	f += 1;				/* point at beginning of free area	*/

	f->size = s;			/* setup the size of this free block    */

	/* Free blocks have to be stored in ascending address order for merging */
	/* Don't call MergeFreeVM, it will attempt to free this mem immediately */
	tptr = &m->free;
	while((*tptr) && (*tptr<f)) tptr=&((*tptr)->next);
	f->next=*tptr; *tptr=f;		/* link our block into the free list    */
	return(TRUE);
    }
    else return(FALSE);
}

/******************************************************************************
*
* MergeFreeVM( memhdr, freeblk, size)
*		 a0	  a1     d0
*
* Given a chunk of free memory, merges it into the current list of free memory.
* If the block is contiguous with any other free blocks then they are merged
* into one larger chunk.  This routine works with exact pointers only and the
* given size must be correct.  It is legal to free part of an allocated block
* of low overhead memory providing you stick to the alignment restrictions.
*
* When memory has been merged a check is made to see if an allocation block is
* no longer in use (no allocated memory in it).  If this is the case AND that
* allocation block is not the first one, then it will be freed.  This nicely
* handles the case of large composite objects that consume VM transiently.
*
******************************************************************************/
void MergeFreeVM(memhdr *m, freeblk *f, ulong size) {

freeblk **tptr= &m->free;
freeblk *prev = NULL;

    while((*tptr)&&(*tptr<f)) { /* while next block is at a lower address */
	prev = *tptr;		/* save pointer to previous block	  */
	tptr = &prev->next;	/* and get ready to fetch next block	  */
    }


    /* tptr is now pointing at the pointer to the block that will come after */
    /* the freeblk we are merging.  prev points at the block that will come  */
    /* before the memblk we are merging.  Because the free list is sorted we */
    /* only have to consider the three blocks (prev,*tptr and b) for merging */

    /* check for previous block (if any) merging with block we are freeing */
    if((prev) && ((freeblk *)((ulong)prev + prev->size) == f)) {
	prev->size+=size;	/* just add freed memory to previous block */
	f=prev;		/* and make it look like the freed block   */
    }
    else {	/* not contiguous with previous, so link in freed block */
	f->size = size;	/* setup the size of this free block */
	f->next=*tptr;	/* and merge into the free list */
	*tptr=f;
    }

    /* check for the case of the current block merging with the next one too */
    if( (freeblk *)((ulong)f + f->size) == f->next) {
	f->size+=f->next->size;	/* merge with our block */
	f->next =f->next->next;	/* and remove next block from free list */
    }

    /* **NOTE** Maybe make this part of a garbage collect routine instead!  */

    /* finally, check to see if we have merged an allocation block and can  */
    /* free it.  If we can then we must remove the free block from the free */
    /* memory pool too and adjust the total available free memory.	    */
    prev = f-1;		/* possibly point at an allocation block    */
    if(prev != m->first) {	/* if this isn't the first allocation block */
	tptr = &m->blocks;	/* point at list of allocation blocks	    */
	while((*tptr) && (*tptr!=prev)) tptr = &((*tptr)->next); /* search for allocation block */
	if(*tptr && ((*tptr)->size==(f->size+sizeof(freeblk)))) {
	    *tptr=prev->next;	/* so unlink from blocks list	    */
	    tptr=&m->free;	/* search for the free block again  */
	    while(*tptr != f) tptr=&((*tptr)->next);
	    *tptr = f->next;	/* unlink from list of free memory  */
	    FreeMem((char *)prev,prev->size);	/* free allocation block */
	}
    }
}

/******************************************************************************
*
* memory = Alloc(memhdr,size)
*   d0		   a0    d0
*
* Attempts to allocate size bytes of memory from the given memory pool. Returns
* pointer on success else returns NULL.  If an initial allocation fails then an
* attempt is made to procure a new allocation block for the given memory pool,
* if this succeeds then allocation will come from there.  If this fails then no
* memory can be allocated and a NULL pointer will be returned.
******************************************************************************/
void *Alloc(memhdr *m, ulong s) {
freeblk *b,*try,**freeptr;
ulong split_size;

    if(!(s=ROUNDUP(s)))	return NULL;	/* round to nearest ALLOCSIZE */
    split_size=s+sizeof(freeblk);

try_again:
    /* Attempt to find a free block that is exactly the right size or bigger. */
    try = NULL;		/* nothing found yet */
    freeptr = &m->free;	/* initialise list pointer */

    while(b=*freeptr) {/* while another free block in the list */
	if(b->size >= s) {	/* this is a possible candidate */
	    if(b->size == s) {	/* found an exact fit */
		*freeptr = b->next;	/* so unlink from free list */
		return(b);		/* we've found our block    */
	    }
	    /* Not an exact fit, check if it's a splitting candidate. */
	    /* We must be careful that the free block is large enough */
	    /* to be split into the allocated memory and a free block */
	    else if((try==NULL) || (b->size<try->size)) {
		if(b->size >= split_size) try=b; /* must have room to split */
	    }
	}
	freeptr = &b->next;		/* get ready to link to next block */
    }

    if(try) {	/* found a candidate for splitting up  */
	try->size -= s;	/* this much of the free block used up */
	return((void *)(try->size+(ulong)try)); /* use space from end of free block */
    }

    /* Couldn't find any free store big enough.  Try to allocate more storage */
    else if(GetMoreVM(m,s)) goto try_again; /* sorry, but goto is the only way!! */

    /* Couldn't even allocate more storage so the allocation must fail now */
    else return NULL;
}


/******************************************************************************
*
* Total = GetTotalVM(vmem)
*   d0		   		  a0
*
* Return total memory used by allocation blocks in the low overhead pool.  May
* start counting tracked VM if we ever use it.
******************************************************************************/
int GetTotalVM( vmem *v ) {
freeblk *f;
int i=0;
	f=v->lo.blocks;
	while(f) {
		i+=f->size; f=f->next;
    }
    return(i);
}

/******************************************************************************
*
* Total = GetFreeVM(vmem)
*   d0		   		 a0
*
* Return total memory used by allocation blocks in the low overhead pool.  May
* start counting tracked VM if we ever use it.
******************************************************************************/
int GetFreeVM( vmem *v ) {
freeblk *f;
int i=0;

	f=v->lo.free;
	while(f) {
		i+=f->size; f=f->next;
	}
	return(i);
}

#ifdef DEBUG
/******************************************************************************
			 debug routine
******************************************************************************/
void ShowVM(vmem *v) {
freeblk *f;
int i;

	printf("Block List\n");
	f=v->lo.blocks;
	i=0;
	while(f) { 
		printf("%4ld/%6lx\t",f->size,f);
		f=f->next;
		if(++i > 4) { i=0; printf("\n"); }
	}
	printf("\n");

	printf("Free List\n");
	f=v->lo.free;
	i=0;
	while(f) { 
		printf("%4ld/%6lx\t",f->size,f);
		f=f->next;
		if(++i > 4) { i=0; printf("\n"); }
	}
	printf("\n");

 }

/******************************************************************************
			 debug routine
******************************************************************************/
void ShowTrackedVM(vmem *v) {
freeblk *f;
alocblk *a;
int i;
int fb,ab,tf,ta;

	fb=ab=tf=ta=0;
	
	printf("Block List\n");
	f=v->hi.blocks;
	i=0;
	while(f) { 
		printf("%4ld/%6lx\t",f->size,f);
		f=f->next;
		if(++i > 4) { i=0; printf("\n"); }
	}
	printf("\n");

	printf("Free List\n");
	f=v->hi.free;
	i=0;
	while(f) { 
		++fb; tf+=f->size;
		printf("%4ld/%6lx\t",f->size,f);
		f=f->next;
		if(++i > 4) { i=0; printf("\n"); }
	}
	printf("\n");

	printf("Alloc List\n");
	a=v->alloc;
	i=0;
	while(a) { 
		++ab; ta+=a->size;
		printf("%2d/%4d/%6lx\t",a->access_count,a->size,a);
		a=a->next;
		if(++i > 4) { i=0; printf("\n"); }
	}
	printf("\n");

	printf("%ld free blocks and %ld allocated blocks\n",fb,ab);
	printf("%ld free total and %ld allocated total\n",tf,ta);
 }

#endif
