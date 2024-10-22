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
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/cghooks.h>
#include <intuition/gadgetclass.h>
#include <graphics/gfx.h>
#include <libraries/iffparse.h>
#include <libraries/locale.h>
#include <dos.h>

#include <clib/macros.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/locale_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/locale_pragmas.h>

/*****************************************************************************/

#define	MASTER_LOCK	0
#define	DATATYPE_LIST	1
#define	OBJECT_LIST	2
#define	HOTLINK_LIST	3
#define	WINDOW_LIST	4
#define	MAX_LOCKS	10

/*****************************************************************************/

struct DataTypesLib
{
    struct Library		 dtl_Lib;
    UWORD			 dtl_UsageCnt;

    APTR			 dtl_SysBase;
    APTR			 dtl_DOSBase;
    APTR			 dtl_IntuitionBase;
    APTR			 dtl_GfxBase;
    APTR			 dtl_LayersBase;
    APTR			 dtl_UtilityBase;
    APTR			 dtl_IFFParseBase;
    APTR			 dtl_RexxSysBase;
    APTR			 dtl_LocaleBase;

    APTR			 dtl_Catalog;
    BPTR			 dtl_SegList;

    struct SignalSemaphore	 dtl_Lock[MAX_LOCKS];	/* Access lock */
    struct Token		*dtl_Token;		/* All the DataType lists */
    Class			*dtl_DTClass;		/* DataTypes class */
    Class			*dtl_DTNClass;		/* DataTypesNode class */
    struct Process		*dtl_NotifyProc;
    struct Process		*dtl_TickProc;
    ULONG			 dtl_Flags;
};

#define	DTLIBRARY_NAME	"datatypes.library"

/*****************************************************************************/

struct DTObjectPrivate
{
    struct Node			 dtm_Node;
    struct SignalSemaphore	 dtm_Lock;
    ULONG			 dtm_UseCnt;
    struct List		 	 dtm_ObjectList;
    struct MinList		 dtm_NotifyList;
    struct MinList		 dtm_TickList;
};

/*****************************************************************************/

struct WindowList
{
    struct Node		 wl_Node;
    UBYTE		 wl_Address[16];	/* Address of window is the name */
    struct Window	*wl_Window;		/* Pointer to the window itself */
    struct Gadget	*wl_Gadget;		/* The gadget that covers the whole window... */
    struct DrawInfo	*wl_DrawInfo;		/* Screen draw info */
    struct List		 wl_List;		/* List of objects in the window */
    LONG		 wl_NumNodes;		/* Number of nodes in the list */
};

/*****************************************************************************/

struct WindowObject
{
    struct Node		 wo_Node;
    UBYTE		 wo_Address[16];	/* Address of the object is the name */
    Object		*wo_Object;		/* Pointer to the object */
    struct GadgetInfo	 wo_ObjectInfo;		/* Embedded GadgetInfo structure */
};

/*****************************************************************************/

#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))
#define MEMORY_N_FOLLOWING(ptr,n)  ((void *)( ((ULONG)ptr) + n ))

#define ASM           __asm
#define REG(x)	      register __ ## x

#define SysBase		dtl->dtl_SysBase
#define DOSBase		dtl->dtl_DOSBase
#define	IntuitionBase	dtl->dtl_IntuitionBase
#define	GfxBase		dtl->dtl_GfxBase
#define	LayersBase	dtl->dtl_LayersBase
#define UtilityBase	dtl->dtl_UtilityBase
#define	LocaleBase	dtl->dtl_LocaleBase
#define IFFParseBase	dtl->dtl_IFFParseBase
#define RexxSysBase	dtl->dtl_RexxSysBase

/*****************************************************************************/

ULONG __stdargs DoMethodA (Object *obj, Msg message);
ULONG __stdargs DoMethod (Object *obj, unsigned long MethodID, ...);
ULONG __stdargs DoSuperMethodA (struct IClass *cl, Object *obj, Msg message);
ULONG __stdargs DoSuperMethod (struct IClass *cl, Object *obj, unsigned long MethodID, ...);
ULONG __stdargs CoerceMethodA (struct IClass *cl, Object *obj, Msg message);
ULONG __stdargs CoerceMethod (struct IClass *cl, Object *obj, unsigned long MethodID, ...);
ULONG __stdargs SetSuperAttrs (struct IClass *cl, Object *obj, unsigned long Tag1, ...);

kprintf (STRPTR,...);
sprintf (STRPTR,...);
void NewList (struct List *);

struct Node * ASM FindNameI(REG (a0) struct List *, REG (a1) STRPTR);
int ASM stricmpn (REG(a0) char *, REG(a1) char *, REG(d2) int);

VOID ASM DrawBox (REG (a6) struct DataTypesLib * dtl, REG(a2) struct RastPort *rp, REG(d2) WORD Xoff, REG(d3) WORD Yoff, REG(d4) WORD lastX, REG(d5) WORD lastY);
VOID ASM AnimateDragSelectBox (REG (a6) struct DataTypesLib * dtl, REG(a2) struct RastPort *rp, REG(d2) WORD Xoff, REG(d3) WORD Yoff, REG(d4) WORD lastX, REG(d5) WORD lastY);
VOID ASM CheckSortRect(REG(a0) struct Rectangle *);

/*****************************************************************************/

#ifndef	DATATYPE
#define	DATATYPE
struct DataType
{
    struct Node	 		 dtn_Node1;		/* Reserved for system use */
    struct Node			 dtn_Node2;		/* Reserved for system use */
    struct DataTypeHeader	*dtn_Header;		/* Pointer to the DataTypeHeader */
    struct List			 dtn_ToolList;		/* List of tool nodes */
    STRPTR			 dtn_FunctionName;	/* Name of comparision routine */
    struct TagItem		*dtn_AttrList;		/* Object creation tags */
    ULONG			 dtn_Length;		/* Length of the memory block */

    ULONG			 dtn_SysFlags;
    ULONG			 dtn_TLength;
    STRPTR			 dtn_Token;
    APTR			 dtn_Executable;
    ULONG			 dtn_ExecSize;
    BPTR			 dtn_SegList;
    LONG (* ASM dtn_Function)(REG (a0) struct DTHookContext *dthc);
    LONG			 dtn_UseCnt;		/* Number of obtainers */
};
#endif

/*****************************************************************************/

#include "datatypes_lib.h"
#include "datatypesclass.h"
#include "datatypes_iprotos.h"
#include <internal/datatypes_token.h>

#define	DATATYPES	TRUE
#include <localestr/libs.h>
