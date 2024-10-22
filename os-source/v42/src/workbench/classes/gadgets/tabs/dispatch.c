/* tabs gadget
 * Copyright (c) 1994 Commodore International Services, Co.
 * Written by David N. Junod
 *
 */

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/classusr.h>
#include <intuition/classes.h>
#include <intuition/cghooks.h>
#include <gadgets/tabs.h>
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

/****** tabs.gadget/OM_SET **************************************************
*
*    NAME
*       OM_SET--Set the attributes of a tabs.gadget object.     (V42)
*
*    FUNCTION
*	The OM_SET method is used to set the attributes of a tabs.gadget.
*	This method is passed to the superclass first.
*
*    ATTRIBUTES
*	The following attributes can be changed at OM_SET or OM_UPDATE.
*
*	GA_Disabled (BOOL) -- Determines whether the gadget is disabled or
*	    not.  Changing disable state will invoke GM_RENDER.  A disabled
*	    gadget's border and label are all rendered in SHADOWPEN and then
*	    dusted in a ghosting pattern that is rendered in SHADOWPEN.
*	    Defaults to FALSE.
*
*	GA_TextAttr (struct TextAttr *) -- Text attribute for the font to
*	    use for the labels.
*
*	LAYOUTA_ChildMaxWidth (BOOL) -- Indicate whether the tabs should be
*	    the width of the widest label.  Defaults to TRUE.
*
*	TABS_Labels (TabLabelP) -- An array of TabLabel structures used to
*	    indicate the labels for each of the tabs.
*
*	TABS_Current (LONG) -- Currently selected tab.  Defaults to zero.
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
    LONG refresh = 0;
    ULONG tidata;
    UWORD flags = g->Flags;

    /* Initialize the variables */
    if (init)
    {
	/* Show that we need GM_LAYOUT messages */
	g->Flags |= GFLG_RELSPECIAL;
	od->od_Flags |= ODF_MAXWIDTH;
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

	    case TABS_Labels:
		/* Clear the number of labels */
		od->od_Maximum = 0;

		/* Free any existing tab labels */
		FreeVec (od->od_TLP);
		od->od_TLP = NULL;

		/* Set the labels */
		if (tidata)
		{
		    TabLabelP tl;
		    char *ptr;
		    LONG i;

		    /* Now count the number of labels */
		    tl = (TabLabelP) tidata;
		    while (tl->tl_Label)
		    {
			od->od_Maximum++;
			tl++;
		    }

		    /* Allocate the tab label array */
		    if (od->od_TLP = AllocVec (od->od_Maximum * sizeof (WTabLabel), MEMF_CLEAR))
		    {
			tl = (TabLabelP) tidata;
			ptr = (char *)od->od_TLP;
			for (i = 0; i < od->od_Maximum; i++)
			{
			    CopyMem (tl, ptr, sizeof (TabLabel));
			    ptr += sizeof (WTabLabel);
			    tl++;
			}
		    }
		}
		od->od_Flags |= ODF_LAYOUT;
		break;

	    case TABS_Current:
		od->od_Previous = od->od_Current;
		od->od_Current = (WORD) tidata;
		refresh = TRUE;
		break;

	    case LAYOUTA_ChildMaxWidth:
		if (tidata)
		    od->od_Flags |= ODF_MAXWIDTH;
		else
		    od->od_Flags &= ~ODF_MAXWIDTH;
		od->od_Flags |= ODF_LAYOUT;
		break;
	}
    }

    if (flags != g->Flags)
	refresh = TRUE;

    if (od->od_Flags & ODF_LAYOUT)
	refresh = TRUE;

    return (refresh);
}

/****** tabs.gadget/OM_GET **************************************************
*
*    NAME
*       OM_GET--Get an attribute of a tabs.gadget object.       (V42)
*
*    FUNCTION
*	The OM_GET method is used to get an attributes of a tabs.gadget.
*	This method is passed to the superclass for any attribute the
*	class doesn't understand..
*
*    ATTRIBUTES
*	The following attributes can be obtained from the tabs.gadget.
*
*	TABS_Current (LONG) -- Currently selected tab.
*
*    RESULT
*	Returns TRUE (1) if the attribute can be obtained, otherwise
*	returns FALSE (0).
*
*******************************************************************************
*
* David N. Junod
*
*/

