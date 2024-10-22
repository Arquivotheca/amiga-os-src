#ifndef LIBRARIES_APPSHELL_H
#define	LIBRARIES_APPSHELL_H

/************************************************************************
 *                                                                      *
 *                            Preliminary                               *
 *                        Amiga AppShell (tm)                           *
 *                                                                      *
 *   Copyright (c) 1990  Commodore-Amiga, Inc.  All Rights Reserved.    *
 *                                                                      *
 *   This software and information is proprietary, preliminary, and     *
 *   subject to change without notice.                                  *
 *                                                                      *
 *                            DISCLAIMER                                *
 *                                                                      *
 *   THIS SOFTWARE IS PROVIDED "AS IS".                                 *
 *   NO REPRESENTATIONS OR WARRANTIES ARE MADE WITH RESPECT TO THE      *
 *   ACCURACY, RELIABILITY, PERFORMANCE, CURRENTNESS, OR OPERATION      *
 *   OF THIS SOFTWARE, AND ALL USE IS AT YOUR OWN RISK.                 *
 *   NEITHER COMMODORE NOR THE AUTHORS ASSUME ANY RESPONSIBILITY OR     *
 *   LIABILITY WHATSOEVER WITH RESPECT TO YOUR USE OF THIS SOFTWARE.    *
 *                                                                      *
 *                          Non-Disclosure                              *
 *                                                                      *
 *   This information is not to be disclosed to any other company,      *
 *   individual or party.  Discussion is to be restricted to CBM        *
 *   approved discussion areas, such as a private area on bix           *
 *   dedicated to BETA versions of the Amiga AppShell.                  *
 *                                                                      *
 ************************************************************************
 * appshell.h
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * header file for the Amiga AppShell
 *
 */

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/semaphores.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/rdargs.h>
#include <dos/dosasl.h>

#ifndef LIBRARIES_APPOBJECTS_H
#include <libraries/appobjects.h>
#endif

#ifndef LIBRARIES_APSHATTRS_H
#include <libraries/apshattrs.h>
#endif

/* Maximum number of arguments in a command string.  Don't bother changing. */
#define	MAXARG 64

/* global object flags */
#define	APSHF_OPEN	(1L<<0)	/* object is open/available */
#define	APSHF_DISABLED	(1L<<1)	/* disabled from use */
#define	APSHF_PRIVATE	(1L<<2)	/* can't be called from command line */
#define	APSHF_ALIAS	(1L<<3)	/* aliased object */
#define	APSHF_LOCKON	(1L<<4)	/* object can't be disabled */
#define	APSHF_SYSTEM	(1L<<5)	/* Belongs to AppShell */

/* The Funcs structure contains base information for a command.  This
 * information gets translated into a Function Table entry and gets appended
 * to the Function Table list. */
struct Funcs
{
    UBYTE *fe_Name;		/* Name of function */
    VOID (*fe_Func)(struct Hook *, struct AppInfo *, struct AppFunction *);
    ULONG fe_ID;		/* ID of function */
    STRPTR fe_Template;		/* Command template */
    ULONG fe_NumOpts;		/* Number of options */
    ULONG fe_Flags;		/* Status of function */
    ULONG fe_HelpID;		/* index into the the text catalogue for help */
    STRPTR fe_Params;		/* optional parameters for function */
    ULONG *fe_GroupID;		/* ~0 terminated array of group ID's */
    LONG *fe_Options;		/* ReadOnly! ReadArgs */
};

/* no internal function defined for event */
#define	NO_FUNCTION	NULL

/* The Project structure is used to maintain information on a project set.
 * Project sets are not limited to the main project list, but also include
 * such things as lists of components for a project. */
struct Project
{
    struct List p_ProjList;	/* Project list */
    struct ProjNode *p_CurProj;	/* Current project */
    LONG p_NumProjs;		/* Number of projects */
    LONG p_MaxID;		/* Next available ID */
    LONG p_State;		/* Listview state */
    LONG p_TopView;		/* Listview top line */
    LONG p_CurLine;		/* Listview current line */
    ULONG p_Flags;		/* Project flags */
    APTR p_UserData;		/* User data extension */
    APTR p_SysData;		/* System data extension */
};

/* When this flag is set, the project information is being displayed by
 * the Project List data entry requester. */
#define	APSHF_PROJVIEW (1L<<1)


