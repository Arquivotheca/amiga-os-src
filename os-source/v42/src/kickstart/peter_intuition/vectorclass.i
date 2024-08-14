	IFND	INTUITION_VECTORCLASS_I
INTUITION_VECTORCLASS_I	SET	1

*
*  Vector image class interface
*
*  $Id: vectorclass.i,v 38.4 92/05/15 14:10:52 peter Exp $
*
*  Confidential Information: Commodore-Amiga, Inc.
*  Copyright (C) 1990, Commodore-Amiga, Inc.
*  All Rights Reserved
*

	IFND	INTUITION_INTUITION_I
	INCLUDE	'intuition/intuition.i'
	ENDC

	IFND	INTUITION_IMAGECLASS_I
	INCLUDE 'intuition/imageclass.i'
	ENDC

* vector image attributes

 ENUM TAG_USER+$21000
    EITEM VIA_Packet,			; information packet

* see imageclass.i for additional tags

 STRUCTURE Vector,0
	UBYTE v_Flags		; Flags (see below)
	UBYTE v_States		; Image states that this vector applies to
	WORD v_DataOffset	; Short pointer to data
	LABEL SIZEOF_Vector

* values of v_Flags
VF_PENMASK	EQU	$0F	; Pen number goes into bottom nybble

VF_TEXT		EQU	TEXTPEN		; $02
VF_SHINE	EQU	SHINEPEN	; $03
VF_SHADOW	EQU	SHADOWPEN	; $04
VF_FILL		EQU	FILLPEN		; $05
VF_FILLTEXT	EQU	FILLTEXTPEN	; $06
VF_BACKG	EQU	BACKGROUNDPEN	; $07

VF_BDETAIL	EQU	BARDETAILPEN	; $09
VF_BBLOCK	EQU	BARBLOCKPEN	; $0A

VF_MONO		EQU	$10	; Vector is used for monochrome rendering
VF_COLOR	EQU	$20	; Vector is used for color rendering

VF_LINED	EQU	$00	; NB THIS VALUE IS ZERO
VF_FILLED	EQU	$40	; filled shape, not outline
VF_RECTANGLE	EQU	$00	; NB THIS VALUE IS ZERO
VF_POLYGON	EQU	$80	; polygon, not rectangle

VF_LINERECT	EQU	VF_LINED!VF_RECTANGLE
VF_FILLRECT	EQU	VF_FILLED!VF_RECTANGLE
VF_LINEPOLY	EQU	VF_LINED!VF_POLYGON
VF_FILLPOLY	EQU	VF_FILLED!VF_POLYGON


; v_States can take the following possible values:
; (NB: These are a collapsed subset of the IDS_ states)

VSB_NORMAL	EQU	0		; IDS_NORMAL
VSB_SELECTED	EQU	1		; IDS_SELECTED
VSB_INANORMAL	EQU	2		; IDS_INACTIVENORMAL

VS_NORMAL	EQU	1<<VSB_NORMAL
VS_SELECTED	EQU	1<<VSB_SELECTED
VS_INANORMAL	EQU	1<<VSB_INANORMAL

; Images for Window borders have this particular combination:
VS_WBORDERIMG	EQU	VS_NORMAL!VS_SELECTED!VS_INANORMAL

VS_NUMSTATES	EQU	3

	ENDC
