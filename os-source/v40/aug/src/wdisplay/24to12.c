/* 24to12.c
 *
 */

#include "wdisplay.h"

#if TRUECOLOR

#if 1
/*             1          2    */
/*  01234567 89012345 67890123 */
/*  RRRRRRRR GGGGGGGG BBBBBBBB */
/*       RRR      GGG       BB */

static WORD s_table[] =
{
    7,6,5,
    15,14,13,
    23,22
};

static WORD f_table[] =
{
     0, 1, 2, 3, 4,
     8, 9,10,11,12,
    16,17,18,19,20,21
};
#else
/*             1          2    */
/*  01234567 89012345 67890123 */
/*  RRRRRRRR GGGGGGGG BBBBBBBB */
/*  RRR      GGG      BB */

static WORD s_table[] =
{
    0,1,2,
    8,9,10,
    16,17
};

static WORD f_table[] =
{
     3, 4, 5, 6, 7,
    11,12,13,14,15,
    18,19,20,21,22,23
};
#endif

static VOID ConvertTrueColor (struct BitMap * bm, LONG width, LONG height)
{
    register WORD i;

    /* Free the unused planes */
    for (i = 0; i < 16; i++)
    {
	/* Free the raster */
	FreeRaster (bm->Planes[f_table[i]], width, height);
    }

    /* Swap in the new pointers */
    for (i = 0; i < 8; i++)
    {
	bm->Planes[i] = bm->Planes[s_table[i]];
    }

    /* Clear the old pointers */
    for (i = 8; i < 24; i++)
    {
	bm->Planes[i] = NULL;
    }

    bm->Depth = 8;
}

VOID TrimTrueColor (struct AppInfo *ai)
{
    ILBM *ir = ai->ai_Picture;
    WORD i, r, g, b;

    /* Show that we are busy remapping the bitmap */
    SetWindowTitle (ai, NULL, TITLE_CONVERTING_TRUE_COLOR);

    /* Construct a color map */
    ir->ir_NumColors = 256;
    for (i = 0; i < ir->ir_NumColors; i++)
    {
	b = i & 3;
	g = (i >> 2) & 7;
	r = (i >> 5) & 7;

#if AA
	if (GfxBase->lib_Version >= 39)
	{
	    ir->ir_CRegs[i][0] = (LONG) r << 30;
	    ir->ir_CRegs[i][1] = (LONG) g << 29;
	    ir->ir_CRegs[i][2] = (LONG) b << 29;

	}
	else
	{
#endif
	    ir->ir_CRegs[i][0] = (LONG) r << 2;
	    ir->ir_CRegs[i][1] = (LONG) g << 1;
	    ir->ir_CRegs[i][2] = (LONG) b << 1;
#if AA
	}
#endif

	/* For LoadRGB4() */
	if (i < 32)
	{
	    ir->ir_RGB4[i] = (r << 8) | (g << 4) | (b);
	}
    }

    /* Convert the bitmap */
    ConvertTrueColor (ir->ir_BMap, ir->ir_BMHD.w, ir->ir_BMHD.h);

    /* Set the size */
    ir->ir_BMHD.nplanes = 8;
}
#endif
