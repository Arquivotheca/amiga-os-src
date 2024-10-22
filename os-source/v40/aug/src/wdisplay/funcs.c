/* funcs.c
 * Gadget functions
 */

#include "wdisplay.h"

VOID SetWindowTitle (struct AppInfo * ai, STRPTR title, LONG id)
{
    if (id)
    {
	title = GetWDisplayString (ai, id);
    }

#if DOBACKDROP
    if (ai->ai_Options[OPT_BACKDROP])
    {
	/* Set the title */
	SetWindowTitles (ai->ai_Window, NO_TITLE, title);
    }
    else
    {
#endif
	/* Set the title */
	SetWindowTitles (ai->ai_Window, title, NO_TITLE);

	/* Display the status bar */
	StatusFunc (ai);

#if DOBACKDROP
    }
#endif
}

LONG SetDisplayMode (struct AppInfo * ai)
{
    /* Position the view & the scroll bars */
    ai->ai_SWidth = ai->ai_Picture->ir_BMHD.w;
    ai->ai_SHeight = ai->ai_Picture->ir_BMHD.h;

    /* Reset the scaling factor to 1/1 */
    ai->ai_XFactor = ai->ai_YFactor = 100L;

    /* What do we do? */
    if (ai->ai_Flags & AIF_SCALE)
    {
	ai->ai_CurColumn = ai->ai_CurRow = 0;
    }
    /* Clear the newly exposed pieces */
    else if (((ai->ai_Picture->ir_BMHD.w < ai->ai_Columns) ||
	      (ai->ai_Picture->ir_BMHD.h < ai->ai_Rows)))
    {
	ClearDisplay (ai);
    }

    /* Clear the sliders */
    ai->ai_Flags |= AIF_UPDATE;

    /* Don't let a double-click happen easily */
    ai->ai_DSecs = ai->ai_DMics = 0;

    /* Update the status bar */
    StatusFunc (ai);

    /* Clear the scroll flag */
    ai->ai_Flags &= ~AIF_SCROLL;

    /* Clear the scroll pointer */
    ClearPointer (ai->ai_Window);

    return TRUE;
}

LONG FullSizeFunc (struct AppInfo * ai)
{
    LONG update = FALSE;
    LONG px, py;

    if (ai->ai_Flags & AIF_SCALE)
    {
	/* Are we sorta normal? */
	if (ai->ai_Flags & AIF_ZOOMED)
	{
	    /* Reset the scaling */
	    ai->ai_Flags |= AIF_CHANGED;
	}

	/* Turn on full view */
	ai->ai_Flags &= ~(AIF_SCALE | AIF_ZOOMED);

	/* Get the current pixel size */
	px = (ULONG) ai->ai_MouseX * ai->ai_XFactor / 100L;
	py = (ULONG) ai->ai_MouseY * ai->ai_YFactor / 100L;

	/* Set the modes */
	update = SetDisplayMode (ai);

	/* Figure out center */
	ai->ai_CurColumn = px - (ai->ai_Columns / 2);
	ai->ai_CurRow = py - (ai->ai_Rows / 2);
    }

    return update;
}

LONG ScaleSizeFunc (struct AppInfo * ai)
{
    LONG update = FALSE;

    if (!(ai->ai_Flags & AIF_SCALE) || (ai->ai_Flags & AIF_ZOOMED))
    {
	/* Are we sorta normal? */
	if (ai->ai_Flags & AIF_ZOOMED)
	{
	    /* Reset the scaling */
	    ai->ai_Flags |= AIF_CHANGED;
	}

	/* Turn on scaling */
	ai->ai_Flags &= ~AIF_ZOOMED;
	ai->ai_Flags |= AIF_SCALE;

	/* Set the modes */
	update = SetDisplayMode (ai);
    }

    return update;
}

LONG StatusFunc (struct AppInfo * ai)
{
    UWORD tx, ty;
    UBYTE apen;
    UBYTE bpen;
    WORD mlen;

#if DOBACKDROP
    if (!ai->ai_Options[OPT_BACKDROP])
    {
#endif
	if (ai->ai_Flags & AIF_SCALE)
	{
	    if (ai->ai_Flags & AIF_ZOOMED)
	    {
		strcpy (ai->ai_Buffer, " Zoom ");
	    }
	    else
	    {
		strcpy (ai->ai_Buffer, " Scale ");
	    }
	}
	else
	{
	    strcpy (ai->ai_Buffer, " Full ");
	}

	if (IntuitionBase->lib_Version >= 36)
	{
	    if (ai->ai_Window->Flags & WFLG_WINDOWACTIVE)
	    {
		apen = ai->ai_DI->dri_Pens[FILLTEXTPEN];
		bpen = ai->ai_DI->dri_Pens[FILLPEN];
	    }
	    else
	    {
		apen = ai->ai_DI->dri_Pens[TEXTPEN];
		bpen = ai->ai_DI->dri_Pens[BACKGROUNDPEN];
	    }
	}
	else
	{
	    apen = ai->ai_Window->DetailPen;
	    bpen = ai->ai_Window->BlockPen;
	}

	SetAPen (ai->ai_Window->RPort, apen);
	SetBPen (ai->ai_Window->RPort, bpen);
	SetDrMd (ai->ai_Window->RPort, JAM2);

	mlen = TextLength (ai->ai_Window->RPort, ai->ai_Buffer, strlen (ai->ai_Buffer));
	ty = ai->ai_Window->RPort->TxBaseline + 1;
	tx = ai->ai_Window->Width - SYSGAD_WIDTH - mlen;

	if (ai->ai_MLen > mlen)
	{
	    SetAPen (ai->ai_Window->RPort, bpen);

	    RectFill (ai->ai_Window->RPort,
		      ai->ai_Window->Width - SYSGAD_WIDTH - ai->ai_MLen, 1,
		      ai->ai_Window->Width - SYSGAD_WIDTH, ai->ai_Window->RPort->TxHeight + 1);

	    SetAPen (ai->ai_Window->RPort, apen);
	}
	ai->ai_MLen = mlen;

	Move (ai->ai_Window->RPort, tx, ty);
	Text (ai->ai_Window->RPort, ai->ai_Buffer, strlen (ai->ai_Buffer));

#if DOBACKDROP
    }
#endif

    return (0L);
}

LONG VScrollBarFunc (struct AppInfo * ai)
{
    register LONG vtop;
    register LONG vvis;
    register LONG vtot;
    ULONG tmp;

    vtop = 0;
    vvis = ai->ai_Rows;
    vtot = ai->ai_Picture->ir_BMHD.h;

    if (ai->ai_Flags & AIF_ZOOMED)
    {
	vvis = ai->ai_SHeight;
    }

    if ((tmp = (vtot - vvis)) > 0)
    {
	vtop = (tmp * (ULONG) ai->ai_PIV.VertPot + (0xffffL >> 1)) / 0xffffL;
    }

    ai->ai_CurRow = vtop;

    return (0L);
}

LONG HScrollBarFunc (struct AppInfo * ai)
{
    register LONG htop;
    register LONG hvis;
    register LONG htot;
    ULONG tmp;

    htop = 0;
    hvis = ai->ai_Columns;
    htot = ai->ai_Picture->ir_BMHD.w;

    if (ai->ai_Flags & AIF_ZOOMED)
    {
	hvis = ai->ai_SWidth;
    }

    if ((tmp = (htot - hvis)) > 0)
    {
	htop = (tmp * (ULONG) ai->ai_PIH.HorizPot + (0xffffL >> 1)) / 0xffffL;
    }

    ai->ai_CurColumn = htop;

    return (0L);
}
