/* columniclass.c --- ListView image class
 * Copyright (C) 1991 Commodore-Amiga, Inc.
 * All Rights Reserved Worldwide
 * Written by David N. Junod
 *
 */

#include "appobjectsbase.h"
#include "appobjects_protos.h"

#define	IM(x)	((struct Image *)(x))

extern struct Library *GfxBase, *LayersBase, *IntuitionBase, *UtilityBase;

/* Public ListView class */
#define MYCLASSID	"columniclass"
#define SUPERCLASSID	IMAGECLASS

/* lviclass.c */
Class *initColumnIClass (VOID);
ULONG freeColumnIClass (Class * cl);
ULONG __saveds dispatchColumnI (Class * cl, Object * o, Msg msg);
ULONG initColumnI (Class *cl, Object *o, struct opSet *msg);
ULONG setColumnIAttrs (Class * cl, Object * o, struct opSet * msg);
ULONG drawColumnI (Class * cl, Object * o, struct impDrawColumn * msg);

struct ColumnImageData
{
    struct LVSpecialInfo *lod_SI;
    LONG lod_OTop;			/* Previous top line */
    ULONG lod_Flags;			/* See defines below (and appobjectsbase) */
    ULONG lod_FieldOffset;
    ULONG lod_FieldType;
    UBYTE lod_Buffer[36];		/* Numeric buffer */
    struct TextExtent lod_TBuf;		/* Temporary text extent */
};

#define	AOLVF_HIGHLIGHT		0x00000002

/* AOLV_Justification flags */
#ifndef AOLVF_LEFT
#define	AOLVF_LEFT		0x00000000
#define	AOLVF_RIGHT		0x00000010
#define	AOLVF_CENTER		0x00000020
#endif
#define	JUSTIFY	(AOLVF_LEFT | AOLVF_CENTER | AOLVF_RIGHT)

#define	LODSIZE	(sizeof(struct ColumnImageData))

Class *initColumnIClass (VOID)
{
    extern ULONG hookEntry ();
    Class *cl = NULL;

    if (cl = MakeClass (MYCLASSID, SUPERCLASSID, NULL, LODSIZE, 0))
    {
	/* initialize the cl_Dispatcher Hook */
	cl->cl_Dispatcher.h_Entry = hookEntry;
	cl->cl_Dispatcher.h_SubEntry = dispatchColumnI;

	/* Make the class public */
	AddClass (cl);
    }

    return (cl);
}

ULONG freeColumnIClass (Class * cl)
{

    if (cl)
    {
	/* Free the gadget class */
	return (ULONG) FreeClass (cl);
    }

    return (NULL);
}

ULONG __saveds dispatchColumnI (Class * cl, Object * o, Msg msg)
{
    ULONG retval = 0L;
    Object *newobj;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    /* Get a new object */
	    if (newobj = (Object *) DSM (cl, o, msg))
	    {
		/* Initialize the object */
		if (!(initColumnI (cl, newobj, (struct opSet *) msg)))
		{
		    /* free what's there if failure */
		    CoerceMethod (cl, newobj, OM_DISPOSE);
		    newobj = NULL;
		}
	    }

	    retval = (ULONG) newobj;
	    break;

	case OM_SET:
	    /* let the superclass see the atts */
	    DSM (cl, o, msg);

	    /* set the ones I care about */
	    retval = setColumnIAttrs (cl, o, (struct opSet *) msg);
	    break;

	case LV_DRAWCOLUMN:	/* Only type of draw we should ever receive */
	    retval = drawColumnI (cl, o, (struct impDrawColumn *) msg);
	    break;

	/* Pass it on to the superclass */
	default:
	    retval = (ULONG) DSM (cl, o, msg);
	    break;

    }

    return (retval);
}

ULONG initColumnI (Class *cl, Object *o, struct opSet *msg)
{
    setColumnIAttrs (cl, o, msg);
    return (TRUE);
}

