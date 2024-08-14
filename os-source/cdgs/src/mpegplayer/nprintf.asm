
	OPTIMON

;---------------------------------------------------------------------------

	NOLIST

	INCLUDE "exec/types.i"

	LIST

;---------------------------------------------------------------------------

	XREF	_SysBase
	XREF	_LVORawDoFmt

	XDEF	_nprintf

;---------------------------------------------------------------------------

   STRUCTURE PrintData,0
	APTR pd_Buffer
	LONG pd_Offset
	LONG pd_MaxOffset
   LABEL PrintData_SIZEOF

;---------------------------------------------------------------------------

PutCh:
	move.l	pd_Offset(a3),d1
	cmp.l   pd_MaxOffset(a3),d1
	bge.s	1$

	addq.l  #1,pd_Offset(a3)
        move.l  pd_Buffer(a3),a0
        move.b  d0,0(a0,d1.l)
1$
        rts

;---------------------------------------------------------------------------

_nprintf:	; ( buffer, maxLen, template, {values} )

	movem.l	a2/a3/a6,-(sp)
	sub.w 	#PrintData_SIZEOF,a7    ; make room for the PrintData structure
        move.l	a7,a3			; address of PrintData structure
        move.l  $1c(a7),pd_Buffer(a3)	; store pd_Buffer
        clr.l   pd_Offset(a3)		; clear pd_Offset to 0
        move.l  $20(a7),pd_MaxOffset(a3); set pd_MaxOffset

	move.l  $24(a7),a0		; 'template'
	lea	$28(a7),a1		; 'dataStream'
	lea	PutCh(pc),a2		; 'putCharProc'
	move.l	4,a6			; load SysBase
	jsr	_LVORawDoFmt(a6)	; do the work

	move.l  pd_Offset(a7),d0	; return value
	add.w   #PrintData_SIZEOF,a7	; free stack space
	movem.l	(sp)+,a2/a3/a6
        rts

;---------------------------------------------------------------------------

	END