/* Within the Project structure is an Exec list called p_ProjList.  This
 * is a list of ProjNode structures.  This structure contains information
 * on projects or project components.  This information can be obtained at
 * startup time, from AppWindow/AppIcon messages, ASL file requester or
 * through application constructs. */
struct ProjNode
{
    struct Node pn_Node;	/* embedded Exec node */

    /* AppShell information.  Read only for application */
    struct DateStamp pn_Added;	/* date stamp when added to list */
    BPTR pn_ProjDir;		/* lock on project directory */
    STRPTR pn_ProjPath;		/* pointer to the projects' complete name */
    STRPTR pn_ProjName;		/* pointer to the projects' name */
    STRPTR pn_ProjComment;	/* pointer to the projects' comment */
    struct DiskObject *pn_DObj;	/* pointer to the projects' icon */
    LONG pn_ID;			/* user selected order */
    APTR pn_SysData;		/* System data extension */

    /* Application information */
    ULONG pn_Status;		/* status of project */
    ULONG pn_ProjID;		/* project ID */
    UBYTE pn_Name[32];		/* project name */
    APTR pn_UserData;		/* UserData for project */
    BOOL pn_Changed;		/* has project been modified? */
};

/* AppInfo structure that contains an AppShell application's global
 * variables.  This structure is variable length, so NEVER embedd this
 * within your own structures.  If you must, then reference it by a
 * pointer. */
#ifndef AppInfo

/* This is the locale context which AppShell maintains for each
 * running AppShell application.  It also gets passed into the
 * AppObjects stuff. */
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


struct AppInfo
{
    /* control information */
    UBYTE *ai_TextRtn;		/* Text return string */
    LONG ai_Pri_Ret;		/* Primary error (severity) */
    LONG ai_Sec_Ret;		/* Secondary error (actual) */
    BOOL ai_Done;		/* Done with main loop? */

    /* startup arguments */
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

    /* base application information */
    ULONG ai_Pad1;
    ULONG ad_Pad2;
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

    /* project information */
    struct Project ai_Project;	/* embedded Project structure */

    /* application information */
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
};

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

#endif

/* ai_Startup flags */
#define	APSHF_START_WB		(1L<<0)	/* if set then WB, else Shell */
#define	APSHF_START_CLONE	(1L<<1)

/* This structure is used to search through a Exec linked list using
 * DOS pattern matching */
struct AnchorList
{
    struct Node *al_CurNode;
    struct Node *al_NxtNode;
    STRPTR al_Token;
    ULONG al_Flags;
};

/* Pattern matching is case insensitive */
#define	APSHF_ANCHOR_NOCASE	(1L<<0)
#define	APSHF_ANCHOR_reserved	(1L<<1)

/*--------------------------------------------------------------------------*/
/* Following is information used by the main portion of the AppShell        */
/*--------------------------------------------------------------------------*/

/* Each message handler gets a base ID from which all of their commands are
 * offset.  The main functions are offset from zero. */
#define APSH_MAIN_ID 0L

/* Following are ID's for the functions that are implemented by the AppShell
 * whether there are any message handlers or not. */
#define MAIN_Dummy	APSH_MAIN_ID
#define AliasID		(MAIN_Dummy+1L) /* set up function w/parameters     */
#define DebugID		(MAIN_Dummy+2L) /* general debugging                */
#define DisableID	(MAIN_Dummy+3L) /* Disable a function               */
#define EditID		(MAIN_Dummy+4L) /* Edit an object                   */
#define EnableID	(MAIN_Dummy+5L) /* Enable a function                */
#define ExecMacroID	(MAIN_Dummy+6L) /* Execute the internal macro       */
#define FaultID		(MAIN_Dummy+7L) /* Return error text                */
#define GetID		(MAIN_Dummy+8L) /* Get object attribute             */
#define GroupID		(MAIN_Dummy+9L) /* Maintain object groups           */
#define HelpID		(MAIN_Dummy+10L)/* Help                             */
#define LearnID		(MAIN_Dummy+11L)/* Learn macro function             */
#define StopLearnID	(MAIN_Dummy+12L)/* Stop learn macro function        */
#define LoadMacroID	(MAIN_Dummy+13L)/* Load a macro into the internal memory */
#define PriorityID	(MAIN_Dummy+14L)/* Set an objects priority (process) */
#define	RemoveID	(MAIN_Dummy+15L)/* remove an object (project)       */
#define SaveMacroID	(MAIN_Dummy+16L)/* Save the internal macro to disk  */
#define SelectID	(MAIN_Dummy+17L)/* select an object (project)       */
#define SelectTopID	(MAIN_Dummy+18L)/* select first object (project)    */
#define SelectNextID	(MAIN_Dummy+19L)/* select next object (project)     */
#define SelectPrevID	(MAIN_Dummy+20L)/* select previous object (project) */
#define SelectBottomID	(MAIN_Dummy+21L)/* select last object (project)     */
#define SetID		(MAIN_Dummy+22L)/* Set object attributes            */
#define StatusID	(MAIN_Dummy+23L)/* Give status of an object         */
#define StopID		(MAIN_Dummy+24L)/* Stop an operation                */
#define StubID		(MAIN_Dummy+25L)/* NOP function                     */
#define	VersionID	(MAIN_Dummy+26L)/* Version                          */
#define	SystemID	(MAIN_Dummy+27L)/* Perform AmigaDOS command         */

