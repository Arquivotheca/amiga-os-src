; ---------------------------------------------------------------------------------
; func2.asm - glue code for ss.library routine ( test only)
;
; $Locker$
;
; $Id$
;
; $Revision$
;
; $Header$
;
;-----------------------------------------------------------------------------------

	NOLIST
	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i
	INCLUDE "libsupp.i"
	INCLUDE "sslib_def.i"
	LIST

	XREF	_SSBase
	XREF	_Func2

; ---------------------------------------------------------------
; C calling sequence:     ret = Func2(val) ;
;      registers:          d0            a0
; ---------------------------------------------------------------

_Func2:

	; get input value from stack into register
		move.l		4(sp),a0

	; put library base into A2 register
		move.l		_SSBase,a2

	; jump to library table
		LINKLIB FUNC2,a2
		rts

	END

; =========== end of func2.asm ================