static LONG getAttrMethod (Class * cl, struct Gadget * g, struct opGet * msg)
{
    struct objectData *od = INST_DATA (cl, g);

    switch (msg->opg_AttrID)
    {
	case TABS_Current:
	    *msg->opg_Storage = (ULONG) od->od_Current;
	    break;

	default:
	    return (DoSuperMethodA (cl, (Object *) g, msg));
    }

    return (1L);
}

/*****************************************************************************/

static LONG updateMethod (Class * cl, struct Gadget * g, struct opSet * msg)
{
    /* Now we handle it */
    if (setAttrsMethod (cl, g, msg, FALSE))
	render (cl, g, msg->ops_GInfo, GREDRAW_REDRAW);
    return (0);
}

/****** tabs.gadget/GM_LAYOUT ***********************************************
*
*    NAME
*       GM_LAYOUT--Calculate the positioning of the imagry.     (V42)
*
*    FUNCTION
*	The GM_LAYOUT method is used to calculate the domain of the gadget
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
	WORD i, x, width, mw;
	struct IBox *tb;
	WTabLabelP tl;

	/* Set the font */
	if (od->od_TF)
	    SetFont (rp, od->od_TF);

	/* Compute text sizes */
	for (i = mw = 0, x = db->Left + 4; i < od->od_Maximum; i++)
	{
	    tl = &od->od_TLP[i];
	    tb = &tl->tl_Domain;

	    tl->tl_LabelLength = strlen (tl->tl_Label);
	    width = TextLength (rp, tl->tl_Label, tl->tl_LabelLength);
	    mw = MAX (mw, width);
	    tb->Left   = x;
	    tb->Top    = db->Top;
	    tb->Width  = (WORD) width + 16;
	    tb->Height = db->Height - 2;

	    /* Offset of the label */
	    tl->tl_Offset = x + ((tb->Width - width) / 2);

	    x += tb->Width + 4;
	}

	if (od->od_Flags & ODF_MAXWIDTH)
	{
	    for (i = 0, x = db->Left + 4; i < od->od_Maximum; i++)
	    {
		tl = &od->od_TLP[i];
		tb = &tl->tl_Domain;
		tl->tl_Offset = x + (((mw + 16) - (tb->Width - 16)) / 2);
		tb->Left = x;
		tb->Width = mw + 16;
		x += tb->Width + 4;
	    }
	}

	/* Release the rastport */
	ReleaseGIRPort (rp);
    }
    return (0);
}

/****** tabs.gadget/GM_RENDER ***********************************************
*
*    NAME
*       GM_RENDER--Render the visuals of the gadget.            (V42)
*
*    FUNCTION
*	The GM_RENDER method is used to render the visuals of the gadget.
*	This method overrides the superclass.
*
*	If the gadget is disabled, then the ghosting pattern is applied.
*
*    RESULT
*	This method returns 0.
*
******************************************************************************/

