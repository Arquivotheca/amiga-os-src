
#ifndef	ZAPHOD_H
#define	ZAPHOD_H

/* **************************************************************************
 * 
 * The primary include file for the video work of the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY     Name           Description
 * ----------- -------------- --------------------------------------------
 * 25 Jan 88   -RJ            Added mouse.com support (labelled with MOUSE)
 * 15 Dec 86   RJ             Added MANX and AZTEC definitions
 * 22 Feb 86   =RJ Mical=     Created this file!
 *
 * *************************************************************************/

/* Comment out the following lines if Manx Aztec C isn't being used. */
#define MANX
#define AZTEC


/* Comment out the following line when not making a real release */
/*???#define RELEASE*/



#ifndef AZTEC
#include "include.h"
#endif


/*???#include <janus/janusbase.h>*/
/*???#include <janus/janusreg.h>*/
/*???#include <janus/services.h>*/



/* === Oh, can it be true? =============================================== */
#define JANUS		TRUE
/*???#define OLD_HARDWARE    TRUE*/



/* === MouseFlags Definitions ============================================ */
#define PC_HAS_MOUSE	0x0001



/* === Menu Definitions ================================================== */
#define MENU_HEADERS_COUNT	3

/*???ADJUST KEY TIMING*/
#define PROJECT_ITEMS_COUNT	6
#define EDIT_ITEMS_COUNT	2
#define DISPLAY_ITEMS_COUNT	13

#define PROJECT_MENU		0
#define EDIT_MENU		1
#define DISPLAY_MENU		2

/*???ADJUST KEY TIMING*/
#define SAVE_ITEM		0
#define RESTORE_ITEM		1
#define HELP_ITEM		2
#define ADJUST_ITEM		3
#define ABOUT_ITEM		4
#define CLOSE_ITEM		5

#define COPY_ITEM		0
#define PASTE_ITEM		1

#define FULLSIZE_ITEM		0
#define SMALLSIZE_ITEM		1
#define SHOWBORDER_ITEM		2
#define HIDEBORDER_ITEM		3
#define FREEZE_ITEM		4
#define LOCALE_ITEM		5
/*???#define MOUSEOWNER_ITEM		6*/
#define COLOR_ITEM		6
#define CURSOR_ITEM		7
#define NEWWINDOW_ITEM		8
#define REFRESH_ITEM		9
#define DEPTH_ITEM		10
#define PRIORITY_ITEM		11
#define INTERLACE_ITEM		12

#define HELP_SUBITEMS_COUNT	7


#define SAFE_HEIGHT	   9



/* === Aux Tools Manager Stuff =========================================== */
struct AuxToolsMessage
    {
    struct IORequest ExecRequest;
    SHORT AuxType;
    LONG AuxArg;
    };

#define AUX_TOOLS_PORT	"FakeZaphodLibPort.2.Really"
#define AUX_TASK_NAME	"FakeZaphodLibTask.2.Really"


/* These are the TalkWithZaphod definitions */
#define ADD_DISPLAY		1
#define REMOVE_DISPLAY		2
#define GET_DISPLAYLIST 	3
#define NEW_PRIORITY		4
#define GET_QUEUEROUTINE 	5
#define DEADEN_BLINK		6
#define REVIVE_BLINK		7
/*???#define TOGGLE_PCMOUSE		8*/
/*???#define CLEAR_PCMOUSE		9*/
/*NOSCROLL #define GET_SCROLLFLAGS		10*/
#define COUNT_WBWINDOWS		11
#define CHANGE_WBWINDOWS	12
/*NOSCROLL #define SCROLL_THAT_SUCKER	32767*/



/* === ScreenExtension Definitions ======================================= */
struct ScreenExt
    {
    struct ScreenExt *NextScreenExt;

    struct Screen *Screen;
    SHORT Flags;
    SHORT UserCount;
    };

/* === Flags Definitions === */
#define PRIVATE_SCREENING	0x0001



/* === WindowExtension Definitions ======================================= */
/* After a window is opened, one of these extension structures is allocated 
 * and attached to the window using the window's UserData field.
 */
struct WindowExtension
    {
    SHORT Flags;
    struct Window *Window;	/* Points back to the window */
    struct Region *Region;	/* Used for this window's damagelist stuff */
    struct Display *Display;	/* Points to this window's display */
    };

/* === Flags Definitions === */
#define RJ_IS_GROOVY	0x0001



