*
* $Id: wbfind.asm,v 40.1 93/06/07 16:50:44 mks Exp $
*
* WBFind initial startup romtag
*
        INCLUDE "exec/types.i"
	INCLUDE "exec/resident.i"

	INCLUDE "wbfind_rev.i"
*
	XREF	endSkip
	XREF	_Entry
romtag:
	DC.W	RTC_MATCHWORD		;(50) UWORD RT_MATCHWORD
	DC.L	romtag			;(52) APTR  RT_MATCHTAG
	DC.L	endSkip			;(56) APTR  RT_ENDSKIP
	DC.B	RTF_AFTERDOS		;(5A) UBYTE RT_FLAGS
	DC.B	VERSION			;(5B) UBYTE RT_VERSION
	DC.B	0			;(5C) UBYTE RT_TYPE
	DC.B	-110			;(5D) BYTE  RT_PRI
	DC.L	name			;(5E) APTR  RT_NAME
	DC.L	name			;(62) APTR  RT_IDSTRING
*	DC.L	Seglist			;(66) APTR  RT_INIT
	DC.L	_Entry			;(66) APTR  RT_INIT
					;(6A) LABEL RT_SIZE

name:	VSTRING
	cnop	0,4
*
*******************************************************************************
*
* This is the patch itself...
*
		xref	_DoPatch		; (a0,a1,d0)
_PatchData:	xdef	_PatchData
OldFunction:	dc.l	0
NewFunction:	lea	_PatchData(pc),a0	; Get patch data pointer
		jmp	_DoPatch		; Do the patch in C...
_EndPatch:	xdef	_EndPatch
*
*******************************************************************************

        END
