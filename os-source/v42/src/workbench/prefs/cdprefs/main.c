/* main.c
 * 1200CD Preference Editor
 * Copyright (C) 1994 Commodore-Amiga, Inc.
 * Written by David N. Junod
 *
 * 07-Feb-94 Started
 */

#define	LOWLEVEL_PRIVATE_PRAGMAS	TRUE

#include <dos/dos.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <graphics/gfx.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <gadgets/tabs.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <libraries/locale.h>
#include <libraries/lowlevel.h>
#include <internal/nonvolatile.h>
#include <workbench/startup.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <clib/asl_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/locale_protos.h>
#include <clib/nonvolatile_protos.h>
#include <clib/lowlevel_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/asl_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/nonvolatile_pragmas.h>
#include <pragmas/lowlevel_pragmas.h>
#include <pragmas/utility_pragmas.h>

#define CATCOMP_NUMBERS TRUE
#define CATCOMP_BLOCK TRUE
#include "texttable.h"

/*****************************************************************************/

#include "cdprefs_rev.h"
char *version = VERSTAG;

/*****************************************************************************/

struct ExecBase *SysBase;

/*****************************************************************************/

#define ASM		__asm
#define REG(x)		register __ ## x

/*****************************************************************************/

#define	IDCMP_FLAGS	IDCMP_CLOSEWINDOW | IDCMP_VANILLAKEY | IDCMP_GADGETDOWN | IDCMP_GADGETUP \
			| IDCMP_MOUSEMOVE | IDCMP_MOUSEBUTTONS | IDCMP_INTUITICKS

/*****************************************************************************/

#define	GID_TABS	0

/* Nonvolatile page */
#define	GID_CONTEXT	0
#define	GID_LIST	1
#define	GID_PROTECT	2
#define	GID_DELETE	3
#define	GID_SEP1	4
#define	GID_LOCKED	5
#define	GID_UNLOCKED	6
#define	GID_SEP2	7
#define	GID_PREPARE	8

#define	MAX_GADS	9

/*****************************************************************************/

typedef struct LocalEntry
{
    struct MinNode le_Link;
    STRPTR le_ItemName;
    STRPTR le_AppName;
    ULONG le_Size;
    ULONG le_Protection;

} LE, *LEP;

/*****************************************************************************/

typedef struct LangEntry
{
    struct MinNode le_Link;
    LONG le_Which;
    UWORD *le_Data;

} LGE, *LGEP;

/*****************************************************************************/

typedef struct tagGlobalData
{
    struct Library *gd_LowLevelBase;
    struct Library *gd_LocaleBase;
    struct Library *gd_AslBase;
    struct Library *gd_DOSBase;
    struct Library *gd_GadToolsBase;
    struct Library *gd_GfxBase;
    struct Library *gd_IntuitionBase;
    struct Library *gd_NVBase;
    struct Library *gd_UtilityBase;
    struct Library *gd_TabsClass;
    struct Catalog *gd_Catalog;

    struct WBStartup *gd_SM;
    BPTR gd_oldDir;

    struct Screen *gd_Screen;
    struct DrawInfo *gd_DRI;
    struct TextFont *gd_TF;
    struct TextAttr gd_TA;
    struct Window *gd_Window;
    void *gd_VI;

    struct Gadget *gd_TabGadget;
    struct Image *gd_Sep;
    struct Gadget *gd_LastAdded;
    struct Gadget *gd_GTG[MAX_GADS];

    TabLabel gd_TabLabels[4];
    UWORD gd_Page;

    /* Startup */
    STRPTR gd_StartupLabels[3];
    UWORD gd_PrevStartup, gd_CurStartup;

    /* Nonvolatile */
    struct MinList *gd_EList;
    struct Hook gd_ListHook;
    struct MinList gd_ItemList;
    struct LocalEntry *gd_Entry;
    UWORD gd_CurNV;
    UWORD gd_MaxNV;

    /* Languages */
    struct Hook gd_LanguageHook;
    struct MinList gd_LanguageList;
    UWORD gd_PrevLang, gd_CurLang;

} GD, *GDP;

/*****************************************************************************/

#define	LowLevelBase		 gd->gd_LowLevelBase
#define	LocaleBase		 gd->gd_LocaleBase
#define	AslBase			 gd->gd_AslBase
#define	DOSBase			 gd->gd_DOSBase
#define	GadToolsBase		 gd->gd_GadToolsBase
#define	GfxBase			 gd->gd_GfxBase
#define	IntuitionBase		 gd->gd_IntuitionBase
#define	NVBase			 gd->gd_NVBase
#define	UtilityBase		 gd->gd_UtilityBase

/*****************************************************************************/

struct TextAttr topaz8 = {"topaz.font", 8,};

/*****************************************************************************/

