	NAME	FILLER
	TITLE	Fill out front of 64K rom
	PAGE	59,132
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program	   *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent.			   *
;***************************************************************************
;
;	Written:   /  /84 by Ira J. Perlow
;	Revised:   /  /85 by Ira J. Perlow
;		  5/85, 6/85 stan
;------------------------------------------------------------------------------
;
LISTINC	EQU	TRUE		;define this so libraries get listed
INCLUDE	ROMOPT.INC		; ROM Options file - must be included 1st
INCLUDE	MACROS.INC		; ROM Macros file
;The following are not used but are listed in this module:
INCLUDE	ROMEQU.INC		;list this
INCLUDE	KYBEQU.INC		;list this
;------------------------------------------------------------------------------
;
	OUT1	<FILLER - ROM Fill code>
;
CGROUP	GROUP	CODE
CODE	SEGMENT	BYTE PUBLIC 'CODE'
	ASSUME	CS:CODE,DS:NOTHING,ES:NOTHING
;
	SYNCA	0h
FILLSIZ	EQU	0FFFFh-ROMSIZ+1		; Arithmetic to not cause numeric
					;   overflow
	SYNCA	FILLSIZ			; Create some fill space
	PRGSIZA	<Fill to beginning of ROM>	; Display program size
	TOTAL
;
CODE	ENDS
	END
