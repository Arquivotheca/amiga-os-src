
/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <devices/inputevent.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <workbench/workbench.h>
#include <libraries/gadtools.h>

/* prototypes */
#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/layers_protos.h>

/* direct ROM interface */
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/layers_pragmas.h>

/* application includes */
#include "sketchpad.h"
#include "iemessages.h"
#include "iemain.h"
#include "sp_magnify.h"
#include "ieutils.h"

/*****************************************************************************/

extern struct Library *SysBase;
extern struct Library *GfxBase;
extern struct Library *IntuitionBase;
extern struct Library *GadToolsBase;

/*****************************************************************************/

/* Private internal functions */
#define	NUMPOINTS 5
USHORT SPDither[] =
{0x5555, 0xAAAA};
USHORT SPVerBar[] =
{0x5555, 0x5555};

static VOID SPSetTile (SketchPadPtr sp);

/*****************************************************************************/

SketchPadPtr OpenSketchPad (NewSketchPadPtr nsp)
{
    SketchPadPtr sp;
    SHORT width;
    SHORT height;
    SHORT depth;

    if (sp = AllocVec (sizeof (struct SketchPad), MEMF_PUBLIC | MEMF_CLEAR))
    {
	sp->Window = nsp->Window;
	sp->MagX = nsp->MagX;
	sp->MagY = nsp->MagY;
	sp->Drp = nsp->Window->RPort;
	sp->LeftEdge = nsp->LeftEdge;
	sp->TopEdge = nsp->TopEdge;
	sp->Width = nsp->Width;
	sp->Height = nsp->Height;
	sp->Depth = nsp->Depth;
	sp->Flags = nsp->Flags;
	sp->APen = nsp->APen;
	sp->BPen = nsp->BPen;
	sp->Draw_Mode = nsp->Draw_Mode;
	sp->Flood_Mode = 1;
	sp->Visual = nsp->Visual;
	sp->Gad.LeftEdge = sp->LeftEdge;
	sp->Gad.TopEdge = sp->TopEdge;
	sp->Gad.Width = (SHORT) (nsp->Width * sp->MagX);
	sp->Gad.Height = (SHORT) (nsp->Height * sp->MagY);
	sp->Gad.Flags = GADGHNONE;
	sp->Gad.Activation = GADGIMMEDIATE | RELVERIFY | FOLLOWMOUSE;
	sp->Gad.GadgetType = BOOLGADGET;
	sp->Gad.GadgetID = SPSKETCHPAD_ID;
	sp->Gad.UserData = sp;

	if (sp->Flags & SP_GRID)
	    sp->Grid = TRUE;

	width = (sp->Width * sp->MagX);
	height = (sp->Height * sp->MagY);
	depth = sp->Depth;

	InitDynamicImage (&sp->Brush, MAXDEPTH, width, height);
	InitDynamicImage (&sp->Backup, MAXDEPTH, width, height);
	InitDynamicImage (&sp->Undo, MAXDEPTH, width, height);

	InitRastPort (&sp->Wrp);
	InitBitMap (&sp->Wbm, depth, width, height);
	sp->Wrp.BitMap = &sp->Wbm;

	while (depth-- > 0)
	{
	    if (!(sp->Wbm.Planes[depth] = AllocRaster (width, height)))
	    {
		CloseSketchPad (sp);
		return (NULL);
	    }
	}

	if (AllocDynamicImage (&sp->Brush) && AllocDynamicImage (&sp->Undo) && AllocDynamicImage (&sp->Backup))
	{
	    AddGList (sp->Window, &sp->Gad, (USHORT) - 1, (USHORT) - 1, NULL);

	    DrawBevelBox (sp->Drp, (SHORT) (sp->LeftEdge - 2), (SHORT) (sp->TopEdge - 1),
			  (SHORT) (sp->Gad.Width + 4), (SHORT) (sp->Gad.Height + 2),
			  GT_VisualInfo, sp->Visual,
			  TAG_DONE);
	    return (sp);
	}
	CloseSketchPad (sp);
    }
    return (NULL);
}

/*****************************************************************************/

