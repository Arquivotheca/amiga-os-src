#ifndef LIBRARIES_APSHATTRS_H
#define	LIBRARIES_APSHATTRS_H

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
 * apshattrs.h
 * Copyright (C) 1990,1991 Commodore-Amiga, Inc.
 *
 * Attribute ID's for AppShell objects
 *
 */

#include <exec/types.h>
#include <utility/tagitem.h>

/* AppShell object kinds */
#define	APSH_KIND_SIPCMSG	1L
#define	APSH_KIND_MSGHANDLER	2L
#define	APSH_KIND_MHOBJECT	3L
#define	APSH_KIND_ANCHORLIST	4L
#define	APSH_KIND_PROJECT	5L
#define	APSH_KIND_PROJNODE	6L
#define	APSH_KIND_FUNCENTRY	7L

/*
 * C	- Can be set at object creation time, using NewAPSHObject(), or
 *	  HandleApp(), HandleAppSync().
 * S	- Can use SetAPSHAttr() to set the value.
 * G	- Can use GetAPSHAttr() to get the value of.
 *
 */

/*------------------------------------------------------------------------*/
/* The AppShell uses the following tags. Reserved TAG_USR+24000L - 25999L */
/*------------------------------------------------------------------------*/

#define	APSH_Dummy		TAG_USER+24000L

/* library management */
#define	APSH_OpenLibraries	(APSH_Dummy+1L)		/* open libraries */
#define	APSH_LibNameTag		(APSH_Dummy+2L)		/* library name tag */
#define	APSH_LibName		(APSH_Dummy+3L)		/* library name */
#define	APSH_LibVersion		(APSH_Dummy+4L)		/* library version */
#define	APSH_LibStatus		(APSH_Dummy+5L)		/* required/optional */
#define	APSH_LibReserved5	(APSH_Dummy+10L)	/* RESERVED FOR SYSTEM USE */
#define	APSH_LibBase		(APSH_Dummy+11L)	/* library base */
#define	APSH_ARexxSys		(APSH_Dummy+12L)	/* rexxsyslib.library */
#define	APSH_ARexxSup		(APSH_Dummy+13L)	/* rexxsupport.library */
#define	APSH_ASL		(APSH_Dummy+14L)	/* asl.library */
#define	APSH_Commodities	(APSH_Dummy+15L)	/* commodities.library */
#define	APSH_DiskFont		(APSH_Dummy+16L)	/* diskfont.library */
#define	APSH_DOS		(APSH_Dummy+17L)	/* dos.library */
#define	APSH_GadTools		(APSH_Dummy+18L)	/* gadtools.library */
#define	APSH_Gfx		(APSH_Dummy+19L)	/* graphics.library */
#define	APSH_Icon		(APSH_Dummy+20L)	/* icon.library */
#define	APSH_Intuition		(APSH_Dummy+21L)	/* intuition.library */
#define	APSH_Layers		(APSH_Dummy+22L)	/* layers.library */
#define	APSH_IFF		(APSH_Dummy+23L)	/* iffparse.library */
#define	APSH_Translate		(APSH_Dummy+24L)	/* translator.library */
#define	APSH_Utility		(APSH_Dummy+25L)	/* utility.library */
#define	APSH_Workbench		(APSH_Dummy+26L)	/* workbench.library */
#define	APSH_AppObjects		(APSH_Dummy+27L)	/* appobjects.library */
#define	APSH_Hyper		(APSH_Dummy+28L)	/* hyper.library */
#define	APSH_Prefs		(APSH_Dummy+29L)	/* prefs.library */

