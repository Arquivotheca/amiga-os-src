/******************************************************************************
 *
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1992, 1993
 * Commodore-Amiga, Inc.  All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability, performance,
 * currentness, or operation of this software, and all use is at your own risk.
 * Neither commodore nor the authors assume any responsibility or liability
 * whatsoever with respect to your use of this software.
 *
 ******************************************************************************
 * clipview.h
 *
 */

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <dos/dosextens.h>
#include <devices/clipboard.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/icclass.h>
#include <intuition/imageclass.h>
#include <intuition/gadgetclass.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/cghooks.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/layers.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/textclass.h>
#include <libraries/locale.h>
#include <utility/hooks.h>
#include <workbench/icon.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <string.h>

#include <clib/macros.h>
#include <clib/asl_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/icon_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <clib/datatypes_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/layers_protos.h>
#include <clib/locale_protos.h>
#include <clib/wb_protos.h>

#include <pragmas/asl_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/datatypes_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/wb_pragmas.h>

#include "windowclass.h"

/*****************************************************************************/

#define	TEMPLATE	"PUBSCREEN/K,CLIPUNIT/K/N"
#define	OPT_PUBSCREEN	0
#define	OPT_CLIPUNIT	1
#define	NUM_OPTS	2

/*****************************************************************************/

#define	NAMEBUFSIZE	512

struct GlobalData
{
    /* Library bases */
    struct Library		*gd_SysBase;
    struct Library		*gd_DOSBase;
    struct Library		*gd_IconBase;
    struct Library		*gd_IntuitionBase;
    struct Library		*gd_GfxBase;
    struct Library		*gd_UtilityBase;
    struct Library		*gd_GadToolsBase;
    struct Library		*gd_LayersBase;
    struct Library		*gd_AslBase;
    struct Library		*gd_DataTypesBase;
    struct Library		*gd_LocaleBase;

    /* Clipboard hook */
    struct Hook			 gd_Hook;
    struct IOClipReq		*gd_ClipIO;
    LONG			 gd_Unit;

    /* Application data */
    APTR			 gd_Catalog;
    struct Process		*gd_Process;
    struct RDArgs		*gd_RDArgs;
    ULONG			 gd_Options[NUM_OPTS];
    struct IBox			 gd_Snapshot;			/* Window snapshot */
    struct IBox			 gd_FRMemory;			/* File requester snapshot */
    struct FileRequester	*gd_FR;
    struct FrameInfo		 gd_FrameInfo;
    Object			*gd_DisplayObject;		/* Data object */
    UBYTE			 gd_NameBuffer[NAMEBUFSIZE];
    UBYTE			 gd_ScreenNameBuffer[MAXPUBSCREENNAME+1];
    UBYTE			 gd_TempBuffer[512];
    STRPTR			 gd_ScreenName;
    ULONG			 gd_TotVert;
    ULONG			 gd_TotHoriz;
    struct Class		*gd_WindowClass;		/* Window class */
    struct MsgPort		*gd_IDCMPPort;			/* Intuition message port */
    struct MsgPort		*gd_AWPort;			/* AppWindow message port */
    ULONG			 gd_Signals;
    struct GadgetInfo		 gd_GInfo;
    struct dtTrigger		 gd_Trigger;
    struct TextAttr		 gd_TextAttr;
    struct TextFont		*gd_TextFont;
    struct Screen		*gd_Screen;
    struct DrawInfo		*gd_DrawInfo;
    APTR			 gd_VI;
    struct WindowObj		*gd_WindowObj;
    struct Window		*gd_Window;
    struct Menu			*gd_Menu;
    union printerIO		*gd_PIO;		/* Printer IO request */
    struct MsgPort		*gd_PrintPort;		/* Printer message port */
    struct Window		*gd_PrintWin;		/* Printing... window */
    struct TmpRas		 gd_TmpRas;
    PLANEPTR			 gd_TmpRasBuffer;
    BOOL			 gd_Going;
};

/*****************************************************************************/

#define	SysBase		gd->gd_SysBase
#define	DOSBase		gd->gd_DOSBase
#define	IconBase	gd->gd_IconBase
#define	IntuitionBase	gd->gd_IntuitionBase
#define	GfxBase		gd->gd_GfxBase
#define	UtilityBase	gd->gd_UtilityBase
#define	GadToolsBase	gd->gd_GadToolsBase
#define	LayersBase	gd->gd_LayersBase
#define	AslBase		gd->gd_AslBase
#define	DataTypesBase	gd->gd_DataTypesBase
#define	LocaleBase	gd->gd_LocaleBase

/*****************************************************************************/

#define	MMC_NOP			0
#define	MSG_NOTHING		0

#define	MMC_SAVEAS		502
#define	MMC_PRINT		503
#define	MMC_ABOUT		504
#define	MMC_QUIT		505

#define	MMC_MARK		520
#define	MMC_COPY		521

/*****************************************************************************/

#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))
#define MEMORY_N_FOLLOWING(ptr,n)  ((void *)( ((ULONG)ptr) + n ))

#define ASM		__asm
#define REG(x)		register __ ## x

#define	V(x)		((VOID *)(x))

#define	G(o)		((struct Gadget *)(o))

/*****************************************************************************/

#define	IDCMP_FLAGS	IDCMP_CLOSEWINDOW | IDCMP_NEWSIZE | IDCMP_REFRESHWINDOW | \
			IDCMP_GADGETUP | IDCMP_GADGETDOWN | IDCMP_MENUPICK | \
			IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_VANILLAKEY | \
			IDCMP_IDCMPUPDATE

/*****************************************************************************/

ULONG __stdargs DoMethodA (Object *obj, Msg message);
ULONG __stdargs DoMethod (Object *obj, unsigned long MethodID, ...);
ULONG __stdargs DoSuperMethodA (struct IClass *cl, Object *obj, Msg message);
ULONG __stdargs DoSuperMethod (struct IClass *cl, Object *obj, unsigned long MethodID, ...);
ULONG __stdargs CoerceMethodA (struct IClass *cl, Object *obj, Msg message);
ULONG __stdargs CoerceMethod (struct IClass *cl, Object *obj, unsigned long MethodID, ...);
ULONG __stdargs SetSuperAttrs (struct IClass *cl, Object *obj, unsigned long Tag1, ...);

void __stdargs NewList (struct List *);

/*****************************************************************************/

void kprintf (void *, ...);
void __stdargs sprintf (void *, ...);

/*****************************************************************************/

#include "texttable.h"
#include "clipview_iprotos.h"
