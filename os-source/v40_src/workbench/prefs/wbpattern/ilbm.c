/* ilbm.c
 * from pointer
 */

#include "wbpattern.h"

#define	DB(x)	;
#define	DC(x)	;
#define	BMHSIZE	(sizeof(struct BitMapHeader))
#define	BPR(w)		((w) + 15 >> 4 << 1)	/* Bytes per row */

/*****************************************************************************/

/* Properties that we can deal with */
LONG ilbmprops[] =
{
    ID_ILBM, ID_BMHD,
};

EdStatus ReadPrefs (EdDataPtr ed, struct IFFHandle * iff)
{
    struct WBPatternPrefs *pp = &(ed->ed_PrefsTemp);
    EdStatus result = ES_IFFERROR;
    struct StoredProperty *sp;
    struct BitMapHeader *bmhd;

    /* Open the IFF handle for reading */
    if (!(OpenIFF (iff, IFFF_READ)))
    {
	/* Register collection chunks */
	PropChunks (iff, ilbmprops, 1);

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

		/* We don't work with anything more than 8 bit-planes */
		if (bmhd->nplanes <= 8)
		{
		    /* Get the size information */
		    pp->wp_Width = bmhd->w;
		    pp->wp_Height = bmhd->h;
		    pp->wp_Depth = bmhd->nplanes;

		    /* Allocate a bitmap w/rasters to hold the image */
		    result = ES_NO_MEMORY;
		    if (allocbitmap (ed, &pp->wp_BMap, pp->wp_Depth, pp->wp_Width, pp->wp_Height))
		    {
			/* Initialize the rastport */
			InitRastPort (&pp->wp_RPort);
			pp->wp_RPort.BitMap = &pp->wp_BMap;
			SetRast (&pp->wp_RPort, 0);

			/* Get the image data itself */
			if (GetBody (ed, iff, bmhd, &pp->wp_BMap))
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
	SetPrefsRast (ed, 0);

	/* Copy from the temporary record to the preference record */
	bltbm (&(pp->wp_BMap), 0, 0,
	       &(ed->ed_PrefsWork.wp_BMap),
	       ed->ed_Which, 0, ed->ed_Width, ed->ed_Height,
	       0xC0, 0xFF, NULL, ed->ed_GfxBase);

	/* Free the temporary work buffer */
	FreeILBM (ed, pp);
    }

    return (result);
}

/*****************************************************************************/

EdStatus WritePrefs (EdDataPtr ed, struct IFFHandle * iff)
{
    struct WBPatternPrefs *pp = &(ed->ed_PrefsTemp);
    EdStatus result = ES_IFFERROR;
    struct BitMapHeader *bmhd;

    /* Copy the work into the temp */
    ed->ed_PrefsTemp = ed->ed_PrefsWork;

    /* Allocate a bitmap w/rasters to hold the image */
    if (allocbitmap (ed, &pp->wp_BMap, ed->ed_Depth, ed->ed_Width, ed->ed_Height))
    {
	/* Initialize the rastport */
	InitRastPort (&pp->wp_RPort);
	pp->wp_RPort.BitMap = &pp->wp_BMap;
	SetRast (&pp->wp_RPort, 0);

	/* Copy from the preference record to the temporary record */
	bltbm (&(ed->ed_PrefsWork.wp_BMap), ed->ed_Which, 0,
	       &(pp->wp_BMap), 0, 0, ed->ed_Width, ed->ed_Height,
	       0xC0, 0xFF, NULL, ed->ed_GfxBase);

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
			/* Write the body */
			if (PutBody (ed, iff, bmhd, &(pp->wp_BMap)))
			{
			     result = ES_NORMAL;
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

/*****************************************************************************/

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

/*****************************************************************************/

BOOL PutBMHD (EdDataPtr ed, struct IFFHandle * iff, struct BitMapHeader * bmhd, struct WBPatternPrefs * pp)
{
    bmhd->w = pp->wp_BMap.BytesPerRow * 8;
    bmhd->h = pp->wp_BMap.Rows;
    bmhd->x = 0;
    bmhd->y = 0;
    bmhd->nplanes = pp->wp_Depth;
    bmhd->Masking = mskHasTransparentColor;
    bmhd->Compression = cmpNone;
    bmhd->pad1 = 0;
    bmhd->TransparentColor = 0;
    bmhd->XAspect = 44;
    bmhd->YAspect = 52;;
    bmhd->PageWidth = ed->ed_Screen->Width;
    bmhd->PageHeight = ed->ed_Screen->Height;

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

/*****************************************************************************/

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

/*****************************************************************************/

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

/*****************************************************************************/

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

/*****************************************************************************/

VOID FreeILBM (EdDataPtr ed, struct WBPatternPrefs * pp)
{
    register WORD i;

    /* Free each plane of the bitmap */
    for (i = 0; i < pp->wp_BMap.Depth; i++)
    {
	if (pp->wp_BMap.Planes[i])
	{
	    /* Free the raster */
	    FreeRaster (pp->wp_BMap.Planes[i], pp->wp_Width, pp->wp_Height);
	}
    }
}
