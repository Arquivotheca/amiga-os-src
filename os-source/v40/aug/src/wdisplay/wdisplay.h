/* wdisplay.h
 * Include file for WDisplay
 * Written by David N. Junod
 *
 */

#define	AA		TRUE
#define	TRUECOLOR	FALSE
#define	DOBACKDROP	TRUE

#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <dos/dosextens.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <libraries/compat.h>
#include <libraries/gadtools.h>
#include <devices/keymap.h>
#include <devices/inputevent.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <graphics/text.h>
#include <graphics/displayinfo.h>
#include <graphics/scale.h>
#include <graphics/sprite.h>
#include <graphics/view.h>
#include <devices/console.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <libraries/iffparse.h>
#include <libraries/compat.h>
#include <iff/ilbm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <clib/macros.h>
#include <clib/compat_protos.h>
#include <clib/console_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/icon_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/intuition_protos.h>
#include <clib/locale_protos.h>
#include <clib/wb_protos.h>

#include <pragmas/diskfont_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/wb_pragmas.h>

#include "texttable.h"

#if DOBACKDROP
#define	TEMPLATE	"PICTURE,TITLE/K,PUBSCREEN/K,LEFT/N,TOP/N,WIDTH/N,HEIGHT/N,BACKDROP/S,SCALE/S,NOREMAP/S,MENUITEM/K"
#else
#define	TEMPLATE	"PICTURE,TITLE/K,PUBSCREEN/K,LEFT/N,TOP/N,WIDTH/N,HEIGHT/N,SCALE/S,NOREMAP/S,MENUITEM/K"
#endif
#define	OPT_FILE	0
#define	OPT_TITLE	1
#define	OPT_PUBSCREEN	2
#define	OPT_LEFT	3
#define	OPT_TOP		4
#define	OPT_WIDTH	5
#define	OPT_HEIGHT	6
#if DOBACKDROP
#define	OPT_BACKDROP	7
#define	OPT_SCALE	8
#define	OPT_NOREMAP	9
#define	OPT_MENUITEM	10
#define	OPT_COUNT	11
#else
#define	OPT_SCALE	7
#define	OPT_NOREMAP	8
#define	OPT_MENUITEM	9
#define	OPT_COUNT	10
#endif

extern struct Library *SysBase, *DOSBase, *IFFParseBase;
extern struct Library *IntuitionBase, *GfxBase, *DiskfontBase, *IconBase;
extern struct Library *KeymapBase, *WorkbenchBase, *LocaleBase;
extern struct ConsoleDevice *ConsoleDevice;

extern struct TextAttr Topaz80;

#define	BASENAME	"WDISPLAY"
#define	CATALOGNAME	"wdisplay.catalog"

#define	SYSGAD_WIDTH	54

#define	NO_TITLE	((UBYTE *)~0)

#define	IDCMP_FLAGS	IDCMP_RAWKEY | IDCMP_CLOSEWINDOW | \
			IDCMP_GADGETUP | IDCMP_GADGETDOWN | \
			IDCMP_NEWSIZE | \
			IDCMP_REFRESHWINDOW | \
			IDCMP_ACTIVEWINDOW | IDCMP_INACTIVEWINDOW | \
			IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE

#define	IDCMP_SLOW	IDCMP_RAWKEY | IDCMP_CLOSEWINDOW | \
			IDCMP_GADGETUP | IDCMP_GADGETDOWN | \
			IDCMP_ACTIVEWINDOW | IDCMP_INACTIVEWINDOW | \
			IDCMP_NEWSIZE | \
			IDCMP_REFRESHWINDOW | IDCMP_INTUITICKS | \
			IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE

#define	MAXCOLORS	256
#define	SQ(x)		((x)*(x))
#define	AVGC(i1,i2,c)	((ir->ir_GRegs[i1][c]>>1)+(ir->ir_GRegs[i2][c]>>1))
#define VANILLA_COPY	0xC0
#define NO_MASK		0xFF
#define	MAXSRCPLANES	24

