/*** generic.c ************************************************************
*
*   generic.c	- Generic gadget routines
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*
*   $Id: generic.c,v 39.10 92/10/16 18:32:49 vertex Exp $
*
**************************************************************************/

/*------------------------------------------------------------------------*/

#include "gtinclude.h"

/*------------------------------------------------------------------------*/

/* Function Prototypes: */

/* Public: */
void __asm
LIB_FreeGadgets (register __a0 struct ExtGadget *gad);


/* Internal: */
struct ExtGadget *CreateGenericA (struct ExtGadget *prevgad, struct NewGadget *ng,
    struct TagItem *taglist);
struct TagItem *TagAbleGadget (struct ExtGadget *gad, struct Window *win,
    struct TagItem *taglist);
void SelectGadget (struct ExtGadget *gad, struct Window *win,
                   BOOL select);

struct ExtGadget *CreateGenericBase (struct ExtGadget *gad, struct NewGadget *ng,
	ULONG extrasize, struct TagItem *moretags);

/*------------------------------------------------------------------------*/

/* CreateGenericA() */

struct ExtGadget *CreateGenericA(struct ExtGadget *prevgad,
                                 struct NewGadget *ng,
                                 struct TagItem *taglist)
{
    /* !!! NOTE WELL:  CreateContext() passes us a NewGadget structure
     * with a VisualInfo pointer of ~0, so that CreateGadgetA() doesn't
     * complain.  We had better not depend on the VisualInfo for that
     * case (the whole NewGadget structure [including ng_GadgetText]
     * is NULL except for VisualInfo) !!!
     *
     * !!! The routines that call me here generally pass me their taglist.
     * They probably should filter out what they've used!
     */
    ULONG extrasize = getGTTagData(GT_ExtraSize, 0, taglist);
    char underline = getGTTagData(GT_Underscore, 0, taglist);
    ULONG textmem = 0;
    UWORD labellen;
    struct IntuiText *gitext;
    struct ExtGadget *gad;

    DP(("CG:  Enter.  VisualInfo at $%lx\n", ng->ng_VisualInfo));
    if (!prevgad)
    {
	DP(("CG: failing due to NULL prevgad\n"));
	return(NULL);
    }

    if (ng->ng_GadgetText)
    {
	/* Need an IntuiText structure */
	textmem = sizeof(struct IntuiText);
	/* To handle underlines, I need a string buffer plus another
	 * IntuiText structure and a TextAttr
	 */
	if (underline)
	{
	    labellen = strlen(ng->ng_GadgetText);
	    DP(("CG:  Underline symbol: '%lc' $%lx\n", underline, underline));
	    textmem += (sizeof(struct TTextAttr) + sizeof(struct IntuiText)) +
		labellen+3;
	}
    }

