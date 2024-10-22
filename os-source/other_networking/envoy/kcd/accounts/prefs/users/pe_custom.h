#ifndef PE_CUSTOM_H
#define PE_CUSTOM_H

/* includes */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/execbase.h>
#include <intuition/intuition.h>
#include <intuition/sghooks.h>
#include <intuition/screens.h>
#include <graphics/text.h>
#include <libraries/asl.h>
#include <workbench/workbench.h>
#include <prefs/prefhdr.h>
#include <string.h>

/* application includes */
#include "userprefs_rev.h"
#include "pe_strings.h"
#include "//transactions.h"
#include <envoy/envoy.h>
#include <clib/envoy_protos.h>
#include <clib/nipc_protos.h>
#include <pragmas/envoy_pragmas.h>
#include <pragmas/nipc_pragmas.h>

/*****************************************************************************/


/* define the editor's command-line template here
 * OPT_FROM, OPT_EDIT, OPT_USE, OPT_SAVE, OPT_SCREEN and OPT_COUNT must all
 * be defined. If any of these do not apply, artificially bump OPT_COUNT
 * by one, and set the value of the unused arguments to OPT_COUNT-1
 */
#define TEMPLATE     "HOSTNAME/K,USERNAME/K,PASSWORD/K,PUBSCREEN/K" VERSTAG
#define OPT_HOST     0
#define OPT_USER     1
#define OPT_PASSWORD 2
#define OPT_SCREEN   3
#define OPT_COUNT    5
#define OPT_FROM     OPT_COUNT-1
#define OPT_EDIT     OPT_COUNT-1
#define OPT_USE      OPT_COUNT-1
#define OPT_SAVE     OPT_COUNT-1
#define OPEN_NIPC    TRUE

/*****************************************************************************/


/* You don't generally touch these */
#define NAMEBUFSIZE  512
#define PROGPATHSIZE 256

/*****************************************************************************/


/* Fill-in these values for the editor */
#define PORT_NAME   "User_Prefs"
#define ENV_NAME    "ENV:Envoy/nipc.prefs"
#define ARC_NAME    "ENVARC:Envoy/nipc.prefs"
#define PRESET_DIR  "SYS:Prefs/Presets"
#define PRESET_NAME ""


/*****************************************************************************/


/*****************************************************************************/
struct ExtPrefs
{
	struct List	ep_UserList;
	struct List	ep_GroupList;
};

/* This is the global data structure containing all global variables used by
 * the editor. The first part of the structure should not change from editor to
 * editor, while the second part can be changed to whatever the current editor
 * needs
 *
 * This structure is normally passed to you as a parameter. In exceptional
 * cases, the pointer call also be found in the tc_UserData field of the
 * task structure.
 */
