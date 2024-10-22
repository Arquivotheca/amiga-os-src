	ifnd LIBRARIES_APPSHELL_I
LIBRARIES_APPSHELL_I	EQU	1

************************************************************************
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
* appshell.i
* Copyright (C) 1990 Commodore-Amiga, Inc
* written by David N. Junod
*
* Translated for Assembly by James Nelson, INOVAtronics.
*
* header file for the Amiga AppShell
*
*

	include "exec/types.i"
	include "exec/nodes.i"
	include "exec/lists.i"
	include "exec/semaphores.i"
	include "intuition/intuition.i"
	include "intuition/screens.i"
	include "dos/dos.i"
	include "dos/dosextens.i"
	include "dos/rdargs.i"
	include "dos/dosasl.i"

	ifnd LIBRARIES_APPOBJECTS_I
	include "libraries/appobjects.i"
	endc

;ARRPTR	MACRO	;name,size	;To remind ASM programmers that the structure
;	APTR	\1		;must have memory allocated.
;	ENDM			;Though not used here, size is the number of
				;bytes needing to be allocated.

;Maximum number of arguments in a command string.  Don't bother changing.

MAXARG	EQU	64

; global object flags

	BITDEF	APSH,OPEN,0		;object is open/available
	BITDEF	APSH,DISABLED,1		;disabled from use
	BITDEF	APSH,PRIVATE,2		;can't be called from command line
	BITDEF	APSH,ALIAS,3		;aliased object
	BITDEF	APSH,LOCKON,4		;object can't be disabled

; The Funcs structure contains base information for a command.  This
; information gets translated into a Function Table entry and gets appended
; to the Function Table list. */

	STRUCTURE Funcs,0
	APTR	fe_Name			;Name of function
	APTR	fe_Func			;AppInfo, STRPTR, TagItem;
	ULONG	fe_ID			;ID of function
	APTR	fe_Template		;Command template
	ULONG	fe_NumOpts		;Number of options
	ULONG	fe_Flags		;Status of function
	ULONG	fe_HelpID	;index into the the text catalogue for help
	APTR	fe_Params		;optional parameters for function
	APTR	fe_GroupID		;~0 terminated array of group ID's
	APTR	fe_Options		;ReadOnly! ReadArgs
	LABEL	fe_SIZEOF

;no internal function defined for event

NO_FUNCTION	EQU	0

; The BasePrefs structure contains preference information that all AppShell
; applications honor.  Envision a preference editor and a global preference
; file to contain this information.

	STRUCTURE BasePrefs,0
	ULONG	bp_Flags		;misc. preference flags
	LABEL	bp_SIZEOF

; Set this flag to enable saving of icons with files. This feature is
; not implemented.

	BITDEF	APSH,USEICONS,1
	BITDEF	BP,USEICONS,1		;ASHL flag

; The AppShell defaults to using a console window for the Command Shell.
; By setting the following flag, the AppShell will use a Scrolling List
; gadget to show history and use a Text gadget for command entry.
; This feature is not implemented.

	BITDEF	APSH,LISTVIEW,2
	BITDEF	BP,LISTVIEW,2		;use listview & text gadget

; The Project structure is used to maintain information on a project set.
; Project sets are not limited to the main project list, but also include
; such things as lists of components for a project.

	STRUCTURE Project,0
	STRUCT	p_ProjList,LH_SIZE		;Project list
	APTR	p_CurProj			;Current project (ProjNode)
	LONG	p_NumProjs			;Number of projects
	LONG	p_MaxID				;Next available ID
	LONG	p_State				;Listview state
	LONG	p_TopView			;Listview top line
	LONG	p_CurLine			;Listview current line
	ULONG	p_Flags				;Project flags
	APTR	p_UserData			;User data extension
	APTR	p_SysData			;System data extension
	LABEL	p_SIZEOF

; When this flag is set, the project information is being displayed by
; the Project List data entry requester

	BITDEF	APSH,PROJVIEW,1
	BITDEF	P,PROJVIEW,1

; Within the Project structure is an Exec list called p_ProjList.  This
; is a list of ProjNode structures.  This structure contains information
; on projects or project components.  This information can be obtained at
; startup time, from AppWindow/AppIcon messages, ASL file requester or
; through application constructs. */

	STRUCTURE ProjNode,0
	STRUCT	pn_Node,LN_SIZE		;embedded Exec node

	; AppShell information.  Read only for application

	STRUCT	pn_Added,ds_SIZEOF	;date stamp when added to list
	BPTR	pn_ProjDir		;lock on project directory
	APTR	pn_ProjPath	;pointer to the projects' complete name
	APTR	pn_ProjName	;pointer to the projects' name
	APTR	pn_ProjComment	;pointer to the projects' comment
	APTR	pn_DObj		;pointer to the projects' icon (DiskObj)
	LONG	pn_ID		;user selected order
	APTR	pn_SysData	;System data extension

	;Application information */

	ULONG	pn_Status	;status of project
	ULONG	pn_ProjID	;project ID

	ARRPTR	pn_Name,32	;project name, only 32 bytes

	APTR	pn_UserData	;UserData for project
	BOOL	pn_Changed	;has project been modified?
	LABEL	pn_SIZEOF


