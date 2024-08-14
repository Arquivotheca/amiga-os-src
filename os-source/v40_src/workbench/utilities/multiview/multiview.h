#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosasl.h>
#include <dos/rdargs.h>
#include <dos/dosextens.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/icclass.h>
#include <intuition/imageclass.h>
#include <intuition/gadgetclass.h>
#include <intuition/pointerclass.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/cghooks.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/layers.h>
#include <libraries/amigaguide.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <libraries/diskfont.h>
#include <datatypes/datatypes.h>
#include <datatypes/pictureclass.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>
#include <datatypes/textclass.h>
#include <libraries/locale.h>
#include <workbench/icon.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <rexx/storage.h>
#include <rexx/rxslib.h>
#include <string.h>

#include <clib/macros.h>
#include <clib/amigaguide_protos.h>
#include <clib/asl_protos.h>
#include <clib/datatypes_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/icon_protos.h>
#include <clib/intuition_protos.h>
#include <clib/layers_protos.h>
#include <clib/locale_protos.h>
#include <clib/rexxsyslib_protos.h>
#include <clib/utility_protos.h>
#include <clib/wb_protos.h>

#include <pragmas/amigaguide_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/datatypes_pragmas.h>
#include <pragmas/diskfont_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/rexxsyslib_pragmas.h>
#include <pragmas/wb_pragmas.h>

#include "arexx.h"
#include "windowclass.h"
#include "cmdprocessor.h"

/*****************************************************************************/

struct EdMenu
{
    UBYTE	 em_Type;		/* NM_XXX from gadtools.h */
    LONG	 em_Label;
    ULONG	 em_Command;
    UWORD	 em_ItemFlags;
};

/*****************************************************************************/

#define	MAXNAME	512

/*****************************************************************************/

struct EdMenuNode
{
    struct Node	 em_Node;		/* Node in the menu list */
    UBYTE	 em_Name[32];		/* Name of the node */
    UBYTE	 em_Command[2];		/* Command string */
};

/*****************************************************************************/

extern struct EdMenu newmenu[];

/*****************************************************************************/

#define	MSG_NOTHING		0

/*****************************************************************************/

#define	IDCMP_FLAGS	IDCMP_CLOSEWINDOW | IDCMP_NEWSIZE | IDCMP_REFRESHWINDOW | \
			IDCMP_GADGETUP | IDCMP_GADGETDOWN | IDCMP_MENUPICK | \
			IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | \
			IDCMP_VANILLAKEY | IDCMP_RAWKEY | \
			IDCMP_CHANGEWINDOW | IDCMP_NEWSIZE | \
			IDCMP_IDCMPUPDATE | IDCMP_INTUITICKS

#define	ALTED		(IEQUALIFIER_RALT | IEQUALIFIER_LALT)
#define	SHIFTED		(IEQUALIFIER_RSHIFT | IEQUALIFIER_LSHIFT)

/*****************************************************************************/

#define	TEMPLATE	"FILE,CLIPBOARD/S,CLIPUNIT/K/N,SCREEN/S,PUBSCREEN/K,REQUESTER/S,BOOKMARK/S,FONTNAME/K,FONTSIZE/K/N,BACKDROP/S,WINDOW/S,PORTNAME/K"
#define	OPT_FILE	0
#define	OPT_CLIPBOARD	1
#define	OPT_CLIPUNIT	2
#define	OPT_SCREEN	3
#define	OPT_PUBSCREEN	4
#define	OPT_REQUESTER	5
#define	OPT_BOOKMARK	6
#define	OPT_FONTNAME	7
#define	OPT_FONTSIZE	8
#define	OPT_BACKDROP	9
#define	OPT_WINDOW	10
#define	OPT_PORTNAME	11
#define	NUM_OPTS	12

/*****************************************************************************/

enum NavigateCmds
{
    AC_TOP = 1,
    AC_BOTTOM,
    AC_FAR_LEFT,
    AC_FAR_RIGHT,

    AC_PAGE_UP,
    AC_PAGE_DOWN,
    AC_PAGE_LEFT,
    AC_PAGE_RIGHT,

    AC_UNIT_UP,
    AC_UNIT_DOWN,
    AC_UNIT_LEFT,
    AC_UNIT_RIGHT,

    AC_FIELD_NEXT,
    AC_FIELD_PREV,

    AC_NODE_NEXT,
    AC_NODE_PREV,

    AC_ACTIVATE_FIELD,

    AC_CONTENTS,
    AC_INDEX,
    AC_HELP,
    AC_RETRACE,
};

/*****************************************************************************/

#define	NAMEBUFSIZE	512

/*****************************************************************************/

struct Bookmark
{
    LONG	bm_TopVert;
    LONG	bm_TopHoriz;
    LONG	bm_NameLen;
    STRPTR	bm_Name;
    LONG	bm_NodeLen;
    STRPTR	bm_Node;
};