static LONG renderMethod (Class * cl, struct Gadget * g, struct gpRender * msg)
{
    struct ClassLib *cb = (struct ClassLib *) cl->cl_UserData;
    struct objectData *od = INST_DATA (cl, g);
    struct DrawInfo *dri;
    struct RastPort *rp;
    UWORD tpen, fpen;
    struct IBox *tb;
    struct IBox *b;
    WORD i, x, ofs, hgt;
    WTabLabelP tl;
    UWORD *pens;

    /* Get the things that we need */
    b    = &od->od_Domain;
    dri  = msg->gpr_GInfo->gi_DrInfo;
    rp   = msg->gpr_RPort;
    pens = dri->dri_Pens;

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

    if (msg->gpr_Redraw == GREDRAW_UPDATE)
    {
	tl = &od->od_TLP[od->od_Previous];
	tb = &tl->tl_Domain;
	x  = tb->Left;
	ofs = hgt = 0;
	fpen = (tl->tl_Pens[TL_BACKGROUNDPEN] == -1) ? pens[BACKGROUNDPEN] : tl->tl_Pens[TL_BACKGROUNDPEN];
	if (((struct Library *)IntuitionBase)->lib_Version < 39)
	{
	    SetAPen (rp, fpen); SetBPen (rp, fpen); SetDrMd (rp, JAM2);
	}
	else
	    SetABPenDrMd (rp, fpen, fpen, JAM2);
	RectFill (rp, x + 1, tb->Top + tb->Height - 1, x + tb->Width - 2, tb->Top + tb->Height + hgt - 1);
	SetAPen (rp, pens[SHADOWPEN]);
	Move (rp, x, b->Top + b->Height - 2);
	Draw (rp, x + tb->Width - 1, b->Top + b->Height - 2);
	SetAPen (rp, pens[SHINEPEN]);
	Move (rp, x, b->Top + b->Height - 1);
	Draw (rp, x + tb->Width - 1, b->Top + b->Height - 1);
	Move (rp, x + tb->Width - 1, b->Top + b->Height - 3);
	Draw (rp, x + tb->Width - 1, b->Top + b->Height - 3);

	tl = &od->od_TLP[od->od_Current];
	tb = &tl->tl_Domain;
	x  = tb->Left;
	ofs = 0; hgt = 2;
	fpen = (tl->tl_Pens[TL_BACKGROUNDPEN] == -1) ? pens[BACKGROUNDPEN] : tl->tl_Pens[TL_BACKGROUNDPEN];
	if (((struct Library *)IntuitionBase)->lib_Version < 39)
	{
	    SetAPen (rp, fpen); SetBPen (rp, fpen); SetDrMd (rp, JAM2);
	}
	else
	    SetABPenDrMd (rp, fpen, fpen, JAM2);
	RectFill (rp, x + 1, b->Top + b->Height - 3, x + tb->Width - 2, b->Top + b->Height - 1);
	SetAPen (rp, pens[SHINEPEN]);
	RectFill (rp, x, b->Top + b->Height - 4, x + 1, b->Top + b->Height - 2);
	SetAPen (rp, pens[SHADOWPEN]);
	RectFill (rp, x + tb->Width - 2, b->Top + b->Height - 4, x + tb->Width - 1, b->Top + b->Height - 2);
	return 0;
    }

    /* Draw the first part of the line */
    SetAPen (rp, pens[SHADOWPEN]);
#if 0
    Move (rp, b->Left, b->Top + b->Height - 6);
    Draw (rp, b->Left + 3, b->Top + b->Height - 6);
#endif
    Move (rp, b->Left, b->Top + b->Height - 4);
    Draw (rp, b->Left + 3, b->Top + b->Height - 4);
    Move (rp, b->Left, b->Top + b->Height - 2);
    Draw (rp, b->Left + 4, b->Top + b->Height - 2);
    SetAPen (rp, pens[SHINEPEN]);
#if 0
    Move (rp, b->Left, b->Top + b->Height - 5);
    Draw (rp, b->Left + 3, b->Top + b->Height - 5);
#endif
    Move (rp, b->Left, b->Top + b->Height - 3);
    Draw (rp, b->Left + 3, b->Top + b->Height - 3);
    Move (rp, b->Left, b->Top + b->Height - 1);
    Draw (rp, b->Left + 4, b->Top + b->Height - 1);

    /* Step through the labels */
    for (i = 0; i < od->od_Maximum; i++)
    {
	/* Get a pointer to the tab label */
	tl = &od->od_TLP[i];
	tb = &tl->tl_Domain;
	x  = tb->Left;

	/* Erase the background */
	ofs = hgt = 0;
	if (i == od->od_Current)
	    hgt = 2;
	tpen = (tl->tl_Pens[TL_TEXTPEN] == -1) ? pens[TEXTPEN] : tl->tl_Pens[TL_TEXTPEN];
	fpen = (tl->tl_Pens[TL_BACKGROUNDPEN] == -1) ? pens[BACKGROUNDPEN] : tl->tl_Pens[TL_BACKGROUNDPEN];
	if (((struct Library *)IntuitionBase)->lib_Version < 39)
	{
	    SetAPen (rp, fpen); SetBPen (rp, fpen); SetDrMd (rp, JAM2);
	}
	else
	    SetABPenDrMd (rp, fpen, fpen, JAM2);
	if ((i == od->od_Current) || (i == od->od_Previous))
	{
	    RectFill (rp, x + 2, tb->Top + 2, x + tb->Width - 3, tb->Top + tb->Height - 1);
	    RectFill (rp, x + 3, tb->Top + 1, x + tb->Width - 4, tb->Top + hgt + 1);
	    RectFill (rp, x + 1, tb->Top + tb->Height - 1, x + tb->Width - 2, tb->Top + tb->Height + hgt - 1);
	}

	/* Draw the tab */
	SetAPen (rp, pens[SHADOWPEN]);
	Move (rp, x + tb->Width - 3, tb->Top + 1);
	Draw (rp, x + tb->Width - 2, tb->Top + 1);
	Draw (rp, x + tb->Width - 2, tb->Top + tb->Height + hgt - 2);
	Move (rp, x + tb->Width - 1, tb->Top + tb->Height + hgt - 2);
	Draw (rp, x + tb->Width - 1, tb->Top + 2);
	Move (rp, x + tb->Width - 1, tb->Top + tb->Height + hgt - 2);
	Draw (rp, x + tb->Width + 3, tb->Top + tb->Height + hgt - 2);
	if (i == od->od_Current)
	{
	    Move (rp, x + tb->Width, tb->Top + tb->Height + hgt - 4);
	    Draw (rp, x + tb->Width + 4, tb->Top + tb->Height + hgt - 4);
	}
	else
	{
	    Move (rp, x, tb->Top + tb->Height + hgt);
	    Draw (rp, x + tb->Width + 4, tb->Top + tb->Height + hgt);
	}

	SetAPen (rp, pens[SHINEPEN]);
	Move (rp, x,     tb->Top + 2);
	Draw (rp, x,     tb->Top + tb->Height + hgt - 2);
	Move (rp, x + 1, tb->Top + tb->Height + hgt - 2);
	Draw (rp, x + 1, tb->Top + 1);
	Draw (rp, x + 2, tb->Top + 1);
	Move (rp, x + 3, tb->Top + ofs);
	Draw (rp, x + tb->Width - 4, tb->Top + ofs);
	Move (rp, x, tb->Top + tb->Height + hgt - 1);
	Draw (rp, x, tb->Top + tb->Height + hgt - 1);
	Move (rp, x + tb->Width - 1, tb->Top + tb->Height + hgt - 1);
	Draw (rp, x + tb->Width + 3, tb->Top + tb->Height + hgt - 1);
	if (i == od->od_Current)
	{
	    Move (rp, x + tb->Width, tb->Top + tb->Height + hgt - 3);
	    Draw (rp, x + tb->Width + 4, tb->Top + tb->Height + hgt - 3);
	}
	else
	{
	    Move (rp, x, tb->Top + tb->Height + hgt + 1);
	    Draw (rp, x + tb->Width + 4, tb->Top + tb->Height + hgt + 1);
	}

	/* Draw the label */
	SetAPen (rp, tpen);
	Move (rp, tl->tl_Offset, tb->Top + rp->TxBaseline + 2);
	Text (rp, tl->tl_Label, tl->tl_LabelLength);
    }

    /* Draw the rest of the line */
    x += tb->Width;
    SetAPen (rp, pens[SHADOWPEN]);
#if 0
    Move (rp, x, b->Top + b->Height - 6);
    Draw (rp, b->Left + b->Width - 6, b->Top + b->Height - 6);
#endif
    Move (rp, x, b->Top + b->Height - 4);
    Draw (rp, b->Left + b->Width - 4, b->Top + b->Height - 4);
    Move (rp, x, b->Top + b->Height - 2);
    Draw (rp, b->Left + b->Width - 2, b->Top + b->Height - 2);
    SetAPen (rp, pens[SHINEPEN]);
#if 0
    Move (rp, x, b->Top + b->Height - 5);
    Draw (rp, b->Left + b->Width - 5, b->Top + b->Height - 5);
#endif
    Move (rp, x, b->Top + b->Height - 3);
    Draw (rp, b->Left + b->Width - 3, b->Top + b->Height - 3);
    Move (rp, x, b->Top + b->Height - 1);
    Draw (rp, b->Left + b->Width - 1, b->Top + b->Height - 1);

    /* Draw the ghosting pattern if we are disabled */
    if (g->Flags & GFLG_DISABLED)
    {
	SetAfPt (rp, ghost_pattern, 2);
	InnerFill (cb, rp, b, pens[SHADOWPEN]);
	SetAfPt (rp, NULL, 0);
    }

    return (0);
}