UWORD __chip lnData[1422] =
{
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x001E,0x0000,0x0000,0x0C00,
	0x0000,0x0000,0x0033,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0061,0x8FF8,0x3E1B,0x8C3E,0x1F0F,0xC000,0x0061,0x8CCC,
	0x631C,0x4C61,0x018C,0x6000,0x0061,0x8CCC,0x6318,0x0C60,
	0x018C,0x6000,0x007F,0x8CCC,0x7F18,0x0C60,0x1F8C,0x6000,
	0x0061,0x8CCC,0x6018,0x0C60,0x318C,0x6000,0x0061,0x8CCC,
	0x6318,0x0C61,0x318C,0x6000,0x0061,0x8CCC,0x3E18,0x0C3E,
	0x1F8C,0x6000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x07F0,
	0x0000,0x0003,0x0000,0x0000,0x0000,0x0618,0x0000,0x0003,
	0x0000,0x0000,0x0000,0x0618,0x7C3F,0x07C3,0x1800,0x0000,
	0x0000,0x0618,0x0631,0x8C23,0x3000,0x0000,0x0000,0x0618,
	0x0631,0x8C03,0x6000,0x0000,0x0000,0x0618,0x7E31,0x87C3,
	0xC000,0x0000,0x0000,0x0618,0xC631,0x8063,0x6000,0x0000,
	0x0000,0x0618,0xC631,0x8863,0x3000,0x0000,0x0000,0x07F0,
	0x7E31,0x87C3,0x1800,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0007,0xF000,0x0006,0x0000,0x0180,0x0000,0x0006,0x1800,
	0x0006,0x0000,0x0180,0x0000,0x0006,0x187C,0x319F,0x8F83,
	0xE180,0x0000,0x0006,0x18C6,0x3186,0x18C6,0x11F8,0x0000,
	0x0006,0x18C6,0x3186,0x1806,0x018C,0x0000,0x0006,0x18FE,
	0x3186,0x0F86,0x018C,0x0000,0x0006,0x18C0,0x3186,0x00C6,
	0x018C,0x0000,0x0006,0x18C6,0x3186,0x18C6,0x118C,0x0000,
	0x0007,0xF07C,0x1F83,0x0F83,0xE18C,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x7F00,0x0000,0xC300,0x0600,0x0000,
	0x0000,0x6000,0x0000,0xC000,0x0600,0x0000,0x0000,0x601F,
	0x83F0,0xC30F,0x8600,0x0000,0x0000,0x6018,0xC630,0xC318,
	0x47E0,0x0000,0x0000,0x7C18,0xC630,0xC318,0x0630,0x0000,
	0x0000,0x6018,0xC630,0xC30F,0x8630,0x0000,0x0000,0x6018,
	0xC630,0xC300,0xC630,0x0000,0x0000,0x6018,0xC630,0xC318,
	0xC630,0x0000,0x0000,0x7F18,0xC3F0,0xC30F,0x8630,0x0000,
	0x0000,0x0000,0x0030,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0430,0x0000,0x0000,0x0000,0x0000,0x0000,0x03E0,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x00E4,0x0000,0x0000,0x0001,0xFC00,0x0000,0x0138,
	0x0018,0x0000,0x0001,0x8000,0x0000,0x0000,0x0018,0x0000,
	0x0001,0x803E,0x1F83,0xE1F8,0x3E18,0x0000,0x0001,0x8063,
	0x18C0,0x318C,0x6318,0x0000,0x0001,0xF060,0x18C0,0x318C,
	0x6318,0x0000,0x0001,0x803E,0x18C3,0xF18C,0x6318,0x0000,
	0x0001,0x8003,0x18C6,0x318C,0x6318,0x0000,0x0001,0x8043,
	0x18C6,0x318C,0x6318,0x0000,0x0001,0xFC3E,0x1F83,0xF18C,
	0x3E18,0x0000,0x0000,0x0000,0x1800,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x1800,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x1800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x001F,0xC000,
	0x0000,0x0000,0x0C00,0x0000,0x0018,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0018,0x0DC7,0xC3F0,0x7C1F,0x0C3E,0x0000,
	0x0018,0x0E20,0x6318,0xC201,0x8C61,0x0000,0x001F,0x0C00,
	0x6318,0xC201,0x8C60,0x0000,0x0018,0x0C07,0xE318,0xC01F,
	0x8C3E,0x0000,0x0018,0x0C0C,0x6318,0xC031,0x8C03,0x0000,
	0x0018,0x0C0C,0x6318,0xC231,0x8C63,0x0000,0x0018,0x0C07,
	0xE318,0x7C1F,0x8C3E,0x0000,0x0000,0x0000,0x0000,0x3000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x3000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x6000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0001,0x8100,0x0186,0x0000,0x0000,0x0000,0x0001,0x8300,
	0x0180,0x0000,0x0000,0x0000,0x0001,0x8FC7,0xC186,0x1F0F,
	0xC1F0,0x0000,0x0001,0x8300,0x6186,0x018C,0x6318,0x0000,
	0x0001,0x8300,0x6186,0x018C,0x6318,0x0000,0x0001,0x8307,
	0xE186,0x1F8C,0x6318,0x0000,0x0001,0x830C,0x6186,0x318C,
	0x6318,0x0000,0x0001,0x830C,0x6186,0x318C,0x6318,0x0000,
	0x0001,0x8187,0xE186,0x1F8C,0x61F0,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x000C,0x0000,0x3000,
	0x0006,0x0000,0x0830,0x000C,0x0000,0x3000,0x0006,0x0000,
	0x0C30,0x000C,0x0000,0x3000,0x0006,0x0000,0x0E31,0xF0FC,
	0x7C6E,0x31F1,0xF87E,0x3E00,0x0F33,0x198C,0xC671,0x3019,
	0x8CC6,0x6300,0x0DB3,0x198C,0xC660,0x3019,0x8CC6,0x6000,
	0x0CF3,0xF98C,0xFE60,0x31F9,0x8CC6,0x3E00,0x0C73,0x018C,
	0xC060,0x3319,0x8CC6,0x0300,0x0C33,0x198C,0xC660,0x3319,
	0x8CC6,0x6300,0x0C11,0xF0FC,0x7C60,0x31F9,0x8C7E,0x3E00,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x020C,0x0000,0x0003,
	0x0000,0x0000,0x0000,0x030C,0x0000,0x0003,0x0000,0x0000,
	0x0000,0x038C,0x3E1B,0x87C3,0x1800,0x0000,0x0000,0x03CC,
	0x631C,0x4C63,0x3000,0x0000,0x0000,0x036C,0x6318,0x0C03,
	0x6000,0x0000,0x0000,0x033C,0x6318,0x07C3,0xC000,0x0000,
	0x0000,0x031C,0x6318,0x0063,0x6000,0x0000,0x0000,0x030C,
	0x6318,0x0C63,0x3000,0x0000,0x0000,0x0304,0x3E18,0x07C3,
	0x1800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x01C0,0x0000,0x03F8,0x0000,
	0x0C00,0x0000,0x0360,0x0000,0x030C,0x0000,0x0C00,0x0000,
	0x0000,0x0000,0x030C,0x7C6E,0x3F31,0x8FCC,0x63E1,0xF000,
	0x030C,0xC671,0x0C31,0x98CC,0x6633,0x1800,0x03F8,0xC660,
	0x0C31,0x98CC,0x6633,0x0000,0x0300,0xC660,0x0C31,0x98CC,
	0x67F1,0xF000,0x0300,0xC660,0x0C31,0x98CC,0x6600,0x1800,
	0x0300,0xC660,0x0C31,0x98CC,0x6603,0x1800,0x0300,0x7C60,
	0x061F,0x8FC7,0xB3E1,0xF000,0x0000,0x0000,0x0000,0x00C0,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x10C0,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0F80,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x01FC,0x0000,0x0000,0x1800,0x0000,0x0000,0x0306,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0300,0x3187,0xC3FE,
	0x1800,0x0000,0x0000,0x0300,0x318C,0x6333,0x1800,0x0000,
	0x0000,0x01FC,0x318C,0x6333,0x1800,0x0000,0x0000,0x0006,
	0x318C,0x6333,0x1800,0x0000,0x0000,0x0006,0x318C,0x6333,
	0x1800,0x0000,0x0000,0x0306,0x318C,0x6333,0x1800,0x0000,
	0x0000,0x01FC,0x1F07,0xC333,0x1800,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0007,0xF000,0x0000,0x0000,0xC000,0x0000,
	0x000C,0x1800,0x0000,0x0000,0xC000,0x0000,0x000C,0x00C6,
	0x1F0F,0xC1F0,0xC63E,0x0000,0x000C,0x00C6,0x318C,0x6308,
	0xCC03,0x0000,0x0007,0xF0C6,0x318C,0x6300,0xD803,0x0000,
	0x0000,0x18C6,0x3F8C,0x61F0,0xF03F,0x0000,0x0000,0x186C,
	0x300C,0x6018,0xD863,0x0000,0x000C,0x1838,0x318C,0x6218,
	0xCC63,0x0000,0x0007,0xF010,0x1F0C,0x61F0,0xC63F,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0C00,0x0006,0x0000,0x0000,0x0000,0x0000,0x040F,0xE002,
	0x0000,0x0000,0x0000,0x0001,0xC400,0x4082,0x0000,0x0000,
	0x0000,0x0007,0xE400,0x41C2,0x0000,0x0000,0x0000,0x0001,
	0xC400,0xC222,0x0000,0x0000,0x0000,0x0002,0x473F,0xF23E,
	0x0000,0x0000,0x0000,0x0002,0x4402,0x0222,0x0000,0x0000,
	0x0000,0x0001,0xC402,0x01C2,0x0000,0x0000,0x0000,0x0001,
	0x841F,0xC002,0x0000,0x0000,0x0000,0x0000,0x8000,0x4002,
	0x0000,0x0000,0x0000,0x0000,0xF800,0x4002,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x4002,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x4000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x1800,0x0180,0x0000,0x0000,
	0x0000,0x0000,0x1C00,0x01C0,0x0000,0x0000,0x0000,0x0000,
	0x1800,0x0180,0x0000,0x0000,0x0000,0x0000,0x1801,0x8001,
	0x8000,0x0000,0x0000,0x0007,0xFFE1,0xFFFF,0x8000,0x0000,
	0x0000,0x0007,0x1860,0xE007,0x0000,0x0000,0x0000,0x0007,
	0x18E0,0x0018,0x0000,0x0000,0x0000,0x0003,0x99C0,0x3C3C,
	0x0000,0x0000,0x0000,0x0003,0x99C0,0x7E70,0x0000,0x0000,
	0x0000,0x0001,0xFF80,0x07E0,0x0000,0x0000,0x0000,0x0000,
	0x1800,0x03C0,0x0000,0x0000,0x0000,0x0000,0x1800,0x0660,
	0x0000,0x0000,0x0000,0x0000,0x1800,0x1C3C,0x0000,0x0000,
	0x0000,0x0000,0x1800,0x781F,0x8000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0020,0x00FC,0xFFE0,0x0000,
	0x0000,0x7FF8,0x0020,0x0000,0x0400,0x0000,0x0000,0x4008,
	0x3FFF,0xF1FE,0x0400,0x0000,0x0000,0x4008,0x0070,0x0000,
	0x7F80,0x0000,0x0000,0x4008,0x00F8,0x00FC,0x0880,0x0000,
	0x0000,0x4008,0x01AC,0x0000,0x1080,0x0000,0x0000,0x7FF8,
	0x0326,0x00FC,0xFFE0,0x0000,0x0000,0x4008,0x0623,0x0000,
	0x0000,0x0000,0x0000,0x4008,0x0C21,0x80FC,0x7FC0,0x0000,
	0x0000,0x4008,0x3BFE,0xE084,0x4040,0x0000,0x0000,0x4008,
	0x1020,0x4084,0x4040,0x0000,0x0000,0x7FF8,0x0020,0x00FC,
	0x7FC0,0x0000,0x0000,0x4008,0x0020,0x0080,0x4040,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
};