/* === Locking Protocol Definitions ====================================== */
struct Lock 
    {
    struct Task *LockingTask;	/* Which task has the current lock */
    SHORT LockCount;		/* How many times the task has locked */
    SHORT LockWanted;		/* How many tasks are waiting for this lock */
    };



/* === Cursor Structure Definitions ====================================== */
/* These are the definitions for the CursorFlags variable 
 *
 * Note that the CURSOR_MOVED flag is multi-purpose.  The cursor task 
 * recognizes this flag to mean that the cursor has moved in the text grid 
 * or alternatively that the cursor has moved in the display grid (because,
 * for instance, the user has made the borders disappear, or has moved
 * one of the proportional gadgets).  This flag is also used by the CRT()
 * routine when the size of the cursor has changed (OK, so that's stretching
 * the concept of "cursor moved" a bit far, so I'm sorry). 
 */ 
#define CURSOR_ON	    0x0001
#define CURSOR_MOVED	    0x0002  /* Cursor has moved.  See note above! */ 
#define CURSOR_INACTIVE     0x0004

#define DEFAULT_CURSOR_SECONDS	0
#define DEFAULT_CURSOR_MICROS	(1000000 / 4)

#define CURSORPRIORITY_OFFSET	0



/* === Border Structure Definitions ====================================== */
struct BorderKontrol
    {
    SHORT Left, Top, Right, Bottom;
    UBYTE *Title;
    struct Gadget *GadgetList;
    };

/* These are the possible selectType args passed to BorderPatrol() */
#define BORDER_TOGGLE	1
#define BORDER_ON	2
#define BORDER_OFF	3



/* === System Macros ===================================================== */
#define SetFlag(v,f)		((v)|=(f))
#define ClearFlag(v,f)		((v)&=~(f))
#define ToggleFlag(v,f) 	((v)^=(f))
#define FlagIsSet(v,f)		((BOOL)((v&f)!=0))
#define FlagIsClear(v,f)	((BOOL)((v&f)==0))
#define ABS(n)	    		(((n)<0)?-(n):(n))



/* === Abort() Definitions =============================================== */
#define ALERT_ABORT		0
#define ALERT_BAD_KEYS_FILE	1
#define ALERT_NO_SIGNALS	2
#define ALERT_INCOMPLETESYSTEM	3
#define ALERT_NO_MEMORY 	4
#define ALERT_NO_LIBRARY	5
#define ALERT_NO_NEWREGION	6
#define ALERT_NO_INPUTPORT	7
#define ALERT_NO_PCFONT 	8
#define ALERT_NO_TIMERPORT	9
#define ALERT_BAD_UNLOCK_TASK	10
#define ALERT_NO_JANUS_SIG	11
#define ALERT_BAD_SETTINGS_FILE 12
#define ALERT_COPY_HELP		13
#define ALERT_DUPLICATE_DISPLAY	14
#define ALERT_BAD_SCANCODE_FILE	15
#define ALERT_FROZEN_BORDER	16
#define ALERT_FROZEN_TEXT	17

#define ALERT_RJ		18

#define RJ_WAS_HERE		19
#define ALERT_OLD_KEYSFILE	20



/* === Data Register Definitions ========================================= */
#define IOREG_OFFSET	(IoAccessOffset + IoRegOffset)

#define COLOR_CRT_OFFSET	    (IOREG_OFFSET + jio_ColorData)
#define MONO_CRT_OFFSET 	    (IOREG_OFFSET + jio_MonoData)
#define COLOR_CONTROL_OFFSET	    (IOREG_OFFSET + jio_ColorControlReg)
#define COLOR_SELECT_OFFSET	    (IOREG_OFFSET + jio_ColorSelectReg)
#define COLOR_STATUS_OFFSET	    (IOREG_OFFSET + jio_ColorStatusReg)
#define PCINTGEN_OFFSET 	    (IOREG_OFFSET + jio_PcIntGen)

#define COLOR_OFFSET		    (ColorOffset)
#define MONO_OFFSET		    (MonoVideoOffset)
#define GRAPHIC_ACCESS_OFFSET	    (GraphicAccessOffset)
#define BYTE_ACCESS_OFFSET	    (ByteAccessOffset)

#define PCKEYIRQOFFSET (IoAccessOffset + IoRegOffset + jio_PcIntReq)
#define PCKEYIRQMASK   (0x01)