    if (gad = AllocVec((sizeof(struct SpecialGadget) + ALIGNSIZE(extrasize) +
	               textmem), MEMF_CLEAR))
    {
        DP(("CG:  Allocated $%lx bytes at $%lx\n", (sizeof(struct SpecialGadget) +
	    ALIGNSIZE(extrasize) + textmem),
        	gad));

	prevgad->NextGadget = gad;
	DP(("CG:  Gadget itself is $%lx bytes at $%lx\n",
	    sizeof(struct ExtGadget), gad));
	gad->LeftEdge       = ng->ng_LeftEdge;
	gad->TopEdge        = ng->ng_TopEdge;
	gad->Width          = ng->ng_Width;
	gad->Height         = ng->ng_Height;
	gad->GadgetID       = ng->ng_GadgetID;
	gad->UserData       = ng->ng_UserData;
	gad->GadgetType     = GADTOOL_TYPE;
	gad->BoundsLeftEdge = ng->ng_LeftEdge;
	gad->BoundsTopEdge  = ng->ng_TopEdge;
	gad->BoundsWidth    = ng->ng_Width;
	gad->BoundsHeight   = ng->ng_Height;
        gad->MoreFlags      = GMORE_BOUNDS | GMORE_GADGETHELP;
        gad->Flags          = GFLG_EXTENDED;

	DP(("CG:  Extrasize $%lx, rounded to $%lx\n", extrasize,
	    ALIGNSIZE(extrasize)));
	if (ng->ng_GadgetText)
	{
	    /* Space for gadget text follows gadget and extrasize: */
	    gitext = gad->GadgetText = (struct IntuiText *)
		MEMORY_N_FOLLOWING(MEMORY_FOLLOWING(SGAD(gad)),
		    ALIGNSIZE(extrasize));
	    DP(("CG:  Gadget IntuiText is $%lx bytes at $%lx\n",
		sizeof(struct IntuiText), gitext));
	    DP(("CG:  ng->ng_GadgetText pointer: $%lx, string '%s'\n",
		ng->ng_GadgetText, ng->ng_GadgetText));
	    DP(("CG:  gad->GadgetText->IText pointer is at $%lx\n",
		&gad->GadgetText->IText));
	    gitext->IText = ng->ng_GadgetText;
	    DP(("CG:  Set IText to '%s'\n", gitext->IText));
	    /* Use hilighttextPen or textPen: */
	    gitext->FrontPen = VI(ng->ng_VisualInfo)->
		vi_DrawInfo->dri_Pens[ (ng->ng_Flags & NG_HIGHLABEL) ?
		HIGHLIGHTTEXTPEN : TEXTPEN];

	    DP(("CG:  FrontPen set to %ld\n", ((LONG)gitext->FrontPen)));
	    gitext->DrawMode = JAM1;

	    gitext->ITextFont = ng->ng_TextAttr;
	    DP(("CG:  ITextFont set to $%lx\n", gitext->ITextFont));

	    if (underline)
	    {
		struct IntuiText *uitext;
		char *dest;
		char *src;

		uitext = MEMORY_FOLLOWING(gitext);
		*uitext = *gitext;
		gitext->NextText = uitext;

		uitext->ITextFont = MEMORY_FOLLOWING(uitext);
		/* Try *ng->ng_TextAttr */

                if (gitext->ITextFont->ta_Style & FSF_TAGGED)
                    *(struct TTextAttr *)uitext->ITextFont = *(struct TTextAttr *)gitext->ITextFont;
		else
                    *uitext->ITextFont = *gitext->ITextFont;

		/* Set style to underlined */
		uitext->ITextFont->ta_Style |= FSF_UNDERLINED;

		/* gitext should point to the text, with the underscore
		 * symbol removed.  uitext should point to an underscored
		 * copy of the right character
		 */
		dest = gitext->IText = (APTR)((ULONG)MEMORY_FOLLOWING(((struct TTextAttr *)uitext->ITextFont)));
		uitext->IText = (dest + (labellen+1));
		src = ng->ng_GadgetText;
		while (*src)
		{
		    if (*src != underline)
		    {
			DP(("CG: Copying '%lc'\n", *src));
			*dest++ = *src++;
		    }
		    else
		    {
			DP(("CG: Found underline.  Underscoring '%lc'\n",
			    *(src+1) ));
			/* Character after the underline */
			*uitext->IText = *(++src);
			uitext->LeftEdge = IntuiTextLength(gitext);
		    }
		}
		*dest = '\0';
		DP(("CG: GadgetText is now '%s'\n", gitext->IText));
		DP(("CG: UnderlineText is now '%s'\n", uitext->IText));
	    }
	}
    }
    return(gad);
}


/*------------------------------------------------------------------------*/

/****** gadtools.library/FreeGadgets *****************************************
*
*   NAME
*	FreeGadgets -- free a linked list of gadgets. (V36)
*
*   SYNOPSIS
*	FreeGadgets(glist)
*	            A0
*
*	VOID FreeGadgets(struct Gadget *glist);
*	                 A0
*
*   FUNCTION
*	Frees any GadTools gadgets found on the linked list of gadgets
*	beginning with the specified one.  Frees all the memory that was
*	allocated by CreateGadgetA().  This function will return safely
*	with no action if it receives a NULL parameter.
*
*   INPUTS
*	glist - pointer to first gadget in list to be freed.
*
*   SEE ALSO
*	CreateGadgetA()
*
******************************************************************************
*
* Created:  28-Jun-89 (based on jimm's FreeGadgets())
* Modified:  3-Dec-89
*
*/