static struct Image ln =
{
    0, 0,			/* LeftEdge, TopEdge */
    94, 15, 1,			/* Width, Height, Depth */
    lnData,			/* ImageData */
    0x0001, 0x0000,		/* PlanePick, PlaneOnOff */
    NULL			/* NextImage */
};

#define	LN_WIDTH	94
#define	LN_HEIGHT	15
#define	LN_BPR		(((LN_WIDTH + 15) >> 4) << 1)

/*****************************************************************************/

/* map from a language number to a slot in the picture which contains
 * the imagery for that language */
static const UBYTE LangMap[] =
{3, 0, 3, 2, 5, 4, 6, 9, 1, 7, 8, 10, 11, 14, 13, 0xff, 0xff, 0xff, 12};

/* map from a slot number to a language number */
static const UBYTE SlotMap[] =
{
    LANG_AMERICAN,
    LANG_DANISH,
    LANG_GERMAN,
    LANG_ENGLISH,
    LANG_SPANISH,
    LANG_FRENCH,
    LANG_ITALIAN,
    LANG_DUTCH,
    LANG_NORWEGIAN,
    LANG_PORTUGUESE,
    LANG_FINNISH,
    LANG_SWEDISH,
    LANG_KOREAN,
    LANG_CHINESE,
    LANG_JAPANESE
};

/*****************************************************************************/

/* Function prototypes */
void NewList (struct List *);
struct Library *openlibrary (STRPTR name, ULONG version);
struct Library *openclass (STRPTR name, ULONG version);
STRPTR GetString (GDP gd, LONG id);
void MessageBox (GDP gd, void *data, ...);

static GDP AllocGlobalData (struct WBStartup *);
static void HandleEvents (GDP gd);
static void FreeGlobalData (GDP gd);

/*****************************************************************************/

int cmd_start (void)
{
    struct WBStartup *sm = NULL;
    struct Process *process;
    GDP gd;

    LONG failureLevel = RETURN_FAIL;
    LONG failureCode = 0;

    SysBase = (*((struct ExecBase **) 4));
    process = (struct Process *) SysBase->ThisTask;
    if (!(process->pr_CLI))
    {
	WaitPort (&process->pr_MsgPort);
	sm = (struct WBStartup *) GetMsg (&process->pr_MsgPort);
    }

    if (SysBase->LibNode.lib_Version < 37)
    {
	failureCode = ERROR_INVALID_RESIDENT_LIBRARY;
    }
    else if (gd = AllocGlobalData (sm))
    {
	HandleEvents (gd);

	FreeGlobalData (gd);
    }
    else
    {
	failureCode = ERROR_NO_FREE_STORE;
    }

    if (sm)
    {
	Forbid ();
	ReplyMsg ((struct Message *) sm);
    }
    process->pr_Result2 = failureCode;
    return (failureLevel);
}

/*****************************************************************************/

static void FillOldExtent (GDP gd, struct RastPort *rp, struct Rectangle *oldExtent, struct Rectangle *newExtent)
{
    if (oldExtent->MinX < newExtent->MinX)
	EraseRect (rp, oldExtent->MinX,
		   oldExtent->MinY,
		   newExtent->MinX - 1,
		   oldExtent->MaxY);
    if (oldExtent->MaxX > newExtent->MaxX)
	EraseRect (rp, newExtent->MaxX + 1,
		   oldExtent->MinY,
		   oldExtent->MaxX,
		   oldExtent->MaxY);

    if (oldExtent->MaxY > newExtent->MaxY)
	EraseRect (rp, oldExtent->MinX,
		   newExtent->MaxY + 1,
		   oldExtent->MaxX,
		   oldExtent->MaxY);

    if (oldExtent->MinY < newExtent->MinY)
	EraseRect (rp, oldExtent->MinX,
		   oldExtent->MinY,
		   oldExtent->MaxX,
		   newExtent->MinY - 1);
}

/*****************************************************************************/

static void ASM NVCallBack
 (REG (a0)
  struct Hook *h, REG (a1)
  struct LVDrawMsg *msg, REG (a2)
  struct LocalEntry *le)
{
    UWORD *pens = msg->lvdm_DrawInfo->dri_Pens;
    struct RastPort *rp = msg->lvdm_RastPort;
    UBYTE state = msg->lvdm_State;
    struct TextExtent extent;
    GDP gd = h->h_Data;
    UBYTE buffer[60];
    UBYTE name[80];
    WORD slack;
    ULONG apen;
    ULONG bpen;
    ULONG fit;
    WORD x, y;

    /* Get the pens */
    apen = pens[FILLTEXTPEN];
    bpen = pens[FILLPEN];
    if ((state == LVR_NORMAL) || (state == LVR_NORMALDISABLED))
    {
	apen = pens[TEXTPEN];
	bpen = pens[BACKGROUNDPEN];
    }
    SetABPenDrMd (rp, apen, bpen, JAM2);

