/* amigaguide_internal.h
 *
 */

/*****************************************************************************/

#define	ALTED		(IEQUALIFIER_RALT | IEQUALIFIER_LALT)
#define	SHIFTED		(IEQUALIFIER_RSHIFT | IEQUALIFIER_LSHIFT)

/*****************************************************************************/

enum NavigateCmds
{
    AC_TOP,
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

#define	MSG_NOTHING		0
#define	MMC_NOP			0

#define	MMC_OPEN		500
#define	MMC_OPENNEW		501
#define	MMC_SAVEAS		502
#define	MMC_PRINT		503
#define	MMC_ABOUT		504
#define	MMC_QUIT		505

#define	MMC_MINIMIZE		510
#define	MMC_NORMAL		511
#define	MMC_MAXIMIZE		512

#define	MMC_MARK		520
#define	MMC_COPY		521
#define	MMC_SELECTALL		522
#define	MMC_CLEARSELECTED	523
#define	MMC_SHOWCLIP		524
#define	MMC_PRINTCLIP		525

#define	MMC_FIND		530
#define	MMC_NEXT		531
#define	MMC_GOTO		532
#define	MMC_SETBOOKMARK		533
#define	MMC_GOTOBOOKMARK	534

#define	MMC_EXECUTE		540

#define	MMC_SAVE_AS_DEFAULTS	550

/*****************************************************************************/

#define	MAXPATHS	20
#define	MAXARG		64
#define	GADS		12

#define QUOTE		34
#define SQUOTE		39
#define	RSLASH		92

/*****************************************************************************/

struct ARexxContext
{
    struct MsgPort	*ARexxPort;			/* The port messages come in at... */
    LONG		 Outstanding;			/* The count of outstanding ARexx messages... */
    char		 PortName[24];			/* The port name goes here... */
    char		 ErrorName[28];			/* The name of the <base>.LASTERROR... */
    char		 Extension[8];			/* Default file name extension... */
};

#define	AREXXCONTEXT		struct ARexxContext *
#define	REXX_RETURN_ERROR	((struct RexxMsg *)-1L)

/*****************************************************************************/

#define	APSH_TOOL_ID 11000L
#define	StartupMsgID		(APSH_TOOL_ID+1L)	/* Startup message */
#define	LoginToolID		(APSH_TOOL_ID+2L)	/* Login a tool SIPC port */
#define	LogoutToolID		(APSH_TOOL_ID+3L)	/* Logout a tool SIPC port */
#define	ShutdownMsgID		(APSH_TOOL_ID+4L)	/* Shutdown message */
#define	ActivateToolID		(APSH_TOOL_ID+5L)	/* Activate tool */
#define	DeactivateToolID	(APSH_TOOL_ID+6L)	/* Deactivate tool */
#define	ActiveToolID		(APSH_TOOL_ID+7L)	/* Tool Active */
#define	InactiveToolID		(APSH_TOOL_ID+8L)	/* Tool Inactive */
#define	ToolStatusID		(APSH_TOOL_ID+9L)	/* Status message */
#define	ToolCmdID		(APSH_TOOL_ID+10L)	/* Tool command message */
#define	ToolCmdReplyID		(APSH_TOOL_ID+11L)	/* Reply to tool command */
#define	ShutdownToolID		(APSH_TOOL_ID+12L)	/* Shutdown tool */

struct AmigaGuideMsg
{
    struct Message	 agm_Msg;			/* Embedded Exec message structure */
    ULONG		 agm_Type;			/* Type of message */
    APTR		 agm_Data;			/* Pointer to message data */
    ULONG		 agm_DSize;			/* Size of message data */
    ULONG		 agm_DType;			/* Type of message data */
    ULONG		 agm_Pri_Ret;			/* Primary return value */
    ULONG		 agm_Sec_Ret;			/* Secondary return value */
    APTR		 agm_System1;
    APTR		 agm_System2;
};

#define	AGMSIZE		(sizeof (struct AmigaGuideMsg))

/*****************************************************************************/

struct Prefs
{
    WORD		 p_SWidth, p_SHeight;		/* Screen size */
    struct IBox		 p_Window;			/* Window normal position */
    struct IBox		 p_Zoom;			/* Window zoom position */
    struct IBox		 p_FileReq;			/* File requester position */
};

/*****************************************************************************/

#define	NAMEBUFSIZE	512

/* database client information */
struct Client
{
    struct Node		 cl_Node;			/* Node in the client list */
    struct AmigaGuideLib *cl_AG;			/* Cached library base */
    struct NewAmigaGuide *cl_NAG;			/* Pointer to the NewAmigaGuide structure */
    struct Screen	*cl_Screen;			/* Screen to open on */
    UBYTE		 cl_ScreenNameBuffer[MAXPUBSCREENNAME+1];
    struct DrawInfo	*cl_DrInfo;			/* Intuition DrawInfo */
    VOID		*cl_VI;				/* Visual Info */
    struct TextAttr	 cl_TAttr;			/* Text attribute */
    struct TextFont	*cl_TFont;			/* Font */

