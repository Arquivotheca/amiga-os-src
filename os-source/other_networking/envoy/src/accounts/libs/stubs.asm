
; replacements for standard lc.lib routines

	OPTIMON

;---------------------------------------------------------------------------

	NOLIST

	INCLUDE	"accountsbase.i"

	LIST

;---------------------------------------------------------------------------

	XREF	_LVORawDoFmt
	XREF	_LVOCreateEntityA
	XREF	_LVOAllocTransactionA

;	XDEF	_CreateEntity
	XDEF	_AllocTransaction

	XDEF	__sprintf

	XDEF	@stccpy

;---------------------------------------------------------------------------

;_CreateEntity:	move.l	a6,-(sp)
;		movea.l	ACNTS_NIPCBase(a6),a6
;		lea	$8(sp),a0
;		jsr	_LVOCreateEntityA(a6)
;		movem.l	(sp)+,a6
;		rts

_AllocTransaction:
		move.l	a6,-(sp)
		movea.l ACNTS_NIPCBase(a6),a6
		lea	$8(sp),a0
		jsr	_LVOAllocTransactionA(a6)
		movem.l	(sp)+,a6
		rts

__sprintf:	; ( string, format, {values} )
		movem.l	a2/a3/a4/a6,-(sp)
		move.l	5*4(sp),a3

		move.l	6*4(sp),a0
		lea	7*4(sp),a1
		lea	stuffChar(pc),a2
		move.l	ACNTS_SysBase(a6),a6
		jsr	_LVORawDoFmt(a6)

		movem.l	(sp)+,a2/a3/a4/a6
		rts

stuffChar:
		move.b	d0,(a3)+
		rts

;-----------------------------------------------------------------------
; stccpy        - Smaller version of the copy routine...
;  a0 - destination
;  a1 - source
;  d0 - destination length

@stccpy:        subq.l  #1,d0                   ; One less than given...
                bmi.s   2$                      ; if we end up with no bytes!
1$:             move.b  (a1)+,(a0)+             ; Copy the data...
                dbeq.s  d0,1$                   ; Loop for max or null...
                clr.b   -(a0)                   ; Make sure NULL terminated
2$:             rts

;---------------------------------------------------------------------------

		END