/* main AppShell tags */
#define	APSH_NumArgs		(APSH_Dummy+40L)	/* Number of Shell arguments */
#define	APSH_ArgList		(APSH_Dummy+41L)	/* Shell arguments */
#define	APSH_WBStartup		(APSH_Dummy+42L)	/* Workbench arguments */
#define	APSH_ControlPort	(APSH_Dummy+43L)	/* SIPC Control port for a cloned AppShell */
#define	APSH_AppName		(APSH_Dummy+44L)	/* pointer to the application's name */
#define	APSH_AppVersion		(APSH_Dummy+45L)	/* pointer to the application's version */
#define	APSH_AppCopyright	(APSH_Dummy+46L)	/* pointer to the application's (c) notice */
#define	APSH_AppAuthor		(APSH_Dummy+47L)	/* pointer to the application's author */
#define	APSH_AppMsgTitle	(APSH_Dummy+48L)	/* pointer to message title */
#define	APSH_FuncTable		(APSH_Dummy+55L)	/* function table for application */
#define	APSH_DefText		(APSH_Dummy+56L)	/* Default text catalogue */
#define	APSH_AppInit		(APSH_Dummy+57L)	/* Custom application init function ID */
#define	APSH_AppExit		(APSH_Dummy+58L)	/* Custom application shutdown function ID */
#define	APSH_SIG_C		(APSH_Dummy+59L)	/* SIG_BREAK_C function ID */
#define	APSH_SIG_D		(APSH_Dummy+60L)	/* SIG_BREAK_D function ID */
#define	APSH_SIG_E		(APSH_Dummy+61L)	/* SIG_BREAK_E function ID */
#define	APSH_SIG_F		(APSH_Dummy+62L)	/* SIG_BREAK_F function ID */
#define	APSH_ProjInfo		(APSH_Dummy+63L)	/* pointer to a Project/ProjNode structure */
#define	APSH_BaseName		(APSH_Dummy+64L)	/* Base name */
#define	APSH_Template		(APSH_Dummy+65L)	/* Startup template */
#define	APSH_NumOpts		(APSH_Dummy+66L)	/* Number of options */
#define	APSH_FuncEntry		(APSH_Dummy+67L)	/* Funcs structure for command */
#define	APSH_UserData		(APSH_Dummy+68L)	/* Preallocated user data */
#define	APSH_ProjIcon		(APSH_Dummy+69L)	/* Default project icon */
#define	APSH_ProjIconName	(APSH_Dummy+70L)	/* Default project icon name */
#define	APSH_MsgHandler		(APSH_Dummy+71L)	/* Message handler pointer */
#define	APSH_UserDataSize	(APSH_Dummy+72L)	/* Size of data */
#define	APSH_PortAddr		(APSH_Dummy+73L)	/* Message port address */
#define	APSH_AppHandle		(APSH_Dummy+74L)	/* Handle to tool application */
#define	APSH_DispatchHook	(APSH_Dummy+75L)	/* Alt. function dispatching hook */
#define	APSH_HookClass		(APSH_Dummy+76L)	/* Use class style hook */
#define APSH_AppCatalog		(APSH_Dummy+77L)	/* Appl. catalog name (STRPTR) */
#define APSH_AppDefLang		(APSH_Dummy+78L)	/* Appl. default lang (STRPTR) */

/* message handler routines */
#define	APSH_AddHandler		(APSH_Dummy+80L)	/* add a message handler to application */
#define	APSH_Setup		(APSH_Dummy+81L)	/* setup function */
#define	APSH_Status		(APSH_Dummy+82L)	/* active, inactive, multiple, etc... */
#define	APSH_Rating		(APSH_Dummy+83L)	/* optional/required, etc... */
#define	APSH_Port		(APSH_Dummy+84L)	/* name of the message port */
#define	APSH_Handler		(APSH_Dummy+85L)	/* Handler ID */
#define	APSH_CmdData		(APSH_Dummy+86L)	/* Command data */
#define	APSH_CmdDataLength	(APSH_Dummy+87L)	/* Length of command data */
#define	APSH_CmdID		(APSH_Dummy+88L)	/* Command ID (function) */
#define	APSH_CmdString		(APSH_Dummy+89L)	/* Command string */
#define	APSH_CmdTagList		(APSH_Dummy+90L)	/* Command tag list */
#define	APSH_Command		(APSH_Dummy+91L)	/* Handler command */
#define	APSH_NameTag		(APSH_Dummy+92L)	/* Name Tag for object */
#define	APSH_CmdFlags		(APSH_Dummy+93L)	/* Command Flags */
#define	APSH_TextID		(APSH_Dummy+94L)	/* Text ID */
#define	APSH_BaseID		(APSH_Dummy+95L)	/* Base ID */