/* data transmission function ID's via OpenSIPC, SIPCPrintf, CloseSIPC */
#define	LoginID		(MAIN_Dummy+100L)/* request data transmission session */
#define	DataStreamID	(MAIN_Dummy+101L)/* data transmission */
#define	SuspendID	(MAIN_Dummy+102L)/* temporarily suspend session */
#define	ResumeID	(MAIN_Dummy+103L)/* resume data session */
#define	LogoutID	(MAIN_Dummy+104L)/* signal end of session */

/* Following are ID's for functions that should be implemented by the
 * application to help promote a consistent set of functions for the
 * end user. */

/* standard project functions */
#define	NewID		(MAIN_Dummy+500L)/* new project/process */
#define	ClearID		(MAIN_Dummy+501L)/* clear current project */
#define	OpenID		(MAIN_Dummy+502L)/* open an existing project */
#define	SaveID		(MAIN_Dummy+503L)/* save project to existing name */
#define	SaveAsID	(MAIN_Dummy+504L)/* save project to a new name */
#define	RevertID	(MAIN_Dummy+505L)/* revert project to last saved */
#define	PrintID		(MAIN_Dummy+506L)/* print the current project */
#define	PrintAsID	(MAIN_Dummy+507L)/* define print configuration */
#define	AboutID		(MAIN_Dummy+508L)/* display application information */
#define	InfoID		(MAIN_Dummy+509L)/* display project information */
#define	QuitID		(MAIN_Dummy+510L)/* exit from the application */
#define	HideID		(MAIN_Dummy+511L)/* hide the project */
#define	CloseID		(MAIN_Dummy+512L)/* close the project */

#define	DropIconID	(MAIN_Dummy+519L)/* icon dropped in window */

/* application standard edit functions */
#define	MarkID		(MAIN_Dummy+520L)
#define	CutID		(MAIN_Dummy+521L)
#define	CopyID		(MAIN_Dummy+522L)
#define	PasteID		(MAIN_Dummy+523L)
#define	EraseID		(MAIN_Dummy+524L)
#define	UndoID		(MAIN_Dummy+525L)
#define	OpenClipID	(MAIN_Dummy+526L)
#define	SaveClipID	(MAIN_Dummy+527L)
#define	PrintClipID	(MAIN_Dummy+528L)
#define	ShowClipID	(MAIN_Dummy+529L)

/* application standard search functions */
#define	GotoID		(MAIN_Dummy+540L)
#define	FindID		(MAIN_Dummy+541L)
#define	NextID		(MAIN_Dummy+542L)
#define	ReplaceID	(MAIN_Dummy+543L)


/*--------------------------------------------------------------------------*/
/* Following is information used by the ARexx message handler               */
/*--------------------------------------------------------------------------*/

/* ID assigned to the ARexx message handler */
#define APSH_AREXX_ID 5000L

/* Following are ID's for the functions that are implemented by the ARexx
 * message handler. */
#define	AREXX_Dummy	APSH_AREXX_ID
#define	RXID		(AREXX_Dummy+1L)/* Execute an ARexx command */
#define	WhyID		(AREXX_Dummy+2L)/* Return information on the last error */
#define	RXSID		(AREXX_Dummy+3L)/* Execute an ARexx string file */

/* ID for an ARexx low-level message handler function */
#define	AH_SENDCMD 4

/*--------------------------------------------------------------------------*/
/* Following is information used by the Command Shell message handler       */
/*--------------------------------------------------------------------------*/

