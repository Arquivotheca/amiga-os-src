/* ilbm.c
 * from pointer
 */

#include "pointer.h"

#define	BPR(w)		((w) + 15 >> 4 << 1)	/* Bytes per row */

/* Properties that we can deal with */
LONG ilbmprops[] =
{
    ID_ILBM, ID_BMHD,
    ID_ILBM, ID_CMAP,
    ID_ILBM, ID_GRAB,
};

UWORD ConvertPP (EdDataPtr ed, struct PointerPrefs * sp, struct PointerPrefs * dp, WORD adj)
{
    register UWORD x, y, c;	/* x coord, y coord, color */
    UWORD w, h, n;
    UBYTE *pptr;		/* plane pointer */
    WORD plane;			/* plane number */

    /* How wide do we want to be */
    w = MIN (sp->pp_Width, ed->ed_Width);
    h = MIN (sp->pp_Height, ed->ed_Height);
    n = 4;

    /* Remap our image */
    for (y = 0; y < h; y++)
    {
	for (x = 0; x < w; x++)
	{
	    c = 0;
	    for (plane = (sp->pp_BMap.Depth) - 1; plane >= 0; plane--)
	    {
		c = c << 1;
		pptr = (UBYTE *) sp->pp_BMap.Planes[plane];
		if (pptr[y * sp->pp_BMap.BytesPerRow + x / 8] & (1 << (7 - x % 8)))
		{
		    c |= 1;
		}
	    }

	    if (c != 0)
	    {
		n = MAX (n, c);
		SetAPen (&dp->pp_RPort, c + adj);
		WritePixel (&dp->pp_RPort, x, y);
	    }
	}
    }

    /* Copy the support fields */
    dp->pp_Depth = sp->pp_Depth;
    dp->pp_NumColors = sp->pp_NumColors;
    for (c = 0; c < MAXCOLORS; c++)
	dp->pp_CMap[c] = sp->pp_CMap[c];

    dp->pp_Grab = sp->pp_Grab;
    if (dp->pp_Grab.x >= ed->ed_Width)
	dp->pp_Grab.x = ed->ed_Width - 1;
    if (dp->pp_Grab.y >= ed->ed_Height)
	dp->pp_Grab.y = ed->ed_Height - 1;

    dp->pp_CMap[0].red = 0;
    dp->pp_CMap[0].green = 0;
    dp->pp_CMap[0].blue = 0;

    return (n);
}

EdStatus ReadPrefs (EdDataPtr ed, struct IFFHandle * iff)
{
    struct PointerPrefs *pp = &(ed->ed_PrefsTemp);
    EdStatus result = ES_IFFERROR;
    struct StoredProperty *sp;
    struct BitMapHeader *bmhd;

    /* Open the IFF handle for reading */
    if (!(OpenIFF (iff, IFFF_READ)))
    {
	/* Register collection chunks */
	PropChunks (iff, ilbmprops, 3);

	/* Register where we want to stop */
	StopChunk (iff, ID_ILBM, ID_BODY);

	/* Parse the file, stopping at the body */
	result = ES_IFF_UNKNOWNCHUNK;
	if (ParseIFF (iff, IFFPARSE_SCAN) == 0L)
	{
	    /* See if we collected a bitmap header */
	    if (sp = FindProp (iff, ID_ILBM, ID_BMHD))
	    {
		/* Get a pointer to the bitmap header */
		bmhd = (struct BitMapHeader *) sp->sp_Data;

		/* We can only handle 8 or less bit-planes */
		if (bmhd->nplanes <= 8)
		{
		    /* Get the size information */
		    pp->pp_Width = bmhd->w;			/* 16; */
		    pp->pp_Height = bmhd->h;		/* MIN (bmhd->h, 200); */
		    pp->pp_Depth = bmhd->nplanes;		/* MIN (bmhd->nplanes, 2); */

		    /* Allocate a bitmap w/rasters to hold the image */
		    result = ES_NO_MEMORY;
		    if (allocbitmap (ed, &pp->pp_BMap, pp->pp_Depth, pp->pp_Width, pp->pp_Height))
		    {
			/* Initialize the rastport */
			InitRastPort (&pp->pp_RPort);
			pp->pp_RPort.BitMap = &pp->pp_BMap;
			SetRast (&pp->pp_RPort, 0);

			/* Get the color map */
			pp->pp_NumColors = GetColors (ed, iff, pp->pp_CMap);

			/* Get the hot spot */
			GetHotSpot (ed, iff, &(pp->pp_Grab), pp->pp_Width, pp->pp_Height);

			/* Get the image data itself */
			if (GetBody (ed, iff, bmhd, &pp->pp_BMap))
			{
			    result = ES_NORMAL;
			}
		    }
		}
	    }
	}

	/* Close the IFF handle */
	CloseIFF (iff);
    }

    /* Copy the image to the work area */
    if (result == ES_NORMAL)
    {
	/* Clear the destination */
	SetRast (&(ed->ed_PrefsWork.pp_RPort), 0);

	/* Remap the image */
	ConvertPP (ed, pp, &(ed->ed_PrefsWork), 4);

	/* Free the temporary work buffer */
	FreeILBM (ed, pp);
    }

    return (result);
}