VOID CloseSketchPad (SketchPadPtr sp)
{
    struct BitMap *bm;
    SHORT depth;

    if (sp)
    {
	bm = &sp->Wbm;
	depth = sp->Depth;

	while (depth-- > 0)
	    if (bm->Planes[depth])
		FreeRaster (bm->Planes[depth], (sp->Width * sp->MagX), (sp->Height * sp->MagY));

	FreeDynamicImage (&sp->Undo);
	FreeDynamicImage (&sp->Brush);
	FreeDynamicImage (&sp->Backup);
	FreeVec (sp);
    }
}

/*****************************************************************************/

VOID SPMagnifyDI (SketchPadPtr sp, DynamicImagePtr im)
{
    MagnifyBitMap (&im->di_BMap, &sp->Wbm, sp->MagX, sp->MagY,
		   im->di_Image.Width, sp->Grid);

    ClipBlit (&sp->Wrp, 0, 0,
	      sp->Drp, sp->LeftEdge, sp->TopEdge,
	      (sp->Width * sp->MagX), (sp->Height * sp->MagY), 0xC0);
}

/*****************************************************************************/

VOID SPSetRepeat (SketchPadPtr sp, DynamicImagePtr di,
		  SHORT LeftEdge, SHORT TopEdge)
{
    sp->Adi = di;

    if (di)
    {
	sp->RLeftEdge = LeftEdge;
	sp->RTopEdge = TopEdge;
	SPRefresh (sp);
    }
    else
    {
	sp->RLeftEdge = sp->RTopEdge = 0;
    }
}

/*****************************************************************************/

VOID SPRefresh (SketchPadPtr sp)
{
    SPMagnifyDI (sp, sp->Adi);
    DrawImage (sp->Drp, &sp->Adi->di_Image, sp->RLeftEdge, sp->RTopEdge);
}

/*****************************************************************************/

VOID SPClear (SketchPadPtr sp)
{

    SPSaveToUndo (sp);
    SetRast (&sp->Adi->di_RPort, sp->APen);
    SetAPen (sp->Drp, sp->APen);
    RectFill (sp->Drp, sp->RLeftEdge, sp->RTopEdge, (sp->RLeftEdge + sp->Width - 1), (sp->RTopEdge + sp->Height - 1));
    SPRefresh (sp);
}

/*****************************************************************************/

VOID SPUndo (SketchPadPtr sp)
{
#if 1
    /* Draw from Undo to temporary */
    DrawImage (&sp->Brush.di_RPort, &sp->Undo.di_Image, 0, 0);

    /* Draw from Screen to Undo */
    DrawImage (&sp->Undo.di_RPort, &sp->Adi->di_Image, 0, 0);

    /* Draw from temporary to Screen */
    DrawImage (&sp->Adi->di_RPort, &sp->Brush.di_Image, 0, 0);
#else
    DrawImage (&sp->Adi->di_RPort, &sp->Undo.di_Image, 0, 0);
#endif
    SPRefresh (sp);
}

/*****************************************************************************/

VOID SPSaveToUndo (SketchPadPtr sp)
{
    DrawImage (&sp->Undo.di_RPort, &sp->Adi->di_Image, 0, 0);
}

/*****************************************************************************/

static VOID SPEllipseCursor (SketchPadPtr sp)
{
    DrawEllipse (&sp->Adi->di_RPort, sp->cursor[0], sp->cursor[1],
		 ABS (sp->cursor[4] - sp->cursor[0]), ABS (sp->cursor[5] - sp->cursor[1]));
}

/*****************************************************************************/

static VOID SPEllipse (SketchPadPtr sp, struct IntuiMessage *msg)
{
    struct RastPort *rp = &sp->Adi->di_RPort;
    UWORD a, b;

    if (sp->Draw_Mode == 1)
    {
	SPEllipseCursor (sp);
	if (msg->Qualifier & IEQUALIFIER_CONTROL)
	{
	    sp->cursor[4]--;
	    SPEllipseCursor (sp);
	}
    }
    else
    {
	a = ABS (sp->cursor[4] - sp->cursor[0]);
	b = ABS (sp->cursor[5] - sp->cursor[1]);

	AreaEllipse (rp, sp->cursor[0], sp->cursor[1], a, b);
	AreaEnd (rp);
    }
}