#define	APSH_DOS_ID 6000L
#define	CMDShellID	(APSH_DOS_ID+1L)	/* Activate the Command Shell */

/*--------------------------------------------------------------------------*/
/* Following is information used by the Intuition message handler           */
/*--------------------------------------------------------------------------*/

/* Delete a Window's winnode */
#define	IH_DELWIN		4

/* Close the screen context */
#define	IH_DELSCR		5


#define APSH_IDCMP_ID 7000L
#define	ActivateID	(APSH_IDCMP_ID+1L)	/* Open GUI */
#define	ButtonID	(APSH_IDCMP_ID+2L)	/* Edit gadget */
#define	KeystrokeID	(APSH_IDCMP_ID+3L)	/* Edit keystroke */
#define	MenuID		(APSH_IDCMP_ID+4L)	/* Edit menu */
#define	WindowToFrontID	(APSH_IDCMP_ID+5L)	/* bring current window to front */
#define	WindowToBackID	(APSH_IDCMP_ID+6L)	/* send current window to back */
#define	WindowID	(APSH_IDCMP_ID+7L)	/* open/close window */
#define	DeActivateID	(APSH_IDCMP_ID+8L)	/* Shutdown GUI */
#define	LockGUIID	(APSH_IDCMP_ID+9L)	/* Lock the GUI */
#define	UnLockGUIID	(APSH_IDCMP_ID+10L)	/* Unlock the GUI */

/* extended MenuItem structure */
struct EMenuItem
{
    struct MenuItem emi_Item;	/* embedded MenuItem structure */
    ULONG emi_MenuID;		/* ID used for function number to perform */
    APTR emi_UserData;		/* UserData (like other Intuition structs) */
};

/*--- KEYBOARD RELATED ITEMS ---*/
#define	SPECIAL 255

/* some useful defines */
#define	TAB      9
#define	RETURN  13
#define	ESC	27
#define	DELETE	127
#define	HELP	(SPECIAL+'?')
#define	FUNC1	(SPECIAL+'0')
#define	FUNC2	(SPECIAL+'1')
#define	FUNC3	(SPECIAL+'2')
#define	FUNC4	(SPECIAL+'3')
#define	FUNC5	(SPECIAL+'4')
#define	FUNC6	(SPECIAL+'5')
#define	FUNC7	(SPECIAL+'6')
#define	FUNC8	(SPECIAL+'7')
#define	FUNC9	(SPECIAL+'8')
#define	FUNC10	(SPECIAL+'9')
#define	UP	(SPECIAL+'A')
#define	DOWN	(SPECIAL+'B')
#define	RIGHT	(SPECIAL+'C')
#define	LEFT	(SPECIAL+'D')

/* component for our keyboard command array */
struct KeyboardCMD
{
    ULONG kbc_Key;		/* key */
    ULONG kbc_FuncID;		/* function ID */
};

/*--------------------------------------------------------------------------*/
/* Following is information used by the Simple IPC message handler          */
/*--------------------------------------------------------------------------*/

#define	APSH_SIPC_ID 10000L

/* This structure is used by the AppShell to communicate with tools and
 * other AppShell applications.
 *
 * If sipc_DType equal NULL, then the function ID in sipc_Type is performed
 * with no arguments.
 *
 *    PerfFunc (ai, sipc_Type, NULL, NULL);
 *
 * If sipc_DType equal APSH_SDT_TagList, then the function ID in sipc_Type is
 * performed with sipc_Data as the tag list arguments.
 *
 *    PerfFunc (ai, sipc_Type, NULL, sipc_Data);
 *
 * If sipc_DType equal APSH_SDT_Data, then the function ID in sipc_Type is
 * performed with with the following tags as arguments:
 *
 *    APSH_SIPCData,       sipc_Data
 *    APSH_SIPCDataLength, sipc_DSize
 *
 * If sipc_DType equal APSH_SDT_Command, then the string command line
 * passed in the sipc_Data field is performed:
 *
 *    PerfFunc (ai, NULL, sipc_Data, NULL);
 *
 */
struct SIPCMessage
{
    struct Message sipc_Msg;	/* Embedded Exec message structure */
    ULONG sipc_Type;		/* Type of message */
    APTR sipc_Data;		/* Pointer to message data */
    ULONG sipc_DSize;		/* Size of message data */
    ULONG sipc_DType;		/* Type of message data */
    ULONG sipc_Pri_Ret;		/* Primary return value */
    ULONG sipc_Sec_Ret;		/* Secondary return value */
    APTR sipc_Extens1;		/* *** PRIVATE *** SYSTEM USE ONLY! */
    APTR sipc_Extens2;		/* *** PRIVATE *** SYSTEM USE ONLY! */
};