/* These are the byte offsets from the base address to the register */
#define CRT_R0		0
#define CRT_R1		2
#define CRT_R2		4
#define CRT_R3		6
#define CRT_R4		8
#define CRT_R5		10
#define CRT_R6		12
#define CRT_R7		14
#define CRT_R8		16
#define CRT_R9		18
#define CRT_R10 	20
#define CRT_R11 	22
#define CRT_R12 	24
#define CRT_R13 	26
#define CRT_R14 	28
#define CRT_R15 	30
#define CRT_IOBLOCKSIZE 32  /* This *must* be an even number */

/* These are the masks of the register information that interests us */
#define CRT_R1MASK	0xFF
#define CRT_R6MASK	0x7F
#define CRT_R8MASK	0xF3
#define CRT_R9MASK	0x1F
#define CRT_R10MASK	0x7F
#define CRT_R11MASK	0x1F
#define CRT_R12MASK	0x3F
#define CRT_R13MASK	0xFF
#define CRT_R14MASK	0x3F
#define CRT_R15MASK	0xFF
#define CRT_CCMASK	0x33
#define CRT_CSMASK	0x3F

/* Here are some individual masks and definitions of the values found in
 * the various registers
 */
#define VIDEO_MASK	    0x03
#define VIDEO_INTERLACE     0x01
#define VIDEO_COMPRESS	    0x03

#define CURSOR_BLINK_MASK   0x60
#define CURSOR_STEADY	    0x00
#define CURSOR_OFF	    	0x20
#define CURSOR_BLINK16	    0x40
#define CURSOR_BLINK32	    0x60

#define CURSOR_START_MASK   0x1F



/* === KeyFlags Definitions ============================================== */
#define KEY_ALTKEY	    	0x0001
#define KEY_AMIGA	    	0x0002
#define KEY_AMIGAPENDING    	0x0004
#define KEY_NUMLOCK_SET		0x0008

#define ALTKEYS     (ALTLEFT | ALTRIGHT)

/* The key output buffer size should be a power of two, to allow fast
 * mask operation on the buffer counter variables (avoids a division
 * every time a keystroke is queued and another division every time a
 * keystroke is sent from the buffer to the PC).
 * Need 2 entries in the KeyBuffer for each keystroke (up and down).  
 * The PC's buffer is 16 wide.  I'm always typing to the end of the
 * darned buffer.  How big shall our buffer be?
 */
#define KEYBUFFER_SIZE	    512
#define KEYBUFFER_MASK	    (KEYBUFFER_SIZE - 1)




/* === Color & Mono Display Modes Definitions ============================ */
#define MEDIUM_RES	0x0001	/* 320 columns. Not set = 640 column mode */
#define TEXT_MODE	0x0002	/* Text mode.  Not set = then graphics mode */
#define SELECTED_AREA 	0x0004  /* Text area selected currently */
#define BLINK_TEXT	0x0008	/* 1 = text blinks.  0 = 16-color background */
#define PALETTE_1	0x0010	/* For medium-res graphics palette 0 or 1 */
#define OPEN_SCREEN	0x0020	/* 1 = Wants own screen.  0 = Use Workbench */
#define NEWSIZE_REFRESH 0x0040	/* 1 = Refresh when NEWSIZE event received */
#define SELECT_PRESSED	0x0080  /* Set when the select button is pressed */
#define BORDER_VISIBLE	0x0100
#define GOT_NEWSIZE	0x0200
#define PAL_PRESENCE	0x0400
#define COLOR_DISPLAY	0x0800	/* 1 = color display.  0 = mono */
#define VERBOSE 	0x1000	/* For verbose Zaphod, else shhhhh */
#define SQUELCH_NEWSIZE	0x2000
#define PROP_ACTIVE	0x4000  /* Proportional gadget active */
#define COUNT_DISPLAY	0x8000

#define PRIMARY_ATTRIBUTES (MEDIUM_RES | TEXT_MODE | PALETTE_1)


#define COLOR_MIN_INTENSITY	3
#define COLOR_LOW_INTENSITY	10
#define COLOR_HIGH_INTENSITY	15
#define MIN_RED 		(COLOR_MIN_INTENSITY << 8)
#define MIN_GREEN		(COLOR_MIN_INTENSITY << 4)
#define MIN_BLUE		(COLOR_MIN_INTENSITY << 0)
#define LOW_RED 		(COLOR_LOW_INTENSITY << 8)
#define LOW_GREEN		(COLOR_LOW_INTENSITY << 4)
#define LOW_BLUE		(COLOR_LOW_INTENSITY << 0)
#define HIGH_RED		(COLOR_HIGH_INTENSITY << 8)
#define HIGH_GREEN		(COLOR_HIGH_INTENSITY << 4)
#define HIGH_BLUE		(COLOR_HIGH_INTENSITY << 0)

