**
**	$Id: macros.i,v 1.2 91/08/09 21:25:05 bryce Exp $
**
**	Generic device driver: Local macro defintions & handy constants
**
**	(C) Copyright 1989 Commodore-Amiga, Inc.
**	    All Rights Reserved
**

		OPTIMON $FF000000   ;turn on CAPE optimizations (MOVEM!!!)


DEBUG_DETAIL	SET 0
AZ_LYZER	SET 0
ABSEXECBASE	EQU 4

		INCLUDE "exec/macros.i"


;
; Write to location $2FE, a trigger for the analyzer
;
AZ_TRIG 	MACRO	;level
		IFGE   AZ_LYZER-\1
		  MOVE.W #$5555,$2FE
		ENDC
		ENDM
;
; Write to location $2FE, a trigger for the analyzer.  Then
; stop the CPU! (Supervisor mode only)
;
AZ_STOP 	MACRO	;level
		IFGE   AZ_LYZER-\1
		  NOP
		  MOVE.W #$5555,$2FE
		  STOP	#$2000
		  NOP
		ENDC
		ENDM
