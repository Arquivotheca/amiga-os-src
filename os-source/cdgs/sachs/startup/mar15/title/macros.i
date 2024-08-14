; :ts=8 
*
*	macros.i - commonly used macros in the ss.lib
*
*	William A. Ware			C206
*
*************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY			*
*   Copyright (C) 1991, Silent Software Incorporated.			*
*   All Rights Reserved.						*
*************************************************************************

  
	IFND	SS_MACROS_I
SS_MACROS_I	SET	1

;
; LSYSBASE	
;		Moves the system base into a6 - in the least amount of
;		instructions.
;
;		NOTE:	This is an accepted way of doing it.
;
LSYSBASE	MACRO
			movea.l	(4).w,a6
		ENDM



	ENDC ;	SS_MACROS_I