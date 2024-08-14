/* drawbox.c
 *
 */

#include "amigaguidebase.h"

/*****************************************************************************/

VOID DrawBox (struct AGLib *ag, Class *cl, Object *o, struct RastPort *rp, struct IBox *box, struct IBox *sel, LONG status)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct ClientData *cd = INST_DATA (cl, o);
    LONG lx = box->Left + sel->Left - si->si_TopHoriz;
    LONG ly = cd->cd_Top + ((sel->Top - si->si_TopVert) * si->si_VertUnit);
    LONG x = box->Left + sel->Width - si->si_TopHoriz;
    LONG y = cd->cd_Top + ((sel->Height - si->si_TopVert) * si->si_VertUnit);
    LONG h = ABS (y - ly);
    LONG ty = MIN (ly, y);
    LONG by = MAX (ly, y);
    LONG width, height;

    LONG tx, bx;

    SetDrMd (rp, COMPLEMENT);
    SetAPen (rp, ~0);
    rp->Mask = 0x1;

    width = box->Left + box->Width - 1;
    height = cd->cd_Top + box->Height - 1;

    if (h <= 1)
    {
	if (x <= lx)
	{
	    tx = x;
	    bx = lx + si->si_HorizUnit - 1;
	}
	else
	{
	    tx = lx;
	    bx = x - 1;
	}
	tx = MAX (tx, box->Left);
	bx = MIN (bx, width);
	RectFill (rp, tx, y, bx, y + si->si_VertUnit - 1);
    }
    else if ((by >= cd->cd_Top) && (ty <= height))
    {
	if (h > si->si_VertUnit)
	{
	    ty = MAX (ty, cd->cd_Top - si->si_VertUnit);
	    by = MIN (by, height);
	    RectFill (rp,
		      box->Left, ty + si->si_VertUnit,
		      width, by - 1);
	}

	if (y < ly)
	{
	    bx = MIN (lx + si->si_HorizUnit - 1, width);
	    if (y >= cd->cd_Top)
		RectFill (rp, MAX (x, box->Left), y, width, y + si->si_VertUnit - 1);
	    RectFill (rp, box->Left, ly, bx, ly + si->si_VertUnit - 1);
	}
	else
	{
	    tx = MAX (lx, box->Left);
	    bx = MIN (x - 1, width);

	    RectFill (rp, tx, ly, width, ly + si->si_VertUnit - 1);
	    if (y + si->si_VertUnit - 1 < height)
		RectFill (rp, box->Left, y, bx, y + si->si_VertUnit - 1);
	}
    }
    rp->Mask = 0xFF;
}