/* Work structure */
typedef struct ILBMRec
{
    /* Bitmap for picture */
    ULONG ir_ModeID;		/* Display mode id */
    struct BitMapHeader ir_BMHD;/* BitMap header from ILBM */
    struct BitMap *ir_BMap;	/* Bitmap */

    /* Color information */
    struct ColorRegister ir_Colors[MAXCOLORS];
    WORD ir_RGB4[32];		/* Color table used by LoadRGB4() */
    WORD ir_NumColors;		/* Number of colors in color table */
    WORD ir_NumAlloc;		/* Number of colors allocated */
    LONG ir_CRegs[256][3];	/* Color table */
    LONG ir_GRegs[512][3];	/* Colors that we got */
    UBYTE ir_ColorTable[256];	/* Remap table 1 */
    UBYTE ir_ColorTable2[256];	/* Remap table 2 */
    UBYTE ir_Allocated[512];	/* Allocation table */

    /* Additional IFF information */
    struct Point2D ir_Grab;	/* GRAB coordinates */
    STRPTR ir_Name;		/* NAME of the picture */

} ILBM;

#define	BPR(w)			((w) + 15 >> 4 << 1)
#define MaxPackedSize(rowSize)  ((rowSize) + (((rowSize)+127) >> 7 ))
#define	RowBytes(w)		((((w) + 15) >> 4) << 1)
#define	ChunkMoreBytes(cn)	(cn->cn_Size - cn->cn_Scan)
#define UGetByte()		(*source++)
#define UPutByte(c)		(*dest++ = (c))


#define	V(x)	((VOID *)(x))
#define	ALT	(IEQUALIFIER_RALT | IEQUALIFIER_LALT)
#define	SHIFT	(IEQUALIFIER_RSHIFT | IEQUALIFIER_LSHIFT)

struct AppInfo
{
    ILBM *ai_Picture;			/* Current picture */

    /* Arguments */
    struct RDArgs *ai_ArgsPtr;		/* ReadArgs pointer */
    LONG ai_Options[OPT_COUNT];		/* Option buffers */

    /* Screen information */
    struct Screen *ai_Screen;		/* Screen that we're opened on */
    struct TextFont *ai_TextFont;	/* Screen font */
    struct DisplayInfo ai_Disp;		/* Screen display information */
    LONG ai_Colors[512];		/* Colors used for the screen */
    UBYTE *ai_ScreenName;		/* Public screen name. */
    struct DrawInfo *ai_DI;		/* DrawInfo */
    struct MsgPort *ai_IDCMP;		/* Message port */

    /* Main window information */
    struct Window *ai_Window;		/* Document window */
    UBYTE ai_ErrorTitle[128];		/* Buffer for error title */
    struct Gadget ai_Gads[8];		/* Document window gadgets */
    struct Gadget *ai_CurGad;		/* Current gadget */
    VOID *ai_Cursor;			/* Bounding box */
    struct IBox *ai_Box;		/* Bounding box */
    ULONG ai_DSecs, ai_DMics;		/* For Gadget DoubleClick */
    WORD ai_MouseX, ai_MouseY;		/* Current mouse information */

    /* Information window */
    struct Window *ai_InfoWin;		/* Information window */
    WORD ai_InfoLeft, ai_InfoTop;	/* Remember where it was */

    /* AppWindow stuff */
    struct AppWindow *ai_AW;
    struct AppMenuItem *ai_AM;
    struct MsgPort *ai_AWMPort;
    BPTR ai_AWOldDir;			/* Prior directory */
    BPTR ai_AWNewDir;			/* Directory that project resides in */
    UBYTE ai_ProjName[128];		/* Project name */

