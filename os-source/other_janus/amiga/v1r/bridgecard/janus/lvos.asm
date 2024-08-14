
;************************************************************************
;
; lvos.asm -- define janus.libraries external entry points
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
;
;************************************************************************

	INCLUDE "exec/types.i" 
	INCLUDE "exec/libraries.i"

		LIBINIT

FUNCDEF 	MACRO
		XDEF	_LVO\1
		LIBDEF	_LVO\1
		ENDM

	INCLUDE "jfuncs.i"


	END 

