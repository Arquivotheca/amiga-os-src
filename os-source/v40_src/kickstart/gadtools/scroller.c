/*** scroller.c ***********************************************************
*
*   scroller.c	- Scroll Bar/Arrows routines
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*
*   $Id: scroller.c,v 39.13 93/05/06 17:05:39 vertex Exp $
*
**************************************************************************/

/*------------------------------------------------------------------------*/

#include "gtinclude.h"

/*------------------------------------------------------------------------*/

/* Function Prototypes: */

/* Internal: */
struct ExtGadget *CreateScrollerA (struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
BOOL HandleScroller (struct ExtGadget *gad, struct IntuiMessage *imsg);
void SetScrollerAttrsA (struct ExtGadget *gad, struct Window *win,
    struct TagItem *taglist);

static void FindScrollerValues (UWORD total, UWORD displayable, UWORD top,
    UWORD *body, UWORD *pot);
static UWORD FindScrollerTop (UWORD total, UWORD displayable, UWORD pot);


static struct ExtGadget *createArrowButtonA ( struct ExtGadget *gad, struct NewGadget *ng,
    struct ExtGadget *scgad, LONG direction, struct TagItem *taglist );


/*****************************************************************************/


/* CreateScrollerA() */

struct ExtGadget *CreateScrollerA(struct ExtGadget *gad, struct NewGadget *ng,
                                  struct TagItem *taglist)
{
struct NewGadget  mod_ng, arrow_ng;
UWORD             arrowsize;
struct ExtGadget *scgad, *preSC;
BOOL              vertical;

extern struct TagItem to_prop_map[];

    /* We must create the dummy-gadget first, but we would like to
     * use it as the return value, so later we move it to last
     * in this piece of gadget-list.  We have to save the value of
     * gad before the scroller was allocated:
     */
    preSC = gad;

    /* Create the dummy gadget with the instance data: */
    mod_ng = *ng;
    if (!(scgad = CreateGenericBase(gad, ng, SCROLLER_IDATA_SIZE,taglist)))
    	return(NULL);

    scgad->Flags |= GADGIMAGE|GADGHNONE;
    SGAD(scgad)->sg_SetAttrs  = SetScrollerAttrsA;
    SGAD(scgad)->sg_Flags     = SG_EXTRAFREE_DISPOSE;
    SGAD(scgad)->sg_GetTable  = Scroller_GetTable;
    SCID(scgad)->scid_Visible = 2;
    SCID(scgad)->scid_Flags   = PackBoolTags(NULL, taglist, to_prop_map);

    placeGadgetText(scgad,ng->ng_Flags,PLACETEXT_LEFT,NULL);

    vertical = FALSE;
    if (getTagData(PGA_FREEDOM,0,taglist) == LORIENT_VERT)
	vertical = TRUE;

    arrowsize = getGTTagData(GTSC_Arrows, 0, taglist);

    /* Make changes to NewGadget */
    mod_ng.ng_GadgetText = NULL;
    arrow_ng = mod_ng;

    /* Calculate dimension/position information based on
     * existence of arrows.
     */
    if (arrowsize)
    {
	if ( vertical )
	{
	    /* Remove height of arrows from scroller height */
	    mod_ng.ng_Height -= 2*arrowsize;
	    /* Move the arrow to past the bottom of the whole
	     * scroller package, and then back up by 2*arrowsize
	     */
	    arrow_ng.ng_TopEdge += arrow_ng.ng_Height - 2*arrowsize;
	    arrow_ng.ng_Height = arrowsize;
	}
	else
	{
	    mod_ng.ng_Width -= 2*arrowsize;
	    arrow_ng.ng_LeftEdge += arrow_ng.ng_Width - 2*arrowsize;
	    arrow_ng.ng_Width = arrowsize;
	}
    }

    /* Note well: Imagery is hanging off the underlay gadget
     * (scgad) though its size depends on the scroller gadget,
     * since above we adjusted that for arrows.
     */
    if (!(scgad->GadgetRender = getBevelImage(0,0,mod_ng.ng_Width,mod_ng.ng_Height,FRAME_BUTTON)))
	return(NULL);

    /* Recall that the scroller actually has to fit nicely inside
     * the border.   Here's that offset:
     */
    mod_ng.ng_LeftEdge += LEFTTRIM;
    mod_ng.ng_TopEdge  += TOPTRIM;
    mod_ng.ng_Width    -= LRTRIM;
    mod_ng.ng_Height   -= TBTRIM;

    if (!(SCID(scgad)->scid_Prop = gad = CreateGenericBase(scgad,&mod_ng,0,taglist)))
        return(NULL);

    MP(("CSC: gadget at $%lx\n", prop));
    MP(("CSC: Filling out gadget structure\n"));

    /* Install required blank image structure: */
    gad->GadgetRender  = (APTR)&SCID(scgad)->scid_PropImage;
    gad->Flags         = GADGHNONE | GADGIMAGE;  /* clears away GFLG_EXTENDED */
    gad->Activation    = GADGIMMEDIATE | RELVERIFY | FOLLOWMOUSE;
    gad->GadgetType   |= PROPGADGET;
    gad->SpecialInfo   = (APTR)&SCID(scgad)->scid_PropInfo;

    /* Initialize handlers in the SpecialGadget: */
    SGAD(gad)->sg_Parent       = scgad;
    SGAD(gad)->sg_EventHandler = HandleScroller;
    SGAD(gad)->sg_Flags        = SG_MOUSEMOVE;

    if (vertical)
    {
        MP(("CSC: Initialing as vertical scroller\n"));
        SCID(scgad)->scid_PropInfo.Flags = AUTOKNOB | PROPBORDERLESS | FREEVERT;
        SCID(scgad)->scid_Pot            = &SCID(scgad)->scid_PropInfo.VertPot;
        SCID(scgad)->scid_Body           = &SCID(scgad)->scid_PropInfo.VertBody;
    }
    else
    {
        MP(("CSC: Initialing as horizontal scroller\n"));
        SCID(scgad)->scid_PropInfo.Flags = AUTOKNOB | PROPBORDERLESS | FREEHORIZ;
        SCID(scgad)->scid_Pot            = &SCID(scgad)->scid_PropInfo.HorizPot;
        SCID(scgad)->scid_Body           = &SCID(scgad)->scid_PropInfo.HorizBody;
    }

    if (arrowsize)
    {
	LONG direction;

	SCID(scgad)->scid_Flags |= SC_ARROWS;
	/* Yep, we're getting arrows! */
	arrow_ng.ng_GadgetText = NULL;

	direction = ARROW_UP;
	if ( !vertical )
	{
	    direction = ARROW_LEFT;
	}

	SCID(scgad)->scid_Up = gad = createArrowButtonA( gad, &arrow_ng,
	    scgad, direction, taglist );
	/* Wherein ARROW_UP becomes ARROW_DOWN or
	 * ARROW_LEFT becomes ARROW_RIGHT
	 */
	direction++;
	if ( vertical )
	{
	    arrow_ng.ng_TopEdge += arrowsize;
	}
	else
	{
	    arrow_ng.ng_LeftEdge += arrowsize;
	}
	SCID(scgad)->scid_Down = gad = createArrowButtonA(gad, &arrow_ng,
	                                                  scgad, direction, taglist );

        if (!gad)
            return(NULL);
    }

    SetScrollerAttrsA(scgad,NULL,taglist);

    /* Move the dummy-gadget to the end of this list, so that
     * it is the one that is returned:
     */
    preSC->NextGadget = scgad->NextGadget;
    gad->NextGadget = scgad;
    scgad->NextGadget = NULL;
    MP(("CSC: Done.\n"));
    return(scgad);
}


/*------------------------------------------------------------------------*/

VOID SetScrollerAttrsA(struct ExtGadget *gad, struct Window *win,
                       struct TagItem *taglist)
{
struct TagItem *tag;

    SCID(gad)->scid_Total   = getGTTagData(GTSC_Total, SCID(gad)->scid_Total, taglist);
    SCID(gad)->scid_Visible = getGTTagData(GTSC_Visible, SCID(gad)->scid_Visible, taglist);
    SCID(gad)->scid_Top     = getGTTagData(GTSC_Top, SCID(gad)->scid_Top, taglist);

    DP(("SSAA:  Total %ld, Visible %ld, Top %ld\n", (LONG)SCID(gad)->scid_Total,
	(LONG)SCID(gad)->scid_Visible, (LONG)SCID(gad)->scid_Top));
    FindScrollerValues(SCID(gad)->scid_Total, SCID(gad)->scid_Visible, SCID(gad)->scid_Top,
	               SCID(gad)->scid_Body, SCID(gad)->scid_Pot);
    DP(("SSAA:  Body %ld, Pot %ld\n", (LONG)(*SCID(gad)->scid_Body),
	(LONG)(*SCID(gad)->scid_Pot)));

    if (win)
    {
        DP(("SSAA:  Calling NMP on gad $%lx, win $%lx, Flags $%lx\n",
            SCID(gad)->scid_Prop, win, (LONG)pi->Flags));
        DP(("SSAA:  HPot %ld VPot %ld HBody %ld VBody %ld\n",
            (LONG)pi->HorizPot, (LONG)pi->VertPot,
            (LONG)pi->HorizBody, (LONG)pi->VertBody));

        NewModifyProp(SCID(gad)->scid_Prop,win,NULL,
                      SCID(gad)->scid_PropInfo.Flags,
                      SCID(gad)->scid_PropInfo.HorizPot,
                      SCID(gad)->scid_PropInfo.VertPot,
                      SCID(gad)->scid_PropInfo.HorizBody,
                      SCID(gad)->scid_PropInfo.VertBody, 1);
    }

    /* Test for GA_Disabled, set GADGDISABLED and refresh accordingly */
    if (tag = TagAbleGadget(SCID(gad)->scid_Prop,win,taglist))
    {
        if (SCID(gad)->scid_Flags & SC_ARROWS)
        {
            TagAbleGadget(SCID(gad)->scid_Up,win,taglist);
            TagAbleGadget(SCID(gad)->scid_Down,win,taglist);
        }
    }
}


/*------------------------------------------------------------------------*/


/*/ HandleScroller()
 *
 * Function to handle IntuiMessages that relate to scrollers.
 *
 * Created:  30-Aug-89, Peter Cherna
 * Modified: 16-Mar-90, Peter Cherna
 *
 */

BOOL HandleScroller( struct ExtGadget *gad, struct IntuiMessage *imsg )
{
    struct ExtGadget *scgad = SGAD(gad)->sg_Parent;
    WORD newtop = SCID(scgad)->scid_Top;
    BOOL hearupdown = FALSE;
    WORD hidden;

    MP(("HS: Gadget at $%lx\n", gad));

    if (gad == SCID(scgad)->scid_Prop)
    {
	/* User hit the prop gadget, so find a new value of top based on
	 * the new value of Pot:
	 */
	newtop = FindScrollerTop(SCID(scgad)->scid_Total, SCID(scgad)->scid_Visible,
	    *SCID(scgad)->scid_Pot);
    }
    else
    {
	/* User hit one of the buttons */
	if (imsg->Class != IDCMP_GADGETUP)
	{
	    /* On the upstroke of a button, all we'd ever do is
	     * notify the app, never scroll.
	     */

	    if (imsg->Class == IDCMP_GADGETDOWN)
	    {
		/* Reset tick counter: */
		MP(("HSC:  Got IDCMP_GADGETDOWN, starting TickCount\n"));
		SCID(scgad)->scid_TickCount = 0;
	    }
	    else /* (imsg->Class == IDCMP_INTUITICKS) */
	    {
		MP(("HSC:  New TickCount %ld, ", SCID(scgad)->scid_TickCount+1));
		if (++SCID(scgad)->scid_TickCount <= ARROW_SKIPTICKS)
		{
		    /* Too early for the first repeat: */
		    MP(("too early.\n"));
		    return(FALSE);
		}
		else if (!(gad->Flags & SELECTED))
		{
		    /* The user rolled the mouse off the gadget */
		    MP(("Mouse no longer over gadget\n"));
		    return(FALSE);
		}
		MP(("going to repeat.\n"));
	    }

	    /* User hit up or down gadget: */
	    if (gad == SCID(scgad)->scid_Up)
	    {
		/* User hit up gadget: */
		if (--newtop < 0)
		{
		    newtop = 0;
		}
	    }
	    else /* if (gad == SCID(scgad)->scid_Down) */
	    {
		/* User hit down gadget: */
		hidden = max(SCID(scgad)->scid_Total - SCID(scgad)->scid_Visible, 0);
		/* If newtop is so great that the remainder of the list won't even
		 * fill the displayable area, reduce it:
		 */
		if (++newtop > hidden)
		   newtop = hidden;
	    }
	    DP(("HSC:  Vis %ld, newtop %ld, oldtop: %ld\n",
		(LONG) SCID(scgad)->scid_Visible, (LONG)newtop, (LONG)SCID(scgad)->scid_Top));
	    /* Figure out new scroller values, and refresh: */
	    FindScrollerValues(SCID(scgad)->scid_Total, SCID(scgad)->scid_Visible, newtop,
		SCID(scgad)->scid_Body, SCID(scgad)->scid_Pot);
	    DP(("HSC:  Body %ld, Pot %ld\n", (LONG)(*SCID(scgad)->scid_Body),
		(LONG)(*SCID(scgad)->scid_Pot)));
	    DP(("HSC:  HPot %ld VPot %ld HBody %ld VBody %ld\n",
		(LONG)pi->HorizPot, (LONG)pi->VertPot,
		(LONG)pi->HorizBody, (LONG)pi->VertBody));
	    NewModifyProp(SCID(scgad)->scid_Prop,imsg->IDCMPWindow,NULL,
                          SCID(scgad)->scid_PropInfo.Flags,
                          SCID(scgad)->scid_PropInfo.HorizPot,
                          SCID(scgad)->scid_PropInfo.VertPot,
                          SCID(scgad)->scid_PropInfo.HorizBody,
                          SCID(scgad)->scid_PropInfo.VertBody, 1);
	}
    }

    if (imsg->Class == GADGETDOWN)
    {
	MP(("HS:  ScrollGadget GADGETDOWN\n"));
	if (SCID(scgad)->scid_Flags & GTPROP_GADGETDOWN)
	{
	    /* The client wants to hear this always: */
	    hearupdown = TRUE;
	}
    }
    else if (imsg->Class == IDCMP_GADGETUP)
    {
	MP(("HS:  ScrollGadget IDCMP_GADGETUP\n"));
	if (SCID(scgad)->scid_Flags & GTPROP_GADGETUP)
	{
	    /* The client wants to hear this always: */
	    hearupdown = TRUE;
	}
    }
    D(else
    {
	MP(("HS: ScrollGadget IDCMP_MOUSEMOVE\n"));
    });

    /* If we cause a message, we want the code field equal to the
     * new level, with the IAddress of the scroller gadget dummy
     */
    imsg->Code = newtop;
    imsg->IAddress = (APTR) scgad;

    /* Only react if the value of top actually changed: */
    MP(("HS:  Old top %ld, new top %ld\n", ((LONG)SCID(scgad)->scid_Top),
	((LONG)newtop)));

    if (newtop != SCID(scgad)->scid_Top)
    {
	SCID(scgad)->scid_Top = newtop;
	MP(("HS:  New Top Set %ld\n", ((LONG)SCID(scgad)->scid_Top)));

	/* This will end up being a proper interconnection to
	 * the ListView, but for now we call SetListViewAttrs()
	 * if this Scroller is part of one:
	 */
	if (SCID(scgad)->scid_ListView)
	{
	    MP(("HS:  Calling ISLVA(): win $%lx, lv $%lx, new top %ld\n",
		imsg->IDCMPWindow, SCID(scgad)->scid_ListView, ((ULONG)newtop)));
	    SetListViewTop(SCID(scgad)->scid_ListView,imsg->IDCMPWindow,newtop);
	    return(FALSE);
	}
	else
	{
	    /* If this is a IDCMP_GADGETUP or IDCMP_GADGETDOWN and our client doesn't
	     * want to hear this type, we satisfy him by converting it
	     * to a IDCMP_MOUSEMOVE, since he MUST hear of every event in which
	     * the level changed:
	     */
	    if (!hearupdown)
		imsg->Class = IDCMP_MOUSEMOVE;
	    MP(("HS: Level Changed\n"));
	    return(TRUE);
	}
    }

    /* The level did not change, so only let the message reach
     * the client if it was a IDCMP_GADGETUP or IDCMP_GADGETDOWN and the client
     * was interested in that - this condition is reflected by
     * the value of hearupdown:
     */
    MP((hearupdown ? "HS: Sending message even though level unchanged\n" :
	"HS: Message dropped since level unchanged.\n"));
    return(hearupdown);
}


/*------------------------------------------------------------------------*/

/*/ FindScrollerValues()
 *
 * Function to calculate the Body and Pot values of a proportional gadget
 * given the three values total, displayable, and top, representing the
 * total number of items in a list, the number of items displayable at one
 * time, and the top item to be displayed.  For example, a file requester
 * may be able to display 10 entries at a time.  The directory has 20
 * entries in it, and the top one displayed is number 3 (the fourth one,
 * counting from zero), then total = 20, displayable = 10, and top = 3.
 *
 */

static void FindScrollerValues( UWORD total, UWORD displayable, UWORD top,
    UWORD *body, UWORD *pot )
{
    UWORD hidden;

    /* Find the number of unseen lines: */
    hidden = max(total - displayable, 0);

    /* If top is so great that the remainder of the list won't even
     * fill the displayable area, reduce top:
     */

    if (top > hidden)
       top = hidden;

    /* body is the relative size of the proportional gadget's rbody.
     * Its size in the container represents the fraction of the total
     * that is in view.  If there are no lines hidden, then body
     * should be full-size (MAXBODY).  Otherwise, body should be the
     * fraction of (the number of displayed lines - 1) over
     * (the total number of lines - 1).  The "-1" is so that when the
     * user scrolls by clicking in the container of the scroll gadget,
     * then there is a one line overlap between the two views.
     */

    (*body) = (hidden > 0) ?
	(UWORD) (((ULONG) (displayable - 1) * MAXBODY) / (total - 1)) : MAXBODY;

    /* pot is the position of the proportional gadget body, with zero
     * meaning that the scroll gadget is all the way up (or left),
     * and full (MAXPOT) meaning that the scroll gadget is all the way
     * down (or right).  If we can see all the lines, pot should be zero.
     * Otherwise, pot is the top displayed line divided by the number of
     * unseen lines.
     */

    (*pot) = (hidden > 0) ?
	(UWORD) (((ULONG) top * MAXPOT) / hidden) : 0;
}


/*------------------------------------------------------------------------*/

/*/ FindScrollerTop()
 *
 * Function to calculate the top line that is displayed in a proportional
 * gadget, given the total number of items in the list and the number
 * displayable, as well as the HorizPot or VertPot value.
 *
 */

static UWORD FindScrollerTop( UWORD total, UWORD displayable, UWORD pot )
{
    UWORD top, hidden;

    /* Find the number of unseen lines: */
    hidden = max(total - displayable, 0);

    /* pot can be thought of as the fraction of the hidden lines that
     * are before the displayed part of the list, in other words a
     * pot of zero means all hidden lines are after the displayed
     * part of the list (i.e. top = 0), and a pot of MAXPOT means all
     * the hidden lines are ahead of the displayed part
     * (i.e. top = hidden).
     */
    top = (((ULONG) hidden * pot) + (MAXPOT/2)) >> 16;

    /* Once you get back the new value of top, only redraw your list
     * if top changed from its previous value.  The proportional gadget
     * may not have moved far enough to change the value of top.
     */

    return(top);
}

/*------------------------------------------------------------------------*/

/*/ createArrowButtonA()
 *
 * Makes an arrow button (up, down, left, or right).
 *
 */

static struct ExtGadget *createArrowButtonA(struct ExtGadget *gad,
                                            struct NewGadget *ng,
                                            struct ExtGadget *scgad,
                                            LONG direction,
                                            struct TagItem *taglist)
{
struct ArrowBorder *arrowborder;

    if (gad = (struct ExtGadget *)CreateGadget(BUTTON_KIND, gad, ng,
                                                 GT_ExtraSize, sizeof(struct ArrowBorder),
                                                 GA_Immediate, TRUE,
                                                 TAG_MORE,     taglist))
    {
	/* Code is smaller if intermediate values are longs */
	struct Rect32 inner;
        WORD *points;

        gad->MoreFlags             &= ~(GMORE_GADGETHELP);
	SGAD(gad)->sg_EventHandler  = HandleScroller;
	SGAD(gad)->sg_Parent        = scgad;
	SGAD(gad)->sg_Flags        |= SG_INTUITICKS;

	/* Now let's go fill out the arrow's border structure */

	arrowborder = MEMORY_FOLLOWING(SGAD(gad));

	inner.MinX = LEFTTRIM;
	inner.MinY = TOPTRIM;
	inner.MaxX = gad->Width - 2 - LEFTTRIM;
	inner.MaxY = gad->Height - 1 - TOPTRIM;

	arrowborder->Border1.BackPen = DESIGNTEXT;
	/* JAM1 == 0, so we don't need to set it: */
    /*
     *  arrowborder->Border1.DrawMode = JAM1;
     */
	arrowborder->Border1.Count = 4;
	points = arrowborder->Border1.XY = arrowborder->Points;
	arrowborder->Border2 = arrowborder->Border1;
	arrowborder->Border2.LeftEdge = 1;
	arrowborder->Border1.NextBorder = &arrowborder->Border2;

	/* GadgetRender is an Image whose IA_Data is a border */
	SetAttrs(gad->GadgetRender, IA_Data, arrowborder,
                                    TAG_DONE );
#if 0
	/* GadgetRender is an Image whose ImageData is really a pointer
	 * to a struct Border
	 */
	arrowborder->Border2.NextBorder = (struct Border *)
	    ((struct Image *)gad->GadgetRender)->ImageData;
	((struct Image *)gad->GadgetRender)->ImageData = (APTR) arrowborder;
#endif

	switch (direction)
	{
	    /* Code is smaller if intermediate values are longs */
	    LONG sum, temp;

	    case ARROW_DOWN:
		/* Reverse MinY and MaxY, and pretend it's an ARROW_UP: */
		temp = inner.MinY;
		inner.MinY = inner.MaxY;
		inner.MaxY = temp;
		/* and fall through to the next case (no break) */
	    case ARROW_UP:
		*points++ = inner.MinX;
		*points++ = inner.MaxY;

		sum = ( inner.MinX + inner.MaxX );
		*points++ = sum / 2;
		*points++ = inner.MinY;

		*points++ = ( sum + 1 ) / 2;
		*points++ = inner.MinY;

		*points++ = inner.MaxX;
    /** NOTE WELL:  The last post-increment is deliberately missing (optimization) **/
		*points = inner.MaxY;

		break;

	    case ARROW_RIGHT:
		/* Reverse MinX and MaxX, and pretend it's an ARROW_LEFT: */
		temp = inner.MinX;
		inner.MinX = inner.MaxX;
		inner.MaxX = temp;
		/* and fall through to the next case (no break) */
	    case ARROW_LEFT:
		*points++ = inner.MaxX;
		*points++ = inner.MinY;

		sum = inner.MinY + inner.MaxY;
		*points++ = inner.MinX;
		*points++ = sum / 2;

		*points++ = inner.MinX;
		*points++ = ( sum + 1 ) / 2;

		*points++ = inner.MaxX;
    /** NOTE WELL:  The last post-increment is deliberately missing (optimization) **/
		*points = inner.MaxY;

		break;
	}
    }
    return( gad );
}
