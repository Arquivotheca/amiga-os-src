*  :ts=8 Version D318.39.2.141
*
*  Version.i -- Version info include file for "title"
*
*  Created by "BumpVersion"      D318
*
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY                        *
*   Copyright 1993 Silent Software, Inc.                                    *
*   All Rights Reserved.                                                    *
*****************************************************************************

VERSION_STRING	  MACRO
		dc.b	'title 39.2 (18.3.93)',13,10
	ENDM 

LIB_NAME		  MACRO
		dc.b	'title.library'
	ENDM
VERSION           EQU	39
REVISION          EQU	2
MINOR_REVISION    EQU	141

PROGRAM_TITLE     MACRO
		dc.b	'title Ver. D318.39.2.141 MC680x0 - Decompressor Library',13,10
	ENDM 

PROGRAM_CREDITS   MACRO
		dc.b	'Written by William A. Ware. Artwork & Designed by Jim Sachs.',13,10
	ENDM 

PROGRAM_COPYRIGHT MACRO
		dc.b	'Copyright 1993 Silent Software, Inc.',13,10
		dc.b	'Artwork Copyright 1993 Sachs Enterprises',13,10
	ENDM 

PROGRAM_NOTICE    MACRO
		dc.b	'*** Licenced by Commodore International, Ltd ***',13,10
	ENDM 


