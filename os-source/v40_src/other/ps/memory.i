*************************************************************************
* Memory.i: This file is the equivalent M/C include to C's memory.h
*
*
*************************************************************************

* Header for block of free memory.  Used for high and low overhead mem pools
	STRUCTURE __freeblk,0		; struct name : fb
		ULONG	fb_next
		ULONG	fb_size
	LABEL	fb_SIZEOF

* typedef struct __freeblk {
* 	struct __freeblk *next;	* the next free block in the list
* 	ulong	size;			* the size of this block (including the header 
* } freeblk;

* Header for a block of allocated memory.  Only for high overhead memory pool. 
	STRUCTURE __alocblk,0		; struct name : al
		ULONG	al_next
		UWORD	al_size
		UWORD	al_access_count
	LABEL	al_SIZEOF

* typedef struct __alocblk {
*	struct __alocblk *next;	* next allocated memory block 
*	uword	size;		* size of this block (including the header) 
*	uword	access_count;	* access count of this block 
* } alocblk;

* Root structure for a particular memory pool.  Contains a list of allocation  
* blocks and a free list.  Also keeps track of the first allocation block and  
* stats for total and remaining free memory.					

	STRUCTURE __memhdr,0		; struct name : mh
		ULONG	mh_blocks
		ULONG	mh_first
		ULONG	mh_free
	LABEL	mh_SIZEOF

* typedef struct __memhdr {
*	freeblk	*blocks;	* memory pool, a list of allocation blocks   
*	freeblk *first;		* first allocation block in the memory pool  
*	freeblk	*free;		* list of free memory fragments (all blocks) 
* } memhdr;

* Root header for a VM pool.  Contains root of free lists for both the high and 
* low overhead pools as well as links and nestcount for PostScript save levels. 

	STRUCTURE __vmem,0			; struct name : vm
		ULONG	vm_prev
		ULONG	vm_next
		ULONG	vm_savelevel
		STRUCT	vm_lo,mh_SIZEOF
		STRUCT	vm_hi,mh_SIZEOF
		ULONG	vm_alloc
	LABEL	vm_SIZEOF

* typedef struct __vmem {
*	struct __vmem *prev;	* previous vmem for restore 
*	struct __vmem *next;	* next vmem after save 
*	ulong	savelevel;	* save/restore nest count 
*	memhdr	lo;		* low overhead memory pool 
*	memhdr	hi;		* high overhead memory pool 
*	alocblk *alloc;		* allocated memory in high overhead pool 
* } vmem;