#define HORIZ_GADGET	  1
#define VERT_GADGET	  2



/* === Color & Mono Display Modes2 Definitions ============================ */
#define SCROLLING	 0x0001	/* Set when in the middle of a scroll op */
#define WBCOLORS_GRABBED 0x0002
#define WINDOW_PRESETS	 0x0004
/*???#define PCMOUSE_TEST	0x8000*/



/* === Text Routines Definitions ======================================= */
/* These are here mostly as placeholders.  If any of these definitions
 * needs to be changed, the text routines may have to be reworked since
 * some of the shortcuts depend on these values.  See the comments in
 * the text routines for more details.
 */
#define BUFFER_WIDTH	80	/* For output text, a multiple of 20 */
#define CHAR_HEIGHT	8	/* To change this one won't be too bad */
#define CHAR_WIDTH	8	/* To change this would be horrendous */

#define CHAR_BASEROW	  6
#define CHAR_BASECOL	  0




/* === Primary Display List structure ================================== */
struct DisplayList
{
    struct Display *FirstDisplay;
    struct Lock DisplayListLock;

    struct ScreenExt *FirstScreenExt;
    struct Lock ScreenListLock;
};




/* === The Grand Display Structure ===================================== */
struct Display
{
    struct Display *NextDisplay;

    struct SuperWindow *FirstWindow;

    /* This variable is set by the task when it starts to run, and cleared
     * when she goes.
     */
    struct Task *TaskAddress;

    /* This is the address of the port where this task's front door.
     * It is here that this task will receive callers.
     */
    struct MsgPort *TaskPort;

    /* The buffer contains the Zaphod copy of the PC display data, whether that
     * data is text or graphics.
     */
    UBYTE *Buffer;
    struct Remember *BufferKey;

    /* Since the Modes variable is used so often, a copy of the active
     * window's modes variable is kept up here in the display structure.
     */
    SHORT Modes;
    SHORT Modes2;


    /* === Display Variables ========================================= */
    /* The "display" is the area of the window where the PC information is
     * rendered.  This area is necessarily equal to or smaller than the 
     * current size of the window.  Under normal circumstances, the display
     * is either the area within the window's borders or the entire window
     * when the borders have been hidden.
     */
    SHORT DisplayWidth, DisplayHeight;

    /* These variables describe the position of the top-left pixel of the 
     * visible display with respect to the true top-left corner (0,0) of 
     * the whole display.
     */
    SHORT DisplayStartCol, DisplayStartRow;

    ULONG OldSeconds, OldMicros;    /* Used for checking double-click */
    ULONG TripleSeconds, TripleMicros;	 /* Used when checking triple-click */


    /* === Text Buffer Variables ===================================== */
    UBYTE TextOutputBuffer[BUFFER_WIDTH];
    struct BitMap TextBitMap;
    UBYTE TextNormalPlane[BUFFER_WIDTH * CHAR_HEIGHT];
    UBYTE TextInversePlane[BUFFER_WIDTH * CHAR_HEIGHT];

    /* The following are text-grid oriented */
    SHORT TextStartCol, TextStartRow;
    SHORT TextWidth, TextHeight;
    SHORT TextNewStart[25];
    SHORT TextNewLength[25];

    /* These two variables are display-oriented.  They describe the
     * offset into the display (sually less than or equal to zero)
     * of the top-left character of the text grid.
     */
    SHORT TextBaseCol, TextBaseRow;

    /* All of the normal cursor variables are character-grid oriented */
    SHORT CursorTop;
    SHORT CursorBottom;
    SHORT CursorAltTop;
    SHORT CursorRow, CursorCol;
    SHORT CursorOldRow, CursorOldCol;
    SHORT CursorFlags;

    /* The "real" cursor variables describe the real window coordinates of
     * the cursor box(es)
     */
    SHORT CursorRealLeft;
    SHORT CursorRealRight;
    SHORT CursorRealTop;
    SHORT CursorRealBottom;
    SHORT CursorRealAltTop;
    SHORT CursorRealAltBottom;


    /* === Graphics variables ======================================= */

    /* The plane pointers are for blasting the graphics display into Zaphod
     * windows, and therefore are not used by Mono display, though they will
     * be initialized anyway (just in case we find some need for it someday).
     */
    struct BitMap BufferBitMap;
    