; AppInfo structure that contains an AppShell application's global
; variables.  This structure is variable length, so NEVER embedd this
; within your own structures.  If you must, then reference it by a
; pointer.
	ifnd	AppInfo
	STRUCTURE AppInfo,0

	; control information

	APTR	appi_TextRtn		;Text return string
	LONG	appi_Pri_Ret		;Primary error (severity)
	LONG	appi_Sec_Ret		;Secondary error (actual)
	BOOL	appi_Done			;Done with main loop?

	; startup arguments

	UWORD	appi_Startup		;see defines below
	APTR	appi_Options		;Option bucket
	LONG	appi_NumOpts		;Number of options
    	APTR	appi_ArgsPtr		;ReadArgs pointer
	APTR	appi_FileArray		;MultiArg array (from the 0 bucket)
				; If the template specifies FILES/M, then
				; the array is expanded using pattern
				; matching and the resulting files are
				; added to the project list
	APTR	appi_ProgDO		;Tool icon (DiskObject)

	; base application information

	APTR	appi_BasePrefs		;base user preferences (BasePrefs)
	ULONG	appi_PrefSize		;sizeof (struct BasePrefs)
	BPTR	appi_ProgDir		;base directory for application
	BPTR	appi_ConfigDir		;configuration file directory
	STRUCT	appi_PrefList,LH_SIZE	;list of preference files.
	APTR	appi_BaseName		;pointer to application base name
	APTR	appi_ProgName		;pointer to application program name
	APTR	appi_AppName		;pointer to application name
	APTR	appi_AppVersion		;pointer to version string
	APTR	appi_AppCopyright		;pointer to copyright notice
	APTR	appi_AppAuthor		;pointer to author
	APTR	appi_AppMsgTitle		;pointer to title for messages

	; project information

    	STRUCT	appi_Project,p_SIZEOF		;embedded Project structure

	; application information

	APTR appi_UserData			;UserData

	; READ ONLY Intuition-specific information

	APTR	appi_ScreenName			:pointer to public screen name
	APTR	appi_Screen			;Active screen
	APTR	appi_Font			;Font for screen (TextFont)
	APTR	appi_Window			;Active window
	APTR	appi_Gadget			;Active gadget
	APTR	appi_CurObj			;Active MHObject
	APTR	appi_DI				;Intuition DrawInfo
	APTR	appi_VI				;GadTools VisualInfo
	WORD	appi_MouseX		;Position at last IDCMP message
	WORD	appi_MouseY		;Position at last IDCMP message
	UWORD	appi_TopLine			;top line

;------------------------------------------------------------------------
; The following fields are not for public consumption
;------------------------------------------------------------------------

	LABEL	appi_SIZEOF
	endc

;appi_Startup flags

	BITDEF	APSH,START_WB,0		;if set then WB, else Shell
	BITDEF	APSH,START_CLONE,1

	BITDEF	AI,START_WB,0
	BITDEF	AI,START_CLONE,1

;This STRUCTure is used to search through a Exec linked list using
;DOS pattern matching

	STRUCTURE AnchorList,0
	STRUCT	al_CurNode,LN_SIZE
	STRUCT	al_NxtNode,LN_SIZE
	APTR	al_Token
	ULONG	al_Flags
	LABEL	al_SIZEOF

;--------------------------------------------------------------------------
;Following is information used by the main portion of the AppShell
;--------------------------------------------------------------------------

;Each message handler gets a base ID from which all of their commands are
; offset.  The main functions are offset from zero.

APSH_MAIN_ID	EQU	0

;Following are ID's for the functions that are implemented by the AppShell
;whether there are any message handlers or not.

MAIN_Dummy	EQU	APSH_MAIN_ID
AliasID		EQU	(MAIN_Dummy+1)	;set up function w/parameters
DebugID		EQU	(MAIN_Dummy+2)	;general debugging
DisableID	EQU	(MAIN_Dummy+3)	;Disable a function
EditID		EQU	(MAIN_Dummy+4)	;Edit an object
EnableID	EQU	(MAIN_Dummy+5)	;Enable a function
ExecMacroID	EQU	(MAIN_Dummy+6)	;Execute the internal macro
FaultID		EQU	(MAIN_Dummy+7)	;Return error text
GetID		EQU	(MAIN_Dummy+8)	;Get object attribute
GroupID		EQU	(MAIN_Dummy+9)	;Maintain object groups
HelpID		EQU	(MAIN_Dummy+10)	;Help
LearnID		EQU	(MAIN_Dummy+11)	;Learn macro function
StopLearnID	EQU	(MAIN_Dummy+12)	;Stop learn macro function
LoadMacroID	EQU	(MAIN_Dummy+13)	;Load a macro into the internal memory
PriorityID	EQU	(MAIN_Dummy+14)	;Set an objects priority (process)
RemoveID	EQU	(MAIN_Dummy+15)	;remove an object (project)
SaveMacroID	EQU	(MAIN_Dummy+16)	;Save the internal macro to disk
SelectID	EQU	(MAIN_Dummy+17)	;select an object (project)
SelectTopID	EQU	(MAIN_Dummy+18)	;select first object (project)
SelectNextID	EQU	(MAIN_Dummy+19)	;select next object (project)
SelectPrevID	EQU	(MAIN_Dummy+20)	;select previous object (project)
SelectBottomID	EQU	(MAIN_Dummy+21)	;select last object (project)
SetID		EQU	(MAIN_Dummy+22)	;Set object attributes
StatusID	EQU	(MAIN_Dummy+23)	;Give status of an object
StopID		EQU	(MAIN_Dummy+24)	;Stop an operation
StubID		EQU	(MAIN_Dummy+25)	;NOP function
VersionID	EQU	(MAIN_Dummy+26)	;Version

;data transmission function ID's via OpenSIPC, SIPCPrintf, CloseSIPC

LoginID		EQU	(MAIN_Dummy+100)	;request data transmission session
DataStreamID	EQU	(MAIN_Dummy+101)	;data transmission
SuspendID	EQU	(MAIN_Dummy+102)	;temporarily suspend session
ResumeID	EQU	(MAIN_Dummy+103)	;resume data session
LogoutID	EQU	(MAIN_Dummy+104)	;signal end of session