    /* Build the line item */
    sprintf (name, "%.21s-%.24s", le->le_AppName, le->le_ItemName);
    sprintf (buffer, "%-24.24s %3ld", name, le->le_Size);

    /* Measure the item */
    fit = TextFit (rp, buffer, strlen (buffer), &extent, NULL, 1,
		   msg->lvdm_Bounds.MaxX - msg->lvdm_Bounds.MinX - 3,
		   msg->lvdm_Bounds.MaxY - msg->lvdm_Bounds.MinY + 1);
    slack = (msg->lvdm_Bounds.MaxY - msg->lvdm_Bounds.MinY) - (extent.te_Extent.MaxY - extent.te_Extent.MinY);
    x = msg->lvdm_Bounds.MinX - extent.te_Extent.MinX + 2;
    y = msg->lvdm_Bounds.MinY - extent.te_Extent.MinY + ((slack + 1) / 2);
    extent.te_Extent.MinX += x;
    extent.te_Extent.MaxX += x;
    extent.te_Extent.MinY += y;
    extent.te_Extent.MaxY += y;

    /* Draw the item */
    Move (rp, x, y);
    Text (rp, buffer, fit);

    /* Erase the remainder of the line */
    SetAPen (rp, bpen);
    FillOldExtent (gd, rp, &msg->lvdm_Bounds, &extent.te_Extent);
}

/*****************************************************************************/

static void ASM LLCallBack
 (REG (a0)
  struct Hook *h, REG (a1)
  struct LVDrawMsg *msg, REG (a2)
  struct LangEntry *le)
{
    UWORD *pens = msg->lvdm_DrawInfo->dri_Pens;
    struct RastPort *rp = msg->lvdm_RastPort;
    UBYTE state = msg->lvdm_State;
    GDP gd = h->h_Data;
    ULONG apen;
    ULONG bpen;

    apen = pens[FILLTEXTPEN];
    bpen = pens[FILLPEN];
    if ((state == LVR_NORMAL) || (state == LVR_NORMALDISABLED))
    {
	apen = pens[TEXTPEN];
	bpen = pens[BACKGROUNDPEN];
    }
    SetABPenDrMd (rp, apen, bpen, JAM2);
    BltTemplate ((void *) le->le_Data, 0, LN_BPR, rp, msg->lvdm_Bounds.MinX, msg->lvdm_Bounds.MinY, LN_WIDTH, LN_HEIGHT);
}

/*****************************************************************************/

static void InitSupport (GDP gd)
{
    if (gd->gd_Page == 1)
    {
	struct LocalEntry *le;
	struct NVEntry *nve;
	STRPTR appname;

	if (NVBase = openlibrary ("nonvolatile.library", 0))
	{
	    gd->gd_EList = GetNVList (NULL, FALSE);
	    gd->gd_MaxNV = 0;
	    nve = (struct NVEntry *) gd->gd_EList->mlh_Head;
	    while (nve->nve_Node.mln_Succ)
	    {
		if (nve->nve_Protection & NVEF_APPNAME)
		{
		    appname = nve->nve_Name;
		}
		else if (le = AllocVec (sizeof (LE), MEMF_ANY))
		{
		    le->le_ItemName = nve->nve_Name;
		    le->le_AppName = appname;
		    le->le_Size = nve->nve_Size;
		    le->le_Protection = nve->nve_Protection;
		    gd->gd_MaxNV++;
		    AddTail ((struct List *) &gd->gd_ItemList, (struct Node *) le);
		}
		nve = (struct NVEntry *) nve->nve_Node.mln_Succ;
	    }
	}
	else
	    MessageBox (gd, "%s\n", GetString (gd, ERR_COULDNT_OPEN_NONVOLATILE));
    }
    else if (gd->gd_Page == 2)
    {
	if (LowLevelBase = OpenLibrary ("lowlevel.library", 0))
	    gd->gd_PrevLang = gd->gd_CurLang = LangMap[GetLanguageSelection ()];
	else
	    MessageBox (gd, "%s\n", GetString (gd, ERR_COULDNT_OPEN_LOWLEVEL));
    }
}

/*****************************************************************************/

static void BootMode (GDP gd, BOOL set)
{
    if (NVBase = openlibrary ("nonvolatile.library", 0))
    {
	UBYTE buffer[20];
	char *ra;

	if (ra = GetCopyNV ("", "", TRUE))
	{
	    CopyMem (ra, buffer, 16);
	    if (set)
	    {
		if (gd->gd_CurStartup)
		    buffer[1] |= 0x01;
		else
		    buffer[1] &= ~0x01;
		StoreNV ("", "", buffer, 16, TRUE);
	    }
	    else
		gd->gd_CurStartup = (ra[1] & 0x01) ? 1 : 0;

#if 0
{
LONG i;
for (i = 15; i >= 0; i--)
Printf ("%02lx ", (ULONG)buffer[i]);
Printf ("\n");
}
#endif

	    gd->gd_PrevStartup = gd->gd_CurStartup;
	    FreeNVData (ra);
	}
#if 0
	else if (set)
	{
	    UBYTE tmp[18];

	    memset (tmp, 0, sizeof (tmp));
	    tmp[1] = (gd->gd_CurStartup) ? 0x01 : 0;
	    Printf ("Create system area\n");
	    switch (StoreNV ("", "", tmp, 16, TRUE))
	    {
		case 0:
		    Printf ("Stored\n");
		    break;
		case NVERR_BADNAME:
		    Printf ("Error in AppName, or ItemName\n");
		    break;
		case NVERR_WRITEPROT:
		    Printf ("Nonvolatile storage cannot be written\n");
		    break;
		case NVERR_FAIL:
		    Printf ("Failure in writing data (FULL)\n");
		    break;
		case NVERR_FATAL:
		    Printf ("Fatal error.  Possible loss of data\n");
		    break;
		default:
		    Printf ("Unknown return code\n");
	    }
	}
#endif

	CloseLibrary (NVBase);
	NVBase = NULL;
    }
    else
	MessageBox (gd, "%s\n", GetString (gd, ERR_COULDNT_OPEN_NONVOLATILE));
}

/*****************************************************************************/

static void ShutdownSupport (GDP gd)
{
    struct Node *ln;

    if (gd->gd_PrevStartup != gd->gd_CurStartup)
    {
	BootMode (gd, TRUE);
	gd->gd_PrevStartup = gd->gd_CurStartup;
    }

    if (LowLevelBase)
    {
	if (gd->gd_PrevLang != gd->gd_CurLang)
	{
	    SetLanguageSelection (SlotMap[gd->gd_CurLang]);
	    gd->gd_PrevLang = gd->gd_CurLang;
	}

	CloseLibrary (LowLevelBase);
	LowLevelBase = NULL;
    }

    if (NVBase)
    {
	/* Free the item list */
	while (ln = RemHead ((struct List *) &gd->gd_ItemList))
	    FreeVec (ln);

	FreeNVData (gd->gd_EList);
	gd->gd_EList = NULL;

	CloseLibrary (NVBase);
	NVBase = NULL;
    }
}

/*****************************************************************************/

