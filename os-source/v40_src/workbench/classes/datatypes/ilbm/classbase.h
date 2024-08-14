/*****************************************************************************/

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/semaphores.h>
#include <exec/execbase.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/cghooks.h>
#include <intuition/gadgetclass.h>
#include <graphics/gfx.h>
#include <graphics/text.h>
#include <libraries/iffparse.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>
#include <string.h>
#include <dos.h>

#include <clib/macros.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <clib/datatypes_protos.h>
#include <clib/dtclass_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/datatypes_pragmas.h>
#include <pragmas/dtclass_pragmas.h>

/*****************************************************************************/

struct ClassBase
{
    struct Library		 cb_Lib;
    UWORD			 cb_Pad;
    struct Library		*cb_SysBase;
    struct Library		*cb_DOSBase;
    struct Library		*cb_IntuitionBase;
    struct Library		*cb_GfxBase;
    struct Library		*cb_UtilityBase;
    struct Library		*cb_IFFParseBase;
    struct Library		*cb_DataTypesBase;
    struct Library		*cb_SuperClassBase;
    BPTR			 cb_SegList;

    Class			*cb_Class;
    struct SignalSemaphore	 cb_Lock;		/* Access lock */
};

/*****************************************************************************/

#define	MAXSRCPLANES 24
#define	RowBytes(w)  ((((w) + 15) >> 4) << 1)

/*****************************************************************************/

#define ASM			__asm
#define REG(x)			register __ ## x

/*****************************************************************************/

#define SysBase			cb->cb_SysBase
#define DOSBase			cb->cb_DOSBase
#define UtilityBase		cb->cb_UtilityBase
#define	IntuitionBase		cb->cb_IntuitionBase
#define	IFFParseBase		cb->cb_IFFParseBase
#define	GfxBase			cb->cb_GfxBase
#define	DataTypesBase		cb->cb_DataTypesBase

/*****************************************************************************/

ULONG __stdargs DoMethodA (Object *obj, Msg message);
ULONG __stdargs DoMethod (Object *obj, unsigned long MethodID, ...);
ULONG __stdargs DoSuperMethodA (struct IClass *cl, Object *obj, Msg message);
ULONG __stdargs DoSuperMethod (struct IClass *cl, Object *obj, unsigned long MethodID, ...);
ULONG __stdargs CoerceMethodA (struct IClass *cl, Object *obj, Msg message);
ULONG __stdargs CoerceMethod (struct IClass *cl, Object *obj, unsigned long MethodID, ...);
ULONG __stdargs SetSuperAttrs (struct IClass *cl, Object *obj, unsigned long Tag1, ...);

void kprintf (void *, ...);


/*****************************************************************************/

#include "class_iprotos.h"
