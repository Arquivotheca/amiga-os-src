/* dtdesc.h
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <string.h>

#include <clib/macros.h>
#include <clib/asl_protos.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/icon_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/utility_protos.h>
#include <clib/wb_protos.h>

#include <pragmas/asl_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/wb_pragmas.h>

extern struct Library *AslBase;
extern struct Library *SysBase;
extern struct Library *DOSBase;
extern struct Library *IntuitionBase;
extern struct Library *GfxBase;
extern struct Library *GadToolsBase;
extern struct Library *IconBase;
extern struct Library *IFFParseBase;
extern struct Library *WorkbenchBase;
extern struct Library *UtilityBase;

/*****************************************************************************/

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
    ULONG			 dtn_UseCnt;
};

#include <datatypes/datatypes.h>

#ifdef	DTSIZE
#undef	DTSIZE
#endif
#define	DTSIZE		(sizeof (struct DataType))

/**************************************/

#define	FNBUF_SIZE	64

struct FileNode
{
    struct Node			 fn_Node;			/* Embedded node */
    BPTR			 fn_Lock;			/* Lock on the file */
    BPTR			 fn_FH;				/* File handle */
    UBYTE			 fn_Name[32];			/* Name of file */
    STRPTR			 fn_Extens;
    UBYTE			 fn_Buffer[FNBUF_SIZE + 2];	/* File buffer */
    LONG			 fn_Len;			/* Length read into buffer */
    UWORD			 fn_Flags;
};

#define	FNSIZE	sizeof(struct FileNode)


/**************************************/

#define	AIG_CONTEXT		0
#define	AIG_LASTGADGET		1
#define	AIG_DATATYPE		2
#define	AIG_BASENAME		3
#define	AIG_PATTERN		4
#define	AIG_FUNCTION		5
#define	AIG_CASE		6
#define	AIG_PRIORITY		7
#define	AIG_TYPE		8
#define	AIG_FILELIST		9
#define	AIG_REMOVE		10
#define	AIG_REMOVEALL		11
#define	AIG_NUM			12

#define	FR_SAVE		0
#define	FR_OPEN		1
#define	FR_MULTIOPEN	2

struct AppInfo
{
    struct FileInfoBlock	 ai_FIB;
    struct TextAttr		 ai_TextAttr;
    struct TextFont		*ai_Font;
    struct Screen		*ai_Screen;
    struct VisualInfo		*ai_VI;

    struct FileRequester	*ai_FR;
    struct IBox			 ai_FRMemory;

    struct List			 ai_Files;
    struct FileNode		*ai_CurFile;
    WORD			 ai_CurNum;
    WORD			 ai_NumFiles;

    ULONG			 ai_Signal;

    struct MsgPort		*ai_AppWinPort;
    ULONG			 ai_SigA;
    struct AppMessage		*ai_AMsg;

    struct MsgPort		*ai_IDCMPPort;
    struct IntuiMessage		*ai_IMsg;
    struct MenuItem		*ai_MenuItem;

    struct Window		*ai_Window;
    struct Menu			*ai_Menu;
    struct AppWindow		*ai_AppWin;
    ULONG			 ai_WinSig;
    ULONG			 ai_WinFlags;
    struct Gadget		*ai_WinGad[AIG_NUM];
    WORD			 ai_SBuffer[FNBUF_SIZE + 2];
    WORD			 ai_DBuffer[FNBUF_SIZE + 2];
    WORD			 ai_BufSize;

    LONG			 ai_Flags;
    BOOL			 ai_Done;
    UBYTE			 ai_Buffer[512];
    UBYTE			 ai_Dir[512];
    UBYTE			 ai_Name[32];

    struct IBox			 ai_ViewBox;
    ULONG			 ai_GroupID;
    ULONG			 ai_ID;
};

#define	AIF_TEXT	0x00010000L
#define	AIF_HEX		0x00020000L
#define	AIF_ABORTED	0x00040000L
#define	AIF_DOWN	0x00080000L
#define	AIF_SET		0x00100000L
#define	AIF_CHANGED	0x00200000L

#define	AISIZE	sizeof(struct AppInfo)


/**************************************/

#ifndef MEMORY_FOLLOWING
/* Usually allocate one large block of memory for a group of items and then
 * divy out to the appropriate pointers.  Use with caution---must be
 * consistent with field types! */
#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))
#endif

/**************************************/

#define DISPLAY  1
#define REMOVE   2

#define TMERROR_MEMORY     1
#define TMERROR_INTUITION  2
#define TMERROR_GADTOOLS   3
#define TMERROR_SCREEN     4
#define TMERROR_VISUALINFO 5

#define TMWF_OPEN 0x00000001

#define	VB_LEFT		226
#define	VB_TOP		78
#define	VB_WIDTH	311
#define	VB_HEIGHT	43

#define	IDCMP_FLAGS	IDCMP_REFRESHWINDOW | IDCMP_CLOSEWINDOW | IDCMP_MENUPICK | \
			STRINGIDCMP | LISTVIEWIDCMP | IDCMP_VANILLAKEY | IDCMP_RAWKEY | \
			IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE

#define	V(x)		((VOID *)(x))

#define	MMC_NEW		1
#define	MMC_OPEN	2
#define	MMC_SAVE	3
#define	MMC_SAVEAS	4
#define	MMC_QUIT	5
#define	MMC_REMOVE	6
#define	MMC_REMOVEALL	7
#define	MMC_CASE	8
#define	MMC_PRIORITY	9
#define	MMC_EXTENSION	10
#define	MMC_DATATYPE	11
#define	MMC_BASENAME	12
#define	MMC_PATTERN	13
#define	MMC_FUNCTION	14
#define	MMC_SAMPLE	15
#define	MMC_GRAPHIC	16
#define	MMC_TEXT	17
#define	MMC_HEX		18
#define	MMC_LOAD	19
#define	MMC_ADDDATATYPE	20
#define	MMC_LOADFUNC	21

#define	MMC_SYSTEM_G		GID_SYSTEM
#define	MMC_TEXT_G		GID_TEXT
#define	MMC_DOCUMENT_G		GID_DOCUMENT
#define	MMC_SOUND_G		GID_SOUND
#define	MMC_INSTRUMENT_G	GID_INSTRUMENT
#define	MMC_MUSIC_G		GID_MUSIC
#define	MMC_PICTURE_G		GID_PICTURE
#define	MMC_ANIMATION_G		GID_ANIMATION
#define	MMC_MOVIE_G		GID_MOVIE

void kprintf (void *, ...);
void sprintf (void *, ...);

#include "dtdesc_iprotos.h"
