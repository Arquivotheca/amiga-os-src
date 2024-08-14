;***************************************************************************
;
; Project:	Sapphire
; File:		defs.asm
; Date:		29 March 1993
; Status:	Draft
;
; Description:	contains the generally used equates
;
; Decisions:	-
;
; HISTORY		AUTHOR COMMENTS
; 26 November 1992	creation H.v.K.
;
;***************************************************************************

%SET (WEARNES,1)
%SET (DSA_PCDT,2)
%SET (COMMODORE,3)
%SET (APPLICATION,%COMMODORE)

;equates for the process execution states (should be the same as in defs.h)
BUSY		equ	0
READY	    	equ	1
ERROR		equ	2
PROCESS_READY	equ	3
	
;equates for the compare_time function (should be the same as in defs.h)
SMALLER		equ	0
EQUAL		equ	1
BIGGER		equ	2
