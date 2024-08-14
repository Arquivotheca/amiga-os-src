/*****************************************************************************/

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/semaphores.h>
#include <exec/execbase.h>
#include <intuition/intuition.h>
#include <intuition/cghooks.h>
#include <intuition/gadgetclass.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <graphics/gfx.h>
#include <graphics/text.h>
#include <graphics/rpattr.h>
#include <libraries/iffparse.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/textclass.h>
#include <string.h>
#include <dos.h>

#include <clib/macros.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <clib/dtclass_protos.h>
#include <clib/datatypes_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/layers_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/dtclass_pragmas.h>
#include <pragmas/datatypes_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/layers_pragmas.h>

/*****************************************************************************/

struct TextLib
{
    struct Library		 txl_Lib;
    UWORD			 txl_Pad;
    struct Library		*txl_SysBase;
    struct Library		*txl_DOSBase;
    struct Library		*txl_IntuitionBase;
    struct Library		*txl_GfxBase;
    struct Library		*txl_UtilityBase;
    struct Library		*txl_LayersBase;
    struct Library		*txl_IFFParseBase;
    struct Library		*txl_DataTypesBase;
    BPTR			 txl_SegList;

    struct SignalSemaphore	 txl_Lock;		/* Access lock */
    Class			*txl_Class;
};

/*****************************************************************************/

#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))
#define ASM		__asm
#define REG(x)		register __ ## x
#define	G(x)		((struct Gadget *)(x))

#define	EG(o)		((struct ExtGadget *)(o))
#ifndef	GMORE_SCROLLRASTER
#define	GMORE_SCROLLRASTER	4
#endif

/*****************************************************************************/

#define TextBase		((struct TextLib *)getreg(REG_A6))

#define SysBase			txl->txl_SysBase
#define DOSBase			txl->txl_DOSBase
#define	IntuitionBase		txl->txl_IntuitionBase
#define	GfxBase			txl->txl_GfxBase
#define UtilityBase		txl->txl_UtilityBase
#define	LayersBase		txl->txl_LayersBase
#define	IFFParseBase		txl->txl_IFFParseBase
#define	DataTypesBase		txl->txl_DataTypesBase

/*****************************************************************************/

VOID ASM CheckSortRect (REG (a0) struct Rectangle *);

ULONG __stdargs DoMethodA (Object *obj, Msg message);
ULONG __stdargs DoMethod (Object *obj, unsigned long MethodID, ...);
ULONG __stdargs DoSuperMethodA (struct IClass *cl, Object *obj, Msg message);
ULONG __stdargs DoSuperMethod (struct IClass *cl, Object *obj, unsigned long MethodID, ...);
ULONG __stdargs CoerceMethodA (struct IClass *cl, Object *obj, Msg message);
ULONG __stdargs CoerceMethod (struct IClass *cl, Object *obj, unsigned long MethodID, ...);
ULONG __stdargs SetSuperAttrs (struct IClass *cl, Object *obj, unsigned long Tag1, ...);

void NewList (struct List *);
void kprintf (void *, ...);
void sprintf (void *, ...);

/*****************************************************************************/

#include "classdata.h"
#include "text_iprotos.h"
