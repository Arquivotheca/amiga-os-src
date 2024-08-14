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
 * $Header: /usr/gilligan/eric/amiga/src/editors/pointer/RCS/pref.h,v 36.0 90/07/17 14:50:53 eric Exp $
 *
 * $Locker:  $
 *
 ****************************************************************************/

#ifndef PREF_H
#define PREF_H

#include <exec/types.h>
#include <graphics/text.h>
#include "ilbm.h"

/* ===== Pointer.prefs ==================================================== */


#define ENV_NAME	"ENV:Sys/pointer.ilbm"
#define ARC_NAME	"ENVARC:Sys/pointer.ilbm"
#define PRE_DIR		"Sys:Prefs/Presets"
#define PRE_NAME	"Pointer.pre"
#define REQ_LOAD	"Load Pointer Preferences"
#define REQ_SAVE	"Save Pointer Preferences"
#define F_LENGTH	(sizeof(PREF))

#define ENV_TITLE	"Pointer Preferences"
#define TASK_NAME	"pointer_pref_task"
#define TOOL_NAME	"SYS:Prefs/Pointer"

#define DEF_PL0 \
	0xC000, \
	0x7000, \
	0x3C00, \
	0x3F00, \
	0x1FC0, \
	0x1FC0, \
	0x0F00, \
	0x0D80, \
	0x04C0, \
	0x0460, \
	0x0020, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000

#define DEF_PL1 \
	0x4000, \
	0xB000, \
	0x4C00, \
	0x4300, \
	0x20C0, \
	0x2000, \
	0x1100, \
	0x1280, \
	0x0940, \
	0x08A0, \
	0x0040, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000, \
	0x0000


typedef struct PointerPref {
    struct ColorRegister cmap[16];
    struct Point2D grab;
    struct BitMap *bm;
} PREF;

#endif

