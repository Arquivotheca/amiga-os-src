/* misc.c
 * misc. for pointer
 *
 */

#include "pointer.h"

#define	DB(x)	;

/*****************************************************************************/

VOID UpdateSketch (EdDataPtr ed)
{
    if (ed->ed_GSketch)
    {
	/* Show that the bitmap has been updated */
	setgadgetattrs (ed, ed->ed_GSketch, SPA_Update, TRUE, TAG_DONE);
    }
}

/*****************************************************************************/

VOID SetPrefsRast (EdDataPtr ed, UWORD pen)
{
    SetAPen (&ed->ed_PrefsWork.ep_RPort, pen);
    RectFill (&ed->ed_PrefsWork.ep_RPort,
	      (ed->ed_Which * MAXWIDTH), 0,
	      (ed->ed_Which * MAXWIDTH) + MAXWIDTH - 1, MAXHEIGHT - 1);
}

/*****************************************************************************/

EdStatus allocpp (EdDataPtr ed, struct ExtPrefs * ep)
{
    EdStatus result = ES_NO_MEMORY;

    if (ep->ep_BMap = AllocBitMap ((MAXWIDTH * 2), MAXHEIGHT, ed->ed_Depth, BMF_DISPLAYABLE | BMF_STANDARD | BMF_CLEAR, NULL))
    {
	/* Initialize the rastport */
	InitRastPort (&ep->ep_RPort);
	ep->ep_RPort.BitMap = ep->ep_BMap;
	result = ES_NORMAL;
    }

    return result;
}

/*****************************************************************************/

UWORD ReMap (EdDataPtr ed, struct ExtPrefs * ep, WORD adj)
{
    register UWORD c, x, y;
    UWORD w, h;

    /* How wide do we want to be */
    w = MAXWIDTH * 2;
    h = MAXHEIGHT;

    /* Remap our image */
    for (y = 0; y < h; y++)
    {
	for (x = 0; x < w; x++)
	{
	    if ((c = ReadPixel (&ep->ep_RPort, x, y)) != 0)
	    {
		if (adj > 0)
		{
		    c = ed->ed_ColorTable[c];
		    DB (kprintf ("%2ld.", (ULONG)c));
		    SetAPen (&ep->ep_RPort, c);
		}
		else
		{
		    c = GetIndex (ed, c);
		    DB (kprintf ("%2ld.", (ULONG)c));
		    SetAPen (&ep->ep_RPort, c);
		}
		WritePixel (&ep->ep_RPort, x, y);
	    }
	    else
	    {
		DB (kprintf ("  ."));
	    }
	}
	DB (kprintf ("\n"));
    }
    return (0);
}
