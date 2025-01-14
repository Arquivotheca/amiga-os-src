#define	RETURN_KEY	10

struct TextInfo
{
    UBYTE *Buffer;		/* address of buffer */
    UBYTE *UndoBuffer;		/* undo buffer */
    WORD BufferPos;		/* cursor position */
    WORD MaxChars;		/* maximum characters in string */
    WORD DispPos;		/* UNUSED: position of first displayed */

    WORD UndoPos;		/* UNUSED: Character position in undo buffer */
    WORD NumChars;		/* Current length of string */
    WORD DispCount;		/* UNUSED: Number of whole characters */
    WORD CLeft, CTop;		/* UNUSED: TopLeft offset of container */

    struct StringExtend *Extension;	/* pointer to string extend */
    LONG LongInt;		/* UNUSED: */
    struct KeyMap *AltKeyMap;	/* alternate key map */

    /*--- custom portion ----------------------------------------------------*/
    WORD Flags;
    WORD LinesInBuffer;		/* # of text lines in the buffer */
    WORD LinesInDisplay;	/* # of lines in display region */
    WORD TopLineSync;		/* Repeat of ScrollPosition */
    WORD ScrollPosition;	/* amount gadget is scrolled */
    WORD FirstVisible;		/* index of first visible character */
    WORD LastVisible;		/* index of last visible character */
    WORD AnchorPos;		/* for highlighting */
    UBYTE HighPens[2];		/* Pens used for highlighting */
    UBYTE CursorPens[2];	/* Pens used for cursor */
    struct IBox Domain;		/* Domain of gadget */
    struct StringExtend SE;	/* Embedded string extension */
    WORD reserved[4];		/* for future expansion */
    UBYTE *ClipBuffer;		/* address of clip buffer */
};

#define TXT_NOWRAP	(1<<0)	/* disable word wrap */
#define TXT_ABLEHIGH	(1<<1)	/* enable mouse selection */
#define TXT_HIGHLIGHT	(1<<2)	/* text is currently highlighted */
#define	TXT_MYBUFFER	(1<<3)	/* We allocated buffer */
#define	TXT_READONLY	(1<<4)	/* Display is readonly */
#define	TXT_MYUNDO	(1<<5)	/* We allocated undo buffer */

/* for text editing hooks... */
struct TGWork
{
    /* set up when gadget is first activated */
    struct Gadget *Gadget;	/* the contestant itself */
    struct TextInfo *TextInfo;	/* easy access to sinfo */
    UBYTE *WorkBuffer;		/* intuition's planned result */
    UBYTE *PrevBuffer;		/* what was there before */
    ULONG Modes;		/* current mode */

    /* modified for each input event */
    struct InputEvent *IEvent;	/* actual event: do not change */
    UWORD Code;			/* character code, if one byte */
    WORD BufferPos;		/* cursor position */
    WORD NumChars;
    ULONG Actions;		/* what Intuition will do */
    LONG LongInt;		/* temp storage for longint */

    struct GadgetInfo *GadgetInfo;
    UWORD EditOp;		/* from constants below */

    /*--- custom portion -------------------------------------------------*/
    WORD AnchorPos;		/* highlight anchor position */
    WORD CurX, CurY;		/* current cursor XY position */
    WORD SeekX;			/* x seek position */
    WORD Dragging;		/* currently dragging cursor */
};

#define SGA_FIX	(0x80L)		/* set seek column to here...	 */

/* No semaphore need, because only one gadget could be active at a time */
struct MTextG
{
    struct TGWork tgWork;	/* TextGadget Working structure */
    UBYTE UndoBuffer[512];	/* Shared undo buffer */
    struct Hook EditHook;	/* Main editing hook */
};

#define RAW_Q			0x10
#define RAW_X			0x32
#define	RAW_C			0x33
#define	RAW_V			0x34
#define	RAW_E			0x12
#define	RAW_U			0x16
#define RAW_BKSP		0x41
#define	RAW_TAB			0x42
#define	RAW_ESC			0x45
#define RAW_DEL			0x46
#define	RAW_ENTER		0x43
#define RAW_RETURN		0x44
#define RAW_LEFT		0x4f
#define RAW_RIGHT		0x4e
#define RAW_UP			0x4c
#define RAW_DOWN		0x4d
#define	RAW_HELP		0x5f
