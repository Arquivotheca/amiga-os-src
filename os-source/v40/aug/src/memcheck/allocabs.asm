;*************************************************************************************
; allocabs.asm
;*************************************************************************************

	INCLUDE "exec/types.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/memory.i"
	INCLUDE "globaldata.i"

;*************************************************************************************

	XDEF	_nAllocAbs

;*************************************************************************************

	XREF	_LVOForbid
	XREF	_LVOPermit

;*************************************************************************************

	XREF	_gd

;*************************************************************************************
; ULONG ASM nAllocAbs (REG (d0) ULONG size, REG (a1) APTR address, REG (a6) struct Library *SysBase);
;*************************************************************************************

_nAllocAbs:
	jsr	_LVOForbid(a6)
	movem.l	d2/a2/a3,-(a7)
	move.l	d0,d2			; save the size
	move.l	a1,a3			; save the address
	move.l	_gd,a2			; get a pointer to our global data

	; Try calling system AllocAbs
	move.l	gd_AllocAbs(a2),a0
	jsr	(a0)
	tst.l	d0
	bne.b	_Done

	; That failed, so look at our list
	move.l	d2,d0			; size
	move.l	a3,a1			; address
	move.l	gd_MemHeader(a2),a0	; memheader
	jsr	_AllocAbsolute

_Done:
	movem.l	(a7)+,d2/a2/a3
	jmp	_LVOPermit(a6)

;*************************************************************************************

CLEAR       MACRO
            MOVEQ.L #0,\1
            ENDM

;*************************************************************************************
; VOID * ASM AllocAbsolute (REG (d0) ULONG size, REG (a1) APTR addr, REG (a0) struct MemHeader *);
;*************************************************************************************

_AllocAbsolute:
	;------ round address to block boundary add delta to size
	MOVE.L  A1,D1
	AND.L   #MEM_BLOCKMASK,D1
	SUB.L   D1,A1
	ADD.L   D1,D0			; Inc byteSize

	;------ round size up to next block
	ADDQ.L  #MEM_BLOCKMASK,D0
	AND.W   #~MEM_BLOCKMASK,D0

	MOVE.L  D0,D1
	;[D1=byteSize]
	MOVEQ	#0,D0			; Clear out result

;*************************************************************************************
; void * ASM AllocAbsInternal (REG (a0) struct MemHeader *, REG (d1) ULONG size, REG (a1) APTR addr);
;*************************************************************************************
*
*	A0-MemHeader
*	A1-startcurrent (D3)
*	A2-prevchunk
*	A3-startwant
*	D0-byteSize
*	D1-***UNUSED***
*	D2-endwant
*	D3-startcurrent (D3)
*	D4-endcurrent
*
_AllocAbsInternal:

	MOVEM.L D2-D4/A2-A3,-(SP)
	MOVE.L	D1,D0

	;------ Do a quick check for space:
	CMP.L   MH_FREE(A0),D0
	BHI.S    aa_fail

	MOVE.L  A1,A3		;startwant
	MOVE.L  A1,D2
	ADD.L   D0,D2		;loc+size=endwant

	LEA	MH_FIRST(A0),A2	;prevchunk

	;------ scan list until chunk+chunklen >= endwant
aa1:
	MOVE.L  (A2),D3
	BEQ.S   aa_fail		* end of list
	MOVE.L  D3,A1
	MOVE.L  MC_BYTES(A1),D4
	ADD.L   D3,D4
	CMP.L   D2,D4
	BCC.S   aa_found	;BHS
	MOVE.L  A1,A2	;------ bump next pointer
	BRA.S   aa1

	;------ Found it!
aa_found:
	;------ see if the chunk is large enough
	CMP.L   A3,D3
	BHI.S   aa_fail

	;------ we found the right region.  update the free mem count
	SUB.L   D0,MH_FREE(A0)

	;------ see if we need to split the end of the chunk
	SUB.L   D2,D4		;endcurrent-endwant
	BNE.S   aa_splitsucc

	;------ the ends are the same.  Get the next ptr.
	MOVE.L  (A1),A0
	BRA.S   aa_checkhead

aa_splitsucc:
	;------ get the address of the new node
	LEA     0(A3,D0.L),A0	;just past end of new block
	MOVE.L  (A1),(A0)	;(startcurrent),(newblock)
	MOVE.L  A0,(A1)		;newblock,(startcurrent)
	MOVE.L  D4,MC_BYTES(A0)	;difference is newsize

aa_checkhead:
	CMP.L   A3,D3		;startwant vs. startcurrent
	BEQ.S   aa_removenode

	SUB.L   A3,D3
	NEG.L   D3
	MOVE.L  D3,MC_BYTES(A1)
	BRA.S   aa_done

aa_removenode:
	;------ update the previous pointer
	MOVE.L  A0,(A2)

aa_done:
	MOVE.L  A3,D0
aa_end:
	MOVEM.L (SP)+,D2-D4/A2-A3
	RTS

aa_fail:
	CLEAR   D0
	BRA.S   aa_end