ULONG setColumnIAttrs (Class * cl, Object * o, struct opSet * msg)
{
    struct ColumnImageData *lod = INST_DATA (cl, o);
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG retval = 0;
    ULONG tidata;

    /* process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case AOLV_SpecialInfo:
		lod->lod_SI = (struct LVSpecialInfo *) tidata;
		break;

	    case AOLV_FieldOffset:
		lod->lod_FieldOffset = (ULONG) tidata;
		break;

	    case AOLV_FieldType:
		lod->lod_FieldType = (ULONG) tidata;
		break;

	    case AOLV_Justification:
		lod->lod_Flags &= ~JUSTIFY;
		lod->lod_Flags |= (tidata & JUSTIFY);
		break;

	    case AOLV_ColumnWidth:
		IM (o)->ImageData = (USHORT *) tidata;
		break;
	}
    }

    return (retval);
}

#define	MAKE_LONG(a,b,c,d) ((unsigned char)(a)<<24 | (unsigned char)(b)<<16 | (unsigned char)(c)<<8 | (unsigned char)(d))
#define	MAKE_SHORT(a,b)	((unsigned char)(a)<<8 | (unsigned char)(b))

#if 0
VOID longtostr (ULONG num, STRPTR buffer, WORD digits)
{

    buffer += digits;
    *buffer = '\0';
    while (--digits >= 0)
    {
	*(--buffer) = num % 10 + '0';
	num /= 10;
    }
}

#endif

STRPTR GetField (Class * cl, Object * o, struct Node * node)
{
    struct ColumnImageData *lod = INST_DATA (cl, o);
    UBYTE *n = (unsigned char *) node;
    ULONG type = lod->lod_FieldType;
    ULONG l = lod->lod_FieldOffset;
    STRPTR retval;

    lod->lod_Buffer[0] = 0;
    retval = lod->lod_Buffer;

    /* Decode the field type */
    switch (type)
    {
	case 0:
	    retval = node->ln_Name;
	    break;

	case FT_STRPTR:
	    retval = (STRPTR) MAKE_LONG (n[(l + 0)], n[(l + 1)], n[(l + 2)], n[(l + 3)]);
	    break;

	case FT_STRING:
	    retval = &n[l];
	    break;

	case FT_LONG:
	    sprintf (retval, "%ld", (long) MAKE_LONG (n[(l + 0)], n[(l + 1)], n[(l + 2)], n[(l + 3)]));
	    break;

	case FT_ULONG:
	    sprintf (retval, "%ld", (unsigned long) MAKE_LONG (n[(l + 0)], n[(l + 1)], n[(l + 2)], n[(l + 3)]));
	    break;

	case FT_SHORT:
	    sprintf (retval, "%d", (short) MAKE_SHORT (n[(l + 0)], n[(l + 1)]));
	    break;

	case FT_USHORT:
	    sprintf (retval, "%d", (unsigned short) MAKE_SHORT (n[(l + 0)], n[(l + 1)]));
	    break;

	case FT_BYTE:
	    sprintf (retval, "%d", (char) n[l]);
	    break;

	case FT_UBYTE:
	    sprintf (retval, "%d", (unsigned char) n[l]);
	    break;

	default:
	    break;
    }
    if (retval == NULL)
	retval = lod->lod_Buffer;
    return (retval);
}

