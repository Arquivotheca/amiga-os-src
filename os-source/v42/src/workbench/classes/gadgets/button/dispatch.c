/* button gadget
 * Copyright (c) 1994 Commodore International Services, Co.
 * Written by David N. Junod
 *
 */

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/classusr.h>
#include <intuition/classes.h>
#include <intuition/cghooks.h>
#include <gadgets/button.h>
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
#include "utils.h"

/*****************************************************************************/

#define POINTLONG(pt)	(*((ULONG *)&(pt)))

/*****************************************************************************/

static UWORD ghost_pattern[] = {0x4444, 0x1111, 0x4444, 0x1111};

/*****************************************************************************/

static void InnerFill (struct ClassLib *cb, struct RastPort * rp, struct IBox * b, UWORD pen)
{
    if (((struct Library *)IntuitionBase)->lib_Version < 39)
    {
	SetAPen (rp, pen);
	SetDrMd (rp, JAM1);
    }
    else
	SetABPenDrMd (rp, pen, pen, JAM1);
    RectFill (rp, b->Left + 2, b->Top + 1, b->Left + b->Width - 3, b->Top + b->Height - 2);
}

/*****************************************************************************/

static void InnerFill1 (struct ClassLib *cb, struct RastPort * rp, struct IBox * b, UWORD pen)
{
    if (((struct Library *)IntuitionBase)->lib_Version < 39)
    {
	SetAPen (rp, pen);
	SetDrMd (rp, JAM1);
    }
    else
	SetABPenDrMd (rp, pen, pen, JAM1);
    RectFill (rp, b->Left + 1, b->Top + 1, b->Left + b->Width - 2, b->Top + b->Height - 2);
}

/*****************************************************************************/

static void render (Class *cl, struct Gadget *g, struct GadgetInfo *gi, LONG redraw)
{
    struct ClassLib *cb = (struct ClassLib *) cl->cl_UserData;
    struct RastPort *rp;

    /* Refresh if needed and able */
    if (rp = ObtainGIRPort (gi))
    {
	DoMethod ((Object *) g, GM_RENDER, gi, rp, redraw);
	ReleaseGIRPort (rp);
    }
}

/****** button.gadget/OM_SET **************************************************
*
*    NAME
*       OM_SET--Set the attributes of a button.gadget object.	(V42)
*
*    FUNCTION
*	The OM_SET method is used to set the attributes of a button.gadget.
*	This method is passed to the superclass first.
*
*    ATTRIBUTES
*	The following attributes can be changed at OM_SET or OM_UPDATE.
*
*	GA_Selected (BOOL) -- Determines whether the button is selected or
*	    not.  Changing selection state will invoke GM_RENDER.  Setting
*	    selected causes BUTTON_Current to go to 1.  Clearing selected
*	    causes BUTTON_Current to go to 0.  Defaults to FALSE.
*
*	GA_Disabled (BOOL) -- Determines whether the button is disabled or
*	    not.  Changing disable state will invoke GM_RENDER.  A disabled
*	    button's border and label are all rendered in SHADOWPEN and then
*	    dusted in a ghosting pattern that is rendered in SHADOWPEN.
*	    Defaults to FALSE.
*
*	GA_Text (STRPTR) -- Used to specify the NULL terminated string to use
*	    as the text for the gadget.  The Text() function is used to draw
*	    the text.  The class does not currently support underlining of
*	    the keyboard shortcut character.  NULL is valid input.  Changing
*	    the label will invoke GM_LAYOUT and then GM_RENDER.
*
*	GA_Image (struct Image *) -- Used to specify the image to use for the
*	    label of the gadget.  The DrawImage() function is used to draw
*	    the image.  NULL is valid input.  Changing the label will invoke
*	    GM_LAYOUT and then GM_RENDER.
*
*	GA_TextAttr (struct TextAttr *) -- Text attribute for the font to
*	    use for the labels.
*
*	GA_ReadOnly (BOOL) -- Read-only gadgets ignore activation attempts.
*	    Defaults to FALSE.
*
*	BUTTON_Glyph (struct Image *) -- Used to specify the image to use for
*	    the label of the gadget.  The BltTemplate() function is used to
*	    draw each plane of the image.  NULL is valid input.  Changing the
*	    label will invoke GM_LAYOUT and then GM_RENDER.
*
*	BUTTON_Current (LONG) -- Used to select the current image from the
*	    BUTTON_Array.  Changing the current image will invoke GM_LAYOUT
*	    and then GM_RENDER.  Defaults to zero.
*
*	BUTTON_TextPen (LONG) -- Indicate the pen number used to render the
*	    IDS_NORMAL text.  If -1 is specified, then TEXTPEN is used.
*	    Defaults to -1.  Changing the pen will invoke GM_RENDER.
*
*	BUTTON_BackgroundPen (LONG) -- Indicate the pen number used to render
*	    the IDS_NORMAL background.  If -1 is specified, then BACKGROUNDPEN
*	    is used.  Defaults to -1.  Changing the pen will invoke GM_RENDER.
*
*	BUTTON_FillTextPen (LONG) -- Indicate the pen number used to render
*	    the IDS_SELECTED text.  If -1 is specified, then FILLTEXTPEN is
*	    used.  Defaults to -1.  Changing the pen will invoke GM_RENDER.
*
*	BUTTON_FillPen (LONG) -- Indicate the pen number used to render the
*	    IDS_SELECTED background.  If -1 is specified, then FILLPEN is used.
*	    Defaults to -1.  Changing the pen will invoke GM_RENDER.
*
*    RESULT
*	The class will update the attributes of object.  Changing some
*	attributes will result in GM_LAYOUT and/or GM_RENDER being called.
*
*	The return value will be non-zero if the gadget needs to be refreshed.
*
*******************************************************************************
*
* David N. Junod
*
*/

