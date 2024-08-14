	NAME	ROMFNT
	TITLE	ROM CRTDSR GRAPHICS MODE FONT
	PAGE	59,132
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program	   *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent.			   *
;***************************************************************************
;
;	Written: 
;	Revised: 4/85 Ira J. Perlow - Corrected fonts
;		 5/85 Stan - extracted font to FONTGN.INC, made DBs binary
;
;------------------------------------------------------------------------------
;
INCLUDE	ROMOPT.INC		; ROM Options file - must be included 1st
INCLUDE	ROMEQU.INC		; ROM General equate file
INCLUDE	MACROS.INC		; Handy Macros
;
;------------------------------------------------------------------------------
;
	OUT1	<ROMFNT - ROM Font table>
;
CGROUP	GROUP	CODE
CODE	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME CS:CODE,DS:NOTHING,ES:NOTHING
;
; CHARACTER FONT F0R 320X200 AND 640X200 GRAPHICS
;
	SYNC	ROMFNTA
	INCLUDE FONTGN.INC
	PRGSIZ	<ROM Font>		; Display program size
;
	SYNC	TIMDSRA
	TOTAL
;
CODE	ENDS
	END