/*****************************************************************************/

struct WindowData
{
    struct Node			 wd_Node;
    struct Window		*wd_Window;			/* Window */
    struct IBox			 wd_Memory;			/* Position memory */
    APTR			 wd_UserData;			/* Window specific data */
};

/*****************************************************************************/

struct Prefs
{
    WORD			 p_SWidth, p_SHeight;		/* Screen size */
    struct IBox			 p_Window;			/* Window normal position */
    struct IBox			 p_Zoom;			/* Window zoom position */
    struct IBox			 p_FileReq;			/* File requester position */
};

/*****************************************************************************/

#define REXX_NO_ERROR		0
#define REXX_PROGRAM_NOT_FOUND	ERR10_001	/* program not found */
#define REXX_NO_MEMORY		ERR10_003	/* no memory available */
#define REXX_NO_LIBRARY		ERR10_014	/* required library not found */
#define REXX_WRONG_NUMBER_ARGS	ERR10_017	/* wrong number of arguments */
#define REXX_BAD_ARGS		ERR10_018	/* invalid argument to function */
#define REXX_INVALID_OPERAND	ERR10_048	/* invalid operand */

/*****************************************************************************/

/* structure for holding return information */
struct RetBlock
{
    LONG Type;
    union
    {
	LONG IntVal;
	STRPTR ArgString;
    } values;
};

/* variable types */
#define	RBINT		1
#define	RBARGSTRING	2

/*****************************************************************************/

#define ARG2(rmp) (rmp->rm_Args[2])
#define ARG3(rmp) (rmp->rm_Args[3])
#define ARG4(rmp) (rmp->rm_Args[4])
#define ARG5(rmp) (rmp->rm_Args[5])

/*****************************************************************************/

/* structure for holding the function records */
struct RexxCmds
{
   STRPTR rc_FName;				/* function name */
   LONG (*rc_Func)(struct RexxMsg *, struct RetBlock *, struct FileTypesLib *);
   STRPTR rc_Template;				/* option template */
   WORD rc_NumArgs;				/* number of arguments */
};

/*****************************************************************************/

#define	NUM_ARROWS	4

struct GlobalData
{
    /* Library bases */
    struct Library		*gd_SysBase;
    struct Library		*gd_DOSBase;
    struct Library		*gd_IntuitionBase;
    struct Library		*gd_GfxBase;
    struct Library		*gd_UtilityBase;
    struct Library		*gd_WorkbenchBase;
    struct Library		*gd_IconBase;
    struct Library		*gd_GadToolsBase;
    struct Library		*gd_LayersBase;
    struct Library		*gd_AmigaGuideBase;
    struct Library		*gd_AslBase;
    struct Library		*gd_DataTypesBase;
    struct Library		*gd_RexxSysBase;
    struct Library		*gd_LocaleBase;
    APTR			 gd_Catalog;

    /* Error processing */
    BOOL			 gd_Going;
    BOOL			 gd_Canceled;
    LONG			 gd_SecondaryResult;
    STRPTR			 gd_ErrorFileName;

    /* Argument handling */
    APTR			 gd_CmdHeader;
    struct Process		*gd_Process;
    struct RDArgs		*gd_RDArgs;
    ULONG			 gd_Result;
    ULONG			 gd_Result2;
    ULONG			 gd_Options[NUM_OPTS];
    ULONG			 gd_SourceType;
    APTR			 gd_SourceName;
    UBYTE			 gd_Buffer[128];
    UBYTE			 gd_NameBuffer[NAMEBUFSIZE];
    UBYTE			 gd_ScreenNameBuffer[MAXPUBSCREENNAME+1];

    /* ARexx */
    UBYTE			 gd_ARexxPortName[128];		/* ARexx port name */
    AREXXCONTEXT		 gd_ARexxHandle;
    STRPTR			 gd_ARexxReturn;		/* ARexx return string */
    struct MinList		 gd_UserMenuList;		/* User menu list */
    ULONG			 gd_NumUserMenus;		/* Number of items in User Menus */

    /* Keyboard navigation */
    struct Message		*gd_Msg;			/* Current message */
    ULONG			 gd_MsgType;			/* Message type: 0=Intuition, 1=ARexx */

