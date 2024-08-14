
/* includes */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/execbase.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/text.h>
#include <graphics/gfxbase.h>
#include <libraries/asl.h>
#include <workbench/workbench.h>
#include <prefs/screenmode.h>
#include <prefs/prefhdr.h>
#include <string.h>

/* application includes */
#include "screenmode_rev.h"
#include "pe_strings.h"


/*****************************************************************************/


/* define the editor's command-line template here
 * OPT_FROM, OPT_EDIT, OPT_USE, OPT_SAVE, OPT_SCREEN and OPT_COUNT must all
 * be defined. If any of these do not apply, artificially bump OPT_COUNT
 * by one, and set the value of the unused arguments to OPT_COUNT-1
 */
#define TEMPLATE     "FROM,EDIT/S,USE/S,SAVE/S" VERSTAG
#define OPT_FROM     0
#define OPT_EDIT     1
#define OPT_USE      2
#define OPT_SAVE     3
#define OPT_SCREEN   4
#define OPT_COUNT    5


/*****************************************************************************/


/* You don't generally touch these */
#define NAMEBUFSIZE  512
#define PROGPATHSIZE 256


/*****************************************************************************/


/* Fill-in these values for the editor */
#define PORT_NAME   "ScreenMode_Prefs"
#define ENV_NAME    "ENV:Sys/screenmode.prefs"
#define ARC_NAME    "ENVARC:Sys/screenmode.prefs"
#define PRESET_DIR  "SYS:Prefs/Presets"
#define PRESET_NAME ""


/*****************************************************************************/


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
    struct GfxBase       *ed_GfxBase;
    struct Library       *ed_AslBase;
    struct Library       *ed_IFFParseBase;
    struct Library       *ed_IconBase;
    struct Library       *ed_GadToolsBase;
    struct Library       *ed_UtilityBase;
    struct Library	 *ed_LocaleBase;
    struct Library	 *ed_WorkbenchBase;

    /* execution environment details, can be looked at anytime */
    struct RDArgs	 *ed_RdArgs;
    ULONG		  ed_Args[OPT_COUNT];
    char	          ed_ProgramPath[PROGPATHSIZE];
    STRPTR		  ed_PubScreenName;
    char		  ed_ScreenNameBuf[32];
    APTR                  ed_OldWindowPtr;

    /* message port used to detect other running instances of this program.
     * This is used by the common code, do not touch
     */
    struct MsgPort        ed_RendezVous;

    /* data needed for the AppWindow and AppMenu */
    struct MsgPort	  ed_AppPort;
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

    /* when processing a command, this will contain a copy of the
     * IntuiMessage that caused this command to be executed.
     */
    struct IntuiMessage   ed_CurrentMsg;

    /* various read/write state flags */
    BOOL		  ed_Quit;           /* set to TRUE to quit */
    BOOL                  ed_Cancelled;
    BOOL		  ed_InitFail;       /* set to TRUE when InitEdData fails */
    BOOL                  ed_CheckCmdState;  /* tell the main code to validate the state of menus and gadgets */
    BOOL                  ed_SaveIcons;      /* should we save icons? */
    LONG                  ed_SecondaryResult;/* pr_Result2 for this program */
    STRPTR		  ed_ErrorFileName;  /* name for IO error displays */

    /* this should be changed to be of the current editor's favorite type */
    struct ScreenModePrefs ed_PrefsDefaults;
    struct ScreenModePrefs ed_PrefsWork;
    struct ScreenModePrefs ed_PrefsInitial;

    /* used by file requester calls */
    char                  ed_NameBuf[NAMEBUFSIZE];

    /* information used for localization */
    struct LocaleInfo     ed_LocaleInfo;

    /* the rest of this structure is editor-specific and is not used by the
     * common code
     */

    struct Remember      *ed_Tracker;

    struct Gadget        *ed_ModeList;
    struct Gadget        *ed_Colors;
    struct Gadget        *ed_Width;
    struct Gadget        *ed_Height;
    struct Gadget        *ed_DefWidth;
    struct Gadget        *ed_DefHeight;
    struct Gadget        *ed_AutoScroll;

    struct List		  ed_DisplayModes;
    struct List           ed_Properties;
    APTR                  ed_LastMode;
    struct Node           ed_PropNodes[12];
    char		  ed_VisibleSize[40];
    char		  ed_MaximumSize[40];
    char		  ed_MinimumSize[40];
    char		  ed_MaximumColors[40];
    char                  ed_ScanRates[40];
} EdData;

typedef EdData *EdDataPtr;

#define DOSBase       ed->ed_DOSBase
#define IntuitionBase ed->ed_IntuitionBase
#define AslBase       ed->ed_AslBase
#define IFFParseBase  ed->ed_IFFParseBase
#define IconBase      ed->ed_IconBase
#define GfxBase	      ed->ed_GfxBase
#define GadToolsBase  ed->ed_GadToolsBase
#define UtilityBase   ed->ed_UtilityBase
#define LocaleBase    ed->ed_LocaleBase
#define WorkbenchBase ed->ed_WorkbenchBase
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
    ES_DOSERROR
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
    EC_MODELIST,
    EC_COLORS,
    EC_WIDTH,
    EC_HEIGHT,
    EC_DEFWIDTH,
    EC_DEFHEIGHT,
    EC_AUTOSCROLL
} EdCommand;


/* this describes the state of an editor command */
typedef struct CmdState
{
    BOOL cs_Available;
    BOOL cs_Selected;
} CmdState;

typedef CmdState *CmdStatePtr;


/*****************************************************************************/


EdStatus InitEdData(EdDataPtr ed);
VOID CleanUpEdData(EdDataPtr ed);
VOID ProcessArgs(EdDataPtr ed, struct DiskObject *diskObj);

EdStatus OpenPrefs(EdDataPtr ed, STRPTR name);
EdStatus SavePrefs(EdDataPtr ed, STRPTR name);

BOOL CreateDisplay(EdDataPtr ed);
VOID DisposeDisplay(EdDataPtr ed);
VOID RenderDisplay(EdDataPtr ed);
VOID RenderGadgets(EdDataPtr ed);
VOID SyncTextGadgets(EdDataPtr ed);

VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec);
VOID GetSpecialCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state);


#define CopyPrefs(ed,p1,p2) (*p1 = *p2)
#define InitDisp(ed)        (TRUE)
#define GetCommand(ed)      (EC_NOP)