#define	BMHSIZE	(sizeof(struct BitMapHeader))

EdStatus WritePrefs (EdDataPtr ed, struct IFFHandle * iff)
{
    struct PointerPrefs *pp = &(ed->ed_PrefsTemp);
    EdStatus result = ES_IFFERROR;
    struct BitMapHeader *bmhd;
    UWORD background;

    /* Copy the work into the temp */
    ed->ed_PrefsTemp = ed->ed_PrefsWork;

    /* Allocate a bitmap w/rasters to hold the image */
    if (allocbitmap (ed, &pp->pp_BMap, ed->ed_Depth, ed->ed_Width, ed->ed_Height))
    {
	/* Initialize the rastport */
	InitRastPort (&pp->pp_RPort);
	pp->pp_RPort.BitMap = &pp->pp_BMap;
	SetRast (&pp->pp_RPort, 0);

	/* Move the colors up, and trim unused planes. */
        switch ((ConvertPP (ed, &(ed->ed_PrefsWork), pp, (-4)) - 4))
        {
            case 0:
            case 1:
                pp->pp_Depth = 1;
                break;

            case 2:
            case 3:
                pp->pp_Depth = 2;
                break;

            case 4:
            case 5:
            case 6:
            case 7:
                pp->pp_Depth = 3;
                break;

            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
                pp->pp_Depth = 4;
                break;
        }
        pp->pp_NumColors = 1 << pp->pp_Depth;

	/* Set color zero to the Workbench background */
	background = 0xAAA;
	if (ed->ed_PScreen)
	{
	    background = GetRGB4 (ed->ed_PScreen->ViewPort.ColorMap, 0);
	}
	pp->pp_CMap[0].red   = (background & 0xF00) >> 4;
	pp->pp_CMap[0].green = (background & 0xF0);
	pp->pp_CMap[0].blue  = background << 4;

	/* Open the IFF handle for reading */
	if (!(OpenIFF (iff, IFFF_WRITE)))
	{
	    /* Start writing the preference record ... */
	    if (!(PushChunk (iff, ID_ILBM, ID_FORM, IFFSIZE_UNKNOWN)))
	    {
		/* Allocate a temporary bitmap header */
		result = ES_NO_MEMORY;
		if (bmhd = AllocMem (BMHSIZE, MEMF_CLEAR))
		{
		    /* Write the bitmap header */
		    if (PutBMHD (ed, iff, bmhd, pp))
		    {
			/* Write the colors */
			PutColors (ed, iff, bmhd, pp->pp_CMap);

			/* Write the grab point */
			if (PutHotSpot (ed, iff, &(pp->pp_Grab)))
			{
			    /* Write the body */
			    if (PutBody (ed, iff, bmhd, &(pp->pp_BMap)))
			    {
				result = ES_NORMAL;
			    }
			}
		    }

		    FreeMem (bmhd, BMHSIZE);
		}

		/* Finish out the entire write */
		PopChunk (iff);
	    }

	    /* Close the IFF handle */
	    CloseIFF (iff);
	}

	/* Free the temporary work buffer */
	FreeILBM (ed, pp);
    }

    return (result);
}