typedef struct EdData
{
    /* all the libraries opened automatically by the common code */
    struct ExecBase      *ed_SysBase;
    struct Library       *ed_IntuitionBase;
    struct Library       *ed_DOSBase;
    struct Library       *ed_GfxBase;
    struct Library       *ed_AslBase;
    struct Library       *ed_IFFParseBase;
    struct Library       *ed_IconBase;
    struct Library       *ed_GadToolsBase;
    struct Library       *ed_UtilityBase;
    struct Library       *ed_LocaleBase;
    struct Library       *ed_WorkbenchBase;
    struct Library	 *ed_NIPCBase;
    struct Library	 *ed_EnvoyBase;

    /* execution environment details, can be looked at anytime */
    struct RDArgs        *ed_RdArgs;
    ULONG                 ed_Args[OPT_COUNT];
    char                  ed_ProgramPath[PROGPATHSIZE];
    STRPTR                ed_PubScreenName;
    char                  ed_ScreenNameBuf[32];
    APTR                  ed_OldWindowPtr;

    /* message port used to detect other running instances of this program.
     * This is used by the common code, do not touch
     */
    struct MsgPort        ed_RendezVous;

    /* data needed for the AppWindow and AppMenu */
    struct MsgPort        ed_AppPort;
    struct AppWindow     *ed_AppWindow;
    struct AppMenu       *ed_AppMenu;

    /* another private field used by the common code's gadget creation
     * function
     */
    struct Gadget        *ed_LastAdded;

    /* display attributes for this editor */
    struct Window        *ed_Window;
    struct Screen        *ed_Screen;
    struct Menu          *ed_Menus;
    struct Gadget        *ed_Gadgets;
    struct Font          *ed_Font;
    struct DrawInfo      *ed_DrawInfo;
    struct FileRequester *ed_FileReq;
    APTR                  ed_VisualInfo;
    struct TextAttr       ed_TextAttr;

    /* when processing a command, this will point to the Intuition message
     * that caused this command to be executed. Do NOT reply this message
     * yourself, the common code does this
     */
    struct IntuiMessage   ed_CurrentMsg;

    /* various read/write state flags */
    BOOL                  ed_Quit;           /* set to TRUE to quit */
    BOOL                  ed_Cancelled;
    BOOL                  ed_InitFail;       /* set to TRUE when InitEdData fails */
    BOOL                  ed_CheckCmdState;  /* tell the main code to validate the state of menus and gadgets */
    BOOL                  ed_SaveIcons;      /* should we save icons? */
    LONG                  ed_SecondaryResult;/* pr_Result2 for this program */
    STRPTR                ed_ErrorFileName;  /* name for IO error displays */

    /* this should be changed to be of the current editor's favorite type */
    struct ExtPrefs       ed_PrefsDefaults;
    struct ExtPrefs       ed_PrefsWork;
    struct ExtPrefs       ed_PrefsInitial;

    /* used by file requester calls */
    char                  ed_NameBuf[NAMEBUFSIZE];

    /* information used for localization */
    struct LocaleInfo     ed_LocaleInfo;

    /* the rest of this structure is editor-specific and is not used by the
     * common code
     */

    ULONG		  ed_V39;
    ULONG		  ed_EntitySignal;
    struct UserNode	 *ed_CurrentUser;
    struct Hook		  ed_PWEditHook;
    struct UserData	  ed_AuthUser;
    struct UserData	  ed_NextUser;
    struct GroupData	  ed_NextGroup;
    UBYTE		  ed_MachineName[128];
    UBYTE		  ed_NewMachineName[128];
    UBYTE		  ed_UserStr[32];
    UBYTE		  ed_PassStr[32];
    UBYTE		  ed_TmpPassStr[32];
    UBYTE		  ed_GroupStr[32];
    UBYTE		  ed_NewGroupStr[32];
    UBYTE		  ed_LoginStr[256];
    UBYTE		  ed_WindowTitle[256];
    STRPTR		  ed_MachineStr;

    struct Entity	 *ed_LocalEntity;
    struct Entity	 *ed_RemoteEntity;
    struct AdminUserRequest ed_AdminUser;
    struct Transaction   *ed_Transaction;
    struct Gadget	 *ed_UserList;
    struct Gadget	 *ed_AddUser;
    struct Gadget	 *ed_RemUser;
    struct Gadget	 *ed_UserName;
    struct Gadget	 *ed_UserPassword;
    struct Gadget	 *ed_HostName;
    struct Gadget	 *ed_GroupButton;
    struct Gadget	 *ed_GroupText;
    struct Gadget	 *ed_AuthName;
    struct Gadget	 *ed_AuthPassword;
    struct Gadget	 *ed_UserIsAdmin;
    struct Gadget	 *ed_UserEdGroups;
    struct Gadget	 *ed_UserEdName;
    struct Gadget	 *ed_UserEdPassword;
    struct Gadget	 *ed_UserNeedsPassword;

} EdData;

typedef EdData *EdDataPtr;

#define DOSBase       ed->ed_DOSBase
#define IntuitionBase ed->ed_IntuitionBase
#define AslBase       ed->ed_AslBase
#define IFFParseBase  ed->ed_IFFParseBase
#define IconBase      ed->ed_IconBase
#define GfxBase       ed->ed_GfxBase
#define GadToolsBase  ed->ed_GadToolsBase
#define UtilityBase   ed->ed_UtilityBase
#define LocaleBase    ed->ed_LocaleBase
#define WorkbenchBase ed->ed_WorkbenchBase
#define NIPCBase      ed->ed_NIPCBase
#define EnvoyBase     ed->ed_EnvoyBase
#define window        ed->ed_Window


