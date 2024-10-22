	IFND	GADGETS_TABS_I
GADGETS_TABS_I	SET	1

**
**	$VER: tabs.i 42.3 (14.2.94)
**	Includes Release 42.1
**
**	Definitions for the tabs BOOPSI gadget class
**
**	(C) Copyright 1994 Commodore-Amiga Inc.
**	All Rights Reserved
**

;*****************************************************************************

    IFND EXEC_TYPES_I
    INCLUDE "exec/types.i"
    ENDC

    IFND UTILITY_TAGITEM_H
    INCLUDE "utility/tagitem.i"
    ENDC

    IFND INTUITION_GADGETCLASS_I
    INCLUDE "intuition/gadgetclass.i"
    ENDC

;*****************************************************************************

TL_TEXTPEN		equ	0
TL_BACKGROUNDPEN	equ	1
TL_FILLTEXTPEN		equ	2
TL_FILLPEN		equ	3
MAX_TL_PENS		equ	4

;*****************************************************************************

; This structure is used to describe the labels for each of the tabs
    STRUCTURE TabLabel,0
	APTR	 tl_Label			; Label
	STRUCT	 tl_Pens,(MAX_TL_PENS*2)	; Pens (array of UWORD's)
	APTR	 tl_Attrs			; Additional attributes

    LABEL TabLabel_SIZEOF

;*****************************************************************************

; Additional attributes defined by the tabs.gadget class
TABS_Dummy		equ	(TAG_USER+$04000000)

TABS_Labels		equ	(TABS_Dummy+1)
    ; (TabLabelP) Array of labels

TABS_Current		equ	(TABS_Dummy+2)
    ; (LONG) Current tab

;*****************************************************************************

    ENDC	; GADGETS_TABS_I