ULONG drawColumnI (Class * cl, Object * o, struct impDrawColumn * msg)
{
    struct ColumnImageData *lod = INST_DATA (cl, o);
    struct LVSpecialInfo *si = lod->lod_SI;
    struct RastPort *rp = msg->imp_RPort;
    LONG visible = si->si_VisibleVert;
    struct List *list = si->si_List;
    LONG top = si->si_TopVert;
    UWORD etx, ety, ebx, eby;
    struct TextFont *font;
    struct Node *nxtnode;
    WORD tx, ty, bx, by;
    STRPTR text = NULL;
    struct Node *node;
    LONG display = 0L;
    LONG count = 0L;
    struct IBox box;
    UBYTE backpen;
    UWORD cx, cy;
    UWORD *pens;		/* pen spec array */
    ULONG just;			/* Offset to justify */
    ULONG flen;			/* Fit length of string */
    BOOL clear;
    UBYTE bpen;
    UBYTE tpen;
    ULONG len;			/* Actual length of string */
    BOOL draw;
    BOOL high;
    UWORD adj;
    LONG move;
    BOOL ceob = TRUE;
    WORD cby;

    if ((list == (struct List *) ~0) || (list == NULL))
    {
	return (1);
    }

    /* Set the font */
    font = msg->imp_DrInfo->dri_Font;
    SetFont (rp, font);
    SetDrMd (rp, JAM2);

    /* Let's be sure that we were passed a DrawInfo */
    pens = (msg->imp_DrInfo) ? msg->imp_DrInfo->dri_Pens : NULL;
    backpen = pens[BACKGROUNDPEN];

    /* Get Image.Left/Top/Width/Height */
    box = *IM_BOX (IM (o));
    box.Left += msg->imp_Offset.X;
    box.Top += msg->imp_Offset.Y;
    box.Width = msg->imp_Dimensions.Width;
    box.Height = msg->imp_Dimensions.Height;

    /* Get the size to increment by */
    adj = (UWORD) si->si_UnitHeight;
    if (adj == 0)
    {
	adj = font->tf_YSize + 1;
    }

    /* Calculate the rectangle */
    tx = box.Left;
    ty = box.Top;
    bx = box.Left + box.Width - 1;
    by = box.Top + box.Height - 1;
    cby = box.Top + ((box.Height / si->si_UnitHeight) * si->si_UnitHeight) - 1;

    /* Set the beginning */
    etx = tx;
    cx = tx + 2;
    ebx = etx + box.Width - 1;
    cy = ty + font->tf_Baseline + 1;
    ety = ty;
    eby = ety + font->tf_YSize;

    /* Only draw the whole thing if needed */
    if (msg->imp_Mode == IDSM_UPDATE)
    {
	move = si->si_TopVert - lod->lod_OTop;
	if (!(ABS(move) >= (visible * 60 / 100)))
	{
	    ScrollRaster (rp, 0, (adj * move), tx, ty, bx, cby);
	    if (move > 0)
	    {
		cy += (adj * (visible - move));
		ety += (adj * (visible - move));
		eby += (adj * (visible - move));
		top += (visible - move);
		visible = move;
	    }
	    else if (move < 0)
	    {
		visible = -(move);
		ceob = FALSE;
	    }
	}
    }

    /* Let's start at the very beginning... */
    node = list->lh_Head;

    /* Make sure there are entries in the list */
    if (list && (list->lh_TailPred != (struct Node *) list))
    {
	/* Continue while there are still nodes to display */
	while ((nxtnode = node->ln_Succ) && (display < visible))
	{
	    /* Can we display yet? */
	    if (count >= top)
	    {
		/* Clear them */
		draw = high = clear = FALSE;

		/* Are we doing a complete redraw or scroll? */
		if (msg->imp_Mode != IDSM_TOGGLE)
		{
		    draw = TRUE;

		    /* Are we a MultiSelect list view? */
		    if ((si->si_Flags & LVSF_MULTISELECT) &&
			((node->ln_Type & LNF_SELECTED) ||
			(node->ln_Type & LNF_TEMPORARY)))
		    {
			high = TRUE;
		    }
		}
		/* Toggle select an item */
		else
		{
		    /* If we're not highlighting, then clear it */
		    clear = TRUE;

		    if (si->si_Flags & LVSF_DRAGGING)
		    {
			if (node->ln_Type & LNF_REFRESH)
			{
			    draw = TRUE;
			}

			if ((node->ln_Type & LNF_SELECTED) ||
			    (node->ln_Type & LNF_TEMPORARY))
			{
			    high = TRUE;
			}
		    }
		    else if (count == msg->imp_Anchor.Y)
		    {
			/* Downpress? */
			draw = TRUE;
			if ((msg->imp_State == IDS_SELECTED) &&
			    (msg->imp_Over.Y == msg->imp_Anchor.Y))
			{
			    high = TRUE;
			}
		    }
		    else if (count == msg->imp_Over.Y)
		    {
			if (si->si_Flags & LVSF_MULTISELECT)
			{
			    draw = high = TRUE;
			    if (!(node->ln_Type & LNF_SELECTED))
			    {
				high = FALSE;
			    }
			}
		    }
		}

		if (draw)
		{
		    tpen = pens[TEXTPEN];
		    bpen = backpen;
		    if (high)
		    {
			bpen = pens[FILLPEN];
			tpen = pens[FILLTEXTPEN];
		    }

		    if (clear || high)
		    {
			SetAPen (rp, bpen);
			RectFill (rp, etx, ety, ebx, eby);
		    }

		    text = GetField (cl, o, node);
		    flen = len = strlen (text);

		    /* Justify the text */
		    just = 0;
		    if (lod->lod_Flags & AOLVF_RIGHT)
		    {
			flen = TextFit (rp, &text[(len-1)], len, &lod->lod_TBuf, NULL, -1, ebx - etx - 1, rp->TxHeight);
			len -= flen;
			while (len--)
			    *text++;

			/* How much to adjust to get it to right justify */
			just = box.Width - TextLength (rp, text, flen) - 4;

			/* Clear in front of the line */
			if (tx < (cx + just))
			{
			    SetAPen (rp, bpen);
			    RectFill (rp, tx, ety, cx + just, eby);
			}
		    }
		    else
			flen = TextFit (rp, text, len, &lod->lod_TBuf, NULL, 1, ebx - etx - 1, rp->TxHeight);

		    /* Position and display the text */
		    SetAPen (rp, tpen);
		    SetBPen (rp, bpen);
		    Move (rp, cx + just, cy);
		    Text (rp, text, flen);

		    /* Set foreground to background so that we can RectFill */
		    SetAPen (rp, bpen);

		    /* See if we should clear beyond the line */
		    if (rp->cp_x < ebx)
		    {
			/* Clear behind the line */
			RectFill (rp, rp->cp_x, ety, ebx, eby);
		    }

		    /* Clear the little line above the text */
		    RectFill (rp, tx, ety, bx, ety);

		    /* Clear the little line on the left of the text */
		    RectFill (rp, tx, ety, (cx - 1), (ety + adj - 1));
		}

		/* Next line */
		cy += adj;
		ety += adj;
		eby += adj;

		/* Increment the number displayed */
		display++;
	    }

	    /* Go on to the next node */
	    node = nxtnode;
	    count++;
	}
    }

    /* Remember the last top line */
    lod->lod_OTop = si->si_TopVert;

    /* Do we have stuff to clear from the bottom? */
    if ((msg->imp_Mode != IDSM_TOGGLE) && ceob &&
	((cy = ((cy - adj) + (font->tf_YSize - font->tf_Baseline))) < by))
    {
	/* Clear to End of Box */
	SetAPen (rp, pens[BACKGROUNDPEN]);
	RectFill (rp, tx, cy, bx, by);
    }
    return (1L);
}
