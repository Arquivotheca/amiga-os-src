/*
 * $Id: appshell_internal.h,v 1.4 1992/09/07 17:45:01 johnw Exp johnw $
 *
 * $Log: appshell_internal.h,v $
 * Revision 1.4  1992/09/07  17:45:01  johnw
 * Headers modified for latest screen/window changes.
 *
 * Revision 1.1  92/05/14  11:19:42  johnw
 * Initial revision
 *
 * Revision 1.2  92/01/23  11:00:39  johnw
 * This version is cleaned up for use from V37.4 Includes
 *
 * Revision 1.1  91/12/12  14:44:34  davidj
 * Initial revision
 *
 * Revision 1.1  90/07/02  11:13:54  davidj
 * Initial revision
 *
 *
 */

#define	AppInfo	internal_AppInfo

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <exec/libraries.h>

#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/sghooks.h>

#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>

#include <libraries/gadtools.h>
#include <libraries/prefs.h>
#include <libraries/amigaguide.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/rdargs.h>
#include <dos/dosasl.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>

#include <clib/macros.h>
#include <clib/alib_protos.h>
#include <clib/alib_stdio_protos.h>
#include <clib/console_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/amigaguide_protos.h>
#include <clib/icon_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/layers_protos.h>
#include <clib/wb_protos.h>
#include <clib/rexxsyslib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/amigaguide_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/wb_pragmas.h>
#include <pragmas/prefs_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/rexxsyslib_pragmas.h>

#include <stdlib.h>
#include <string.h>
#include <dos.h>

#ifndef SKIP_IPROTOS
#include "appshell_iprotos.h"
#endif

#include "appshell.h"
#include <libraries/appobjects.h>
#include <clib/appobjects_protos.h>
#include <pragmas/appobjects_pragmas.h>
#include "gfxextend.h"
#include "gfxextend_proto.h"

#define	APSH_KIND_APPINFO	0L

#define	APSH_ListDummy		(TAG_USER + 1L)
#define	APSH_TextList		(APSH_ListDummy + 1L)
#define	APSH_ImageList		(APSH_ListDummy + 2L)
#define	APSH_PointerList	(APSH_ListDummy + 3L)
#define	APSH_SoundList		(APSH_ListDummy + 4L)

/* structure for function table entry node */
struct FuncEntry
{
    struct Node fe_Node;	/* embedded Exec node */

    STRPTR fe_Name;		/* Name of function */
    VOID (*fe_Func)(struct AppInfo *, STRPTR, struct TagItem *);
    ULONG fe_ID;		/* ID of function */
    STRPTR fe_Template;		/* ReadArgs template */
    ULONG fe_NumOpts;		/* Number of ReadArgs options */
    ULONG fe_Flags;		/* Status of function */
    ULONG fe_HelpID;		/* index into the the text catalogue for help */
    STRPTR fe_Params;		/* optional parameters for function */
    ULONG *fe_GroupID;		/* ~0 terminated array of group ID's */
    LONG *fe_Options;		/* ReadArgs options */
    struct RDArgs *fe_ArgsPtr;	/* ReadArgs pointer */

    ULONG fe_HitCnt;		/* Access count */
    STRPTR fe_GadName;		/* Name of gadget that function is assigned to */
    struct WinNode *fe_WinNode;	/* Window node that <> is assigned to */
    struct ObjectNode *fe_Object;/* Object node that this is assigned to */
    struct Gadget *fe_Gadget;	/* Gadget that function is assigned to */
    USHORT fe_MenuNumber;	/* Menu item assigned to function */
    SHORT fe_Disable;		/* Disable count */
    VOID (*fe_Entry)();		/* Hook Entry */
};

struct FuncIDEntry
{
    struct Node fie_Node;
    struct FuncEntry *fie_FE;
    UBYTE fie_ID[12];
};

#define	APSHF_FTE_DISABLED	(1L<<1)
#define	APSHF_FTE_ALIAS 	(1L<<2)

/* AI_FUNCTION */
struct AppFunction
{
    ULONG MethodID;
    STRPTR af_CmdLine;		/* Untouched command line */
    struct TagItem *af_Attrs;	/* Attributes */
    LONG af_Kind;		/* ID of the active message handler */
    struct Message *af_Msg;	/* Pointer to the active message */
    struct Funcs *af_FE;	/* Current function entry */
};

