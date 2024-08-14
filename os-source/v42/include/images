	IFND	IMAGES_LED_I
IMAGES_LED_I	SET	1

**
**	$VER: led.h 42.1 (10.1.94)
**	Includes Release 42.1
**
**	Definitions for the LED BOOPSI image class
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

    IFND INTUITION_IMAGECLASS_I
    INCLUDE "intuition/imageclass.i"
    ENDC

;*****************************************************************************

LED_Dummy		equ	(TAG_USER+$04000000)

LED_Pairs		equ	(LED_Dummy+1)
    ; (WORD) Number of digit pairs (1-8)

LED_Values		equ	(LED_Dummy+2)
    ; (WORD *) Array of digit pairs.  Must be LED_Pairs in size.

LED_Colon		equ	(LED_Dummy+3)
    ; (BOOL) Colon on or off

LED_Negative		equ	(LED_Dummy+4)
    ; (BOOL) Negative sign on or off

LED_Signed		equ	(LED_Dummy+5)
    ; (BOOL) Leave space for negative sign

;*****************************************************************************

    ENDC	; IMAGES_LED_I