;Following are ID's for functions that should be implemented by the
;application to help promote a consistent set of functions for the
;end user.

;standard project functions

NewID		EQU	(MAIN_Dummy+500)	;new project/process
ClearID		EQU	(MAIN_Dummy+501)	;clear current project
OpenID		EQU	(MAIN_Dummy+502)	;open an existing project
SaveID		EQU	(MAIN_Dummy+503)	;save project to existing name
SaveAsID	EQU	(MAIN_Dummy+504)	;save project to a new name
RevertID	EQU	(MAIN_Dummy+505)	;revert project to last saved
PrintID		EQU	(MAIN_Dummy+506)	;print the current project
PrintAsID	EQU	(MAIN_Dummy+507)	;define print configuration
AboutID		EQU	(MAIN_Dummy+508)	;display application information
InfoID		EQU	(MAIN_Dummy+509)	;display project information
QuitID		EQU	(MAIN_Dummy+510)	;exit from the application
HideID		EQU	(MAIN_Dummy+511)	;Hide the project
CloseID		EQU	(MAIN_Dummy+512)	;Close the project
DropIconID	EQU	(MAIN_Dummy+513)	;Icon dropped in window

;application standard edit functions

MarkID		EQU	(MAIN_Dummy+520)
CutID		EQU	(MAIN_Dummy+521)
CopyID		EQU	(MAIN_Dummy+522)
PasteID		EQU	(MAIN_Dummy+523)
EraseID		EQU	(MAIN_Dummy+524)
UndoID		EQU	(MAIN_Dummy+525)
OpenClipID	EQU	(MAIN_Dummy+526)
SaveClipID	EQU	(MAIN_Dummy+527)
PrintClipID	EQU	(MAIN_Dummy+528)

;application standard search functions

GotoID		EQU	(MAIN_Dummy+540)
FindID		EQU	(MAIN_Dummy+541)
NextID		EQU	(MAIN_Dummy+542)
ReplaceID	EQU	(MAIN_Dummy+543)


;--------------------------------------------------------------------------
;Following is information used by the ARexx message handler
;--------------------------------------------------------------------------

;ID assigned to the ARexx message handler

APSH_AREXX_ID 	EQU 5000

;Following are ID's for the functions that are implemented by the ARexx
;message handler.

AREXX_Dummy	EQU APSH_AREXX_ID
RXID		EQU (AREXX_Dummy+1)	;Execute an ARexx command
WhyID		EQU (AREXX_Dummy+2)	;Return information on the last error

;ID for an ARexx low-level message handler function

AH_SENDCMD 	EQU 4

;--------------------------------------------------------------------------
;Following is information used by the Command Shell message handler
;--------------------------------------------------------------------------

APSH_DOS_ID 	EQU 6000
CMDShellID	EQU (APSH_DOS_ID+1)	;Activate the Command Shell

;--------------------------------------------------------------------------
;Following is information used by the Intuition message handler
;--------------------------------------------------------------------------

; Make a window definition without opening it
APSH_MH_MAKE	EQU	4

; Hide a window
APSH_MH_HIDE	EQU	5

APSH_IDCMP_ID 	EQU 7000
ActivateID	EQU (APSH_IDCMP_ID+1)	;Open GUI
ButtonID	EQU (APSH_IDCMP_ID+2)	;Edit gadget
KeystrokeID	EQU (APSH_IDCMP_ID+3)	;Edit HotKey
MenuID		EQU (APSH_IDCMP_ID+4)	;Edit menu
WindowToFrontID	EQU (APSH_IDCMP_ID+5)	;bring current window to front
WindowToBackID	EQU (APSH_IDCMP_ID+6)	;send current window to back
WindowID	EQU (APSH_IDCMP_ID+7)	;open/close window
DeActivateID	EQU (APSH_IDCMP_ID+8)	;Shutdown GUI
LockGUIID	EQU (APSH_IDCMP_ID+9)	;Lock the GUI
UnLockGUIID	EQU (APSH_IDCMP_ID+10)	;Unlock the GUI

;extended MenuItem STRUCTure

	STRUCTURE EMenuItem,0
	STRUCT	emi_Item,mi_SIZEOF	;embedded MenuItem STRUCTure
	ULONG	emi_MenuID		;ID used for function number to perform
	APTR	emi_UserData		;UserData (like other Intuition STRUCTs)
	LABEL	emi_SIZEOF


;--- KEYBOARD RELATED ITEMS ---

SPECIAL 	EQU	255

;some useful defines

;TAB      	EQU 9
RETURN		EQU 13
;ESC		EQU 27
DELETE		EQU 127
HELP		EQU (SPECIAL+'?')
FUNC1		EQU (SPECIAL+'0')
FUNC2		EQU (SPECIAL+'1')
FUNC3		EQU (SPECIAL+'2')
FUNC4		EQU (SPECIAL+'3')
FUNC5		EQU (SPECIAL+'4')
FUNC6		EQU (SPECIAL+'5')
FUNC7		EQU (SPECIAL+'6')
FUNC8		EQU (SPECIAL+'7')
FUNC9		EQU (SPECIAL+'8')
FUNC10		EQU (SPECIAL+'9')
UP		EQU (SPECIAL+'A')
DOWN		EQU (SPECIAL+'B')
RIGHT		EQU (SPECIAL+'C')
LEFT		EQU (SPECIAL+'D')

;component for our keyboard command array

	STRUCTURE KeyboardCMD,0
	ULONG	kbc_Key			;key
	ULONG	kbc_FuncID		;function ID
	LABEL	kbc_SIZEOF

;--------------------------------------------------------------------------
;Following is information used by the Simple IPC message handler
;--------------------------------------------------------------------------

APSH_SIPC_ID 	EQU 10000

