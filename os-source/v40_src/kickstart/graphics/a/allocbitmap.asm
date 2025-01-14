*******************************************************************************
*
*	$Id: allocbitmap.asm,v 39.28 93/03/19 07:32:21 chrisg Exp $
*
*******************************************************************************

	include	"exec/types.i"
	include	"/gfxbase.i"
	include	"/gfx.i"
	include	"/macros.i"
	include	"exec/memory.i"

	SECTION graphics

	OPTIMON

	xref	_LVOAllocMem,_LVOFreeMem,_LVOAllocVec,_LVOFreeVec


	xdef	_AllocBitMapData,_FreeBitMapData
	xdef	_AllocBitMap,_FreeBitMap
	xdef	_GetBitMapAttr

******* graphics.library/GetBitMapAttr ****************************************
*
*   NAME
*	GetBitMapAttr -- Returns information about a bitmap (V39)
*
*
*   SYNOPSIS
*	value=GetBitMapAttr(bitmap,attribute_number);
*	  d0	                 a0       d1
*
*	ULONG GetBitMapAttr(struct BitMap *,ULONG);
*
*   FUNCTION
*	Determines information about a bitmap. This function should be used
*	instead of reading the bitmap structure fields directly. This will
*	provide future compatibility.
*
*   INPUTS
*	bm  =  A pointer to a BitMap structure.
*
*	attribute_number = A number telling graphics which attribute
*	                   of the bitmap should be returned:
*
*	                BMA_HEIGHT returns the height in pixels
*	                BMA_WIDTH  returns the width in pixels.
*	                BMA_DEPTH  returns the depth. This is the number of
*	                        bits which are required to store the information
*	                        for one pixel in the bitmap.
*	                BMA_FLAGS  returns a longword bitfield describing
*	                        various attributes which the bitmap may have.
*	                        Currently defined flags are BMF_DISPLAYABLE,
*	                        BMF_INTERLEAVED (see AllocBitMap()). The flag
*	                        BMF_STANDARD returns will be set if the
*	                        bitmap is represented as planar data in Amiga
*	                        Chip RAM.
*
*   BUGS
*
*   NOTES
*
*	Unknown attributes are reserved for future use, and return zero.
*
*	BMF_DISPLAYABLE will only be set if the source bitmap meets all of the
*	required alignment restrictions. A bitmap which does not meet these
*	restrictions may still be displayable at some loss of efficiency.
*
*	Size values returned by this function may not exactly match the values
*	which were passed to AllocBitMap(), due to alignment restrictions.
*
*   SEE ALSO
*	AllocBitMap()
*
*********************************************************************************
_GetBitMapAttr:
	moveq	#0,d0
	subq.l	#4,d1
	bpl.s	not_height
	move.w	bm_Rows(a0),d0
	rts
not_height:
	subq.l	#4,d1
	bpl.s	not_depth
	move.b	bm_Depth(a0),d0
	rts
not_depth:
	cmp.w	#UNLIKELY_WORD,bm_Pad(a0)
	bne.s	not_interleaved1
	cmp.b	#2,bm_Depth(a0)
	blt.s	not_interleaved1
	move.l	bm_Planes+4(a0),a1
	sub.l	bm_Planes(a0),a1
	cmp.w	#0,a1
	beq.s	not_interleaved1
	cmp.w	bm_BytesPerRow(a0),a1
	bhi.s	not_interleaved1
	bset	#BMB_INTERLEAVED,d0
not_interleaved1:
	subq.l	#4,d1
	bpl.s	not_width
	move.w	bm_BytesPerRow(a0),d1
	tst.l	d0
	beq.s	no_il2
	move.l	a1,d1
no_il2:
; do bytesperrow / 8 = pixels
	lsl.w	#3,d1
	move.w	d1,d0
	rts