static void UpdateSizes (GDP gd)
{
    ULONG locked, unlocked;
    LEP le;

    /* Count the number of blocks that are locked and unlocked */
    locked = unlocked = 0;
    for (le = (LEP) gd->gd_ItemList.mlh_Head; le->le_Link.mln_Succ; le = (LEP) le->le_Link.mln_Succ)
    {
	if (le->le_Protection & NVEF_DELETE)
	    locked += le->le_Size;
	else
	    unlocked += le->le_Size;
    }

    /* Update the gadgets */
    GT_SetGadgetAttrs (gd->gd_GTG[GID_LOCKED], gd->gd_Window, NULL, GTNM_Number, locked, TAG_DONE);
    GT_SetGadgetAttrs (gd->gd_GTG[GID_UNLOCKED], gd->gd_Window, NULL, GTNM_Number, unlocked, TAG_DONE);
    GT_SetGadgetAttrs (gd->gd_GTG[GID_PROTECT], gd->gd_Window, NULL,
		       GTCB_Checked, (gd->gd_Entry) ? (gd->gd_Entry->le_Protection & NVEF_DELETE) : FALSE,
		       GA_Disabled, (gd->gd_Entry ? FALSE : TRUE),
		       TAG_DONE);
    GT_SetGadgetAttrs (gd->gd_GTG[GID_DELETE], gd->gd_Window, NULL,
		       GA_Disabled, (gd->gd_Entry) ? (gd->gd_Entry->le_Protection & NVEF_DELETE) : TRUE,
		       TAG_DONE);
}

/*****************************************************************************/

static void SetCurrentNV (GDP gd, UWORD current)
{
    UWORD i;

    gd->gd_CurNV = current;
    if (IsListEmpty ((struct List *) &gd->gd_ItemList))
    {
	gd->gd_Entry = NULL;
    }
    else
    {
	for (i = 0, gd->gd_Entry = (struct LocalEntry *) gd->gd_ItemList.mlh_Head;
	     (i < current) && gd->gd_Entry->le_Link.mln_Succ;
	     i++, gd->gd_Entry = (struct LocalEntry *) gd->gd_Entry->le_Link.mln_Succ)
	{
	}
    }
    UpdateSizes (gd);
}

/*****************************************************************************/

static void ChangePages (GDP gd, UWORD page, BOOL init)
{
    struct Window *win = gd->gd_Window;
    struct RastPort *rp = win->RPort;
    struct NewGadget ng;
    LONG innerWidth;

    SetWindowPointer (win, WA_BusyPointer, TRUE, TAG_DONE);

    /* Remove any previous pages here */
    if (gd->gd_GTG[GID_CONTEXT])
    {
	RemoveGList (win, gd->gd_TabGadget, -1);
	gd->gd_TabGadget->NextGadget = NULL;
	SetAPen (rp, gd->gd_DRI->dri_Pens[BACKGROUNDPEN]);
	RectFill (rp,
		  win->BorderLeft, gd->gd_TabGadget->TopEdge + gd->gd_TabGadget->Height,
		  win->Width - win->BorderRight - 1, win->Height - win->BorderBottom - 1);
	FreeGadgets (gd->gd_GTG[GID_CONTEXT]);
	gd->gd_GTG[GID_CONTEXT] = NULL;
    }

    /* Close down any previous lists */
    ShutdownSupport (gd);

    /* Set the current page */
    gd->gd_Page = page;

    /* Initialize the NV list */
    InitSupport (gd);

    /* Compute the inner width */
    innerWidth = win->Width - win->BorderLeft - win->BorderRight - 8;

    /* Start creating GagTools gadgets */
    gd->gd_GTG[GID_CONTEXT] = NULL;
    gd->gd_LastAdded = CreateContext (&gd->gd_GTG[GID_CONTEXT]);
    memset (&ng, 0, sizeof (struct NewGadget));

    ng.ng_TextAttr = &topaz8;
    ng.ng_VisualInfo = gd->gd_VI;
    ng.ng_UserData = gd;

    /* STARTUP */
    if (page == 0)
    {
	/* Startup mode */
	ng.ng_Width = 86;
	ng.ng_LeftEdge = win->BorderLeft + 4 + ((innerWidth - ng.ng_Width) / 2);
	ng.ng_TopEdge = gd->gd_TabGadget->TopEdge + gd->gd_TabGadget->Height + 4 + 26;
	ng.ng_Height = 18;
	ng.ng_GadgetID = GID_LIST;
	ng.ng_Flags = PLACETEXT_RIGHT;
	gd->gd_LastAdded = gd->gd_GTG[GID_LIST] = CreateGadget (MX_KIND, gd->gd_LastAdded, &ng,
								GTMX_Labels, gd->gd_StartupLabels,
								GTMX_Spacing, 4,
								GTMX_Active, (ULONG) gd->gd_CurStartup,
								GTMX_TitlePlace, NG_HIGHLABEL | PLACETEXT_ABOVE,
								TAG_DONE);
    }
    /* NONVOLATILE */
    else if (page == 1)
    {
	struct Gadget *g;
	LONG width;

	/* Set the width */
	width = 120;
	SetAttrs (gd->gd_Sep, IA_Width, width, TAG_DONE);

	/* List of nonvolatile items */
	ng.ng_LeftEdge = win->BorderLeft + 4;
	ng.ng_TopEdge = gd->gd_TabGadget->TopEdge + gd->gd_TabGadget->Height + 4;
	ng.ng_Width = win->Width - win->BorderLeft - win->BorderRight - 12 - width;
	ng.ng_Height = win->Height - ng.ng_TopEdge - win->BorderBottom - 2;
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = GID_LIST;
	gd->gd_LastAdded = gd->gd_GTG[GID_LIST] = CreateGadget (LISTVIEW_KIND, gd->gd_LastAdded, &ng,
								GTLV_ShowSelected, NULL,
								GTLV_CallBack, &gd->gd_ListHook,
								GTLV_Labels, &gd->gd_ItemList,
								GTLV_ScrollWidth, 18,
								TAG_DONE);

	/* Is the current one locked or not */
	ng.ng_LeftEdge += ng.ng_Width + 4;
	ng.ng_Width = 24;
	ng.ng_Height = CHECKBOX_HEIGHT;
	ng.ng_Flags = PLACETEXT_RIGHT;
	ng.ng_GadgetText = GetString (gd, ID_LOCK);
	ng.ng_GadgetID = GID_PROTECT;
	gd->gd_LastAdded = gd->gd_GTG[GID_PROTECT] = CreateGadget (CHECKBOX_KIND, gd->gd_LastAdded, &ng,
								   GA_Disabled, TRUE,
								   TAG_DONE);

	/* Delete the current */
	ng.ng_TopEdge += ng.ng_Height + 2;
	ng.ng_Width = width;
	ng.ng_Height = 14;
	ng.ng_GadgetText = GetString (gd, ID_DELETE);
	ng.ng_GadgetID = GID_DELETE;
	ng.ng_Flags = NULL;
	gd->gd_LastAdded = gd->gd_GTG[GID_DELETE] = CreateGadget (BUTTON_KIND, gd->gd_LastAdded, &ng,
								  GA_Disabled, TRUE,
								  TAG_DONE);

	/* First separator bar */
	ng.ng_TopEdge += ng.ng_Height + 4;
	ng.ng_Height = 2;
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = GID_SEP1;
	gd->gd_LastAdded = gd->gd_GTG[GID_SEP1] = CreateGadget (GENERIC_KIND, gd->gd_LastAdded, &ng,
								TAG_DONE);
	if (g = gd->gd_LastAdded)
	{
	    g->Flags |= (GFLG_GADGIMAGE | GFLG_GADGHNONE);
	    g->GadgetRender = gd->gd_Sep;
	}

	/* Locked blocks */
	ng.ng_LeftEdge += 82;
	ng.ng_TopEdge += ng.ng_Height + 4;
	ng.ng_Width = 38;
	ng.ng_Height = 14;
	ng.ng_GadgetText = GetString (gd, ID_LOCKED);
	ng.ng_GadgetID = GID_LOCKED;
	gd->gd_LastAdded = gd->gd_GTG[GID_LOCKED] = CreateGadget (NUMBER_KIND, gd->gd_LastAdded, &ng,
								  GTNM_Border, TRUE,
								  GTNM_Justification, GTJ_RIGHT,
								  TAG_DONE);

	/* Locked blocks */
	ng.ng_TopEdge += ng.ng_Height + 2;
	ng.ng_GadgetText = GetString (gd, ID_UNLOCKED);
	ng.ng_GadgetID = GID_UNLOCKED;
	gd->gd_LastAdded = gd->gd_GTG[GID_UNLOCKED] = CreateGadget (NUMBER_KIND, gd->gd_LastAdded, &ng,
								    GTNM_Border, TRUE,
								    GTNM_Justification, GTJ_RIGHT,
								    TAG_DONE);

	/* Second separator bar */
	ng.ng_LeftEdge -= 82;
	ng.ng_TopEdge += ng.ng_Height + 4;
	ng.ng_Width = width;
	ng.ng_Height = 2;
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = GID_SEP2;
	gd->gd_LastAdded = gd->gd_GTG[GID_SEP2] = CreateGadget (GENERIC_KIND, gd->gd_LastAdded, &ng,
								TAG_DONE);
	if (g = gd->gd_LastAdded)
	{
	    g->Flags |= (GFLG_GADGIMAGE | GFLG_GADGHNONE);
	    g->GadgetRender = gd->gd_Sep;
	}

	/* Format a new disk */
	ng.ng_TopEdge += ng.ng_Height + 4;
	ng.ng_Height = 14;
	ng.ng_GadgetText = GetString (gd, ID_PREPARE);
	ng.ng_GadgetID = GID_PREPARE;
	gd->gd_LastAdded = gd->gd_GTG[GID_PREPARE] = CreateGadget (BUTTON_KIND, gd->gd_LastAdded, &ng,
								   TAG_DONE);
    }
    /* LANGUAGE */
    else if (page == 2)
    {
	/* List of languages */
	ng.ng_Width = LN_WIDTH + 18 + 4;
	ng.ng_LeftEdge = win->BorderLeft + 4 + ((innerWidth - ng.ng_Width) / 2);
	ng.ng_TopEdge = gd->gd_TabGadget->TopEdge + gd->gd_TabGadget->Height + 4;
	ng.ng_Height = win->Height - ng.ng_TopEdge - win->BorderBottom - 2;
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = GID_LIST;
	gd->gd_LastAdded = gd->gd_GTG[GID_LIST] = CreateGadget (LISTVIEW_KIND, gd->gd_LastAdded, &ng,
								GTLV_CallBack, &gd->gd_LanguageHook,
								GTLV_ItemHeight, 15,
								GTLV_Labels, &gd->gd_LanguageList,
								GTSC_Arrows, 10,
								GTLV_ScrollWidth, 18,
								GTLV_ShowSelected, NULL,
								GTLV_Selected, (ULONG) gd->gd_CurLang,
								GTLV_MakeVisible, (ULONG) gd->gd_CurLang,
								TAG_DONE);
    }

    /* Add the gadgets */
    AddGList (win, gd->gd_TabGadget, -1, -1, NULL);
    if (gd->gd_LastAdded)
	AddGList (win, gd->gd_GTG[GID_CONTEXT], -1, -1, NULL);
    if (init)
	RefreshGList (gd->gd_TabGadget, win, NULL, -1);
    else
	RefreshGList (gd->gd_GTG[GID_CONTEXT], win, NULL, -1);
    GT_RefreshWindow (win, NULL);

    if (page == 1)
	UpdateSizes (gd);

    SetWindowPointer (win, WA_BusyPointer, FALSE, TAG_DONE);
}

