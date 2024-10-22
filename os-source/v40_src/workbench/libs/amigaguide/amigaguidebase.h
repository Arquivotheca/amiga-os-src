/* amigaguidebase.h
 * Written by David N. Junod
 *
 *
 */

#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/semaphores.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <dos/dosextens.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/cghooks.h>
#include <intuition/icclass.h>
#include <intuition/imageclass.h>
#include <intuition/gadgetclass.h>
#include <graphics/gfx.h>
#include <libraries/asl.h>
#include <libraries/locale.h>
#include <libraries/iffparse.h>
#include <libraries/diskfont.h>
#include <libraries/gadtools.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <utility/name.h>
#include <utility/hooks.h>
#include <utility/tagitem.h>
#include <rexx/errors.h>
#include <rexx/rexxio.h>
#include <rexx/rxslib.h>
#include <rexx/storage.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <string.h>
#include <dos.h>

#include <clib/macros.h>
#include <clib/asl_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/locale_protos.h>
#include <clib/rexxsyslib_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/datatypes_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/wb_protos.h>

#include <pragmas/asl_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/wb_pragmas.h>
#include <pragmas/rexxsyslib_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/datatypes_pragmas.h>
#include <pragmas/diskfont_pragmas.h>

#include <clib/amigaguideclass_protos.h>
#include <pragmas/amigaguideclass_pragmas.h>

/*****************************************************************************/

/* global variables required for library */
struct AmigaGuideLib
{
    struct Library		 ag_Lib;
    ULONG			 ag_SegList;		/* Seglist */
    UWORD			 ag_UseCnt;		/* Useage count */

    /* ROM libraries */
    struct Library		*ag_SysBase;
    struct Library		*ag_DOSBase;
    struct Library		*ag_UtilityBase;
    struct Library		*ag_GfxBase;
    struct Library		*ag_IntuitionBase;
    struct Library		*ag_GadToolsBase;
    struct Library		*ag_WorkbenchBase;

    /* disk libraries */
    struct Library		*ag_AslBase;
    struct Library		*ag_RexxSysBase;
    struct Library		*ag_LocaleBase;
    struct Library		*ag_DataTypesBase;
    struct Library		*ag_AmigaGuideClassBase;	/* amigaguide.class library base */

    /* other stuff */
    struct AmigaGuideToken	*ag_Token;			/* Global cross reference information */
    APTR			 ag_Catalog;
    struct SignalSemaphore	 ag_Lock;			/* Exclusive access */
    Class			*ag_WindowClass;		/* Window class */
    VOID			*ag_MemoryPool;			/* Memory pool */
    ULONG			 ag_Flags;			/* Useage flags */
    UBYTE			 ag_Work[512];			/* Another temporary work area */
};

/*****************************************************************************/

/* Usually allocate one large block of memory for a group of items and then
 * divy out to the appropriate pointers.  Use with caution---must be
 * consistent with field types! */
#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))
#define MEMORY_N_FOLLOWING(ptr,n)  ((void *)( ((ULONG)ptr) + n ))

#define ASM		__asm
#define REG(x)		register __ ## x

#define	G(x)		((struct Gadget *)(x))
#define V(x)		((void *)x)

/* remember ',' is the decimal point in some countries */
#define isspace(c)	((c==' ')||(c=='\t')||(c=='\n')||(c==','))
#define	EVEN(a)		((a%2)?0:1)

/*****************************************************************************/

#define SysBase			ag->ag_SysBase
#define DOSBase			ag->ag_DOSBase
#define	UtilityBase		ag->ag_UtilityBase
#define	GfxBase			ag->ag_GfxBase
#define	IntuitionBase		ag->ag_IntuitionBase
#define	GadToolsBase		ag->ag_GadToolsBase
#define	WorkbenchBase		ag->ag_WorkbenchBase
#define	AslBase			ag->ag_AslBase
#define	RexxSysBase		ag->ag_RexxSysBase
#define	LocaleBase		ag->ag_LocaleBase
#define	DataTypesBase		ag->ag_DataTypesBase
#define	AmigaGuideClassBase	ag->ag_AmigaGuideClassBase

/*****************************************************************************/

/* Intuition support functions in amiga.lib */
ULONG __stdargs DoMethodA (Object *obj, Msg message);
ULONG __stdargs DoMethod (Object *obj, unsigned long MethodID, ...);
ULONG __stdargs DoSuperMethodA (struct IClass *cl, Object *obj, Msg message);
ULONG __stdargs DoSuperMethod (struct IClass *cl, Object *obj, unsigned long MethodID, ...);
ULONG __stdargs CoerceMethodA (struct IClass *cl, Object *obj, Msg message);
ULONG __stdargs CoerceMethod (struct IClass *cl, Object *obj, unsigned long MethodID, ...);
ULONG __stdargs SetSuperAttrs (struct IClass *cl, Object *obj, unsigned long Tag1, ...);

/* ARexx support functions in amiga.lib */
BOOL __stdargs CheckRexxMsg (struct Message *rexxmsg);
LONG __stdargs GetRexxVar (struct Message *rexxmsg, UBYTE *name, UBYTE **result);
LONG __stdargs SetRexxVar (struct Message *rexxmsg, UBYTE *name, UBYTE *value, long length);

VOID __stdargs NewList (struct List *list);

/* debug.lib */
void kprintf (STRPTR,...);
void sprintf (STRPTR,...);

/* findnamei.asm */
struct Node * ASM FindNameI (REG(a0) struct List *, REG(a1) STRPTR);

/*****************************************************************************/

#include "amigaguide_internal.h"
#include "amigaguide_iprotos.h"
#include "V39:src/workbench/classes/datatypes/amigaguide/texttable.h"
#include "windowclass.h"

#include <internal/amigaguide_token.h>
