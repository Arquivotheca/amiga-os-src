    IFND	GRAPHICS_ANIMATE_I
GRAPHICS_ANIMATE_I	SET	1

**
**	$Id: animate.i,v 40.1 93/04/15 18:51:01 vertex Exp Locker: jerryh $
**
**	graphics AnimateControlA() definitions
**
**	(C) Copyright 1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
**

    IFND    EXEC_TYPES_I
    include 'exec/types.i'
    ENDC

ACTAG_ColorRange	EQU	$80000001
ACTAG_ScrollLines	EQU	$80000002
ACTAG_RepeatLines	EQU	$80000003

    STRUCTURE	ColorRange,0
	ULONG	cor_Pen
	UWORD	cor_WAIT_X
	UWORD	cor_WAIT_Y
	ULONG	cor_Red
	ULONG	cor_Green
	ULONG	cor_Blue
	ULONG	cor_Flags;
	STRUCT	cor_Private,36
    LABEL	cor_SIZEOF

    BITDEF	COR,ANIMATE,0	/* Set to modify an existing ColorRange */
    BITDEF	COR,MODIFY,1	/* Set to modify this Colour */
    BITDEF	COR,RESTORE,2	/* Set to restore the pen to its original value */

AC_ERR_NoMem	EQU	1

    ENDC
