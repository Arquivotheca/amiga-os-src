/* AddDataTypes.h
 *
 */

/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <workbench/icon.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <libraries/iffparse.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dosasl.h>
#include <string.h>
#include <stdlib.h>

/* prototypes */
#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/icon_protos.h>
#include <clib/utility_protos.h>
#include <clib/locale_protos.h>
#include <clib/intuition_protos.h>
#include <clib/iffparse_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

#include "texttable.h"
#include <internal/datatypes_token.h>

/*****************************************************************************/

#define	DEFAULT_DIR	"DEVS:DataTypes"
#define	DEFAULT_WILD	"DEVS:DataTypes/#?"

#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))
#define MEMORY_N_FOLLOWING(ptr,n)  ((void *)( ((ULONG)ptr) + n ))

#define ASM           __asm
#define REG(x)	      register __ ## x

/*****************************************************************************/

struct MyFileHandle
{
    UBYTE	*data;
    LONG	 size;
    LONG	 ofs;
};

/*****************************************************************************/

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
    LONG			 dtn_UseCnt;
};

#include <datatypes/datatypes.h>

#undef	DTSIZE
#define	DTSIZE	(sizeof(struct DataType))

/*****************************************************************************/

struct GlobalData
{
    struct Library	*gd_SysBase;
    struct Library	*gd_DOSBase;
    struct Library	*gd_IntuitionBase;
    struct Library	*gd_UtilityBase;
    struct Library	*gd_IFFParseBase;
    struct LocaleInfo	 gd_LocaleInfo;
    struct Process	*gd_Process;
    struct Token	*gd_Token;
    UBYTE		 gd_InfoPattern[48];
    ULONG		 gd_Flags;
};

#define	GDF_QUIET	(1L<<0)

/*****************************************************************************/

void __stdargs NewList (struct List *);
struct Node * ASM FindNameI (REG(a0) struct List *, REG(a1) STRPTR);
void kprintf (void *, ...);

#include "adddatatypes_iprotos.h"

/*****************************************************************************/

#define SysBase		gd->gd_SysBase
#define DOSBase		gd->gd_DOSBase
#define IntuitionBase	gd->gd_IntuitionBase
#define UtilityBase	gd->gd_UtilityBase
#define LocaleBase	gd->gd_LocaleInfo.li_LocaleBase
#define	IFFParseBase	gd->gd_IFFParseBase

