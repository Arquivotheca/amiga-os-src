/* loadpicture.c
 *
 */

#include "wdisplay.h"

/* 6 7 3 */
#define	RED_SCALE	6
#define	GREEN_SCALE	7
#define	BLUE_SCALE	3

static ULONG color_error (ULONG r1, ULONG r2, ULONG g1, ULONG g2, ULONG b1, ULONG b2)
{

    r1 >>= 24;
    g1 >>= 24;
    b1 >>= 24;

    r2 >>= 24;
    g2 >>= 24;
    b2 >>= 24;

    return (SQ (r1 - r2) + SQ (g1 - g2) + SQ (b1 - b2));
}

static LONG averagergb (LONG rgb0, LONG rgb1)
{
    register LONG r, g, b;

    r = (((rgb0 >> 8) & 15) + ((rgb1 >> 8) & 15)) >> 1;
    g = (((rgb0 >> 4) & 15) + ((rgb1 >> 4) & 15)) >> 1;
    b = ((rgb0 & 15) + (rgb1 & 15)) / 2;
    return ((r << 8) | (g << 4) | (b));
}

static LONG colordifference (LONG r1, LONG g1, LONG b1, LONG rgb1)
{
    register LONG r, g, b;

    r = SQ (r1 - ((rgb1 >> 8) & 15)) * RED_SCALE;
    g = SQ (g1 - ((rgb1 >> 4) & 15)) * GREEN_SCALE;
    b = SQ (b1 - (rgb1 & 15)) * BLUE_SCALE;
    return (r + g + b);
}

static LONG pickbestcolor (LONG colors[], LONG num, LONG r1, LONG g1, LONG b1)
{
    LONG difference;
    LONG i, j;

    for (j = 0; j < 5000; j++)
    {
	for (i = 0; i < num; i++)
	{
	    if ((difference = colordifference (r1, g1, b1, colors[i])) == j)
	    {
		return (i);
	    }
	}
    }

    return (0);
}

