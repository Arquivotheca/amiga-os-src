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
 * $Header: /usr/gilligan/eric/amiga/src/editors/icontrol/RCS/pref.h,v 36.0 90/08/03 12:29:02 eric Exp Locker: eric $
 *
 * $Locker: eric $
 *
 ****************************************************************************/

#ifndef PREF_H
#define PREF_H

#include <exec/types.h>
#include <devices/inputevent.h>


/* ===== IControl.prefs =================================================== */

/* IControl Flags (ip_ICFlags) */

/* use "palette degradation" to minimize mixed-mode distortion */
#define IC_COERCE_COLORS	(0x0001)
/* use interlace to minimize mixed-mode distortion */
#define IC_COERCE_LACE		(0x0002)
/* filter control characters out of string gadgets (and ^H=BS) */
#define IC_STRINGG_CTRL		(0x0004)
/* do magic menu (AKA Screen Menu Snap) */
#define IC_DOMAGICMENU		(0x0008)


typedef struct ICPref {
    LONG  fp_Reserved[4];		/* System reserved */
    UWORD ip_TimeOut;			/* Verify timeout */
    WORD  ip_MetaDrag;			/* Meta drag mouse event */
    ULONG ip_ICFlags;			/* IControl flags (see above) */
    UBYTE ip_WBtoFront;			/* CKey: WB to front */
    UBYTE ip_FrontToBack;		/* CKey: front screen to back */
    UBYTE ip_ReqTrue;			/* CKey: Requester TRUE */
    UBYTE ip_ReqFalse;			/* CKey: Requester FALSE */
} PREF;

#define ENV_NAME "ENV:Sys/icontrol.prefs"
#define ARC_NAME "ENVARC:Sys/icontrol.prefs"
#define PRE_DIR  "Sys:Prefs/Presets"
#define PRE_NAME "IControl.pre"
#define REQ_LOAD "Load IControl Preferences"
#define REQ_SAVE "Save IControl Preferences"
#define F_LENGTH (sizeof(PREF))

#define ENV_TITLE	"IControl Preferences"
#define TASK_NAME	"icontrol_pref_task"
#define TOOL_NAME	"SYS:Prefs/IControl"

#define ENV_DEF		{0L, 0L, 0L, 0L, 50, IEQUALIFIER_LCOMMAND, 0x0FL, 'N', 'M', 'V', 'B'}

#endif