#define	AI_FUNCTION	0

/* This is the locale context which AppShell maintains for each
 * running AppShell application.  It also gets passed into the
 * AppObjects stuff.
 *
 */
struct AppLocContext
{
	struct Library		*alc_LocBase;
	struct Locale		*alc_Loc;
	struct Catalog		*alc_ApshCat;
	struct Catalog		*alc_AppCat;
	struct AppString	*alc_ApshDef;
	struct AppString	*alc_AppDef;

	/* Ugly kludge of the week, this next field contains the */
	/* 'Next ID' for when emulating STRPTR arrays using the  */
	/* GT_Loc routine and such.
	                              */
	LONG				alc_nID;
};

/* AppInfo structure that contains an AppShell application's global
 * variables.  This structure is variable length, so NEVER embedd this
 * within your own structures.  If you must, then reference it by a
 * pointer. */
struct internal_AppInfo
{
    /* Control information */
    UBYTE *ai_TextRtn;		/* Text return string */
    LONG ai_Pri_Ret;		/* Primary error (severity) */
    LONG ai_Sec_Ret;		/* Secondary error (actual) */
    BOOL ai_Done;		/* Done with main loop? */

    /* Startup arguments */
    UWORD ai_Startup;		/* see defines below */
    LONG *ai_Options;		/* Option bucket */
    LONG ai_NumOpts;		/* Number of options */
    struct RDArgs *ai_ArgsPtr;	/* ReadArgs pointer */
    STRPTR *ai_FileArray;	/* MultiArg array (from the 0 bucket) */
				/* If the template specifies FILES/M, then
				 * the array is expanded using pattern
				 * matching and the resulting files are
				 * added to the project list */
    struct DiskObject *ai_ProgDO;/* Tool icon */

    /* Future expansion */
    ULONG ai_Pad1;
    ULONG ai_Pad2;

    /* Base application information */
    BPTR ai_ProgDir;		/* base directory for application */
    BPTR ai_ConfigDir;		/* configuration file directory */
    struct List ai_PrefList;	/* list of preference files */
    STRPTR ai_BaseName;		/* pointer to application base name */
    STRPTR ai_ProgName;		/* pointer to application program name */
    STRPTR ai_AppName;		/* pointer to application name */
    STRPTR ai_AppVersion;	/* pointer to version string */
    STRPTR ai_AppCopyright;	/* pointer to copyright notice */
    STRPTR ai_AppAuthor;	/* pointer to author */
    STRPTR ai_AppMsgTitle;	/* pointer to title for messages */

    /* Project information */
    struct Project ai_Project;	/* embedded Project structure */

    /* Application information */
    APTR ai_UserData;		/* UserData */

    /* READ ONLY Intuition-specific information */
    STRPTR ai_ScreenName;	/* pointer to public screen name */
    struct Screen *ai_Screen;	/* Active screen */
    struct TextFont *ai_Font;	/* Font for screen */
    struct Window *ai_Window;	/* Active window */
    struct Gadget *ai_Gadget;	/* Active gadget */
    struct MHObject *ai_CurObj;	/* Active object (gadget) */
    struct DrawInfo *ai_DI;	/* Intuition DrawInfo */
    VOID *ai_VI;		/* GadTools VisualInfo */
    WORD ai_MouseX;		/* Position at last IDCMP message */
    WORD ai_MouseY;		/* Position at last IDCMP message */
    UWORD ai_TopLine;		/* top line */

    /* This is the Locale support information */
    struct AppLocContext *ai_ALC;

/*------------------------------------------------------------------------*/
/* The following fields are not for public consumption                    */
/*------------------------------------------------------------------------*/
    struct List ai_MsgList;	/* list of message handlers */
    struct List ai_FuncList;	/* function table list by name */
    struct List ai_FuncList2;	/* function table by ID */
    struct List ai_ProjList;	/* main project list */
    struct List ai_SONList;	/* selected ObjectNode list */
    struct List ai_ToolList;	/* list of local tools */
    struct List ai_ImageList;	/* List of images used */