    struct MsgPort	*cl_IDCMPPort;			/* Intuition message port */
    struct MsgPort	*cl_AWPort;			/* Workbench AppWindow message port */
    struct MsgPort	*cl_AsyncPort;			/* Asynchronous message port */
    struct MsgPort	*cl_ChildPort;			/* Child message port */
    APTR		 cl_RexxHandle;			/* ARexx message port */
    ULONG		 cl_Signals;			/* Signal bits to wait on */

    struct List		 cl_WindowList;			/* List of windows */
    ULONG		 cl_ClientData;			/* Unique client data */
    ULONG		 cl_HelpGroup;			/* Unique help group */
    struct Menu		*cl_Menu;			/* Menu to share among all the windows */
    VOID		*cl_MemoryPool;			/* Memory pool */

    STRPTR		*cl_Context;			/* NULL terminated context table */
    ULONG		 cl_ContextID;			/* Current context ID */
    UBYTE		 cl_WorkText[128];		/* Temporary work text */

    ULONG		 cl_Flags;			/* Misc. flags */

    /* Current Window Information */
    struct FrameInfo	 cl_FrameInfo;			/* Embedded frame message */
    struct dtTrigger	 cl_Trigger;			/* Embedded trigger message */
    UBYTE		 cl_NameBuffer[NAMEBUFSIZE];	/* Name of the current data object */
    ULONG		 cl_TotVert, cl_TotHoriz;	/* Total width and height of the current object */
    Object		*cl_DataObject;			/* Current data object */
    Object		*cl_WinObject;			/* Current window object */
    struct Window	*cl_Window;			/* Current window pointer */
    LONG		 cl_Going;			/* Are we still going */
    struct FileRequester *cl_FR;			/* File reqeuster */
    struct Prefs	 cl_Prefs;			/* Preference information */

    /* Data for asynchronous process */
    struct Process	*cl_Process;			/* Async process */
    struct Process	*cl_ParentProc;			/* Parent process */
    struct AmigaGuideMsg cl_StartupMsg;			/* Embedded startup message */

    /* Printer */
    union printerIO	*cl_PIO;			/* Printer IO request */
    struct MsgPort	*cl_PrintPort;			/* Printer message port */
    struct Window	*cl_PrintWin;			/* Printing... window */

    /* Error return */
    ULONG		 cl_ErrorLevel;			/* Error level */
    ULONG		 cl_ErrorNumber;		/* Error number */
    STRPTR		 cl_ErrorString;		/* Error string */
    UBYTE		 cl_ErrorBuffer[256];

