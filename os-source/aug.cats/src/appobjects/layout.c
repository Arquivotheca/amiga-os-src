/* layout.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * layout gadgets
 *
 */

#include "appobjectsbase.h"

/* layout.c */
BOOL computeObjectDomain (struct WorkData * pwd, struct Object * cob, struct IBox * box, BOOL doit);
VOID scaleBox (struct WorkData * pwd, struct Object * cob, struct IBox * dest);
VOID computeWindowBox (struct WorkData * pwd, struct Object * cob, struct IBox * dest);
VOID computeObjectBox (struct WorkData * pwd, struct Object * cob, struct IBox * dest, struct IBox * cns);
VOID InitLayoutWorkData (struct WorkData * pwd);
VOID FreeLayoutWorkData (struct WorkData * pwd);
VOID computeWindowDomain (struct WorkData * pwd, struct Object * objl, struct TagItem * tl);
BOOL getfakeBox (struct IBox * wbox, struct IBox * dst);

/* This initialize the layout stuff.  This will replace the compute window
 * domain function. */
VOID InitLayoutWorkData (struct WorkData * pwd)
{
    struct List *list = &(pwd->pwd_GroupList);
    struct IBox *domain = &(pwd->pwd_NomWin);
    struct ObjectInfo *oi = pwd->pwd_OI;
    struct Object *objl = oi->oi_Objects;
    ULONG msize = sizeof (struct Group);
    struct Window *win = oi->oi_Window;
    struct Screen *scr = oi->oi_Screen;
    struct Object *group = NULL;
    struct IBox box, constraint;
    WORD width = 0, height = 0;
    struct Object *cob = objl;
    struct Group *gn = NULL;
    WORD minw = 0, minh = 0;
    struct TagItem *tag;
    UBYTE buffer[16];

    /* Initialize the group list */
    NewList (list);

    /* Step through the object array */
    while (cob)
    {
	/* Get the maximum level used */
	pwd->pwd_MaxLevel = MAX (pwd->pwd_MaxLevel, cob->o_Priority);

	/* Clear the box */
	box.Left = box.Top = box.Width = box.Height = 0;

	/* Make the group name */
	sprintf (buffer, "%ld", (LONG) cob->o_Group);

	/* What type of object do we have? */
	switch (cob->o_Type)
	{
		/* Handle the group types */
	    case OBJ_Window:
	    case OBJ_Group:
	    case OBJ_VGroup:
	    case OBJ_HGroup:
#if 0
	    case OBJ_MGroup:
#endif
		/* Allocate a group node */
		if (gn = (struct Group *) AllocVec (msize, MEMF_CLEAR))
		{
		    /* Increment the group counter */
		    pwd->pwd_Groups++;

		    /* Set up the name of the node */
		    strcpy (gn->gn_Name, buffer);
		    gn->gn_Node.ln_Name = gn->gn_Name;

		    /* Remember this position */
		    gn->gn_Header = group = cob;

		    /* Add the group to the list */
		    AddTail (list, (struct Node *) gn);

		    /* Special handling of the window group */
		    if (cob->o_Type == OBJ_Window)
		    {
			/* Are we in a refresh state? */
			if (oi->oi_Flags & AOIF_REFRESH)
			{
			    /* Set up the box sizes */
			    pwd->pwd_MinWin.Width = win->MinWidth - win->BorderRight - win->BorderLeft;
			    pwd->pwd_MinWin.Height = win->MinHeight - win->BorderBottom - win->BorderTop;
			    domain->Width = win->Width - win->BorderRight - win->BorderLeft;
			    domain->Height = win->Height - win->BorderBottom - win->BorderTop;
			    pwd->pwd_MaxWin.Width = win->MaxWidth - win->BorderRight - win->BorderLeft;
			    pwd->pwd_MaxWin.Height = win->MaxHeight - win->BorderBottom - win->BorderTop;

			    /* Figure in the window borders */
			    pwd->pwd_WBorLeft = win->BorderLeft - scr->WBorLeft;
			    pwd->pwd_WBorTop = win->BorderTop - scr->WBorTop;
			    pwd->pwd_WBorRight = win->BorderRight - scr->WBorRight;
			    pwd->pwd_WBorBottom = win->BorderBottom - scr->WBorBottom;

			    /* Copy the window normal box over */
			    box = *(&(pwd->pwd_NomWin));
			}
			else
			{
			    tag = oi->oi_WindowAttrs;

			    /* Compute the window box */
			    computeWindowBox (pwd, cob, &box);

			    /* Set the mins */
			    minw = pwd->pwd_MinWin.Width;
			    minh = pwd->pwd_MinWin.Height;

			    /* Make sure we have window attributes */
			    if (tag == NULL)
			    {
				if (cob->o_Tags)
				{
				    tag = CloneTagItems (cob->o_Tags);
				}
				else
				{
				    tag = MakeNewTagList (
							WA_MenuHelp, TRUE,
							WA_InnerWidth, NULL,
							WA_InnerHeight, NULL,
							WA_MinWidth, NULL,
							WA_MinHeight, NULL,
							TAG_DONE);
				}
			    }

			    /* Make sure we have inner tags */
			    if (!(FindTagItem (WA_InnerWidth, tag)) &&
				!(FindTagItem (WA_InnerHeight, tag)))
			    {
				struct TagItem *otags = tag;

				/* Make an attribute list */
				tag = MakeNewTagList (
							WA_MenuHelp, TRUE,
							WA_InnerWidth, NULL,
							WA_InnerHeight, NULL,
							TAG_MORE, otags,
							TAG_DONE);

				/* Free tags */
				FreeTagItems (otags);
			    }

			    /* Okay, dudes */
			    oi->oi_WindowAttrs = tag;
			}

			/* Copy the window minimum box over */
			constraint = *(&(pwd->pwd_MinWin));
		    }
		    else
		    {
			/* Make sure refresh events come through */
			oi->oi_NewWindow->IDCMPFlags |= REFRESHWINDOW;

			/* Environment contain GadTools objects */
			oi->oi_Flags |= AOIF_GADTOOLS;

			/* Compute the object size */
			computeObjectBox (pwd, cob, &box, &constraint);
		    }
		}
		break;

		/* Handle the object types */
	    default:
		/* Compute the object size */
		computeObjectBox (pwd, cob, &box, &constraint);

		/* Is this a GadTools object? */
		if (IsGadToolObj (cob))
		{
		    /* Make sure refresh events come through */
		    oi->oi_NewWindow->IDCMPFlags |= REFRESHWINDOW;

		    /* Environment contain GadTools objects */
		    oi->oi_Flags |= AOIF_GADTOOLS;
		}

		/* Make sure it is something we care about */
		if ((cob->o_Group > 0) && (cob->o_Type < OBJ_Screen))
		{
		    /* Do we have the correct group pointer? */
		    if (!group || (group && (group->o_Group != cob->o_Group)))
		    {
			/* Get the group record */
			if (gn = (struct Group *) FindName (list, buffer))
			{
			    /* Get the group object */
			    group = gn->gn_Header;
			}
		    }

		    /* Okay, do we have a group header? */
		    if (group && gn)
		    {
			/* Increment the number of members */
			gn->gn_Members++;

			/* Get the maximum level used */
			gn->gn_Level = MAX (gn->gn_Level, cob->o_Priority);

			/* Set the maximum sizes */
			gn->gn_MaxWidth = MAX (gn->gn_MaxWidth, box.Width);
			gn->gn_MaxHeight = MAX (gn->gn_MaxHeight, box.Height);
		    }
		}
		break;
	}

	/* Calculate some size stuff */
	width = MAX (width, ABS (box.Left + box.Width));
	height = MAX (height, ABS (box.Top + box.Height));
	minw = MAX (minw, ABS (box.Left + constraint.Left));
	minh = MAX (minh, ABS (box.Top + constraint.Top));

	/* Get the next object in the array */
	cob = cob->o_NextObject;
    }

    /* Are we in a refresh state? */
    if (!(oi->oi_Flags & AOIF_REFRESH))
    {
	/* Do we have any groups?  Should at least have the window group! */
	if (list->lh_TailPred != (struct Node *) list)
	{
	    struct Node *node, *nxtnode;
	    WORD gw, gh;

	    node = list->lh_Head;
	    while (nxtnode = node->ln_Succ)
	    {
		gn = (struct Group *) node;
		group = gn->gn_Header;
		gw = gh = 0;

		switch (group->o_Type)
		{
		    case OBJ_VGroup:
			gh = gn->gn_MaxHeight * gn->gn_Members;
			break;

		    case OBJ_HGroup:
			gw = gn->gn_MaxWidth * gn->gn_Members;
			break;
		}

		/* Calculate some size stuff */
		width = MAX (width, gw);
		height = MAX (height, gh);

		/* Get the next node in the list */
		node = nxtnode;
	    }
	}

	/* Adjust for the right margin */
	width += 8;
	height += 4;

	/* Don't allow the current to get smaller than the minimum */
	width = MAX (width, minw);
	height = MAX (height, minh);

	/* Set up the domain */
	domain->Left = domain->Top = 0;
	domain->Width = width;
	domain->Height = height;

	/* Set the work data */
	pwd->pwd_MinWin.Width = minw;
	pwd->pwd_MinWin.Height = minh;

	/* For backwards compatibility */
	oi->oi_NewWindow->Width = width;
	oi->oi_NewWindow->Height = height;
	oi->oi_NewWindow->MinWidth = minw;
	oi->oi_NewWindow->MinHeight = minh;

	/* Set the width */
	if (tag = FindTagItem (WA_InnerWidth, oi->oi_WindowAttrs))
	{
	    tag->ti_Data = (LONG) width;
	}

	/* Set the height */
	if (tag = FindTagItem (WA_InnerHeight, oi->oi_WindowAttrs))
	{
	    tag->ti_Data = (LONG) height;
	}

	/* Set the minimum width */
	if (tag = FindTagItem (WA_MinWidth, oi->oi_WindowAttrs))
	{
	    tag->ti_Data = (LONG) minw;
	}

	/* Set the minimum height */
	if (tag = FindTagItem (WA_MinHeight, oi->oi_WindowAttrs))
	{
	    tag->ti_Data = (LONG) minh;
	}
    }
}

