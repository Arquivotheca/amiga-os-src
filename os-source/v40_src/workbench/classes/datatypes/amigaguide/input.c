/* input.c
 *
 */

#include "amigaguidebase.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

static VOID translateInput (Class * cl, struct IBox *box, Object * o2, struct gpInput *msg)
{
    msg->gpi_Mouse.X -= (G (o2)->LeftEdge - box->Left);
    msg->gpi_Mouse.Y -= (G (o2)->TopEdge - box->Top);
}

/*****************************************************************************/

ULONG propagateHit (struct AGLib *ag, Class * cl, Object * o, struct gpHitTest *msg)
{
    struct ClientData *cd = INST_DATA (cl, o);
    struct NodeData *nd;
    struct IBox *box;

    GetAttr (DTA_Domain, o, (ULONG *) & box);
    cd->cd_Active = NULL;

    if ((cd->cd_Flags & AGF_CONTROLS) && (msg->gpht_Mouse.Y < cd->cd_Top - box->Top))
    {
	/* Set the control flag */
	cd->cd_Flags |= AGF_CONTROL;
    }
    else if (cd->cd_CurrentNode)
    {
	/* Clear the control flag */
	cd->cd_Flags &= ~AGF_CONTROL;

	nd = INST_DATA (ag->ag_NodeClass, cd->cd_CurrentNode);
	if (nd->nd_Object)
	{
	    cd->cd_Active = nd->nd_Object;
	}
    }
    return (GMR_GADGETHIT);
}

/*****************************************************************************/

