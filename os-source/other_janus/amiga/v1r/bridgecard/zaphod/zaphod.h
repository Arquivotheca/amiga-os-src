
/* ***************************************************************************
 * 
 * The primary include file for the video work of the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name		Description
 * -----------	--------------	--------------------------------------------
 * 22 Feb 86	=RJ Mical=	Created this file!
 *
 * **************************************************************************/


#include "exec/types.h"

#include "exec/memory.h"
#include "exec/tasks.h"
#include "intuition/intuition.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"
#include "graphics/display.h"
#include "graphics/gfxbase.h"
#include "graphics/regions.h"
#include "janus/janusreg.h"
#include "janus/services.h"
#include "janus/setupsig.h"



/* === Oh, can it be true? ================================================ */
#define JANUS		TRUE
/*???#define OLD_HARDWARE    TRUE*/

/*#define DIAG 	TRUE*/

/* === Menu Definitions =================================================== */
#define MENU_HEADERS_COUNT	2

#define PROJECT_ITEMS_COUNT	4
#define DISPLAY_ITEMS_COUNT	9

#define PROJECT_MENU		0
#define DISPLAY_MENU		1
				 
#define SAVE_ITEM		0
#define RESTORE_ITEM		1
#define INFO_ITEM		2
#define CLOSE_ITEM		3

#define SIZE_ITEM		0
#define BORDER_ITEM		1
#define COLOR_ITEM		2
#define CURSOR_ITEM		3
#define NEWWINDOW_ITEM		4
#define REFRESH_ITEM		5
#define DEPTH_ITEM		6
#define PRIORITY_ITEM		7
#define INTERLACE_ITEM		8


#define INFO_SUBITEMS_COUNT	6


#define SAFE_HEIGHT	   9




/* === Aux Tools Manager Stuff ============================================ */
struct AuxToolsMessage
    {
    struct IORequest ExecRequest;
    SHORT AuxType;
    LONG AuxArg;
    };

#define AUX_TOOLS_PORT	"FakeZaphodLibPort.Really"
#define AUX_TASK_NAME	"FakeZaphodLibTask.Really"


#define ADD_DISPLAY	1
#define REMOVE_DISPLAY	2
#define GET_DISPLAYLIST 3
#define NEW_PRIORITY	4



/* === Locking Protocol Definitions ======================================= */
struct Lock 
    {
    struct Task *LockingTask;	/* Which task has the current lock */
    SHORT LockCount;		/* How many times the task has locked */
    SHORT LockWanted;		/* How many tasks are waiting for this lock */
    };



/* === Cursor Structure Definitions ======================================= */
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



/* === Border Structure Definitions ======================================= */
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
#define SetFlag(v,f)		(v) |= (f)
#define ClearFlag(v,f)		(v) &= ~(f)
#define ToggleFlag(v,f) 	(v) ^= (f)
#define ABS(n)			((n < 0) ? -n : n)


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



/* === Data Register Definitions ========================================= */
#define IOREG_OFFSET	(IoAccessOffset + IoRegOffset)

#define COLOR_CRT_OFFSET	    (IOREG_OFFSET + jio_ColorData)
#define MONO_CRT_OFFSET 	    (IOREG_OFFSET + jio_MonoData)
#define COLOR_CONTROL_OFFSET	    (IOREG_OFFSET + jio_ColorControlReg)
#define COLOR_SELECT_OFFSET	    (IOREG_OFFSET + jio_ColorSelectReg)
#define COLOR_STATUS_OFFSET	    (IOREG_OFFSET + jio_ColorStatusReg)
#define KEYBOARD_OFFSET 	    (IOREG_OFFSET + jio_KeyboardData)
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
#define CURSOR_OFF	    0x20
#define CURSOR_BLINK16	    0x40
#define CURSOR_BLINK32	    0x60

#define CURSOR_START_MASK   0x1F



/* === KeyFlags Definitions ============================================== */
#define KEY_ALTKEY	    0x0001
#define KEY_AMIGA	    0x0002
#define KEY_AMIGAPENDING    0x0004

#define ALTKEYS     (ALTLEFT | ALTRIGHT)

/* The key output buffer size should be a power of two, to allow fast
 * mask operation on the buffer counter variables (avoids a division
 * every time a keystroke is queued and another division every time a
 * keystroke is sent from the buffer to the PC).
 * The PC's buffer is 16 wide.  I'm always typing to the end of the
 * darned buffer.  How big shall our buffer be?
 */
#define KEYBUFFER_SIZE	    64
#define KEYBUFFER_MASK	    (KEYBUFFER_SIZE - 1)