not_width:
; return flags
	subq.l	#4,d1
	bpl.s	unknown_attr
	bset	#BMB_STANDARD,d0
	movem.l	d2/d3,-(a7)
	moveq	#0,d2
	moveq	#2,d1
	move.b	gb_MemType(a6),d2
	move.l	gb_bwshifts(a6),a1
	move.b	0(a1,d2.w),d3
	asl.b	d3,d1
	subq.w	#1,d1
	move.w	bm_BytesPerRow(a0),d2
	and	d1,d2
	bne.s	not_displayable
	moveq	#0,d2
	move.b	bm_Depth(a0),d2
	subq	#1,d2
	lea	bm_Planes(a0),a0
1$:	move.l	(a0)+,d3
	and	d1,d3
	dbne	d2,1$
	bne.s	not_displayable
	bset	#BMB_DISPLAYABLE,d0
not_displayable:
	movem.l	(a7)+,d2/d3
	rts
unknown_attr:
	moveq	#0,d0
	rts

; AllocBitMapData
; initialize pad field of bitmap and allocates planes. returns non-zero on error.
;	flags = BMF_CLEAR to specify that the allocated raster should be
;		filled with color 0.
;
;		BMF_DISPLAYABLE to specify that this bitmap data should
;		be allocated in such a manner that it can be displayed.
;		Displayable data has more severe alignment restrictions
;		than non-displyable data.
;
;		BMF_INTERLEAVED tells graphics that you would like your
;		bitmap to be allocated with one large chunk of display
;		memory for all bitplanes. This minimizes color flashing
;		on deep displays.


_AllocBitMapData:
; first, align sizex
	movem.l	d2-d4/a2/a3/a6,-(a7)
	moveq	#16,d4
	btst	#BMB_DISPLAYABLE,d3
	beq.s	1$
	swap	d3
	clr.w	d3
	move.b	gb_MemType(a6),d3
	move.l	gb_bwshifts(a6),a1
	move.b	0(a1,d3.w),d3
	asl.b	d3,d4
	swap	d3
1$:	subq.w	#1,d4
	add.w	d4,d0
	addq.w	#1,d4
	neg.w	d4		; d4=$ffff-d4
	and.w	d4,d0
	move	d1,bm_Rows(a0)
	lsr.w	#3,d0
	cmp	#2,d2
	blt.s	no_il
	btst	#GFXB_BIG_BLITS,gb_ChipRevBits0(a6)	; don't do on stupid old chips
	beq.s	no_il
	cmp	#4093,d0		; don't do for bitmaps wider than 4094
	bgt.s	no_il
	cmp	#4093,d1
	bgt.s	no_il
	btst	#BMB_INTERLEAVED,d3
	bne.s	do_interleaved
no_il:
	move.w	d0,bm_BytesPerRow(a0)
	mulu	d1,d0
	clr.b	bm_Depth(a0)	; count up bm_depth for recovery reasons
	subq.w	#1,d2		; DBRA adjustment
	move.l	gb_ExecBase(a6),a6
	move.l	d0,d4		; allocsize
	move.l	#MEMF_CHIP,d1
	btst	#BMB_CLEAR,d3
	beq.s	2$
	or.l	#MEMF_CLEAR,d1
2$:	move.l	d1,d3
	lea	bm_Planes(a0),a2
	move.l	a0,a3
3$:	move.l	d4,d0
	move.l	d3,d1
	jsr	_LVOAllocMem(a6)
	move.l	d0,(a2)+
	beq.s	alloc_failure
	addq.b	#1,bm_Depth(a3)
	dbra	d2,3$
	movem.l	(a7)+,d2-d4/a2/a3/a6
	moveq	#0,d0	
	rts
alloc_failure:
	move.l	a3,a0
	movem.l	(a7)+,d2-d4/a2/a3/a6
	bsr.s	_FreeBitMapData
	moveq	#-1,d0
	rts

