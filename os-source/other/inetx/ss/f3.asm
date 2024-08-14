; ---------------------------------------------------------------------------------
; func3.asm		ss.library glue code for test function   Manx 5.0d
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
	XREF	_Func3

; ---------------------------------------------------------------
; C calling sequence:     ret = Func3() ;
;      registers:          d0
; ---------------------------------------------------------------

_Func3:

	; put library base into A2 register
		move.l		_SSBase,a2

	; jump to library table
		LINKLIB FUNC3,a2
		rts

	END

; =========== end of func3.asm ================
