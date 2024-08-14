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

#define DTCLASS              "ico.datatype"

/*****************************************************************************/

struct ClassBase
{
    struct Library		 cb_Lib;
    UWORD			 cb_UsageCnt;
    struct Library		*cb_SysBase;
    struct Library		*cb_DOSBase;
    struct Library		*cb_IntuitionBase;
    struct Library		*cb_GfxBase;
    struct Library		*cb_UtilityBase;
    struct Library		*cb_IFFParseBase;
    struct Library		*cb_DataTypesBase;
    struct Library		*cb_SuperClassBase;
    BPTR			 cb_SegList;

    struct SignalSemaphore	 cb_Lock;		/* Access lock */
    Class			*cb_Class;
};

/*****************************************************************************/

#define	MAXCOLORS		256
#define	SQ(x)			((x)*(x))
#define	AVGC(i1,i2,c)		((ir->ir_GRegs[i1][c]>>1)+(ir->ir_GRegs[i2][c]>>1))
#define VANILLA_COPY		0xC0
#define NO_MASK			0xFF
#define	MAXSRCPLANES		24
#define	BPR(w)			((w) + 15 >> 4 << 1)
#define MaxPackedSize(rowSize)  ((rowSize) + (((rowSize)+127) >> 7 ))
#define	RowBytes(w)		((((w) + 15) >> 4) << 1)
#define	ChunkMoreBytes(cn)	(cn->cn_Size - cn->cn_Scan)
#define UGetByte()		(*source++)
#define UPutByte(c)		(*dest++ = (c))

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

void kprintf (void *, ...);

VOID ASM ExpandByteRun (REG (a0) BYTE *srcData, REG (a1) BYTE *dstData, REG (d0) WORD srcBytes);

/*****************************************************************************/

#include "class_iprotos.h"
