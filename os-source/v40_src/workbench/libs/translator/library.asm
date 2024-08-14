**
**	$Id: library.asm,v 36.3 90/04/09 15:58:44 kodiak Exp $
**
**      translator library code
**
**      (C) Copyright 1986,1990 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	translator

**	Includes

 	INCLUDE "exec/types.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/initializers.i"


**	Exports

	XDEF	TLInitTable


**	Imports

XLVO	MACRO
	XREF	_LVO\1
	ENDM

	XLVO	FreeMem

	XREF	TLName
	XREF	VERSION
	XREF	REVISION
	XREF	TLTranslate

	
**	Locals

CALLLVO	MACRO	* &sysroutine
		jsr	_LVO\1(a6)
	ENDM

*		;------ Translator library node
 STRUCTURE TR,LIB_SIZE
    APTR    tl_ExecBase		; SysBase
    APTR    tl_SegList		; pointer to segment list
    LABEL   TranslatorLibrary_SIZEOF



;------- translator.library/Init -------------------------------------
tlFuncInit:
		dc.w	-1
		dc.w	TLOpen-tlFuncInit
		dc.w	TLClose-tlFuncInit
		dc.w	TLExpunge-tlFuncInit
		dc.w	TLExtFunc-tlFuncInit
		dc.w	TLTranslate+(*-tlFuncInit)
		dc.w	-1

tlStructInit:
	    INITBYTE	LN_TYPE,NT_LIBRARY			; 1.4 redundant
	    INITLONG	LN_NAME,TLName				; 1.4 redundant
	    INITWORD	LIB_REVISION,REVISION
	    INITWORD	LIB_VERSION,VERSION			; 1.4 redundant
	    INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED	; 1.4 redundant
		dc.w		0

TLInitTable:
		dc.l	TranslatorLibrary_SIZEOF
		dc.l	tlFuncInit
		dc.l	tlStructInit
		dc.l	tlInit


;
;   d0	library base
;
tlInit:
		move.l	d0,a1
		move.l	a0,tl_SegList(a1)
		move.l	a6,tl_ExecBase(a1)
		rts


;------ translator.library/OpenLibrary -------------------------------
TLOpen:		
*		;-- Increment open count
		addq.w	#1,LIB_OPENCNT(a6)

		;-- Clear the delayed expnge bit
		bclr	#LIBB_DELEXP,LIB_FLAGS(a6)
		move.l	a6,d0
		rts


;------ translator.library/CloseLibrary ------------------------------
TLClose:
		subq.w  #1,LIB_OPENCNT(a6)
		bne.s	closeDone
		btst	#LIBB_DELEXP,LIB_FLAGS(a6)
		beq.s	closeDone
		jmp	LIB_EXPUNGE(a6)

closeDone:
		moveq	#0,d0
		rts


;------	translator.library/Expunge ------------------------------------
TLExpunge:
		bset	#LIBB_DELEXP,LIB_FLAGS(a6)
		tst.w	LIB_OPENCNT(a6)
		bne.s	closeDone

		move.l	a6,a1
		REMOVE

		move.l	tl_SegList(a6),-(a7)
		move.l	a6,a1
		move.l	tl_ExecBase(a6),a6
		moveq	#0,d0
		move.w	LIB_NEGSIZE(a1),d0
		suba.w	d0,a1
		add.w	LIB_POSSIZE(a1,d0.w),d0
		CALLLVO	FreeMem
		move.l	(a7)+,d0
		rts


;------	translator.library/<reserved> --------------------------------
TLExtFunc:
		moveq	#0,d0		; historically returns 0
		rts

	END