;This STRUCTure is used by the AppShell to communicate with tools and
; other AppShell applications.
;
; If sipc_DType equal NULL, then the function ID in sipc_Type is performed
; with no arguments.
;
;    PerfFunc (ai, sipc_Type, NULL, NUL)	;
;
; If sipc_DType equal APSH_SDT_TagList, then the function ID in sipc_Type is
; performed with sipc_Data as the tag list arguments.
;
;    PerfFunc (ai, sipc_Type, NULL, sipc_Data)	;
;
; If sipc_DType equal APSH_SDT_Data, then the function ID in sipc_Type is
; performed with with the following tags as arguments:
;
;    APSH_SIPCData,       sipc_Data
;    APSH_SIPCDataLength, sipc_DSize
;
; If sipc_DType equal APSH_SDT_Command, then the string command line
; passed in the sipc_Data field is performed:
;
;    PerfFunc (ai, NULL, sipc_Data, NUL)	;
;

	STRUCTURE SIPCMessage,0
	STRUCT	sipc_Msg,MN_SIZE	;Embedded Exec message STRUCTure
	ULONG	sipc_Type		;Type of message
	APTR	sipc_Data		;Pointer to message data
	ULONG	sipc_DSize		;Size of message data
	ULONG	sipc_DType		;Type of message data
	ULONG	sipc_Pri_Ret		;Primary return value
	ULONG	sipc_Sec_Ret		;Secondary return value
	APTR	sipc_Extens1;		;*** PRIVATE *** SYSTEM USE ONLY!
	APTR	sipc_Extens2;		;*** PRIVATE *** SYSTEM USE ONLY!
	LABEL	sipc_SIZEOF

;These flags are used in the sipc_DType field to indicate what type of
;information is in the sipc_Data field.

APSH_SDT_Command	EQU  1<<1	;Data is a STRPTR
APSH_SDT_TagList	EQU  1<<2	;Data is a list of TagItem's
APSH_SDT_Data		EQU  1<<3	;Data is a pointer to a data block
APSH_SDT_Text		EQU  1<<4	;text transmissions via sprintf

;Public SIPC port name given to the AppShell remote debugger.  Accessed
;using OpenSIPC, SIPCPrintf and CloseSIPC.

DEBUGGERNAME MACRO
DEBUGGER 	dc.b	"AppShell_Debugger",0
	     ENDM

;--------------------------------------------------------------------------
;Following is information used by the Tool message handler
;--------------------------------------------------------------------------

APSH_TOOL_ID 		EQU 11000
StartupMsgID		EQU (APSH_TOOL_ID+1)	;Startup message
LoginToolID		EQU (APSH_TOOL_ID+2)	;Login a tool SIPC port
LogoutToolID		EQU (APSH_TOOL_ID+3)	;Logout a tool SIPC port
ShutdownMsgID		EQU (APSH_TOOL_ID+4)	;Shutdown message
ActivateToolID		EQU (APSH_TOOL_ID+5)	;Activate tool
DeactivateToolID 	EQU (APSH_TOOL_ID+6)	;Deactivate tool
ActiveToolID		EQU (APSH_TOOL_ID+7)	;Tool Active
InactiveToolID		EQU (APSH_TOOL_ID+8)	;Tool Inactive
ToolStatusID		EQU (APSH_TOOL_ID+9)	;Status message
ToolCmdID		EQU (APSH_TOOL_ID+10)	;Tool command message
ToolCmdReplyID		EQU (APSH_TOOL_ID+11)	;Reply to tool command
ShutdownToolID		EQU (APSH_TOOL_ID+12)	;Shutdown tool

;typedef VOID (*F_PTR)(APTR, STRUCT MsgPort *)	;

TOOL_ACTIVATE		EQU (1<<1)

;STRUCTure for tool table entry

	STRUCTURE Tools,0
	STRUCT	tools_Node,LN_SIZE		;Node for tool entry
	APTR	tools_Func			;Address of function
	ULONG	tools_ID			;ID of function
	ULONG	tools_Flags		;Status of function
	ULONG	tools_HitCnt		;Access count
	ULONG	tools_Stack		;Stack requirements for function
	ULONG	tools_Pri			;Default priority for function
	ULONG	tools_UseCnt		;Current use count of function
	ULONG	tools_MaxCnt		;Maximum instances of function
	APTR	tools_Port			;Port name of owner
	LABEL	tools_SIZEOF

;--------------------------------------------------------------------------
;Following is information used by the Workbench message handler
;--------------------------------------------------------------------------

APSH_WB_ID		EQU 12000

;APSH_CmdFlags

	BITDEF	APSH_WB,DISPLAY,1	;maintain display box for icon
	BITDEF	APSH_WB,PROJLIST,2	;add the WBArgs to the project list
	BITDEF	APSH_WB,NOLIST,3	;don't add the WBArgs to a list

;--------------------------------------------------------------------------
;Following is information for use by the Application
;--------------------------------------------------------------------------

;base tag for application functions
APSH_USER_ID 		EQU 100000


;--------------------------------------------------------------------------
;Following is low-level message handler information
;--------------------------------------------------------------------------

;message handler object node

	STRUCTURE MHObject,0
	STRUCT	mho_Node,LN_SIZE	;embedded Exec node
	STRUCT	mho_ObjList,LH_SIZE	;embedded List of children objects
	APTR	mho_Parent		;pointer to parent object (MHObject)
	APTR	mho_CurNode		;pointer to current child object (MHObject)
	ULONG	mho_ID			;numeric ID of object
	ULONG	mho_Status		;status of object
	APTR	mho_SysData		;message handler data
	APTR	mho_UserData		;application data
	APTR	mho_Extens1;		;*** PRIVATE ***

	ARRPTR	mho_Name,1		;name of object

	LABEL	mho_SIZEOF

