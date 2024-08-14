	IFND	PREFS_INPUT_I
PREFS_INPUT_I	SET	1
**
**	$Id: input.i,v 38.2 91/06/28 09:10:59 vertex Exp $
**
**	File format for input preferences
**
**	(C) Copyright 1991 Commodore-Amiga, Inc.
**	All Rights Reserved
**

;---------------------------------------------------------------------------

    IFND EXEC_TYPES_I
    INCLUDE "exec/types.i"
    ENDC

    IFND DEVICES_TIMER_I
    INCLUDE "devices/timer.i"
    ENDC

;---------------------------------------------------------------------------

ID_INPT equ "INPT"


   STRUCTURE InputPrefs,0
	STRUCT ip_Keymap,16
	UWORD  ip_PointerTicks
	STRUCT ip_DoubleClick,TV_SIZE
	STRUCT ip_KeyRptDelay,TV_SIZE
	STRUCT ip_KeyRptSpeed,TV_SIZE
	WORD   ip_MouseAccel
   LABEL InputPrefs_SIZEOF

;---------------------------------------------------------------------------

	ENDC	; PREFS_INPUT_I
