
/* includes */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/execbase.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/screens.h>
#include <libraries/iffparse.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <graphics/text.h>
#include <libraries/asl.h>
#include <workbench/workbench.h>
#include <prefs/wbpattern.h>
#include <prefs/prefhdr.h>
#include <string.h>

/* application includes */
#include "wbpattern_rev.h"
#include "pe_strings.h"

/*****************************************************************************/

/* Pref shell controls */
#define	DATATYPES	TRUE

/*****************************************************************************/

#define	MAXPENS		8
#define	MAXPRESETS	8
#define	MAXCOLORS	8
#define	PATHNAMESIZE	300

/*****************************************************************************/

/* Fill-in these values for the editor */
#define PORT_NAME   "WBPattern_Prefs"
#define ENV_NAME    "ENV:Sys/WBPattern.prefs"
#define ARC_NAME    "ENVARC:Sys/WBPattern.prefs"
#define PRESET_DIR  "SYS:Prefs/Presets"
#define PRESET_NAME "WBPattern.pre"

/*****************************************************************************/

struct ExtPrefs
{
    struct WBPatternPrefs	 ep_WBP[3];
    UBYTE			 ep_Path[3][PATHNAMESIZE];
    struct BitMap		 ep_BMap;
    struct RastPort		 ep_RPort;
};

/*****************************************************************************/

/* define the editor's command-line template here
 * OPT_FROM, OPT_EDIT, OPT_USE, OPT_SAVE, OPT_SCREEN and OPT_COUNT must all
 * be defined. If any of these do not apply, artificially bump OPT_COUNT
 * by one, and set the value of the unused arguments to OPT_COUNT-1
 */
#define TEMPLATE	"FROM,EDIT/S,USE/S,SAVE/S,CLIPUNIT/K/N,NOREMAP/S" VERSTAG
#define OPT_FROM	0
#define OPT_EDIT	1
#define OPT_USE		2
#define OPT_SAVE	3
#define	OPT_CLIPUNIT	4
#define	OPT_NOREMAP	5
#define	OPT_SCREEN	6
#define OPT_COUNT	7

/*****************************************************************************/

/* You don't generally touch these */
#define NAMEBUFSIZE	512
#define PROGPATHSIZE	256

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
    struct ExecBase *ed_SysBase;
    struct Library *ed_IntuitionBase;
    struct Library *ed_DOSBase;
    struct Library *ed_GfxBase;
    struct Library *ed_AslBase;
    struct Library *ed_IFFParseBase;
    struct Library *ed_IconBase;
    struct Library *ed_GadToolsBase;
    struct Library *ed_UtilityBase;
    struct Library *ed_LocaleBase;
    struct Library *ed_WorkbenchBase;
    struct Library *ed_DiskfontBase;

    /* execution environment details, can be looked at anytime */
    struct RDArgs *ed_RdArgs;
    ULONG ed_Args[OPT_COUNT];
    char ed_ProgramPath[PROGPATHSIZE];
    STRPTR ed_PubScreenName;
    char ed_ScreenNameBuf[32];
    APTR ed_OldWindowPtr;

    /*
     * message port used to detect other running instances of this program. This is used by the common code, do not touch
     */
    struct MsgPort ed_RendezVous;

    /* data needed for the AppWindow and AppMenu */
    struct MsgPort ed_AppPort;
    struct AppWindow *ed_AppWindow;
    struct AppMenu *ed_AppMenu;

    /*
     * another private field used by the common code's gadget creation function
     */
    struct Gadget *ed_LastAdded;

    /* display attributes for this editor */
    struct Window *ed_Window;
    struct Screen *ed_Screen;
    struct Menu *ed_Menus;
    struct Gadget *ed_Gadgets;
    struct Font *ed_Font;
    struct DrawInfo *ed_DrawInfo;
    struct FileRequester *ed_FileReq;
    APTR ed_VisualInfo;
    struct TextAttr ed_TextAttr;

    /*
     * when processing a command, this will point to the Intuition message that caused this command to be executed. Do NOT
     * reply this message yourself, the common code does this
     */
    struct IntuiMessage ed_CurrentMsg;

    /* various read/write state flags */
    BOOL ed_Quit;		/* set to TRUE to quit */
    BOOL ed_Cancelled;
    BOOL ed_InitFail;		/* set to TRUE when InitEdData fails */
    BOOL ed_CheckCmdState;	/* tell the main code to validate the state of menus and gadgets */
    BOOL ed_SaveIcons;		/* should we save icons? */
    LONG ed_SecondaryResult;	/* pr_Result2 for this program */
    STRPTR ed_ErrorFileName;	/* name for IO error displays */

    /* this should be changed to be of the current editor's favorite type */
    struct ExtPrefs       ed_PrefsDefaults;
    struct ExtPrefs       ed_PrefsWork;
    struct ExtPrefs       ed_PrefsInitial;

    /* used by file requester calls */
    char ed_NameBuf[NAMEBUFSIZE];

    /* information used for localization */
    struct LocaleInfo ed_LocaleInfo;

    /*
     * the rest of this structure is editor-specific and is not used by the common code
     */

    /* DataTypes-related variables */
    struct Library	*ed_DataTypesBase;
    struct Hook		 ed_FilterHook;
    Object		*ed_DataObject;

    struct WBPatternPrefs ed_PrefsTemp;
    struct BitMap ed_Undo;			/* Undo bitmap */
    struct BitMap ed_Temp;			/* Temporary bitmap for magnify */
    struct BitMap ed_Presets;			/* Bitmap that holds the presets */

    ULONG ed_ClipUnit;

    struct Screen *ed_PScreen;

    Class *ed_SketchClass;
    struct RastPort *ed_IRPort;
    UWORD ed_WBColors[6];
    UBYTE ed_ColorTable[MAXPENS];

    struct Gadget *ed_GWhich;
    struct Gadget *ed_GTest;
    struct Gadget *ed_GClear;
    struct Gadget *ed_GUndo;
    struct Gadget *ed_GSketch;
    struct Gadget *ed_GPalette;

    struct Gadget *ed_GType;
    struct Gadget *ed_GSelect;
    struct Gadget *ed_GName;

    struct Gadget *ed_GPresets[MAXPRESETS];

    struct FileRequester *ed_SampleReq;

    STRPTR ed_WhichLabels[4];
    STRPTR ed_TypeLabels[3];

    UWORD ed_APen;
    UWORD ed_Which;
    UWORD ed_Type;

    UWORD ed_Width;
    UWORD ed_Height;
    UWORD ed_Depth;

    UWORD ed_XMag;
    UWORD ed_YMag;

    UWORD ed_FileType;

    ULONG ed_Flags;

    struct FileRequester *ed_ImageReq;
    char ed_ImageBuf[NAMEBUFSIZE];

    BOOL ed_RemapImage;
} EdData;