VOID ASM LIB_FreeGadgets(REG(a0) struct ExtGadget *gad)
{
struct ExtGadget *nextgad;

    DP(("\nFG: gad at $%lx\n", gad));
    while (gad)
    {
	nextgad = (struct ExtGadget *)gad->NextGadget;
	if (gad->GadgetType & GADTOOL_TYPE)
	{
	    /* The most common ExtraFree job is a DisposeObject()
	     * on the imagery
	     */
	    if (SGAD(gad)->sg_Flags & SG_EXTRAFREE_DISPOSE)
		DisposeObject(gad->GadgetRender);

	    if (SGAD(gad)->sg_Flags & SG_EXTRAFREE_DISPOSE_LV)
		closeFont(LVID(gad)->lvid_Font);

	    if (SGAD(gad)->sg_Flags & SG_EXTRAFREE_CLOSEFONT)
		closeFont(STID(gad)->stid_Sex.Font);

	    /* It can happen that our caller calls FreeGadgets() before
	     * the last call to GT_ReplyIMsg().  In that case, we want
	     * to let GT_ReplyIMsg() free the context gadget (and its
	     * embedded message.
	     */
	    if (SGAD(gad)->sg_Flags & SG_CONTEXT)
	    {
		/* If QuasiMessage is in use, then set the
		 * Context Gadget's DelayedFree marker.
		 * Then, GT_PostFilterIMsg() will look for the
		 * DelayedFree marker and free the Context gadget
		 * for us.
		 *
		 * ** NOTE assignment, not equality test! **
		 *
		 */
		if (CTID(gad)->ctid_DelayedFree = CTID(gad)->ctid_QuasiUsed)
		{
		    /* Short-circuit the upcoming FreeVec() */
		    gad = NULL;
		}
	    }
	    FreeVec(gad);
	}
	gad = nextgad;
    }
}


/*------------------------------------------------------------------------*/

/*/ TagAbleGadget()
 *
 * Function to enable/disable a gadget and refresh, based on GA_Disabled
 * existing in the supplied tag-list.
 *
 */

struct TagItem *TagAbleGadget(struct ExtGadget *gad, struct Window *win,
                              struct TagItem *taglist)
{
UWORD           pos;
BOOL            disable, currently_disabled;
struct TagItem *tag;

    if (tag = findTagItem(GA_Disabled,taglist))
    {
	disable = tag->ti_Data;
	currently_disabled = (gad->Flags & GADGDISABLED);

	if (((disable) && (!currently_disabled)) ||
	    ((!disable) && (currently_disabled)))
	{
            pos = removeGadget(win,gad);
	    gad->Flags ^= GADGDISABLED;
            AddRefreshGadget(gad, win, pos);
	}
    }

    return(tag);
}

/*------------------------------------------------------------------------*/

/*/ SelectGadget()
 *
 * Function to (de)select a gadget and refresh, in the boring old way.
 *
 */


void SelectGadget(struct ExtGadget *gad, struct Window *win,
                  BOOL select)
{
WORD pos;

    pos = removeGadget(win,gad);

    if (select)
	gad->Flags |= SELECTED;
    else
	gad->Flags &= ~SELECTED;

    AddRefreshGadget(gad,win,pos);
}

/*------------------------------------------------------------------------*/

struct ExtGadget *CreateGenericBase(struct ExtGadget *gad, struct NewGadget *ng,
                                    ULONG extrasize, struct TagItem *moretags)
{
    return ((struct ExtGadget *)CreateGadget(GENERIC_KIND,gad,ng,
                                             GT_ExtraSize, extrasize,
                                             TAG_MORE,     moretags));
}