VOID LoadPicture (struct AppInfo * ai, BPTR dir, STRPTR name)
{
    struct Window *win = ai->ai_Window;
    struct Screen *scr = win->WScreen;
    struct ViewPort *vp = &(scr->ViewPort);
    struct ColorMap *cm = vp->ColorMap;
    LONG testr, testg, testb;
    BOOL remap = FALSE;
    LONG numcolors;
    UBYTE odepth;
    LONG depth;
    LONG i, j;
    LONG err1;
    LONG err2;
    LONG got;
    ILBM *ir;

    /* Clear the flags */
    ai->ai_Flags &= ~(AIF_SECONDARY | AIF_SCALE | AIF_ZOOMED);

    if (ai->ai_Options[OPT_SCALE])
    {
	/* Set the scale flag */
	ai->ai_Flags |= AIF_SCALE;
    }

    /* Status message showing that we are attempting to load the picture */
    SetWaitPointer (win);
    SetWindowTitle (ai, NULL, TITLE_LOADING);

    /* Do we have a picture? */
    if (ai->ai_Picture)
    {
#if AA
	for (i = 0; i < ai->ai_Picture->ir_NumAlloc; i++)
	{
	    FreePalette (win->WScreen->ViewPort.ColorMap, ai->ai_Picture->ir_Allocated[i]);
	}
	ai->ai_Picture->ir_NumAlloc = 0;
#endif

	/* Free the old picture */
	FreeILBM (ai->ai_Picture);

	/* Clear the project name */
	strcpy (ai->ai_ProjName, "");

	/* The picture that we no longer have */
	ClearDisplay (ai);
    }

    /* Clear the scaling */
    ai->ai_XFactor = ai->ai_YFactor = 100L;

    /* Clear the sliders */
    ai->ai_OCurColumn = ai->ai_OCurRow = ~0;

    /* Get the current screen depth */
    depth = scr->RastPort.BitMap->Depth;
    numcolors = 1 << depth;

    /* Load the ILBM */
    if (ai->ai_Picture = ir = ReadILBM (dir, name, depth))
    {
#if TRUECOLOR
	/* True color */
	if (ir->ir_BMHD.nplanes == 24)
	{
	    TrimTrueColor (ai);
	}
#endif

	if (!ai->ai_Options[OPT_NOREMAP])
	{
	    /* Show that we are busy remapping the bitmap */
	    SetWindowTitle (ai, NULL, TITLE_CALCULATING);

#if AA
	    if (GfxBase->lib_Version < 39)
	    {
#endif
		/* Get the current screen colors */
		for (i = 0; i < numcolors; i++)
		{
		    ai->ai_Colors[i] = GetRGB4 (cm, (LONG) i);
		}
#if AA
	    }
#endif
	    /* Color match */
	    for (i = 0; i < ir->ir_NumColors; i++)
	    {
#if AA
		if (GfxBase->lib_Version >= 39)
		{
		    got = PickBestColor (cm, ir->ir_CRegs[i][0], ir->ir_CRegs[i][1], ir->ir_CRegs[i][2], 0, -1);
		    GetRGB32 (cm, got, 1, &(ir->ir_GRegs[ir->ir_NumAlloc][0]));
		    ir->ir_Allocated[ir->ir_NumAlloc++] = got;
		}
		else
		{
#endif
		    /* Get the closest color */
		    got = pickbestcolor (ai->ai_Colors, numcolors, ir->ir_CRegs[i][0], ir->ir_CRegs[i][1], ir->ir_CRegs[i][2]);

		    /* Show what color we are using */
		    ir->ir_GRegs[i][0] = (ai->ai_Colors[got] >> 8) & 15;
		    ir->ir_GRegs[i][1] = (ai->ai_Colors[got] >> 4) & 15;
		    ir->ir_GRegs[i][2] = ai->ai_Colors[got] & 15;
#if AA
		}
#endif

		if (got != i)
		{
		    remap = TRUE;
		}

		/* Set up the remap table */
		ir->ir_ColorTable[i] = got;
		ir->ir_ColorTable2[i] = got;
	    }

	    if (remap)
	    {
		for (i = 0; i < ir->ir_NumColors; i++)
		{
		    err1 = color_error (
				       ir->ir_CRegs[i][0], ir->ir_GRegs[i][0],
				       ir->ir_CRegs[i][1], ir->ir_GRegs[i][1],
				      ir->ir_CRegs[i][2], ir->ir_GRegs[i][2]);

		    if (err1 > 2000)
		    {
			for (j = 0; j < ir->ir_NumAlloc; j++)
			{
			    testr = AVGC (i, j, 0);
			    testg = AVGC (i, j, 1);
			    testb = AVGC (i, j, 2);

			    err2 = color_error (
						   ir->ir_CRegs[i][0], testr,
						   ir->ir_CRegs[i][1], testg,
						   ir->ir_CRegs[i][2], testb);

			    if (err2 < err1)
			    {
				err1 = err2;
				ir->ir_ColorTable2[i] = ir->ir_Allocated[j];
			    }
			}
		    }
		}

		/* Show that we are busy remapping the bitmap */
		SetWindowTitle (ai, NULL, TITLE_REMAPPING);

		/* The remap function can't handle anything over 8 bitplanes */
		odepth = ir->ir_BMap->Depth;
		ir->ir_BMap->Depth = MIN (odepth, 8);

		/* Remap the bitmap */
		XLateBitMap (ir->ir_BMap, ir->ir_BMap,
			     ir->ir_ColorTable, ir->ir_ColorTable2,
			     ir->ir_BMHD.w);

		/* Restore the depth */
		ir->ir_BMap->Depth = odepth;
	    }

	    /* Get rid of unused planes */
	    TrimExtraPlanes (ir, depth);
	}

	/* Allocate the secondary bitmap */
	if (AllocSecondaryBM (ai))
	{
	    ai->ai_Flags |= AIF_SECONDARY;
	}

	/* Reset the scaling */
	ai->ai_SWidth = ir->ir_BMHD.w;
	ai->ai_SHeight = ir->ir_BMHD.h;

	/* Do we have a name within the ILBM? */
	if (ir->ir_Name)
	{
	    /* Update the project name */
	    strcpy (ai->ai_ProjName, ir->ir_Name);
	}
	else
	{
	    /* Update the project name */
	    strcpy (ai->ai_ProjName, name);
	}

	/* Update the flags */
	ai->ai_Flags |= (AIF_CHANGED | AIF_UPDATE);

	/* Bring in the new picture */
	UpdateDisplay (ai);
    }
    else
    {
	/* Error in loading the ILBM */
	DisplayBeep (win->WScreen);
    }

    /* See if they specified a title */
    if (ai->ai_Options[OPT_TITLE])
    {
	/* Copy the title over */
	strcpy (ai->ai_ProjName, (STRPTR) ai->ai_Options[OPT_TITLE]);
    }

    /* Refresh the window title */
    SetWindowTitle (ai, ai->ai_ProjName, 0L);

    /* Update the about window */
    UpdateAboutWindow (ai);

    /* Get rid of the busy pointer */
    ClearPointer (win);
}