;message handler node

	STRUCTURE MsgHandler,0
	STRUCT	mh_Header,mho_SIZEOF	;embedded MHObject STRUCTure
	APTR	mh_Port			;message port for handler (MsgPort)
	APTR	mh_PortName		;port name, if public
	ULONG	mh_SigBits		;signal bits to watch for

;handler functions

	WORD	mh_NumFuncs		;number of functions in handler
	APTR	mh_Func			;bool = ai,mh,ti
	APTR	mh_DefText		;Default text catalogue
	APTR	mh_Catalogue		;*** PRIVATE ***

	APTR	mh_Extens1		;*** PRIVATE ***
	APTR	mh_Extens2		;*** PRIVATE ***
	LABEL	mh_SIZEOF

;--- interface function array pointers ---

APSH_MH_OPEN		EQU 0	;make a message handler active
APSH_MH_HANDLE		EQU 1	;handle messages
APSH_MH_CLOSE		EQU 2	;make a message handler inactive
APSH_MH_SHUTDOWN	EQU 3	;free resources and delete message handler

;--- node types ---

APSH_MH_HANDLER_T	EQU 100	;message handler node
APSH_MH_DATA_T		EQU 101	;data node

;--- message handler object types

APSH_MHO_WINDOW		EQU 110	;Intuition window
APSH_MHO_INTOBJ		EQU 111	;AppShell Intuition object
APSH_MHO_TOOL		EQU 120	;Tool

;--- node priorities ---

APSH_MH_HANDLER_P	EQU 10	;message handler node default priority
APSH_MH_DATA_P		EQU -10	;data node default priority

;--- overall status ---

APSHP_INACTIVE		EQU (1<<1)
APSHP_SINGLE		EQU (1<<2)
APSH_REQUIRED		EQU TRUE
APSH_OPTIONAL		EQU FALSE

;--------------------------------------------------------------------------
;The AppShell uses the following tags.  Reserved TAG_USER+24000L - 25999L
;--------------------------------------------------------------------------

;Tags

APSH_Dummy		EQU TAG_USER+24000

;library management

APSH_OpenLibraries	EQU (APSH_Dummy+1)	;open libraries
APSH_LibNameTag		EQU (APSH_Dummy+2)	;library name tag
APSH_LibName		EQU (APSH_Dummy+3)	;library name
APSH_LibVersion		EQU (APSH_Dummy+4)	;library version
APSH_LibStatus		EQU (APSH_Dummy+5)	;required/optional
APSH_LibReserved5	EQU (APSH_Dummy+10)	;RESERVED FOR SYSTEM USE
APSH_LibBase		EQU (APSH_Dummy+11)	;library base
APSH_ARexxSys		EQU (APSH_Dummy+12)	;rexxsyslib.library
APSH_ARexxSup		EQU (APSH_Dummy+13)	;rexxsupport.library
APSH_ASL		EQU (APSH_Dummy+14)	;asl.library
APSH_Commodities	EQU (APSH_Dummy+15)	;commodities.library
APSH_DiskFont		EQU (APSH_Dummy+16)	;diskfont.library
APSH_DOS		EQU (APSH_Dummy+17)	;dos.library
APSH_GadTools		EQU (APSH_Dummy+18)	;gadtools.library
APSH_Gfx		EQU (APSH_Dummy+19)	;graphics.library
APSH_Icon		EQU (APSH_Dummy+20)	;icon.library
APSH_Intuition		EQU (APSH_Dummy+21)	;intuition.library
APSH_Layers		EQU (APSH_Dummy+22)	;layers.library
APSH_IFF		EQU (APSH_Dummy+23)	;iffparse.library
APSH_Translate		EQU (APSH_Dummy+24)	;translator.library
APSH_Utility		EQU (APSH_Dummy+25)	;utility.library
APSH_Workbench		EQU (APSH_Dummy+26)	;workbench.library
APSH_AppObjects		EQU (APSH_Dummy+27)	;appobjects.library
APSH_Hyper		EQU (APSH_Dummy+28)	;hyper.library
APSH_Prefs		EQU (APSH_Dummy+29)	;prefs.library

;main AppShell tags

APSH_NumArgs		EQU (APSH_Dummy+40)	;Number of Shell arguments
APSH_ArgList		EQU (APSH_Dummy+41)	;Shell arguments
APSH_WBStartup		EQU (APSH_Dummy+42)	;Workbench arguments
APSH_ControlPort	EQU (APSH_Dummy+43)	;SIPC Control port for a cloned AppShell
APSH_AppName		EQU (APSH_Dummy+44)	;pointer to the application's name
APSH_AppVersion		EQU (APSH_Dummy+45)	;pointer to the application's version
APSH_AppCopyright	EQU (APSH_Dummy+46)	;pointer to the application's (c) notice
APSH_AppAuthor		EQU (APSH_Dummy+47)	;pointer to the application's author
APSH_AppMsgTitle	EQU (APSH_Dummy+48)	;pointer to message title
APSH_FuncTable		EQU (APSH_Dummy+55)	;function table for application
APSH_DefText		EQU (APSH_Dummy+56)	;Default text catalogue
APSH_AppInit		EQU (APSH_Dummy+57)	;Custom application init function ID
APSH_AppExit		EQU (APSH_Dummy+58)	;Custom application shutdown function ID
APSH_SIG_C		EQU (APSH_Dummy+59)	;SIG_BREAK_C function ID
APSH_SIG_D		EQU (APSH_Dummy+60)	;SIG_BREAK_D function ID
APSH_SIG_E		EQU (APSH_Dummy+61)	;SIG_BREAK_E function ID
APSH_SIG_F		EQU (APSH_Dummy+62)	;SIG_BREAK_F function ID
APSH_ProjInfo		EQU (APSH_Dummy+63)	;pointer to a Project STRUCTure
APSH_BaseName		EQU (APSH_Dummy+64)	;Base name
APSH_Template		EQU (APSH_Dummy+65)	;Startup template
APSH_NumOpts		EQU (APSH_Dummy+66)	;Number of options
APSH_FuncEntry		EQU (APSH_Dummy+67)	;Funcs STRUCTure for command
APSH_UserData		EQU (APSH_Dummy+68)	;Preallocated user data
APSH_ProjIcon		EQU (APSH_Dummy+69)	;Default project icon
APSH_ProjIconName	EQU (APSH_Dummy+70)	;Default project icon name
APSH_MsgHandler		EQU (APSH_Dummy+71)	;Message handler pointer
APSH_UserDataSize	EQU (APSH_Dummy+72)	;Size of data
APSH_PortAddr		EQU (APSH_Dummy+73)
APSH_AppHandle		EQU (APSH_Dummy+74)