/****** tabs.gadget/GM_HITTEST **********************************************
*
*    NAME
*       GM_HITTEST--Is gadget hit.                              (V42)
*
*    FUNCTION
*	The GM_HITTEST method is used to determine if the given mouse
*	coordinates are within the domain of the gadget.  This method
*	overrides the superclass.
*
*    RESULT
*	This method returns GMR_GADGETHIT if within the domain, otherwise
*	zero is returned.
*
******************************************************************************/

static LONG hitMethod (Class *cl, struct Gadget *g, struct gpHitTest *msg)
{
    struct objectData *od = INST_DATA (cl, g);
    struct IBox *b = &od->od_Domain;
    WORD i, x, y;

    /* No hit right now */
    od->od_Hit = -1;

    /* Adjust the coordinates */
    x = msg->gpht_Mouse.X + b->Left;
    y = msg->gpht_Mouse.Y + b->Top;

    /* See if we were hit */
    if ((x < b->Left) || (x > (b->Left + b->Width - 1)) ||
	(y < b->Top)  || (y > (b->Top + b->Height - 1)))
	return 0;

    for (i = 0; i < od->od_Maximum; i++)
    {
	b = &(od->od_TLP[i].tl_Domain);
	if ((x >= b->Left) && (x <= (b->Left + b->Width - 1)) &&
	    (y >= b->Top)  && (y <= (b->Top + b->Height - 1)))
	{
	    od->od_Hit = i;
	    return GMR_GADGETHIT;
	}
    }
    return 0;
}