    /* Scaling information */
    LONG ai_WidthS;			/* Width of image */
    LONG ai_HeightS;			/* Height of image */
    struct BitMap *ai_BMapS;		/* Bitmap */
    ULONG ai_XFactor;
    ULONG ai_YFactor;
    LONG ai_PixelW;			/* Pixel width */
    LONG ai_PixelH;			/* Pixel height */
    LONG ai_SLeft;			/* Left edge */
    LONG ai_STop;			/* Top edge */
    LONG ai_SWidth;			/* Width */
    LONG ai_SHeight;			/* Height */

    /* Vertical */
    struct PropInfo ai_PIV;		/* Slider PropInfo */
    struct Image ai_SIV;		/* Scroller image */
    UWORD ai_PVFlags;
    WORD ai_MouseYD;			/* Mouse at Down */
    LONG ai_Rows;
    LONG ai_CurRow;
    LONG ai_OCurRow;

    /* Horizontal */
    struct PropInfo ai_PIH;		/* Slider PropInfo */
    struct Image ai_SIH;		/* Scroller image */
    UWORD ai_PHFlags;
    WORD ai_MouseXD;			/* Mouse at Down */
    LONG ai_Columns;
    LONG ai_CurColumn;
    LONG ai_OCurColumn;

    /* Bitmap for zooming */
    struct BitMap ai_BMapZ;		/* Zoom bitmap */

    /* Misc. */
    BOOL ai_Done;			/* Done with main loop yet? */
    ULONG ai_Flags;
    BPTR ai_Prefs;			/* Preference directory */
    UBYTE ai_Buffer[128];		/* Temporary work buffer */
    WORD ai_MLen;			/* Previous mode length */
    APTR ai_Catalog;			/* Localization catalog */
};

#define	ASIZE	sizeof(struct AppInfo)

/* Refresh the bitmap */
#define	AIF_REFRESH	0x00000001L

/* Scale the bitmap */
#define	AIF_SCALE	0x00000002L

/* We are in the process of scrolling */
#define	AIF_SCROLL	0x00000004L

/* Window is at maximum size */
#define	AIF_MAX		0x00000008L

/* We are zoomed in */
#define	AIF_ZOOMED	0x00000010L

/* We have a work bitmap */
#define	AIF_SECONDARY	0x00000020L

/* Window size has changed since last scale */
#define	AIF_CHANGED	0x00000040L

/* Error title is being displayed */
#define	AIF_ERROR	0x00000080L

/* Update display */
#define	AIF_UPDATE	0x00000100L

/* Run from Workbench */
#define	AIF_WORKBENCH	0x00000200L

/* Show that we have a zoom bitmap to work with */
#define	AIF_ZOOMBITMAP	0x00000400L

/* Show that we have a custom pointer */
#define	AIF_POINTER	0x00000800L

struct EMenu
{
    struct Menu em_Menu;		/* Embedded menu strip */
    LONG em_ID;				/* Localization ID */
    struct Image em_Bar;		/* Separator bar */
};

struct EMenuItem
{
    struct MenuItem em_MenuItem;	/* Embedded menu item */
    LONG (*em_Func)(struct Client *cl);	/* Menu function */
    LONG em_ID;				/* Localization ID */
    struct IntuiText em_Label;		/* Embedded label */
};

#include "wdisplay_protos.h"

/* debug.lib */
void kprintf(void *, ...);

/* bm_xlate.asm */
void __asm XLateBitMap(register __a0 struct BitMap *sbm,
			register __a1 struct BitMap *dbm,
			register __a2 char *table1,
			register __a3 char *table2,
			register __d0 ULONG width);

/* V39 graphics.library */
#if 0
ULONG PickBestColor(struct ColorMap *cm, ULONG r, ULONG g, ULONG b, ULONG precision, ULONG maxcolor);
VOID SetRGB32(struct ViewPort *vp, ULONG n, ULONG r, ULONG g, ULONG b);
GetRGB32(struct ColorMap *cm, ULONG firstcolor, ULONG ncolors, ULONG *table);
#endif

/* amiga.lib */
struct MsgPort *CreatePort (STRPTR, LONG);
VOID DeletePort (struct MsgPort *);