#define	APSH_AddARexx_UI	(APSH_Dummy+96L)	/* ARexx UI */
#define	APSH_AddCmdShell_UI	(APSH_Dummy+97L)	/* Command Shell UI */
#define	APSH_AddIntui_UI	(APSH_Dummy+98L)	/* Graphical UI */
#define	APSH_AddSIPC_UI		(APSH_Dummy+99L)	/* Simple IPC UI */
#define	APSH_AddTool_UI		(APSH_Dummy+100L)	/* Tool UI */
#define	APSH_AddWB_UI		(APSH_Dummy+101L)	/* Workbench UI */
#define	APSH_AddClone_UI	(APSH_Dummy+102L)	/* PRIVATE */

/* ARexx information */
#define	APSH_Extens		(APSH_Dummy+120L)	/* ARexx macro name extension */
#define	APSH_ARexxError		(APSH_Dummy+121L)	/* ARexx command ERROR function ID */
#define	APSH_ARexxOK		(APSH_Dummy+122L)	/* ARexx command OK function ID */
#define	APSH_ARexxString	(APSH_Dummy+123L)	/* ARexx string file */

/* Command shell */
#define	APSH_CloseMsg		(APSH_Dummy+140L)	/* Closing message */
#define	APSH_CMDWindow		(APSH_Dummy+141L)	/* Command Shell window spec */
#define	APSH_Prompt		(APSH_Dummy+142L)	/* Command Shell prompt */
#define	APSH_CMDTitle		(APSH_Dummy+143L)	/* Command Shell title */
#define	APSH_CMDParent		(APSH_Dummy+144L)	/* Command Shell parent window */