/* These flags are used in the sipc_DType field to indicate what type of
 * information is in the sipc_Data field. */
#define	APSH_SDT_Command (1L<<1)/* Data is a STRPTR */
#define	APSH_SDT_TagList (1L<<2)/* Data is a list of TagItem's */
#define	APSH_SDT_Data    (1L<<3)/* Data is a pointer to a data block */
#define	APSH_SDT_Text    (1L<<4)/* text transmissions via sprintf */

/* Public SIPC port name given to the AppShell remote debugger.  Accessed
 * using OpenSIPC, SIPCPrintf and CloseSIPC. */
#define	DEBUGGER "AppShell_Debugger"

/*--------------------------------------------------------------------------*/
/* Following is information used by the Tool message handler                */
/*--------------------------------------------------------------------------*/

#define	APSH_TOOL_ID 11000L
#define	StartupMsgID	(APSH_TOOL_ID+1L)	/* Startup message */
#define	LoginToolID	(APSH_TOOL_ID+2L)	/* Login a tool SIPC port */
#define	LogoutToolID	(APSH_TOOL_ID+3L)	/* Logout a tool SIPC port */
#define	ShutdownMsgID	(APSH_TOOL_ID+4L)	/* Shutdown message */
#define	ActivateToolID	(APSH_TOOL_ID+5L)	/* Activate tool */
#define	DeactivateToolID (APSH_TOOL_ID+6L)	/* Deactivate tool */
#define	ActiveToolID	(APSH_TOOL_ID+7L)	/* Tool Active */
#define	InactiveToolID	(APSH_TOOL_ID+8L)	/* Tool Inactive */
#define	ToolStatusID	(APSH_TOOL_ID+9L)	/* Status message */
#define	ToolCmdID	(APSH_TOOL_ID+10L)	/* Tool command message */
#define	ToolCmdReplyID	(APSH_TOOL_ID+11L)	/* Reply to tool command */
#define	ShutdownToolID	(APSH_TOOL_ID+12L)	/* Shutdown tool */

typedef VOID (*F_PTR)(APTR, struct MsgPort *);

#define	TOOL_ACTIVATE (1L<<1)

/*--------------------------------------------------------------------------*/
/* Following is information used by the Workbench message handler           */
/*--------------------------------------------------------------------------*/

#define	APSH_WB_ID 12000L

/* APSH_CmdFlags */
#define	APSH_WBF_DISPLAY (1L<<1)	/* maintain display box for icon */
#define	APSH_WBF_PROJLIST (1L<<2)	/* add the WBArgs to the project list */
#define	APSH_WBF_NOLIST (1L<<3)		/* don't add the WBArgs to a list */

/*--------------------------------------------------------------------------*/
/* Following is information for use by the Application                      */
/*--------------------------------------------------------------------------*/

/* base tag for application functions */
#define	APSH_USER_ID 100000L


/*--------------------------------------------------------------------------*/
/* Following is low-level message handler information                       */
/*--------------------------------------------------------------------------*/

/* message handler object node */
struct MHObject
{
    struct Node mho_Node;	/* embedded Exec node */
    struct List mho_ObjList;	/* embedded List of children objects */
    struct MHObject *mho_Parent;/* pointer to parent object */
    struct MHObject *mho_CurNode;/* pointer to current child object */
    ULONG mho_ID;		/* numeric ID of object */
    ULONG mho_Status;		/* status of object */
    APTR mho_SysData;		/* message handler data */
    APTR mho_UserData;		/* application data */
    APTR mho_Extens1;		/* *** PRIVATE *** */
    UBYTE mho_Name[1];		/* name of object */
};

/* message handler node */
struct MsgHandler
{
    struct MHObject mh_Header;	/* embedded MHObject structure */

    struct MsgPort *mh_Port;	/* message port for handler */
    STRPTR mh_PortName;		/* port name, if public */
    ULONG mh_SigBits;		/* signal bits to watch for */

    /* handler functions */
    WORD mh_NumFuncs;		/* number of functions in handler */
    BOOL (**mh_Func)(struct AppInfo *,struct MsgHandler *,struct TagItem *);