    struct FuncEntry *ai_CurFunc;/* last function used */
    ULONG ai_NumFuncs;		/* number of functions in the function table */
    WORD ai_NumCmds;		/* number of outstanding commands */
    ULONG ai_SigBits;		/* sum of all signal bits */
    ULONG ai_Flags;		/* flags... */
    ULONG ai_Funcs[10];		/* main AppShell function ID's */
    STRPTR *ai_DefText1;	/* application text table */
    APTR ai_Catalogue1;		/* locale.library handle */
    STRPTR *ai_DefText2;	/* AppShell text table */
    APTR ai_Catalogue2;		/* locale.library handle */
    UBYTE ai_WorkText[512];	/* text work area */
    BPTR ai_SysLock;		/* temporary lock for anything */
    BPTR ai_MacroFH;		/* learn macro file handle */
    struct AnchorPath ai_AP;	/* Anchor path for pattern expansion */
    struct RDArgs ai_CmdRDA;	/* ReadArgs structure for commands */
    ULONG ai_LastError;		/* last error number */

    /* For manipulating the template */
    STRPTR ai_Template;		/* A copy of the template */
    STRPTR ai_Tempargv[MAXARG];	/* Parsed template */
    LONG ai_Tempargc;		/* Number of arguments */

    struct AppNode *ai_AN;	/* Pointer back to header... */

    /* Control information */
    struct SignalSemaphore ai_Lock;	/* Lock for exclusive access */
    struct Process *ai_Process;	/* Process that this AppInfo belongs to */
    struct Window *ai_Temp;	/* Temporary window holder for DOS req. */

    struct TagItem ai_TmpTags[20];	/* Temporary tags */
    UBYTE ai_TempText[512];	/* Temporary text work area */
    UBYTE ai_LastText[512];	/* Last text error message */

    LONG ai_FailAt;		/* NotifyUser suppression level */

    LONG ai_ActvMH;		/* Active message handler */
    struct Message *ai_ActvMsg;	/* Active message */

#define	MH_AREXX	0
#define	MH_DOS		1
#define	MH_IDCMP	2
#define	MH_SIPC		3
#define	MH_TOOL		4
#define	MH_WB		5
#define	MH_MAX		6

    struct MsgHandler *ai_MH[MH_MAX];	/* Cached pointers */

    struct NewAmigaGuide ai_NewHyper;	/* Embedded NewAmigaGuide structure */
    AMIGAGUIDECONTEXT ai_HyperText;	/* HyperText context */

    /* This is for other languages */
    struct Hook ai_Hook;
    struct AppFunction ai_AF;	/* Embedded AppFunction */

    ULONG ai_Padding[20];
};

/*-------------------------------------------------------------------------*/
/* Library stuff                                                           */
/*-------------------------------------------------------------------------*/

extern ULONG SysBase, DOSBase, GfxBase, IconBase, IntuitionBase;
extern ULONG UtilityBase, WorkbenchBase, GadToolsBase, AppShellBase;
extern ULONG RexxSysBase, IFFParseBase, PrefsBase, AppObjectsBase;
extern ULONG LayersBase, AmigaGuideBase;

/* global variables required for library */
struct LibBase
{
    struct SignalSemaphore lb_Lock;	/* Data lock */
    struct List lb_ToolList;		/* Tool list */
    struct List lb_AppList;		/* List of applications */
    LONG lb_AppCount;			/* Number of applications running */
    ULONG lb_Flags;			/* Flags... */
    struct AppInfo *lb_Exchange;	/* Pointer to AppExchange */
    struct IOStdReq lb_IOReq;		/* Rawkey conversion IO request */
    struct Hook lb_Hook;		/* Default hook */
    struct Hook lb_NewHook;		/* New Hook */
};

struct AppNode
{
    struct Node an_Node;		/* Node in the list */
    struct SignalSemaphore an_Lock;	/* Lock for this AppInfo */
    struct AppInfo *an_AI;		/* Pointer to the AppInfo */
    ULONG an_Flags;			/* Flags... */
};

struct ExtLibrary
{
    struct Library lib;
    LONG seglist;
    struct LibBase *base;
};

/*-------------------------------------------------------------------------*/

#define	CONFIG_DRAWER "config"
#define	MAXKEYS	512