BOOL computeObjectDomain (struct WorkData * pwd, struct Object * cob, struct IBox * box, BOOL doit)
{
    struct ObjectInfo *oi = pwd->pwd_OI;
    struct Screen *scr = oi->oi_Screen;
    struct NewWindow *nw = oi->oi_NewWindow;
    struct List *list = &(pwd->pwd_GroupList);
    struct Object *group = NULL;
    struct Group *gn = NULL;
    struct IBox *gbox = NULL;
    BOOL create = TRUE;
    UBYTE buffer[16];
    struct TagItem *otag = cob->o_Tags;
    struct TagItem *btag;
    UWORD border = NULL;
    UWORD top, left;

    /* Sniff that funky border */
    if (otag)
    {
	if (btag = FindTagItem (GA_Border, otag))
	{
	    border = (UWORD) btag->ti_Data;
	}
	else if (btag = FindTagItem (GA_RightBorder, otag))
	{
	    border = GACT_RIGHTBORDER;
	}
	else if (btag = FindTagItem (GA_LeftBorder, otag))
	{
	    border = GACT_LEFTBORDER;
	}
	else if (btag = FindTagItem (GA_TopBorder, otag))
	{
	    border = GACT_TOPBORDER;
	}
	else if (btag = FindTagItem (GA_BottomBorder, otag))
	{
	    border = GACT_BOTTOMBORDER;
	}
    }

    /* Build the name to look for */
    sprintf (buffer, "%ld", (LONG) cob->o_Group);

    /* Get the object's box size */
    computeObjectBox (pwd, cob, box, NULL);

    if (!(cob->o_Flags & APSH_OBJF_NOADJUST))
    {
	top = box->Top;
	left = box->Left;

	/* Accomodate the window's title bar if there's a need */
	if ((border == NULL) ||
	    (border & (GACT_LEFTBORDER | GACT_RIGHTBORDER)))
	{
	    if (box->Top >= 0)
	    {
		box->Top += (scr->WBorTop + pwd->pwd_WBorTop);
	    }
	    else
	    {
		box->Top -= (scr->WBorBottom + pwd->pwd_WBorBottom - 1);
	    }
	}

	/* Accomodate the window's left border */
	if ((border == NULL) ||
	    (border & (GACT_TOPBORDER | GACT_BOTTOMBORDER)))
	{
	    if (box->Left >= 0)
	    {
		box->Left += (scr->WBorLeft + pwd->pwd_WBorLeft);
	    }
	    else
	    {
		box->Left -= (scr->WBorRight + pwd->pwd_WBorRight - 1);
	    }
	}

	/* Accomodate the window's left & right borders */
	if (box->Width < 0)
	{
	    box->Width -= ((scr->WBorLeft + pwd->pwd_WBorLeft) +
			   (scr->WBorRight + pwd->pwd_WBorRight));
	}
	else if ((box->Width == 0) &&
		 (border & (GACT_TOPBORDER | GACT_BOTTOMBORDER)))
	{
	    box->Width = -(scr->WBorLeft + pwd->pwd_WBorLeft + left +
			   scr->WBorRight + pwd->pwd_WBorRight - 5);
	}

	/* Accomodate the window's top & bottom borders */
	if (box->Height < 0)
	{
	    box->Height -= ((scr->WBorTop + pwd->pwd_WBorTop) +
			    (scr->WBorBottom + pwd->pwd_WBorBottom));
	}
	else if ((box->Height == 0) &&
		 (border & (GACT_LEFTBORDER | GACT_RIGHTBORDER)))
	{
	    box->Height = -(scr->WBorTop + pwd->pwd_WBorTop + top +
			    scr->WBorBottom + pwd->pwd_WBorBottom - 2);
	}
    }

    nw->IDCMPFlags |= (SIZEVERIFY | NEWSIZE);

    /* Time to adjust? */
    if (oi->oi_Flags & AOIF_GADTOOLS)
    {
	struct IBox wbox;

	/* Copy the box */
	wbox.Width = scr->WBorLeft + pwd->pwd_WBorLeft + pwd->pwd_NomWin.Width + scr->WBorRight + pwd->pwd_WBorRight;
	wbox.Height = scr->WBorTop + pwd->pwd_WBorTop + pwd->pwd_NomWin.Height + scr->WBorBottom + pwd->pwd_WBorBottom;

	/* Is the box relative? */
	if (getfakeBox (&wbox, box))
	{
	    /* Is this a GadTools object? */
	    if ((oi->oi_Flags & AOIF_GADTOOLS) || IsGadToolObj (cob))
	    {
		/* Tell the window to do special magic */
		oi->oi_Flags |= AOIF_REMOVE;
		nw->IDCMPFlags |= (SIZEVERIFY | NEWSIZE);
	    }
	}

	/* Get the group record */
	if ((cob->o_Group > 0) &&
	    (gn = (struct Group *) FindName (list, buffer)))
	{
	    /* Get the group object */
	    group = gn->gn_Header;

	    /* Are we in ourselves? */
	    if (group == cob)
	    {
		/* Copy the box over */
		gn->gn_Domain = *box;
	    }

	    /* Cache the pointer to the group box */
	    gbox = &(gn->gn_Domain);
	}

	/* Have a group to worry about? */
	if (group &&
	    (cob->o_Group > 0) &&
	    (cob->o_Type < OBJ_Window))
	{
	    struct Object *objl = oi->oi_Objects;
	    struct Object *me = NULL;
	    struct Object *node;
	    BOOL nofit = TRUE;	/* Do we need to try fitting again? */
	    WORD level;		/* Dropout level */
	    WORD members;	/* Total number of members */
	    WORD prior;		/* # members before current */
	    WORD mw, mh;	/* total member width and height */
	    WORD vfill;		/* Vertical fill amount */
	    WORD hfill;		/* Horizontal fill amount */
	    WORD left;
	    WORD top;

	    /* Get the domain of the group */
	    getfakeBox (&(pwd->pwd_NomWin), gbox);

	    /* Start at maximum, plus one */
	    level = gn->gn_Level + 1;

	    /* Do we fit yet? (was o_Group) */
	    while (nofit && (cob->o_Priority <= level))
	    {
		/* Clear the variables */
		members = prior = 0;
		node = objl;
		mw = mh = 0;
		me = NULL;

		/* Decrement the maximum dropout level */
		level--;

		/* Loop through the object list */
		while (node)
		{
		    /* Is it a member of the current group? */
		    if (node->o_Group == cob->o_Group)
		    {
			switch (node->o_Type)
			{
			    case OBJ_Group:
			    case OBJ_VGroup:
			    case OBJ_HGroup:
			    case OBJ_Window:
				break;

			    default:
				/* Include this one? */
				if (node->o_Priority <= level)
				{
				    mw += gn->gn_MaxWidth;
				    mh += gn->gn_MaxHeight;
				    members++;

				    /* Inefficient way of finding position */
				    if ((node != cob) && (me == NULL))
				    {
					prior++;
				    }
				    else
				    {
					me = cob;
				    }
				}

				break;
			}
		    }		/* End of if in our group */

		    /* Step to the next object */
		    node = node->o_NextObject;

		}		/* End of while node */

		/* Say we fit... */
		nofit = FALSE;

		/* Are we within a workable group? */
		if (level >= 0)
		{
		    if (group->o_Type == OBJ_HGroup)
		    {
			box->Width = gn->gn_MaxWidth;
			box->Height = gn->gn_MaxHeight;

			vfill = 0;
			if (members > 1)
			{
			    vfill = (gbox->Width - (members * box->Width)) / (members - 1);
			    vfill = MAX (0, vfill);
			}

			left = gbox->Left + (prior * (vfill + box->Width));
			box->Left = MAX (gbox->Left, left);

			top = gbox->Top + ((gbox->Height - box->Height) / 2);
			box->Top = MAX (gbox->Top, top);

			if (((members * box->Width) +
			     ((members - 1) * vfill)) >
			    gbox->Width)
			{
			    nofit = TRUE;
			}
		    }
		    else if (group->o_Type == OBJ_VGroup)
		    {
			box->Height = gn->gn_MaxHeight;

			hfill = 0;
			if (members > 1)
			{
			    hfill = (gbox->Height - (members * box->Height)) / (members - 1);
			    hfill = MAX (0, hfill);
			}

			box->Left = gbox->Left + box->Left;

			top = gbox->Top + (prior * (hfill + box->Height));
			box->Top = MAX (gbox->Top, top);

			if (((members * box->Height) +
			     ((members - 1) * hfill)) >
			    gbox->Height)
			{
			    nofit = TRUE;
			}
		    }
#if 0
		    else if (group->o_Type == OBJ_MGroup)
		    {
		    }
		    else if (group->o_Type == OBJ_Group)
		    {
		    }
#endif
		}
	    }

	    /* Do we fit into the plan? */
	    if (cob->o_Priority > level)
	    {
		/* Don't create it */
		create = FALSE;
	    }
	}
    }

    return (create);
}