    STRPTR *mh_DefText;		/* Default text catalogue */
    APTR mh_Catalogue;		/* *** PRIVATE *** */

    LONG mh_Outstanding;	/* Number of outstanding messages */
    APTR mh_Extens2;		/* *** PRIVATE *** */
};

/*--- interface function array pointers ---*/
#define	APSH_MH_OPEN		0	/* make a message handler active */
#define	APSH_MH_HANDLE		1	/* handle messages */
#define	APSH_MH_CLOSE		2	/* make a message handler inactive */
#define	APSH_MH_SHUTDOWN	3	/* free resources and delete message handler */

/* other common extensions */
#define	APSH_MH_WRITE		4	/* Write to message handler */
#define	APSH_MH_READ		5	/* Read from message handler */

/*--- node types ---*/
#define	APSH_MH_HANDLER_T	100	/* message handler node */
#define	APSH_MH_DATA_T		101	/* data node */

/*--- message handler object types */
#define	APSH_MHO_WINDOW		110	/* Intuition window */
#define	APSH_MHO_INTOBJ		111	/* AppShell Intuition object */
#define	APSH_MHO_TOOL		120	/* Tool */

/*--- node priorities ---*/
#define	APSH_MH_HANDLER_P	10	/* message handler node default priority */
#define	APSH_MH_DATA_P		-10	/* data node default priority */

/*--- overall status ---*/
#define	APSHP_INACTIVE		(1L<<1)
#define	APSHP_SINGLE		(1L<<2)
#define	APSH_REQUIRED		TRUE
#define APSH_OPTIONAL		FALSE

/*--------------------------------------------------------------------------*/
/* Following are ID's to use to access the AppShell text table              */
/*--------------------------------------------------------------------------*/

#ifndef APSH_CATALOG_H