static LONG setAttrsMethod (Class * cl, struct Gadget * g, struct opSet * msg, BOOL init)
{
    struct ClassLib *cb = (struct ClassLib *) cl->cl_UserData;
    struct objectData *od = INST_DATA (cl, g);
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG tidata;

    void *image = od->od_Image;
    UWORD flags = g->Flags;
    LONG refresh = 0;

    /* Initialize the variables */
    if (init)
    {
	/* Clear the table */
	for (tidata = 0; tidata < NUMDRIPENS; tidata++)
	    od->od_Pens[tidata] = -1;

	/* Show that we need GM_LAYOUT messages */
	g->Flags |= GFLG_RELSPECIAL;
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
	    case GA_Width:
	    case GA_RelWidth:
		g->Width = tidata;
		break;

	    case GA_Height:
	    case GA_RelHeight:
		g->Height = tidata;
		break;

	    case GA_TextAttr:
		if (od->od_TF)
		    CloseFont (od->od_TF);
		od->od_TF = OpenFont ((struct TextAttr *) tidata);
		od->od_Flags |= ODF_LAYOUT;
		break;

	    case GA_Selected:
		if (tidata)
		{
		    g->Flags |= GFLG_SELECTED;
		    od->od_Current = 1;
		}
		else
		{
		    g->Flags &= ~GFLG_SELECTED;
		    od->od_Current = 0;
		}
		break;

	    case GA_Disabled:
		if (tidata)
		    g->Flags |= GFLG_DISABLED;
		else
		    g->Flags &= ~GFLG_DISABLED;
		refresh = TRUE;
		break;

	    case GA_Text:
		od->od_Array = od->od_Image = (void *) tidata;
		od->od_Flags = ODF_TEXT | (od->od_Flags & 0xFFF0);
		od->od_ISize = (od->od_Image) ? strlen ((STRPTR)od->od_Image) : 0;
		od->od_Flags |= ODF_LAYOUT;
		break;

	    case GA_Image:
		od->od_Array = od->od_Image = (void *) tidata;
		od->od_Flags = ODF_IMAGE | (od->od_Flags & 0xFFF0);
		od->od_Flags |= ODF_LAYOUT;
		break;

	    case BUTTON_Glyph:
		od->od_Array = od->od_Image = (void *) tidata;
		od->od_Flags = ODF_IMAGE | ODF_GLYPH | (od->od_Flags & 0xFFF0);
		od->od_Flags |= ODF_LAYOUT;
		break;

	    case BUTTON_PushButton:
		if (tidata)
		    od->od_Flags |= ODF_PUSH;
		break;

	    case BUTTON_Array:
		od->od_Maximum = (WORD) tidata;
		od->od_Flags |= ODF_LAYOUT;
		break;

	    case BUTTON_Current:
		od->od_Current = (WORD) tidata;
		break;

	    case GA_ReadOnly:
		if (tidata)
		    od->od_Flags |= ODF_READONLY;
		else
		    od->od_Flags &= ~ODF_READONLY;
		break;

	    case BUTTON_TextPen:
		od->od_Pens[TEXTPEN] = tidata;
		refresh = TRUE;
		break;

	    case BUTTON_FillPen:
		od->od_Pens[FILLPEN] = tidata;
		refresh = TRUE;
		break;

	    case BUTTON_FillTextPen:
		od->od_Pens[FILLTEXTPEN] = tidata;
		refresh = TRUE;
		break;

	    case BUTTON_BackgroundPen:
		od->od_Pens[BACKGROUNDPEN] = tidata;
		refresh = TRUE;
		break;
	}
    }

    /* Set the image */
    if (od->od_Maximum)
	od->od_Image = od->od_Array[od->od_Current];

    /* See if we changed */
    if (image != od->od_Image)
	od->od_Flags |= ODF_LAYOUT;
    if (flags != g->Flags)
	refresh = TRUE;
    if (od->od_Flags & ODF_LAYOUT)
	refresh = TRUE;

    return (refresh);
}

