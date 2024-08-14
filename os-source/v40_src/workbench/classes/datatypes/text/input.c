/* input.c
 *
 */

#include "textbase.h"

#define	DS(x)	;

/*****************************************************************************/

ULONG goinactive (struct TextLib * txl, Class * cl, Object * o, struct gpInput * msg)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct localData *lod = INST_DATA (cl, o);
    LONG wordt, topvt, topht;
    BOOL notify = FALSE;
    struct RastPort *rp;
    struct IBox *box;
    struct IBox sel;

    /* Ignore these values until they change... */
    wordt = topvt = topht = TAG_IGNORE;

    /* Get a pointer to the rastport */
    if (rp = ObtainGIRPort (msg->gpi_GInfo))
    {
	/* Get the domain of the gadget */
	GetAttr (DTA_Domain, o, (ULONG *) & box);

	/* Were we drag selecting? */
	if (si->si_Flags & DTSIF_DRAGSELECT)
	{
	    /* Good select? */
	    if (G (o)->Flags & SELECTED)
	    {
		if (lod->lod_Select.Top <= lod->lod_Select.Height)
		{
		    /* Prepare the select box */
		    sel.Left   = lod->lod_Select.Left;
		    sel.Top    = lod->lod_Select.Top;
		    sel.Width  = lod->lod_Select.Width;
		    sel.Height = lod->lod_Select.Height;
		}
		else
		{
		    /* Prepare the select box */
		    sel.Left   = lod->lod_Select.Left;
		    sel.Top    = lod->lod_Select.Height;
		    sel.Width  = lod->lod_Select.Width;
		    sel.Height = lod->lod_Select.Top;
		}

		/* Set the selected domain of the object */
		setdtattrs (txl, o, DTA_SelectDomain, &sel, TAG_DONE);

		/* Show that we are highlighted */
		si->si_Flags |= DTSIF_HIGHLIGHT;
		lod->lod_Flags |= LODF_HIGHLIGHT;
	    }
	    else
	    {
		/* Aborted by the right mouse button */
		DrawBox (txl, cl, o, rp, box, &lod->lod_Select, DBS_UP);
	    }

	    /* We need to clear the pointer */
	    notify = TRUE;
	}

	/* Was a word selected? */
	if (lod->lod_Flags & LODF_WORDSELECT)
	{
	    /* Double-click */
	    lod->lod_Flags ^= LODF_WORDSELECT;
	    wordt = TDTA_WordSelect;
	    notify = TRUE;
	}

	/* Did the top changed? */
	if (lod->lod_Flags & LODF_CHANGED)
	{
	    lod->lod_Flags &= ~LODF_CHANGED;
	    topvt = DTA_TopVert;
	    topht = DTA_TopHoriz;
	    notify = TRUE;
	}

	if (notify)
	{
	    /* Tell the world about our new data */
	    notifyAttrChanges (txl, o, msg->gpi_GInfo, NULL,
			       GA_ID, G (o)->GadgetID,
			       wordt, lod->lod_WordBuffer,
			       topvt, si->si_TopVert,
			       topht, si->si_TopHoriz,
			       DTA_Busy, FALSE,
			       TAG_DONE);
	}

	ReleaseGIRPort (rp);
    }

    /* Clear the selection flags */
    si->si_Flags &= ~(DTSIF_DRAGGING | DTSIF_DRAGSELECT);
    G (o)->Flags &= ~(SELECTED | ACTIVEGADGET);
    return (0);
}

/*****************************************************************************/