    UBYTE		 cl_TempBuffer[256];
};

typedef void *AMIGAGUIDECONTEXT;

/*****************************************************************************/

struct ClientWindow
{
    struct Node		 cw_Node;			/* Embedded node */
    Object		*cw_Object;			/* Window object */
};

/*****************************************************************************/

/* Allocation description structure */
struct NewAmigaGuide
{
    BPTR		 nag_Lock;			/* Lock on the document directory */
    STRPTR		 nag_Name;			/* Name of document file */
    struct Screen	*nag_Screen;			/* Screen to place windows within */
    STRPTR		 nag_PubScreen;			/* Public screen name to open on */
    STRPTR		 nag_HostPort;			/* Application's ARexx port name */
    STRPTR		 nag_ClientPort;		/* Name to assign to the clients ARexx port */
    STRPTR		 nag_BaseName;			/* Base name of the application */
    ULONG		 nag_Flags;			/* Flags */
    STRPTR		*nag_Context;			/* NULL terminated context table */
    STRPTR		 nag_Node;			/* Node to align on first (defaults to Main) */
    LONG		 nag_Line;			/* Line to align on */
    struct TagItem	*nag_Extens;			/* Tag array extension */
    VOID		*nag_Client;			/* Private! MUST be NULL */
};

/* public Client flags */
#define	AGF_LOAD_INDEX		(1L<<0)			/* Force load the index at init time */
#define	AGF_LOAD_ALL		(1L<<1)			/* Force load the entire database at init */
#define	AGF_CACHE_NODE		(1L<<2)			/* Cache each node as visited */
#define	AGF_CACHE_DB		(1L<<3)			/* Keep the buffers around until expunge */
#define	AGF_UNIQUE		(1L<<15)		/* Unique ARexx port name */
#define	AGF_NOACTIVATE		(1L<<16)		/* Don't activate window */

#define	AGFC_SYSGADS	0x80000000

/* private Client flags */
#define	AGF_DATABASE		(1L<<4)			/* set if file is a help file */
#define	AGF_EXPUNGE		(1L<<5)			/* database is ready to be expunged */
#define	AGF_ADDED		(1L<<6)			/* show that node was added to list */
#define	AGF_PUBSCREEN		(1L<<7)			/* Public screen */
#define	AGF_LARGER		(1L<<8)			/* Buttons are larger */
#define	AGF_AMIGAGUIDEHOST	(1L<<9)			/* Database is from a AmigaGuideHost */
#define	AGF_HOSTEXPUNGE		(1L<<10)		/* Host has gone away */
#define	AGF_HEIGHT		(1L<<11)		/* Height specified by document */
#define	AGF_FONT		(1L<<12)		/* Indicate that we did a OpenFont */
#define	AGF_IGNORECASE		(1L<<13)		/* Ignore case */
#define	AGF_WHOLEWORDS		(1L<<14)		/* Whole words */
#define	AGF_DONE		(1L<<17)
#define	AGF_PORTDEAD		(1L<<18)		/* The port is dead, long live the port. */

#define	AGERR_NOT_ENOUGH_MEMORY		100L
#define	AGERR_CANT_OPEN_DATABASE	101L
#define	AGERR_CANT_FIND_NODE		102L
#define	AGERR_CANT_OPEN_NODE		103L
#define	AGERR_CANT_OPEN_WINDOW		104L
#define	AGERR_INVALID_COMMAND		105L
#define	AGERR_CANT_COMPLETE		106L
#define	AGERR_PORT_CLOSED		107L
#define	AGERR_CANT_CREATE_PORT		108L

#define	FIND_LEVEL0	0	/* Immediate database */
#define	FIND_LEVEL1	1	/* Global database (help file) */
#define	FIND_LEVEL2	2	/* AmigaGuideHost databases */
#define	FIND_LEVEL3	3	/* All loaded databases */
#define	FIND_LEVEL4	4	/* Cross Reference list */

struct AmigaGuideHost
{
    struct Hook hh_Dispatcher;		/* Dispatcher */
    ULONG hh_Reserved;			/* Must be 0 */
    ULONG hh_Flags;
    ULONG hh_UseCnt;			/* Number of open nodes */
    APTR hh_SystemData;			/* Reserved for system use */
    APTR hh_UserData;			/* Anything you want... */
};

#define	HHSIZE	(sizeof(struct AmigaGuideHost))

/* Methods */
#define	HM_FindNode	1
#define	HM_OpenNode	2
#define	HM_CloseNode	3
#define	HM_FindXRef	4
#define	HM_Expunge	10		/* Expunge DataBase */

#if 0
amigaguide_internal.h 301 Warning 224: item "Msg" already defined
                                       See line 31 file "SC:INCLUDE/intuition/classusr.h"

#ifndef Msg
typedef struct
{
    ULONG MethodID;
} *Msg;
#endif
#endif

/* HM_FindNode */
struct opFindHost
{
    ULONG MethodID;
    struct TagItem *ofh_Attrs;		/*  R: Additional attributes */
    STRPTR ofh_Node;			/*  R: Name of node */
    STRPTR ofh_TOC;			/*  W: Table of Contents */
    STRPTR ofh_Title;			/*  W: Title to give to the node */
    STRPTR ofh_Next;			/*  W: Next node to browse to */
    STRPTR ofh_Prev;			/*  W: Previous node to browse to */
};

/* HM_FindXRef */
struct opFindXRef
{
    ULONG MethodID;
    struct TagItem *fxr_Attrs;		/*  R: Additional attributes */
    STRPTR fxr_Node;			/*  R: Name of node */
    STRPTR fxr_FileName;		/*  W: File name buffer */
    LONG fxr_Line;			/*  W: Line to align on */
    ULONG fxr_Flags;			/*  W: Control flags */
};

#define	HTXF_ASCII	(1L<<0)	/* File is straight ASCII */

/* HM_OpenNode, HM_CloseNode */
struct opNodeIO
{
    ULONG MethodID;
    struct TagItem *onm_Attrs;		/*  R: Additional attributes */
    STRPTR onm_Node;			/*  R: Node name and arguments */
    STRPTR onm_FileName;		/*  W: File name buffer */
    STRPTR onm_DocBuffer;		/*  W: Node buffer */
    ULONG onm_BuffLen;			/*  W: Size of buffer */
    ULONG onm_Flags;			/* RW: Control flags */
};

/* onm_Flags */
#define	HTNF_KEEP	(1L<<0)		/* Don't flush this node until database is
					 * closed. */
#define	HTNF_Reserved1	(1L<<1)		/* Reserved for system use */
#define	HTNF_Reserved2	(1L<<2)		/* Reserved for system use */
#define	HTNF_ASCII	(1L<<3)		/* Node is straight ASCII */
#define	HTNF_Reserved3	(1L<<4)		/* Reserved for system use */
#define	HTNF_CLEAN	(1L<<5)		/* Remove the node from the database */
#define	HTNF_DONE	(1L<<6)		/* Done with node */

/* onm_Attrs */
#define	HTNA_Screen	(TAG_USER+1)	/* Screen that window resides in */
#define	HTNA_Pens	(TAG_USER+2)	/* Pen array (from DrawInfo) */
#define	HTNA_Rectangle	(TAG_USER+3)	/* Window box */

/* HM_Expunge */
struct opExpungeNode
{
    ULONG MethodID;
    struct TagItem *oen_Attrs;		/*  R: Additional attributes */
};

#ifndef	XRSIZE
/* Cross reference node */
struct XRef
{
    struct Node xr_Node;	/* Embedded node */
    UWORD xr_Pad;		/* Padding */
    struct DocFile *xr_DF;	/* Document defined in */
    STRPTR xr_File;		/* Name of document file */
    STRPTR xr_Name;		/* Name of item */
    LONG xr_Line;		/* Line defined at */
};

#define	XRSIZE	(sizeof (struct XRef))

/* Types of cross reference nodes */
#define	XR_GENERIC	0
#define	XR_FUNCTION	1
#define	XR_COMMAND	2
#define	XR_INCLUDE	3
#define	XR_MACRO	4
#define	XR_STRUCT	5
#define	XR_FIELD	6
#define	XR_TYPEDEF	7
#define	XR_DEFINE	8
#endif

#define	AGA_Dummy		(TAG_USER)
#define	AGA_Path		(AGA_Dummy+1)
#define	AGA_XRefList		(AGA_Dummy+2)
#define	AGA_Activate		(AGA_Dummy+3)
#define	AGA_Context		(AGA_Dummy+4)
#define	AGA_HelpGroup		(AGA_Dummy+5)
#define	AGA_ClientData		(AGA_Dummy+6)

#define	AGA_HomeDir		(AGA_Dummy+7)
    /* (BPTR) The original directory that we were opened from (V40) */

#define	AGA_SIPCPort		(AGA_Dummy+8)
    /* (struct MsgPort *) Get a pointer to the SIPC port */

#define	AGA_ARexxPort		(AGA_Dummy+9)
    /* (struct MsgPort *) Pointer to the ARexx message port (V40) */

#define	AGA_ARexxPortName	(AGA_Dummy+10)
   /* (STRPTR) Used to specify the ARexx port name (V40) (not copied) */

