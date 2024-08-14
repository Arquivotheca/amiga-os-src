/* mask.c
 *
 */

#include "dtdesc.h"

#define	DB(x)	;

/*****************************************************************************/

UWORD dither[] = {0xAAAA, 0x5555};

/*****************************************************************************/

BOOL FindPoint (struct AppInfo *ai)
{
    WORD px, py;
    BOOL draw;
    WORD x, y;
    WORD w, h;
    WORD r, c;
    WORD i;

    x = ai->ai_IMsg->MouseX - (ai->ai_ViewBox.Left + 4);
    y = ai->ai_IMsg->MouseY - (ai->ai_ViewBox.Top + 2);

    if ((x > 0) && (x < ai->ai_ViewBox.Width - 8) &&
        (y > 0) && (y < ai->ai_ViewBox.Height - 4) &&
	(ai->ai_Files.lh_TailPred != (struct Node *) &ai->ai_Files))
    {
	r = 4;
	c = ai->ai_BufSize / r;
	w = (ai->ai_ViewBox.Width - 8) / c + 1;
	h = (ai->ai_ViewBox.Height - 4) / r + 1;

	px = x / w;
	py = y / h;

	i = (py * c) + px;

	if (!(ai->ai_Flags & AIF_DOWN))
	{
	    ai->ai_Flags |= AIF_DOWN;
	    if (ai->ai_DBuffer[i] != (-1))
	    {
		ai->ai_Flags |= AIF_SET;
	    }
	}

	draw = FALSE;
	if (ai->ai_Flags & AIF_SET)
	{
	    if (ai->ai_DBuffer[i] != (-1))
	    {
		ai->ai_DBuffer[i] = (-1);
		draw = TRUE;
	    }
	}
	else
	{
	    if (ai->ai_DBuffer[i] != ai->ai_SBuffer[i])
	    {
		ai->ai_DBuffer[i] = ai->ai_SBuffer[i];
		draw = TRUE;
	    }
	}

	if (draw)
	{
	    DrawPixel (ai, ai->ai_DBuffer[i], (ai->ai_ViewBox.Left + 4) + (px * w), (ai->ai_ViewBox.Top + 2) + (py * h), w, h);
	}

	return (TRUE);
    }

    return (FALSE);
}

/*****************************************************************************/

VOID DrawPixel (struct AppInfo *ai, WORD ch, WORD tx, WORD ty, WORD w, WORD h)
{
    UBYTE buff[4];
    WORD ofs = 0;

    SetAPen (ai->ai_Window->RPort, 0);
    RectFill (ai->ai_Window->RPort, tx, ty, tx + w - 2, ty + h - 2);

    if (ch != (-1))
    {
	SetAPen (ai->ai_Window->RPort, 1);
    }
    else
    {
	/* Turn on the special ghosting pattern */
	SetAPen (ai->ai_Window->RPort, 3);
	SetAfPt (ai->ai_Window->RPort, dither, 1);
    }

    if ((ai->ai_Flags & (AIF_HEX | AIF_TEXT)) && (ch != (-1)))
    {
	if (ai->ai_Flags & AIF_TEXT)
	{
	    DB (kprintf ("AIF_TEXT\n"));
	    sprintf (buff, "%lc", (LONG)ch);
	    ofs = 4;
	}
	else
	{
	    DB (kprintf ("AIF_HEX\n"));
	    sprintf (buff, "%02lx", (LONG)ch);
	}

	Move (ai->ai_Window->RPort, tx + ofs, ty + h - 4);
	Text (ai->ai_Window->RPort, buff, strlen (buff));
    }
    else
    {
	DB (kprintf ("AIF_GRAPHIC\n"));
	RectFill (ai->ai_Window->RPort, tx, ty, tx + w - 2, ty + h - 2);
    }

    /* Turn off the special ghosting pattern */
    SetAfPt (ai->ai_Window->RPort, NULL, 0);
}

/*****************************************************************************/

VOID ShowMask (struct AppInfo *ai)
{
    WORD stx, sty;
    WORD tx, ty;
    WORD cr, cc;
    WORD r, c;
    WORD w, h;
    WORD i;

    tx = stx = ai->ai_ViewBox.Left + 4;
    ty = sty = ai->ai_ViewBox.Top + 2;

    r = 4;
    c = ai->ai_BufSize / r;
    w = (ai->ai_ViewBox.Width - 8) / c + 1;
    h = (ai->ai_ViewBox.Height - 4) / r + 1;

    for (i = cr = cc = 0; i < ai->ai_BufSize; i++)
    {
        DrawPixel (ai, ai->ai_DBuffer[i], tx, ty, w, h);
        cc++;
        tx += w;
        if (cc >= c)
        {
            cc = 0;
            tx = stx;
            ty += h;
        }
    }

    SetAppAttrs (ai);
}
