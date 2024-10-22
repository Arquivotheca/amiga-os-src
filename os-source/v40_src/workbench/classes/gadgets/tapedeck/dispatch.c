/* tapedeck controller
 * by David N. Junod
 *
 */

/*****************************************************************************/

#define	USE_PP_BUTTONS	FALSE
#define	DB(x)	;

/*****************************************************************************/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/classusr.h>
#include <intuition/classes.h>
#include <intuition/cghooks.h>
#include <graphics/displayinfo.h>
#include <graphics/gfxmacros.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/macros.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "classbase.h"
#include "classdata.h"
#include "tapedeck.h"
#include "utils.h"

/*****************************************************************************/

static UWORD __chip rew_data[]   = {0x0301, 0x8000, 0x0F07, 0x8000, 0x3F1F, 0x8000, 0xFF7F,
				    0x8000, 0x3F1F, 0x8000, 0x0F07, 0x8000, 0x0301, 0x8000};
static UWORD __chip play_data[]  = {0xC000, 0xF000, 0xFC00, 0xFF00, 0xFC00, 0xF000, 0xC000};
static UWORD __chip ff_data[]    = {0xC060, 0x0000, 0xF078, 0x0000, 0xFC7E, 0x0000, 0xFF7F,
				    0x8000, 0xFC7E, 0x0000, 0xF078, 0x0000, 0xC060, 0x0000};
static UWORD __chip stop_data[]  = {0xFF80, 0xFF80, 0xFF80, 0xFF80, 0xFF80, 0xFF80, 0xFF80};
static UWORD __chip pause_data[] = {0xE380, 0xE380, 0xE380, 0xE380, 0xE380, 0xE380, 0xE380};
static UWORD __chip begin_data[] = {0xC0C0, 0xC3C0, 0xCFC0, 0xFFC0, 0xCFC0, 0xC3C0, 0xC0C0};
static UWORD __chip end_data[]   = {0xC0C0, 0xF0C0, 0xFCC0, 0xFFC0, 0xFCC0, 0xF0C0, 0xC0C0};
static UWORD __chip pp_data[]    = {0xC000, 0x71C0, 0xF000, 0x71C0, 0xFC00, 0x71C0, 0xFF00,
				    0x71C0, 0xFC00, 0x71C0, 0xF000, 0x71C0, 0xC000, 0x71C0};

#if USE_PP_BUTTONS
/* Highlight plane for PAUSE */
static UWORD __chip pp1_data[]   = {0x0000, 0x71C0, 0x0000, 0x71C0, 0x0000, 0x71C0, 0x0000,
				    0x71C0, 0x0000, 0x71C0, 0x0000, 0x71C0, 0x0000, 0x71C0};
/* Highlight plane for PLAY */
static UWORD __chip pp2_data[]   = {0xC000, 0x0000, 0xF000, 0x0000, 0xFC00, 0x0000, 0xFF00,
				    0x0000, 0xFC00, 0x0000, 0xF000, 0x0000, 0xC000, 0x0000};
#endif

/*****************************************************************************/

static struct Glyph anim_glyphs[4] =
{
    {17, 7, rew_data},
#if USE_PP_BUTTONS
    {26, 7, pp_data},
#else
    { 8, 7, play_data},
#endif
    {17, 7, ff_data},
    { 9, 7,  NULL},		/* Actually the frame control */
};

#if USE_PP_BUTTONS
/* Plane two data for Play/Pause button */
static struct Glyph pp_glyphs[2] =
{
    {26, 7, pp1_data},
    {26, 7, pp2_data},
};
#endif

/*****************************************************************************/

static struct Glyph tape_glyphs[5] =
{
    {17, 7, rew_data},
    { 8, 7, play_data},
    {17, 7, ff_data},
    { 9, 7, stop_data},
    { 9, 7, pause_data},
};

/*****************************************************************************/

static ULONG setattrs (Class * cl, struct Gadget * g, ULONG tags,...)
{
    return SetAttrsA (g, (struct TagItem *) & tags);
}

/*****************************************************************************/

static struct Gadget *newobject (Class * cl, STRPTR name, ULONG tags,...)
{
    return (struct Gadget *) NewObjectA (NULL, name, (struct TagItem *) & tags);
}

/*****************************************************************************/