    /* Application data */
    ULONG			 gd_Flags;
    struct Prefs		 gd_Prefs;			/* Preference information */
    struct FileRequester	*gd_FR;
    struct FrameInfo		 gd_FrameInfo;
    Object			*gd_DataObject;			/* Data object */
    UBYTE			 gd_TempBuffer[512];
    STRPTR			 gd_ScreenName;
    Class			*gd_WindowClass;		/* Window class */
    struct MsgPort		*gd_IDCMPPort;			/* Intuition message port */
    struct MsgPort		*gd_AWPort;			/* AppWindow message port */
    ULONG			 gd_Signals;
    struct GadgetInfo		 gd_GInfo;
    struct dtTrigger		 gd_Trigger;
    struct TextAttr		 gd_TextAttr;
    struct TextFont		*gd_TextFont;
    UBYTE			 gd_FontName[32];
    struct Screen		*gd_Screen;
    struct DrawInfo		*gd_DrawInfo;
    APTR			 gd_VI;
    Object			*gd_WindowObj;
    union printerIO		*gd_PIO;			/* Printer IO request */
    struct MsgPort		*gd_PrintPort;			/* Printer message port */
    struct Window		*gd_PrintWin;			/* Printing... window */
    ULONG			 gd_Unit;			/* Clipboard unit */
    struct List			 gd_WindowList;			/* Support window list */

    /* This used to be in the window class */

    struct Window		*gd_Window;
    struct Menu			*gd_Menu;
    APTR			 gd_OldWinPtr;
    struct AppWindow		*gd_AppWindow;
    struct Gadget		*gd_First;
    struct Gadget		*gd_HGad;			/* Horizontal gadget */
    struct Gadget		*gd_VGad;			/* Vertical gadget */
    Object			*gd_Model;			/* Model */
    Object			*gd_IC;				/* Interconnect */
    struct MinList		 gd_List;			/* List of gadgets */
    struct Image		*gd_ArrowImg[NUM_ARROWS];
    UBYTE			 gd_Title[128];
    ULONG			*gd_Methods;
    ULONG			 gd_WinFlags;
    LONG			 gd_TopVert;
    LONG			 gd_VisVert;
    LONG			 gd_TotVert;
    LONG			 gd_TopHoriz;
    LONG			 gd_VisHoriz;
    LONG			 gd_TotHoriz;

    /* Block pointer */
    struct BitMap		 gd_BlockBM;			/* Bitmap for block pointer */
    Object			*gd_BlockO;			/* Block pointer object */
};

#define	GDF_SCREEN		(1L<<0)
#define	GDF_OPENFONT		(1L<<1)
#define	GDF_OPENSCREEN		(1L<<2)
#define	GDF_SNAPSHOT		(1L<<3)
#define	GDF_BUSY		(1L<<4)
#define	GDF_MARK		(1L<<5)
#define	GDF_BITMAP		(1L<<6)		/* Using custom bitmap */
#define	GDF_CLEARPOINTER	(1L<<7)		/* Clear the pointer */
#define	GDF_REQUIRES_SCREEN	(1L<<8)

/*****************************************************************************/

#define	SysBase		gd->gd_SysBase
#define	DOSBase		gd->gd_DOSBase
#define	IntuitionBase	gd->gd_IntuitionBase
#define	GfxBase		gd->gd_GfxBase
#define	UtilityBase	gd->gd_UtilityBase
#define	WorkbenchBase	gd->gd_WorkbenchBase
#define	IconBase	gd->gd_IconBase
#define	GadToolsBase	gd->gd_GadToolsBase
#define	LayersBase	gd->gd_LayersBase
#define	AmigaGuideBase	gd->gd_AmigaGuideBase
#define	AslBase		gd->gd_AslBase
#define	DataTypesBase	gd->gd_DataTypesBase
#define	RexxSysBase	gd->gd_RexxSysBase
#define	LocaleBase	gd->gd_LocaleBase

/*****************************************************************************/

#define	window		gd->gd_Window

/*****************************************************************************/

#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))
#define MEMORY_N_FOLLOWING(ptr,n)  ((void *)( ((ULONG)ptr) + n ))

#define ASM		__asm
#define REG(x)		register __ ## x
#define	V(x)		((VOID *)(x))
#define	G(o)		((struct Gadget *)(o))

/*****************************************************************************/

ULONG __stdargs DoMethodA (Object *obj, Msg message);
ULONG __stdargs DoMethod (Object *obj, unsigned long MethodID, ...);
ULONG __stdargs DoSuperMethodA (struct IClass *cl, Object *obj, Msg message);
ULONG __stdargs DoSuperMethod (struct IClass *cl, Object *obj, unsigned long MethodID, ...);
ULONG __stdargs CoerceMethodA (struct IClass *cl, Object *obj, Msg message);
ULONG __stdargs CoerceMethod (struct IClass *cl, Object *obj, unsigned long MethodID, ...);
ULONG __stdargs SetSuperAttrs (struct IClass *cl, Object *obj, unsigned long Tag1, ...);

void __stdargs NewList (struct List *);

/*****************************************************************************/

void kprintf (void *, ...);
void __stdargs sprintf (void *, ...);

/*****************************************************************************/

#define	MULTIVIEW	TRUE
#include <localestr/utilities.h>

#include "multiview_iprotos.h"