do_interleaved:
	movem.l	d0/d1/a6,-(a7)
	move.l	d0,d4
	move.l	a0,a3
	move.b	d2,bm_Depth(a0)
	mulu	d2,d0
	move.w	d0,bm_BytesPerRow(a0)
	mulu	bm_Rows(a0),d0
	move.l	#MEMF_CHIP,d1
	btst	#BMB_CLEAR,d3
	beq.s	2$
	or.l	#MEMF_CLEAR,d1
2$:	move.l	gb_ExecBase(a6),a6
	jsr	_LVOAllocMem(a6)
	tst.l	d0
	bne.s	doi_1
	movem.l	(a7)+,d0/d1/a6
	move.l	a3,a0
	bra	no_il
doi_1:	subq	#1,d2
	lea	bm_Planes(a3),a2
	ext.l	d4
3$:	move.l	d0,(a2)+
	add.l	d4,d0
	dbra	d2,3$
	move.w	#UNLIKELY_WORD,bm_Pad(a3)
	lea	12(a7),a7
	movem.l	(a7)+,d2-d4/a2/a3/a6
	moveq	#0,d0
	rts


_FreeBitMapData:
; void __asm FreeBitMapData(register __a0 struct BitMap *bm);
	movem.l	d2/d3/a2/a6,-(a7)
	moveq	#0,d2
	move.b	bm_Depth(a0),d2
	cmp.w	#UNLIKELY_WORD,bm_Pad(a0)
	bne.s	not_interleaved
; this bitmap is interleaved, so we had better treat it as a one bit deep bitmap
	moveq	#1,d2
not_interleaved:
	move.l	gb_ExecBase(a6),a6
	move.w	bm_BytesPerRow(a0),d3
	mulu	bm_Rows(a0),d3
	lea	bm_Planes(a0),a2
	bra.s	2$
1$:	move.l	(a2)+,a1
	move.l	d3,d0
	jsr	_LVOFreeMem(a6)
2$:	dbra	d2,1$
	movem.l	(a7)+,d2/d3/a2/a6	
	rts

