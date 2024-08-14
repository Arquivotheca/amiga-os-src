/*** nway.c ***************************************************************
*
*   nway.c	- Cycle gadget routines
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*
*   $Id: nway.c,v 39.8 92/05/29 15:29:12 vertex Exp $
*
**************************************************************************/

/*------------------------------------------------------------------------*/

#include "gtinclude.h"

/*------------------------------------------------------------------------*/

/* Function Prototypes: */

/* Internal: */
struct ExtGadget *CreateCycleA (struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
static void UpdateCycle(struct ExtGadget *gad, struct Window *win);
BOOL HandleCycle (struct ExtGadget *gad, struct IntuiMessage *imsg);
void SetCycleAttrsA (struct ExtGadget *gad, struct Window *win,
    struct TagItem *taglist);

/*------------------------------------------------------------------------*/

/* CreateCycleA() */

struct ExtGadget *CreateCycleA(struct ExtGadget *gad, struct NewGadget *ng,
                            struct TagItem *taglist)
{
struct Border *border;

    /* Create a generic gadget, with an extra space for a BevelBox
     * and the little divider border (two borders which have two
     * points each, but which can be shared):
     */
    if (gad = CreateGenericBase(gad,ng,CYCLE_IDATA_SIZE,taglist))
    {
        /* A boolean RELVERIFY gadget with complement-highlighting: */

        gad->Flags      |= GADGIMAGE|GADGHIMAGE;
        gad->Activation  = RELVERIFY;
        gad->GadgetType |= BOOLGADGET;

        SGAD(gad)->sg_EventHandler = HandleCycle;
        SGAD(gad)->sg_SetAttrs     = SetCycleAttrsA;
        SGAD(gad)->sg_GetTable     = Cycle_GetTable;
        SGAD(gad)->sg_Flags        = SG_EXTRAFREE_DISPOSE;

        placeGadgetText( gad, ng->ng_Flags, PLACETEXT_LEFT, NULL );

        /* GTButtonIClass will render this as TEXTPEN or FILLTEXTPEN.
         * The text itself will be set and positioned when UpdateCycle() is
         * called.
         */
        CYID(gad)->cyid_IText.DrawMode = JAM1;
        CYID(gad)->cyid_IText.ITextFont = ng->ng_TextAttr;

        border = MakeCycleBorder(&CYID(gad)->cyid_CycleBorder, gad->Height,
            VI(ng->ng_VisualInfo));

        /* Connect DarkStroke following cycle glyph, and point
         * 'border' at it.
         */
        border = border->NextBorder = &CYID(gad)->cyid_DarkStroke;

        border->XY = (UWORD *) CYID(gad)->cyid_StrokePoints;
        border->LeftEdge = CYCLEGLYPHWIDTH;
        border->TopEdge = 2;
        /* GTButtonIClass will render this as SHADOWPEN (SHINEPEN when selected) */
        border->BackPen = DESIGNSHADOW;
    /*
     *  border->DrawMode = JAM1;
     */
        border->Count = 2;
        border->XY[3] = gad->Height - 5;

        /* Connect LightStroke after DarkStroke, and point
         * 'border' at it
         */
        border = border->NextBorder = &CYID(gad)->cyid_LightStroke;
        /* LightStroke is a lot like DarkStroke, so start by copying it */
        *border = CYID(gad)->cyid_DarkStroke;
        border->LeftEdge++;
        /* GTButtonIClass will render this as SHINEPEN (SHADOWPEN when selected) */
        border->BackPen = DESIGNSHINE;
        border->NextBorder = NULL;

        if (!(gad->SelectRender = gad->GadgetRender =
                               NewObject(GadToolsBase->gtb_GTButtonIClass, NULL,
                                         IA_Width, gad->Width,
                                         IA_Height, gad->Height,
                                         IA_Data, &CYID(gad)->cyid_CycleBorder,
                                         IA_FrameType, FRAME_BUTTON,
                                         TAG_DONE)))
        {
            return(NULL);
        }

        SetCycleAttrsA(gad,NULL,taglist);

        if ( !CYID(gad)->cyid_Labels )
        {
            return( NULL );
        }
    }

    return(gad);
}


/*------------------------------------------------------------------------*/

/*/ HandleCycle()
 *
 * Function to handle messages pertaining to a Cycle gadget.
 *
 *  Notes:
 *
 *  When we go with the new custom gadget stuff, we should make
 *  the highlighted image contain the next label in sequence (but
 *  highlighted), and when the user releases the mouse, it stays,
 *  or reverts if the user rolls off.
 *
 * Created:  13-Oct-89, Peter Cherna
 * Modified: 17-Oct-89, Peter Cherna
 *
 */

BOOL HandleCycle( struct ExtGadget *gad, struct IntuiMessage *imsg )
{
    if (imsg->Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
    {
	/* Cycle backwards: */
	if (CYID(gad)->cyid_Active-- == 0)
	{
	    CYID(gad)->cyid_Active = CYID(gad)->cyid_MaxLabel;
	}
    }
    else
    {
	/* Cycle forward, unless we were at the last: */
	if (CYID(gad)->cyid_Active++ == CYID(gad)->cyid_MaxLabel)
	{
	    CYID(gad)->cyid_Active = 0;
	}
    }
    UpdateCycle(gad,imsg->IDCMPWindow);

    imsg->Code = CYID(gad)->cyid_Active;

    return(TRUE);
}


/*------------------------------------------------------------------------*/

void SetCycleAttrsA( struct ExtGadget *gad, struct Window *win,
    struct TagItem *taglist )
{
    UWORD newactive = getGTTagData(GTCY_Active, CYID(gad)->cyid_Active, taglist);
    STRPTR *newlabels = (STRPTR *)getGTTagData(GTCY_Labels, 0, taglist);

    DP(("SCYA: Active is %ld (could be from before)\n", newactive ));
    DP(("SCYA: Labels at $%lx\n", newlabels ));
    /* At create-time, we require newlabels, and we historically
     * tested that the first label was itself non-null.
     * If either of those happen at create-time, we'll return with
     * cyid_Labels unaltered (i.e. NULL).
     * At set time, we take newlabels==NULL to mean unchanged.
     */
    if ( (newlabels) && ( newlabels[0] ) )
    {
	DP(("SCYA: Adopting labels\n"));
	CYID(gad)->cyid_Labels = newlabels;
	/* Find the highest-numbered label: */
	for (CYID(gad)->cyid_MaxLabel = 0;
	    CYID(gad)->cyid_Labels[CYID(gad)->cyid_MaxLabel+1];
	    CYID(gad)->cyid_MaxLabel++)
	    ;
	DP(("SCYA: MaxLabel is %ld\n", CYID(gad)->cyid_MaxLabel));
    }
    /* Idiot-Proofing:  don't let active surpass the last non-null
     * label:
     */
    CYID(gad)->cyid_Active = min(newactive, CYID(gad)->cyid_MaxLabel);
    DP(("SCYA: Adjusted Active is %ld\n", CYID(gad)->cyid_Active ));

    UpdateCycle(gad,win);

    /* Test for GA_Disabled, set GADGDISABLED and refresh accordingly */
    TagAbleGadget(gad, win, taglist);
}


/*------------------------------------------------------------------------*/

/*/ UpdateCycle()
 *
 * Function to change the label on the Cycle gadget.
 *
 * Created:  13-Oct-89, Peter Cherna
 * Modified:  4-Apr-90, Peter Cherna
 *
 */

static void UpdateCycle(struct ExtGadget *gad, struct Window *win)
{
struct Rectangle rect;
UWORD            pos;

    DP(("UCycle:  Welcome\n"));

    rect.MinX = CYCLEGLYPHWIDTH;
    rect.MinY = 0;
    rect.MaxX = gad->Width - 1;
    rect.MaxY = gad->Height - 1;

    pos = removeGadget(win,gad);

    DP(("UCycle:  Active is %ld\n", CYID(gad)->cyid_Active));
    CYID(gad)->cyid_IText.IText    = CYID(gad)->cyid_Labels[CYID(gad)->cyid_Active];
    CYID(gad)->cyid_IText.LeftEdge = 0;
    CYID(gad)->cyid_IText.TopEdge  = 0;

    DP(("UCycle:  Re-placing Label text '%s'\n", CYID(gad)->cyid_IText.IText));
    PlaceIntuiText(&CYID(gad)->cyid_IText, &rect, PLACETEXT_IN);

    DP(("UCycle:  Calling SetAttrs\n"));
    SetAttrs(gad->GadgetRender, MYIA_IText, &CYID(gad)->cyid_IText,
                                TAG_DONE);

    AddRefreshGadget(gad, win, pos);

    DP(("UCycle:  All done\n"));
}