BOOL GetBMHD (EdDataPtr ed, struct IFFHandle * iff, struct BitMapHeader * bmhd)
{
    struct StoredProperty *sp;

    if (sp = FindProp (iff, ID_ILBM, ID_BMHD))
    {
	*bmhd = *((struct BitMapHeader *) sp->sp_Data);
	return (TRUE);
    }

    return (FALSE);
}

BOOL PutBMHD (EdDataPtr ed, struct IFFHandle * iff, struct BitMapHeader * bmhd, struct PointerPrefs * pp)
{
    bmhd->w = pp->pp_BMap.BytesPerRow * 8;
    bmhd->h = pp->pp_BMap.Rows;
    bmhd->x = 0;
    bmhd->y = 0;
    bmhd->nplanes = pp->pp_Depth;
    bmhd->Masking = mskHasTransparentColor;
    bmhd->Compression = cmpNone;
    bmhd->pad1 = 0;
    bmhd->TransparentColor = 0;
    bmhd->XAspect = 44;
    bmhd->YAspect = 52;;
    bmhd->PageWidth = (ed->ed_Screen) ? ed->ed_Screen->Width : 640;
    bmhd->PageHeight = (ed->ed_Screen) ? ed->ed_Screen->Height : 200;

    if ((PushChunk (iff, 0, ID_BMHD, sizeof (struct BitMapHeader))) == 0L)
    {
	if ((WriteChunkBytes (iff, bmhd, sizeof (struct BitMapHeader))) ==
	    sizeof (struct BitMapHeader))
	{
	    if ((PopChunk (iff)) == 0L)
	    {
		return (TRUE);
	    }
	}
    }

    return (FALSE);
}

WORD GetColors (EdDataPtr ed, struct IFFHandle * iff, struct ColorRegister * cmap)
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

BOOL PutColors (EdDataPtr ed, struct IFFHandle * iff, struct BitMapHeader * bmhd, struct ColorRegister * cmap)
{
    if ((PushChunk (iff, 0, ID_CMAP, IFFSIZE_UNKNOWN)) == 0L)
    {
	if ((WriteChunkRecords (iff, cmap, sizeof (struct ColorRegister),
				(1 << bmhd->nplanes))) == (1 << bmhd->nplanes))
	{
	    if ((PopChunk (iff)) == 0L)
	    {
		return (TRUE);
	    }
	}
    }
    return (FALSE);
}

void GetHotSpot (EdDataPtr ed, struct IFFHandle * iff, struct Point2D * grab, WORD width, WORD height)
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

BOOL PutHotSpot (EdDataPtr ed, struct IFFHandle * iff, struct Point2D * grab)
{
    if ((PushChunk (iff, 0, ID_GRAB, sizeof (struct Point2D))) == 0L)
    {
	if ((WriteChunkBytes (iff, grab, sizeof (struct Point2D))) ==
	    sizeof (struct Point2D))
	{
	    if ((PopChunk (iff)) == 0L)
	    {
		return (TRUE);
	    }
	}
    }
    return (FALSE);
}