VOID computeWindowBox (struct WorkData * pwd, struct Object * cob, struct IBox * dest)
{
    struct ObjectInfo *oi = pwd->pwd_OI;
    struct NewWindow *nw = oi->oi_NewWindow;
    struct TagItem *attrs = cob->o_Tags;
    WORD tw = 0, th = 0, wx, wy, ww, wh;
    struct Screen *scr = oi->oi_Screen;

    /* Scale the box coordinates */
    {
	struct IBox domain = {NULL};

	/* Scale the rectangle */
	scaleBox (pwd, cob, &domain);

	/* Copy the values back */
	wx = domain.Left;
	wy = domain.Top;
#if 1
	ww = domain.Width;
	wh = domain.Height;
#else
	ww = (domain.Width && (nw->Width==0)) ? domain.Width : nw->Width;
	wh = (domain.Height && (nw->Height==0)) ? domain.Height : nw->Height;
#endif
    }

    /* Do we have any attributes? */
    if (attrs)
    {
	struct TagItem *tags = attrs;
	struct TagItem *tag;
	ULONG tidata;

	while (tag = NextTagItem (&tags))
	{
	    tidata = tag->ti_Data;
	    switch (tag->ti_Tag)
	    {
		case WA_SizeGadget:
		    if (tidata)
		    {
			nw->Flags |= WINDOWSIZING;
		    }
		    else
		    {
			nw->Flags &= ~WINDOWSIZING;
		    }
		    break;

		case WA_DragBar:
		case WA_DepthGadget:
		case WA_CloseGadget:
		case WA_Zoom:
		    if (tidata)
		    {
			WORD tmp = (scr->Font->ta_YSize + 1);

			/* Adjust the window title bar size */
			pwd->pwd_WBorTop = MAX (pwd->pwd_WBorTop, tmp);
		    }
		    break;

		case WA_MinWidth:
		    pwd->pwd_MinWin.Width = (WORD)
		      SCALE (pwd->pwd_swidth, pwd->pwd_fwidth, tidata);
		    tw = MAX (tw, pwd->pwd_MinWin.Width);
		    break;

		case WA_MinHeight:
		    pwd->pwd_MinWin.Height = (WORD)
		      SCALE (pwd->pwd_sheight, pwd->pwd_fheight, tidata);
		    th = MAX (th, pwd->pwd_MinWin.Height);
		    break;

		case WA_InnerWidth:
		    pwd->pwd_NomWin.Width = (WORD)
		      SCALE (pwd->pwd_swidth, pwd->pwd_fwidth, tidata);
		    tw = MAX (tw, pwd->pwd_NomWin.Width);
		    break;

		case WA_InnerHeight:
		    pwd->pwd_NomWin.Height = (WORD)
		      SCALE (pwd->pwd_sheight, pwd->pwd_fheight, tidata);
		    th = MAX (th, pwd->pwd_NomWin.Height);
		    break;
	    }
	}
    }

    if ((nw->Flags & (WINDOWDRAG | WINDOWDEPTH | WINDOWCLOSE)) ||
	(nw->Title))
    {
	WORD tmp = (scr->Font->ta_YSize + 1);

	/* Adjust the window title bar size */
	pwd->pwd_WBorTop = MAX (pwd->pwd_WBorTop, tmp);
    }

    if (nw->Flags & WINDOWSIZING)
    {
	/* Adjust for the sizing gadget */
	pwd->pwd_WBorRight = MAX (pwd->pwd_WBorRight, 19);
    }

    /* Do we have a window title */
    if (cob->o_LabelID > 0L)
    {
	struct RastPort *rp = &(oi->oi_RastPort);
	STRPTR text = NULL;
	WORD tmp = (scr->Font->ta_YSize + 1);

	/* Adjust the window title bar size */
	pwd->pwd_WBorTop = MAX (pwd->pwd_WBorTop, tmp);

	/* Get the window title */
	if (cob->o_Flags & APSH_OBJF_GLOBALTEXT)
	{
	    if (oi->oi_LocalText)
	    {
		text = (UBYTE *) (oi->oi_LocalText[cob->o_LabelID]);
	    }
	}
	else
	{
	    text = (UBYTE *) (oi->oi_TextTable[cob->o_LabelID]);
	}
	nw->Title = text;

	/* Get the length of the window title */
	tw = TextLength (rp, text, strlen (text));

	/* Provide some reasonable minimum */
	pwd->pwd_MinWin.Width = MAX (pwd->pwd_MinWin.Width, (tw + (3 * 19)));
	pwd->pwd_MinWin.Height = MAX (pwd->pwd_MinWin.Height, (scr->WBorTop + scr->Font->ta_YSize + 1));

	/* Width is the maximum of text or preset */
	ww = MAX (tw, ww);

	/* Height is the maximum of text or preset */
	wh = MAX ((pwd->pwd_fheight + 1), wh);
    }

    /* Set which ever is maximum */
    ww = MAX (ww, pwd->pwd_MinWin.Width);
    wh = MAX (wh, pwd->pwd_MinWin.Height);
    ww = MAX (tw, ww);
    wh = MAX (th, wh);

    /* Only adjust width & height if sizeable */
#if 0
    if (nw->Flags & WINDOWSIZING)
#endif
    {
	ww = (ww && (nw->Width == 0)) ? ww : (nw->Width - 8);
	wh = (wh && (nw->Height == 0)) ? wh : (nw->Height - 4);
    }

    /* Copy the values over */
    dest->Left = pwd->pwd_NomWin.Left = wx;
    dest->Top = pwd->pwd_NomWin.Top = wy;
    dest->Width = pwd->pwd_NomWin.Width = ww;
    dest->Height = pwd->pwd_NomWin.Height = wh;
}

