	IFND	GADGETS_BUTTON_I
GADGETS_BUTTON_I	SET	1

**
**	$VER: button.i 42.1 (10.1.94)
**	Includes Release 42.1
**
**	Definitions for the button BOOPSI class
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

; Additional attributes defined by the button.gadget class
BUTTON_Dummy		equ	(TAG_USER+$04000000)

BUTTON_PushButton	equ	(BUTTON_Dummy+1)
    ; (BOOL) Indicate whether button stays depressed when clicked

BUTTON_Glyph		equ	(BUTTON_Dummy+2)
    ; (struct Image *) Indicate that image is to be drawn using BltTemplate

BUTTON_Array		equ	(BUTTON_Dummy+3)
    ; (LONG) Indicate that text or image pointer is an array

BUTTON_TextPen		equ	(BUTTON_Dummy+5)
    ; (LONG) Pen to use for text (-1 uses TEXTPEN)

BUTTON_FillPen		equ	(BUTTON_Dummy+6)
    ; (LONG) Pen to use for fill (-1 uses FILLPEN)

BUTTON_FillTextPen	equ	(BUTTON_Dummy+7)
    ; (LONG) Pen to use for fill (-1 uses FILLTEXTPEN)

BUTTON_BackgroundPen	equ	(BUTTON_Dummy+8)
    ; (LONG) Pen to use for fill (-1 uses BACKGROUNDPEN)

BUTTON_Current		equ	(BUTTON_Dummy+9)
    ; (LONG) Indicate which item in the array is current

;*****************************************************************************

    ENDC	; GADGETS_BUTTON_I
