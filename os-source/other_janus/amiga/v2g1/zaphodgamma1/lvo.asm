
;************************************************************************
;
; lvos.asm -- define janus.libraries external entry points
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
;
;************************************************************************


	IFND	AZTEC
	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"
	ENDC

	INCLUDE "zaphod.i" 

FUNCDEF	MACRO
	XDEF	_LVO\1
	LIBDEF	_LVO\1
	ENDM


	LIBINIT

	INCLUDE "jfuncs.i"
	INCLUDE "jfuncs2.i"

	END 


