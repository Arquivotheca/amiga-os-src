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
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>
#include <libraries/iffparse.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/cghooks.h>
#include <intuition/gadgetclass.h>
#include <graphics/gfxmacros.h>
#include <hardware/blit.h>
#include <graphics/gfx.h>
#include <graphics/text.h>
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

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/dtclass_pragmas.h>
#include <pragmas/datatypes_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

/*****************************************************************************/

struct ClassBase
{
    struct Library		 cb_Lib;
    UWORD			 cb_UsageCnt;
    BPTR			 cb_SegList;
    struct Library		*cb_SysBase;
    struct Library		*cb_DOSBase;
    struct Library		*cb_IntuitionBase;
    struct Library		*cb_GfxBase;
    struct Library		*cb_UtilityBase;
    struct Library		*cb_DataTypesBase;
    struct Library		*cb_IFFParseBase;

    struct SignalSemaphore	 cb_Lock;		/* Access lock */
    Class			*cb_Class;
};

/*****************************************************************************/

#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))
#define MEMORY_N_FOLLOWING(ptr,n)  ((void *)( ((ULONG)ptr) + n ))

#define ASM           __asm
#define REG(x)	      register __ ## x

/*****************************************************************************/

#define SysBase			cb->cb_SysBase
#define DOSBase			cb->cb_DOSBase
#define UtilityBase		cb->cb_UtilityBase
#define	IntuitionBase		cb->cb_IntuitionBase
#define	GfxBase			cb->cb_GfxBase
#define	DataTypesBase		cb->cb_DataTypesBase
#define	IFFParseBase		cb->cb_IFFParseBase

/*****************************************************************************/

ULONG __stdargs DoMethodA (Object *obj, Msg message);
ULONG __stdargs DoMethod (Object *obj, unsigned long MethodID, ...);
ULONG __stdargs DoSuperMethodA (struct IClass *cl, Object *obj, Msg message);
ULONG __stdargs DoSuperMethod (struct IClass *cl, Object *obj, unsigned long MethodID, ...);
ULONG __stdargs CoerceMethodA (struct IClass *cl, Object *obj, Msg message);
ULONG __stdargs CoerceMethod (struct IClass *cl, Object *obj, unsigned long MethodID, ...);
ULONG __stdargs SetSuperAttrs (struct IClass *cl, Object *obj, unsigned long Tag1, ...);

void kprintf (void *, ...);

/* remap.asm */
void ASM XLateBitMap (REG (a0) struct BitMap *sbm, REG (a1) struct BitMap *dbm, REG (a2) char *table1, REG (a3) char *table2, REG (d0) ULONG width);

/* histogram.asm */
VOID ASM TallyBitMap (REG (a0) struct BitMap *sbm, REG (a1) ULONG *histogram, REG (d0) ULONG width);

/*****************************************************************************/

#include "classdata.h"
#include "class_iprotos.h"
