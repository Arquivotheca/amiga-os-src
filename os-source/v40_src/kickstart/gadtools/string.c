/*** string.c *************************************************************
*
*   string.c	- String gadget support routines
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*
*   $Id: string.c,v 39.13 92/10/16 18:31:52 vertex Exp $
*
**************************************************************************/

/*------------------------------------------------------------------------*/

/*
    Notes:

    I think I should consider resetting the StringInfo->BufferPos
    and StringInfo->DispPos during a Set...Attrs()
*/

/*------------------------------------------------------------------------*/

#include "gtinclude.h"
#include <intuition/sghooks.h>
#include <intuition/gadgetclass.h>

/* These guys haven't yet appeared in the public includes */
/*
#ifndef GFLG_TABCYCLE
#define GFLG_TABCYCLE	0x0200
#endif

#ifndef STRINGA_ExitHelp
#define STRINGA_ExitHelp	(STRINGA_Dummy + 0x0013)
#endif

#ifndef SGM_EXITHELP
#define SGM_EXITHELP	(1L << 7)
#endif
*/

/*------------------------------------------------------------------------*/

/* Function Prototypes: */

/* Internal: */

struct ExtGadget *CreateStringGadgetA (struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
struct ExtGadget *CreateIntegerGadgetA (struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
void SetStringGadgetAttrsA (struct ExtGadget *gad, struct Window *win,
    struct TagItem *taglist);
void SetIntegerGadgetAttrsA (struct ExtGadget *gad, struct Window *win,
    struct TagItem *taglist);


/*****************************************************************************/


/* CreateStringGadgetA() */

/* NOTE VERY WELL:
 * The need for GA_Immediate support is very compelling.  Apps need
 * to be able to know when a string gadget went active for several
 * reasons, including being able to re-activate the last one, and
 * also knowing which gadgets have been touched for validation or
 * updating reasons.  The direct setting of the GACT_IMMEDIATE flag
 * in the gadget after creation is officially tolerated provided
 * it is only done under V37, and provided the GA_Immediate tag was
 * also provided.  However, it's inevitable that some will have
 * poked this flag regardless.  This could have serious implications
 * on underlay gadgets, since CreateGadgetA() would return the
 * underlay gadget (which should not be made GACT_IMMEDIATE).
 */
struct ExtGadget *CreateStringGadgetA(struct ExtGadget *gad, struct NewGadget *ng,
                                   struct TagItem *taglist)
{
struct Rectangle  rect;
ULONG             place;
UWORD             maxchars;
struct Hook      *edithook;

    /* Add one for the cursor, so that you'll really be able to type
     * as many characters as you asked for
     */
    maxchars = getGTTagData(GTST_MaxChars, 64, taglist) + 1;
    edithook = (struct Hook *)getGTTagData(GTST_EditHook, NULL, taglist);

    /* Allocate space for the instance data and two or three buffers
     */
    if (gad = CreateGenericBase(gad, ng, STRING_IDATA_SIZE +
	                        ( (edithook ? 3 : 2) * ALIGNSIZE(maxchars+1) ),
                                taglist))
    {
        /* leave room for borders */
        gad->TopEdge  += TOPTRIM + BEVELYSIZE;
        gad->Height   -= (TBTRIM + 2 * BEVELYSIZE);
        gad->LeftEdge += LEFTTRIM + BEVELXSIZE;
        gad->Width    -= (LRTRIM + 2 * BEVELXSIZE);
	gad->Flags    |= GADGIMAGE|GADGHCOMP;

	/* By default, these gadgets are tab-cycling, but it can be
	 * turned off.
	 */
	if ( getGATagData( GA_TabCycle, TRUE, taglist ) )
	{
	    gad->Flags |= GFLG_TABCYCLE;
	}

	/* Get justification of left/right/center */
	gad->Activation = RELVERIFY | STRINGEXTEND |
	    getSTRINGTagData(STRINGA_Justification, STRINGLEFT, taglist);
	if ( getGATagData( GA_Immediate, FALSE, taglist ) )
	{
	    gad->Activation |= GACT_IMMEDIATE;
	}
	gad->GadgetType |= STRGADGET;
	SGAD(gad)->sg_SetAttrs = SetStringGadgetAttrsA;
	SGAD(gad)->sg_GetTable = String_GetTable;
	SGAD(gad)->sg_Flags = SG_EXTRAFREE_DISPOSE | SG_EXTRAFREE_CLOSEFONT;

	STID(gad)->stid_Sex.ActivePens[0] = STID(gad)->stid_Sex.Pens[0] =
	    VI(ng->ng_VisualInfo)->vi_textPen;
	STID(gad)->stid_Sex.ActivePens[1] = STID(gad)->stid_Sex.Pens[1] =
	    VI(ng->ng_VisualInfo)->vi_backgroundPen;
	/* set rect.MinY early, so we can tweak it later: */
	rect.MinY = - (TOPTRIM + BEVELYSIZE);
	if (ng->ng_TextAttr)
	{
	    STID(gad)->stid_Sex.Font = OpenFont(ng->ng_TextAttr);
	    if (!STID(gad)->stid_Sex.Font)
	    {
		return(NULL);
	    }
	    /* The gadget imagery has a blank pixel row both above
	     * and below the actual gadget area.  If the font is
	     * a little too tall, we can make use first of the
	     * bottom row, then of the top.  If that's not enough,
	     * we have to fail:
	     */
	    /* Temporarily using place */
	    place = max(0, STID(gad)->stid_Sex.Font->tf_YSize - gad->Height);
	    gad->Height += place;
	    place >>= 1;
	    gad->TopEdge -= place;
	    rect.MinY += place;
	    if (place > 2)
	    {
		return(NULL);
	    }
	}
	if (getSTRINGTagData(STRINGA_ReplaceMode, FALSE, taglist))
	{
	    STID(gad)->stid_Sex.InitialModes = SGM_REPLACE;
	}

	if (getSTRINGTagData(STRINGA_ExitHelp, FALSE, taglist))
	{
	    STID(gad)->stid_Sex.InitialModes |= SGM_EXITHELP;
	}
	STID(gad)->stid_StringInfo.Extension = &STID(gad)->stid_Sex;
	STID(gad)->stid_StringInfo.Buffer = (UBYTE *) MEMORY_FOLLOWING(STID(gad));
	STID(gad)->stid_StringInfo.UndoBuffer = (UBYTE *)
	    MEMORY_N_FOLLOWING(STID(gad)->stid_StringInfo.Buffer,
	    ALIGNSIZE(maxchars+1));
	if (edithook)
	{
	    STID(gad)->stid_Sex.EditHook = edithook;
	    STID(gad)->stid_Sex.WorkBuffer =
		(UBYTE *) MEMORY_N_FOLLOWING(STID(gad)->stid_StringInfo.UndoBuffer,
		ALIGNSIZE(maxchars+1));
	}

	/* This is the "extent" to the gadget visuals from the actual
	 * gadget upper left:
	 */
	rect.MinX = - (LEFTTRIM + BEVELXSIZE);
	/* rect.MinY is set already */
	/* Don't forget, Max = size - 1. */
	rect.MaxX = rect.MinX + ng->ng_Width - 1;
	rect.MaxY = rect.MinY + ng->ng_Height - 1;
	if (!(place = (ng->ng_Flags & PLACETEXT_MASK)))
	    place = PLACETEXT_LEFT;
	PlaceIntuiText(gad->GadgetText, &rect, place);
	PlaceHelpBox(gad);

	if ( !(gad->GadgetRender = (APTR) getBevelImage( rect.MinX, rect.MinY,
	    ng->ng_Width, ng->ng_Height, FRAME_RIDGE ) ) )
	{
	    return( NULL );
	}

	STID(gad)->stid_StringInfo.MaxChars = maxchars;
	gad->SpecialInfo = (APTR) &STID(gad)->stid_StringInfo;
        SetStringGadgetAttrsA(gad,NULL,taglist);
    }

    return(gad);
}


/*****************************************************************************/


VOID SetStringGadgetAttrsA(struct ExtGadget *gad, struct Window *win,
                           struct TagItem *taglist)
{
struct TagItem *tag;
WORD            pos;
UWORD           selected;

    if (tag = findGTTagItem(GTST_String,taglist))
    {
        selected = gad->Flags & GFLG_SELECTED;
	pos = removeGadget(win,gad);

	if (tag->ti_Data)
	{
	    stccpy(STID(gad)->stid_StringInfo.Buffer,(STRPTR)tag->ti_Data,STID(gad)->stid_StringInfo.MaxChars+1);
	}
	else
	{
	    *STID(gad)->stid_StringInfo.Buffer = '\0';
	}

        AddRefreshGadget(gad, win, pos);

        if (selected && win)
            ActivateGadget(gad,win,NULL);
    }

    TagAbleGadget(gad, win, taglist);
}


/*****************************************************************************/


struct ExtGadget *CreateIntegerGadgetA(struct ExtGadget *gad, struct NewGadget *ng,
                                       struct TagItem *taglist)
{
    if (gad = (struct ExtGadget *)CreateGadget(STRING_KIND,gad,ng,
                           GTST_MaxChars, getGTTagData(GTIN_MaxChars, 10, taglist),
                           GTST_String,   "0",
                           TAG_MORE,      taglist))
    {
	SGAD(gad)->sg_SetAttrs = SetIntegerGadgetAttrsA;
	SGAD(gad)->sg_GetTable = Integer_GetTable;
	gad->Activation |= LONGINT;
	SetIntegerGadgetAttrsA(gad,NULL,taglist);
    }

    return(gad);
}


/*****************************************************************************/


VOID SetIntegerGadgetAttrsA(struct ExtGadget *gad, struct Window *win,
                            struct TagItem *taglist)
{
struct TagItem *tag;
WORD            pos;
char            string[20];
UWORD           selected;

    if (tag = findGTTagItem(GTIN_Number, taglist))
    {
        selected = gad->Flags & GFLG_SELECTED;

        pos = removeGadget(win,gad);

	sprintf(string, "%ld", tag->ti_Data);

	STID(gad)->stid_StringInfo.LongInt = tag->ti_Data;

	stccpy(STID(gad)->stid_StringInfo.Buffer,string,STID(gad)->stid_StringInfo.MaxChars);

        AddRefreshGadget(gad, win, pos);

        if (selected && win)
            ActivateGadget(gad,win,NULL);
    }

    TagAbleGadget(gad, win, taglist);
}
