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
 * $Header: /usr/gilligan/eric/amiga/src/editors/font/RCS/pref.h,v 36.1 90/07/31 15:20:57 eric Exp Locker: eric $
 *
 * $Locker: eric $
 *
 ****************************************************************************/

#ifndef PREF_H
#define PREF_H

#include <exec/types.h>
#include <graphics/text.h>

#define FNAMESIZE (128)

typedef struct FontPref {
    LONG  fp_Reserved[4];
    UBYTE fp_FrontPen;
    UBYTE fp_BackPen;
    UBYTE fp_DrawMode;
    struct TextAttr fp_TextAttr;
    BYTE fp_Name[FNAMESIZE];
} PREF;

#define ENV_TITLE	"Font Preferences"
#define ENV_NAME	"ENV:Sys/font.prefs"
#define ENV_NAME0	"ENV:Sys/wbfont.prefs"
#define ENV_NAME1	"ENV:Sys/screenfont.prefs"
#define ENV_NAME2	"ENV:Sys/sysfont.prefs"
#define ARC_NAME	"ENVARC:Sys/font.prefs"
#define ARC_NAME0	"ENVARC:Sys/wbfont.prefs"
#define ARC_NAME1	"ENVARC:Sys/screenfont.prefs"
#define ARC_NAME2	"ENVARC:Sys/sysfont.prefs"
#define REQ_LOAD	"Load Font Preferences"
#define REQ_SAVE	"Save Font Preferences"
#define PRE_DIR		"Sys:Prefs/Presets"
#define PRE_NAME	"Font.pre"
#define PRE_NAME0	"WbFont.pre"
#define PRE_NAME1	"ScreenFont.pre"
#define PRE_NAME2	"SysFont.pre"

#define F_LENGTH (sizeof(PREF))

#define TASK_NAME	"font_pref_task"
#define TOOL_NAME	"SYS:Prefs/Font"

#define ENV_DEF0	{0L, 0L, 0L, 0L, WHITE, BLACK, JAM1, {NULL, 8, 0, 0}, "topaz.font"}
#define ENV_DEF1	{0L, 0L, 0L, 0L, WHITE, BLACK, JAM1, {NULL, 8, 0, 0}, "topaz.font"}
#define ENV_DEF2	{0L, 0L, 0L, 0L, WHITE, BLACK, JAM1, {NULL, 8, 0, 0}, "topaz.font"}

#endif