/*****************************************************************************/

static GDP AllocGlobalData (struct WBStartup *sm)
{
    UWORD *data;
    LONG i, j;
    LGEP le;
    GDP gd;

    if (gd = AllocVec (sizeof (GD), MEMF_CLEAR))
    {
	DOSBase = OpenLibrary ("dos.library", 37);
	GadToolsBase = OpenLibrary ("gadtools.library", 37);
	GfxBase = OpenLibrary ("graphics.library", 37);
	IntuitionBase = OpenLibrary ("intuition.library", 37);
	UtilityBase = OpenLibrary ("utility.library", 37);
	if (LocaleBase = OpenLibrary ("locale.library", 38))
	    gd->gd_Catalog = OpenCatalogA (NULL, "Sys/amigacd.catalog", NULL);

	/* Build the strings */
	gd->gd_StartupLabels[0] = GetString (gd, ID_AMIGA);
	gd->gd_StartupLabels[1] = GetString (gd, ID_AMIGACD);
	for (i = 0; i < 3; i++)
	{
	    gd->gd_TabLabels[i].tl_Label = GetString (gd, TAB_STARTUP + i);
	    for (j = 0; j < MAX_TL_PENS; j++)
		gd->gd_TabLabels[i].tl_Pens[j] = -1;
	}

	/* Make sure we have the correct current directory */
	if (gd->gd_SM = sm)
	    gd->gd_oldDir = CurrentDir (sm->sm_ArgList->wa_Lock);

	/* Initialize the lists */
	NewList ((struct List *) &gd->gd_ItemList);

	/* Initialize the list view call back */
	gd->gd_ListHook.h_Entry = (HOOKFUNC) NVCallBack;
	gd->gd_ListHook.h_Data = gd;

	/* Initialize the list view call back */
	gd->gd_LanguageHook.h_Entry = (HOOKFUNC) LLCallBack;
	gd->gd_LanguageHook.h_Data = gd;
	NewList ((struct List *) &gd->gd_LanguageList);
	for (i = 0, data = lnData; i < 15; i++)
	{
	    if (le = AllocVec (sizeof (LGE), MEMF_CLEAR))
	    {
		le->le_Which = i;
		le->le_Data = data;
		data += LN_BPR * LN_HEIGHT / 2;
		AddTail ((struct List *) &gd->gd_LanguageList, (struct Node *) le);
	    }
	}

	/* Get the boot mode */
	BootMode (gd, FALSE);

	if (gd->gd_TabsClass = openclass ("gadgets/tabs.gadget", 39))
	{
	    gd->gd_Screen = LockPubScreen (NULL);
	    gd->gd_DRI = GetScreenDrawInfo (gd->gd_Screen);
	    gd->gd_VI = GetVisualInfoA (gd->gd_Screen, NULL);

	    gd->gd_Sep = NewObject (NULL, FRAMEICLASS,
				    IA_Width, 2,
				    IA_Height, 2,
				    IA_Recessed, TRUE,
				    IA_FrameType, FRAME_DEFAULT,
				    SYSIA_DrawInfo, gd->gd_DRI,
				    TAG_DONE);

	    if (gd->gd_Window = OpenWindowTags (NULL,
						WA_PubScreen, (ULONG) gd->gd_Screen,
						WA_InnerWidth, 383,
						WA_InnerHeight, 118L,
						WA_Title, (ULONG) GetString (gd, WIN_TITLE),
						WA_DragBar, TRUE,
						WA_DepthGadget, TRUE,
						WA_CloseGadget, TRUE,
						WA_NoCareRefresh, TRUE,
						WA_NewLookMenus, TRUE,
						WA_AutoAdjust, TRUE,
						WA_IDCMP, IDCMP_FLAGS,
						TAG_DONE))
	    {
		/* The height of the gadget should be the font height + 7 */
		if (gd->gd_TabGadget = NewObject (NULL, "tabs.gadget",
						  GA_Top, gd->gd_Window->BorderTop + 2,
						  GA_Left, gd->gd_Window->BorderLeft + 4,
						  GA_Height, 8 + 7,
						  GA_RelWidth, -(gd->gd_Window->BorderLeft + 8 + gd->gd_Window->BorderRight),
						  GA_RelVerify, TRUE,
						  GA_Immediate, TRUE,
						  GA_TextAttr, &topaz8,
						  GA_ID, GID_TABS,
						  LAYOUTA_ChildMaxWidth, FALSE,
						  TABS_Labels, gd->gd_TabLabels,
						  TABS_Current, 0,
						  TAG_DONE))
		{
		    /* Initialize the pages */
		    ChangePages (gd, 0, TRUE);
		    return gd;
		}
		else
		    MessageBox (gd, "%s\n", GetString (gd, ERR_NO_FREE_STORE));
		CloseWindow (gd->gd_Window);
		gd->gd_Window = NULL;
	    }
	    else
		MessageBox (gd, "%s\n", GetString (gd, ERR_NO_FREE_STORE));
	}
	else
	    MessageBox (gd, "%s\n", GetString (gd, ERR_COULDNT_OPEN_TABS));
	FreeVec (gd);
    }
    return NULL;
}