/* Window information */
#define	APSH_WindowEnv		(APSH_Dummy+160L)	/* Window Environment */
#define	APSH_TextAttr		(APSH_Dummy+161L)	/* Text Attributes */
#define	APSH_NewScreen		(APSH_Dummy+162L)	/* NewScreen structure */
#define	APSH_NewScreenTags	(APSH_Dummy+163L)	/* Tags for new screen */
#define	APSH_Palette		(APSH_Dummy+164L)	/* Color Palette */
#define	APSH_NewWindow		(APSH_Dummy+165L)	/* NewWindow structure */
#define	APSH_NewWindowTags	(APSH_Dummy+166L)	/* Tags for new window */
#define	APSH_HotKeys		(APSH_Dummy+167L)	/* HotKey command array */
#define	APSH_Menu		(APSH_Dummy+168L)	/* Intuition-style Menu array */
#define	APSH_Gadgets		(APSH_Dummy+169L)	/* Intuition-style Gadget array */
#define	APSH_GTMenu		(APSH_Dummy+170L)	/* GadTools-style Menu array */
#define	APSH_GTGadgets		(APSH_Dummy+171L)	/* GadTools-style NewGadget array */
#define	APSH_GTFlags		(APSH_Dummy+172L)	/* flags for GadTools objects */
#define	APSH_Objects		(APSH_Dummy+173L)	/* Object array */
#define	APSH_ObjDown		(APSH_Dummy+174L)	/* Gadget downpress function ID */
#define	APSH_ObjHold		(APSH_Dummy+175L)	/* Gadget hold function ID */
#define	APSH_ObjRelease		(APSH_Dummy+176L)	/* Gadget release function ID */
#define	APSH_ObjDblClick	(APSH_Dummy+177L)	/* Gadget double-click function ID */
#define	APSH_ObjAbort		(APSH_Dummy+178L)	/* Gadget abort function ID */
#define	APSH_ObjAltHit		(APSH_Dummy+179L)	/* Gadget ALT hit function ID */
#define	APSH_ObjShiftHit	(APSH_Dummy+180L)	/* Gadget SHIFT hit function ID */
#define	APSH_ObjData		(APSH_Dummy+181L)	/* Gadget image or data */
#define	APSH_ObjInner		(APSH_Dummy+182L)	/* Inner rectangle */
#define	APSH_ObjPointer		(APSH_Dummy+183L)	/* pointer name prefix */
#define	APSH_DefWinFlags	(APSH_Dummy+184L)	/* Default window flags */
#define	APSH_ObjName		(APSH_Dummy+185L)	/* Object name */
#define	APSH_WinName		(APSH_Dummy+186L)	/* Window name */
#define	APSH_WinPointer		(APSH_Dummy+188L)	/* Pointer to window */
#define	APSH_ShowSelected	(APSH_Dummy+189L)	/* Name of text object for list */
#define APSH_Screen		(APSH_Dummy+190L)	/* Screen pointer */
#define	APSH_ObjExtraRelease	(APSH_Dummy+191L)	/* Alternate button release */
#define	APSH_ObjAltData		(APSH_Dummy+192L)	/* Alternate image */
#define	APSH_ParentWindow	(APSH_Dummy+193L)	/* Parent window (defaults to Main) */
#define	APSH_TTMenu		(APSH_Dummy+194L)	/* Index of menu in text table */
#define	APSH_WinText		(APSH_Dummy+195L)	/* Window text catalogue */
#define	APSH_ObjCreate		(APSH_Dummy+196L)	/* After object is created */
#define	APSH_ObjDelete		(APSH_Dummy+197L)	/* Before object is removed */
#define	APSH_GA_Image		(APSH_Dummy+198L)	/* Name of boopsi image */
#define	APSH_GA_SelectRender	(APSH_Dummy+199L)	/* Name of boopsi image */
#define	APSH_ObjUpdate		(APSH_Dummy+200L)	/* Gadget IDCMPUPDATE function */
#define	APSH_GA_LabelImage	(APSH_Dummy+201L)	/* Name of boopsi image */
#define	APSH_MenuTable		(APSH_Dummy+202L)	/* Pointer to menu text table */
#define	APSH_ObjIDHold		(APSH_Dummy+203L)	/* System use only */

/* IDCMP messages */
#define	APSH_SizeVerify		(APSH_Dummy+220L)	/* SIZEVERIFY function ID */
#define	APSH_NewSize		(APSH_Dummy+221L)	/* NEWSIZE function ID */
#define	APSH_RefreshWindow	(APSH_Dummy+222L)	/* REFRESHWINDOW function ID */
#define	APSH_MouseButtons	(APSH_Dummy+223L)	/* MOUSEBUTTONS function ID */
#define	APSH_ReqSet		(APSH_Dummy+224L)	/* REQSET function ID */
#define	APSH_CloseWindow	(APSH_Dummy+225L)	/* CLOSEWINDOW  function ID */
#define	APSH_ReqVerify		(APSH_Dummy+226L)	/* REQVERIFY function ID */
#define	APSH_ReqClear		(APSH_Dummy+227L)	/* REQCLEAR function ID */
#define	APSH_MenuVerify		(APSH_Dummy+228L)	/* MENUVERIFY function ID */
#define	APSH_DiskInserted	(APSH_Dummy+229L)	/* DISKINSERTED function ID */
#define	APSH_DiskRemoved	(APSH_Dummy+230L)	/* DISKREMOVED function ID */
#define	APSH_ActiveWindow	(APSH_Dummy+231L)	/* ACTIVEWINDOW function ID */
#define	APSH_InactiveWindow	(APSH_Dummy+232L)	/* INACTIVEWINDOW function ID */
#define	APSH_IntuiTicks		(APSH_Dummy+233L)	/* INTUITICKS function */
#define	APSH_MouseMove		(APSH_Dummy+234L)	/* MOUSEMOVE function */
#define	APSH_WinBOpen		(APSH_Dummy+235L)	/* Call before opening window */
#define	APSH_WinAOpen		(APSH_Dummy+236L)	/* Call after opening window */
#define	APSH_WinBClose		(APSH_Dummy+237L)	/* Call before closing window */
#define	APSH_WinAClose		(APSH_Dummy+238L)	/* Call after closing window */
#define	APSH_RefreshData	(APSH_Dummy+239L)	/* Refresh gadget data */
#define	APSH_MenuHelp		(APSH_Dummy+240L)	/* Menu Help */
#define	APSH_IDCMPUpdate	(APSH_Dummy+241L)	/* IDCMP Update */
#define	APSH_ChangeWindow	(APSH_Dummy+242L)	/* ChangeWindow size */
#define	APSH_VanillaKey		(APSH_Dummy+243L)	/* VanillaKey */

