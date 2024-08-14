/*** checkbox.c ***********************************************************
*
*   checkbox.c	- Checkbox gadget support routines
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*
*   $Id: checkbox.c,v 39.10 92/05/29 15:31:56 vertex Exp $
*
**************************************************************************/

/*------------------------------------------------------------------------*/

#include "gtinclude.h"

/*------------------------------------------------------------------------*/

/* Function Prototypes: */

/* Internal: */
struct ExtGadget *CreateCheckBoxA (struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
void SetCheckBoxAttrsA (struct ExtGadget *gad, struct Window *win,
    struct TagItem *taglist);
BOOL HandleCheckbox (struct ExtGadget *gad, struct IntuiMessage *imsg);


/*---------------------------------------------------------------------*/

/* CreateCheckBoxA() */

struct ExtGadget *CreateCheckBoxA(struct ExtGadget *gad, struct NewGadget *ng,
                               struct TagItem *taglist)
{
    if (gad = CreateGenericBase(gad, ng, CHECKBOX_IDATA_SIZE,taglist))
    {
        /* V37 always had a fixed width checkbox.  We override for
         * compatibility, unless the new GTCB_Scaled tag is supplied.
         */
        if ( !getGTTagData( GTCB_Scaled, FALSE, taglist ) )
        {
            gad->Width = CHECKBOX_WIDTH;
            gad->Height = CHECKBOX_HEIGHT;
            gad->BoundsWidth  = CHECKBOX_WIDTH;
            gad->BoundsHeight = CHECKBOX_HEIGHT;

            /* Here we do a strange thing.  For most gadget classes, the text
             * is contained in a box whose TopEdge corresponds to what
             * the caller calls ng.ng_TopEdge.  So for any given font size,
             * the font is a constant distance below this point.  Unfortunately,
             * for checkboxes (and mx gadgets), the ng_TopEdge represented the
             * TopEdge of the CheckBox gadget, but the text is not a fixed
             * distance below this point, but rather vertically centered
             * with respect to the checkbox height (a constant).  So the
             * bigger the font, the higher it would appear.
             *
             * Smart checkboxes (GTCB_Scaled) allows checkboxes of arbitrary
             * height.  In these cases, the font is positioned correctly.
             *
             * In the meantime, we behave similar to that, except the checkbox
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
            if ( ( (ng->ng_Flags & PLACETEXT_MASK) <= PLACETEXT_RIGHT ) && (gad->GadgetText))
            {
                gad->TopEdge += (ng->ng_TextAttr->ta_YSize-7)/2;
                gad->BoundsTopEdge = gad->TopEdge;
            }
        }

        gad->Flags |= GADGIMAGE | GADGHIMAGE;
        SetCheckBoxAttrsA(gad,NULL,taglist);
        gad->Activation = RELVERIFY | TOGGLESELECT;
        gad->GadgetType |= BOOLGADGET;

        placeGadgetText(gad, ng->ng_Flags, PLACETEXT_LEFT, NULL);

        SGAD(gad)->sg_SetAttrs     = SetCheckBoxAttrsA;
        SGAD(gad)->sg_EventHandler = HandleCheckbox;
        SGAD(gad)->sg_GetTable     = Checkbox_GetTable;

        if (!(gad->SelectRender = gad->GadgetRender = getSysImage(ng->ng_VisualInfo,gad->Width,gad->Height,CHECKIMAGE)))
            return(NULL);
    }

    DP(("CCB:  Done\n"));

    return(gad);
}

/*------------------------------------------------------------------------*/

/*/ HandleCheckbox()
 *
 * Function to handle IDCMP events that relate to Checkbox gadgets.
 * Call this with each IDCMP message that relates to a checkbox gadget.
 *
 */

BOOL HandleCheckbox( struct ExtGadget *gad, struct IntuiMessage *imsg )
{
    DP(("HCB:  Entry\n"));

    imsg->Code = CBID(gad)->cbid_Checked = ((gad->Flags & GFLG_SELECTED) != 0);

    return(TRUE);
}

/*------------------------------------------------------------------------*/

void SetCheckBoxAttrsA(struct ExtGadget *gad, struct Window *win,
                       struct TagItem *taglist)
{
struct TagItem *ti;

    if (ti = findGTTagItem(GTCB_Checked,taglist))
    {
	CBID(gad)->cbid_Checked = (ti->ti_Data != 0);
	SelectGadget(gad,win,CBID(gad)->cbid_Checked);
    }

    TagAbleGadget(gad,win,taglist);

    DP(("SCBA:  Exit\n"));
}
