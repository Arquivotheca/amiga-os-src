/*** visual.c *************************************************************
*
*   visual.c	- Gadget Toolkit routines for Visuals
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*
*   $Id: visual.c,v 39.7 92/07/31 19:23:57 vertex Exp $
*
**************************************************************************/

/*------------------------------------------------------------------------*/

#include "gtinclude.h"

/*------------------------------------------------------------------------*/

/****** gadtools.library/GetVisualInfoA **************************************
*
*   NAME
*	GetVisualInfoA -- get information GadTools needs for visuals. (V36)
*	GetVisualInfo -- varargs stub for GetVisualInfoA(). (V36)
*
*   SYNOPSIS
*	vi = GetVisualInfoA(screen, tagList)
*	D0                  A0      A1
*
*	APTR vi = GetVisualInfoA(struct Screen *, struct TagItem *);
*
*	vi = GetVisualInfo(screen, firsttag, ...)
*
*	APTR vi = GetVisualInfo(struct Screen *, Tag, ...);
*
*   FUNCTION
*	Get a pointer to a (private) block of data containing various bits
*	of information that GadTools needs to ensure the best quality
*	visuals.  Use the result in the NewGadget structure of any gadget
*	you create, or as a parameter to the various menu calls.  Once the
*	gadgets/menus are no longer needed (after the last CloseWindow),
*	call FreeVisualInfo().
*
*   INPUTS
*	screen - pointer to the screen you will be opening on. This parameter
*		 may be NULL, in which case this function fails.
*	tagList - pointer to an array of tags providing optional extra
*		  parameters, or NULL.
*
*   TAGS
*	There are currently no tags defined for this function.
*
*   RESULT
*	vi - pointer to private data, or NULL for failure
*
*   SEE ALSO
*	FreeVisualInfo(), intuition/LockPubScreen(),
*	intuition/UnlockPubScreen()
*
******************************************************************************
*
* NOTES
*	I could in the future call graphics/GetVPModeID() on &scr->ViewPort
*	so that I could find out more about the display mode, like aspect
*	ratio.
*
*	Since the taglist parameter is not currently used, the actual
*	function does not define this parameter which makes the code
*	smaller.
*
* Created:   3-Dec-89, Peter Cherna
* Modified: 14-Feb-90, Peter Cherna
*
*/

struct VisualInfo * ASM LIB_GetVisualInfoA(REG(a0) struct Screen *scr)
{
struct VisualInfo *vi;

    if (vi                = AllocVec(sizeof(struct VisualInfo),MEMF_CLEAR))
    if (vi->vi_Screen     = scr)
    if (vi->vi_ScreenFont = OpenFont(scr->Font))
    if (vi->vi_DrawInfo   = GetScreenDrawInfo(scr))
    {
        vi->vi_textPen       = vi->vi_DrawInfo->dri_Pens[TEXTPEN];
        vi->vi_backgroundPen = vi->vi_DrawInfo->dri_Pens[BACKGROUNDPEN];
        return(vi);
    }

    FreeVisualInfo(vi);
    return(NULL);
}


/*------------------------------------------------------------------------*/

/****** gadtools.library/FreeVisualInfo **************************************
*
*   NAME
*	FreeVisualInfo -- return any resources taken by GetVisualInfo. (V36)
*
*   SYNOPSIS
*	FreeVisualInfo(vi)
*	               A0
*
*	VOID FreeVisualInfo(APTR);
*
*   FUNCTION
*	FreeVisualInfo() returns any memory or other resources that
*	were allocated by GetVisualInfoA().  You should only call this function
*	once you are done with using the gadgets (i.e. after CloseWindow()),
*	but while the screen is still valid (i.e. before CloseScreen() or
*	UnlockPubScreen()).
*
*   INPUTS
*	vi - pointer that was obtained by calling GetVisualInfoA(). This
*	     value may be NULL.
*
*   SEE ALSO
*	GetVisualInfoA()
*
******************************************************************************
*
*   NOTES
*	Privately, vi is declared as a struct VisualInfo *
*
* Created:   3-Dec-89, Peter Cherna
* Modified: 14-Jan-90, Peter Cherna
*
*/

VOID ASM LIB_FreeVisualInfo(REG(a0) struct VisualInfo *vi)
{
struct ImageLink *il;
struct ImageLink *next;

    if (vi)
    {
        il = vi->vi_Images;
        while (il)
        {
            next = il->il_Next;
            DisposeObject(il->il_Image);
            FreeVec(il);
            il = next;
        }

	FreeScreenDrawInfo(vi->vi_Screen,vi->vi_DrawInfo);
	closeFont(vi->vi_ScreenFont);
	FreeVec(vi);
    }
}


/*------------------------------------------------------------------------*/
#if 0

/*/ ColorLevel()
 *
 * Find a measure of the color intensity.  It is calculated as
 * 3(R^2) + 6(G^2) + (B^2)
 *
 * Created:  15-Dec-89, Peter Cherna (from MKS RenderInfo)
 * Modified: 15-Dec-89, Peter Cherna
 *
 */

WORD ColorLevel( UWORD rgb )
{
    WORD level;
    WORD tmp;

    /* Red (30%) */
    tmp = ((rgb>>8) & 15);
    level = (tmp*tmp) * 3;

    /* Green (60%) */
    tmp = ((rgb>>4) & 15);
    level += (tmp*tmp) * 6;

    /* Blue (10%) */
    tmp = (rgb & 15);
    level += (tmp*tmp);


    return(level);
}


/*------------------------------------------------------------------------*/