/*****************************************************************************/

static LONG updateMethod (Class * cl, struct Gadget * g, struct opSet * msg)
{
    /* Now we handle it */
    if (setAttrsMethod (cl, g, msg, FALSE))
	render (cl, g, msg->ops_GInfo, GREDRAW_REDRAW);
    return (0);
}

/****** button.gadget/GM_LAYOUT ***********************************************
*
*    NAME
*	GM_LAYOUT--Calculate the positioning of the imagry.	(V42)
*
*    FUNCTION
*	The GM_LAYOUT method is used to calculate the domain of the button
*	and to center the label within the domain.  This method is passed to
*	the superclass first.
*
*	Gadget relativity is fully supported.
*
*    RESULT
*	This method returns 0.
*
******************************************************************************/

static LONG layoutMethod (Class * cl, struct Gadget * g, struct gpLayout * msg)
{
    struct ClassLib *cb = (struct ClassLib *) cl->cl_UserData;
    struct IBox *b = &msg->gpl_GInfo->gi_Domain;
    struct objectData *od = INST_DATA (cl, g);
    struct IBox *ib = &od->od_IDomain;
    struct IBox *db = &od->od_Domain;
    struct RastPort *rp;

    /* Give the superclass a chance */
    DoSuperMethodA (cl, (Object *) g, (Msg *) msg);

    /* Layout the gadget box */
    *db = *((struct IBox *) &g->LeftEdge);
    if (g->Flags & GFLG_RELBOTTOM)
	db->Top = b->Height + g->TopEdge;
    if (g->Flags & GFLG_RELRIGHT)
	db->Left = b->Width + g->LeftEdge;
    if (g->Flags & GFLG_RELWIDTH)
	db->Width = b->Width + g->Width;
    if (g->Flags & GFLG_RELHEIGHT)
	db->Height = b->Height + g->Height;

    /* Get a pointer to the rastport */
    if (rp = ObtainGIRPort (msg->gpl_GInfo))
    {
	/* Set the font */
	if (od->od_TF)
	    SetFont (rp, od->od_TF);

	/* Layout the image box */
	if (od->od_Image == NULL)
	{
	    ib->Width = ib->Height = 0;
	}
	else if (od->od_Flags & ODF_TEXT)
	{
	    ib->Width  = TextLength (rp, (STRPTR)od->od_Image, od->od_ISize);
	    ib->Height = rp->TxHeight;
	}
	else if (od->od_Flags & ODF_IMAGE)
	{
	    ib->Width  = ((struct Image *)od->od_Image)->Width;
	    ib->Height = ((struct Image *)od->od_Image)->Height;
	}

	/* Center the contents */
	ib->Left = (db->Width  - ib->Width ) >> 1;
	ib->Top  = (db->Height - ib->Height) >> 1;

	/* Release the rastport */
	ReleaseGIRPort (rp);
    }

    /* Adjust the edges of the insides */
    ib->Left += db->Left;
    ib->Top  += db->Top;

    return (0);
}