/* ai_Flags */
#define	APSHF_DEBUG	(1L<<1)
#define	APSHF_LEARN	(1L<<2)
#define	APSHF_RO	(1L<<3)
#define	APSHF_CMDSHELL	(1L<<4)
#define	APSHF_USERDATA	(1L<<5)

/* variable argument stuff */
typedef char *va_list;
#define va_dcl int va_alist
#define va_start(list) list = (char *) &va_alist
#define va_end(list)
#define va_arg(list, mode) ((mode *)(list += sizeof(mode)))[-1]

/* debugging macros */
#define	OUTP(x) SIPCPrintf(ai->ai_Debug, x)
#define	D(x) if (ai->ai_Flags & APSH_DEBUG) x
#define	DP(x) x

#define MAXARG 64		/* maximum command line arguments */

/*** MAIN STUFF ***********************************************************/
/* AppShell functions */
#define	APSH_CUSTOM_EXIT 0
#define	APSH_SIGC 1
#define	APSH_SIGD 2
#define	APSH_SIGE 3
#define	APSH_SIGF 4

#define	TellAppToActivate(name)	(SendSIPCMessage(name, ActivateID, NULL))

/*** AREXX ****************************************************************/
/* variables required for ARexx */
struct ARexxInfo
{
    STRPTR ari_Extens;		/* ARexx macro name extension */
    ULONG ari_UFunc[2];		/* User call-back function ID's */
    struct TagItem ari_Tags[2];	/* Temporary tags */
};

/* User call-back function pointers */
#define ARX_MACRO_ERROR	0
#define ARX_MACRO_GOOD	1

/* ARexx Functions Prototypes
struct RexxMsg *CreateRexxMsg (struct MsgPort *, STRPTR, STRPTR);
STRPTR CreateArgstring (STRPTR, int);
VOID DeleteRexxMsg (struct RexxMsg *);
VOID DeleteArgstring (STRPTR);
BOOL IsRexxMsg (struct Message *);
*/

/*** DOS COMMAND SHELL ****************************************************/
/* DOS shell status definitions */
#define AS_CLOSED 0
#define AS_CLOSING 1
#define AS_GOING 2
#define AS_OPEN 3

#define BUFFLEN      100

/* variables required for a DOS shell */
struct DOSInfo
{
    struct MsgPort *drport;	/* DOS reply port */
    UBYTE *winspec;		/* Shell window spec */
    UBYTE *closing_msg;		/* Closing message */
    struct StandardPacket *dmsg;/* DOS packet */
    struct Process *atask;	/* Our own process structure */
    BPTR old_CIS;		/* Orig. console input stream */
    BPTR old_COS;		/* Orig. console output stream */
    APTR old_ConsoleTask;	/* Orig. console task */
    BPTR acons;			/* Console file handle */
    BOOL packet_out;		/* is a READ outstanding? */
    SHORT dstatus;		/* The current status of the shell */
    UBYTE buff[BUFFLEN + 1];	/* used for reading user input */
    UBYTE aprompt[25];		/* prompt */
    ULONG di_NumCmds;		/* number of outstanding commands */
    struct Window *di_Win;	/* window used for the console */
    struct NewWindow di_NW;	/* For snapshotting the position */
    UBYTE di_Title[128];	/* Buffer for window title */
};

struct FindWindow
{
        struct StandardPacket FW_Pack;
        struct InfoData  FW_Info;
};

/*** INTUITION SPECIFIC DATA **********************************************/

/* component for our keyboard command array */
struct Hotkeys
{
    ULONG hk_FuncID;		/* function ID */
    ULONG hk_Flags;		/* Status of hotkey */
    ULONG hk_System1;		/* PRIVATE! SYSTEM USE ONLY */
    APTR hk_System2;		/* PRIVATE! SYSTEM USE ONLY */
    APTR hk_System3;		/* PRIVATE! SYSTEM USE ONLY */
};

