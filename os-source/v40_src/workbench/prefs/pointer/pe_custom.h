
/* includes */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/execbase.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/screens.h>
#include <datatypes/datatypes.h>
#include <graphics/text.h>
#include <graphics/monitor.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>
#include <workbench/workbench.h>
#include <prefs/pointer.h>
#include <prefs/prefhdr.h>
#include <string.h>

/* application includes */
#include "pointer_rev.h"
#include "pe_strings.h"

/*****************************************************************************/

/* Pref shell controls */
#define	DATATYPES	TRUE
#define	USES_OWN_SCREEN	TRUE

/*****************************************************************************/

#define	MAXWIDTH	32
#define	MAXHEIGHT	48
#define MAXDEPTH	4
#define MAXCOLORS	16
#define	MAXPENS		256
#define LOWBITSPERGUN	2
#define HIGHBITSPERGUN	8   /* only 8 because of CMAP IFF chunk */

/*****************************************************************************/

/* Fill-in these values for the editor */
#define PORT_NAME	"Pointer_Prefs"
#define ENV_NAME	"ENV:Sys/Pointer.prefs"
#define ARC_NAME	"ENVARC:Sys/Pointer.prefs"
#define PRESET_DIR	"SYS:Prefs/Presets"
#define PRESET_NAME	"Pointer.pre"

/*****************************************************************************/

struct ExtPrefs
{
    struct PointerPrefs		 ep_PP[2];
    struct BitMap		*ep_BMap;		/* Both pointers side by side */
    struct RastPort		 ep_RPort;		/* RastPort for rendering */
    struct RGBTable		 ep_CMap[MAXCOLORS];	/* Color table */
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

#define OPT_SCREEN	6
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

    /**************************************************************
     * the rest of this structure is editor-specific and is not
     * used by the common code
     **************************************************************/

    struct Library		*ed_DataTypesBase;
    struct Hook			 ed_FilterHook;
    Object			*ed_DataObject;

    struct PointerPrefs		 ed_PrefsTemp;
    struct BitMap		*ed_UndoBM;
    struct BitMap		*ed_TempBM;
    struct BitMap		 ed_WorkBM;
    struct RastPort		 ed_WorkRP;

    ULONG			 ed_ClipUnit;

    struct DisplayInfo		 ed_DispInfo;
    ULONG			 ed_ModeId;
    ULONG			 ed_SWidth;		/* Screen width */
    ULONG			 ed_SHeight;		/* Screen height */
    ULONG			 ed_WWidth;		/* Window width */
    ULONG			 ed_WHeight;		/* Window height */
    LONG			 ed_Pens[MAXPENS];
    UBYTE			 ed_ColorTable[MAXPENS];
    UWORD			 ed_PenOffset;
    UWORD			 ed_Pad;

    Class			*ed_SketchClass;

    /* Gadgets */
    struct Gadget		*ed_GSketch;
    struct Gadget		*ed_GTest;
    struct Gadget		*ed_GClear;
    struct Gadget		*ed_GSetPoint;
    struct Gadget		*ed_GResetColor;
    struct Gadget		*ed_GPalette;
    struct Gadget		*ed_GRed;
    struct Gadget		*ed_GGreen;
    struct Gadget		*ed_GBlue;

    struct Gadget		*ed_GType;		/* Type of pointer (0=Normal, 1=Busy) */
    struct Gadget		*ed_GSize;		/* Size of pointer (0=LORES, 1=HIRES, 2=SHIRES) */
    struct Gadget		*ed_GScanDbl;		/* Scan double */

    STRPTR			 ed_WhichLabels[3];
    STRPTR			 ed_SizeLabels[4];

    UWORD			 ed_Which;
    UWORD			 ed_Size;

    UWORD			 ed_RedBits;
    UWORD			 ed_GreenBits;
    UWORD			 ed_BlueBits;

    /* Current pen */
    UWORD			 ed_APen;

    /* Amount of magnification */
    UBYTE			 ed_XMag;
    UBYTE			 ed_YMag;

    /* Size of pointer */
    UWORD			 ed_Width;
    UWORD			 ed_Height;
    UWORD			 ed_Depth;

    ULONG			 ed_Flags;

    struct FileRequester *ed_ImageReq;
    char ed_ImageBuf[NAMEBUFSIZE];

    BOOL			 ed_RemapImage;

} EdData;

#define	EDF_OUR_PUBLIC	0x0001
#define	EDF_TESTED	0x0002
#define	EDF_WINDOW	0x0004
#define	EDF_2024	0x0008
#define	EDF_EXCLUSIVE	0x0010

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
#define	DataTypesBase ed->ed_DataTypesBase
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

    ES_REQUIRES_DATATYPES,
    ES_NOCLIPBOARD,
    ES_NOGRAPHIC,
    ES_NOT_PICTURE,

} EdStatus;

#define	DATATYPE_ERROR	MSG_PTR_NO_DATATYPES_LIBRARY

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
    EC_SET,
    EC_RESET,

    EC_PALETTE,

    EC_RED,
    EC_GREEN,
    EC_BLUE,

    EC_SKETCH,

    EC_TYPE,
    EC_SIZE,

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

typedef EdData *EdDataPtr;

typedef CmdState *CmdStatePtr;

typedef EdStatus (*IFFFunc)(EdDataPtr,struct IFFHandle *, struct ContextNode *);

/*****************************************************************************/

#define	ASM	__asm
#define	REG(x)	register __ ## x

/* stub.asm */
BOOL ASM bltbmrp  (REG(a0) struct BitMap *, REG(d0) WORD srcx, REG(d1) WORD srcy, REG(a1) struct RastPort *, REG(d2) WORD dstx, REG(d3) WORD dsty, REG(d4) WORD w, REG(d5) WORD y, REG(d6) UBYTE min, REG(a6) struct Library *);
BOOL ASM bltbm    (REG(a0) struct BitMap *, REG(d0) WORD srcx, REG(d1) WORD srcy, REG(a1) struct BitMap *,   REG(d2) WORD dstx, REG(d3) WORD dsty, REG(d4) WORD w, REG(d5) WORD y, REG(d6) UBYTE min, REG(d7) UBYTE mask, REG(a2) UWORD *tempa, REG(a6) struct Library *);
BOOL ASM clipblit (REG(a0) struct RastPort *, REG(d0) WORD srcx, REG(d1) WORD srcy, REG(a1) struct RastPort *, REG(d2) WORD dstx, REG(d3) WORD dsty, REG(d4) WORD w, REG(d5) WORD y, REG(d6) UBYTE min, REG(a6) struct Library *);

/* magnify.asm */
void __asm MagnifyBitMap (register __a0 struct BitMap *, register __a1 struct BitMap *, register __d0 UBYTE, register __d1 UBYTE, register __d2 SHORT, register __d3 SHORT);

/*****************************************************************************/

#include "pointer_iprotos.h"
