
/* includes */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/execbase.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/text.h>
#include <graphics/displayinfo.h>
#include <libraries/asl.h>
#include <workbench/workbench.h>
#include <prefs/overscan.h>
#include <prefs/prefhdr.h>
#include <string.h>

/* application includes */
#include "overscan_rev.h"
#include "pe_strings.h"


/*****************************************************************************/


/* define the editor's command-line template here
 * OPT_FROM, OPT_EDIT, OPT_USE, OPT_SAVE, OPT_SCREEN and OPT_COUNT must all
 * be defined. If any of these do not apply, artificially bump OPT_COUNT
 * by one, and set the value of the unused arguments to OPT_COUNT-1
 */
#define TEMPLATE     "FROM,EDIT/S,USE/S,SAVE/S,PUBSCREEN/K" VERSTAG
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
#define PORT_NAME   "Overscan_Prefs"
#define ENV_NAME    "ENV:Sys/overscan.prefs"
#define ARC_NAME    "ENVARC:Sys/overscan.prefs"
#define PRESET_DIR  "SYS:Prefs/Presets"
#define PRESET_NAME ""


/*****************************************************************************/


#define MENAMELEN   30
#define NUMMONITORS 50

struct MonitorEntry
{
    struct Node          me_Node;
    ULONG                me_ID;
    UWORD                me_HStart;
    UWORD                me_HStop;
    UWORD                me_VStart;
    UWORD                me_VStop;
    struct DisplayInfo   me_DisplayInfo;
    struct DimensionInfo me_DimensionInfo;
    struct MonitorInfo   me_MonitorInfo;
    char                 me_Name[DISPLAYNAMELEN];
};

struct ExtPrefs
{
    struct ExtPrefs     *ep_Next;
    struct MonitorEntry  ep_Monitor;
};


struct Handles
{
    Point pos[3];
};


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
    APTR		  ed_OldWindowPtr;

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

    /* when processing a command, this will point to the Intuition message
     * that caused this command to be executed. Do NOT reply this message
     * yourself, the common code does this
     */
    struct IntuiMessage   ed_CurrentMsg;

    /* various read/write state flags */
    BOOL		  ed_Quit;           /* set to TRUE to quit */
    BOOL		  ed_Cancelled;
    BOOL		  ed_InitFail;       /* set to TRUE when InitEdData fails */
    BOOL                  ed_CheckCmdState;  /* tell the main code to validate the state of menus and gadgets */
    BOOL                  ed_SaveIcons;      /* should we save icons? */
    LONG                  ed_SecondaryResult;/* pr_Result2 for this program */
    STRPTR		  ed_ErrorFileName;  /* name for IO error displays */

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

    struct Remember      *ed_Tracker;
    struct Gadget        *ed_EditText;
    struct Gadget        *ed_EditGfx;
    struct List		  ed_MonitorList;
    struct MonitorEntry  *ed_CurrentMon;

    struct Rectangle      MaxOScan, StdOScan, TxtOScan;
    WORD                  NomWidth, NomHeight;
    UWORD                 HandleWidth, HandleHeight;
    UWORD                 xscale, yscale;

    struct RastPort       *ed_EditRP;
    struct Region         *ed_EditRegion;

    struct Rectangle      Current, Prev, Undo;
    struct Handles        CurrentHandles, PrevHandles;

    /*  Delta tells us how far the mouse was from each edge when
        the select button went down: */
    struct Rectangle      Delta;

    /*  LoLim and HiLim are the lower and higher limits for each of
        the four edges during a sizing operation: */
    struct Rectangle      LoLim, HiLim;

    /*  horiz = 1, 2, or 3 depending on whether the user hit a left,
        middle, or right handle.  vert = 1, 2, or 3 similarly: */
    WORD                  horiz, vert;

    Point                 dViewPos, ViewPos;

    WORD                  OldX;
    WORD                  OldY;
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

    EC_MONITORLIST,
    EC_EDITTEXT,
    EC_EDITGFX
} EdCommand;


/* this describes the state of an editor command */
typedef struct CmdState
{
    BOOL cs_Available;
    BOOL cs_Selected;
} CmdState;

typedef CmdState *CmdStatePtr;


/* For EdData.Pushing */
#define SYNC_NONE  0
#define SYNC_UP    1
#define SYNC_LEFT  2
#define SYNC_RIGHT 3
#define SYNC_DOWN  4


/*****************************************************************************/


EdStatus InitEdData(EdDataPtr ed);
VOID CleanUpEdData(EdDataPtr ed);

EdStatus OpenPrefs(EdDataPtr ed, STRPTR name);
EdStatus SavePrefs(EdDataPtr ed, STRPTR name);
VOID CopyPrefs(EdDataPtr ed, struct ExtPrefs *p1, struct ExtPrefs *p2);

BOOL CreateDisplay(EdDataPtr ed);
VOID DisposeDisplay(EdDataPtr ed);
VOID RenderDisplay(EdDataPtr ed);
VOID RenderGadgets(EdDataPtr ed);

VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec);
VOID GetSpecialCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state);

VOID CenterLine(EdDataPtr ed, struct Window *wp, AppStringsID id,
                UWORD x, UWORD y, UWORD w);

#define ProcessArgs(ed,diskObj) {}
#define InitDisp(ed)            (TRUE)
#define SyncTextGadgets(ed)     {}
#define GetCommand(ed)          (EC_NOP)