/*****************************************************************************/

static void AdjustName (GDP gd, STRPTR name)
{
    LONG tmp;

    tmp = strlen (name);
    if (name[tmp - 1] == '\n')
	name[--tmp] = 0;
    if ((tmp > 11) && (Stricmp (&name[tmp - 11], "nonvolatile") == 0))
	name[tmp - 11] = 0;
}

/*****************************************************************************/

static void PrepareNV (GDP gd)
{
    struct FileInfoBlock __aligned fib;
    struct FileRequester *fr;
    UBYTE name[512];
    BPTR lock;
    LONG tmp;

    /* Open the common dialog library */
    if (AslBase = OpenLibrary ("asl.library", 0))
    {
	/* See if we already have a name to work with */
	memset (name, 0, sizeof (name));
	if (lock = Open ("ENVARC:Sys/nv_location", MODE_OLDFILE))
	{
	    if (Read (lock, name, sizeof (name)) > 0)
		AdjustName (gd, name);
	    Close (lock);
	}

	/* Allocate the file requester */
	if (fr = AllocAslRequestTags (ASL_FileRequest,
				      ASLFR_InitialDrawer, name,
				      ASLFR_TitleText, GetString (gd, WIN_PREPARE),
				      ASLFR_DrawersOnly, TRUE,
				      ASLFR_RejectIcons, TRUE,
				      TAG_DONE))
	{
	    /* Open the file requester */
	    if (AslRequestTags (fr, TAG_DONE))
	    {
		/* Make sure we can lock the drawer */
		if (lock = Lock (fr->fr_Drawer, ACCESS_READ))
		{
		    /* Make sure it is a drawer or volume */
		    tmp = 0;
		    if (Examine (lock, &fib) && (fib.fib_DirEntryType > 0))
			tmp = 1;

		    /* Get the full name */
		    NameFromLock (lock, name, sizeof (name));
		    UnLock (lock);

		    if (tmp)
		    {
			/* Get rid of any nonvolatile name */
			AdjustName (gd, name);

			/* Add the nonvolatile portion to the name */
			tmp = 0;
			AddPart (name, "nonvolatile", sizeof (name));
			if (lock = CreateDir (name))
			{
			    tmp = 1;
			    UnLock (lock);
			}
			else if (lock = Lock (name, ACCESS_READ))
			{
			    if (Examine (lock, &fib) && (fib.fib_DirEntryType > 0))
				tmp = 1;
			    UnLock (lock);
			}

			/* Make sure we should point to it */
			if (tmp)
			{
			    /* Create the pointer file */
			    if (lock = Open ("ENVARC:Sys/nv_location", MODE_NEWFILE))
			    {
				Write (lock, name, strlen (name));
				Close (lock);

				/* Update things */
				GT_SetGadgetAttrs (gd->gd_GTG[GID_LIST], gd->gd_Window, NULL,
						   GTLV_Labels, ~0,
						   TAG_DONE);
				gd->gd_Entry = NULL;
				ShutdownSupport (gd);
				InitSupport (gd);
				GT_SetGadgetAttrs (gd->gd_GTG[GID_LIST], gd->gd_Window, NULL,
						   GTLV_Labels, &gd->gd_ItemList,
						   TAG_DONE);
				SetCurrentNV (gd, gd->gd_CurNV);
			    }
			    else
				MessageBox (gd, "%s\n", GetString (gd, ERR_COULDNT_CREATE_POINTER));
			}
			else
			    MessageBox (gd, "%s\n", GetString (gd, ERR_COULDNT_CREATE_NONVOLATILE));
		    }
		    else
			MessageBox (gd, "%s\n", GetString (gd, ERR_REQUIRES_A_DIRECTORY_NAME));
		}
		else
		    MessageBox (gd, "%s %s\n", GetString (gd, ERR_COULDNT_LOCK_DIRECTORY), name);
	    }
	    FreeAslRequest (fr);
	}
	else
	    MessageBox (gd, "%s\n", GetString (gd, ERR_NO_FREE_STORE));
	CloseLibrary (AslBase);
    }
    else
	MessageBox (gd, "%s\n", GetString (gd, ERR_COULDNT_OPEN_ASL));
}

/*****************************************************************************/

