
/* includes */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/execbase.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/text.h>
#include <libraries/asl.h>
#include <workbench/workbench.h>
#include <utility/hooks.h>
#include <prefs/printerps.h>
#include <prefs/prefhdr.h>
#include <string.h>

/* application includes */
#include "printerps_rev.h"
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
#define PORT_NAME   "PrinterPS_Prefs"
#define ENV_NAME    "ENV:Sys/printerps.prefs"
#define ARC_NAME    "ENVARC:Sys/printerps.prefs"
#define PRESET_DIR  "SYS:Prefs/Presets"
#define PRESET_NAME ""


/*****************************************************************************/


#define LOCKLAYERINFO
#define	NUMVECTORS     5
#define	RASWIDTH       42
#define	RASHEIGHT      42


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
    struct Library	 *ed_LocaleBase;
    struct Library	 *ed_WorkbenchBase;
    struct Library	 *ed_LayersBase;

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
    struct PrinterPSPrefs ed_PrefsDefaults;
    struct PrinterPSPrefs ed_PrefsWork;
    struct PrinterPSPrefs ed_PrefsInitial;

    /* used by file requester calls */
    char                  ed_NameBuf[NAMEBUFSIZE];

    /* information used for localization */
    struct LocaleInfo     ed_LocaleInfo;

    /* the rest of this structure is editor-specific and is not used by the
     * common code
     */

    UWORD                 ed_CurrentPanel;
    UWORD		  ed_CurrentFormat;
    UWORD		  ed_CurrentSystem; /* 0=CM, 1=INCHES, 2=POINTS */

    UWORD		  ed_Refresh;
    UBYTE		  ed_AreaBuffer[NUMVECTORS*5];
    struct TmpRas	  ed_TmpRas;
    struct AreaInfo	  ed_AreaInfo;
    PLANEPTR		  ed_PlanePtr;

    struct Hook		  ed_FloatHook;

    struct Gadget        *ed_DriverMode;
    struct Gadget	 *ed_Copies;
    struct Gadget        *ed_PaperFormat;
    struct Gadget        *ed_PaperWidth;
    struct Gadget        *ed_PaperHeight;
    struct Gadget        *ed_HorizontalDPI;
    struct Gadget	 *ed_VerticalDPI;
    struct Gadget	 *ed_Panel;

    struct Gadget        *ed_PrintFont;
    struct Gadget        *ed_Pitch;
    struct Gadget        *ed_Orientation;
    struct Gadget        *ed_Tab;

    struct Gadget	 *ed_LeftMargin;
    struct Gadget	 *ed_RightMargin;
    struct Gadget	 *ed_TopMargin;
    struct Gadget	 *ed_BottomMargin;
    struct Gadget	 *ed_FontPointSize;
    struct Gadget	 *ed_LineLeading;
    struct Gadget	 *ed_LinesPerInch;
    struct Gadget	 *ed_LinesPerPage;

    struct Gadget        *ed_Aspect;
    struct Gadget        *ed_ScalingType;
    struct Gadget        *ed_Centering;

    struct Gadget	 *ed_LeftEdge;
    struct Gadget	 *ed_TopEdge;
    struct Gadget	 *ed_Width;
    struct Gadget	 *ed_Height;
    struct Gadget        *ed_Image;
    struct Gadget        *ed_Shading;
    struct Gadget        *ed_Dithering;

    STRPTR                ed_DriverModeLabels[3];
    STRPTR	          ed_PaperFormatLabels[5];
    STRPTR		  ed_PanelLabels[5];

    STRPTR		  ed_PrintFontLabels[9];
    STRPTR		  ed_PitchLabels[4];
    STRPTR		  ed_OrientationLabels[3];
    STRPTR		  ed_TabLabels[6];

    STRPTR		  ed_AspectLabels[3];
    STRPTR		  ed_ScalingTypeLabels[8];
    STRPTR		  ed_CenteringLabels[5];

    STRPTR		  ed_ImageLabels[3];
    STRPTR		  ed_ShadingLabels[4];
    STRPTR		  ed_DitheringLabels[5];

    UBYTE		  ed_NumBuffer[48];

    char                  ed_LinesPerInchBuf[32];
    char                  ed_LinesPerPageBuf[32];

    LONG		  ed_CurPaperWidth;
    LONG		  ed_CurPaperHeight;
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
#define	LayersBase    ed->ed_LayersBase
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

    EC_CM,
    EC_INCHES,
    EC_POINTS,

    EC_DRIVERMODE,
    EC_COPIES,
    EC_PAPERFORMAT,
    EC_PAPERWIDTH,
    EC_PAPERHEIGHT,
    EC_HORIZONTALDPI,
    EC_VERTICALDPI,
    EC_PANEL,

    EC_PRINTFONT,
    EC_PITCH,
    EC_ORIENTATION,
    EC_TAB,

    EC_LEFTMARGIN,
    EC_RIGHTMARGIN,
    EC_TOPMARGIN,
    EC_BOTTOMMARGIN,
    EC_FONTPOINTSIZE,
    EC_LINELEADING,
    EC_LINESPERINCH,
    EC_LINESPERPAGE,

    EC_LEFTEDGE,
    EC_TOPEDGE,
    EC_WIDTH,
    EC_HEIGHT,
    EC_IMAGE,
    EC_SHADING,
    EC_DITHERING,

    EC_ASPECT,
    EC_SCALINGTYPE,
    EC_CENTERING,

    EC_TIMER
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

EdStatus OpenPrefs(EdDataPtr ed, STRPTR name);
EdStatus SavePrefs(EdDataPtr ed, STRPTR name);

BOOL CreateDisplay(EdDataPtr ed);
VOID DisposeDisplay(EdDataPtr ed);
VOID RenderDisplay(EdDataPtr ed);
VOID RenderGadgets(EdDataPtr ed);
VOID SyncTextGadgets(EdDataPtr ed);
VOID CleanUpEdData(EdDataPtr ed);

VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec);
VOID GetSpecialCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state);


#define InitDisp(ed)            (TRUE)
#define ProcessArgs(ed,diskObj) {}
#define GetCommand(ed)          (EC_NOP)
#define CopyPrefs(ed,p1,p2) (*p1 = *p2)