static LONG setAttrsMethod (Class * cl, struct Gadget * g, struct opSet * msg, BOOL init)
{
    struct objectData *od = INST_DATA (cl, g);
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *tstate;
    struct TagItem *tag;
    LONG refresh = 0;
    ULONG tidata;

    /* Initialize the variables */
    if (init)
    {
	/* Some arbitrary number */
	od->od_Frames = 10;

	/* Force the size */
	g->Width = 202;
	g->Height = 16;

	/* Show that we need GM_LAYOUT messages */
	g->Flags |= GFLG_RELSPECIAL;

	/* Start out stopped... */
	od->od_OMode = od->od_Mode = BUT_STOP;

	/* What kind of controller are we? */
	od->od_Glyphs = anim_glyphs;
	od->od_NumButtons = 4;
	if (GetTagData (TDECK_Tape, FALSE, tags))
	{
	    od->od_Glyphs = tape_glyphs;
	    od->od_Flags |= ODF_TAPE;
	    od->od_NumButtons = 5;
	}
    }
    else
    {
	/* Let the super class handle it first */
	refresh = DoSuperMethodA (cl, (Object *) g, msg);
    }

    /* process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case TDECK_Mode:
		if (tidata >= BUT_REWIND && tidata <= BUT_STOP)
		{
		    if ((UWORD) tidata != od->od_Mode)
			refresh++;
		    od->od_Mode = (UWORD) tidata;
		}
		else if (tidata == BUT_PAUSE)
		{
		    od->od_Flags ^= ODF_PAUSED;
		    od->od_Flags |= ODF_PCHANGE;
		    refresh++;
		}
		break;

	    case TDECK_Paused:
		if (tidata)
		    od->od_Flags |= ODF_PAUSED;
		else
		    od->od_Flags &= ~ODF_PAUSED;
		od->od_Flags |= ODF_PCHANGE;
		refresh++;
		break;

	    case TDECK_CurrentFrame:
		if (od->od_ScrollG)
		    setattrs (cl, od->od_ScrollG, PGA_Top, tidata, TAG_DONE);
		if (od->od_CurrentFrame != tidata)
		    refresh++;
		od->od_CurrentFrame = tidata;
		break;

	    case TDECK_Frames:
		if (od->od_ScrollG)
		    setattrs (cl, od->od_ScrollG, PGA_Total, tidata, TAG_DONE);
		od->od_Frames = tidata;
		break;
	}
    }

    return (refresh);
}

/*****************************************************************************/

static LONG updateMethod (Class * cl, struct Gadget * g, struct opSet * msg)
{
    struct RastPort *rp;

    /* Now we handle it */
    if (setAttrsMethod (cl, g, msg, FALSE))
    {
	if (rp = ObtainGIRPort (msg->ops_GInfo))
	{
	    struct gpRender gpr;

	    gpr.MethodID   = GM_RENDER;
	    gpr.gpr_GInfo  = msg->ops_GInfo;
	    gpr.gpr_RPort  = rp;
	    gpr.gpr_Redraw = GREDRAW_UPDATE;
	    DoMethodA ((Object *) g, &gpr);
	    ReleaseGIRPort (rp);
	}
    }

    return (0);
}

/*****************************************************************************/

static LONG getAttrMethod (Class * cl, struct Gadget * g, struct opGet * msg)
{
    struct objectData *od = INST_DATA (cl, g);

    switch (msg->opg_AttrID)
    {
	case TDECK_Mode:
	    *msg->opg_Storage = (ULONG) od->od_Mode;
	    break;

	case TDECK_Paused:
	    *msg->opg_Storage = (ULONG) (od->od_Flags & ODF_PAUSED);
	    break;

	case TDECK_CurrentFrame:
	    *msg->opg_Storage = od->od_CurrentFrame;
	    break;

	case TDECK_Frames:
	    *msg->opg_Storage = od->od_Frames;
	    break;

	default:
	    return (DoSuperMethodA (cl, (Object *) g, msg));
    }

    return (1L);
}

/*****************************************************************************/

static LONG layoutMethod (Class * cl, struct Gadget * g, struct gpLayout * msg)
{
    struct IBox *b = &msg->gpl_GInfo->gi_Domain;
    struct objectData *od = INST_DATA (cl, g);
    struct IBox gbox;
    WORD b1, b2;
    WORD left;

    /* Get the gadget box */
    gbox = *((struct IBox *) & g->LeftEdge);
    if (g->Flags & GFLG_RELBOTTOM)
	gbox.Top = b->Height + g->TopEdge;
    if (g->Flags & GFLG_RELRIGHT)
	gbox.Left = b->Width + g->LeftEdge;
    left = gbox.Left;
    gbox.Height = 15;

    if (od->od_Flags & ODF_TAPE)
    {
	/* Rewind */
	gbox.Width = 27;
	CopyMem (&gbox, &od->od_Buttons[0], sizeof (struct IBox));
	od->od_ID[0] = BUT_REWIND;

	/* Play */
	gbox.Left += gbox.Width;
	gbox.Width = 48;
	CopyMem (&gbox, &od->od_Buttons[1], sizeof (struct IBox));
	od->od_ID[1] = BUT_PLAY;

	/* Fast Forward */
	gbox.Left += gbox.Width;
	gbox.Width = 27;
	CopyMem (&gbox, &od->od_Buttons[2], sizeof (struct IBox));
	od->od_ID[2] = BUT_FORWARD;

	/* Stop */
	gbox.Left += gbox.Width;
	gbox.Width = 48;
	CopyMem (&gbox, &od->od_Buttons[3], sizeof (struct IBox));
	od->od_ID[3] = BUT_STOP;

	/* Pause */
	gbox.Left += gbox.Width + 3;
	CopyMem (&gbox, &od->od_Buttons[4], sizeof (struct IBox));
	od->od_ID[4] = BUT_PAUSE;
    }
    else
    {
	/* Compute the size of the buttons */
	b1 = (g->Width * 100) / 748;
	b2 = (g->Width * 100) / 420;
	b1 = MAX (b1, 23);
	b2 = MAX (b2, 14);
	if (g->Width < 80)
	    b1 = 0;

	/* Rewind */
	gbox.Width = b1;	// 27;
	CopyMem (&gbox, &od->od_Buttons[0], sizeof (struct IBox));
	od->od_ID[0] = BUT_REWIND;

	/* Play (range from 16 to 48 in width) */
	gbox.Left += gbox.Width;
	gbox.Width = b2;	// 48;
	CopyMem (&gbox, &od->od_Buttons[1], sizeof (struct IBox));
	od->od_ID[1] = BUT_PLAY;

	/* Fast Forward */
	gbox.Left += gbox.Width;
	gbox.Width = b1;	// 27;
	CopyMem (&gbox, &od->od_Buttons[2], sizeof (struct IBox));
	od->od_ID[2] = BUT_FORWARD;

	/* Frame Control */
	gbox.Left += gbox.Width;
	gbox.Width = g->Width - (gbox.Left - left) + 1;

	CopyMem (&gbox, &od->od_Buttons[3], sizeof (struct IBox));
	od->od_ID[3] = BUT_FRAME;
	if (od->od_ScrollG)
	{
	    setattrs (cl, od->od_ScrollG,
		      GA_Left,	(LONG) (gbox.Left + 4),
		      GA_Top,	(LONG) (gbox.Top + 2),
		      GA_Width,	(LONG) (gbox.Width - 8),
		      TAG_DONE);
	}
    }

    return (0);
}

/*****************************************************************************/

static void InnerFill (struct RastPort * rp, struct IBox * b, UWORD pen)
{
    SetAPen (rp, pen);
    RectFill (rp,
	      b->Left + 2,
	      b->Top + 1,
	      b->Left + b->Width - 3,
	      b->Top + b->Height - 2);
}

/*****************************************************************************/

static void DrawGlyph (struct RastPort * rp, struct IBox * b, struct Glyph * g, UWORD pen)
{
    WORD w = g->w;
    WORD h = g->h;

    SetAPen (rp, pen);
    BltTemplate ((void *) g->data,
                 0, ((w + 15) >> 4) << 1,
                 rp, b->Left + (b->Width - w) / 2, b->Top + (b->Height - h) / 2,
                 w, h);
}

/*****************************************************************************/

#define GREDRAW_SLIDER	(100)

/*****************************************************************************/

static LONG renderMethod (Class * cl, struct Gadget * g, struct gpRender * msg)
{
    struct objectData *od = INST_DATA (cl, g);
    UBYTE pen1, pen2, fpen, gpen;
    struct DrawInfo *dri;
    struct RastPort *rp;
    register UWORD i;
    struct IBox *b;
    BOOL redraw;
    UWORD *pens;

    if (msg->gpr_Redraw != GREDRAW_SLIDER)
    {
	/* Make sure we have everything we need to render */
	if (!msg->gpr_GInfo || !msg->gpr_GInfo->gi_DrInfo)
	{
	    kprintf ("gi=%08lx\n", msg->gpr_GInfo);
	    return (0);
	}

	dri  = msg->gpr_GInfo->gi_DrInfo;
	rp   = msg->gpr_RPort;
	pens = dri->dri_Pens;

	/* Draw the boxes */
	for (i = 0; i < od->od_NumButtons; i++)
	{
	    redraw = FALSE;
	    if (msg->gpr_Redraw == GREDRAW_REDRAW)
		redraw = TRUE;

	    /* Default pens */
	    pen1 = SHINEPEN;
	    pen2 = SHADOWPEN;
	    fpen = BACKGROUNDPEN;
	    gpen = TEXTPEN;

	    if (od->od_ID[i] == BUT_PAUSE)
	    {
		if (od->od_Flags & ODF_PCHANGE)
		    redraw = TRUE;

		if (od->od_Flags & ODF_PAUSED)
		{
		    pen1 = SHADOWPEN;
		    pen2 = SHINEPEN;
		    fpen = FILLPEN;
		    gpen = FILLTEXTPEN;
		}
	    }
	    else if (od->od_ID[i] == od->od_Mode)
	    {
		redraw = TRUE;

		if ((((g->Flags & GFLG_SELECTED) || (od->od_Flags & ODF_TAPE)) && (od->od_Mode != BUT_FRAME)) ||
		    ((od->od_Mode == BUT_PLAY) && !(od->od_Flags & ODF_TAPE)))
		{
		    pen1 = SHADOWPEN;
		    pen2 = SHINEPEN;
		    fpen = FILLPEN;
		    gpen = FILLTEXTPEN;
		}
	    }
	    else if (od->od_ID[i] == od->od_OMode)
		redraw = TRUE;

	    if (redraw)
	    {
		b = &od->od_Buttons[i];
		if (b->Width)
		{
		    QuickBevel (rp, b, pens[pen1], pens[pen2]);
		    InnerFill (rp, b, pens[fpen]);
		    if (od->od_Glyphs[i].data)
			DrawGlyph (rp, b, &od->od_Glyphs[i], pens[gpen]);

#if USE_PP_BUTTONS
		    if (!(od->od_Flags & ODF_TAPE) && (od->od_ID[i] == BUT_PLAY))
		    {
			if (od->od_Mode == BUT_STOP)
			    DrawGlyph (rp, b, &pp_glyphs[0], pens[SHINEPEN]);
			else if (od->od_Mode == BUT_PLAY)
			    DrawGlyph (rp, b, &pp_glyphs[1], pens[SHINEPEN]);
		    }
#endif
		}
	    }
	}

	/* Remember the last mode drawn */
	od->od_OMode = od->od_Mode;
	od->od_Flags &= ~ODF_PCHANGE;
    }

    /* Invoke the render method on the scroller */
    if (od->od_ScrollG)
    {
	if (msg->gpr_Redraw == GREDRAW_SLIDER)
	    msg->gpr_Redraw = GREDRAW_UPDATE;
	DoMethodA ((Object *) od->od_ScrollG, msg);
    }

    return (0);
}

/*****************************************************************************/
/* Tape Controller Event Handler */
static LONG inputTMethod (Class * cl, struct Gadget * g, struct gpInput * msg)
{
    struct objectData *od = INST_DATA (cl, g);
    struct InputEvent *ie = msg->gpi_IEvent;
    LONG retval = GMR_NOREUSE | GMR_VERIFY;
    struct RastPort *rp;
    struct IBox *b;
    UWORD i, mode;
    WORD x;

    /* Remember the current mode */
    mode = od->od_Mode;

    /* See what button was hit */
    if ((ie->ie_Class == IECLASS_RAWMOUSE) && (ie->ie_Code == SELECTDOWN))
    {
	/* Get the current X position */
	x = od->od_Buttons[0].Left + msg->gpi_Mouse.X;

	/* See which button was pressed */
	for (i = 0; i < od->od_NumButtons; i++)
	{
	    b = &od->od_Buttons[i];
	    if ((x >= b->Left) && (x <= b->Left + b->Width - 1))
	    {
		mode = od->od_ID[i];
		break;
	    }
	}
    }

    /* See if there was a change */
    if (mode != od->od_Mode)
    {
	/* Pause is a toggle button */
	if (mode == BUT_PAUSE)
	{
	    od->od_Flags ^= ODF_PAUSED;
	    od->od_Flags |= ODF_PCHANGE;
	    mode = od->od_Mode;
	}

	/* Set the mode */
	od->od_Mode = mode;

	/* Refresh if needed and able */
	if (rp = ObtainGIRPort (msg->gpi_GInfo))
	{
	    struct gpRender gpr;

	    /* Update the visuals */
	    gpr.MethodID   = GM_RENDER;
	    gpr.gpr_GInfo  = msg->gpi_GInfo;
	    gpr.gpr_RPort  = rp;
	    gpr.gpr_Redraw = GREDRAW_UPDATE;
	    DoMethodA ((Object *) g, &gpr);

	    ReleaseGIRPort (rp);
	}
    }

    /* Set the termination value */
    *msg->gpi_Termination = mode | ((od->od_Flags & ODF_PAUSED) ? 0x1000 : 0x0000);
    g->SpecialInfo = (APTR) (mode | ((od->od_Flags & ODF_PAUSED) ? 0x1000 : 0x0000));

    return (retval);
}

/*****************************************************************************
 * Animation Controller Event Handler
 *
 * With this controller there are only two states:
 *
 *  BUT_PLAY
 *  BUT_STOP
 *
 * This controller also provides the additional information on the current
 * frame that should be displayed.
 *
 */
static LONG inputAMethod (Class * cl, struct Gadget * g, struct gpInput * msg)
{
    struct objectData *od = INST_DATA (cl, g);
    struct InputEvent *ie = msg->gpi_IEvent;
    LONG retval = GMR_NOREUSE | GMR_VERIFY;
    ULONG redraw = GREDRAW_UPDATE;
    BOOL refresh = FALSE;
    struct RastPort *rp;
    struct IBox *b;
    UWORD i, mode;
    WORD x, y;

    /* No mode selected yet... */
    mode = BUT_STOP;

    /* Get the current X position */
    x = (WORD) od->od_Buttons[0].Left + msg->gpi_Mouse.X;
    y = (WORD) od->od_Buttons[0].Top + msg->gpi_Mouse.Y;

    /* See which button was pressed */
    for (i = 0; i < od->od_NumButtons; i++)
    {
	b = &od->od_Buttons[i];
	if ((x >= b->Left) && (x <= b->Left + b->Width - 1) && (y >= b->Top) && (y <= b->Top + b->Height - 1))
	{
	    mode = od->od_ID[i];
	    break;
	}
    }

    /* Remember activation method */
    if ((ie->ie_Class == IECLASS_RAWMOUSE) && (ie->ie_Code == SELECTDOWN))
    {
	GetAttr (PGA_Top, (Object *) od->od_ScrollG, &od->od_OldFrame);
	od->od_OldMode = od->od_Mode;
	g->Flags |= GFLG_SELECTED;

	if ((od->od_AMode = mode) == BUT_FRAME)
	{
	    redraw = GREDRAW_REDRAW;
	    refresh = TRUE;
	    mode = BUT_STOP;
	}
    }

    /* Abortion time? */
    if ((ie->ie_Class == IECLASS_RAWMOUSE) && (ie->ie_Code == MENUDOWN))
    {
	setattrs (cl, od->od_ScrollG, PGA_Top, od->od_OldFrame, TAG_DONE);
	redraw = GREDRAW_REDRAW;
	mode = od->od_OldMode;
	retval = GMR_NOREUSE;
	refresh = TRUE;
    }
    /* Are they messing with the scroller? */
    else if (od->od_AMode == BUT_FRAME)
    {
	/* Hit the frame control */
	if (od->od_ScrollG)
	{
	    /* Adjust the mouse */
	    msg->gpi_Mouse.X -= (od->od_Buttons[3].Left - od->od_Buttons[0].Left + 4);
	    msg->gpi_Mouse.Y -= 2;

	    /* Pass the message to the scroller */
	    retval = DoMethodA ((Object *) od->od_ScrollG, msg);

	    /* Get the current top and pass it along */
	    GetAttr (PGA_Top, (Object *) od->od_ScrollG, &od->od_CurrentFrame);
	    g->SpecialInfo = (APTR) (0x8000 | od->od_CurrentFrame);
	    *msg->gpi_Termination = (0x8000 | od->od_CurrentFrame);
	}

	/* Make sure we remember that this is the active object */
	od->od_Active = od->od_ScrollG;
    }
    /* Messing with a button */
    else
    {
	retval = GMR_MEACTIVE;

	switch (ie->ie_Class)
	{
	    case IECLASS_RAWMOUSE:
		switch (ie->ie_Code)
		{
		    case SELECTDOWN:
			if (mode == od->od_OldMode)
			{
			    retval = GMR_NOREUSE | GMR_VERIFY;
			    redraw = GREDRAW_REDRAW;
			    refresh = TRUE;
			    mode = BUT_STOP;
			}
			break;

			/* Everything kosher, so tell everyone about the status */
		    case SELECTUP:
			retval = GMR_NOREUSE | GMR_VERIFY;
			redraw = GREDRAW_REDRAW;
			if (mode == od->od_AMode)
			{
			    if (mode == od->od_OldMode)
				mode = BUT_STOP;

			    switch (mode)
			    {
				case BUT_REWIND:
				    mode = BUT_STOP;
				    break;

				case BUT_PLAY:
				    break;

				case BUT_STOP:
				    break;

				case BUT_FORWARD:
				    mode = BUT_STOP;
				    break;
			    }
			}
			else
			    mode = BUT_STOP;
			refresh = TRUE;
			break;

			/* Mouse movement */
		    default:
			if (mode != od->od_AMode)
			{
			    g->Flags &= ~GFLG_SELECTED;
			    mode = BUT_STOP;
			}
			else
			    g->Flags |= GFLG_SELECTED;
			refresh = TRUE;
			break;
		}
		break;
	}

	/* Set the termination value */
	*msg->gpi_Termination = mode;
	g->SpecialInfo = (APTR) mode;
    }

    /* Set the mode */
    od->od_Mode = mode;

    /* Refresh if needed and able */
    if (refresh && (rp = ObtainGIRPort (msg->gpi_GInfo)))
    {
	struct gpRender gpr;

	/* Update the visuals */
	gpr.MethodID   = GM_RENDER;
	gpr.gpr_GInfo  = msg->gpi_GInfo;
	gpr.gpr_RPort  = rp;
	gpr.gpr_Redraw = redraw;
	DoMethodA ((Object *) g, &gpr);

	ReleaseGIRPort (rp);
    }

    return (retval);
}

/*****************************************************************************/

static LONG newMethod (Class * cl, struct Gadget * g, struct opSet * msg)
{
    struct Gadget *newobj;
    struct objectData *od;

    /* Create the new object */
    if (newobj = (struct Gadget *) DoSuperMethodA (cl, (Object *) g, msg))
    {
	od = INST_DATA (cl, newobj);

	/* set up the slider attributes from the tag list */
	setAttrsMethod (cl, newobj, msg, TRUE);

	/* See if we are an animation controller */
	if (!(od->od_Flags & ODF_TAPE))
	{
	    /* Initialize the frame control */
	    if (od->od_ScrollG = newobject (cl, "propgclass",
					    GA_Width,		89,
					    GA_Height,		11,
					    PGA_Freedom,	FREEHORIZ,
					    PGA_Top,		od->od_CurrentFrame,
					    PGA_Visible,	1,
					    PGA_Total,		od->od_Frames,
					    PGA_Borderless,	TRUE,
					    PGA_NewLook,	FALSE,
					    TAG_DONE))
	    {
	    }
	}
    }

    return ((LONG) newobj);
}

/*****************************************************************************/

LONG ASM ClassDispatcher (REG (a0) Class * cl, REG (a1) ULONG * msg, REG (a2) struct Gadget * g)
{
    struct objectData *od = INST_DATA (cl, g);

    switch (*msg)
    {
	case OM_NEW:
	    return (newMethod (cl, g, (struct opSet *) msg));

	case OM_SET:
	case OM_UPDATE:
	    return (updateMethod (cl, g, (struct opSet *) msg));

	case OM_GET:
	    return (getAttrMethod (cl, g, (struct opGet *) msg));

	case GM_LAYOUT:
	    return (layoutMethod (cl, g, (struct gpLayout *) msg));

	case GM_RENDER:
	    return (renderMethod (cl, g, (struct gpRender *) msg));

	case GM_HITTEST:
	    return (GMR_GADGETHIT);

	case GM_GOACTIVE:
	case GM_HANDLEINPUT:
	    if (od->od_Flags & ODF_TAPE)
		return (inputTMethod (cl, g, (struct gpInput *) msg));
	    else
		return (inputAMethod (cl, g, (struct gpInput *) msg));

	case GM_GOINACTIVE:
	    g->Flags &= ~GFLG_SELECTED;
	    if (od->od_Active)
		DoMethodA ((Object *) od->od_Active, msg);
	    od->od_Active = NULL;
	    break;

	case OM_DISPOSE:
	    /* Dispose of the frame control */
	    if (od->od_ScrollG)
		DisposeObject (od->od_ScrollG);

	default:
	    return (DoSuperMethodA (cl, (Object *) g, (Msg *) msg));
    }
}
