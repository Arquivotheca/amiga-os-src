
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
#include <prefs/prefhdr.h>
#include <prefs/palette.h>
#include <utility/hooks.h>
#include <string.h>

/* application includes */
#include "palette_rev.h"
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
#define PORT_NAME   "Palette_Prefs"
#define ENV_NAME    "ENV:Sys/palette.prefs"
#define ARC_NAME    "ENVARC:Sys/palette.prefs"
#define PRESET_DIR  "SYS:Prefs/Presets"
#define PRESET_NAME ""


/*****************************************************************************/


#define MAXDEPTH        3
#define MAXCOLORS       8
#define MAXWHEELPENS    98
#define MAXGRADIENTPENS 32
#define LOWBITSPERGUN   2
#define HIGHBITSPERGUN  16   /* only 16 because of ColorSpec */
#define NUM_KNOWNPENS   12   /* changing this requires changing stuff in many places! */


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
    struct Library       *ed_GfxBase;
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
    struct PalettePrefs   ed_PrefsDefaults;
    struct PalettePrefs   ed_PrefsWork;
    struct PalettePrefs   ed_PrefsInitial;

    /* used by file requester calls */
    char                  ed_NameBuf[NAMEBUFSIZE];

    /* information used for localization */
    struct LocaleInfo     ed_LocaleInfo;

    /* the rest of this structure is editor-specific and is not used by the
     * common code
     */

    struct Screen        *ed_WBScreen;

    struct Library       *ed_ColorWheelBase;
    struct Library       *ed_GradientSliderBase;

    struct Gadget        *ed_Pens;
    struct Gadget        *ed_ColorPalette;
    struct Gadget	 *ed_Slider1;
    struct Gadget	 *ed_Slider2;
    struct Gadget	 *ed_Slider3;
    struct Gadget        *ed_PenPalette;
    struct Gadget        *ed_ColorWheel;
    struct Gadget        *ed_GradientSlider;
    struct Gadget        *ed_ColorMode;

    ULONG		  ed_CurrentRed;
    ULONG		  ed_CurrentGreen;
    ULONG		  ed_CurrentBlue;
    UWORD		  ed_CurrentColor;
    UWORD                 ed_CurrentPen;

    UWORD                 ed_RedBits;
    UWORD		  ed_GreenBits;
    UWORD		  ed_BlueBits;

    UBYTE                 ed_ColorTable[MAXCOLORS];
    WORD                  ed_NumColorsAllocated;
    WORD                  ed_GradPens[MAXGRADIENTPENS];
    WORD                  ed_NumGradPensAllocated;
    WORD                  ed_FirstGradPen;

    struct List           ed_PenList;
    struct Node           ed_PenNodes[NUM_KNOWNPENS];
    struct Hook           ed_PenCallBack;

    BOOL                  ed_EmbeddedSample;
    struct Window        *ed_SampleWindow;
    struct RastPort      *ed_SampleRastPort;
    WORD                  ed_SampleLeft;
    WORD                  ed_SampleTop;
    WORD                  ed_Depth;      /* 2 or 3 only */
    BOOL                  ed_SlidersRGB;
    BOOL                  ed_4ColorMode;

    UWORD                 ed_LeftBorder;
    UWORD                 ed_TopBorder;

    struct Window        *ed_WheelWindow;
    struct Screen        *ed_WheelScreen;
    struct DrawInfo      *ed_WheelDrawInfo;
    struct Gadget        *ed_WheelGadgets;
    struct Gadget        *ed_WheelLastAdded;
    APTR                  ed_WheelVisualInfo;
    UWORD                 ed_WheelSamplePen;
    BOOL                  ed_DoWheelScreen;
    struct ColorSpec      ed_WheelBackup;

    struct Requester      ed_SleepReq;
    BOOL                  ed_ASleep;

    STRPTR                ed_ColorModeLabels[3];

    struct BitMap        *ed_ClickMap;
    struct RastPort       ed_ClickRP;
} EdData;

typedef EdData *EdDataPtr;

#define DOSBase        ed->ed_DOSBase
#define IntuitionBase  ed->ed_IntuitionBase
#define AslBase        ed->ed_AslBase
#define IFFParseBase   ed->ed_IFFParseBase
#define IconBase       ed->ed_IconBase
#define GfxBase	       ed->ed_GfxBase
#define GadToolsBase   ed->ed_GadToolsBase
#define UtilityBase    ed->ed_UtilityBase
#define LocaleBase     ed->ed_LocaleBase
#define WorkbenchBase  ed->ed_WorkbenchBase
#define ColorWheelBase ed->ed_ColorWheelBase
#define window         ed->ed_Window


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

    ES_NO_COLORWHEEL,
    ES_NO_GRADIENT
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
    EC_CLOSEGADGET,

    /* custom editor commands */

    EC_SHOWSAMPLE,
    EC_COLORMODE,

    EC_COLORWHEEL,
    EC_GRADIENTSLIDER,

    EC_PENS,
    EC_PENPALETTE,

    EC_COLORPALETTE,
    EC_SLIDER1,
    EC_SLIDER2,
    EC_SLIDER3,

    EC_RGBMODEL,
    EC_HSBMODEL,

    EC_PRESET1,
    EC_PRESET2,
    EC_PRESET3,
    EC_PRESET4,
    EC_PRESET5,
    EC_PRESET6,
    EC_PRESET7,
    EC_PRESET8,
    EC_PRESET9,

    EC_SHOWWHEEL,
    EC_WHEELOK,
    EC_WHEELCANCEL,

    EC_SAMPLECLICK
} EdCommand;


#define SEND_EC_CLOSEGADGET   /* since we support EC_CLOSEGADGET */
#define USES_OWN_SCREEN


/* this describes the state of an editor command */
typedef struct CmdState
{
    BOOL cs_Available;
    BOOL cs_Selected;
} CmdState;

typedef CmdState *CmdStatePtr;


/*****************************************************************************/


BOOL InitDisp(EdDataPtr ed);
EdStatus InitEdData(EdDataPtr ed);
VOID CleanUpEdData(EdDataPtr ed);

EdStatus OpenPrefs(EdDataPtr ed, STRPTR name);
EdStatus SavePrefs(EdDataPtr ed, STRPTR name);

BOOL CreateDisplay(EdDataPtr ed);
VOID DisposeDisplay(EdDataPtr ed);
VOID RenderGadgets(EdDataPtr ed);

VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec);
VOID GetSpecialCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state);

VOID RenderDisplay(EdDataPtr ed);

EdCommand GetCommand(EdDataPtr ed);

#define ProcessArgs(ed,diskObj) {}
#define CopyPrefs(ed,p1,p2)     (*p1 = *p2)
#define SyncTextGadgets(ed)     {}
