; ---------------------------------------------------------------------------------
; func1.asm - Glue code for ss.library routine (test only!)
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
	XREF	_Func1

; ---------------------------------------------------------------
; C calling sequence:     ret = Func1() ;
;      registers:          d0
; ---------------------------------------------------------------

_Func1:

	; put library base into A2 register
		move.l		_SSBase,a2

	; jump to library table
		LINKLIB FUNC1,a2
		rts

	END

; =========== end of func1.asm ================