/****** tabs.gadget/GM_GOACTIVE *********************************************
*
*    NAME
*       GM_GOACTIVE--Activate a gadget.                         (V42)
*
*    FUNCTION
*	The GM_GOACTIVE method is used to indicate to a gadget that it has
*	become active.  This method overrides the superclass.
*
*	Invokes GM_RENDER with GREDRAW_UPDATE set if the current tab is
*	changed.
*
*    RESULT
*	If a new tab is selected, then returns GMR_VERIFY | GMR_NOREUSE.
*
*	All other cases returns GMR_NOREUSE.
*
*	Sets the *msg->gpi_Termination field to TABS_Current, which in turn
*	fills in the IntuiMessage->Code field.
*
******************************************************************************/

static LONG activeMethod (Class * cl, struct Gadget * g, struct gpInput * msg)
{
    struct objectData *od = INST_DATA (cl, g);
    struct InputEvent *ie = msg->gpi_IEvent;
    LONG retval = GMR_NOREUSE;

    /* Remember the current state */
    od->od_Previous = od->od_Current;

    /* See if we are being pushed */
    if (ie && (ie->ie_Class == IECLASS_RAWMOUSE) && (ie->ie_Code == SELECTDOWN))
    {
	if ((od->od_Hit >= 0) && (od->od_Current != od->od_Hit))
	{
	    od->od_Current = od->od_Hit;
	    retval = GMR_VERIFY | GMR_NOREUSE;
	    render (cl, g, msg->gpi_GInfo, GREDRAW_UPDATE);
	}
    }

    /* Set the termination value */
    *msg->gpi_Termination = (LONG)od->od_Current;

    return (retval);
}

/****** tabs.gadget/OM_NEW **************************************************
*
*    NAME
*       OM_NEW--Create a tabs.gadget object.                    (V42)
*
*    FUNCTION
*	The OM_NEW method is used to create an instance of the tabs.gadget
*	class.  This method is passed to the superclass first.
*
*    ATTRIBUTES
*	The following attributes can be specified at creation time.
*
*	GA_Top (LONG) -- The top of the gadget.  Should be
*	    win->BorderTop + 2.
*
*	GA_Left (LONG) -- The left edge of the gadget.
*
*	GA_Height (LONG) -- Height of the gadget.  Should be the font
*	    height plus seven.
*
*	GA_RelHeight (LONG) -- Although this attribute is supported, it is
*	    stylistically a bad thing to do.
*
*	GA_Disabled (BOOL) -- Determines whether the gadget is disabled or
*	    not.  Defaults to FALSE.
*
*	GA_TextAttr (struct TextAttr *) -- Text attribute for the font to
*	    use for the labels.
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

    /* Free any existing tab labels */
    FreeVec (od->od_TLP);
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

	case OM_GET:
	    return getAttrMethod (cl, g, (struct opGet *) msg);

	case GM_LAYOUT:
	    return layoutMethod (cl, g, (struct gpLayout *) msg);

	case GM_RENDER:
	    return renderMethod (cl, g, (struct gpRender *) msg);

	case GM_HITTEST:
	    return hitMethod (cl, g, (struct gpHitTest *) msg);

	case GM_GOACTIVE:
	    return activeMethod (cl, g, (struct gpInput *) msg);

	case OM_DISPOSE:
	    deleteMethod (cl, g, msg);
	default:
	    return DoSuperMethodA (cl, (Object *) g, (Msg *) msg);
    }
}
