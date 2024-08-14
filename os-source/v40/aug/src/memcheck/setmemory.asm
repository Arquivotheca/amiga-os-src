;*****************************************************************************/
; setmemory.asm
; walk through the memory list an mung it
; Written by David N. Junod
;   16-Sep-92  Created
;*****************************************************************************/

	INCLUDE "exec/types.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/memory.i"
	INCLUDE "exec/execbase.i"
	INCLUDE "globaldata.i"

;*****************************************************************************/

	XDEF	_SetMemory

;*************************************************************************************

	XREF	_LVOForbid
	XREF	_LVOPermit
	XREF	_MemoryCorrupt
	XREF	_MungMem
	XREF	_kprintf

;*************************************************************************************

	XREF	badsum

;*****************************************************************************/
; static void ASM SetAllocatedMemory
;	(REG (a0) struct GlobalData *gd, REG (a1) ULONG *ustart, REG (d0) ULONG size);
;*****************************************************************************/

_SetAllocatedMemory:
	movem.l	d2-d5,-(a7)		; save registers
	move.w	mci_PreSize(a0),d3	; get prewall size - WARNING: WORD
	move.w	mci_PostSize(a0),d4	; get postwall size - WARNING: WORD
	move.b	mci_FillChar(a0),d5	; get fill character - WARNING: BYTE

	; load d2 with $C0DEF00D in two steps
 	move.l	#$C0DE0000,d2		; do it in two steps so magic cookie
	move.w	#$F00D,d2		; doesn't appear in this code

	; get number of double longwords
	addq.l	#7,d0
	lsr.l	#3,d0
	bra.s	endloop0		; get things rolling

__SAM_1:
	cmp.l	(a1),d2			; search for the MemCheck cookie
	beq.s	__SAM_2			; found cookie, so exit
	addq.l	#8,a1			; skip double longword
endloop0:
	dbra	d0,__SAM_1		; decrement lower word of counter
	sub.l	#$10000,d0		; decrement upper word of counter
	bpl.s	__SAM_1			; if counter > 0, loop
	bra.s	__SAM_3

__SAM_2:
	move.l	a1,a0			; save pointer to MemCheck structure

	; Set pre-wall
	add.l	#INFOSIZE,a1		; skip past the MemCheck structure
	move.w	d3,d1			; get presize
	bra.s	endloop2		; enter loop
__SAM_PreWall:
	move.b	d5,(a1)+		; move the fill character into the wall
endloop2:
	dbra	d1,__SAM_PreWall

	; Set post-wall
	add.l	mc_Size(a0),a1		; add the memory block size
	move.w	d4,d1			; use the post-wall size
	bra.s	endloop3
__SAM_PostWall:
	move.b	d5,(a1)+		; move the fill character into the wall
endloop3:
	dbra	d1,__SAM_PostWall

	; double-long word align the pointer before continuing loop
        move.l	a1,d1
        addq.l	#7,d1
        and.b	#$F8,d1
        move.l	d1,a1
        bra.s	__SAM_1

__SAM_3:
	movem.l	(a7)+,d2-d5
	rts

;*****************************************************************************/
; void ASM SetMemory
;	(REG (a0) struct GlobalData *gd, REG (d0) ULONG flags);
;*****************************************************************************/
;
; ADDRESS REGISTERS
;	A2	Current MemHeader
;	A3	Current MemChunk
;	A5	GlobalData
;	A6	SysBase
;
; DATA REGISTERS
;	D2	flags
;	D3	Sum
;	D4	fstart
;	D5	fsize
;	D6	ustart
;	D7	usize

_SetMemory:
	movem.l	d2/d3/a2-a6,-(a7)	; save the registers on the stack
	move.l	a0,a5			; save GlobalData
	move.l	d0,d2			; save flags
	move.l	gd_SysBase(a5),a6	; we need to use SysBase
	lea	MemList(a6),a2		; get the address of the memory list

	; forbid multi-tasking for a moment
	jsr	_LVOForbid(a6)

