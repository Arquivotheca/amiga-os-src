	NAME	FILLR2
	TITLE	Fill out ROM to AT compatibility section (0F000:0E000)
	PAGE	59,132
;***************************************************************************
;* Copyright (c) 1985-1989 Phoenix Technologies Ltd.  This program	   *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent.			   *
;***************************************************************************


;***************************************************************************
;	Revision Information	$Revision:   3.0  $	
;				$Date:   25 Jan 1989 14:21:24  $
;***************************************************************************

;------------------------------------------------------------------------------
INCLUDE	ROMOPT.INC		; ROM Options file - must be included 1st
INCLUDE	MACROS.INC		; ROM Macros file
include romequ.inc
;------------------------------------------------------------------------------
	OUT1	<FILLR2 - ROM Fill to AT Compatibility section>

; The value of CODSIZ is the size of the ROM code without the FILLER and FILLR2
; sections linked in.  This needs to be only approximate as a special object
; file MOD8K.OBJ that was created by hand will be linked immediately before the
; compatibility section forcing it to the next 8k boundary.  The fill area will
; be about 4k less allowing significant size changes to occur without changing
; this file

CGROUP	GROUP	CODE
CODE	SEGMENT	BYTE PUBLIC 'CODE'
	ASSUME	CS:CODE,DS:NOTHING,ES:NOTHING

F2START	EQU	(0FFFFh-ROMSIZ+1)	;+CODSIZ)	; this is where we are now
	SYNCA	F2START

	;  break SYNC macro up into pieces:
;       IF	(0AC79h-1000h) GT F2START
;	SYNCA	(0AC79h-1000h)
;       ENDIF	;(0AC79h-1000h) GT F2START
;       IF	(0AC79h+1000h) GT F2START
;	SYNCB	(0AC79h+1000h),0CCh	; Compaq DOS requires 
;					;   non-00 non-FF at 0FAC79
;       ENDIF	;(0AC79h+1000h) GT F2START
;	SYNCA	(0E000h-1000h)		; Fill to F000:E000 - MOD8K.OBJ will
;					;  take up slack

	PUBLIC	ROMBGN
ROMBGN:						; Beginning of ROM
;
;----------------------------------------------------------------------------
;

	IF	ROMNUM EQ 2 			; 2 roms for bios
	DB 'CCooppyyrriigghhtt  11998855--11998899  PPhhooeenniixx  '
	DB 'TTeecchhnnoollooggiieess  LLttdd..'
	ENDIF	;ROMNUM EQ 2

;
;----------------------------------------------------------------------------
;
        IF	ROMNUM EQ 4
        DB 'CCCCooooppppyyyyrrrriiiigggghhhhtttt    1111999988885555----'
        DB '1111999988889999    PPPPhhhhooooeeeennnniiiixxxx    '
        DB 'TTTTeeeecccchhhhnnnnoooollllooooggggiiiieeeessss    LLLLttttdddd....'
        ENDIF	;ROMNUM EQ 4
;
;----------------------------------------------------------------------------
;
	IF	ROMNUM EQ 1
	DB 'Copyright 1985-1989  Phoenix '
	DB 'Technologies Ltd.  '
	DB "Copyright 1989 "
	DB "Commodore Electronics Ltd. "	
	ENDIF	;ROMNUM EQ 1
;
;----------------------------------------------------------------------------
;

	SYNC	FLOPPY_NEW
	PRGSIZA	<Fill to FDD handler section (0F000:08800)>

;	SYNC	BASIC_INT
;	PRGSIZA	<Fill to compatibility section (0F000:0C000)>
					; Display program size
	TOTAL

CODE	ENDS
	END