;message handler routines

APSH_AddHandler		EQU (APSH_Dummy+80)	;add a message handler to application
APSH_Setup		EQU (APSH_Dummy+81)	;setup function
APSH_Status		EQU (APSH_Dummy+82)	;active, inactive, multiple, etc...
APSH_Rating		EQU (APSH_Dummy+83)	;optional/required, etc...
APSH_Port		EQU (APSH_Dummy+84)	;name of the message port
APSH_Handler		EQU (APSH_Dummy+85)	;Handler ID
APSH_CmdData		EQU (APSH_Dummy+86)	;Command data
APSH_CmdDataLength	EQU (APSH_Dummy+87)	;Length of command data
APSH_CmdID		EQU (APSH_Dummy+88)	;Command ID (function)
APSH_CmdString		EQU (APSH_Dummy+89)	;Command string
APSH_CmdTagList		EQU (APSH_Dummy+90)	;Command tag list
APSH_Command		EQU (APSH_Dummy+91)	;Handler command
APSH_NameTag		EQU (APSH_Dummy+92)	;Name Tag for object
APSH_CmdFlags		EQU (APSH_Dummy+93)	;Command Flags
APSH_TextID		EQU (APSH_Dummy+94)	;Text ID
APSH_BaseID		EQU (APSH_Dummy+95)	;Base ID

APSH_AddARexx_UI	EQU (APSH_Dummy+96)	;ARexx UI
APSH_AddCmdShell_UI	EQU (APSH_Dummy+97)	;Command Shell UI
APSH_AddIntui_UI	EQU (APSH_Dummy+98)	;Graphical UI
APSH_AddSIPC_UI		EQU (APSH_Dummy+99)	;Simple IPC UI
APSH_AddTool_UI		EQU (APSH_Dummy+100)	;Tool UI
APSH_AddWB_UI		EQU (APSH_Dummy+101)	;Workbench UI
APSH_AddClone_UI	EQU (APSH_Dummy+102)	;PRIVATE

;ARexx information

APSH_Extens		EQU (APSH_Dummy+120)	;ARexx macro name extension
APSH_ARexxError		EQU (APSH_Dummy+121)	;ARexx command ERROR function ID
APSH_ARexxOK		EQU (APSH_Dummy+122)	;ARexx command OK function ID

;Command shell

APSH_CloseMsg		EQU (APSH_Dummy+140)	;Closing message
APSH_CMDWindow		EQU (APSH_Dummy+141)	;Command window spec
APSH_Prompt		EQU (APSH_Dummy+142)	;Command window prompt

;Window information

APSH_WindowEnv		EQU (APSH_Dummy+160)	;Window Environment
APSH_TextAttr		EQU (APSH_Dummy+161)	;Text Attributes
APSH_NewScreen		EQU (APSH_Dummy+162)	;NewScreen STRUCTure
APSH_NewScreenTags	EQU (APSH_Dummy+163)	;Tags for new screen
APSH_Palette		EQU (APSH_Dummy+164)	;Color Palette
APSH_NewWindow		EQU (APSH_Dummy+165)	;NewWindow STRUCTure
APSH_NewWindowTags	EQU (APSH_Dummy+166)	;Tags for new window
APSH_HotKeys		EQU (APSH_Dummy+167)	;HotKey command array
APSH_Menu		EQU (APSH_Dummy+168)	;Intuition-style Menu array
APSH_Gadgets		EQU (APSH_Dummy+169)	;Intuition-style Gadget array
APSH_GTMenu		EQU (APSH_Dummy+170)	;GadTools-style Menu array
APSH_GTGadgets		EQU (APSH_Dummy+171)	;GadTools-style NewGadget array
APSH_GTFlags		EQU (APSH_Dummy+172)	;flags for GadTools objects
APSH_Objects		EQU (APSH_Dummy+173)	;Object array
APSH_ObjDown		EQU (APSH_Dummy+174)	;Gadget downpress function ID
APSH_ObjHold		EQU (APSH_Dummy+175)	;Gadget hold function ID
APSH_ObjRelease		EQU (APSH_Dummy+176)	;Gadget release function ID
APSH_ObjDblClick	EQU (APSH_Dummy+177)	;Gadget double-click function ID
APSH_ObjAbort		EQU (APSH_Dummy+178)	;Gadget abort function ID
APSH_ObjAltHit		EQU (APSH_Dummy+179)	;Gadget ALT hit function ID
APSH_ObjShiftHit	EQU (APSH_Dummy+180)	;Gadget SHIFT hit function ID
APSH_ObjData		EQU (APSH_Dummy+181)	;Gadget image or data
APSH_ObjInner		EQU (APSH_Dummy+182)	;Inner rectangle
APSH_ObjPointer		EQU (APSH_Dummy+183)	;pointer name prefix
APSH_DefWinFlags	EQU (APSH_Dummy+184)	;Default window flags
APSH_ObjName		EQU (APSH_Dummy+185)	;Object name
APSH_WinName		EQU (APSH_Dummy+186)	;Window name
APSH_WinPointer		EQU (APSH_Dummy+188)	;Pointer to window
APSH_ShowSelected	EQU (APSH_Dummy+189)	;Name of txt object for list
APSH_Screen		EQU (APSH_Dummy+190)	;Screen Pointer
APSH_ObjExtraRelease	EQU (APSH_Dummy+191)	;Alternate button release
APSH_ObjAltData		EQU (APSH_Dummy+192)	;Alternate image
APSH_ParentWindow	EQU (APSH_Dummy+193)	;Parent window
APSH_TTMenu		EQU (APSH_Dummy+194)	;Index of menu in text table
APSH_WinText		EQU (APSH_Dummy+195)	;Window text catalog
APSH_ObjCreate		EQU (APSH_Dummy+196)	;After object is created
APSH_ObjDelete		EQU (APSH_Dummy+197)	;Before object is removed

