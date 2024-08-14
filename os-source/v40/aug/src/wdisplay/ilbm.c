/* ilbm.c --- read an IFF ILBM file into a record.
 */

#include "wdisplay.h"

#ifndef	MakeID
/* Four-character IDentifier builder.*/
#define MakeID(a,b,c,d)  ( (LONG)(a)<<24L | (LONG)(b)<<16L | (c)<<8 | (d) )
#endif

#ifndef	ID_NAME
#define ID_NAME      MakeID('N', 'A', 'M', 'E')
#endif

/* Properties that we can deal with */
LONG ilbmprops[] =
{
    ID_ILBM, ID_BMHD,
    ID_ILBM, ID_CMAP,
    ID_ILBM, ID_GRAB,
    ID_ILBM, ID_CAMG,
    ID_ILBM, ID_NAME,
};

ILBM *ReadILBM (BPTR drawer, STRPTR name, UWORD depth)
{
    struct IFFHandle *iff;
    BPTR lock = NULL;
    ILBM *ir = NULL;

    /* Did they give a directory lock? */
    if (drawer)
    {
	/* Go to the required directory */
	lock = CurrentDir (drawer);
    }

    /* Allocate an IFF handle */
    if (iff = AllocIFF ())
    {
	/* Open the preference file */
	if (iff->iff_Stream = Open (name, MODE_OLDFILE))
	{
	    /* Indicate that the IFF handle is for a file */
	    InitIFFasDOS (iff);

	    /* Load the picture */
	    ir = GetILBM (iff, depth);

	    /* Close the file */
	    Close (iff->iff_Stream);
	}

	/* Free the IFF handle */
	FreeIFF (iff);
    }

    if (lock)
    {
	CurrentDir (lock);
    }

    return (ir);
}

