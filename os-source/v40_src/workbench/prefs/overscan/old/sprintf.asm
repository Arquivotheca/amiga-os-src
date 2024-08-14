		BASEREG	BLink
		SECTION tex,code

	XDEF @Chk_Abort
	XDEF @chkabort
	XDEF _sprintf
	XREF _AbsExecBase
	XREF _LVORawDoFmt
	XREF	_UtilityBase

_sprintf:	; ( ostring, format, {values} )
	movem.l a2/a3/a6,-(sp)

	move.l	4*4(sp),a3       ;Get the output string pointer
	move.l	5*4(sp),a0       ;Get the FormatString pointer
	lea.l	6*4(sp),a1       ;Get the pointer to the DataStream
	lea.l	stuffChar(pc),a2
	move.l	_AbsExecBase,a6
	jsr	_LVORawDoFmt(a6)

	movem.l (sp)+,a2/a3/a6
	rts

;------ PutChProc function used by RawDoFmt -----------
stuffChar:
	move.b	d0,(a3)+        ;Put data to output string
	rts



*******************************************************************************
*
* This code assumes that _UtilityBase(a4) is the place where the base
* pointer to the utility.library is stored.
*

*******************************************************************************
*
* __CXD33 replacement that uses utility.library
*
		XREF	_LVOSDivMod32
		XDEF	__CXD33			; Signed divide

__CXD33:	move.l	a6,-(sp)		; Save your base pointer
		move.l	_UtilityBase(a4),a6	; Get utility base pointer
		jsr	_LVOSDivMod32(a6)	; Do the divide
PopA6Rts:	movea.l	(A7)+,A6
@Chk_Abort:
@chkabort:
		rts
*
*******************************************************************************
*
* __CXM33 replacement that uses utility.library
*
		XREF	_LVOSMult32
		XDEF	__CXM33			; Signed multiply

__CXM33:	move.l	a6,-(sp)		; Save your base pointer
		move.l	_UtilityBase(a4),a6	; Get utility base pointer
		jsr	_LVOSMult32(a6)		; Do the multiply
		bra.s	PopA6Rts
*
*******************************************************************************