VOID computeObjectBox (struct WorkData * pwd, struct Object * cob,
		        struct IBox * dest, struct IBox * cns)
{
    struct ObjectInfo *oi = pwd->pwd_OI;
    struct RastPort *rp = &(oi->oi_RastPort);
    struct TagItem *otag = cob->o_Tags;
    struct TagItem *btag;
    UWORD border = NULL;
    WORD tw = 0;

    /* Scale the rectangle */
    scaleBox (pwd, cob, dest);

    /* Initialize the constraints to be the same as the nominal */
    if (cns)
    {
	/* Minimum */
	cns->Left = MAX (0, dest->Width);
	cns->Top = MAX (0, dest->Height);

	/* Maximum */
	cns->Width = MAX (0, dest->Width);
	cns->Height = MAX (0, dest->Height);
    }

    /* Do we have a label */
    if (cob->o_LabelID > 0L)
    {
	STRPTR text = NULL;

	if (cob->o_Flags & APSH_OBJF_GLOBALTEXT)
	{
	    if (oi->oi_LocalText)
	    {
		text = (UBYTE *) (oi->oi_LocalText[cob->o_LabelID]);
	    }
	}
	else
	{
	    text = (UBYTE *) (oi->oi_TextTable[cob->o_LabelID]);
	}


	/* Get the label text */
	if (text)
	{
	    /* Get the length of the label */
	    tw = TextLength (rp, text, strlen (text));
	}
    }