    /* === PC Interface Variables =================================== */

    UBYTE *JanusStart;

    /* The current address (including the current offset) of the PC
     * buffer (text or graphics).
     */
    UBYTE *PCDisplay;
    /* The address of the CRT registers */
    UBYTE *PCCRTRegisterBase;
    /* The offsets describe the byte offset from the base of the PC display
     * memory to the current start of display and cursor position.
     */
    SHORT PCDisplayOffset;
    SHORT PCCursorOffset;

    UBYTE ZaphodCRT[CRT_IOBLOCKSIZE];
    /* These are copies of the Color Display registers.  These registers
     * have no Monochrome equivalent.
     */
    UBYTE ColorSelect;
    UBYTE ColorControl;
		    

    /* === Locking mechanism variables ============================== */
    struct Lock DisplayLock;
       

    /* === Janus Sig Management ==================================== */
    struct SetupSig *DisplayWriteSig;
    struct SetupSig *CRTWriteSig;

    /* === Stash Workbench Colors ================================== */
    USHORT WBColors[4];

    /* === Presets ================================================= */
    SHORT PresetMonoWidth;
    SHORT PresetMonoHeight;
    SHORT PresetMonoTopEdge;
    SHORT PresetMonoLeftEdge;
    SHORT PresetMonoDepth;
    USHORT PresetMonoFlags;

    SHORT PresetColorWidth;
    SHORT PresetColorHeight;
    SHORT PresetColorTopEdge;
    SHORT PresetColorLeftEdge;
    SHORT PresetColorDepth;
    USHORT PresetColorFlags;
};

/* === PresetFlags Definitions (works for both Mono and Color) === */
#define PRESET_HIRES		0x0001
#define PRESET_BORDER_ON	0x0002
#define PRESETS_GOT		0x0004	/* This display mode's presets gotten */
#define PRESET_PRIVACY		0x0008



/* === The SuperWindow Structure ========================================= */
struct SuperWindow
/* This structure has the data that's specific to any given window in a
 * display.  The information here is only partially what's needed to 
 * create a window in the display.  The remainder of the information can be
 * found in the Display structure.  This data allows multiple windows to be
 * associated with a given display.  The Display structure has the data
 * required to support the display itself *and* the data that pertains to
 * the currently active window.
 */
{

    struct SuperWindow *NextWindow;

    SHORT DisplayModes;   /* The primary mode flags for the task */
    SHORT DisplayModes2;  /* The primary mode flags for the task */
    SHORT Flags;


    /* === Window/Screen Variables ================================= */
    struct NewWindow DisplayNewWindow; /* For this display's window */
    struct Window *DisplayWindow;      /* This display's window */
    struct NewScreen DisplayNewScreen; /* This display's screen (as needed) */
    struct Screen *DisplayScreen;      /* If this display opens a screen */
    struct Image HorizImage, VertImage;     /* Dummies for prop gadgets */
    struct PropInfo HorizInfo, VertInfo;    /* For prop gadgets */
    struct Gadget HorizGadget, VertGadget;  /* For prop gadgets */

    /* Every universe needs border control measures */
    struct BorderKontrol DisplayBorder;
				    
    /* These variables are used to record the most recent 
     * less-than-full-screen size and position of the window, in case the
     * user wants the window blown back out to that size (using either
     * triple-click or a menu option)
     */
    SHORT LastSmallX, LastSmallY; 
    SHORT LastSmallWidth, LastSmallHeight;


    /* This guy has the user's preference for text depth.  Note well that
     * this is for text only.  In Color Display mode, you don't get to 
     * specify the depth of graphics.
     */
    SHORT TextDepth;

    SHORT DisplayDepth;

    /* Each window gets to specify its own cursor rate */
    LONG CursorSeconds, CursorMicros;
		   
};

/* === Flags Definitions ==== */
#define VIRGIN_NEWWINDOW	0x0001 /* The NewWindow is unused */
#define WINDOW_WAS_HIRES	0x0002 /* Last window had been HIRES */
#define FROZEN_HOSEHEAD		0x0004 /* This superwindow is frozen */
#define WANTS_PRIVACY		0x0008 /* This superwindow gets its own */
#define HEIGHT_INITED		0x0010



/* === Aux Task Definitions =============================================== */
#define INPUT_TASK_NAME     "InputMonitorTask.2.Zaphod"
#define INPUT_PORT_NAME     "InputMonitorPort.2.Zaphod"
#define CURSOR_TASK_NAME    "CursorTask.2.Zaphod"

