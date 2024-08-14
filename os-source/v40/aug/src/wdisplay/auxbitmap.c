/* auxbitmap.c
 *
 */

#include "wdisplay.h"

struct BitMap *AllocMyBitMap (UWORD depth, LONG width, LONG height)
{
    struct BitMap *bm;
    register WORD i;
    ULONG extra;

    if (GfxBase->lib_Version >= 39)
    {
	bm = (struct BitMap *) AllocBitMap (width, height, depth, (BMF_CLEAR | BMF_DISPLAYABLE), NULL);
    }
    else
    {
	extra = (ULONG) (depth > 8 ? depth - 8 : 0);
	if (bm = AllocVec (sizeof (struct BitMap) + (extra << 2), MEMF_CLEAR))
	{
	    /* Initialize the BitMap */
	    InitBitMap (bm, depth, width, height);

	    /* Allocate the rasters */
	    for (i = 0; i < depth; i++)
	    {
		if (!(bm->Planes[i] = (PLANEPTR) AllocRaster (width, height)))
		{
		    /* Free partial bitmap */
		    FreeMyBitMap (bm, width, height);

		    return (NULL);
		}
		BltClear (bm->Planes[i], (((ULONG)bm->Rows << 16L) | (ULONG)bm->BytesPerRow), 0x2L);
	    }
	}
    }

    return (bm);
}

VOID FreeMyBitMap (struct BitMap * bm, LONG width, LONG height)
{
    register UBYTE i;

    if (bm)
    {
	if (GfxBase->lib_Version >= 39)
	{
	    FreeBitMap (bm);
	}
	else
	{
	    /* Free each plane of the bitmap */
	    for (i = 0; i < bm->Depth; i++)
	    {
		if (bm->Planes[i])
		{
		    /* Free the raster */
		    FreeRaster (bm->Planes[i], width, height);
		    bm->Planes[i] = NULL;
		}
	    }
	    FreeVec (bm);
	}
    }
}

VOID TrimExtraPlanes (ILBM * ir, UWORD depth)
{

#if 0
    register struct BitMap *bm = ir->ir_BMap;
    register WORD i;

    if (bm->Depth > depth)
    {
	/* Free the unused planes */
	for (i = depth; i < bm->Depth; i++)
	{
	    /* Free the raster */
	    FreeRaster (bm->Planes[i], ir->ir_BMHD.w, ir->ir_BMHD.h);
	    bm->Planes[i] = NULL;
	}

	ir->ir_BMHD.nplanes = depth;
	bm->Depth = depth;
    }
#endif
}

BOOL AllocSecondaryBM (struct AppInfo * ai)
{
    if (ai->ai_BMapS)
    {
	/* Free the bitmap... */
	FreeMyBitMap (ai->ai_BMapS, ai->ai_WidthS, ai->ai_HeightS);

	/* Remember the new size */
	ai->ai_WidthS = ai->ai_Columns;
	ai->ai_HeightS = ai->ai_Rows;

	/* Allocate a bitmap w/rasters to hold the image */
	if (ai->ai_BMapS = AllocMyBitMap (ai->ai_Screen->BitMap.Depth, ai->ai_WidthS, ai->ai_HeightS))
	{
	    return (TRUE);
	}
    }

    return (FALSE);
}

VOID FreeSecondaryBM (struct AppInfo * ai)
{

    if (ai->ai_BMapS)
    {
	/* Free the BitMap */
	FreeMyBitMap (ai->ai_BMapS, ai->ai_WidthS, ai->ai_HeightS);
	ai->ai_BMapS = NULL;
    }
}

VOID FreeILBM (ILBM * ir)
{

    if (ir)
    {
	/* Free the BitMap */
	FreeMyBitMap (ir->ir_BMap, ir->ir_BMHD.w, ir->ir_BMHD.h);

	/* Free the name */
	FreeVec (ir->ir_Name);

	/* Free the ILBM record */
	FreeVec ((APTR) ir);
    }
}