/*****************************************************************************/

static VOID SPSortCursor (SketchPadPtr sp)
{
    WORD minx = MIN (sp->cursor[0], sp->cursor[4]);
    WORD miny = MIN (sp->cursor[1], sp->cursor[5]);
    WORD maxx = MAX (sp->cursor[0], sp->cursor[4]);
    WORD maxy = MAX (sp->cursor[1], sp->cursor[5]);

    sp->cursor[0] = sp->cursor[6] = sp->cursor[8] = minx;
    sp->cursor[1] = sp->cursor[3] = sp->cursor[9] = miny;
    sp->cursor[2] = sp->cursor[4] = maxx;
    sp->cursor[5] = sp->cursor[7] = maxy;
}

/*****************************************************************************/

static VOID SPDrawRect (SketchPadPtr sp, struct IntuiMessage *msg)
{
    struct RastPort *rp = &sp->Adi->di_RPort;

    if (msg->Qualifier & (IEQUALIFIER_LALT | IEQUALIFIER_RALT))
    {
	WORD tmp, w, h;

	/* Sort the X coordinate */
	if (sp->cursor[0] > sp->cursor[4])
	{
	    tmp = sp->cursor[0];
	    sp->cursor[0] = sp->cursor[4];
	    sp->cursor[4] = tmp;
	}

	/* Sort the Y coordinate */
	if (sp->cursor[1] > sp->cursor[5])
	{
	    tmp = sp->cursor[1];
	    sp->cursor[1] = sp->cursor[5];
	    sp->cursor[5] = tmp;
	}

	/* Compute the dimensions */
	w = sp->cursor[4] - sp->cursor[0] + 1;
	h = sp->cursor[5] - sp->cursor[1] + 1;

#if 1
	DrawBevelBox (rp, sp->cursor[0], sp->cursor[1], w, h, GT_VisualInfo, sp->Visual,
		      ((msg->Qualifier & IEQUALIFIER_RALT) ? GTBB_Recessed : TAG_IGNORE), TRUE,
		      TAG_DONE);
#else
	if (msg->Qualifier & IEQUALIFIER_LALT)
	{
	    DrawBevelBox (rp, sp->cursor[0], sp->cursor[1], w, h, GT_VisualInfo, sp->Visual,
			  TAG_DONE);
	}
	else
	{
	    DrawBevelBox (rp, sp->cursor[0], sp->cursor[1], w, h, GT_VisualInfo, sp->Visual,
			  GTBB_Recessed, TRUE,
			  TAG_DONE);
	}
#endif
    }
    else
    {
	PolyDraw (rp, NUMPOINTS, &sp->cursor[0]);
	if (msg->Qualifier & IEQUALIFIER_CONTROL)
	{
	    SPSetTile (sp);
	    PolyDraw (rp, NUMPOINTS, &sp->cursor[0]);
	}
    }
}

/*****************************************************************************/

static VOID SPDrawLine (SketchPadPtr sp, struct IntuiMessage *msg)
{
    struct RastPort *rp = &sp->Adi->di_RPort;

    Move (rp, sp->cursor[0], sp->cursor[1]);
    Draw (rp, sp->cursor[4], sp->cursor[5]);

    if (msg->Qualifier & IEQUALIFIER_CONTROL)
    {
	Move (rp, --sp->cursor[0], sp->cursor[1]);
	Draw (rp, --sp->cursor[4], sp->cursor[5]);
    }
}

/*****************************************************************************/

static VOID SPSetUL (SketchPadPtr sp, WORD X, WORD Y)
{
    sp->cursor[0] = sp->cursor[2] = sp->cursor[4] = X;
    sp->cursor[6] = sp->cursor[8] = X;
    sp->cursor[1] = sp->cursor[3] = sp->cursor[5] = Y;
    sp->cursor[7] = sp->cursor[9] = Y;
}

/*****************************************************************************/