#define	EDF_TESTWIN	0x0002
#define	EDF_TESTBD	0x0004
#define	EDF_CHANGED	0x0008

#define	WBP_ENV		0
#define	WBP_ARC		1
#define	WBP_FILE	2

typedef EdData *EdDataPtr;

#define DOSBase		ed->ed_DOSBase
#define IntuitionBase	ed->ed_IntuitionBase
#define AslBase		ed->ed_AslBase
#define IFFParseBase	ed->ed_IFFParseBase
#define IconBase	ed->ed_IconBase
#define GfxBase		ed->ed_GfxBase
#define GadToolsBase	ed->ed_GadToolsBase
#define UtilityBase	ed->ed_UtilityBase
#define LocaleBase	ed->ed_LocaleBase
#define WorkbenchBase	ed->ed_WorkbenchBase
#define window		ed->ed_Window
#define	DataTypesBase	ed->ed_DataTypesBase
#define DiskfontBase    ed->ed_DiskfontBase

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

    ES_REQUIRES_DATATYPES,
    ES_NOCLIPBOARD,
    ES_NOGRAPHIC,
    ES_NOT_A_PICTURE,

} EdStatus;

#define	DATATYPE_ERROR			MSG_WBP_NO_DATATYPES_LIBRARY

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

    EC_CUT,
    EC_COPY,
    EC_PASTE,
    EC_ERASE,
    EC_UNDO,
    EC_LOAD_IMAGE,

    EC_TEST,
    EC_CLEAR,
    EC_WHICH,
    EC_PRESETS,
    EC_PALETTE,
    EC_SKETCH,

    EC_TYPE,
    EC_SELECT,

    EC_MYRESETALL,
    EC_MYLASTSAVED,
    EC_MYRESTORE,

} EdCommand;

/* this describes the state of an editor command */
typedef struct CmdState
{
    BOOL cs_Available;
    BOOL cs_Selected;
} CmdState;

typedef CmdState *CmdStatePtr;

typedef EdStatus (*IFFFunc)(EdDataPtr,struct IFFHandle *, struct ContextNode *);

/*****************************************************************************/

#define	ASM	__asm
#define	REG(x)	register __ ## x

VOID ASM IntuiHook(REG(a0) struct Hook *hook, REG(a2) struct FileRequester *fr, REG(a1) struct IntuiMessage *intuiMsg);

/* stub.asm */
BOOL ASM bltbmrp (REG(a0) struct BitMap *, REG(d0) WORD srcx, REG(d1) WORD srcy, REG(a1) struct RastPort *, REG(d2) WORD dstx, REG(d3) WORD dsty, REG(d4) WORD w, REG(d5) WORD y, REG(d6) UBYTE min, REG(a6) struct Library *);
BOOL ASM bltbm   (REG(a0) struct BitMap *, REG(d0) WORD srcx, REG(d1) WORD srcy, REG(a1) struct BitMap *,   REG(d2) WORD dstx, REG(d3) WORD dsty, REG(d4) WORD w, REG(d5) WORD y, REG(d6) UBYTE min, REG(d7) UBYTE mask, REG(a2) UWORD *tempa, REG(a6) struct Library *);

/* magnify.asm */
void __asm MagnifyBitMap (register __a0 struct BitMap *, register __a1 struct BitMap *, register __d0 UBYTE, register __d1 UBYTE, register __d2 SHORT, register __d3 SHORT);

#include "wbpattern_iprotos.h"