    /* Adjust based on type */
    switch (cob->o_Type)
    {
	case OBJ_Button:
	    /* Set minimum height to be font height plus border */
	    dest->Height = pwd->pwd_fheight + 6;

	    /* Is the destination width already set? */
	    if (dest->Width == 0)
	    {
		dest->Width = (tw + 4 + pwd->pwd_fwidth);
	    }

	    if (cns)
	    {
		cns->Width = cns->Left = dest->Width;
		cns->Height = cns->Top = dest->Height;
	    }
	    break;

	case OBJ_MultiText:
	case OBJ_Listview:
	    if (cns)
	    {
		cns->Width = cns->Left = MAX (dest->Width, 50);
		cns->Height = cns->Top = MAX (dest->Height, 30);
	    }
	    break;

	case OBJ_Integer:
	case OBJ_Number:
	case OBJ_String:
	case OBJ_Text:
	case OBJ_DirString:
	case OBJ_DirNumeric:
	    dest->Height = pwd->pwd_fheight + 6;
	    if (cns)
	    {
		cns->Height = cns->Top = dest->Height;
	    }
	    break;

	case OBJ_Checkbox:
	    dest->Height = pwd->pwd_fheight;

	    /* Is the destination width already set? */
	    if (dest->Width == 0)
	    {
		dest->Width = (tw + 20 + pwd->pwd_fwidth);
	    }

	    if (cns)
	    {
		cns->Width = cns->Left = dest->Width;
		cns->Height = cns->Top = dest->Height;
	    }
	    break;

	case OBJ_MX:
	    dest->Height = pwd->pwd_fheight;
	    if (cns)
	    {
		cns->Height = cns->Top = dest->Height;
	    }
	    break;

	case OBJ_Cycle:
	    dest->Height = MAX ((pwd->pwd_fheight + 6), 14);
	    if (cns)
	    {
		cns->Height = cns->Top = dest->Height;
	    }
	    break;

	case OBJ_Dropbox:
	    /* This might have to change to support relativity */
	    dest->Left = dest->Left + MAX (0, ((dest->Width - 90) / 2));
	    dest->Top = dest->Top + MAX (0, ((dest->Height - 44) / 2));
	    dest->Width = 90;
	    dest->Height = 44;
	    if (cns)
	    {
		cns->Width = cns->Left = dest->Width;
		cns->Height = cns->Top = dest->Height;
	    }
	    break;

	    /* Find minimum size of all it's members */
	default:
	    break;
    }

