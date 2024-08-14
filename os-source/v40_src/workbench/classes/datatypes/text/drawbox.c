/* drawbox.c
 *
 */

#include "textbase.h"

#define	DB(x)	;

/*****************************************************************************/

VOID DrawBox (struct TextLib * txl, Class * cl, Object * o, struct RastPort * rp, struct IBox * box, struct IBox * sel, LONG status)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct localData *lod = INST_DATA (cl, o);
    LONG topY, bottomY;
    LONG width, height;
    LONG lastX, lastY;
    LONG curX, curY;
    LONG numLines;
    LONG tx, bx;
    LONG i, l;

    if (status == DBS_SCROLL1)
	return;

    if (status == DBS_SCROLL2)
	return;

    /* Initialize drawing area */
    SetDrMd (rp, COMPLEMENT);
    SetAPen (rp, ~0);
    rp->Mask = 0x1;

    /* Initialize coordinates */
    curX     = box->Left + sel->Width - si->si_TopHoriz;
    curY     = box->Top + ((sel->Height - si->si_TopVert) * si->si_VertUnit);
    lastX    = box->Left + sel->Left - si->si_TopHoriz;
    lastY    = box->Top + ((sel->Top - si->si_TopVert) * si->si_VertUnit);
    numLines = ABS (curY - lastY);
    topY     = MIN (lastY, curY);
    bottomY  = MAX (lastY, curY);
    width    = box->Left + box->Width - 1;
    height   = box->Top + box->Height - 1;

    /* Only one line? */
    if (numLines <= 1)
    {
	if (curX <= lastX)
	{
	    tx = curX;
	    bx = lastX + si->si_HorizUnit - 1;
	}
	else
	{
	    tx = lastX;
	    bx = curX - 1;
	}
	tx = MAX (tx, box->Left);
	bx = MIN (bx, width);
	RectFill (rp, tx, curY, bx, curY + si->si_VertUnit - 1);

	topY    = curY - si->si_VertUnit;
	bottomY = curY + 1;
    }
    else if ((bottomY >= box->Top) && (topY <= height))
    {
	if (status != DBS_MOVE)
	{
	    RectFill (rp, box->Left, topY + si->si_VertUnit, width, bottomY - 1);
	}

	if (curY < lastY)
	{
	    bx = MIN (lastX + si->si_HorizUnit - 1, width);
	    if (curY >= box->Top)
		RectFill (rp, MAX (curX, box->Left), curY, width, curY + si->si_VertUnit - 1);
	    RectFill (rp, box->Left, lastY, bx, lastY + si->si_VertUnit - 1);
	}
	else
	{
	    tx = MAX (lastX, box->Left);
	    bx = MIN (curX - 1, width);

	    if (curY + si->si_VertUnit - 1 < height)
		RectFill (rp, box->Left, curY, bx, curY + si->si_VertUnit - 1);
	    RectFill (rp, tx, lastY, width, lastY + si->si_VertUnit - 1);
	}

	topY    = MAX (topY, box->Top - si->si_VertUnit);
	bottomY = MIN (bottomY, height);
    }

    if (status == DBS_MOVE)
    {
	for (i = 0; i < si->si_TotVert; i++)
	{
	    l = box->Top + (i * si->si_VertUnit);
	    if ((l >= topY + si->si_VertUnit) && (l < bottomY - 1))
	    {
		if (lod->lod_Selected[i] == 0)
		{
		    RectFill (rp, box->Left, l, width, l + si->si_VertUnit - 1);
		    lod->lod_Selected[i] = 1;
		}
	    }
	    else
	    {
		if (lod->lod_Selected[i] == 1)
		{
		    RectFill (rp, box->Left, l, width, l + si->si_VertUnit - 1);
		    lod->lod_Selected[i] = 0;
		}
	    }
	}
    }
    else
    {
	memset (lod->lod_Selected, 0, (sizeof (ULONG) * (si->si_VisVert + 1)));
    }

    rp->Mask = 0xFF;
}