static VOID SPSetLR (SketchPadPtr sp, WORD X, WORD Y)
{
    UWORD a, b;

    sp->cursor[2] = sp->cursor[4] = X;
    sp->cursor[5] = sp->cursor[7] = Y;

    if (sp->Draw_Mode == 9)
    {
	a = ABS (sp->cursor[4] - sp->cursor[0]);
	b = ABS (sp->cursor[5] - sp->cursor[1]);

	if (a > ICON_WIDTH / 2)
	    sp->cursor[4] = sp->cursor[2] = ICON_WIDTH / 2 + sp->cursor[0];

	if (b > ICON_HEIGHT / 2)
	    sp->cursor[5] = sp->cursor[7] = ICON_HEIGHT / 2 + sp->cursor[1];
    }
}

/*****************************************************************************/

static VOID SPSetTile (SketchPadPtr sp)
{
    sp->cursor[0] = sp->cursor[6] = sp->cursor[8] = sp->cursor[0] + 1;
    sp->cursor[2] = sp->cursor[4] = sp->cursor[4] - 1;
}

/*****************************************************************************/

static VOID SPCursor (SketchPadPtr sp, WORD x, WORD y, UBYTE cursor)
{
    struct RastPort *rp = &sp->Adi->di_RPort;

    Move (rp, sp->cursor[0], sp->cursor[1]);
    SPSetLR (sp, x, y);
    switch (cursor)
    {
	case 1:
	case 9:
	    DrawImage (rp, &(sp->Backup.di_Image), 0, 0);
	    SPEllipseCursor (sp);
	    break;
	case 2:
	    DrawImage (rp, &(sp->Backup.di_Image), 0, 0);
	    Move (rp, sp->cursor[0], sp->cursor[1]);
	    Draw (rp, sp->cursor[4], sp->cursor[5]);
	    break;
	case 4:		/* SMOOTH Drawing, not a cursor... */
	    Draw (rp, sp->cursor[4], sp->cursor[5]);
	    sp->cursor[0] = sp->cursor[4];
	    sp->cursor[1] = sp->cursor[5];
	    break;
	case 5:
	case 3:		/* CUT */
	case 13:
	    DrawImage (rp, &(sp->Backup.di_Image), 0, 0);
	    PolyDraw (rp, NUMPOINTS, &sp->cursor[0]);
	    break;
	case 7:
	    DrawImage (rp, &(sp->Backup.di_Image), 0, 0);
	    PolyDraw (rp, NUMPOINTS, &sp->cursor[0]);
	    break;
    }
}

/*****************************************************************************/

static VOID SPStartCursor (SketchPadPtr sp, WORD x, WORD y, UBYTE cursor)
{
    /* Save the current image */
    DrawImage (&(sp->Backup.di_RPort), &(sp->Adi->di_Image), 0, 0);
    SPSetUL (sp, x, y);
    SPSetLR (sp, x, y);
    SPCursor (sp, x, y, cursor);
}

/*****************************************************************************/

static VOID SPEndCursor (SketchPadPtr sp, UBYTE cursor,
			 struct IntuiMessage *msg)
{

    switch (sp->Draw_Mode)
    {
	case 1:		/* CIRCLE */
	case 9:		/* FILLED CIRCLE */
	    SPEllipse (sp, msg);
	    break;

	case 2:		/* LINE */
	    SPDrawLine (sp, msg);
	    break;

	case 3:		/* CUT */
	    break;

	case 5:		/* RECTANGLE */
	    SPDrawRect (sp, msg);
	    break;

	case 13:		/* FILLED RECTANGLE */
	    SPSortCursor (sp);
	    RectFill (&sp->Adi->di_RPort,
		      sp->cursor[0], sp->cursor[1],
		      sp->cursor[4], sp->cursor[5]);
	    break;
    }				/* End of SWITCH Draw_Mode */
}

/*****************************************************************************/