#define COLOR_CRT_NOTHING   1
#define COLOR_CRT_REVAMP    2
#define COLOR_CRT_REDRAW    3



/* === The definitions for ColorWindow =================================*/
#define COLOR_BOX_LEFT		7
#define COLOR_BOX_TOP		6
#define COLOR_BOX_RIGHT 	(COLOR_BOX_LEFT + 15)
#define COLOR_BOX_BOTTOM	(COLOR_BOX_TOP + 29)
#define COLOR_COLOR_TOP 	45

#define COLOR_KNOB_BODY 	0x1111

#define COLORWINDOW_WIDTH	208
#define COLORWINDOW_HEIGHT	91

#define CHARACTER_WIDTH 	8
#define CHARACTER_HEIGHT	8

/* GREEN and RED are out of order to facilitate the color cycle window sharing
 * these gadgets with the color palette window (color cycle uses the gadgets
 * only down to the COLOR_GREEN gadget)
 */
#define COLOR_COPY		0
#define COLOR_RANGE		1
#define COLOR_OK		2
#define COLOR_CANCEL		3
#define COLOR_GREEN		4
#define COLOR_RED		5
#define COLOR_BLUE		6
#define COLOR_HSL_RGB		7
#define COLOR_GADGETS_COUNT	8



/* === Ascii To PC Translation ========================================= */
#define PC_NOTHING 	0x0000
#define PC_CONTROL	0x0001
#define PC_SHIFT	0x0002



/* === Miscellaneous =================================================== */
#define	REGIST	REGISTER

#define DISPLAY_IDCMP_FLAGS (NEWSIZE | REFRESHWINDOW | MOUSEBUTTONS \
			      | GADGETDOWN | GADGETUP | MENUPICK \
			      | CLOSEWINDOW | RAWKEY | ACTIVEWINDOW \
			      | INACTIVEWINDOW | MOUSEMOVE | NEWPREFS) 

#define MILLION 1000000

#define ZAPHOD_HEIGHT	200

#define SHIFTY		(IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT \
				| IEQUALIFIER_CAPSLOCK)


#define TITLEBAR_HEIGHT		10

#define NTSC_TOP_ONHEIGHT		0
#define NTSC_TOP_OFFHEIGHT		0

#define NTSC_EXTRAHEIGHT		0

#define PAL_TOP_ONHEIGHT		11
#define PAL_TOP_OFFHEIGHT		11

#define PAL_EXTRAHEIGHT		(PAL_TOP_ONHEIGHT + 10)

#define PAL_MEDIUM_OFFSET	8
#define PAL_HIGH_OFFSET		7




/* === Function Definitions ============================================ */
UBYTE *AllocMem();
UBYTE *AllocRemember();

struct AuxToolsMessage *CreateExtIO();
struct MsgPort *CreatePort();
SHORT CreateSignalNumber();
struct IOStdReq *CreateStdIO();
struct Task *CreateTask();

BOOL DuplicateDisplay();

UBYTE *ExamineTextClip();

struct MsgPort *FindPort();
struct Screen *FindSharedScreen();
struct Task *FindTask();

struct Window *GetActiveWindow();
USHORT *GetColorAddress();
struct DisplayList *GetDisplayList();
struct Screen *GetExtraScreen();
struct IntuiMessage *GetMsg();
UBYTE *GetJanusStart();

struct MenuItem *ItemAddress();

BOOL Lock();

struct Screen *MakeExtraScreen();
UBYTE MakePCRawKey();

struct Region *NewRegion();

ULONG Open();
struct TextFont *OpenDiskFont();
struct TextFont *OpenFont();
UBYTE *OpenLibrary();
struct Screen *OpenScreen();
struct Screen *OpenSharedScreen();
BOOL OpenTextClip();
struct Window *OpenWindow();

BOOL ReadSettingsFile();
struct Window *RevampWindow();

struct SetupSig *SetupJanusSig();
SHORT StringLength();
SHORT StupidCreateSignalNumber();

LONG TalkWithZaphod();
SHORT TextXPos();
SHORT TextYPos();

ULONG ZAllocSignal();
SHORT ZGetChar();


/* === And Finally ===================================================== */
#ifndef EGLOBAL_CANCEL
#include "eglobal.c"
#endif


#endif	/* of #ifndef ZAPHOD_H */