/* state the editor can be in, ES_NORMAL to ES_DOSERROR are required,
 * everything else is optional
 */
typedef enum EdStatus
{
    ES_NORMAL,
    ES_NO_MEMORY,
    ES_IFF_UNKNOWNCHUNK,
    ES_IFF_UNKNOWNVERSION,
    ES_IFFERROR,
    ES_DOSERROR,
    ES_NO_SERVER
} EdStatus;


/* commands this editor supports */
typedef enum EdCommand
{
    /* standard prefs editor commands */
    EC_NOP,
    EC_OPEN,
    EC_SAVE,
    EC_SAVEAS,
    EC_LASTSAVED,
    EC_RESTORE,
    EC_RESETALL,
    EC_SAVEICONS,
    EC_USE,
    EC_CANCEL,
    EC_ACTIVATE,

    /* custom editor commands */
    EC_SETHOST,
    EC_ADDUSER,
    EC_REMUSER,
    EC_USERLIST,
    EC_USERNAME,
    EC_PASSWORD,
    EC_HOSTNAME,
    EC_AUTHNAME,
    EC_AUTHPW,
    EC_ADMIN,
    EC_ADMINGROUPS,
    EC_ADMINNAME,
    EC_ADMINPASSWORD,
    EC_ADMINNEEDPW,
    EC_SETLOGIN,
    EC_SETGROUP

} EdCommand;

/* this describes the state of an editor command */
typedef struct CmdState
{
    BOOL cs_Available;
    BOOL cs_Selected;
} CmdState;

typedef CmdState *CmdStatePtr;


/*****************************************************************************/

#define ASM __asm
#define REG(x) register __## x

EdStatus InitEdData(EdDataPtr ed);
VOID InitPrefs(struct ExtPrefs *prefs);
VOID CleanUpEdData(EdDataPtr ed);

VOID FreePrefs(EdDataPtr ed, struct ExtPrefs *prefs);

#define NoAccountsServer(ed) {}

VOID FreeList(EdDataPtr ed, struct List *list, ULONG nodeSize);
VOID ClearGadPtrs(EdDataPtr ed);

#define OpenPrefs(ed, name)  ES_NORMAL
#define SavePrefs(ed, name)  ES_NORMAL

BOOL HostReq(EdDataPtr ed, ULONG tag1, ...);
BOOL LoginReq(EdDataPtr ed, ULONG tag1, ...);
BOOL UserReq(EdDataPtr ed, ULONG tag1, ...);
BOOL CreateDisplay(EdDataPtr ed);
VOID GetGroupName(EdDataPtr ed);
ULONG GetGroupID(EdDataPtr ed);
VOID DisposeDisplay(EdDataPtr ed);
VOID MakeNewDisplay(EdDataPtr ed);
VOID RenderDisplay(EdDataPtr ed);
VOID RenderGadgets(EdDataPtr ed);
VOID AddNodeSorted(EdDataPtr ed, struct List *list, struct Node *node);
ULONG DoServerCmd(EdDataPtr ed, UBYTE cmd);
VOID GetUserList(EdDataPtr ed);
BOOL ConnectToServer(EdDataPtr ed);
VOID LoginUser(EdDataPtr ed);
struct Entity *CreateEnt(EdDataPtr ed, ULONG tag1, ...);
VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec);
EdCommand GetCommand(EdDataPtr ed);
VOID GetSpecialCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state);
struct Requester *SetBusyPointer(EdDataPtr ed, struct Window *win);
void ResetBusyPointer(EdDataPtr ed, struct Requester *r, struct Window *win);

VOID ASM IntuiHook(REG(a0) struct Hook *hook,
                   REG(a2) struct FileRequester *fr,
                   REG(a1) struct IntuiMessage *intuiMsg);

#define CopyPrefs(ed, p1, p2)   {}
#define ProcessArgs(ed,diskObj) {}
#define InitDisp(ed)            (TRUE)
#define SyncTextGadgets(ed)     {}
#define RenderDisplay(ed)               {}
#define GetCommand(ed)          (EC_NOP)

#endif /* PE_CUSTOM_H */