BOOL GetBody (EdDataPtr ed, struct IFFHandle * iff, struct BitMapHeader * bmhd, struct BitMap * bm)
{
    BOOL status = FALSE;
    WORD i, n, p;
    UWORD *srcline, *destline;
    struct BitMap mapcopy, *destmap;
    UBYTE *linebuf, *csrcline;
    WORD srcw, destw;		/* src and dest width in BYTES */
    WORD srch, desth;		/* src and dest height (rows) */
    WORD srcd, destd;		/* src and dest depth */
    WORD deep, rows, mod;

    /* Check compression type. */
    if ((bmhd->Compression != cmpNone) && (bmhd->Compression != cmpByteRun1))
    {
	return (status);
    }

    /* Copy the bitmap pointer */
    destmap = bm;

    /* Copy the bitmap */
    mapcopy = *destmap;

    /* Set up our size variables */
    srcw = BPR (bmhd->w);
    srch = bmhd->h;
    srcd = bmhd->nplanes;
    destw = mapcopy.BytesPerRow;
    desth = mapcopy.Rows;
    destd = mapcopy.Depth;
    rows = MIN (srch, desth);

    mod = destw - srcw;
    if (mod < 0)
	mod = -mod;

    if (bmhd->Masking == mskHasMask)
	srcd++;

    deep = MIN (srcd, destd);

    /*
     * Allocate a one-line buffer to load imagery in.  The line is then copied into the destination bitmap.  This seeming
     * duplicity makes clipping loaded images easier.
     */

    if (linebuf = (UBYTE *) AllocVec ((LONG)(srcw * srcd), MEMF_CLEAR))
    {
	/*
	 * Load the BODY into the allocated line buffer, then copy into the destination bitmap.
	 */

	for (i = rows; i--;)
	{
	    if (!(status = GetLine (ed, iff, linebuf, srcw, srcd, bmhd->Compression)))
		break;

	    srcline = (UWORD *) linebuf;
	    for (p = 0; p < deep; p++)
	    {
		destline = (UWORD *) mapcopy.Planes[p];
		*destline = 0xffff;
		n = (MIN (srcw, destw)) >> 1;	/* convert #bytes to #words */

		while (n--)
		    *destline++ = *srcline++;

		if (srcw > destw)
		{
		    csrcline = (UBYTE *) srcline;
		    csrcline += mod;
		    srcline = (UWORD *) csrcline;
		}
		mapcopy.Planes[p] += destw;
	    }
	}

	/* Free the temporary line buffer */
	FreeVec ((APTR) linebuf);

	/* Say that it worked */
	status = TRUE;
    }

    return (status);
}

BOOL GetLine (EdDataPtr ed, struct IFFHandle * iff, UBYTE * buf, WORD wide, WORD deep, UBYTE cmptype)
{

    if (cmptype == cmpNone)
    {				/* No compression */
	LONG big = wide * deep;

	if (ReadChunkBytes (iff, buf, big) != big)
	    return (FALSE);
    }
    else
    {
	WORD i, so_far;
	UBYTE *dest = buf;
	BYTE len;

	for (i = deep; i--;)
	{
	    so_far = wide;
	    while (so_far > 0)
	    {
		if (ReadChunkBytes (iff, &len, 1L) != 1)
		    return (FALSE);

		if (len >= 0)
		{		/* Literal byte copy  */
		    if ((so_far -= ++len) < 0)
			break;
		    if (ReadChunkBytes (iff, dest, (LONG) len) != len)
			return (FALSE);
		    dest += len;
		}
		else if ((UBYTE) len == 128)
		     /* NOP  */ ;

		else if (len < 0)
		{		/* Replication count  */
		    UBYTE byte;

		    len = -len + 1;
		    if ((so_far -= len) < 0)
			break;
		    if (ReadChunkBytes (iff, &byte, 1L) != 1)
			return (FALSE);
		    while (--len >= 0)
			*dest++ = byte;
		}
	    }
	    if (so_far)
	    {
		return (FALSE);
	    }
	}
    }

    return (TRUE);
}

BOOL PutBody (EdDataPtr ed, struct IFFHandle * iff, struct BitMapHeader * bmhd, struct BitMap * bitmap)
{
    register UBYTE *pptr;
    register UWORD i;
    register UBYTE p;

    if ((PushChunk (iff, 0, ID_BODY, IFFSIZE_UNKNOWN)) == 0L)
    {
	for (i = 0; i < bmhd->h; i++)
	{
	    for (p = 0; p < bmhd->nplanes; p++)
	    {
		pptr = (UBYTE *) bitmap->Planes[p];
		WriteChunkBytes (iff, &pptr[i * bitmap->BytesPerRow],
				 2 * ((bmhd->w + 15) / 16));
	    }
	}

	if ((PopChunk (iff)) == 0L)
	{
	    return (TRUE);
	}
    }

    return (FALSE);
}

VOID FreeILBM (EdDataPtr ed, struct PointerPrefs * pp)
{
    register WORD i;

    /* Free each plane of the bitmap */
    for (i = 0; i < pp->pp_BMap.Depth; i++)
    {
	if (pp->pp_BMap.Planes[i])
	{
	    /* Free the raster */
	    FreeRaster (pp->pp_BMap.Planes[i], pp->pp_Width, pp->pp_Height);
	}
    }
}