ULONG goinactive (struct AGLib * ag, Class * cl, Object * o, struct gpInput * msg)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct ClientData *cd = INST_DATA (cl, o);
    LONG wordt, topvt, topht;
    struct TextFont *font;
    struct Rectangle rect;
    struct Region *old_r;
    struct RastPort *trp;
    struct RastPort *rp;
    struct NodeData *nd;
    BOOL notify = FALSE;
    struct SIPCMsg *sm;
    struct Line *line;
    struct IBox *box;
    struct IBox sel;
    ULONG msize;

    /* Get the node data for the current node */
    nd = INST_DATA (ag->ag_NodeClass, cd->cd_CurrentNode);

    if (cd->cd_Active)
    {
	struct DTSpecialInfo *esi = (struct DTSpecialInfo *) G (cd->cd_Active)->SpecialInfo;
	struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
	ULONG retval;

	retval = DoMethodA (cd->cd_Active, msg);
	si->si_Flags = esi->si_Flags;
	return (retval);
    }
    /* Get a pointer to the rastport */
    else if (rp = ObtainGIRPort (msg->gpi_GInfo))
    {
	if (nd->nd_Font)
	    font = nd->nd_Font;
	else if (cd->cd_Font)
	    font = cd->cd_Font;
	else
	    font = msg->gpi_GInfo->gi_DrInfo->dri_Font;
	SetFont (rp, font);

	/* Get the domain of the gadget */
	GetAttr (DTA_Domain, o, (ULONG *) & box);

	/* Do we have a current line? */
	if (line = cd->cd_CurLine)
	{
	    /* Set up the rectangle */
	    rect.MinX = box->Left;
	    rect.MinY = cd->cd_Top;
	    rect.MaxX = box->Left + box->Width - 1;
	    rect.MaxY = box->Top + box->Height - 1;

	    if (cd->cd_Flags & AGF_CONTROL)
	    {
		rect.MinY = box->Top;
		rect.MaxY = cd->cd_Top;
		trp = &cd->cd_Control;
	    }
	    else
		trp = &cd->cd_Render;

	    if (trp->Layer)
	    {
		/* Set up the rectangle */
		ClearRegion (cd->cd_Region);
		OrRectRegion (cd->cd_Region, &rect);
		old_r = InstallClipRegion (trp->Layer, cd->cd_Region);

		/* Draw the button frame */
		drawbevel (ag, cd->cd_Frame, rp, msg->gpi_GInfo->gi_DrInfo,
			   cd->cd_CurBox.Left, cd->cd_CurBox.Top,
			   cd->cd_CurBox.Width, cd->cd_CurBox.Height, TRUE);

		/* Draw the button text */
		if (line == cd->cd_ActiveLine)
		    SetABPenDrMd (trp, cd->cd_DrawInfo->dri_Pens[FILLTEXTPEN], cd->cd_DrawInfo->dri_Pens[FILLPEN], JAM2);
		else
		    SetABPenDrMd (trp, line->ln_FgPen, line->ln_BgPen, JAM2);
		Move (trp, cd->cd_CurX, cd->cd_CurY);
		Text (trp, line->ln_Text, line->ln_TextLen);

		/* Remove the clipping region */
		InstallClipRegion (trp->Layer, old_r);
	    }

	    /* Did they select a button */
	    if (cd->cd_Flags & AGF_SELECTED)
	    {
		if (cd->cd_Flags & AGF_CONTROL)
		{
		    struct dtTrigger dtt;

		    dtt.MethodID = DTM_TRIGGER;
		    dtt.dtt_GInfo = NULL;
		    dtt.dtt_Function = line->ln_Box.Left;
		    dtt.dtt_Data = NULL;
		    sam (ag, cd, &dtt, sizeof (struct dtTrigger));
		}
		else if (line && line->ln_Data)
		{
		    /* Set the new active line */
		    cd->cd_ActiveLine = line;
		    cd->cd_ActiveNum = line->ln_LinkNum;

		    /* Allocate the command message */
		    msize = sizeof (struct SIPCMsg) + strlen (line->ln_Data) + 1;

		    if (sm = AllocVec (msize, MEMF_CLEAR))
		    {
			sm->sm_Message.mn_Node.ln_Type = NT_MESSAGE;
			sm->sm_Message.mn_Length = msize;
			sm->sm_Type = SIPCT_COMMAND;
			sm->sm_Data = MEMORY_FOLLOWING (sm);
			strcpy (sm->sm_Data, line->ln_Data);
			PutMsg (cd->cd_SIPCPort, (struct Message *) sm);
		    }
		}
	    }
	}
	/* Was a word selected? */
	else if (cd->cd_Flags & AGF_WORDSELECT)
	{
	    /* Double-click */
	    cd->cd_Flags ^= AGF_WORDSELECT;
	    wordt = TDTA_WordSelect;

	    msize = sizeof (struct SIPCMsg) + (strlen (cd->cd_WordBuffer) * 2) + 14;

	    if (sm = AllocVec (msize, MEMF_CLEAR))
	    {
		sm->sm_Message.mn_Node.ln_Type = NT_MESSAGE;
		sm->sm_Message.mn_Length = msize;
		sm->sm_Type = SIPCT_COMMAND;
		sm->sm_Data = MEMORY_FOLLOWING (sm);
		sprintf (sm->sm_Data, "\"%s\" Link \"%s\"", cd->cd_WordBuffer, cd->cd_WordBuffer);
		PutMsg (cd->cd_SIPCPort, (struct Message *) sm);
	    }
	}
	/* Were we drag selecting */
	else if (si->si_Flags & DTSIF_DRAGSELECT)
	{
	    /* Good select? */
	    if (G (o)->Flags & SELECTED)
	    {
		if (cd->cd_Select.Top <= cd->cd_Select.Height)
		{
		    /* Prepare the select box */
		    sel.Left = cd->cd_Select.Left;
		    sel.Top = cd->cd_Select.Top;
		    sel.Width = cd->cd_Select.Width;
		    sel.Height = cd->cd_Select.Height;
		}
		else
		{
		    /* Prepare the select box */
		    sel.Left = cd->cd_Select.Left;
		    sel.Top = cd->cd_Select.Height;
		    sel.Width = cd->cd_Select.Width;
		    sel.Height = cd->cd_Select.Top;
		}

		/* Set the selected domain of the object */
		setattrs (ag, o, DTA_SelectDomain, &sel, TAG_DONE);

		/* Show that we are highlighted */
		si->si_Flags |= DTSIF_HIGHLIGHT;
	    }
	    else
	    {
		/* Aborted by the right mouse button */
		DrawBox (ag, cl, o, rp, box, &cd->cd_Select, DBS_UP);
	    }

	    /* We need to clear the pointer */
	    notify = TRUE;
	}

	/* Did the top changed? */
	if (cd->cd_Flags & AGF_CHANGED)
	{
	    cd->cd_Flags &= ~AGF_CHANGED;
	    topvt = DTA_TopVert;
	    topht = DTA_TopHoriz;
	    notify = TRUE;
	}

	if (notify)
	{
	    /* Tell the world about our new data */
	    notifyAttrChangesDM (ag, o, msg->gpi_GInfo, NULL,
				 GA_ID, G (o)->GadgetID,
				 wordt, cd->cd_WordBuffer,
				 topvt, si->si_TopVert,
				 topht, si->si_TopHoriz,
				 DTA_Busy, FALSE,
				 TAG_DONE);

	    /* Show that we aren't busy */
	    cd->cd_Flags &= ~AGF_BUSY;
	}

	/* Release the rastport */
	ReleaseGIRPort (rp);
    }

    if (cd->cd_Flags & AGF_CONTROL)
    {
	FreeVec (cd->cd_CurLine);
	cd->cd_Flags &= ~AGF_CONTROL;
    }

    /* Unselect the gadget */
    si->si_Flags &= ~(DTSIF_DRAGGING | DTSIF_DRAGSELECT);
    G (o)->Flags &= ~(SELECTED | ACTIVEGADGET);
    cd->cd_Flags &= ~AGF_SELECTED;
    cd->cd_CurLine = NULL;
    return (0);
}