/* information required for each window in this handler */
struct WinNode
{
    struct MHObject wn_Header;	/* common object header */
    struct AppInfo *wn_AI;	/* Pointer to the owner's AppInfo structure */
    struct ObjectInfo *wn_OI;	/* Window ObjectInfo */
    struct WinNode *wn_Parent;	/* Parent window node */
    struct Window *wn_Win;	/* Window pointer */
    struct TagItem *wn_NWTags;	/* NewWindow tags */
    struct TagItem *wn_WinEnv;	/* Window environment tags */
    struct List wn_MenuList;	/* Menu list */
    struct NewMenu *wn_GTMenu;	/* GadTools NewMenu array */
    struct Menu *wn_Menu;	/* prepared menu list pointer */
    struct Hotkeys *wn_HKey;	/* hotkey array */
    ULONG wn_Flags;		/* Flags */
    struct PointerPref *wn_PP;	/* Pointer preference associated with window */
    struct IBox wn_Zoom;	/* Zoom box */
    struct Requester wn_BR;	/* Blocking requester for lock GUI */
    struct NewWindow wn_NW;	/* cloned NewWindow structure */
    ULONG wn_Funcs[30];		/* Window functions */
    UBYTE wn_Title[128];	/* Window title buffer */
    ULONG wn_SlowIDCMP;
    ULONG wn_FastIDCMP;
};

#define	WMF_APSH_WINDOW	0x10000000

struct TempMenu
{
    struct Node tm_Node;
    struct NewMenu tm_NewMenu;
    ULONG tm_MenuNum;		/* Menu number */
};

#define	APSH_WINF_APPWIN	(1L<<1)	/* window contains App... */
#define	APSH_WINF_LOCKGUI	(1L<<2)	/* Ignore IDCMP for window */
#define	APSH_WINF_TEXTMENU	(1L<<3)	/* Contains a text menu */
#define	APSH_WINF_LOCALTEXT	(1L<<4)	/* Contains a local text table */

/* variables required for IDCMP message handling */
struct IDCMPInfo
{
    struct MsgPort *ii_Port;	/* Message port */

    struct Screen *ii_Screen;	/* Screen */
    struct Screen *ii_PubScreen;/* Public screen info */
    struct NewScreen *ii_NS;	/* NewScreen structure */
    struct TagItem *ii_NSTags;	/* NewScreen tags */
    USHORT ii_ScrType;		/* Screen Type */
    BOOL Open_Screen;		/* Did we open the screen? */

    ULONG ii_NumWins;		/* Number of windows open */
    struct Window *ii_Window;	/* Active Window */
    struct WinNode *ii_CurWin;	/* Current window node */
    ULONG ii_Flags;		/* Default Window flags */
    ULONG ii_SlowIDCMP;		/* Slow IDCMP flags */
    ULONG ii_FastIDCMP;		/* Fast IDCMP flags */
    UWORD ii_TopLine;		/* Top line */
    struct List ii_List;	/* Window list */

    struct List ii_ActvList;	/* List of active stuff */
    struct ObjectNode *ii_CurObj;/* Currrently active object */

    ULONG ii_Funcs[30];		/* IDCMP function ID's */
    struct TextAttr ii_FontAttr;/* Desired font attribute */

    struct TagItem ii_Tags[20];	/* Temporary tags */
    struct IntuiMessage ii_Msg;	/* Temporary IntuiMessage */

    /* Menu parsing */
    STRPTR ii_Template;		/* Menu template */
    LONG ii_NumOpts;		/* Number of options in menu template */
    LONG ii_Options[12];	/* Parsed menu line */

    /* Icon */
    NEWIMAGEBOB ii_NewIBob;	/* NewBob structure */
    struct Bob *ii_Selected;	/* Selected bob */
    SHORT ii_X, ii_Y;		/* Where is bob? */
    SHORT ii_oX, ii_oY;		/* Where was bob? */

};

/* Need something for tools also */
struct ActiveRec
{
    struct Node ar_Node;	/* Node in the active list */
    struct TagItem *ar_WE;	/* Window environment of closed window */
};

#define	APSH_SIZEVERIFY      0
#define	APSH_NEWSIZE         1
#define	APSH_REFRESHWINDOW   2
#define	APSH_MOUSEBUTTONS    3
#define	APSH_REQSET          4
#define	APSH_CLOSEWINDOW     5
#define	APSH_REQVERIFY       6
#define	APSH_REQCLEAR        7
#define	APSH_MENUVERIFY      8
#define	APSH_DISKINSERTED    9
#define	APSH_DISKREMOVED    10
#define	APSH_ACTIVEWINDOW   11
#define	APSH_INACTIVEWINDOW 12

