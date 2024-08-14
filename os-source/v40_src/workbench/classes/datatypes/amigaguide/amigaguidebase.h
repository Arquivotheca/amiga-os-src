/*****************************************************************************/

#include <dos/dos.h>
#include <dos/dostags.h>
#include <dos/dosextens.h>
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/semaphores.h>
#include <exec/execbase.h>
#include <intuition/classes.h>
#include <intuition/icclass.h>
#include <intuition/cghooks.h>
#include <intuition/classusr.h>
#include <intuition/intuition.h>
#include <intuition/imageclass.h>
#include <intuition/gadgetclass.h>
#include <graphics/gfx.h>
#include <graphics/text.h>
#include <graphics/layers.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
//#include <datatypes/textclass.h>
#include <libraries/amigaguide.h>
#include <libraries/locale.h>
#include <libraries/diskfont.h>
#include <libraries/iffparse.h>
#include <rexx/errors.h>
#include <rexx/rexxio.h>
#include <rexx/rxslib.h>
#include <rexx/storage.h>
#include <string.h>
#include <dos.h>

#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <clib/layers_protos.h>
#include <clib/dtclass_protos.h>
#include <clib/datatypes_protos.h>
#include <clib/rexxsyslib_protos.h>
#include <clib/locale_protos.h>
#include <clib/iffparse_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/diskfont_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/dtclass_pragmas.h>
#include <pragmas/datatypes_pragmas.h>
#include <pragmas/rexxsyslib_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

#include "amigaguide_token.h"

/*****************************************************************************/

#define	AMIGAGUIDEDTCLASS	"amigaguide.datatype"

/*****************************************************************************/

struct AGLib
{
    struct Library		 ag_Lib;
    BPTR			 ag_SegList;
    UWORD			 ag_UsageCnt;

    /* ROM libraries */
    struct Library		*ag_SysBase;
    struct Library		*ag_DOSBase;
    struct Library		*ag_IntuitionBase;
    struct Library		*ag_GfxBase;
    struct Library		*ag_UtilityBase;
    struct Library		*ag_LayersBase;

    /* Workbench libraries */
    struct Library		*ag_DiskfontBase;
    struct Library		*ag_DataTypesBase;
    struct Library		*ag_RexxSysBase;
    struct Library		*ag_LocaleBase;
    struct Library		*ag_IFFParseBase;

    /* Other data */
    APTR			 ag_Catalog;		/* Catalog */
    Class			*ag_Class;		/* The AmigaGuide class */
    Class			*ag_ModelClass;		/* Glue it all together */
    Class			*ag_DatabaseClass;	/* Class for handling databases */
    Class			*ag_NodeClass;		/* Class for handling nodes */
    Object			*ag_GlobalHelp;		/* Global help database */
    BPTR			 ag_GlobalPath;		/* Global path list */
    struct AmigaGuideToken	*ag_Token;		/* Global cross reference information */
    ULONG			 ag_Flags;
    struct SignalSemaphore	 ag_Lock;		/* Access lock */
    struct MinList		 ag_DatabaseList;	/* List of database's */
    struct MinList		 ag_HostList;		/* List of global dynamic node hosts */
};

/*****************************************************************************/

struct Path
{
    BPTR path_Next;
    BPTR path_Lock;
};

/*****************************************************************************/

#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))
#define MEMORY_N_FOLLOWING(ptr,n)  ((void *)( ((ULONG)ptr) + n ))

#define ASM           __asm
#define REG(x)	      register __ ## x

#define	G(o)	((struct Gadget *)(o))
#define	EG(o)	((struct ExtGadget *)(o))

/*****************************************************************************/

#define AGBase			((struct AGLib *)getreg(REG_A6))

#define SysBase			ag->ag_SysBase
#define DOSBase			ag->ag_DOSBase
#define UtilityBase		ag->ag_UtilityBase
#define	IntuitionBase		ag->ag_IntuitionBase
#define	GfxBase			ag->ag_GfxBase
#define	LayersBase		ag->ag_LayersBase
#define	DiskfontBase		ag->ag_DiskfontBase
#define	DataTypesBase		ag->ag_DataTypesBase
#define	RexxSysBase		ag->ag_RexxSysBase
#define	LocaleBase		ag->ag_LocaleBase
#define	IFFParseBase		ag->ag_IFFParseBase

/*****************************************************************************/

/* ARexx support functions in amiga.lib */
BOOL __stdargs CheckRexxMsg (struct Message *rexxmsg);
LONG __stdargs GetRexxVar (struct Message *rexxmsg, UBYTE *name, UBYTE **result);
LONG __stdargs SetRexxVar (struct Message *rexxmsg, UBYTE *name, UBYTE *value, long length);

/* boopsi functions in amiga.lib */
ULONG __stdargs DoMethodA (Object *obj, Msg message);
ULONG __stdargs DoMethod (Object *obj, unsigned long MethodID, ...);
ULONG __stdargs DoSuperMethodA (struct IClass *cl, Object *obj, Msg message);
ULONG __stdargs DoSuperMethod (struct IClass *cl, Object *obj, unsigned long MethodID, ...);
ULONG __stdargs CoerceMethodA (struct IClass *cl, Object *obj, Msg message);
ULONG __stdargs CoerceMethod (struct IClass *cl, Object *obj, unsigned long MethodID, ...);
ULONG __stdargs SetSuperAttrs (struct IClass *cl, Object *obj, unsigned long Tag1, ...);

VOID __stdargs NewList (struct List *list);

struct Node * ASM FindNameI (REG(a0) struct List *, REG(a1) STRPTR);

void kprintf (void *, ...);
void sprintf (void *, ...);

/*****************************************************************************/

#include "agdata.h"
#include "amigaguide_iprotos.h"

#include <localestr/amigaguide.h>
