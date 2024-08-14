/* bitmap.c
 */

#include "wdisplay.h"

#define	DB(x)	;
#define	DI(x)	;
#define	DD(x)	;

VOID ClearDisplay (struct AppInfo * ai)
{
    SetAPen (ai->ai_Window->RPort, 0);

    RectFill (ai->ai_Window->RPort,
	      ai->ai_Window->BorderLeft,
	      ai->ai_Window->BorderTop,
	      ai->ai_Window->Width - ai->ai_Window->BorderRight - 1,
	      ai->ai_Window->Height - ai->ai_Window->BorderBottom - 1);
}

/* Scale the image to fit the display area */
BOOL ScaleImage (struct AppInfo * ai)
{
    ILBM *ir;

    if ((ir = ai->ai_Picture) && ai->ai_BMapS)
    {
	/* Status message showing that we are attempting to scale the picture */
	SetWaitPointer (ai->ai_Window);
	if (ai->ai_Flags & AIF_ZOOMED)
	{
	    SetWindowTitle (ai, NULL, TITLE_ZOOMING);
	}
	else
	{
	    SetWindowTitle (ai, NULL, TITLE_SCALING);
	}

	/* Has the work bitmap changed? */
	if (ai->ai_Flags & AIF_CHANGED)
	{
	    /* Scale the source bitmap to the work bitmap */
	    ScaleBitMap (ai);

	    /* Clear the change flag */
	    ai->ai_Flags &= ~AIF_CHANGED;
	}

	/* Blit from the work BitMap to the window */
	BltBitMapRastPort (ai->ai_BMapS,
			   0,
			   0,
			   ai->ai_Window->RPort,
			   ai->ai_Window->BorderLeft,
			   ai->ai_Window->BorderTop,
#if 1
			   MIN ((ai->ai_Window->Width - ai->ai_Window->BorderRight - ai->ai_Window->BorderLeft), ai->ai_Columns),
			   MIN ((ai->ai_Window->Height - ai->ai_Window->BorderBottom - ai->ai_Window->BorderTop), ai->ai_Rows),
#else
			   ai->ai_Columns,
			   ai->ai_Rows,
#endif
			   0xC0);

	/* Show the title of the picture */
	SetWindowTitle (ai, ai->ai_ProjName, 0L);
	ClearPointer (ai->ai_Window);

	return TRUE;
    }

    return FALSE;
}

VOID UpdateDisplay (struct AppInfo * ai)
{
    register LONG vtop;
    register LONG vvis;
    register LONG vtot;
    register LONG htop;
    register LONG hvis;
    register LONG htot;
    BOOL vret = FALSE;
    BOOL hret = FALSE;
    WORD diffysize;
    WORD diffxsize;
    UWORD height;
    UWORD width;
    UWORD body;
    UWORD pot;

    if (ai->ai_Picture)
    {
	htop = ai->ai_CurColumn;
	hvis = ai->ai_Columns;
	htot = ai->ai_Picture->ir_BMHD.w;

	vtop = ai->ai_CurRow;
	vvis = ai->ai_Rows;
	vtot = ai->ai_Picture->ir_BMHD.h;

	ai->ai_Flags &= ~AIF_MAX;
	if (!(ai->ai_Flags & AIF_ZOOMED) && (vvis == vtot) && (hvis == htot))
	{
	    ai->ai_Flags |= AIF_MAX;
	}

	ai->ai_XFactor = ai->ai_YFactor = 100;
	if (ai->ai_Flags & AIF_SCALE)
	{
	    /* Compute the scaling factor */
	    ai->ai_XFactor = (ULONG) (htot * 100) / hvis;
	    ai->ai_YFactor = (ULONG) (vtot * 100) / vvis;

	    if (ai->ai_Flags & AIF_ZOOMED)
	    {
		hvis = ai->ai_SWidth;
		vvis = ai->ai_SHeight;
	    }
	    else
	    {
		htot = hvis;
		vtot = vvis;
	    }
	}

	diffxsize = htot - hvis;
	diffysize = vtot - vvis;

	if (vtop > (vtot - vvis))
	{
	    vtop = vtot - vvis;
	}

	if (htop > (htot - hvis))
	{
	    htop = htot - hvis;
	}

	if (vtop < 0)
	{
	    vtop = 0;
	}

	if (htop < 0)
	{
	    htop = 0;
	}

	if (ai->ai_Flags & AIF_UPDATE)
	{
	    ai->ai_OCurRow = ai->ai_OCurColumn = ~0;
	    ai->ai_Flags &= ~AIF_REFRESH;
	}
	else
	{
	    if (vtop == ai->ai_OCurRow)
	    {
		vret = TRUE;
	    }

	    if (htop == ai->ai_OCurColumn)
	    {
		hret = TRUE;
	    }

	    if (hret && vret)
	    {
		ai->ai_CurRow = vtop;
		ai->ai_CurColumn = htop;
		return;
	    }
	}

#if DOBACKDROP
	if (!(ai->ai_Flags & AIF_REFRESH) && !(ai->ai_Options[OPT_BACKDROP]))
#else
	if (!(ai->ai_Flags & AIF_REFRESH))
#endif
	{
	    /* Adjust the vertical slider */
	    if (ai->ai_OCurRow != vtop)
	    {
		pot = 0;
		body = MAXBODY;
		if (diffysize > 0L)
		{
		    pot = (UWORD) ((0xffffL * vtop + (diffysize >> 1L)) / diffysize);
		    body = (UWORD) ((0xffffL * vvis) / vtot);
		}
		NewModifyProp (&ai->ai_Gads[2], ai->ai_Window, NULL, ai->ai_PVFlags,
			       0, pot, MAXBODY, body, 1L);
	    }

	    /* Adjust the horizontal slider */
	    if (ai->ai_OCurColumn != htop)
	    {
		pot = 0;
		body = MAXBODY;
		if (diffxsize > 0L)
		{
		    pot = (UWORD) ((0xffffL * htop + (diffxsize >> 1L)) / diffxsize);
		    body = (UWORD) ((0xffffL * hvis) / htot);
		}
		NewModifyProp (&ai->ai_Gads[5], ai->ai_Window, NULL, ai->ai_PHFlags,
			       pot, 0, body, MAXBODY, 1L);
	    }
	}

	if ((ai->ai_Flags & AIF_SCALE) && !(ai->ai_Flags & AIF_MAX))
	{
	    ScaleImage (ai);
	}
	else
	{
	    height = MIN (vtot - vtop, vvis);
	    width = MIN (htot - htop, hvis);

	    WaitTOF ();
	    BltBitMapRastPort (ai->ai_Picture->ir_BMap,
			       htop,
			       vtop,
			       ai->ai_Window->RPort,
			       ai->ai_Window->BorderLeft,
			       ai->ai_Window->BorderTop,
#if 1
			       MIN ((ai->ai_Window->Width - ai->ai_Window->BorderRight - ai->ai_Window->BorderLeft), width),
			       MIN ((ai->ai_Window->Height - ai->ai_Window->BorderBottom - ai->ai_Window->BorderTop), height),
#else
			       width,
			       height,
#endif
			       0xC0);
	}

	/* Clear the refresh bit */
	ai->ai_Flags &= ~(AIF_REFRESH | AIF_UPDATE);

	ai->ai_OCurColumn = ai->ai_CurColumn = htop;
	ai->ai_OCurRow = ai->ai_CurRow = vtop;
    }
}