/*/ PickPens()
 *
 * By looking at the available colors, pick the "most appropriate"
 * choices of pen colors for artwork.
 * We have:
 * FieldPen (always pen 0)
 * TextPen (always pen 1)
 * ShinePen (brightest color that isn't the FieldPen)
 * ShadowPen (darkest color that isn't the FieldPen)
 * HighFieldPen (most "contrasty" color to the FieldPen that isn't one of
 *	the above four, or equals the TextPen if no other free colors
 *	are to be had).
 * HighTextPen (if HighFieldPen is different from TextPen, then HighTextPen
 *	is just TextPen, else it's FieldPen)
 *
 * Created:  15-Dec-89, Peter Cherna (from MKS FillInRenderInfo())
 * Modified: 15-Dec-89, Peter Cherna
 *
 */

VOID PickPens( struct VisualInfo *vi, struct Screen *scr )
{
    WORD numcolors;
    WORD loop;
    WORD loop1;
    WORD tmp;
    WORD colors[16];
    WORD pens[16];
    WORD delta, newdelta;
    WORD bestcolor;

    /* !!! If we have a pair of identical (in brightness) colors,
     * we want to guarantee the lower pen number.  Even if the sort
     * algorithm guarantees low to high pens for identical color-levels,
     * ShinePen could end up being the higher numbered one since it
     * picks from the high end of the list
     */
    numcolors = 1 << (scr->RastPort.BitMap->Depth);
    /* !!! For no reason, we limit ourselves to picking from 16 colors */
    if (numcolors > 16)
    {
	numcolors = 16;
    }

    if (vi->vi_ScreenDepth == 1)
    {
	/* running with 2 colors... */
	vi->vi_ShinePen = 1;
	vi->vi_ShadowPen = 1;
	vi->vi_TextPen = 1;
	vi->vi_FieldPen = 0;
	vi->vi_HighTextPen = 0;
	vi->vi_HighFieldPen = 1;
    }
    else
    {
	/* Get the colors: */
	for (loop = 0; loop < numcolors; loop++)
	{
	    colors[loop] = ColorLevel(
		(UWORD)GetRGB4(scr->ViewPort.ColorMap, loop));
	    pens[loop]=loop;
	}

	/* Sort darkest to brightest... */
	for (loop = 0; loop < (numcolors-1); loop++)
	    for (loop1 = loop+1; loop1 < numcolors; loop1++)
	    {
		if (colors[loop] > colors[loop1])
		{
		    tmp = colors[loop];
		    colors[loop] = colors[loop1];
		    colors[loop1] = tmp;
		    tmp = pens[loop];
		    pens[loop] = pens[loop1];
		    pens[loop1] = tmp;
		}
	    }

	D(
	for (loop = 0; loop < numcolors; loop++)
	{
	    DP(("PP:  pen %ld, color level %ld\n",
		(LONG)pens[loop], (LONG)colors[loop]));
	}
	);
	/* Now, pick the pens... */
	vi->vi_TextPen = 1;
	vi->vi_FieldPen = 0;
	/* ShadowPen is darkest other than FieldPen */
	loop = 0;
	while (!(vi->vi_ShadowPen = pens[loop++]) != vi->vi_FieldPen)
	    ;

	/* ShinePen is brightest other than FieldPen */
	loop = numcolors - 1;
	while (!(vi->vi_ShinePen = pens[loop--]) != vi->vi_FieldPen)
	    ;

	/* HighFieldPen is the color farthest from the FieldPen that
	 * is none of FieldPen, ShinePen, ShadowPen
	 */
	/* Set bestcolor (future HighFieldPen) to the TextPen (by default) */
	bestcolor = 0;
	while (pens[bestcolor] != 1)
	{
	    bestcolor++;
	}

	/* Set loop to position number of FieldPen */
	loop = 0;
	while (pens[loop] != vi->vi_FieldPen)
	{
	    loop++;
	}
	DP(("PP:  Field Pen is at #%ld\n", (LONG)loop));
	delta = 0;
	for (loop1 = 0; loop1 < numcolors; loop1++)
	{
	    DP(("PP:  Considering #%ld\n", (LONG)loop1));
	    if ((pens[loop1] != vi->vi_TextPen) &&
		(pens[loop1] != vi->vi_FieldPen) &&
		(pens[loop1] != vi->vi_ShinePen) &&
		(pens[loop1] != vi->vi_ShadowPen))
	    {
		newdelta = colors[loop1] - colors[loop];
		if (newdelta < 0)
		{
		    newdelta = -newdelta;
		}
		DP(("PP:  Its delta is %ld\n", (LONG)newdelta));
		if (newdelta > delta)
		{
		    DP(("PP:  A new best color\n"));
		    delta = newdelta;
		    bestcolor = loop1;
		}
	    }
	}
	vi->vi_HighFieldPen = pens[bestcolor];

	if (vi->vi_HighFieldPen == vi->vi_TextPen)
	{
	    vi->vi_HighTextPen = vi->vi_FieldPen;
	}
	else
	{
	    vi->vi_HighTextPen = vi->vi_TextPen;
	}
    }
    DP(("PP:  TextPen %ld, FieldPen %ld\n", (LONG)vi->vi_TextPen,
	(LONG)vi->vi_FieldPen));
    DP(("PP:  HighTextPen %ld, HighFieldPen %ld\n", (LONG)vi->vi_HighTextPen,
	(LONG)vi->vi_HighFieldPen));
    DP(("PP:  ShinePen %ld, ShadowPen %ld\n", (LONG)vi->vi_ShinePen,
	(LONG)vi->vi_ShadowPen));
}

/*------------------------------------------------------------------------*/
#endif
