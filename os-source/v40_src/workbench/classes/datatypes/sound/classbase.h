/*****************************************************************************/

#include <dos/dos.h>
#include <dos/dostags.h>
#include <dos/dosextens.h>
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/devices.h>
#include <exec/semaphores.h>
#include <exec/execbase.h>
#include <devices/audio.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/cghooks.h>
#include <intuition/gadgetclass.h>
#include <utility/tagitem.h>
#include <graphics/gfx.h>
#include <graphics/text.h>
#include <graphics/gfxbase.h>
#include <libraries/iffparse.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/soundclass.h>
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

#define NTSC_CLOCK 3579545L
#define PAL_CLOCK  3546895L

/*****************************************************************************/

struct ClassBase
{
    struct Library		 cb_Lib;

    struct ExecBase		*cb_SysBase;
    struct Library		*cb_DOSBase;
    struct Library		*cb_IntuitionBase;
    struct GfxBase		*cb_GfxBase;
    struct Library		*cb_UtilityBase;
    struct Library		*cb_IFFParseBase;
    struct Library		*cb_DataTypesBase;
    struct Library		*cb_SuperClassBase;
    BPTR			 cb_SegList;

    struct SignalSemaphore	 cb_Lock;		/* Access lock */
    Class			*cb_Class;
    ULONG			 cb_TClock;
    UWORD			 cb_Period[12];		/* Octave periods */
};

/*****************************************************************************/

#define MEMORY_FOLLOWING(ptr)	((void *)((ptr)+1))
#define MEMORY_N_FOLLOWING(ptr,n)  ((void *)( ((ULONG)ptr) + n ))
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

void __stdargs BeginIO( struct IORequest *ioReq );

void kprintf (void *, ...);

/*****************************************************************************/

#include "class_iprotos.h"
