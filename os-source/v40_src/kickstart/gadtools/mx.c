/*** mx.c *****************************************************************
*
*   mx.c	- Mutually Exclusive gadget routines
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*
*   $Id: mx.c,v 39.12 92/10/16 18:29:32 vertex Exp $
*
**************************************************************************/

/*------------------------------------------------------------------------*/

#include "gtinclude.h"

/*------------------------------------------------------------------------*/

/* Function Prototypes: */

/* Internal: */
struct ExtGadget *CreateMXA (struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
void SetMXAttrsA (struct ExtGadget *gad, struct Window *win,
    struct TagItem *taglist);
BOOL HandleMX (struct ExtGadget *gad, struct IntuiMessage *imsg);

static BOOL NewActiveMX (struct ExtGadget *gad, struct Window *win);

/*------------------------------------------------------------------------*/

/* CreateMXA() */

struct ExtGadget *CreateMXA( struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist )
{
    struct NewGadget mod_ng;
    STRPTR *gadlabel;
    struct ExtGadget *preMX, *mxgad;
    UWORD interspacing;
    struct Image *mximage;
    STRPTR *labels = (STRPTR *)getGTTagData(GTMX_Labels, 0, taglist);
    struct TagItem *ti;
    struct Rectangle newrect, result, gadgetrect;

    DP(("CMX:  labels pointer at $%lx\n", labels));
    if (!labels)
    {
	DP(("CMX:  Failing on account of NULL labels\n"));
	return(NULL);
    }

    /* We've introduced an alternate tag similar to LAYOUTA_SPACING
     * that is font-sensitive.  GTMX_Spacing is added to the font-height,
     * whereas LAYOUTA_SPACING is added to the gadget image height.
     * Better it should be to the font height for people doing
     * font-sensitive layout.  The equivalent LAYOUTA_SPACING is
     * GTMX_Spacing plus font-height minus MX_HEIGHT.  This
     * spacing variable must default to one so that 8-point font
     * users get the same default as LAYOUTA_SPACING gave
     */

    interspacing = getGTTagData(GTMX_Spacing, 1, taglist) +
	ng->ng_TextAttr->ta_YSize;

    /* We give old users what they've always asked for: */
    if (ti = findTagItem(LAYOUTA_SPACING, taglist))
    {
	interspacing = ti->ti_Data + MX_HEIGHT;
    }

    /* We must create the dummy-gadget first, but we would like to
     * return it from CreateMX(), so later we move it to last
     * in this piece of gadget-list.  We have to save the value of
     * gad before mxgad was allocated:
     */
    preMX = gad;

    /* Create the dummy gadget with the instance data: */
    DP(("CMX:  about to create dummy\n"));
    if (mxgad = gad = CreateGenericBase(gad, ng, MX_IDATA_SIZE,taglist))
    {
        mxgad->Flags |= GADGHNONE;

        SGAD(mxgad)->sg_SetAttrs = SetMXAttrsA;
        SGAD(mxgad)->sg_GetTable = MX_GetTable;

        mod_ng              = *ng;
        mod_ng.ng_Flags    &= ~NG_HIGHLABEL;
        mod_ng.ng_GadgetID  = 0;

        if (!getGTTagData(GTMX_Scaled,FALSE,taglist))
        {
            mod_ng.ng_Width  = MX_WIDTH;
            mod_ng.ng_Height = MX_HEIGHT;
            /* Here we do an unusual thing.  For most gadget classes, the text
             * is contained in a box whose TopEdge corresponds to what
             * the caller calls ng.ng_TopEdge.  So for any given font size,
             * the font is a constant distance below this point.  Unfortunately,
             * for mx gadgets (and checkboxes), the ng_TopEdge represented the
             * TopEdge of the mx gadget, but the text is not a fixed
             * distance below this point, but rather vertically centered
             * with respect to the mx gadget height (a constant).  So the
             * bigger the font, the higher it would appear.
             *
             * Smart mx gadgets (GTMX_Scaled) allows radio buttons of arbitrary
             * height.  In these cases, the font is positioned correctly.
             *
             * In the meantime, we behave similar to that, except the mx gadget
             * is of fixed size, and IT is vertically-centered within such
             * a space.  Things are arranged so that for 8-point fonts, the
             * positions are exactly as before.  With the new scheme, the text
             * will appear one pixel lower than before for every two pixels
             * of font height above 8.  This makes font-sensitive layout by
             * a GadTools client possible.
             */

            /* Since we default to PLACETEXT_LEFT, we want this adjustment
             * when the place field is LEFT, RIGHT, or zero.  The most efficient
             * coding for that is to say <= PLACETEXT_RIGHT.  Works out because
             * we got lucky chosing #defines.  Not pretty, but small!
             */
            if ((ng->ng_Flags & PLACETEXT_MASK) <= PLACETEXT_RIGHT)
            {
                mod_ng.ng_TopEdge += (ng->ng_TextAttr->ta_YSize-7)/2;
            }
        }

        if (!(mximage = getSysImage(mod_ng.ng_VisualInfo,mod_ng.ng_Width, mod_ng.ng_Height, MXIMAGE)))
            return(NULL);

        DP(("CMX:  Boopsi MX Image at $%lx\n", mximage));
        DP(("CMX:  im->Width = %ld, im->Height = %ld\n",
            (LONG)mximage->Width, mximage->Height));

        gadgetrect.MinX = gadgetrect.MaxX = mxgad->LeftEdge;
        gadgetrect.MinY = gadgetrect.MaxY = mxgad->TopEdge;

        gadlabel = labels;
        while (*gadlabel)
        {
            DP(("CMX:  gadlabel $%lx, *gadlabel $%lx = '%s'\n", gadlabel,
                *gadlabel, *gadlabel));
            mod_ng.ng_GadgetText = *gadlabel++;
            /* Send taglist on down, so that GT_Underscore gets through */
            if (!(gad = (struct ExtGadget *)CreateGadgetA(GENERIC_KIND, gad, &mod_ng, taglist)))
                return(NULL);

            if (!MXID(mxgad)->mxid_FirstGadget)
                MXID(mxgad)->mxid_FirstGadget = gad;

            placeGadgetText(gad, ng->ng_Flags, PLACETEXT_LEFT, NULL );

            newrect.MinX = gad->BoundsLeftEdge;
            newrect.MinY = gad->BoundsTopEdge;
            newrect.MaxX = newrect.MinX + gad->BoundsWidth - 1;
            newrect.MaxY = newrect.MinY + gad->BoundsHeight - 1;

            CombineRects(&gadgetrect,&newrect,&result);
            gadgetrect = result;

            /* These gadgets have an image, and are highlighted by
             * an alternate image (of the same size and position):
             *
             * This also clears GFLG_EXTENDED, which disables GADGETHELP for
             * the individual gadgets
             */
            gad->Flags = GADGHIMAGE | GADGIMAGE;

            /* These gadgets are hit-select boolean gadgets,
             * with GADGIMMEDIATE
             */
            gad->Activation = GADGIMMEDIATE;
            gad->GadgetType |= BOOLGADGET;

            gad->GadgetRender = gad->SelectRender = (APTR) mximage;

            /* Identify the group the gadget belongs to, and set up
             * the handler routine:
             */
            SGAD(gad)->sg_Parent = mxgad;
            SGAD(gad)->sg_EventHandler = HandleMX;

            /* Move down for the next gadget: */
            mod_ng.ng_TopEdge += interspacing;
            mod_ng.ng_GadgetID++;

            MXID(mxgad)->mxid_NumGadgets++;
        }

        mxgad->LeftEdge = gadgetrect.MinX;
        mxgad->TopEdge  = gadgetrect.MinY;
        mxgad->Width    = gadgetrect.MaxX - gadgetrect.MinX + 1;
        mxgad->Height   = gadgetrect.MaxY - gadgetrect.MinY + 1;

        mxgad->BoundsLeftEdge = mxgad->LeftEdge;
        mxgad->BoundsTopEdge  = mxgad->TopEdge;
        mxgad->BoundsWidth    = mxgad->Width;
        mxgad->BoundsHeight   = mxgad->Height;

        if (ti = findGTTagItem(GTMX_TitlePlace,taglist))
        {
            placeGadgetText(mxgad,ti->ti_Data,PLACETEXT_ABOVE,NULL);
        }
        else
        {
            mxgad->GadgetText = NULL;
        }

        SetMXAttrsA(mxgad,NULL,taglist);

        /* Move the dummy-gadget to the end of this list, so that
         * it is the one that is returned:
         */
        preMX->NextGadget = mxgad->NextGadget;
        gad->NextGadget = mxgad;
        mxgad->NextGadget = NULL;
    }

    return(mxgad);
}


/*------------------------------------------------------------------------*/

/*/ HandleMX()
 *
 * Function to handle IDCMP events that relate to MutualExclude gadgets.
 * Call this with each IDCMP message that relates to a MX gadget.
 *
 * Created:  29-Aug-89, Peter Cherna
 * Modified: 23-Oct-89, Peter Cherna
 *
 */

BOOL HandleMX( struct ExtGadget *gad, struct IntuiMessage *imsg )
{
    DP(("HMX:  Entry\n"));
    DP(("HMX:  gad #%ld at $%lx\n", gad->GadgetID, gad));
    DP(("HMX:  gad->Flags $%lx, gad->Activation $%lx\n", gad->Flags,
	gad->Activation));
    /* Activate the gadget, and see if it was not the active one: */
    DP(("HMX:  Calling NewActiveMX\n"));
    if (NewActiveMX(gad, imsg->IDCMPWindow))
    {
	DP(("HMX:  A new one is active: # %ld\n", gad->GadgetID));
	/* The QuasiMessage has the ordinal value in the code, and
	 * the dummy-gadget (parent) as the object.
	 */
	imsg->IAddress = (APTR) SGAD(gad)->sg_Parent;
	imsg->Code     = gad->GadgetID;
	return(TRUE);
    }

    return(FALSE);
}

/*------------------------------------------------------------------------*/


void SetMXAttrsA(struct ExtGadget *gad, struct Window *win,
                 struct TagItem *taglist)
{
struct ExtGadget *activegad;
UWORD          newactive = getGTTagData(GTMX_Active, MXID(gad)->mxid_Active, taglist);
WORD           cnt;

    DP(("SetMXAttrs: win $%lx, gad $%lx, newactive %d\n",
	win, gad, newactive));

    activegad = MXID(gad)->mxid_FirstGadget;
    cnt = MXID(gad)->mxid_NumGadgets;
    while (cnt--)
    {
	TagAbleGadget(activegad,win,taglist);
	activegad = (struct ExtGadget *)activegad->NextGadget;
    }

    activegad = MXID(gad)->mxid_FirstGadget;
    while (newactive--)
    {
	activegad = (struct ExtGadget *)activegad->NextGadget;
    }
    DP(("New active mx gadget at $%lx\n", activegad));

    /* Activate the gadget: */
    DP(("Activating it\n"));
    NewActiveMX(activegad, win);
}


/*------------------------------------------------------------------------*/

/*/ NewActiveMX()
 *
 * Internal function to make the supplied Gadget the active one
 * in its group (called from HandleMX() or SetMXAttrs()).
 *
 * Returns:  TRUE if the active gadget changed, else returns FALSE.
 *
 * Created:  14-Oct-89, Peter Cherna
 * Modified: 23-Oct-89, Peter Cherna
 */

static BOOL NewActiveMX( struct ExtGadget *gad, struct Window *win)
{
struct ExtGadget *mxgad = SGAD(gad)->sg_Parent;
struct ExtGadget *pastactive;

    DP(("NAMX:  mxgad at $%lx\n", mxgad));
    /* No processing is necessary if this is already the
     * active gadget.
     */
    if (gad != MXID(mxgad)->mxid_ActiveGadget)
    {
	if (pastactive = MXID(mxgad)->mxid_ActiveGadget)
	{
            DP(("NAMX:  Deactivating previously active gadget\n"));
            /* Clear the SELECTED bit of the Flags field of the
             * previously active gadget:
             */
            SelectGadget( pastactive, win, FALSE );
        }

	/* Set the SELECTED bit of the Flags field of the newly
	 * selected gadget:
	 */
	SelectGadget( gad, win, TRUE );

	/* Note the new active gadget: */
	MXID(mxgad)->mxid_ActiveGadget = gad;
	MXID(mxgad)->mxid_Active       = gad->GadgetID;
	return(TRUE);
    }

    return(FALSE);
}