    /* Sniff that funky border */
    if (otag)
    {
	if (btag = FindTagItem (GA_Border, otag))
	{
	    border = (UWORD) btag->ti_Data;
	}
	else if (btag = FindTagItem (GA_RightBorder, otag))
	{
	    border = GACT_RIGHTBORDER;
	}
	else if (btag = FindTagItem (GA_LeftBorder, otag))
	{
	    border = GACT_LEFTBORDER;
	}
	else if (btag = FindTagItem (GA_TopBorder, otag))
	{
	    border = GACT_TOPBORDER;
	}
	else if (btag = FindTagItem (GA_BottomBorder, otag))
	{
	    border = GACT_BOTTOMBORDER;
	}

	if (border)
	{
	    if (border & GACT_RIGHTBORDER)
	    {
		pwd->pwd_WBorRight = MAX (pwd->pwd_WBorRight, dest->Width);
	    }
	    else if (border & GACT_LEFTBORDER)
	    {
		pwd->pwd_WBorLeft = MAX (pwd->pwd_WBorLeft, dest->Width);
	    }
	    else if (border & GACT_TOPBORDER)
	    {
		pwd->pwd_WBorTop = MAX (pwd->pwd_WBorTop, dest->Height);
	    }
	    else if (border & GACT_BOTTOMBORDER)
	    {
		pwd->pwd_WBorBottom = MAX (pwd->pwd_WBorBottom, dest->Height);
	    }
	}
    }

