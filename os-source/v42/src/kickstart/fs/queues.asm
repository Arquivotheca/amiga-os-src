		SECTION	filesystem,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/io.i"
		INCLUDE	"globals.i"
		INCLUDE	"private.i"
		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XDEF	AddHash,UnHash,FindHash,FreeBuffer

;==============================================================================
; FreeBuffer( Buffer )
;	    	a0
; Removes the buffer from whatever queue it's on, UnHashes it, and frees it
;==============================================================================
FreeBuffer:	move.l	a0,a1
		move.l	a1,d1			Remove doesn't touch this
		REMOVE
		movea.l	d1,a0			remove from hash table too
		bsr.s	UnHash
		movea.l	d0,a0
		clr.w	cb_QueueType(a0)
		move.l	FreeBufferQueue(a5),(a0)
		move.l	a0,FreeBufferQueue(a5)
		rts

;==============================================================================
; AddHash( Buffer )
;	     a0
;
; Inserts a cache buffer into the hash table based on the key number contained
; in the cache buffer header.  Nothing is done to the queues that this cache
; buffer belongs to.  This is handled by EnQueue and DeQueue.
;==============================================================================
AddHash		movea.l	CacheHash(a5),a1	address of hash table
		moveq.l	#127,d0			calculate hash address
		and.w	cb_Key+2(a0),d0		(key & 127)<<2
		lsl.w	#2,d0
		adda.w	d0,a1			a1 points to first entry
		move.l	(a1),cb_NextHash(a0)	link us to the next
		move.l	a0,(a1)			and put us in the table
		rts

;==============================================================================
; Buffer = UnHash( Buffer )
;   d0		     a0
;
; Removes a buffer from the hash table but does not affect the queue links.
; The buffer MUST be in the hash table or this routine will crash dismally.
;==============================================================================
UnHash		movea.l	CacheHash(a5),a1	address of hash table
		moveq.l	#127,d0			calculate hash address
	IFD DEBUG_CODE
	  moveq #0,d1
	ENDC
		and.w	cb_Key+2(a0),d0		(key & 127)<<2
		lsl.w	#2,d0
		adda.w	d0,a1			a1 points to first entry
UHLoop		cmpa.l	(a1),a0			is this the one we want
		beq.s	FoundBuffer
		movea.l	(a1),a1
	IFD DEBUG_CODE
	  move.l a1,d0
	  bne.s  1$
 printf <'BAD UnHash of $%lx (block %ld), q/state $%lx'>,a0,cb_Key(a0),cb_QueueType(a0)
	  moveq	#1,d1
	  move.l a0,d0
	  rts
1$:
	ENDC
		lea.l	cb_NextHash(a1),a1
		bra.s	UHLoop
FoundBuffer
		move.l	cb_NextHash(a0),(a1)	unlink us
		move.l	a0,d0			return address of buffer
		rts

;==============================================================================
; Buffer = FindHash( Keynum )
;   d0		       d0
;
; Locates a buffer containing the given key if it exists else returns 0.  The
; queue that this buffer is on is determined by the cb_QueueType field.
;==============================================================================
FindHash	moveq.l	#127,d1
		and.w	d0,d1
		lsl.w	#2,d1			calculate the entry offset
		movea.l	CacheHash(a5),a0
		move.l	0(a0,d1.w),d1		and get the first buffer ptr
		beq.s	FindDone
FindHLoop	movea.l	d1,a0
		cmp.l	cb_Key(a0),d0		correct key ?
		beq.s	FindDone		yep, we found our buffer
		move.l	cb_NextHash(a0),d1	nope, try for another
		bne.s	FindHLoop
FindDone	move.l	d1,d0			return 0 or buffer address
		rts

		END