MemHeader_Top:
	; Get the MemHeader
	TSTNODE	a2,a2
	beq	__SM_Exit

	; See if there is allocated memory before the first chunk
	btst.b	#1,d2			; if (flags & CHECK_ALLOC)
	beq.s	MemChunk_Top
	move.l	MH_LOWER(a2),d1		; if (mh->mh_First != mh->mh_Lower)
	move.l	MH_FIRST(a2),d0
	cmp.l	d1,d0
	beq.s	MemChunk_Top
	sub.l	d1,d0			; usize = mh->mh_First - mh->mh_Lower
	move.l	a3,a1
	move.l	a5,a0
	jsr	_SetAllocatedMemory	; a0=gd, a1=ptr, d0=size

MemChunk_Top:
	; Clear the sum
	moveq.l	#0,d3

	; Step through the list of Memory Chunks
	lea	MH_FIRST(a2),a3
MemChunk_Loop:
	move.l	(a3),d4			; set fstart
	beq.s	MemHeader_Bottom	; done with this MemHeader
	move.l	d4,a3

	; Sum the bytes
	move.l	MC_BYTES(a3),d5		; set fsize
	add.l	d5,d3			; sum bytes

	; Check for allocated memory
	move.l	(a3),d0			; if (mc->mc_Next)
	beq.s	MemChunk_Alloc
	btst.b	#1,d2			; if (flags & CHECK_ALLOC)
	beq.s	MemChunk_Free
	move.l	d4,d6			; ustart = fstart + fsize
	add.l	d5,d6
	sub.l	d6,d0			; usize = mc->mc_Next - ustart
	move.l	d0,d7
	move.l	d6,a1			; move ustart to a1
	move.l	a5,a0			; move GlobalData to a0
	jsr	_SetAllocatedMemory	; a0=gd, a1=ustart, d0=usize

MemChunk_Alloc:
	; Check for allocated memory
	btst.b	#1,d2			; if (flags & CHECK_ALLOC)
	beq	MemChunk_Free
	move.l	d4,d6			; ustart = fstart + fsize
	add.l	d5,d6
	move.l	MH_UPPER(a2),d0		; if (ustart < mh->mh_Upper)
	cmp.l	d6,d0
	bge.s	MemChunk_Free
	move.l	MH_UPPER(a2),d7		; usize = mh->mh_Upper - ustart
	sub	d6,d7
	move.l	d7,d0			; move usize to d0
	move.l	d6,a1			; move ustart to a1
	move.l	a5,a0			; move GlobalData to a0
	jsr	_SetAllocatedMemory	; a0=gd, a1=ustart, d0=usize

MemChunk_Free:
	; Set free memory
	btst.b	#0,d2			; if (flags & CHECK_FREE)
	beq.s	MemChunk_Bottom
	move.l	MC_BYTES(a3),d0		; if (mc->mc_Bytes > 8)
	moveq.l	#8,d1
	cmp.l	d1,d0
	bls.b	MemChunk_Bottom
	lea	$8(a3),a0		; mc + 8
	subq.l	#8,d0			; mc->mc_Bytes - 8
	move.l	#$DEADBEEF,d1
	jsr	_MungMem		; a0=ptr d0=size d1=mungvalue

MemChunk_Bottom:
	; Handle the next Memory Chunk
	bra.s	MemChunk_Loop

MemHeader_Bottom:
	; Compare the sum
	cmp.l	MH_FREE(a2),d3
	beq	MemHeader_Top		; get the next MemHeader

	; Memory list is corrupt
	jsr	_MemoryCorrupt

	; Bad sum
	move.l	d3,-(a7)
	pea	badsum(pc)
	jsr	_kprintf
	lea	8(a7),a7		; restore the stack

	bra	MemHeader_Top

__SM_Exit:
	; permit multi-tasking again
	jsr	_LVOPermit(a6)

	movem.l	(a7)+,d2/d3/a2-a6	; restore the registers
	rts