static VOID SPDrawHandle (SketchPadPtr sp, struct IntuiMessage *rmsg)
{
    struct RastPort *rp = &sp->Adi->di_RPort;
    struct Window *win = rmsg->IDCMPWindow;
    UBYTE cursor = sp->Draw_Mode;
    WORD l, t, bx, by, x, y;

    if (rmsg->Class == GADGETDOWN)
    {
	SPSaveToUndo (sp);
	SetAPen (sp->Drp, sp->APen);
	SetDrMd (sp->Drp, JAM2);
	if (sp->Adi)
	{
	    SetAPen (rp, sp->APen);
	    SetBPen (rp, sp->BPen);
	    SetDrMd (rp, JAM2);
	    SetAfPt (rp, sp->AreaPtrn, sp->AreaPtSz);
	}
    }
    l = sp->LeftEdge;
    t = sp->TopEdge;
    bx = rmsg->MouseX - l;
    by = rmsg->MouseY - t;
    x = bx / sp->MagX;
    y = by / sp->MagY;

    /* Handle Immediate Draw Modes */
    if (sp->Gad.Flags & SELECTED)
    {
	switch (cursor)
	{
	    case 0:		/* FAST DRAW */
		WritePixel (sp->Drp, (sp->RLeftEdge + x), (sp->RTopEdge + y));
		if (sp->Adi)
		    WritePixel (rp, x, y);
		bx = (bx / sp->MagX) * sp->MagX;
		by = (by / sp->MagY) * sp->MagY;
		RectFill (sp->Drp, (l + bx), (t + by),
			  (l + bx + (sp->MagX - 1) - sp->Grid),
			  (t + by + (sp->MagY - 1) - sp->Grid));
		cursor = 0;
		break;

	    case 6:		/* FILL */
	    case 14:
		if (sp->Adi)
		{
		    Flood (rp, (cursor == 6), x, y);
		    SPRefresh (sp);
		}
		cursor = 0;
		break;
	}
    }

    if (cursor && cursor != 6 && cursor != 14)
    {

	/*
	 * We need the most up-to-the-minute cursor coordinates for fast
	 * drawing!
	 */
	x = (win->MouseX - l) / sp->MagX;
	y = (win->MouseY - t) / sp->MagY;
	switch (rmsg->Class)
	{
	    case GADGETDOWN:
		SPStartCursor (sp, x, y, cursor);
		break;

	    case INTUITICKS:
		if (sp->Gad.Flags & SELECTED)
		    SPCursor (sp, x, y, cursor);
		break;

	    case MOUSEMOVE:
		SPCursor (sp, x, y, cursor);
		break;

	    case GADGETUP:
		SPEndCursor (sp, cursor, rmsg);
		break;

	    case MOUSEBUTTONS:
		if (rmsg->Code == SELECTUP)
		    SPEndCursor (sp, cursor, rmsg);
		break;
	    default:
		break;
	}
	SPRefresh (sp);
    }				/* End of IF Cursor */
}

/*****************************************************************************/

VOID SPDraw (SketchPadPtr sp, struct IntuiMessage *msg)
{
    struct Window *win = msg->IDCMPWindow;
    struct IntuiMessage *rmsg, cmsg;
    BOOL going = TRUE, mouse = FALSE, abort = FALSE;

    SPDrawHandle (sp, msg);
    while (going)
    {
	Wait ((1L << win->UserPort->mp_SigBit));
	while (going && (rmsg = (struct IntuiMessage *) GetMsg (win->UserPort)))
	{
	    HandleCursor (rmsg);

	    switch (rmsg->Class)
	    {
		case MOUSEMOVE:
		    cmsg = *rmsg;
		    mouse = TRUE;
		    break;

		case MOUSEBUTTONS:
		    if (rmsg->Code != SELECTUP)
			break;

		case GADGETUP:
		    going = FALSE;

		default:
		    if (!abort)
			SPDrawHandle (sp, rmsg);
		    break;
	    }

	    if (rmsg->Qualifier & IEQUALIFIER_RBUTTON)
	    {
		SPUndo (sp);
		abort = TRUE;
	    }

	    ReplyMsg ((struct Message *) rmsg);
	}

	if (going && mouse)
	{
	    if (!abort)
		SPDrawHandle (sp, &cmsg);
	    mouse = FALSE;
	}
    }
}
