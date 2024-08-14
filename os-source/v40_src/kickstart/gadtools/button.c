/*** button.c *************************************************************
*
*   button.c	- Button gadget routines
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*
*   $Id: button.c,v 39.10 92/10/16 18:30:28 vertex Exp $
*
**************************************************************************/

/*------------------------------------------------------------------------*/

#include "gtinclude.h"

/*------------------------------------------------------------------------*/

/* Function Prototypes: */

/* Private: */
struct ExtGadget *CreateButtonGadgetA (struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
void SetButtonAttrsA (struct ExtGadget *gad, struct Window *win,
    struct TagItem *taglist);

/*------------------------------------------------------------------------*/

typedef void (*SETATTRFCN)();


/* CreateButtonGadgetA() */

struct ExtGadget *CreateButtonGadgetA(struct ExtGadget *gad, struct NewGadget *ng,
                                   struct TagItem *taglist)
{
    DP(("CBG:  About to CreateGadget...\n"));

    /* NB: the scroller's arrows are built out of GadTools buttons,
     * and they depend on being able to pass GT_ExtraSize right
     * through here.  If ever button gadgets grow any extra data
     * of their own, someone will have to change a bit.
     */
    if (gad = (struct ExtGadget *)CreateGadgetA( GENERIC_KIND, gad, ng, taglist ))
    {
        DP(("CBG:  gad at $%lx\n", gad));
        /* A boolean RELVERIFY gadget with complement-highlighting: */

        gad->Flags |= GFLG_GADGIMAGE|GFLG_GADGHIMAGE;

        /* By default, buttons are RELVERIFY only */
        gad->Activation = GACT_RELVERIFY;
        if (getGATagData(GA_Immediate,FALSE,taglist))
        {
            /* V39 and up recognize GA_Immediate to mean GADGIMMEDIATE
             * too.  Scroller buttons use this.
             */
            gad->Activation |= GACT_IMMEDIATE;
        }

        gad->GadgetType        |= GTYP_BOOLGADGET;
        SGAD(gad)->sg_SetAttrs  = (SETATTRFCN)TagAbleGadget;
        SGAD(gad)->sg_GetTable  = Button_GetTable;
        SGAD(gad)->sg_Flags     = SG_EXTRAFREE_DISPOSE;

        placeGadgetText( gad, ng->ng_Flags, PLACETEXT_IN, NULL );

        if (!(gad->SelectRender = gad->GadgetRender =
                             NewObject(GadToolsBase->gtb_GTButtonIClass, NULL,
                                       IA_Width,     gad->Width,
                                       IA_Height,    gad->Height,
                                       MYIA_IText,   gad->GadgetText,
                                       IA_FrameType, FRAME_BUTTON,
                                       TAG_DONE)))
        {
            return(NULL);
        }
        gad->GadgetText = NULL;

        TagAbleGadget(gad,NULL,taglist);
    }

    DP(("CBG:  Done\n"));
    return(gad);
}


/*------------------------------------------------------------------------*/


/*/ SetButtonAttrsA()
 *
 * Function to change a button's attributes.  Currently, only GA_DISABLED
 * is recognized.
 *
 * Created:   8-Jan-90, Peter Cherna
 * Modified:  3-Apr-90, Peter Cherna
 *
 */
#if 0
void SetButtonAttrsA( struct ExtGadget *gad, struct Window *win,
    struct TagItem *taglist )
{
    /* Test for GA_Disabled, set GADGDISABLED and refresh accordingly */
    TagAbleGadget(gad, win, taglist);
}
#endif

/*------------------------------------------------------------------------*/