ILBM *GetILBM (struct IFFHandle * iff, UWORD depth)
{
    struct StoredProperty *sp;
    ULONG extra;
    ILBM *ir;

    /* Open the IFF handle for reading */
    if (OpenIFF (iff, IFFF_READ) == 0)
    {
	/* Register collection chunks */
	PropChunks (iff, ilbmprops, 5);

	/* Register where we want to stop */
	StopChunk (iff, ID_ILBM, ID_BODY);

	/* Parse the file, stopping at the body */
	if (ParseIFF (iff, IFFPARSE_SCAN) == 0L)
	{
	    /* Allocate an ILBM record */
	    if (ir = (ILBM *) AllocVec (sizeof (ILBM), MEMF_CLEAR))
	    {
		/* See if we collected a bitmap header */
		if (GetBMHD (iff, &ir->ir_BMHD))
		{
		    /* Get the size information */
		    ir->ir_BMHD.nplanes = MIN (ir->ir_BMHD.nplanes, MAXSRCPLANES);

		    /* Allocate a bitmap to hold the image */
		    if (ir->ir_BMap = AllocMyBitMap ( MAX (ir->ir_BMHD.nplanes, depth), ir->ir_BMHD.w, ir->ir_BMHD.h))
		    {
			/* See if we have a CAMG chunk */
			if (sp = FindProp (iff, ID_ILBM, ID_CAMG))
			{
			    /* Get the mode ID */
			    ir->ir_ModeID = *((ULONG *) sp->sp_Data);

			    /*
			     * Knock bad bits out of old-style 16-bit viewmode
			     * CAMGs
			     */
			    if ((!(ir->ir_ModeID & MONITOR_ID_MASK)) ||
				((ir->ir_ModeID & EXTENDED_MODE) &&
				 (!(ir->ir_ModeID & 0xFFFF0000))))
			    {
				ir->ir_ModeID &= (~(EXTENDED_MODE | SPRITES | GENLOCK_AUDIO | GENLOCK_VIDEO | VP_HIDE));
			    }

			    /*
			     * Check for bogus CAMG like DPaintII brushes with
			     * junk in upper word and extended bit not set in
			     * lower word.
			     */
			    if ((ir->ir_ModeID & 0xFFFF0000) && (!(ir->ir_ModeID & 0x00001000)))
			    {
				ir->ir_ModeID = 0L;
			    }
			}

			/* See if we have a valid ModeID */
			if (ir->ir_ModeID == 0L)
			{
			    /* We have to fabricate a correct ModeID */
			    ir->ir_ModeID = LORES_KEY;
			    if (ir->ir_BMHD.w >= 640)
				ir->ir_ModeID = HIRES;
			    if (ir->ir_BMHD.h >= 400)
				ir->ir_ModeID |= LACE;
			    if (ir->ir_BMHD.nplanes == 6)
				ir->ir_ModeID |= HAM;
			}

#if 0
			x = (LONG) ai->ai_Disp.Resolution.x;
			y = (LONG) ai->ai_Disp.Resolution.y;

			ph = ((w * x) + (y >> 1)) / y;
			pw = ((h * y) + (x >> 1)) / x;

			if (w < pw)
			{
			    /* Bounding box is narrower than it is tall */
			    h = ph;
			}
			else
			{
			    /* Bounding box is wider than it is tall */
			    w = pw;
			}
#endif

#if 0
/*  Bitmap header (BMHD) structure  */
			struct BitMapHeader
			{
			    UWORD w, h;	/* Width, height in pixels */
			    WORD x, y;	/* x, y position for this bitmap  */
			    UBYTE nplanes;	/* # of planes  */
			    UBYTE Masking;
			    UBYTE Compression;
			    UBYTE pad1;
			    UWORD TransparentColor;
			    UBYTE XAspect, YAspect;
			    WORD PageWidth, PageHeight;
			};

#endif

			/* Get the color map */
			if (ir->ir_NumColors = GetColors (iff, &(ir->ir_Colors[0])))
			{
			    WORD i, r, g, b;

			    for (i = 0; i < ir->ir_NumColors; i++)
			    {
#if AA
				/* AA use FIX THIS!!!! */
				if (GfxBase->lib_Version >= 39)
				{
				    ir->ir_CRegs[i][0] = (LONG) ir->ir_Colors[i].red << 24;
				    ir->ir_CRegs[i][1] = (LONG) ir->ir_Colors[i].green << 24;
				    ir->ir_CRegs[i][2] = (LONG) ir->ir_Colors[i].blue << 24;
				}
				else
				{
#endif
				    ir->ir_CRegs[i][0] = (LONG) ir->ir_Colors[i].red >> 4;
				    ir->ir_CRegs[i][1] = (LONG) ir->ir_Colors[i].green >> 4;
				    ir->ir_CRegs[i][2] = (LONG) ir->ir_Colors[i].blue >> 4;
#if AA
				}
#endif
				/* For LoadRGB4() */
				if (i < 32)
				{
				    r = (ir->ir_Colors[i].red >> 4);
				    g = (ir->ir_Colors[i].green >> 4);
				    b = (ir->ir_Colors[i].blue >> 4);
				    ir->ir_RGB4[i] = (r << 8) | (g << 4) | (b);
				}
			    }
			}

			/* Get the hot spot */
			GetHotSpot (iff, &ir->ir_Grab, ir->ir_BMHD.w, ir->ir_BMHD.h);

			/* Get the name */
			if (sp = FindProp (iff, ID_ILBM, ID_NAME))
			{
			    /* How much room do we need to store the name */
			    extra = strlen (sp->sp_Data) + 1;

			    /* Allocate the buffer */
			    if (ir->ir_Name = (STRPTR) AllocVec (extra, MEMF_CLEAR))
			    {
				/* Copy the name into the buffer */
				strcpy (ir->ir_Name, (STRPTR) sp->sp_Data);
			    }
			}

			/* Get the image data itself */
			if (GetBody (iff, &ir->ir_BMHD, ir->ir_BMap))
			{
			    /* Close the IFF handle!!! */
			    CloseIFF (iff);
			    return (ir);

			}
		    }		/* End of allocate BM */
		}		/* End of get bitmap header */

		/* Free the ilbm buffer */
		FreeILBM (ir);
	    }
	}

	/* Close the IFF handle */
	CloseIFF (iff);
    }

    return (NULL);
}

BOOL GetBMHD (struct IFFHandle * iff, struct BitMapHeader * bmhd)
{
    struct StoredProperty *sp;

    /* Find the BitMap header */
    if (sp = FindProp (iff, ID_ILBM, ID_BMHD))
    {
	/* Copy the data to the destination */
	CopyMem (sp->sp_Data, bmhd, sizeof (struct BitMapHeader) );

	return (TRUE);
    }

    return (FALSE);
}

WORD GetColors (struct IFFHandle * iff, struct ColorRegister * cmap)
{
    struct ColorRegister *rgb;
    struct StoredProperty *sp;
    WORD i, ncolors = 0;

    if (sp = FindProp (iff, ID_ILBM, ID_CMAP))
    {
	/* Compute the actual number of colors we need to convert. */
	i = ncolors = MIN (MAXCOLORS, (sp->sp_Size / 3L));

	rgb = (struct ColorRegister *) sp->sp_Data;
	while (i--)
	{
	    *cmap = *rgb;
	    cmap++;
	    rgb++;
	}
    }

    return (ncolors);
}

VOID GetHotSpot (struct IFFHandle * iff, struct Point2D * grab, WORD width, WORD height)
{
    struct StoredProperty *sp;

    if (sp = FindProp (iff, ID_ILBM, ID_GRAB))
    {
	*grab = *((struct Point2D *) sp->sp_Data);
    }
    else
    {
	grab->x = 0;
	grab->y = 0;
    }

    if ((grab->x >= width) || (grab->y >= height))
    {
	grab->x = 0;
	grab->y = 0;
    }
}