;IDCMP messages

APSH_SizeVerify		EQU (APSH_Dummy+220)	;SIZEVERIFY function ID
APSH_NewSize		EQU (APSH_Dummy+221)	;NEWSIZE function ID
APSH_RefreshWindow	EQU (APSH_Dummy+222)	;REFRESHWINDOW function ID
APSH_MouseButtons	EQU (APSH_Dummy+223)	;MOUSEBUTTONS function ID
APSH_ReqSet		EQU (APSH_Dummy+224)	;REQSET function ID
APSH_CloseWindow	EQU (APSH_Dummy+225)	;CLOSEWINDOW  function ID
APSH_ReqVerify		EQU (APSH_Dummy+226)	;REQVERIFY function ID
APSH_ReqClear		EQU (APSH_Dummy+227)	;REQCLEAR function ID
APSH_MenuVerify		EQU (APSH_Dummy+228)	;MENUVERIFY function ID
APSH_DiskInserted	EQU (APSH_Dummy+229)	;DISKINSERTED function ID
APSH_DiskRemoved	EQU (APSH_Dummy+230)	;DISKREMOVED function ID
APSH_ActiveWindow	EQU (APSH_Dummy+231)	;ACTIVEWINDOW function ID
APSH_InactiveWindow	EQU (APSH_Dummy+232)	;INACTIVEWINDOW function ID
APSH_IntuiTicks		EQU (APSH_Dummy+233)	;INTUITICKS function
APSH_MouseMove		EQU (APSH_Dummy+234)	;Mouse move function
APSH_WinBOpen		EQU (APSH_Dummy+235)	;Call before opening window
APSH_WinAOpen		EQU (APSH_Dummy+236)	;Call after opening window
APSH_WinBClose		EQU (APSH_Dummy+237)	;Call before closing window
APSH_WinAClose		EQU (APSH_Dummy+238)	;Call after closing window
APSH_RefreshData	EQU (APSH_Dummy+239)	;Refresh gadget data

;Real or simulated IntuiMessage fields

APSH_MsgClass		EQU (APSH_Dummy+260)	;message class
APSH_MsgCode		EQU (APSH_Dummy+261)	;message code
APSH_MsgQualifier	EQU (APSH_Dummy+262)	;message qualifier
APSH_MsgIAddress	EQU (APSH_Dummy+263)	;item address
APSH_MsgMouseX		EQU (APSH_Dummy+264)	;mouse X coordinate
APSH_MsgMouseY		EQU (APSH_Dummy+265)	;mouse Y coordinate
APSH_MsgSeconds		EQU (APSH_Dummy+266)	;seconds
APSH_MsgMicros		EQU (APSH_Dummy+267)	;micros
APSH_MsgWindow		EQU (APSH_Dummy+268)	;window for event
APSH_IntuiMessage	EQU (APSH_Dummy+269)	;Complete IntuiMessage

;SIPC message

APSH_SIPCData		EQU (APSH_Dummy+300)	;Pointer the data passed by a SIPC message
APSH_SIPCDataLength	EQU (APSH_Dummy+301)	;Length of the SIPC data

;Tool information

APSH_Tool		EQU (APSH_Dummy+320)	;Name of tool
APSH_ToolAddr		EQU (APSH_Dummy+321)	;Address of tool
APSH_ToolData		EQU (APSH_Dummy+322)	;Data for tool
APSH_ToolStack		EQU (APSH_Dummy+323)	;Stack requirements of tool
APSH_ToolPri		EQU (APSH_Dummy+324)	;Priority of tool

;Workbench tags

APSH_AppWindowEnv	EQU (APSH_Dummy+400)	;AppWindow information
APSH_AppIconEnv		EQU (APSH_Dummy+401)	;AppIcon information
APSH_AppMenuEnv		EQU (APSH_Dummy+402)	;AppMenuItem information
APSH_WBArg		EQU (APSH_Dummy+420)	;pointer to WBArg

;Workbench tags for function ID's

APSH_AppOpen		EQU (APSH_Dummy+403)	;After App... is added
APSH_AppBDrop		EQU (APSH_Dummy+404)	;Before icons are processed
APSH_AppDDrop		EQU (APSH_Dummy+405)	;For each icon in the list
APSH_AppADrop		EQU (APSH_Dummy+406)	;After icons added to project list
APSH_AppClose		EQU (APSH_Dummy+407)	;Before App... closed
APSH_AppRemove		EQU (APSH_Dummy+408)	;Before App... deleted
APSH_AppDblClick	EQU (APSH_Dummy+409)	;When icon double-clicked

APSH_NEXT_TAG		EQU (APSH_Dummy+500)	;remember...