/****** button.gadget/GM_RENDER ***********************************************
*
*    NAME
*       GM_RENDER--Render the visuals of the button.            (V42)
*
*    FUNCTION
*	The GM_RENDER method is used to render the visuals of the button.
*	This method overrides the superclass.
*
*	The border of the gadget is drawn first.  Disabled gadgets get a
*	border that is drawn completely in SHADOWPEN.  A selected or
*	read-only button gets SHADOWPEN for the left and top sides, and
*	SHINEPEN for the right and bottom sides.  A normal button gets
*	SHINEPEN for the left and top sides, and SHADOWPEN for the
*	right and bottom sides.
*
*	The inside of the button is then drawn.  A normal button gets
*	filled with BUTTON_BackgroundPen.  A selected button gets filled
*	with BUTTON_FillPen.
*
*	Then the label is drawn.
*
*	    GA_Text:  Text() is used to render the text.  BUTTON_TextPen
*	    is used for a normal button and BUTTON_FillTextPen is used for a
*	    selected button.
*
*	    GA_Image:  DrawImageState() is used to render the image.  Note
*	    that the background pen color is already set appropriately for
*	    the image state.
*
*	    BUTTON_Glyph:  BltTemplate() is used to render each of the planes
*	    of the image.  BUTTON_TextPen is used for a normal button and
*	    BUTTON_FillTextPen is used for a selected button.  The second
*	    and higher planes are render using SHADOWPEN.
*
*	If the button is disabled, then the ghosting pattern is applied.
*
*    RESULT
*	This method returns 0.
*
******************************************************************************/