/* Real or simulated IntuiMessage fields */
#define	APSH_MsgClass		(APSH_Dummy+260L)	/* message class */
#define	APSH_MsgCode		(APSH_Dummy+261L)	/* message code */
#define	APSH_MsgQualifier	(APSH_Dummy+262L)	/* message qualifier */
#define	APSH_MsgIAddress	(APSH_Dummy+263L)	/* item address */
#define	APSH_MsgMouseX		(APSH_Dummy+264L)	/* mouse X coordinate */
#define	APSH_MsgMouseY		(APSH_Dummy+265L)	/* mouse Y coordinate */
#define	APSH_MsgSeconds		(APSH_Dummy+266L)	/* seconds */
#define	APSH_MsgMicros		(APSH_Dummy+267L)	/* micros */
#define	APSH_MsgWindow		(APSH_Dummy+268L)	/* window for event */
#define	APSH_IntuiMessage	(APSH_Dummy+269L)	/* Complete IntuiMessage */

/* SIPC message */
#define	APSH_SIPCData		(APSH_Dummy+300L)	/* Pointer the data passed by a SIPC message */
#define	APSH_SIPCDataLength	(APSH_Dummy+301L)	/* Length of the SIPC data */
#define APSH_AlreadyRunning	(APSH_Dummy+302L)	/* Function to execute if SIPC already exists */
#define	APSH_SIPCError		(APSH_Dummy+303L)	/* SIPC command ERROR function ID */
#define	APSH_SIPCOK		(APSH_Dummy+304L)	/* SIPC command OK function ID */

/* Tool information */
#define	APSH_Tool		(APSH_Dummy+320L)	/* Name of tool */
#define	APSH_ToolAddr		(APSH_Dummy+321L)	/* Address of tool */
#define	APSH_ToolData		(APSH_Dummy+322L)	/* Data for tool */
#define	APSH_ToolStack		(APSH_Dummy+323L)	/* Stack requirements of tool */
#define	APSH_ToolPri		(APSH_Dummy+324L)	/* Priority of tool */

/* Workbench tags */
#define	APSH_AppWindowEnv	(APSH_Dummy+400L)	/* AppWindow information */
#define	APSH_AppIconEnv		(APSH_Dummy+401L)	/* AppIcon information */
#define	APSH_AppMenuEnv		(APSH_Dummy+402L)	/* AppMenuItem information */
#define	APSH_WBArg		(APSH_Dummy+420L)	/* pointer to WBArg */

/* Workbench tags for function ID's */
#define	APSH_AppOpen		(APSH_Dummy+403L)	/* After App... is added */
#define	APSH_AppBDrop		(APSH_Dummy+404L)	/* Before icons are processed */
#define	APSH_AppDDrop		(APSH_Dummy+405L)	/* For each icon in the list */
#define	APSH_AppADrop		(APSH_Dummy+406L)	/* After icons added to project list */
#define	APSH_AppClose		(APSH_Dummy+407L)	/* Before App... closed */
#define	APSH_AppRemove		(APSH_Dummy+408L)	/* Before App... deleted */
#define	APSH_AppDblClick	(APSH_Dummy+409L)	/* When icon double-clicked */

#define	APSH_NEXT_TAG		(APSH_Dummy+500L)	/* remember... */


