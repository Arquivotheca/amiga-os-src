/****************************************************************************
 **									   **
 **  icon.c								   **
 **									   **
 **  Confidential Information: Commodore-Amiga, Inc.			   **
 **  Copyright (c) 1989 Commodore-Amiga, Inc.				   **
 **									   **
 ****************************************************************************
 *
 * SOURCE CONTROL
 *
 * $Header: /usr/machines/heartofgold/amiga/V37/src/workbench/prefs/overscan/RCS/icon.c,v 37.0 91/01/15 12:12:21 eric Exp $
 *
 * $Locker:  $
 *
 ****************************************************************************/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <clib/icon_protos.h>
#include <pragmas/icon_pragmas.h>

extern struct Library *IconBase;

void WriteIcon(UBYTE *name, BYTE unit);


/* Definitions for Pref Icon */
UWORD  PrefImageData[] = {
 0x0000, 0x0000, 0x0004, 0x0000,
 0x0000, 0x0000, 0x0001, 0x0000,
 0x0000, 0x07FF, 0x8000, 0x4000,
 0x0000, 0x1800, 0x6000, 0x1000,
 0x0000, 0x20FC, 0x1000, 0x0800,
 0x0000, 0x4102, 0x0800, 0x0C00,
 0x0000, 0x4082, 0x0800, 0x0C00,
 0x0000, 0x4082, 0x0800, 0x0C00,
 0x0000, 0x2104, 0x0800, 0x0C00,
 0x0000, 0x1E18, 0x1000, 0x0C00,
 0x0000, 0x0060, 0x2000, 0x0C00,
 0x0000, 0x0080, 0xC000, 0x0C00,
 0x0000, 0x0103, 0x0000, 0x0C00,
 0x0000, 0x021C, 0x0000, 0x0C00,
 0x0000, 0x0108, 0x0000, 0x0C00,
 0x0000, 0x00F0, 0x0000, 0x0C00,
 0x0000, 0x0108, 0x0000, 0x0C00,
 0x0000, 0x0108, 0x0000, 0x0C00,
 0x4000, 0x00F0, 0x0000, 0x0C00,
 0x1000, 0x0000, 0x0000, 0x0C00,
 0x0400, 0x0000, 0x0000, 0x0C00,
 0x01FF, 0xFFFF, 0xFFFF, 0xFC00,
 0x0000, 0x0000, 0x0000, 0x0000,
/**/
 0xFFFF, 0xFFFF, 0xFFF8, 0x0000,
 0xD555, 0x5555, 0x5556, 0x0000,
 0xD555, 0x5000, 0x5555, 0x8000,
 0xD555, 0x47FF, 0x9555, 0x6000,
 0xD555, 0x5F03, 0xE555, 0x5000,
 0xD555, 0x3E55, 0xF555, 0x5000,
 0xD555, 0x3F55, 0xF555, 0x5000,
 0xD555, 0x3F55, 0xF555, 0x5000,
 0xD555, 0x5E53, 0xF555, 0x5000,
 0xD555, 0x4147, 0xE555, 0x5000,
 0xD555, 0x551F, 0xD555, 0x5000,
 0xD555, 0x557F, 0x1555, 0x5000,
 0xD555, 0x54FC, 0x5555, 0x5000,
 0xD555, 0x55E1, 0x5555, 0x5000,
 0xD555, 0x54F5, 0x5555, 0x5000,
 0xD555, 0x5505, 0x5555, 0x5000,
 0xD555, 0x54F5, 0x5555, 0x5000,
 0xD555, 0x54F5, 0x5555, 0x5000,
 0x3555, 0x5505, 0x5555, 0x5000,
 0x0D55, 0x5555, 0x5555, 0x5000,
 0x0355, 0x5555, 0x5555, 0x5000,
 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000,
};


UBYTE *TTypes[2] = {"ACTION=use", NULL};


struct Image PrefImage = {
    0, 0,		/* LeftEdge, TopEdge */
    54, 23,		/* Width, Height */
    2,			/* Depth */
    &PrefImageData[0],	/* ImageData */
    0x033, 0x00,	/* PlanePick, PlaneOnOff */
    NULL		/* NextImage */
};


struct DiskObject PrefObject = {
    WB_DISKMAGIC,	/* do_Magic */
    WB_DISKVERSION,	/* do_Version */

    /* Gadget Structure */
    NULL,		/* Ptr to next gadget */
    0, 0,		/* Leftedge, Topedge */
    54, 23,		/* Width, Height */
    GADGHBOX|GADGIMAGE,	/* Flags */
    RELVERIFY|GADGIMMEDIATE, /* Activation */
    BOOLGADGET,		/* Type */
    (APTR)&PrefImage,	/* Render */
    NULL,		/* Select Render */
    NULL,		/* Text */
    NULL, NULL, NULL,	/* Exclude, Special, ID */
    NULL,		/* UserData */

    0x04,		/* WBObject type */
    "SYS:Prefs/Overscan", /* Default tool */
    TTypes,		/* Tool Types */
    NO_ICON_POSITION,	/* Current X */
    NO_ICON_POSITION,	/* Current Y */
    NULL, NULL, NULL	/* Drawer, ToolWindow, Stack */
};


void WriteIcon(name, unit)
UBYTE *name;
BYTE unit;
{
/*     TTypes[1] = FontUnit[unit]; */

    PutDiskObject(name, &PrefObject);
}
