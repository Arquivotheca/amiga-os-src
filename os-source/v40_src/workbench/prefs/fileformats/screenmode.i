	IFND	PREFS_SCREENMODE_I
PREFS_SCREENMODE_I	SET	1
**
**	$Id: screenmode.i,v 38.4 92/06/25 11:35:50 vertex Exp $
**
**	File format for screen mode preferences
**
**	(C) Copyright 1991 Commodore-Amiga, Inc.
**	All Rights Reserved
**

;---------------------------------------------------------------------------

    IFND EXEC_TYPES_I
    INCLUDE "exec/types.i"
    ENDC

;---------------------------------------------------------------------------

ID_SCRM equ "SCRM"


   STRUCTURE ScreenModePrefs,0
	STRUCT smp_Reserved,4*4
	ULONG  smp_DisplayID
	UWORD  smp_Width
	UWORD  smp_Height
	UWORD  smp_Depth
	UWORD  smp_Control
   LABEL ScreenModePrefs_SIZEOF

; flags for ScreenModePrefs.smp_Control
	BITDEF	SM,AUTOSCROLL,0

;---------------------------------------------------------------------------

	ENDC	; PREFS_SCREENMODE_I