static void HandleEvents (GDP gd)
{
    struct IntuiMessage *imsg;
    LONG going = TRUE;
    struct Gadget *g;
    ULONG sigr;

    while (going)
    {
	sigr = Wait ((1L << gd->gd_Window->UserPort->mp_SigBit) | SIGBREAKF_CTRL_C);

	if (sigr & SIGBREAKF_CTRL_C)
	    going = FALSE;

	while (imsg = GT_GetIMsg (gd->gd_Window->UserPort))
	{
	    switch (imsg->Class)
	    {
		case IDCMP_CLOSEWINDOW:
		    going = FALSE;
		    break;

		case IDCMP_VANILLAKEY:
		    switch (imsg->Code)
		    {
			case 27:
			case 'q':
			case 'Q':
			    going = FALSE;
			    break;
		    }
		    break;

		case IDCMP_GADGETDOWN:
		    g = (struct Gadget *) imsg->IAddress;
		    switch (g->GadgetID)
		    {
			case GID_LIST:
			    if (gd->gd_Page == 0)
				gd->gd_CurStartup = imsg->Code;
			    break;
		    }
		    break;

		case IDCMP_GADGETUP:
		    g = (struct Gadget *) imsg->IAddress;
		    switch (g->GadgetID)
		    {
			case GID_PREPARE:
			    PrepareNV (gd);
			    break;

			case GID_TABS:
			    ChangePages (gd, imsg->Code, FALSE);
			    gd->gd_Entry = NULL;
			    break;

			case GID_LIST:
			    if (gd->gd_Page == 1)
				SetCurrentNV (gd, imsg->Code);
			    else if (gd->gd_Page == 2)
				gd->gd_CurLang = imsg->Code;
			    break;

			case GID_PROTECT:
			    if (gd->gd_Entry)
			    {
				/* Set the protection */
				if (imsg->Code)
				{
				    gd->gd_Entry->le_Protection |= NVEF_DELETE;
				}
				else
				{
				    gd->gd_Entry->le_Protection &= ~NVEF_DELETE;
				}
				SetNVProtection (gd->gd_Entry->le_AppName, gd->gd_Entry->le_ItemName,
						 gd->gd_Entry->le_Protection & NVEF_DELETE, TRUE);
				UpdateSizes (gd);
			    }
			    break;

			case GID_DELETE:
			    /* Delete the nonvolatile entry */
			    if (gd->gd_Entry)
			    {
				/* Adjust the current and maximum */
				gd->gd_MaxNV--;
				if (gd->gd_CurNV >= gd->gd_MaxNV)
				    gd->gd_CurNV--;

				/* Unlink the list from the gadget */
				GT_SetGadgetAttrs (gd->gd_GTG[GID_LIST], gd->gd_Window, NULL,
						   GTLV_Labels, ~0,
						   TAG_DONE);

				/* Delete the entry */
				if (DeleteNV (gd->gd_Entry->le_AppName, gd->gd_Entry->le_ItemName, TRUE))
				{
				    Remove ((struct Node *) gd->gd_Entry);
				    FreeVec (gd->gd_Entry);
				    gd->gd_Entry = NULL;
				}

				/* Relink the list to the gadget */
				GT_SetGadgetAttrs (gd->gd_GTG[GID_LIST], gd->gd_Window, NULL,
						   GTLV_Labels, &gd->gd_ItemList,
						   GTLV_Selected, (ULONG) gd->gd_CurNV,
						   TAG_DONE);
				SetCurrentNV (gd, gd->gd_CurNV);
			    }
			    break;
		    }
		    break;
	    }

	    GT_ReplyIMsg (imsg);
	}
    }
}

/*****************************************************************************/

static void FreeGlobalData (GDP gd)
{
    struct Node *ln;

    if (gd)
    {
	/* Delete the gadget */
	RemoveGList (gd->gd_Window, gd->gd_TabGadget, -1);
	DisposeObject (gd->gd_TabGadget);
	FreeGadgets (gd->gd_GTG[GID_CONTEXT]);

	CloseWindow (gd->gd_Window);
	gd->gd_Window = NULL;
	FreeVisualInfo (gd->gd_VI);
	FreeScreenDrawInfo (gd->gd_Screen, gd->gd_DRI);
	UnlockPubScreen (NULL, gd->gd_Screen);

	/* Free the language list */
	while (ln = RemHead ((struct List *) &gd->gd_LanguageList))
	    FreeVec (ln);

	/* Restore the current directory */
	if (gd->gd_SM)
	    CurrentDir (gd->gd_oldDir);

	if (gd->gd_Catalog)
	    CloseCatalog (gd->gd_Catalog);

	/* Close nonvolatile */
	ShutdownSupport (gd);

	CloseLibrary (gd->gd_TabsClass);
	CloseLibrary (UtilityBase);
	CloseLibrary (LocaleBase);
	CloseLibrary (IntuitionBase);
	CloseLibrary (GfxBase);
	CloseLibrary (GadToolsBase);
	CloseLibrary (DOSBase);
	FreeVec (gd);
    }
}

/*****************************************************************************/

/* Try opening the library from a number of common places */
struct Library *openlibrary (STRPTR name, ULONG version)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct Library *retval;
    STRPTR buffer;

    if ((retval = OpenLibrary (name, version)) == NULL)
    {
	if (buffer = (STRPTR) AllocMem (256, MEMF_CLEAR))
	{
	    sprintf (buffer, ":libs/%s", name);
	    if ((retval = OpenLibrary (buffer, version)) == NULL)
	    {
		sprintf (buffer, "libs/%s", name);
		retval = OpenLibrary (buffer, version);
	    }
	    FreeMem (buffer, 256);
	}
    }

    return retval;
}

/*****************************************************************************/

/* Try opening the class library from a number of common places */
struct Library *openclass (STRPTR name, ULONG version)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct Library *retval;
    STRPTR buffer;

    if ((retval = OpenLibrary (name, version)) == NULL)
    {
	if (buffer = (STRPTR) AllocMem (256, MEMF_CLEAR))
	{
	    sprintf (buffer, ":classes/%s", name);
	    if ((retval = OpenLibrary (buffer, version)) == NULL)
	    {
		sprintf (buffer, "classes/%s", name);
		retval = OpenLibrary (buffer, version);
	    }
	    FreeMem (buffer, 256);
	}
    }

    return retval;
}

/*****************************************************************************/

STRPTR GetString (GDP gd, LONG stringNum)
{
    STRPTR builtIn;
    UWORD *w;
    LONG *l;

    l = (LONG *) CatCompBlock;
    while (*l != stringNum)
    {
	w = (UWORD *) ((ULONG) l + 4);
	l = (LONG *) ((ULONG) l + (ULONG) * w + 6);
    }
    builtIn = (STRPTR) ((ULONG) l + 6);
    if (LocaleBase)
	return (GetCatalogStr (gd->gd_Catalog, stringNum, builtIn));
    return (builtIn);
}

/*****************************************************************************/

void MessageBox (GDP gd, void *data, ...)
{
    struct Process *pr = (struct Process *) FindTask (NULL);
    ULONG *fmt, *args;

    /* Find the format string and the arguments */
    args = fmt = (ULONG *) &data;
    args++;

    if (pr->pr_CLI)
    {
	static void __asm myputch (register __d0 char ch, register __a3 LONG *args);
	ULONG arg[2];

	arg[0] = (ULONG) DOSBase;
	arg[1] = (ULONG) Output ();
	RawDoFmt ((STRPTR)*fmt, args, (void (*)())myputch, (APTR)arg);
	Flush (Output());
    }
    else
    {
	struct EasyStruct es;

	/* Fill in the structure */
	es.es_StructSize = sizeof (struct EasyStruct);
	es.es_Flags = NULL;
	es.es_Title = "CDPrefs Message";
	es.es_TextFormat = (STRPTR)*fmt;
	es.es_GadgetFormat = "OK";
	EasyRequestArgs (gd->gd_Window, &es, NULL, args);
    }
}

/*****************************************************************************/

static void __asm myputch (register __d0 char ch, register __a3 LONG *args)
{
#undef	DOSBase
#define	DOSBase	args[0]
    FPutC (args[1], (LONG)ch);
}