static LONG renderMethod (Class * cl, struct Gadget * g, struct gpRender * msg)
{
    struct ClassLib *cb = (struct ClassLib *) cl->cl_UserData;
    struct objectData *od = INST_DATA (cl, g);
    UBYTE ulpen, lrpen, tpen, fpen, tpen2;
    struct DrawInfo *dri;
    struct RastPort *rp;
    struct IBox *ib;
    struct IBox *b;
    UWORD *pens;
    BOOL border;

    /* Get the things that we need */
    ib   = &od->od_IDomain;
    b    = &od->od_Domain;
    dri  = msg->gpr_GInfo->gi_DrInfo;
    rp   = msg->gpr_RPort;
    pens = dri->dri_Pens;
    border = (g->Activation & 0x00F0);

    if (((struct Library *)IntuitionBase)->lib_Version < 39)
    {
	struct IBox *gb = &msg->gpr_GInfo->gi_Domain;
	struct IBox dbox, *db = &dbox;

	/* See if there is a change */
	if ((g->LeftEdge != b->Left) || (g->TopEdge != b->Top) ||
	    (g->Width != b->Width) || (g->Height != b->Height))
	{
	    *db = *((struct IBox *) &g->LeftEdge);
	    if (g->Flags & GFLG_RELBOTTOM)
		db->Top = gb->Height + g->TopEdge;
	    if (g->Flags & GFLG_RELRIGHT)
		db->Left = gb->Width + g->LeftEdge;
	    if (g->Flags & GFLG_RELWIDTH)
		db->Width = gb->Width + g->Width;
	    if (g->Flags & GFLG_RELHEIGHT)
		db->Height = gb->Height + g->Height;
	    if ((db->Left != b->Left) || (db->Top != b->Top) ||
		(db->Width != b->Width) || (db->Height != b->Height))
	    {
		od->od_Flags |= ODF_LAYOUT;
	    }
	}
    }

    /* See if we need a layout */
    if (od->od_Flags & ODF_LAYOUT)
    {
	DoMethod ((Object *) g, GM_LAYOUT, msg->gpr_GInfo, 0);
	od->od_Flags &= ~ODF_LAYOUT;
    }

    /* Set the font */
    if (od->od_TF)
	SetFont (rp, od->od_TF);

    /* Get the pens to use */
    tpen2 = (od->od_Pens[BACKGROUNDPEN] == -1) ? pens[BACKGROUNDPEN] : od->od_Pens[BACKGROUNDPEN];
    if ((g->Flags & GFLG_SELECTED) || (od->od_Flags & ODF_READONLY))
    {
	ulpen = pens[SHADOWPEN];
	lrpen = pens[SHINEPEN];
	fpen = (od->od_Flags & ODF_READONLY) ? BACKGROUNDPEN : FILLPEN;
	fpen = (od->od_Pens[fpen] == -1) ? pens[fpen] : od->od_Pens[fpen];
	tpen = (od->od_Pens[FILLTEXTPEN] == -1) ? pens[FILLTEXTPEN] : od->od_Pens[FILLTEXTPEN];
    }
    else
    {
	ulpen = pens[SHINEPEN];
	lrpen = pens[SHADOWPEN];
	fpen = (od->od_Pens[BACKGROUNDPEN] == -1) ? pens[BACKGROUNDPEN] : od->od_Pens[BACKGROUNDPEN];
	tpen = (od->od_Pens[TEXTPEN] == -1) ? pens[TEXTPEN] : od->od_Pens[TEXTPEN];
	tpen2 = pens[SHADOWPEN];
    }

    /* Flatten the border if we are disabled */
    if (g->Flags & GFLG_DISABLED)
	tpen = tpen2 = ulpen = lrpen = pens[SHADOWPEN];

    if (border)
    {
	if (msg->gpr_GInfo->gi_Window->Flags & WFLG_WINDOWACTIVE)
	    fpen = (od->od_Pens[FILLPEN] == -1) ? pens[FILLPEN] : od->od_Pens[FILLPEN];
	else
	    fpen = (od->od_Pens[BACKGROUNDPEN] == -1) ? pens[BACKGROUNDPEN] : od->od_Pens[BACKGROUNDPEN];

	if (g->Flags & GFLG_SELECTED)
	    tpen = (od->od_Pens[FILLTEXTPEN] == -1) ? pens[FILLTEXTPEN] : od->od_Pens[FILLTEXTPEN];
	else
	    tpen = pens[SHADOWPEN];

	/* Draw the border */
	QuickBevel1 (cb, rp, b, ulpen, lrpen);

	/* Fill the inside of the button */
	InnerFill1 (cb, rp, b, fpen);
    }
    else
    {
	/* Draw the border */
	QuickBevel (cb, rp, b, ulpen, lrpen);

	/* Fill the inside of the button */
	InnerFill (cb, rp, b, fpen);
    }

    /* Draw the insides */
    if (od->od_Image == NULL)
    {
    }
    else if (od->od_Flags & ODF_TEXT)
    {
	SetAPen (rp, tpen);
	Move (rp, ib->Left, ib->Top + rp->TxBaseline);
	Text (rp, od->od_Image, od->od_ISize);
    }
    else if (od->od_Flags & ODF_GLYPH)
    {
	struct Image *im = (struct Image *) od->od_Image;
	register WORD i;
	LONG offset;
	UBYTE *data;
	UBYTE bpr;

	SetAPen (rp, tpen);
	bpr = ((im->Width + 15) >> 4) << 1;
	offset = UMult32 (bpr, im->Height);
	data = (UBYTE *) im->ImageData;
	for (i = 0; i < im->Depth; i++, data += offset)
	{
	    if (i > 0)
		SetAPen (rp, tpen2);
	    BltTemplate ((void *) data, 0, bpr, rp, ib->Left, ib->Top, im->Width, im->Height);
	}
    }
    else if (od->od_Flags & ODF_IMAGE)
    {
	DrawImageState (rp, (struct Image *) od->od_Image, ib->Left, ib->Top,
			((g->Flags & GFLG_SELECTED) ? IDS_SELECTED : IDS_NORMAL), dri);
    }

    if (g->Flags & GFLG_DISABLED)
    {
	SetAfPt (rp, ghost_pattern, 2);
	InnerFill (cb, rp, b, ulpen);
	SetAfPt (rp, NULL, 0);
    }
    return (0);
}

/****** button.gadget/GM_HITTEST **********************************************
*
*    NAME
*       GM_HITTEST--Is gadget hit.                              (V42)
*
*    FUNCTION
*	The GM_HITTEST method is used to determine if the given mouse
*	coordinates are within the domain of the button.  This method
*	overrides the superclass.
*
*    RESULT
*	This method returns GMR_GADGETHIT if within the domain, otherwise
*	zero is returned.
*
*	If the gadget is GA_ReadOnly, then zero is always returned.
*
******************************************************************************/

