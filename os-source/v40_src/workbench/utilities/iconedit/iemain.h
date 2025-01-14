#ifndef IEMAIN_H
#define IEMAIN_H


/*****************************************************************************/


#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <dos/dos.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>
#include <workbench/workbench.h>

#include "dynamicimages.h"
#include "sketchpad.h"


/*****************************************************************************/

#define	MAXPENS	8

#define VNAM      "IconEdit"
#define	PREF_FILE "IconEdit.prefs"

#define VANILLA_COPY	0xC0
#define NO_MASK		0xFF
#define V(x)	((void *)x)

#define IE_CLEAR	1
#define IE_UNDO		2
#define IE_PALETTE	3
#define IE_SELECT	4
#define IE_REPEAT	5
#define	IE_TOOL         100
#define IE_UP		200
#define	IE_RIGHT	201
#define IE_DOWN		202
#define IE_LEFT		203

#define MAXDEPTH	8	/* Max depth supported (256 colors) */

/*  icon width & height: */
#define ICON_WIDTH	80
#define ICON_HEIGHT	40
#define ICON_SWIDTH     78
#define ICON_SHEIGHT    39

/*  Top Left coordinate of sketchpanel: */
#define PANELLEFT	10
#define PANELTOP	3

/*  Sketchpanel scale */
#define XSCALE	4
#define YSCALE	4

/*  Position and size of icon image repeat window: */
#define REPEATLEFT	428
#define	IMG1TOP		4
#define	IMG2TOP		74

/*  Position and size of palette: */
#define PALETTELEFT	335
#define PALETTETOP	19
#define MAXPALWIDTH	75
#define MAXPALHEIGHT	73


struct ZoomBox
{
    WORD LeftEdge;
    WORD TopEdge;
    WORD Width;
    WORD Height;
};

#define	FR_ICON	0
#define	FR_CLIP	1
#define	FR_ILBM	2
#define	FR_ALT	3
#define	FR_C	4
#define	FR_MAX	5

/* Our 'global' variables */
struct WindowInfo
{
    struct Screen *mysc;		/* Our screen pointer (temp) */
    struct Window *win;			/* Our window POINTER */
    struct DrawInfo *w_DrInfo;		/* DrawInfo */
    VOID *vi;				/* Visual information */
    UWORD topborder;			/* Topborder of window */
    struct TextAttr mydesiredfont;	/* TextAttr of font to use */
    struct TextFont *font;		/* Font to use */

    /* our tool icon */
    struct DiskObject *wi_ToolDOB;	/* Tool DiskObject */

    struct DiskObject *diskobj;
    UBYTE iconname[512];
    UWORD type;
    UWORD hilite;
    WORD changed;			/* Has the item been edited? */
    BOOL clippable;

    /* DataTypes stuff */
    struct Library	*w_DataTypesBase;
    struct Hook		 w_Filter;
    Object		*w_Object;
    UBYTE		 w_ColorTable[MAXPENS];

    /* Intuition stuff */
    struct Menu *menu;
    struct Gadget *firstgad;
    UBYTE w_Depth;			/* Depth of the icon */

    struct DynamicImage images[4];
    struct RastPort *draw_rp;
    WORD currentpen;
    SHORT currentwin;
    SHORT wintopx[2];
    UBYTE wintitle[512];
    struct Gadget *Active_Gad;
    struct Gadget *sketchgad;
    struct Gadget *mxgad;
    struct Gadget *img1;
    struct Gadget *img2;
    struct Gadget *undoGad;
    struct Gadget *clearGad;
    struct Gadget *LarGad;
    struct Gadget *RarGad;
    struct Gadget *UarGad;
    struct Gadget *DarGad;
    struct Gadget *toolGad;
    struct SketchPad *wi_sketch;
    struct DynamicImage *Adi;
    WORD w_LO;				/* Left offset */
    WORD w_Sprite;			/* Which sprite */
    UBYTE w_Buffer[512];		/* Work buffer */
    UBYTE w_Tmp[512];			/* File name temporary buffer */

    /* Application Window stuff */
    struct AppWindow *aw;
    struct MsgPort *msgport;
    BPTR wi_OriginalCD;
    BPTR wi_CurrentCD;

    /* Activation stuff */
    struct MsgPort *actvport;

    struct FileRequester *w_FR[FR_MAX];	/* FileRequester allocation */

    /* Clipboard stuff */
    struct IFFHandle * w_Clipper;

    BOOL w_SaveSrc;

    /* Preference items */
    UWORD w_ClipUnit;			/* Clipboard unit */
    UBYTE w_Palette[260];		/* Palette command */
    UBYTE w_ShowClip[260];		/* Show clipboard command */
    WORD LeftEdge, TopEdge;		/* dimensions of file requester window */
    WORD Width, Height;			/* dimensions of file requester window */
    WORD w_MagX, w_MagY;		/* Magnification */
    BOOL w_SaveIcons;			/* Save icons? */
    BOOL w_UseGrid;			/* Use the grid? */
    BOOL w_RemapImage;			/* Remap imports? */
    BOOL w_RemapClipboard;		/* Remap paste? */
    struct ZoomBox w_Zoom;		/* Zoom placement */
};
typedef struct WindowInfo * WindowInfoPtr;

/* No changes */
#define	CH_NONE		0x0000

/* Type or Highlight changed */
#define	CH_MINOR	0x0001

/* Remap done */
#define	CH_REMAP	0x0002

/* Image has been edited */
#define	CH_MAJOR	0x0004

/* Cancel selected */
#define	CH_CANCEL	0x0008

struct PrefRec
{
    LONG p_Unit;			/* Clipboard unit */
    UBYTE p_Palette[260];		/* Palette command */
    UBYTE p_ShowClip[260];		/* Show clipboard command */
    WORD p_LeftEdge, p_TopEdge;		/* dimensions of file requester window */
    WORD p_Width, p_Height;		/* dimensions of file requester window */
    WORD p_MagX, p_MagY;		/* Magnification */
    BOOL p_SaveIcons;			/* Save icons? */
    BOOL p_UseGrid;			/* Use the grid? */
    struct ZoomBox p_Zoom;		/* Zoom placement */
};


/*****************************************************************************/


#define IDCMPS (REFRESHWINDOW|CLOSEWINDOW|SKETCHPAD|PALETTEIDCMP|MENUPICK|RAWKEY|VANILLAKEY)
#define SHIFTED (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)
#define ALTED (IEQUALIFIER_LALT | IEQUALIFIER_RALT)


/*****************************************************************************/


VOID SetIconEditPrefs(WindowInfoPtr wi);


/*****************************************************************************/


#endif /* IEMAIN_H */