******* graphics.library/AllocBitMap ******************************************
*
*   NAME
*	AllocBitMap -- Allocate a bitmap and attach bitplanes to it. (V39)
*
*
*   SYNOPSIS
*	bitmap=AllocBitMap(sizex,sizey,depth, flags, friend_bitmap)
*	                   d0    d1    d2     d3       a0
*
*	struct BitMap *AllocBitMap(ULONG,ULONG,ULONG,ULONG, struct BitMap *);
*
*   FUNCTION
*	Allocates and initializes a bitmap structure. Allocates and initializes
*	bitplane data, and sets the bitmap's planes to point to it.
*
*   INPUTS
*	sizex = the width (in pixels) desired for the bitmap data.
*
*	sizey = the height (in pixels) desired.
*
*	depth = the number of bitplanes deep for the allocation.
*		Pixels with AT LEAST this many bits will be allocated.
*
*	flags = BMF_CLEAR to specify that the allocated raster should be
*	        filled with color 0.
*
*	        BMF_DISPLAYABLE to specify that this bitmap data should
*	        be allocated in such a manner that it can be displayed.
*	        Displayable data has more severe alignment restrictions
*	        than non-displayable data in some systems. 
*
*	        BMF_INTERLEAVED tells graphics that you would like your
*	        bitmap to be allocated with one large chunk of display
*	        memory for all bitplanes. This minimizes color flashing
*	        on deep displays. If there is not enough contiguous RAM
*		for an interleaved bitmap, graphics.library will fall
*		back to a non-interleaved one.
*
*	        BMF_MINPLANES causes graphics to only allocate enough space
*	        in the bitmap structure for "depth" plane pointers. This
*		is for system use and should not be used by applications use
*		as it is inefficient, and may waste memory.
*
*	friend_bitmap = pointer to another bitmap, or NULL. If this pointer
*	        is passed, then the bitmap data will be allocated in
*	        the most efficient form for blitting to friend_bitmap.
*
*   BUGS
*
*   NOTES
*	When allocating using a friend bitmap, it is not safe to assume
*	anything about the structure of the bitmap data if that friend
*	BitMap might not be a standard amiga bitmap (for instance, if
*	the workbench is running on a non-amiga display device, its
*	Screen->RastPort->BitMap won't be in standard amiga format.
*	The only safe operations to perform on a non-standard BitMap are:
*
*	        - blitting it to another bitmap, which must be either a standard
*	          Amiga bitmap, or a friend of this bitmap.
*
*	        - blitting from this bitmap to a friend bitmap or to a standard
*	          Amiga bitmap.
*
*	        - attaching it to a rastport and making rendering calls. 
*
*	Good arguments to pass for the friend_bitmap are your window's
*	RPort->BitMap, and your screen's RastPort->BitMap.
*	Do NOT pass &(screenptr->BitMap)!
*
*	BitMaps not allocated with BMF_DISPLAYABLE may not be used as
*	Intuition Custom BitMaps or as RasInfo->BitMaps.  They may be blitted
*	to a BMF_DISPLAYABLE BitMap, using one of the BltBitMap() family of
*	functions.
*
*   SEE ALSO
*	FreeBitMap()
*
********************************************************************************
_AllocBitMap:
	movem.l	d3/d4,-(a7)
	cmp.l	#256,d3
	bhs.s	abm_failure
	movem.l	d0/d1/a6,-(a7)
	cmp	#0,a0		; friend_bitmap=0?
	beq.s	abm_1
	cmp.b	bm_Depth(a0),d2
	bne.s	abm_1
	cmp.w	#UNLIKELY_WORD,bm_Pad(a0)
	bne.s	abm_1
	bset	#BMB_INTERLEAVED,d3
abm_1:
	move.l	gb_ExecBase(a6),a6
	moveq	#bm_Planes,d0
	move	d2,d1
	btst	#BMB_MINPLANES,d3
	bne.s	2$
	moveq	#bm_SIZEOF,d0
	subq	#8,d1
	ble.s	11$
2$:	add	d1,d1
	add	d1,d1
	add	d1,d0
11$	move.l	#MEMF_CLEAR,d1
	jsr	_LVOAllocVec(a6)
	move.l	d0,a0
	tst.l	d0
	movem.l	(a7)+,d0/d1/a6
	beq.s	abm_failure
	move.l	a0,-(a7)
	bsr	_AllocBitMapData
	tst.l	d0
	bne.s	abm_failure_1
	move.l	(a7)+,d0
	movem.l	(a7)+,d3/d4
	rts
abm_failure_1:
	move.l	(a7)+,a1
	move.l	a6,-(a7)
	move.l	gb_ExecBase(a6),a6
	jsr	_LVOFreeVec(a6)
	move.l	(a7)+,a6
abm_failure:
	moveq	#0,d0
	movem.l	(a7)+,d3/d4
	rts
		
******* graphics.library/FreeBitMap ****************************************
*
*   NAME
*	FreeBitMap -- free a bitmap created by AllocBitMap (V39)
*
*   SYNOPSIS
*	FreeBitMap(bm)
*	           a0
*
*	VOID FreeBitMap(struct BitMap *)
*
*   FUNCTION
*	Frees bitmap and all associated bitplanes
*
*   INPUTS
*	bm  =  A pointer to a BitMap structure. Passing a NULL-pointer
*	       (meaning "do nothing") is OK.
*
*   BUGS
*
*   NOTES
*	Be careful to insure that any rendering done to the bitmap has
*	completed (by calling WaitBlit()) before you call this function.
*
*   SEE ALSO
*	AllocBitMap()
*
*********************************************************************************
_FreeBitMap:
	move.l	a0,-(a7)
	beq.s	no_bm
	bsr	_FreeBitMapData
	move.l	(a7),a1
	move.l	a6,(a7)
	move.l	gb_ExecBase(a6),a6
1$:	jsr	_LVOFreeVec(a6)
	move.l	(a7)+,a6
	rts
no_bm:	addq.l	#4,a7
	rts