static LONG hitMethod (Class *cl, struct Gadget *g, struct gpHitTest *msg)
{
    struct objectData *od = INST_DATA (cl, g);
    struct IBox *b = &od->od_Domain;
    WORD x, y;

    /* We will never be hit if read only */
    if (od->od_Flags & ODF_READONLY)
	return 0;

    /* Adjust the coordinates */
    x = msg->gpht_Mouse.X + b->Left;
    y = msg->gpht_Mouse.Y + b->Top;

    /* See if we were hit */
    if ((x < b->Left) || (x > (b->Left + b->Width - 1)) ||
	(y < b->Top)  || (y > (b->Top + b->Height - 1)))
	return 0;
    return GMR_GADGETHIT;
}

/****** button.gadget/GM_GOACTIVE *********************************************
*
*    NAME
*       GM_GOACTIVE--Activate a gadget.                         (V42)
*
*    FUNCTION
*	The GM_GOACTIVE method is used to indicate to a gadget that it has
*	become active.  This method overrides the superclass.
*
*	GA_ToggleSelect:  Toggles selection state.
*
*	BUTTON_PushButton:  If not selected, then becomes selected and sets
*	    BUTTON_Current to 1.  If selected and BUTTON_Array is greater than
*	    one, then will cycle through the array, while staying selected,
*	    with 1 being the lower bounds.
*
*	BUTTON_Array: Sets selection state and cycle through the array with 0
*	    being the lower bounds.
*
*	Otherwise: Sets selection state.
*
*	Invokes GM_RENDER with GREDRAW_REDRAW set.
*
*    RESULT
*	For GA_ToggleSelect returns GMR_VERIFY | GMR_NOREUSE.
*
*	For BUTTON_PushButton returns GMR_VERIFY | GMR_NOREUSE when the
*	state changes, otherwise returns GMR_NOREUSE.
*
*	All other cases returns GMR_MEACTIVE.
*
*	Sets the *msg->gpi_Termination field to BUTTON_Current, which in turn
*	fills in the IntuiMessage->Code field.
*
******************************************************************************/