#define	APSH_PAD		0L
#define	APSH_NOT_AN_ICON	(APSH_PAD+1L)	/* %s is not an icon. */
#define	APSH_NOT_AVAILABLE	(APSH_PAD+2L)	/* %s is not available */
#define APSH_PORT_ACTIVE	(APSH_PAD+3L)	/* %s port already active */
#define APSH_PORT_X_ACTIVE	(APSH_PAD+4L)	/* port, %s, already active */
#define APSH_NOT_AN_IFF		(APSH_PAD+5L)	/* %s is not an IFF file */
#define APSH_NOT_AN_IFF_X	(APSH_PAD+6L)	/* %1$s is not an IFF %2$s file */
#define APSH_CLOSE_ALL_WINDOWS	(APSH_PAD+7L)	/* Close all windows */
#define APSH_CMDSHELL_PROMPT	(APSH_PAD+8L)	/* Cmd> */
#define APSH_CLDNT_CREATE_X	(APSH_PAD+9L)	/* Could not create %s */
#define APSH_CLDNT_CREATE_PORT	(APSH_PAD+10L)	/* Could not create port, %s */
#define APSH_CLDNT_CREATE_OBJ	(APSH_PAD+11L)	/* Could not create object */
#define APSH_CLDNT_CREATE_OBJ_X	(APSH_PAD+12L)	/* Could not create object, %s */
#define APSH_CLDNT_CREATE_FILE	(APSH_PAD+13L)	/* Could not create file */
#define APSH_CLDNT_CREATE_FILE_X (APSH_PAD+14L)	/* Could not create file, %s */
#define APSH_CLDNT_INIT_X	(APSH_PAD+15L)	/* Could not initialize %s */
#define APSH_CLDNT_INIT_MSGH	(APSH_PAD+16L)	/* Could not initialize %s message handler */
#define APSH_CLDNT_LOCK		(APSH_PAD+17L)	/* Could not lock %s */
#define APSH_CLDNT_LOCK_DIR	(APSH_PAD+18L)	/* Could not lock directory */
#define APSH_CLDNT_LOCK_DIR_X	(APSH_PAD+19L)	/* Could not lock directory, %s */
#define APSH_CLDNT_LOCK_PUB	(APSH_PAD+20L)	/* Could not lock public screen */
#define APSH_CLDNT_LOCK_PUB_X	(APSH_PAD+21L)	/* Could not lock public screen, %s */
#define APSH_CLDNT_OBTAIN	(APSH_PAD+22L)	/* Could not obtain %s */
#define APSH_CLDNT_OPEN		(APSH_PAD+23L)	/* Could not open %s */
#define APSH_CLDNT_OPEN_FILE	(APSH_PAD+24L)	/* Could not open file */
#define APSH_CLDNT_OPEN_FILE_X	(APSH_PAD+25L)	/* Could not open file, %s */
#define APSH_CLDNT_OPEN_FONT_X	(APSH_PAD+26L)	/* Could not open font, %s */
#define APSH_CLDNT_OPEN_MACRO	(APSH_PAD+27L)	/* Could not open macro file, %s */
#define APSH_CLDNT_OPEN_PREF	(APSH_PAD+28L)	/* Could not open preference file, %s */
#define APSH_CLDNT_OPEN_SCREEN	(APSH_PAD+29L)	/* Could not open screen */
#define APSH_CLDNT_OPEN_WINDOW	(APSH_PAD+30L)	/* Could not open window */
#define APSH_SETUP_TIMER	(APSH_PAD+31L)	/* Could not set up timer event */
#define APSH_SETUP_HOTKEYS	(APSH_PAD+32L)	/* Could not set up HotKeys */
#define APSH_START_PROCESS	(APSH_PAD+33L)	/* Could not start process */
#define APSH_START_TOOL		(APSH_PAD+34L)	/* Could not start tool */
#define APSH_START_TOOL_X	(APSH_PAD+35L)	/* Could not start tool, %s */
#define APSH_WRITE_FILE		(APSH_PAD+36L)	/* Could not write to file */
#define APSH_WRITE_FILE_X	(APSH_PAD+37L)	/* Could not write to file, %s */
#define APSH_WRITE_MACRO	(APSH_PAD+38L)	/* Could not write to macro file */
#define APSH_CMDSHELL_WIN	(APSH_PAD+39L)	/* CON:0/150/600/50/Command Shell/CLOSE */
#define APSH_NO_NAMETAG_WIN	(APSH_PAD+40L)	/* No name given for window */
#define APSH_NO_PORT		(APSH_PAD+41L)	/* No port name specified */
#define APSH_NOT_ENOUGH_MEMORY	(APSH_PAD+42L)	/* Not enough memory */
#define APSH_WAITING_FOR_MACRO	(APSH_PAD+43L)	/* Waiting for macro return */
#define APSH_DISABLED		(APSH_PAD+44L)	/* %s is disabled */
#define APSH_IOERR		(APSH_PAD+45L)	/* IoErr #%ld */
#define APSH_INVALID_NAMETAG	(APSH_PAD+46L)	/* Invalid name tag. */
#define APSH_OKAY_TXT		(APSH_PAD+47L)	/* Okay */
#define APSH_CANCEL_TXT		(APSH_PAD+48L)	/* Cancel */
#define APSH_CONTINUE_TXT	(APSH_PAD+49L)	/* Continue */
#define APSH_DONE_TXT		(APSH_PAD+50L)	/* Done */
#define APSH_ABORT_TXT		(APSH_PAD+51L)	/* Abort */
#define APSH_QUIT_TXT		(APSH_PAD+52L)	/* Quit */
#define APSH_UNNAMED		(APSH_PAD+53L)	/* Unnamed */
#define APSH_SYNTAX_ERROR	(APSH_PAD+54L)	/* Syntax Error:\n%s %s */
#define	APSH_UNKNOWN_COMMAND	(APSH_PAD+55L)	/* Unknown command */
#define	APSH_CMDSHELL_TITLE	(APSH_PAD+56L)	/* Command shell title */
#define	APSH_USER_ABORTED	(APSH_PAD+57L)	/* User request aborted */
#define APSH_CLDNT_OPEN_APSH_CAT	(APSH_PAD+58l)	/* Locale problem */
#define APSH_LAST_MESSAGE	(APSH_PAD+59L)	/* Last error message */

#endif

/* Usually allocate one large block of memory for a group of items and then
 * divy out to the appropriate pointers.  Use with caution---must be
 * consistent with field types! */
#ifndef MEMORY_FOLLOWING
#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))
#define MEMORY_N_FOLLOWING(ptr,n)  ((void *)( ((ULONG)ptr) + n ))
#endif

/* Used to cast an pointer to a void pointer */
#ifndef V
#define V(x) ((void *)x)
#endif

/* BPTR compatibility */
#ifndef TO_BPTR
#define TO_BPTR(x) ((BPTR)(((ULONG)(x)) >> 2))
#endif

#endif /* LIBRARIES_APPSHELL_H */
