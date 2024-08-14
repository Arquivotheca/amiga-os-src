; ---------------------------------------------------------------------------------
; func0.asm	 - Glue code for ss.library routine Func0 (test only)
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
	XREF	_Func0

; ---------------------------------------------------------------
; C calling sequence:     ret = Func0() ;
;      registers:          d0
; ---------------------------------------------------------------

_Func0:

	; put library base into A2 register
		move.l		_SSBase,a2

	; make routine call
		LINKLIB FUNC0,a2
		rts

	END

; =========== end of func0.asm ================
