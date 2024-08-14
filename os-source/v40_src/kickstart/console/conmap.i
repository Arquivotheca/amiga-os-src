	IFND	DEVICES_CONMAP_I
DEVICES_CONMAP_I	SET	1
**
**	$Id: conmap.i,v 36.5 90/06/07 11:33:42 kodiak Exp $
**
**	Console device map definition
**
**	(C) Copyright 1989 Commodore-Amiga, Inc.
**	    All Rights Reserved
**

;------ cm_Attr... entries -------------------------------------------
CMAM_FGPEN	EQU	$0007		; for FgPen
CMAS_FGPEN	EQU	0
CMAM_BGPEN	EQU	$0038		; for BgPen
CMAS_BGPEN	EQU	3
CMAM_SOFTSTYLE	EQU	$01c0		; for SetSoftStyle
CMAS_SOFTSTYLE	EQU	6
CMAF_INVERSVID	EQU	$0200		; RP_INVERSVID set
CMAB_INVERSVID	EQU	9
CMAF_SELECTED	EQU	$0400		; selection
CMAB_SELECTED	EQU	10
CMAF_HIGHLIGHT	EQU	$0800		; highlighted part of selection
CMAB_HIGHLIGHT	EQU	11
CMAF_TABBED	EQU	$1000		; tab immediately preceeded
CMAB_TABBED	EQU	12		;   this character placement
CMAF_IMPLICITNL	EQU	$2000		; CUB_IMPLICITNL set (valid for 1st
CMAB_IMPLICITNL	EQU	13		;   character in line only)
CMAF_CURSOR	EQU	$4000		; cursor cached here during resize
CMAB_CURSOR	EQU	14		;
CMAF_RENDERED	EQU	$8000		; this entry and all entries to the
CMAB_RENDERED	EQU	15		;   left on the line are valid.
					;   (must be the sign bit)

 STRUCTURE ConsoleMap,0
    ULONG   cm_AllocSize		; AllocMem size for cm_AllocBuffer
    LABEL   cm_AllocBuffer		; memory buffer holding everything
    APTR    cm_AttrBufLines		; array with cm_BufferLines elements
					;   containing address/2 of off-screen
					;   attr lines
    APTR    cm_AttrDispLines		; array with cm_DisplayHeight elements
					;   containing address/2 of displayed
					;   attr lines
    LONG    cm_BufferStart		; start of memory for buffer: address/2
    LONG    cm_DisplayStart		; start of memory for display: address/2
    LONG    cm_AttrToChar		; delta to apply to attr address/2 to
					;   find associated character address
    UWORD   cm_BufferLines		; maximum rows in window
    UWORD   cm_BufferWidth		; number of columns off-screen
    UWORD   cm_BufferHeight		; number of rows off-screen
    UWORD   cm_DisplayWidth		; number of columns in display
    UWORD   cm_DisplayHeight		; number of rows in display
    UWORD   cm_BufferXL			; X append loc in last ....Buffer line
    UWORD   cm_BufferYL			; append ....Buffer line
    LABEL   ConsoleMap_SIZEOF

	ENDC	; DEVICES_CONMAP_I