static LONG activeMethod (Class * cl, struct Gadget * g, struct gpInput * msg)
{
    struct objectData *od = INST_DATA (cl, g);
    struct InputEvent *ie = msg->gpi_IEvent;
    WORD current = od->od_Current;
    LONG redraw = GREDRAW_REDRAW;
    LONG retval = GMR_MEACTIVE;
    BOOL refresh = FALSE;

    /* Remember the current state */
    od->od_Previous = od->od_Current;

    /* See if we are being pushed */
    if (ie && (ie->ie_Class == IECLASS_RAWMOUSE))
    {
	if (ie->ie_Code == SELECTDOWN)
	{
	    if (od->od_Flags & ODF_PUSH)
	    {
		if (g->Flags & GFLG_SELECTED)
		{
		    if (od->od_Maximum)
		    {
			if (ie->ie_Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
			    od->od_Current--;
			else
			    od->od_Current++;
			retval = GMR_VERIFY | GMR_NOREUSE;
		    }
		    else
			retval = GMR_NOREUSE;
		}
		else
		{
		    refresh = TRUE;
		    od->od_Current = 1;
		    g->Flags |= GFLG_SELECTED;
		    retval = GMR_VERIFY | GMR_NOREUSE;
		}
	    }
	    else
	    {
		refresh = TRUE;
		g->Flags ^= GFLG_SELECTED;
		if (g->Activation & GACT_TOGGLESELECT)
		{
		    retval = GMR_VERIFY | GMR_NOREUSE;
		    od->od_Current = (g->Flags & GFLG_SELECTED) ? 1 : 0;
		}
	    }
	}
    }

    if (od->od_Maximum && (current != od->od_Current))
    {
	if (od->od_Current == od->od_Maximum)
	    od->od_Current = 1;
	if (od->od_Current < 1)
	    od->od_Current = od->od_Maximum - 1;
	od->od_Image = od->od_Array[od->od_Current];
	od->od_Flags |= ODF_LAYOUT;
	refresh = TRUE;
    }

    /* Refresh if needed and able */
    if (refresh)
	render (cl, g, msg->gpi_GInfo, redraw);

    /* Set the termination value */
    *msg->gpi_Termination = (LONG)od->od_Current;

    return (retval);
}

/****** button.gadget/GM_HANDLEINPUT ******************************************
*
*    NAME
*       GM_HANDLEINPUT--Handle input events.                    (V42)
*
*    FUNCTION
*	The GM_HANDLEINPUT method is used to handle the input events of an
*	active button gadget.  This method overrides the superclass.
*
*	This method correctly handles RMB abort.
*
*    RESULT
*	This method returns GMR_MEACTIVE as long as the gadget is active.
*
******************************************************************************/

static LONG inputMethod (Class * cl, struct Gadget * g, struct gpInput * msg)
{
    struct objectData *od = INST_DATA (cl, g);
    struct InputEvent *ie = msg->gpi_IEvent;
    LONG redraw = GREDRAW_REDRAW;
    LONG retval = GMR_MEACTIVE;
    BOOL refresh = FALSE;
    ULONG pointlong;
    UWORD state;

    /* See if we are inside the hit region */
    pointlong = POINTLONG (((struct gpInput *) msg)->gpi_Mouse);
    state = DoMethod ((Object *)g, GM_HITTEST, msg->gpi_GInfo, pointlong) ? GFLG_SELECTED : 0;
    if (state != (g->Flags & GFLG_SELECTED))
    {
	g->Flags ^= GFLG_SELECTED;
	refresh = TRUE;
    }

    /* Handle the different input events */
    switch (ie->ie_Class)
    {
	case IECLASS_RAWMOUSE:
	    switch (ie->ie_Code)
	    {
		case SELECTDOWN:
		    break;

		/* Support right-button abort */
		case MENUDOWN:
		    if (od->od_Maximum)
		    {
			od->od_Image = od->od_Array[od->od_Current = od->od_Previous];
			od->od_Flags |= ODF_LAYOUT;
		    }
		    retval = GMR_NOREUSE;
		    break;

		case SELECTUP:
		    /* Handle arrays */
		    if (od->od_Maximum)
		    {
			if (ie->ie_Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
			    od->od_Current--;
			else
			    od->od_Current++;
			if (od->od_Current == od->od_Maximum)
			    od->od_Current = 0;
			if (od->od_Current < 0)
			    od->od_Current = od->od_Maximum - 1;
			od->od_Image = od->od_Array[od->od_Current];
			od->od_Flags |= ODF_LAYOUT;
		    }
		    else
			od->od_Current = 0;
		    retval = GMR_VERIFY | GMR_NOREUSE;
		    break;

		default:
		    break;
	    }
	    break;
    }

    /* Set the termination value */
    *msg->gpi_Termination = (LONG)od->od_Current;

    /* Refresh if needed and able */
    if (refresh)
	render (cl, g, msg->gpi_GInfo, redraw);

    return (retval);
}

/****** button.gadget/GM_GOINACTIVE *******************************************
*
*    NAME
*       GM_GOINACTIVE--Button has lost activation.              (V42)
*
*    FUNCTION
*	The GM_GOINACTIVE method is used to indicate that the button has
*	become inactive.  This method overrides the superclass.
*
*	For momentary buttons, this will cause the GFLG_SELECTED flag to be
*	cleared and the GM_RENDER method to be called with GREDRAW_REDRAW.
*
*    RESULT
*	This method returns 0.
*
******************************************************************************/

static LONG inactiveMethod (Class *cl, struct Gadget *g, struct gpGoInactive *msg)
{
    struct objectData *od = INST_DATA (cl, g);

    /* See if we should leave early */
    if ((g->Activation & GACT_TOGGLESELECT) || (od->od_Flags & ODF_PUSH))
	return 0;

    /* Deselect */
    g->Flags &= ~GFLG_SELECTED;

    /* Render */
    render (cl, g, msg->gpgi_GInfo, GREDRAW_REDRAW);
    return 0;
}

/****** button.gadget/OM_NEW **************************************************
*
*    NAME
*       OM_NEW--Create a button.gadget object.                  (V42)
*
*    FUNCTION
*	The OM_NEW method is used to create an instance of the button.gadget
*	class.  This method is passed to the superclass first.
*
*    ATTRIBUTES
*	The following attributes can be specified at creation time.
*
*	GA_ToggleSelect (BOOL) -- Indicate that the gadget is a toggle button.
*	    Defaults to FALSE.
*
*	GA_Selected (BOOL) -- Determines whether the button is selected or
*	    not.  Defaults to FALSE.
*
*	GA_Disabled (BOOL) -- Determines whether the button is disabled or
*	    not.  Defaults to FALSE.
*
*	GA_Text (STRPTR) -- Used to specify the NULL terminated string to use
*	    as the text for the gadget.
*
*	GA_Image (struct Image *) -- Used to specify the image to use for the
*	    label of the gadget.
*
*	GA_TextAttr (struct TextAttr *) -- Text attribute for the font to
*	    use for the labels.
*
*	GA_ReadOnly (BOOL) -- Read-only gadgets ignore activation attempts.
*	    Defaults to FALSE.
*
*	BUTTON_Glyph (struct Image *) -- Used to specify the image to use for
*	    the label of the gadget.
*
*	BUTTON_PushButton (BOOL) -- Used to indicate that the button stays
*	    pressed in when the user selects it with the mouse.  The button
*	    may programmatically be deselected using {GA_Selected, FALSE}.
*	    Defaults to FALSE.
*
*	BUTTON_Array (LONG) -- Indicates that the label is an array, and
*	    indicates the number of elements in the array.  This allows the
*	    gadget to be used as a checkbox or a cycle gadget without the
*	    cycle glyph.  Defaults to 1.
*
*	BUTTON_Current (LONG) -- Used to select the current image from the
*	    BUTTON_Array.  Defaults to zero.
*
*	BUTTON_TextPen (LONG) -- Indicate the pen number used to render the
*	    IDS_NORMAL text.  If -1 is specified, then TEXTPEN is used.
*	    Defaults to -1.
*
*	BUTTON_BackgroundPen (LONG) -- Indicate the pen number used to render
*	    the IDS_NORMAL background.  If -1 is specified, then BACKGROUNDPEN
*	    is used.  Defaults to -1.
*
*	BUTTON_FillTextPen (LONG) -- Indicate the pen number used to render
*	    the IDS_SELECTED text.  If -1 is specified, then FILLTEXTPEN is
*	    used.  Defaults to -1.
*
*	BUTTON_FillPen (LONG) -- Indicate the pen number used to render the
*	    IDS_SELECTED background.  If -1 is specified, then FILLPEN is used.
*	    Defaults to -1.
*
*    RESULT
*	If the object was created then a pointer to the object is returned,
*	otherwise NULL is returned.
*
*******************************************************************************
*
* David N. Junod
*
*/

static LONG newMethod (Class * cl, struct Gadget * g, struct opSet * msg)
{
    struct Gadget *newobj;

    /* Create the new object */
    if (newobj = (struct Gadget *) DoSuperMethodA (cl, (Object *) g, msg))
    {
	/* Update the attributes */
	setAttrsMethod (cl, newobj, msg, TRUE);
    }

    return ((LONG) newobj);
}

/*****************************************************************************/

static void deleteMethod (Class * cl, struct Gadget * g, ULONG * msg)
{
    struct ClassLib *cb = (struct ClassLib *) cl->cl_UserData;
    struct objectData *od = INST_DATA (cl, g);

    /* Free any opened font */
    if (od->od_TF)
	CloseFont (od->od_TF);
}

/*****************************************************************************/

LONG ASM ClassDispatcher (REG (a0) Class * cl, REG (a1) ULONG * msg, REG (a2) struct Gadget * g)
{
    switch (*msg)
    {
	case OM_NEW:
	    return newMethod (cl, g, (struct opSet *) msg);

	case OM_SET:
	case OM_UPDATE:
	    return updateMethod (cl, g, (struct opSet *) msg);

	case GM_LAYOUT:
	    return layoutMethod (cl, g, (struct gpLayout *) msg);

	case GM_RENDER:
	    return renderMethod (cl, g, (struct gpRender *) msg);

	case GM_HITTEST:
	    return hitMethod (cl, g, (struct gpHitTest *) msg);

	case GM_GOACTIVE:
	    return activeMethod (cl, g, (struct gpInput *) msg);

	case GM_HANDLEINPUT:
	    return inputMethod (cl, g, (struct gpInput *) msg);

	case GM_GOINACTIVE:
	    return inactiveMethod (cl, g, (struct gpGoInactive *) msg);

	case OM_DISPOSE:
	    deleteMethod (cl, g, msg);
	default:
	    return DoSuperMethodA (cl, (Object *) g, (Msg *) msg);
    }
}
