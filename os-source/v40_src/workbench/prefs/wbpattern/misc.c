/* misc.c
 * misc. for pointer
 *
 */

#include "wbpattern.h"

/*****************************************************************************/

VOID SetPrefsRast (EdDataPtr ed, UWORD pen)
{
    SetAPen (&(ed->ed_PrefsWork.ep_RPort), pen);
    RectFill (&(ed->ed_PrefsWork.ep_RPort),
	      ed->ed_Which, 0,
	      ed->ed_Which + ed->ed_Width - 1, ed->ed_Height - 1);
}

/*****************************************************************************/

BOOL allocplanes (EdDataPtr ed, struct BitMap * bm, PLANEPTR * planes, UWORD width)
{
    register WORD i;

    /* Allocate the rasters */
    for (i = 0; i < bm->Depth; i++)
    {
	if (i < 3)
	{
	    if (!(planes[i] = (PLANEPTR) AllocRaster (width, bm->Rows)))
	    {
		return (FALSE);
	    }
	    BltClear (planes[i], (((ULONG) bm->Rows << 16L) | (ULONG) bm->BytesPerRow), 0x2L);
	}
	else
	{
	    planes[i] = planes[2];
	}
    }
    return (TRUE);
}

/*****************************************************************************/

VOID freeplanes (EdDataPtr ed, struct BitMap * bm, PLANEPTR * planes, UWORD width)
{
    register WORD i;

    /* Allocate the rasters */
    for (i = 0; i < 3; i++)
    {
	if (planes[i])
	{
	    FreeRaster (planes[i], width, bm->Rows);
	    planes[i] = NULL;
	}
    }
}

/*****************************************************************************/

BOOL allocbitmap (EdDataPtr ed, struct BitMap * bm, UWORD depth, UWORD width, UWORD height)
{
    /* Initialize the BitMap */
    InitBitMap (bm, depth, width, height);

    /* Allocate the rasters */
    return (allocplanes (ed, bm, bm->Planes, width));
}

/*****************************************************************************/

VOID freebitmap (EdDataPtr ed, struct BitMap * bm, UWORD width, UWORD height)
{
    freeplanes (ed, bm, bm->Planes, width);
}

/*****************************************************************************/

EdStatus allocpp (EdDataPtr ed, struct ExtPrefs * pp)
{
    EdStatus result = ES_NO_MEMORY;

    if (allocbitmap (ed, &pp->ep_BMap, ed->ed_Depth, (ed->ed_Width * 3) + 2, ed->ed_Height))
    {
	/* Initialize the rastport */
	InitRastPort (&(pp->ep_RPort));
	pp->ep_RPort.BitMap = &pp->ep_BMap;

	result = ES_NORMAL;
    }

    return result;
}
