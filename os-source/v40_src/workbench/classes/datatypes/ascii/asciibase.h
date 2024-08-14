/*****************************************************************************/

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <exec/semaphores.h>
#include <exec/execbase.h>
#include <intuition/intuition.h>
#include <intuition/cghooks.h>
#include <intuition/gadgetclass.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/textclass.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <graphics/gfx.h>
#include <graphics/text.h>
#include <string.h>
#include <ctype.h>
#include <dos.h>

#include <clib/macros.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>
#include <clib/utility_protos.h>
#include <clib/dtclass_protos.h>
#include <clib/datatypes_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/dtclass_pragmas.h>
#include <pragmas/datatypes_pragmas.h>

/*****************************************************************************/

#define	ASCIIDTCLASS	"ascii.datatype"

/*****************************************************************************/

struct ASCIILib
{
    struct Library		 ascii_Lib;
    UWORD			 ascii_Pad;
    APTR			 ascii_SysBase;
    APTR			 ascii_DOSBase;
    APTR			 ascii_IntuitionBase;
    APTR			 ascii_GfxBase;
    APTR			 ascii_LayersBase;
    APTR			 ascii_UtilityBase;
    APTR			 ascii_DataTypesBase;
    APTR			 ascii_SuperClassBase;
    BPTR			 ascii_SegList;

    struct SignalSemaphore	 ascii_Lock;		/* Access lock */
    Class			*ascii_Class;
};

/*****************************************************************************/

#ifndef MEMORY_FOLLOWING
/* Usually allocate one large block of memory for a group of items and then
 * divy out to the appropriate pointers.  Use with caution---must be
 * consistent with field types! */
#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))
#endif

#define ASM           __asm
#define REG(x)	      register __ ## x

/*****************************************************************************/

#define ASCIIBase		((struct ASCIILib *)getreg(REG_A6))

#define SysBase			ascii->ascii_SysBase
#define DOSBase			ascii->ascii_DOSBase
#define UtilityBase		ascii->ascii_UtilityBase
#define	IntuitionBase		ascii->ascii_IntuitionBase
#define	GfxBase			ascii->ascii_GfxBase
#define	LayersBase		ascii->ascii_LayersBase
#define	DataTypesBase		ascii->ascii_DataTypesBase

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

#include "ascii_iprotos.h"
