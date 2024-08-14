/****************************************************************************
 **									   **
 **  pref.h								   **
 **									   **
 **  Confidential Information: Commodore-Amiga, Inc.			   **
 **  Copyright (c) 1989 Commodore-Amiga, Inc.				   **
 **									   **
 ****************************************************************************
 *
 * SOURCE CONTROL
 *
 * $Header: pref.h,v 36.0 89/09/22 16:25:27 eric Exp $
 *
 * $Locker:  $
 *
 ****************************************************************************/

#ifndef PREF_H
#define PREF_H

#include <exec/types.h>
#include <graphics/text.h>
#define NUMCOLORS	32

typedef struct PalettePref {
    UWORD Colors[NUMCOLORS];
} PREF;

#define ENV_NAME "ENV:palette.prefs"
#define ARC_NAME "SYS:Env/palette.prefs"
#define F_LENGTH (sizeof(PREF))

#define ENV_TITLE	"PREFS: Palette"
#define TASK_NAME	"palette_pref_task"
#define TOOL_NAME	"PREFS:Font"

/*  Someone else can pick these!!! */
#define ENV_DEF		{ 0x0888, 0x0fff, 0x0002, 0x0fb6,\
	0x0000, 0x0000, 0x0000, 0x0000,\
	0x0000, 0x0000, 0x0000, 0x0000,\
	0x0000, 0x0000, 0x0000, 0x0000,\
	0x0000, 0x0000, 0x0000, 0x0000,\
	0x0000, 0x0000, 0x0000, 0x0000,\
	0x0000, 0x0000, 0x0000, 0x0000,\
	0x0000, 0x0000, 0x0000, 0x0000, }
};
			
#endif