;--------------------------------------------------------------------------
;Following are ID's to use to access the AppShell text table
;--------------------------------------------------------------------------

APSH_PAD		EQU 0
APSH_NOT_AN_ICON	EQU (APSH_PAD+1)	;%s is not an icon.
APSH_NOT_AVAILABLE	EQU (APSH_PAD+2)	;%s is not available
APSH_PORT_ACTIVE	EQU (APSH_PAD+3)	;%s port already active
APSH_PORT_X_ACTIVE	EQU (APSH_PAD+4)	;port, %s, already active
APSH_NOT_AN_IFF		EQU (APSH_PAD+5)	;%s is not an IFF file
APSH_NOT_AN_IFF_X	EQU (APSH_PAD+6)	;%1$s is not an IFF %2$s file
APSH_CLOSE_ALL_WINDOWS	EQU (APSH_PAD+7)	;Close all windows
APSH_CMDSHELL_PROMPT	EQU (APSH_PAD+8)	;Cmd>
APSH_CLDNT_CREATE_X	EQU (APSH_PAD+9)	;Could not create %s
APSH_CLDNT_CREATE_PORT	EQU (APSH_PAD+10)	;Could not create port, %s
APSH_CLDNT_CREATE_OBJ	EQU (APSH_PAD+11)	;Could not create object
APSH_CLDNT_CREATE_OBJ_X	EQU (APSH_PAD+12)	;Could not create object, %s
APSH_CLDNT_CREATE_FILE	EQU (APSH_PAD+13)	;Could not create file
APSH_CLDNT_CREATE_FILE_X EQU (APSH_PAD+14)	;Could not create file, %s
APSH_CLDNT_INIT_X	EQU (APSH_PAD+15)	;Could not initialize %s
APSH_CLDNT_INIT_MSGH	EQU (APSH_PAD+16)	;Could not initialize %s message handler
APSH_CLDNT_LOCK		EQU (APSH_PAD+17)	;Could not lock %s
APSH_CLDNT_LOCK_DIR	EQU (APSH_PAD+18)	;Could not lock directory
APSH_CLDNT_LOCK_DIR_X	EQU (APSH_PAD+19)	;Could not lock directory, %s
APSH_CLDNT_LOCK_PUB	EQU (APSH_PAD+20)	;Could not lock public screen
APSH_CLDNT_LOCK_PUB_X	EQU (APSH_PAD+21)	;Could not lock public screen, %s
APSH_CLDNT_OBTAIN	EQU (APSH_PAD+22)	;Could not obtain %s
APSH_CLDNT_OPEN		EQU (APSH_PAD+23)	;Could not open %s
APSH_CLDNT_OPEN_FILE	EQU (APSH_PAD+24)	;Could not open file
APSH_CLDNT_OPEN_FILE_X	EQU (APSH_PAD+25)	;Could not open file, %s
APSH_CLDNT_OPEN_FONT_X	EQU (APSH_PAD+26)	;Could not open font, %s
APSH_CLDNT_OPEN_MACRO	EQU (APSH_PAD+27)	;Could not open macro file, %s
APSH_CLDNT_OPEN_PREF	EQU (APSH_PAD+28)	;Could not open preference file, %s
APSH_CLDNT_OPEN_SCREEN	EQU (APSH_PAD+29)	;Could not open screen
APSH_CLDNT_OPEN_WINDOW	EQU (APSH_PAD+30)	;Could not open window
APSH_SETUP_TIMER	EQU (APSH_PAD+31)	;Could not set up timer event
APSH_SETUP_HOTKEYS	EQU (APSH_PAD+32)	;Could not set up HotKeys
APSH_START_PROCESS	EQU (APSH_PAD+33)	;Could not start process
APSH_START_TOOL		EQU (APSH_PAD+34)	;Could not start tool
APSH_START_TOOL_X	EQU (APSH_PAD+35)	;Could not start tool, %s
APSH_WRITE_FILE		EQU (APSH_PAD+36)	;Could not write to file
APSH_WRITE_FILE_X	EQU (APSH_PAD+37)	;Could not write to file, %s
APSH_WRITE_MACRO	EQU (APSH_PAD+38)	;Could not write to macro file
APSH_CMDSHELL_WIN	EQU (APSH_PAD+39)	;CON:0/150/600/50/Command Shell/CLOSE
APSH_NO_NAMETAG_WIN	EQU (APSH_PAD+40)	;No name given for window
APSH_NO_PORT		EQU (APSH_PAD+41)	;No port name specified
APSH_NOT_ENOUGH_MEMORY	EQU (APSH_PAD+42)	;Not enough memory
APSH_WAITING_FOR_MACRO	EQU (APSH_PAD+43)	;Waiting for macro return
APSH_DISABLED		EQU (APSH_PAD+44)	;%s is disabled
APSH_IOERR		EQU (APSH_PAD+45)	;IoErr #%ld
APSH_INVALID_NAMETAG	EQU (APSH_PAD+46)	;Invalid name tag.
APSH_OKAY_TXT		EQU (APSH_PAD+47)	;Okay
APSH_CANCEL_TXT		EQU (APSH_PAD+48)	;Cancel
APSH_CONTINUE_TXT	EQU (APSH_PAD+49)	;Continue
APSH_DONE_TXT		EQU (APSH_PAD+50)	;Done
APSH_ABORT_TXT		EQU (APSH_PAD+51)	;Abort
APSH_QUIT_TXT		EQU (APSH_PAD+52)	;Quit
APSH_UNNAMED		EQU (APSH_PAD+53)	;Unnamed
APSH_SYNTAX_ERROR	EQU (APSH_PAD+54)	;Syntax Error:\n%s %s
APSH_LAST_MESSAGE	EQU (APSH_PAD+55)

	endif