#define	APSH_WINC_INIT      15	/* Init function */
#define	APSH_WINC_BOPEN     16	/* Function to call before opening window */
#define	APSH_WINC_AOPEN     17	/* Function to call after opening window */
#define	APSH_WINC_BCLOSE    18	/* Function to call before closing window */
#define	APSH_WINC_ACLOSE    19	/* Function to call after closing window */

/*** SIPC *****************************************************************/
/* variables required for SIPC */
struct SIPCInfo
{
    struct MsgPort *sipc_Reply;		/* SIPC reply message port */
    ULONG sipc_ufunc[2];		/* user call-back functions */
    LONG sipc_Messages;			/* Outstanding messages */
};

#define	SIPC_OK		0
#define	SIPC_ERROR	1

/* OpenSIPC, SIPCPrintf and CloseSIPC private structure */
struct SIPCHandle
{
    struct SIPCMessage sh_Msg;		/* embedded SIPC message */
    ULONG sh_Type;			/* type of data transmission */
    struct AppInfo *sh_AI;		/* pointer to the AppInfo structure */
    struct MsgPort *sh_Port;		/* reply message port */
    struct MsgPort *sh_Dest;		/* address of destination SIPC port */
    STRPTR sh_Name;			/* name of the destination SIPC port */
    STRPTR sh_Buffer;			/* data buffer */
};

/*** TOOL STUFF ***********************************************************/
/* structure for tool table entry */
struct Tools
{
    struct Node te_Node;	/* Node for tool entry */
    VOID (*te_Func)(VOID *);	/* Address of function */
    ULONG te_ID;		/* ID of function */
    ULONG te_Flags;		/* Status of function */
    ULONG te_HitCnt;		/* Access count */
    ULONG te_Stack;		/* Stack requirements for function */
    ULONG te_Pri;		/* Default priority for function */
    ULONG te_UseCnt;		/* Current use count of function */
    ULONG te_MaxCnt;		/* Maximum instances of function */
    UBYTE *te_Port;		/* Port name of owner */
};

/* variables required for Tools */
struct ToolInfo
{
    struct MsgPort *ti_Port;	/* Tool message port */
    struct List ti_List;	/* List of active functions */
    BOOL ti_Status;		/* Tool status */
    struct SignalSemaphore ti_Sem;/* Shutdown lock */
};

/* information passed to tool on startup */
struct ToolStartup
{
    UBYTE *ts_Name;		/* Name of process to run */
    APTR ts_UserData;		/* Data requirements of tool */
    struct MsgPort *ts_Port;	/* SIPC port of tool */
};

/*
 * Structure that is a fake SegList for the code.  It will contain
 * a single JMP instruction to the code we wish to start.
 *
 * THIS STRUCTURE MUST BE LONGWORD ALIGNED!!!!
 */
struct CodeHdr
{
    ULONG SegSize;
    ULONG NextSeg;
    UWORD JumpInstr;
    APTR Function;
};

/*
 * My message type for sending message between my processes and
 * the main process...
 */
struct ToolMessage
{
    struct Message tm_Msg;		/* Message header */
    ULONG tm_Type;			/* Message Type */
    struct ToolFuncNode *tm_TFN;	/* ToolFunc Node */
    struct MsgPort *tm_SIPC;		/* SIPC message port */
    struct SignalSemaphore *tm_Sem;	/* pointer to the semaphore */
};

struct ToolFuncNode
{
/* NEEDS TO BE CHANGED TO A MHObject STRUCTURE */
    struct Node tfn_Node;		/* Node header */
    SHORT tfn_Key;			/* Returned process flag */
    struct CodeHdr tfn_SegList;		/* DON'T MOVE! Process SegList */

    struct ToolMessage tfn_Msg;		/* The startup message */
    F_PTR tfn_Func;			/* Address of function */
    VOID *tfn_Data;			/* UserData for process */
    ULONG tfn_Stack;			/* Stack size of process */
    ULONG tfn_Priority;			/* Priority of process */
    BPTR tfn_Lock;			/* Directory lock for process */
    UBYTE tfn_Name[1];			/* Name (keep at end) */
};
