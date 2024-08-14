; ---------------------------------------------------------------------------------
; libsupp.i - macros for ss.library 
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

	IFND	LIBSUPP_I
LIBSUPP_I	SET 1


CLEAR 		MACRO
	moveq	#0,\1
	ENDM

LINKSYS		MACRO
	LINKLIB	_LVO\1,\2
	ENDM

CALLSYS		MACRO
	CALLLIB	_LVO\1
	ENDM

XLIB		MACRO
	XREF	_LVO\1
	ENDM

; =============== end file libsupp.i  ============

	ENDC	; LIBSUPP_I