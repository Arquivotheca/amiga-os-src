/*** strings.c ****************************************************************
 *
 *  These are the routines for handling the String Gadget
 *
 *  $Id: strings.c,v 40.0 94/02/15 17:54:57 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1985, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"

/*****************************************************************************/

#define D(x)	;
#define DFE(x)	;		/* font extent	*/
#define DSC(x)	;		/* size check	*/

/*****************************************************************************/

/* 7000 is a very large number	*/
#define VBIG	(7000)
UBYTE nchar = 'n';
#define FGPEN (1)
#define BGPEN (0)
#define FIX_GALILEO

/*****************************************************************************/

/* this guy does the highlight stuff again, now. */
void displayString
    (struct RastPort *rp, struct Gadget *sg, struct GadgetInfo *gi, struct IBox *gbox, struct StringExtend *sex)
{
    extern struct IFontPrefs topazprefs;
    struct StringInfo *si = (struct StringInfo *) sg->SpecialInfo;
    struct Rectangle gadgrect;
    struct Rectangle textrect;
    struct TextExtent te;
    LONG extrawidth = 0;	/* space on right	*/
    UBYTE apen, bpen;
    struct RastPort clonerp;
    LONG effectiveBPos;		/* indexes last char if BP == NC */
    LONG biggestBPos;		/* numchars, -1 if fixedfield	*/
    struct IBox cursor;
    struct TextFont *fp = NULL;

    /* Peter 17-Jan-91:  We need to adjust the gadget's width,
     * for compatibility with many 1.3 applications.
     */
    LONG gadwidth;

    D (printf ("============\ndisplayString gadget at %lx\n"));
    D (sinfo (si));
    /* D( kquery( "press RTC" ) ); */

    cloneRP (&clonerp, rp);

    /* clonerp.Mask = -1; *already */

    /* set up rastport, from extended information	*/
    if (sex)
    {
	D (printf ("extended string gadget\n"));
	D (dumpSex ("dS:gi sex", sex));

	if (sg->Flags & SELECTED)	/* is active gadget */
	{
	    apen = sex->ActivePens[0];
	    bpen = sex->ActivePens[1];
	}
	else
	{
	    apen = sex->Pens[0];
	    bpen = sex->Pens[1];
	}
	if (sex->Font)
	{
	    D (printf ("sex font: %lx\n", sex->Font));
	    SetFont (&clonerp, sex->Font);
	}
    }
    else
    {
	apen = FGPEN;
	bpen = BGPEN;
	/* would like to leave font found in rp	*/
    }

    if (gbox->Height < clonerp.TxHeight)
    {
	DSC (printf ("gadget height %ld smaller than TxHeight %ld\n",
		     gbox->Height, clonerp.TxHeight));

	/* Slam down to topaz 8	*/
	fp = ISetFont (&clonerp, &topazprefs.ifp_TABuff.tab_TAttr);
    }

    /* 	------  set up BufferPos ------
     * make sure BufferPos is within string (or immediately following)
     * and account for space on right if cursor is past last position
     * (note that 'extrawidth' often indicates that cursor is off right)
     */
    si->NumChars = strlen (si->Buffer);

    effectiveBPos = si->BufferPos;
    biggestBPos = si->NumChars;

    /* keep the cursor on a valid character for fixedfield	*/
    if (sex && (sex->InitialModes & SGM_FIXEDFIELD))
	biggestBPos--;

    if (si->BufferPos >= biggestBPos)
    {
#if 0
	extrawidth = &clonerp->TxWidth;
#else
	extrawidth = TextLength (&clonerp, &nchar, 1);
#endif
	D (printf ("make space for cursor on right side\n"));
	effectiveBPos = si->BufferPos = biggestBPos;
	effectiveBPos--;
    }
    D (printf ("extrawidth: %ld\n", extrawidth));

    D (printf ("effective bufferpos: %ld\n", effectiveBPos));

    /* make sure BufferPos isn't out of sight to the left	*/
    si->DispPos = imax (0, imin (si->DispPos, effectiveBPos));

    /*	----	set up gadget fill bounds, in compatible way ---- */
    boxToRect (gbox, &gadgrect);
    D (dumpBox ("gadget box", gbox));
    D (dumpRect ("gadget rect", &gadgrect));

    gadwidth = gbox->Width;

    /* backward compatibility: used to blast out fixed-width spaces
     * to a maximum of MaxChars or something.  Use FontExtent
     * instead of space-char width, to erase whatever's there.
     */
    if (!sex)
    {
	LONG blastwidth;
	LONG fontwidth;

	DFE (printf ("call FontExtent\n"));
	FontExtent (clonerp.Font, &te);
	DFE (dumpTExtent ("font extent", &te));
	fontwidth = (te.te_Extent.MaxX + 1);

	/* Peter 17-Jan-91:  For fixed width fonts in non-extended
	 * string gadgets, we round down the blasting width
	 * to an even multiple of the character width.
	 */
	if (!TESTFLAG (clonerp.Font->tf_Flags, FPF_PROPORTIONAL))
	{
	    gadwidth -= (gadwidth % fontwidth);
	    DFE (printf ("rounded down width: %ld\n", gadwidth));
	}

	/* only last out as much width as chars might possibly
	 * use up.  For left-justified gadgets only.
	 */
	if (!(sg->Activation & (STRINGCENTER | STRINGRIGHT)))
	{

	    DFE (printf ("blast protection: MaxX: %ld\n", te.te_Extent.MaxX));
	    DFE (printf ("max chars: %ld ", si->MaxChars));

	    /* jimm: 5/4/90: was maxchars -1, but need to clear cursor */
	    blastwidth = (si->MaxChars) * fontwidth;

	    gadwidth = imin (blastwidth, gadwidth);

	    DFE (printf ("blastwidth: %ld\n", blastwidth));
	    DFE (printf ("old width %ld, new width %ld\n",
			 rectWidth (&gadgrect), newgwidth));
	}

	gadgrect.MaxX = gadgrect.MinX + gadwidth - 1;
	gadgrect.MaxY = gadgrect.MinY + clonerp.TxHeight - 1;
    }

    /*	----	Find amount of text to use, and extent ----	*/

    if (si->NumChars)
    {

	si->DispCount = TextFit (&clonerp, &si->Buffer[si->DispPos],
				 si->NumChars - si->DispPos, &te, NULL, 1,
				 gadwidth - extrawidth, VBIG);

	D (printf ("call TextFit with width %ld height %ld\n",
		   gadwidth - extrawidth, gbox->Height));

	D (sinfo (si));
	D (printf ("constraint width: %ld, extent width: %ld\n",
		   gadwidth - extrawidth, rectWidth (&te.te_Extent)));

	D (printf ("textlength: %ld\n", TextLength (&clonerp,
						    &si->Buffer[si->DispPos], si->DispCount)));

	/* BufferPos would be scrolled off to the right	*/
	if (si->DispPos + si->DispCount - 1 < effectiveBPos)
	{
	    si->DispCount = TextFit (&clonerp, &si->Buffer[effectiveBPos],
	    /* ZZZ: jimm: changed but no test yet, was MaxChars */
				     effectiveBPos + 1, &te, NULL, -1,
				     gadwidth - extrawidth, VBIG);

	    si->DispPos = effectiveBPos - si->DispCount + 1;
	    D (printf ("scroll back left.  DP: %ld, DC:%ld EBP: %ld\n",
		       si->DispPos, si->DispCount, effectiveBPos));
	}
	/* be sure to show maximum possible number of chars	*/
	else if ((si->DispCount + si->DispPos >= si->NumChars) && si->DispPos)
	{
	    si->DispCount = TextFit (&clonerp, &si->Buffer[si->NumChars - 1],
				     si->NumChars, &te, NULL, -1,
				     gadwidth - extrawidth, VBIG);
	    si->DispPos = si->NumChars - si->DispCount;
	    D (printf ("not showing max, scroll right, new DC %ld\n",
		       si->DispCount));
	}

	/*	----	set up textrect aligned with ULC of gadget ----	*/
	D (dumpRect ("gadgrect", &gadgrect));
	D (dumpRect ("textextent", &te.te_Extent));

	textrect = te.te_Extent;
	offsetRect (&textrect, gadgrect.MinX - te.te_Extent.MinX,
		    gadgrect.MinY - te.te_Extent.MinY);
    }
    else
	/* NumChars == 0	*/
    {
	D (printf ("NumChars == 0\n"));
	si->DispCount = 0;
	si->DispPos = 0;
    }

    /*	-----	 handle centering, right just ----	*/
    if (sg->Activation & (STRINGCENTER | STRINGRIGHT))
    {
	si->CLeft = rectWidth (&gadgrect) - extrawidth;
	if (si->NumChars)
	{
	    si->CLeft -= rectWidth (&textrect);
	}

	if (sg->Activation & STRINGCENTER)
	{
	    si->CLeft /= 2;
	}
    }
    else
	si->CLeft = 0;

    D (printf ("ready to draw the sucker:\n"));
    D (sinfo (si));

    offsetRect (&textrect, si->CLeft,
		(rectHeight (&gadgrect) - rectHeight (&textrect)) / 2);
    D (dumpRect ("textrect", &textrect));

    /* ----- clear around text area ----	*/
    fillAround (&clonerp, &gadgrect, (si->NumChars ? &textrect : NULL), bpen, ((UBYTE)~0));

    /*	------ Do Text ----	*/
    SetABPenDrMd (&clonerp, apen, bpen, JAM2);

    /* print the text itself		*/
    if (si->DispCount)
    {
	Move (&clonerp, textrect.MinX - te.te_Extent.MinX,
	      textrect.MinY - te.te_Extent.MinY);
	Text (&clonerp, &si->Buffer[si->DispPos], si->DispCount);
    }

    /*	------  set up Cursor Box  ------	*/
    cursor.Height = clonerp.TxHeight;
    if (si->NumChars)
    {
	cursor.Top = textrect.MinY;
	cursor.Left = textrect.MinX +
	    TextLength (&clonerp, &si->Buffer[si->DispPos],
			si->BufferPos - si->DispPos);
    }
    else
    {
	cursor.Top = gbox->Top +
	    (rectHeight (&gadgrect) - cursor.Height) / 2;
	cursor.Left = si->CLeft + gbox->Left;
    }
    D (printf ("cursor position: %ld/%ld\n",
	       cursor.Top, cursor.Left));

    /* cursor may be past end of string (should be safe with NumChars == 0) */
    if (!(cursor.Width = extrawidth))
    {
	TextExtent (&clonerp, &si->Buffer[effectiveBPos], 1, &te);
	cursor.Width = rectWidth (&te.te_Extent);
	D (printf ("cursor on char: %lx, width: %ld\n",
		   si->Buffer[effectiveBPos], cursor.Width));
    }
    D (
	  else
	  printf ("cursor off right: width: %ld\n", extrawidth));

    /* if gadget is active, turn on cursor */
    /* probably want to use some trickier cursor stuff, not XOR, to distinguish overstrike and insert mode */
    if (sg->Flags & SELECTED)
	boxModernize (xorbox, &clonerp, &cursor);

    ICloseFont (fp);		/* no-op if unused (NULL)	*/
}

/*****************************************************************************/

/* assumes you would only pass me a string gadget
 * ZZZ: when I do sgclass, this might become instance data */
struct StringExtend *getSEx (struct Gadget *g)
{
    return (((g->Flags & GFLG_STRINGEXTEND) || (g->Activation & GACT_STRINGEXTEND)) ?
	    ((struct StringInfo *) g->SpecialInfo)->Extension : 0);
}
