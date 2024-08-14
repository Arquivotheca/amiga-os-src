#ifndef PREFS_SCREENMODE_H
#define PREFS_SCREENMODE_H
/*
**	$Id: screenmode.h,v 38.4 92/06/25 11:35:40 vertex Exp $
**
**	File format for screen mode preferences
**
**	(C) Copyright 1991 Commodore-Amiga, Inc.
**	All Rights Reserved
*/

/*****************************************************************************/


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef LIBRARIES_IFFPARSE_H
#include <libraries/iffparse.h>
#endif


/*****************************************************************************/


#define ID_SCRM MAKE_ID('S','C','R','M')


struct ScreenModePrefs
{
    ULONG smp_Reserved[4];
    ULONG smp_DisplayID;
    UWORD smp_Width;
    UWORD smp_Height;
    UWORD smp_Depth;
    UWORD smp_Control;
};

/* flags for ScreenModePrefs.smp_Control */
#define SMB_AUTOSCROLL 1

#define SMF_AUTOSCROLL (1<<0)


/*****************************************************************************/


#endif /* PREFS_SCREENMODE_H */