ULONG handleInput (struct TextLib * txl, Class * cl, Object * o, struct gpInput * msg)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct localData *lod = INST_DATA (cl, o);
    struct InputEvent *ie = msg->gpi_IEvent;
    ULONG retval = GMR_MEACTIVE;
    struct RastPort *rp;
    struct IBox *box;
    struct IBox *sel;
    LONG px, py;

    if ((msg->MethodID == GM_GOACTIVE) && (ie == NULL))
	return (GMR_NOREUSE);

    if (si->si_Flags & DTSIF_LAYOUT)
	return (GMR_NOREUSE);

    if (!AttemptSemaphoreShared (&(si->si_Lock)))
	return (GMR_NOREUSE);

    /* Get a pointer to the rastport */
    if (rp = ObtainGIRPort (msg->gpi_GInfo))
    {
	SetFont (rp, lod->lod_Font);
	rp->Mask = 0x1;

	GetAttr (DTA_Domain, o, (ULONG *) & box);

#if 0
	px = si->si_HorizUnit;
	py = si->si_VertUnit;
#endif

	/* Determine what to do... */
	switch (ie->ie_Class)
	{
		/* Mouse button or movement */
	    case IECLASS_RAWMOUSE:
		switch (ie->ie_Code)
		{
			/* Select button pressed */
		    case SELECTDOWN:
			if (lod->lod_Selected == NULL)
			    lod->lod_Selected = AllocVec ((sizeof (ULONG) * (si->si_VisVert + 2)), MEMF_CLEAR);

			if (si->si_Flags & DTSIF_DRAGSELECT)
			{
			    /* Show that we changed */
			    lod->lod_Flags |= LODF_CHANGED;

			    /* Clear selection */
			    if (GetAttr (DTA_SelectDomain, o, (ULONG *) & sel) && sel)
			    {
				DoMethod (o, DTM_CLEARSELECTED, msg->gpi_GInfo);
				SetFont (rp, lod->lod_Font);
				rp->Mask = 0x1;
			    }

			    /* Show that we are dragging */
			    si->si_Flags |= DTSIF_DRAGGING | DTSIF_DRAGSELECT;

			    /* Find the character that was selected */
			    FindLine (txl, cl, o, msg, rp, 1, box, DBS_DOWN);
			}
			else if (FindLine (txl, cl, o, msg, rp, 0, box, DBS_DOWN))
			{
			    /* Same word and within double-click time? */
			    if ((lod->lod_SWord == lod->lod_Word) &&
				(DoubleClick (lod->lod_SSec, lod->lod_SMic, msg->gpi_IEvent->ie_TimeStamp.tv_secs, msg->gpi_IEvent->ie_TimeStamp.tv_micro)))
			    {
				/* Set all the flag values that we need */
				lod->lod_Flags |= (LODF_CHANGED | LODF_WORDSELECT);
				G (o)->Flags |= SELECTED;

#if 0
				si->si_Flags |= DTSIF_DRAGSELECT;

				/* Clear selection */
				if (GetAttr (DTA_SelectDomain, o, (ULONG *) & sel) && sel)
				{
				    DoMethod (o, DTM_CLEARSELECTED, msg->gpi_GInfo);
				    SetFont (rp, lod->lod_Font);
				    rp->Mask = 0x1;
				}

				/* Show that we are dragging */
				si->si_Flags |= DTSIF_DRAGGING | DTSIF_DRAGSELECT;

				/* Draw the highlight */
				DrawBox (txl, cl, o, rp, box, &lod->lod_Select, DBS_DOWN);
#endif

				retval = (GMR_NOREUSE | GMR_VERIFY);
			    }
			    else
			    {
				/* Prepare the stamp */
				lod->lod_SSec = msg->gpi_IEvent->ie_TimeStamp.tv_secs;
				lod->lod_SMic = msg->gpi_IEvent->ie_TimeStamp.tv_micro;
				lod->lod_SWord = lod->lod_Word;
			    }
			}
			else
			{
			    /* No word, so cancel the activation */
			    retval = GMR_NOREUSE;
			}

			if (retval == GMR_MEACTIVE)
			{
			    G (o)->Flags |= SELECTED;

			    lod->lod_OTopHoriz = si->si_TopHoriz;
			    lod->lod_OTopVert = si->si_TopVert;
			}
			break;

			/* Select button released */
		    case SELECTUP:
			retval = (GMR_NOREUSE | GMR_VERIFY);
			break;

			/* Menu (ABORT) button pressed */
		    case MENUDOWN:
			si->si_TopHoriz = lod->lod_OTopHoriz;
			si->si_TopVert = lod->lod_OTopVert;
			G (o)->Flags &= ~SELECTED;
			retval = GMR_NOREUSE;
			break;

		    default:
			if (si->si_Flags & DTSIF_DRAGSELECT)
			{
			    /* Find the character that was selected */
			    FindLine (txl, cl, o, msg, rp, 1, box, DBS_MOVE);
			}
			break;
		}
		break;

	    case IECLASS_TIMER:
		scrollMethod (txl, cl, o, msg, rp, box);
		break;
	}

	ReleaseGIRPort (rp);
    }

    ReleaseSemaphore (&si->si_Lock);

    return (retval);
}

/*****************************************************************************/

VOID scrollMethod (struct TextLib * txl, Class * cl, Object * o, struct gpInput * msg, struct RastPort * rp, struct IBox * box)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct localData *lod = INST_DATA (cl, o);
    struct gpRender gpr;
    LONG px, py;
    LONG ox, oy;
    LONG topv;
    LONG toph;

    px = si->si_HorizUnit;
    py = si->si_VertUnit;
    toph = ox = si->si_TopHoriz;
    topv = oy = si->si_TopVert;

    if (msg->gpi_Mouse.X < 0)
	toph += (msg->gpi_Mouse.X / px);
    else if (msg->gpi_Mouse.X >= box->Width)
	toph -= ((box->Width - msg->gpi_Mouse.X) / px) - 1;

    if (msg->gpi_Mouse.Y < 0)
	topv += (msg->gpi_Mouse.Y / py);
    else if (msg->gpi_Mouse.Y >= box->Height)
	topv -= ((box->Height - msg->gpi_Mouse.Y) / py) - 1;

    if (toph > si->si_TotHoriz - si->si_VisHoriz)
	toph = si->si_TotHoriz - si->si_VisHoriz;
    if (toph < 0)
	toph = 0;

    if (topv > si->si_TotVert - si->si_VisVert)
	topv = si->si_TotVert - si->si_VisVert;
    if (topv < 0)
	topv = 0;

    if ((ox != toph) || (oy != topv))
    {
	lod->lod_Flags |= LODF_CHANGED;

	/* Were we drag selecting? */
	if (si->si_Flags & DTSIF_DRAGSELECT)
	{
	    /* Turn off the highlight */
	    DrawBox (txl, cl, o, rp, box, &lod->lod_Select, DBS_SCROLL1);
	}

	/* Force a redraw */
	si->si_TopHoriz = toph;
	si->si_TopVert  = topv;
	gpr.MethodID    = GM_RENDER;
	gpr.gpr_GInfo   = msg->gpi_GInfo;
	gpr.gpr_RPort   = rp;
	gpr.gpr_Redraw  = GREDRAW_UPDATE;
	DoMethodA (o, &gpr);

	/* Were we drag selecting? */
	if (si->si_Flags & DTSIF_DRAGSELECT)
	{
	    /* Find the character that was selected */
	    FindLine (txl, cl, o, msg, rp, 1, box, DBS_SCROLL2);
	    DrawBox (txl, cl, o, rp, box, &lod->lod_Select, DBS_SCROLL2);
	}
    }
}