/*****************************************************************************/

ULONG handleInput (struct AGLib * ag, Class * cl, Object * o, struct gpInput * msg)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct ClientData *cd = INST_DATA (cl, o);
    struct InputEvent *ie = msg->gpi_IEvent;
    struct GadgetInfo *gi = msg->gpi_GInfo;
    struct DrawInfo *dri = gi->gi_DrInfo;
    ULONG retval = GMR_MEACTIVE;
    struct Region *old_r = NULL;
    struct TextFont *font;
    struct Rectangle rect;
    struct RastPort *trp;
    struct RastPort *rp;
    struct NodeData *nd;
    struct Line *line;
    struct IBox *box;
    struct IBox *sel;
    LONG fx, fy;
    LONG fw, fh;
    BOOL raised;
    BOOL draw;
    UWORD y;

    if ((cd->cd_CurrentNode == NULL) || (cd->cd_Flags & AGF_BUSY))
	return (GMR_NOREUSE);
    if ((msg->MethodID == GM_GOACTIVE) && (ie == NULL))
	return (GMR_NOREUSE);
    if ((si->si_Flags & DTSIF_LAYOUT) || (!AttemptSemaphoreShared (&(si->si_Lock))))
	return (GMR_NOREUSE);

    cd = INST_DATA (ag->ag_Class, o);
    nd = INST_DATA (ag->ag_NodeClass, cd->cd_CurrentNode);

    prepareRastPorts (ag, cl, o);

    GetAttr (DTA_Domain, o, (ULONG *) & box);

    if (cd->cd_Active)
    {
	struct DTSpecialInfo *esi = (struct DTSpecialInfo *) G (cd->cd_Active)->SpecialInfo;

	esi->si_Flags = si->si_Flags;

	translateInput (cl, box, cd->cd_Active, msg);
	retval = DoMethodA (cd->cd_Active, msg);
    }
    /* Get a pointer to the rastport */
    else if (rp = ObtainGIRPort (msg->gpi_GInfo))
    {
	if (nd->nd_Font)
	    font = nd->nd_Font;
	else if (cd->cd_Font)
	    font = cd->cd_Font;
	else
	    font = gi->gi_DrInfo->dri_Font;
	SetFont (rp, font);

	line = cd->cd_CurLine;

	/* Set up the rectangle */
	rect.MinX = box->Left;
	rect.MinY = cd->cd_Top;
	rect.MaxX = box->Left + box->Width - 1;
	rect.MaxY = box->Top + box->Height - 1;

	if (cd->cd_Flags & AGF_CONTROL)
	{
	    rect.MinY = box->Top;
	    rect.MaxY = cd->cd_Top;
	    trp = &cd->cd_Control;
	}
	else
	    trp = &cd->cd_Render;

	/* Set up clipping region here */
	if (trp->Layer)
	{
	    /* Set up the rectangle */
	    ClearRegion (cd->cd_Region);
	    OrRectRegion (cd->cd_Region, &rect);
	    old_r = InstallClipRegion (trp->Layer, cd->cd_Region);
	}

	/* Determine what to do... */
	switch (ie->ie_Class)
	{
		/* Mouse button or movement */
	    case IECLASS_RAWMOUSE:
		switch (ie->ie_Code)
		{
		    case SELECTDOWN:
			G (o)->Flags |= SELECTED;

			/* Clear selection */
			if (GetAttr (DTA_SelectDomain, o, (ULONG *) & sel) && sel)
			    DoMethod (o, DTM_CLEARSELECTED, msg->gpi_GInfo);

			/* See if they are hitting in the control area */
			if (cd->cd_Flags & AGF_CONTROL)
			{
			    struct Controls *c;
			    LONG i;

			    /* They hit within the control panel */
			    for (i = 0; i < MAX_CONTROLS; i++)
			    {
				c = &cd->cd_Controls[i];

				if ((msg->gpi_Mouse.X >= c->c_Left) &&
				    (msg->gpi_Mouse.X <= c->c_Left + cd->cd_BoxWidth - 1) &&
				    (msg->gpi_Mouse.Y >= c->c_Top - box->Top) &&
				    (msg->gpi_Mouse.Y <= c->c_Top - box->Top + cd->cd_ControlHeight - 1))
				{
				    if (c->c_Flags & CF_DISABLED)
				    {
					retval = GMR_NOREUSE;
				    }
				    else if (line = AllocVec (sizeof (struct Line), MEMF_CLEAR | MEMF_PUBLIC))
				    {
					line->ln_Text = c->c_Label;
					line->ln_TextLen = c->c_LabelLen;
					line->ln_FgPen = dri->dri_Pens[TEXTPEN];
					line->ln_BgPen = dri->dri_Pens[BACKGROUNDPEN];
					line->ln_Box.Left = c->c_ID;
					line->ln_Font = font;

					cd->cd_CurLine = line;

					cd->cd_CurBox.Left = box->Left + c->c_Left;
					cd->cd_CurBox.Top = c->c_Top;
					cd->cd_CurBox.Width = cd->cd_BoxWidth;
					cd->cd_CurBox.Height = cd->cd_ControlHeight;

					cd->cd_CurX = box->Left + c->c_TLeft,
					    cd->cd_CurY = c->c_Top + cd->cd_Control.TxBaseline + 2;

					/* should be a routine for this since it's called so much */
					drawbevel (ag, cd->cd_Frame, rp, msg->gpi_GInfo->gi_DrInfo,
						   cd->cd_CurBox.Left, cd->cd_CurBox.Top,
						   cd->cd_CurBox.Width, cd->cd_CurBox.Height, FALSE);
					SetABPenDrMd (trp, dri->dri_Pens[FILLTEXTPEN], dri->dri_Pens[FILLPEN], JAM2);
					Move (trp, cd->cd_CurX, cd->cd_CurY);
					Text (trp, line->ln_Text, line->ln_TextLen);
					cd->cd_Flags |= AGF_SELECTED | AGF_CONTROL;
				    }
				    i = MAX_CONTROLS;
				}
			    }
			}
			/* Are we drag selecting? */
			else if (si->si_Flags & DTSIF_DRAGSELECT)
			{
			    /* Show that we changed */
			    cd->cd_Flags |= AGF_CHANGED;

			    /* Show that we are dragging */
			    si->si_Flags |= DTSIF_DRAGGING | DTSIF_DRAGSELECT;

			    /* Find the character that was selected */
			    FindLine (ag, cl, o, msg, rp, 1, box, DBS_DOWN);
			}
			/* Are we on a line? */
			else if (line = FindLine (ag, cl, o, msg, rp, 0, box, DBS_DOWN))
			{
			    /* Is this line a link? */
			    if (line->ln_Flags & LNF_LINK)
			    {
				/* If this isn't the active link, then make it the active link */
				if (cd->cd_ActiveLine && (line != cd->cd_ActiveLine))
				{
				    struct Line *oln = cd->cd_ActiveLine;
				    LONG y, vta;

				    /* Calculate the y position of the previous active button */
				    vta = UMult32 (si->si_TopVert, cd->cd_LineHeight);
				    y = (LONG) (cd->cd_Top + cd->cd_RPort.TxBaseline + 1) + (LONG) oln->ln_Box.Top - vta;

				    /* Set the new active line */
				    cd->cd_ActiveLine = line;
				    cd->cd_ActiveNum = line->ln_LinkNum;

				    /* Dehighlight the previous active button */
				    r_renderLink (ag, cd, oln, (LONG) box->Left - si->si_TopHoriz, y, 0);
				}

				fx = (LONG) line->ln_Box.Left - si->si_TopHoriz;
				fw = (LONG) line->ln_Box.Width;
				fh = (LONG) cd->cd_LineHeight;
				y = cd->cd_Top - box->Top + ((cd->cd_WY - si->si_TopVert) * si->si_VertUnit);

				cd->cd_CurLine = line;

				cd->cd_CurBox.Left = box->Left + fx;
				cd->cd_CurBox.Top = box->Top + y;
				cd->cd_CurBox.Width = fw;
				cd->cd_CurBox.Height = fh;

				cd->cd_CurX = box->Left + fx + 2;
				cd->cd_CurY = box->Top + y + font->tf_Baseline + 1;

				/* should be a routine for this since it's called so much */
				drawbevel (ag, cd->cd_Frame, rp, msg->gpi_GInfo->gi_DrInfo,
					   cd->cd_CurBox.Left, cd->cd_CurBox.Top,
					   cd->cd_CurBox.Width, cd->cd_CurBox.Height, FALSE);
				SetABPenDrMd (trp, dri->dri_Pens[FILLTEXTPEN], dri->dri_Pens[FILLPEN], JAM2);
				Move (trp, cd->cd_CurX, cd->cd_CurY);
				Text (trp, line->ln_Text, line->ln_TextLen);
				cd->cd_Flags |= AGF_SELECTED;
			    }
			    /* Did they double-click us? */
			    else if (DoubleClick (cd->cd_SSec, cd->cd_SMic, msg->gpi_IEvent->ie_TimeStamp.tv_secs, msg->gpi_IEvent->ie_TimeStamp.tv_micro))
			    {
				/* Same word? */
				if (cd->cd_SWord == cd->cd_Word)
				{
				    /* Set all the flag values that we need */
				    cd->cd_Flags |= (AGF_CHANGED | AGF_WORDSELECT);
				    G (o)->Flags |= SELECTED;
				    retval = (GMR_NOREUSE | GMR_VERIFY);
				}
			    }
			    /* Remember the time stamp */
			    else
			    {
				/* Prepare the stamp */
				cd->cd_SSec = msg->gpi_IEvent->ie_TimeStamp.tv_secs;
				cd->cd_SMic = msg->gpi_IEvent->ie_TimeStamp.tv_micro;
				cd->cd_SWord = cd->cd_Word;
			    }
			}
			else
			{
			    retval = GMR_NOREUSE;
			}
			break;

			/* Select button released */
		    case SELECTUP:
			retval = (GMR_NOREUSE | GMR_VERIFY);
			break;

			/* Menu (ABORT) button pressed */
		    case MENUDOWN:
			cd->cd_Flags &= ~AGF_SELECTED;
			retval = GMR_NOREUSE;
			break;

		    default:
			if (line)
			{
			    draw = FALSE;
			    raised = TRUE;
			    fx = cd->cd_CurBox.Left - box->Left;
			    fy = cd->cd_CurBox.Top - box->Top;

			    if ((msg->gpi_Mouse.X >= fx) &&
				(msg->gpi_Mouse.X <= fx + cd->cd_CurBox.Width - 1) &&
				(msg->gpi_Mouse.Y >= fy) &&
				(msg->gpi_Mouse.Y <= fy + cd->cd_CurBox.Height - 1))
			    {
				raised = FALSE;
			    }

			    if (cd->cd_Flags & AGF_SELECTED)
			    {
				if (raised)
				{
				    draw = TRUE;
				    if (line == cd->cd_ActiveLine)
					SetABPenDrMd (trp, dri->dri_Pens[FILLTEXTPEN], dri->dri_Pens[FILLPEN], JAM2);
				    else
					SetABPenDrMd (trp, line->ln_FgPen, line->ln_BgPen, JAM2);
				    cd->cd_Flags &= ~AGF_SELECTED;
				}
			    }
			    else
			    {
				if (!raised)
				{
				    draw = TRUE;
				    SetABPenDrMd (trp, dri->dri_Pens[FILLTEXTPEN], dri->dri_Pens[FILLPEN], JAM2);
				    cd->cd_Flags |= AGF_SELECTED;
				}
			    }

			    if (draw)
			    {
				drawbevel (ag, cd->cd_Frame, rp, msg->gpi_GInfo->gi_DrInfo,
					   cd->cd_CurBox.Left, cd->cd_CurBox.Top,
					   cd->cd_CurBox.Width, cd->cd_CurBox.Height, raised);
				Move (trp, cd->cd_CurX, cd->cd_CurY);
				Text (trp, line->ln_Text, line->ln_TextLen);
			    }
			}
			else if (si->si_Flags & DTSIF_DRAGSELECT)
			{
			    /* Find the character that was selected */
			    FindLine (ag, cl, o, msg, rp, 1, box, DBS_MOVE);
			}

			break;
		}
		break;
	}

	/* Remove the clipping region */
	if (trp->Layer)
	    InstallClipRegion (trp->Layer, old_r);

	/* Release the rastport */
	ReleaseGIRPort (rp);
    }

    /* Release the data lock */
    ReleaseSemaphore (&si->si_Lock);

    return (retval);
}
