/* funcs.c
 *
 */

#include "graffiti.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

void ClearFunc (struct GlobalData * gd, UBYTE pen)
{
    /* Make sure we have the RastPort initialized correctly. */
    gd->gd_RPort.BitMap = gd->gd_BitMap;

    /* Clear it */
    SetRast (&gd->gd_RPort, pen);

    BltBitMapRastPort (gd->gd_BitMap, gd->gd_HTop, gd->gd_VTop,
		       gd->gd_Window->RPort,
		       gd->gd_Window->BorderLeft, gd->gd_Window->BorderTop,
		       gd->gd_InnerWidth, gd->gd_InnerHeight, 0xC0);

    /* Refresh the visuals */
    NewSizeFunc (gd);
}

/*****************************************************************************/

void NewSizeFunc (struct GlobalData * gd)
{
    struct Rectangle rect;

    /* Compute the inside dimensions */
    gd->gd_InnerWidth = gd->gd_Window->Width - (gd->gd_Window->BorderLeft + gd->gd_Window->BorderRight);
    gd->gd_InnerHeight = gd->gd_Window->Height - (gd->gd_Window->BorderTop + gd->gd_Window->BorderBottom);

    /* Set the scroll bars */
    SetViewWindowAttrs (gd->gd_WinClass, gd->gd_Window,
			WOA_VisHoriz, gd->gd_InnerWidth,
			WOA_TotHoriz, gd->gd_Width,
			WOA_VisVert, gd->gd_InnerHeight,
			WOA_TotVert, gd->gd_Height,
			TAG_DONE);

    if (gd->gd_Region)
    {
	InstallClipRegion (gd->gd_Window->RPort->Layer, gd->gd_ORegion);
	DisposeRegion (gd->gd_Region);
    }

    if (gd->gd_Region = NewRegion ())
    {
	rect.MinX = gd->gd_Window->BorderLeft;
	rect.MinY = gd->gd_Window->BorderTop;
	rect.MaxX = gd->gd_Window->Width - gd->gd_Window->BorderRight - 1;
	rect.MaxY = gd->gd_Window->Height - gd->gd_Window->BorderBottom - 1;

	OrRectRegion (gd->gd_Region, &rect);
	DB (kprintf ("Window=%08lx WLayer=%08lx RPort->Layer=%08ld\n", gd->gd_Window, gd->gd_Window->WLayer, gd->gd_Window->RPort->Layer));
	gd->gd_ORegion = InstallClipRegion (gd->gd_Window->RPort->Layer, gd->gd_Region);
    }
}