/* === Color & Mono Display Mode Definitions ============================= */
#define MEDIUM_RES	0x0001	/* 320 columns. Not set = 640 column mode */
#define TEXT_MODE	0x0002	/* Text mode.  Not set = then graphics mode */
#define unused0004	0x0004
#define BLINK_TEXT	0x0008	/* 1 = text blinks.  0 = 16-color background */
#define PALETTE_1	0x0010	/* For medium-res graphics palette 0 or 1 */
#define OPEN_SCREEN	0x0020	/* 1 = Wants own screen.  0 = Use Workbench */
#define NEWSIZE_REFRESH 0x0040	/* 1 = Refresh when NEWSIZE event received */
#define unused0080	0x0080
#define BORDER_VISIBLE	0x0100
#define GOT_NEWSIZE	0x0200
#define unused0400	0x0400
#define COLOR_DISPLAY	0x0800	/* 1 = color display.  0 = mono */
#define VERBOSE 	0x1000	/* For verbose Zaphod, else shhhhh */
#define unused2000	0x2000
#define unused4000	0x4000
#define unused8000	0x8000

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


#define DISPLAY_IDCMP_FLAGS (NEWSIZE | REFRESHWINDOW | MOUSEBUTTONS \
			      | GADGETDOWN | GADGETUP | MENUPICK \
			      | CLOSEWINDOW | RAWKEY | ACTIVEWINDOW \
			      | INACTIVEWINDOW) 

 

	

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
     * offset into the display (usually less than or equal to zero)
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

};




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

    SHORT DisplayModes;        /* The primary mode flags for the task */


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




/* === Aux Task Definitions =============================================== */
#define INPUT_TASK_NAME     "InputMonitorTask.Zaphod"
#define INPUT_PORT_NAME     "InputMonitorPort.Zaphod"
#define CURSOR_TASK_NAME    "CursorTask.Zaphod"

#define COLOR_CRT_NOTHING   1
#define COLOR_CRT_REVAMP    2
#define COLOR_CRT_REDRAW    3



/* === The definitions for ColorWindow =================================*/
#define COLOR_KNOB_BODY 	0x1111

#define COLORWINDOW_WIDTH	208
#define COLORWINDOW_HEIGHT	91

#define CHARACTER_WIDTH 	8
#define CHARACTER_HEIGHT	8

#define COLOR_BOX_LEFT		7
#define COLOR_BOX_TOP		6
#define COLOR_BOX_RIGHT 	(COLOR_BOX_LEFT + 15)
#define COLOR_BOX_BOTTOM	(COLOR_BOX_TOP + 29)
#define COLOR_COLOR_TOP 	45
#define COLOR_PROP_LEFT 	38
#define COLOR_PROP_TOP		4
#define COLOR_PROP_WIDTH	165
#define COLOR_PROP_HEIGHT	10
#define COLOR_CLUSTER_LEFT	141
#define COLOR_CLUSTER_TOP	41
#define COLOR_CLUSTER_WIDTH	(CHARACTER_WIDTH * 6 + 4)
#define COLOR_CLUSTER_HEIGHT	9
#define COLOR_HSL_TOP		(COLOR_BOX_TOP - 2)
#define COLOR_HSL_LEFT		(COLOR_BOX_RIGHT + 3)

/* GREEN and RED are out of order to facilitate the color cycle window sharing
 * these gadgets with the color palette window (color cycle uses the gadgets
 * only down to the COLOR_GREEN gadget)
 */
#define COLOR_COPY		00
#define COLOR_RANGE		01
#define COLOR_OK		02
#define COLOR_CANCEL		03
#define COLOR_GREEN		04
#define COLOR_RED		05
#define COLOR_BLUE		06
#define COLOR_HSL_RGB		07
#define COLOR_GADGETS_COUNT	08




/* === Function Definitions =============================================== */
UBYTE *AllocMem();
UBYTE *AllocRemember();
struct AuxToolsMessage *CreateExtIO();
struct MsgPort *CreatePort();
struct IOStdReq *CreateStdIO();
struct Task *CreateTask();
struct MsgPort *FindPort();
struct Task *FindTask();
USHORT *GetColorAddress();
struct DisplayList *GetDisplayList();
struct IntuiMessage *GetMsg();
UBYTE *GetJanusStart();
struct MenuItem *ItemAddress();
BOOL Lock();
UBYTE MakePCRawKey();
struct Region *NewRegion();
ULONG Open();
struct TextFont *OpenDiskFont();
struct Screen *OpenScreen();
struct Window *OpenWindow();
BOOL ReadSettingsFile();
struct Window *RevampWindow();
struct SetupSig *SetupJanusSig();
SHORT StringLength();
SHORT StupidCreateSignalNumber();
LONG TalkWithZaphod();
ULONG ZAllocSignal();
SHORT ZGetChar();



/* === And Finally ===================================================== */
#ifndef EGLOBAL_CANCEL
#include "eglobal.c"
#endif