/*------------------------------------------------------------------------*/
/* The AppShell uses the following tags to set/get values                 */
/*------------------------------------------------------------------------*/

/* AppShell object attributes start from here */
#define	APSH_Attrs		TAG_USER+25000L

/* AppInfo attributes.  These values are only valid while a command
 * is outstanding (hasn't returned control back to the AppShell dispatcher).
 */
#define	APSH_TextRtn		(APSH_Attrs+1L)  /* Text return value (SG) */
#define	APSH_PrimaryReturn	(APSH_Attrs+2L)  /* Primary return value (SG) */
#define	APSH_SecondaryReturn	(APSH_Attrs+3L)  /* Secondary return value (SG) */
#define	APSH_Done		(APSH_Attrs+4L)  /* Done yet? (SG) */
#define	APSH_Startup		(APSH_Attrs+5L)  /* Startup flags (G) */
#define	APSH_Options		(APSH_Attrs+6L)  /* ReadArgs option buckets (G) */
#define	APSH_ArgsPtr		(APSH_Attrs+8L)  /* ReadArgs pointer (G) */
#define	APSH_FileArray		(APSH_Attrs+9L)  /* Multi-arg array from 0 bucket (G) */
#define	APSH_ToolIcon		(APSH_Attrs+10L) /* Pointer to application tool icon (G) */
#define	APSH_ProgramDir		(APSH_Attrs+11L) /* Base directory for application (G) */
#define	APSH_PrefsDir		(APSH_Attrs+12L) /* Application preference directory (G) */
#define	APSH_PrefsList		(APSH_Attrs+13L) /* List of preference records (G) */
#define	APSH_ProgName		(APSH_Attrs+15L) /* Executable name (G) */
#define	APSH_ScreenName		(APSH_Attrs+23L) /* Public screen name (G) */
#define	APSH_ScreenPtr		(APSH_Attrs+24L) /* Screen pointer (G) */
#define	APSH_Font		(APSH_Attrs+25L) /* TextFont used (G) */
#define	APSH_ActvWindow		(APSH_Attrs+26L) /* Active window (G) */
#define	APSH_ActvGadget		(APSH_Attrs+27L) /* Active gadget (G) */
#define	APSH_ActvObject		(APSH_Attrs+28L) /* Active object (G) */
#define	APSH_DrawInfo		(APSH_Attrs+29L) /* DrawInfo for screen (G) */
#define	APSH_VisualInfo		(APSH_Attrs+30L) /* VisualInfo for screen (G) */
#define	APSH_MouseX		(APSH_Attrs+31L) /* Last mouse x value (G) */
#define	APSH_MouseY		(APSH_Attrs+32L) /* Last mouse y value (G) */
#define	APSH_NumCommands	(APSH_Attrs+33L) /* Number of outstanding cmds (G) */
#define	APSH_Process		(APSH_Attrs+35L) /* Process pointer (G) */
#define	APSH_FailAt		(APSH_Attrs+36L) /* Failure level (SG) */
#define	APSH_ActvMH		(APSH_Attrs+37L) /* Where did current msg orig. (G) */
#define	APSH_ActvMessage	(APSH_Attrs+38L) /* Current msg (G) */
#define	APSH_LastError		(APSH_Attrs+39L) /* Last error number (G) */
#define	APSH_LastText		(APSH_Attrs+40L) /* Last text message (G) */
#define	APSH_ARexxMH		(APSH_Attrs+80L) /* ARexx msg. handler (G) */
#define	APSH_CommandShellMH	(APSH_Attrs+81L) /* Command Shell msg. handler (G) */
#define	APSH_IntuitionMH	(APSH_Attrs+82L) /* Intuition msg. handler (G) */
#define	APSH_SIPCMH		(APSH_Attrs+83L) /* SIPC msg. handler (G) */
#define	APSH_ToolMH		(APSH_Attrs+84L) /* Tool msg. handler (G) */
#define	APSH_WorkbenchMH	(APSH_Attrs+85L) /* Workbench msg. handler (G) */

#endif /* LIBRARIES_APSHATTRS_H */