    /* Don't count low priority items */
    if (cns && (cob->o_Priority > 0))
    {
	/* Clear the values */
	cns->Left = cns->Top = 0;
    }
}

VOID FreeLayoutWorkData (struct WorkData * pwd)
{

    /* Free the list */
    FreeExecList (&(pwd->pwd_GroupList), NULL);
}

BOOL getfakeBox (struct IBox * wbox, struct IBox * dst)
{
    struct IBox domain = *wbox;
    struct Gadget gad;
    BOOL rel = FALSE;

    /* Clear the domain to upper-left of 0,0 */
    domain.Left = 0;
    domain.Top = 0;

    /* Clear the flags */
    gad.Flags = NULL;

    /* Make a dummy gadget box */
    if ((gad.LeftEdge = dst->Left) < 0)
    {
	gad.Flags |= GRELRIGHT;
    }

    if ((gad.TopEdge = dst->Top) < 0)
    {
	gad.Flags |= GRELBOTTOM;
    }

    if ((gad.Width = dst->Width) < 0)
    {
	gad.Flags |= GRELWIDTH;
    }

    if ((gad.Height = dst->Height) < 0)
    {
	gad.Flags |= GRELHEIGHT;
    }

    /* It's relative */
    if (gad.Flags)
    {
	/* Get the correct gadget box */
	gadgetBox (&gad, &domain, dst, NULL, NULL);

	rel = TRUE;
    }

    return (rel);
}

VOID scaleBox (struct WorkData * pwd, struct Object * cob, struct IBox * dest)
{

    /* Copy the values over */
    *dest = *(&(cob->o_Outer));

    /* See if we should adjust the coordinates */
    if (!(cob->o_Flags & APSH_OBJF_NOADJUST) &&
	!(cob->o_Flags & APSH_OBJF_NOSCALE))
    {
	/* Calculate new upper-left coordinates */
	dest->Left = (WORD) SCALE (pwd->pwd_swidth, pwd->pwd_fwidth, (LONG) dest->Left);
	dest->Top = (WORD) SCALE (pwd->pwd_sheight, pwd->pwd_fheight, (LONG) dest->Top);

	/* Calculate new size */
	dest->Width = (WORD) SCALE (pwd->pwd_swidth, pwd->pwd_fwidth, (LONG) dest->Width);
	dest->Height = (WORD) SCALE (pwd->pwd_sheight, pwd->pwd_fheight, (LONG) dest->Height);
    }
}
