/* ilbm.c
 * from pointer
 */

#include "prefsbase.h"
#include <intuition/intuitionbase.h>

void kprintf (void *,...);

extern struct Library *SysBase, *IFFParseBase, *GfxBase, *IntuitionBase;

#define	BPR(w)		((w) + 15 >> 4 << 1)	/* Bytes per row */

/* ilbm.c */
ILBM *ReadILBM (BPTR drawer, STRPTR name, struct TagItem * attrs);
VOID FreeILBM (ILBM * ilbm);
ILBM *GetILBM (struct IFFHandle * iff);
BOOL GetBMHD (struct IFFHandle * iff, struct BitMapHeader * bmhd);
BOOL PutBMHD (struct IFFHandle * iff, struct BitMapHeader * bmhd, struct BitMap * bm);
WORD GetColors (struct IFFHandle * iff, struct ColorRegister * cmap);
BOOL PutColors (struct IFFHandle * iff, struct BitMapHeader * bmhd, struct ColorRegister * cmap);
void GetHotSpot (struct IFFHandle * iff, struct Point2D * grab, WORD, WORD);
BOOL PutHotSpot (struct IFFHandle * iff, struct Point2D * grab);
BOOL GetBody (struct IFFHandle * iff, struct BitMapHeader * bmhd, struct BitMap * bm);
BOOL GetLine (struct IFFHandle * iff, UBYTE * buf, WORD wide, WORD deep, UBYTE cmptype);
BOOL PutBody (struct IFFHandle * iff, struct BitMapHeader * bmhd, struct BitMap * bitmap);
BOOL AllocBMRasters (struct BitMap * bm, BYTE depth, WORD width, WORD height);

/* Properties that we can deal with */
LONG ilbmprops[] =
{
    ID_ILBM, ID_BMHD,
    ID_ILBM, ID_CMAP,
    ID_ILBM, ID_GRAB,
    ID_ILBM, ID_CAMG
};

ILBM *ReadILBM (BPTR drawer, STRPTR name, struct TagItem * attrs)
{
    struct IFFHandle *iff;
    ILBM *ilbm = NULL;
    BPTR lock = NULL;

    if (drawer)
    {
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

	    /* Open the IFF handle for reading */
	    if (!(OpenIFF (iff, IFFF_READ)))
	    {
		/* Register collection chunks */
		PropChunks (iff, ilbmprops, 4);

		/* Register where we want to stop */
		StopChunk (iff, ID_ILBM, ID_BODY);

		/* Parse the file, stopping at the body */
		if (ParseIFF (iff, IFFPARSE_SCAN) == 0L)
		{
		    ilbm = GetILBM (iff);
		}

		/* Close the IFF handle */
		CloseIFF (iff);
	    }

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

    return (ilbm);
}

ILBM *GetILBM (struct IFFHandle * iff)
{
    ILBM *ilbm = NULL;
    LONG msize = sizeof (ILBM);

    /* Allocate an ILBM record */
    if (ilbm = (ILBM *) AllocVec (msize, MEMF_CLEAR))
    {
	struct StoredProperty *bmsp;

	/* See if we collected a bitmap header */
	if (bmsp = FindProp (iff, ID_ILBM, ID_BMHD))
	{
	    struct BitMap *bm = &(ilbm->ir_BMap);
	    struct BitMapHeader *bmhd;

	    /* Get a pointer to the bitmap header */
	    bmhd = (struct BitMapHeader *) bmsp->sp_Data;

	    /* Get the size information */
	    ilbm->ir_Width = bmhd->w;	/* 16; */
	    ilbm->ir_Height = bmhd->h;	/* MIN (bmhd->h, 200); */
	    ilbm->ir_Depth = bmhd->nplanes;	/* MIN (bmhd->nplanes, 2); */

	    /* Allocate a bitmap w/rasters to hold the image */
	    if (AllocBMRasters (bm, ilbm->ir_Depth, ilbm->ir_Width, ilbm->ir_Height))
	    {
		struct ColorRegister pp_Colors[MAXCOLORS];
		struct StoredProperty *sp;

		/* Initialize the rastport */
		InitRastPort (&(ilbm->ir_RPort));
		ilbm->ir_RPort.BitMap = bm;
		SetRast (&(ilbm->ir_RPort), 0);

		/* Set the display mode ID */
		if (sp = FindProp (iff, ID_ILBM, ID_CAMG))
		{
		    ilbm->ir_ModeID = (ULONG) sp->sp_Data;
		}
		else
		{
		    /* Need better fitting stuff ... of course */
		    ilbm->ir_ModeID = HIRES_KEY;
		}

		/* Get the color map */
		if (ilbm->ir_NumColors = GetColors (iff, &(pp_Colors[0])))
		{
		    WORD i, r, g, b;

		    for (i = 0; i < ilbm->ir_NumColors; i++)
		    {
			r = (pp_Colors[i].red >> 4);
			g = (pp_Colors[i].green >> 4);
			b = (pp_Colors[i].blue >> 4);
			ilbm->ir_CRegs[i] = (r << 8) | (g << 4) | (b);
		    }
		}

		/* Get the hot spot */
		GetHotSpot (iff, &(ilbm->ir_Grab), ilbm->ir_Width, ilbm->ir_Height);

		/* Get the image data itself */
		if (GetBody (iff, bmhd, bm))
		{
		    return (ilbm);

		}		/* End of get body */
	    }			/* End of allocate BM */
	}			/* End of get bitmap header */

	/* Free the ilbm buffer */
	FreeILBM (ilbm);
    }

    return (NULL);
}

#if 0
/* ======================================================================== *
 * ==									 == *
 * ==	BOOL PutILBM(struct IFFHandle *, PREF *)			 == *
 * ==									 == *
 * ======================================================================== */

BOOL PutILBM (iff, MP)
    struct IFFHandle *iff;
    PREF *MP;
{
    BOOL status = FALSE;
    struct BitMapHeader *bmhd;

    if ((bmhd = (struct BitMapHeader *) AllocVec (sizeof (struct BitMapHeader),
						  MEMF_CLEAR)) == NULL)
	return (FALSE);

    if (PutBMHD (iff, bmhd, MP->bm))
    {
	if (PutColors (iff, bmhd, &(MP->cmap[0])))
	{
	    if (PutHotSpot (iff, &(MP->grab)))
	    {
		if (PutBody (iff, bmhd, MP->bm))
		    status = TRUE;
	    }
	}
    }

    FreeVec ((APTR)bmhd);
    return (status);
}

#endif

/* ======================================================================== *
 * ==									 == *
 * ==	BOOL GetBMHD(struct IFFHandle *, struct BitMapHeader *)		 == *
 * ==									 == *
 * ======================================================================== */

BOOL GetBMHD (iff, bmhd)
    struct IFFHandle *iff;
    struct BitMapHeader *bmhd;
{
    struct StoredProperty *sp;

    if (sp = FindProp (iff, ID_ILBM, ID_BMHD))
    {
	*bmhd = *((struct BitMapHeader *) sp->sp_Data);
	return (TRUE);
    }

    return (FALSE);
}


/* ======================================================================== *
 * ==									 == *
 * ==	BOOL PutBMHD(struct IFFHandle *, struct BitMapHeader *,		 == *
 * ==		     struct BitMap *)					 == *
 * ==									 == *
 * ======================================================================== */

BOOL PutBMHD (iff, bmhd, bm)
    struct IFFHandle *iff;
    struct BitMapHeader *bmhd;
    struct BitMap *bm;
{

    bmhd->w = bm->BytesPerRow * 8;
    bmhd->h = bm->Rows;
    bmhd->x = 0;
    bmhd->y = 0;
    bmhd->nplanes = bm->Depth;
    bmhd->Masking = mskHasTransparentColor;
    bmhd->Compression = cmpNone;
/*     bmhd->Compression = cmpByteRun1; */
    bmhd->pad1 = 0;
    bmhd->TransparentColor = 0;
    bmhd->XAspect = 10;
    bmhd->YAspect = 11;
    bmhd->PageWidth = 320;
    bmhd->PageHeight = 200;

    if ((PushChunk (iff, 0, ID_BMHD, sizeof (struct BitMapHeader))) == 0L)
	if ((WriteChunkBytes (iff, bmhd, sizeof (struct BitMapHeader))) ==
	    sizeof (struct BitMapHeader))
	    if ((PopChunk (iff)) == 0L)
		return (TRUE);

    return (FALSE);
}


/* ======================================================================== *
 * ==									 == *
 * ==	BOOL GetColors(struct IFFHandle *, struct ColorRegister *)	 == *
 * ==									 == *
 * ======================================================================== */

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


/* ======================================================================== *
 * ==									 == *
 * ==	BOOL PutColors(struct IFFHandle *, struct BitMapHeader *,	 == *
 * ==		       struct ColorRegister *)				 == *
 * ==									 == *
 * ======================================================================== */

BOOL PutColors (iff, bmhd, cmap)
    struct IFFHandle *iff;
    struct BitMapHeader *bmhd;
    struct ColorRegister *cmap;
{

    if (cmap == NULL)
	return (FALSE);

    if ((PushChunk (iff, 0, ID_CMAP, IFFSIZE_UNKNOWN)) == 0L)
	if ((WriteChunkRecords (iff, cmap, sizeof (struct ColorRegister),
				(1 << bmhd->nplanes))) == (1 << bmhd->nplanes))
	    if ((PopChunk (iff)) == 0L)
		return (TRUE);
    return (FALSE);
}


/* ======================================================================== *
 * ==									 == *
 * ==	void GetHotSpot(struct IFFHandle *, struct Point2D *)		 == *
 * ==									 == *
 * ======================================================================== */

void GetHotSpot (iff, grab, width, height)
    struct IFFHandle *iff;
    struct Point2D *grab;
    WORD width, height;
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


/* ======================================================================== *
 * ==									 == *
 * ==	BOOL PutHotSpot(struct IFFHandle *, struct Point2D *)		 == *
 * ==									 == *
 * ======================================================================== */

BOOL PutHotSpot (iff, grab)
    struct IFFHandle *iff;
    struct Point2D *grab;
{

    if (grab == NULL)
	return (FALSE);

    if ((PushChunk (iff, 0, ID_GRAB, sizeof (struct Point2D))) == 0L)
	if ((WriteChunkBytes (iff, grab, sizeof (struct Point2D))) ==
	    sizeof (struct Point2D))
	    if ((PopChunk (iff)) == 0L)
		return (TRUE);
    return (FALSE);
}


/* ======================================================================== *
 * ==									 == *
 * ==	BOOL GetBody(struct IFFHandle *, struct BitMapHeader *,		 == *
 * ==		     struct BitMap *)					 == *
 * ==									 == *
 * ======================================================================== */

BOOL GetBody (iff, bmhd, bm)
    struct IFFHandle *iff;
    struct BitMapHeader *bmhd;
    struct BitMap *bm;
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
    LONG msize;

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
     * Allocate a one-line buffer to load imagery in.  The line is then copied
     * into the destination bitmap.  This seeming duplicity makes clipping
     * loaded images easier.
     */

    msize = (LONG) (srcw * srcd);
    if (linebuf = (UBYTE *) AllocVec (msize, MEMF_CLEAR))
    {

	/*
	 * Load the BODY into the allocated line buffer, then copy into the
	 * destination bitmap.
	 */

	for (i = rows; i--;)
	{
	    if (!(status = GetLine (iff, linebuf, srcw, srcd, bmhd->Compression)))
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


/* ======================================================================== *
 * ==									 == *
 * ==	LONG GetLine(struct IFFHandle *, UBYTE *, WORD, WORD, WORD)	 == *
 * ==									 == *
 * ======================================================================== */

BOOL GetLine (iff, buf, wide, deep, cmptype)
    struct IFFHandle *iff;
    UBYTE *buf;
    WORD wide, deep;
    UBYTE cmptype;
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
	buf += wide;
    }
    return (TRUE);
}


/* ======================================================================== *
 * ==									 == *
 * ==	BOOL PutBody(struct IFFHandle *, struct BitMapHeader *,		 == *
 * ==	  struct BitMap *)						 == *
 * ==									 == *
 * ======================================================================== */

BOOL PutBody (iff, bmhd, bitmap)
    struct IFFHandle *iff;
    struct BitMapHeader *bmhd;
    struct BitMap *bitmap;
{
    UWORD i;
    UBYTE p, *pptr;

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
	    return (TRUE);
    }
    return (FALSE);
}

/* ======================================================================== *
 * ==									 == *
 * ==	BOOL AllocBMRasters(struct BitMap *, BYTE, WORD, WORD)		 == *
 * ==									 == *
 * ======================================================================== */

BOOL AllocBMRasters (bm, depth, width, height)
    struct BitMap *bm;
    BYTE depth;
    WORD width, height;
{
    WORD i;

    /* Initialize the BitMap and allocate the Raster: */
    InitBitMap (bm, depth, width, height);
    for (i = 0; i < depth; i++)
    {
	if ((bm->Planes[i] = (PLANEPTR) AllocRaster (width, height)) == NULL)
	    return (FALSE);
    }
    return (TRUE);
}

VOID FreeILBM (ILBM * ilbm)
{
    LONG msize = sizeof (ILBM);

    if (ilbm)
    {
	struct BitMap *bm = &(ilbm->ir_BMap);
	WORD i;

	/* Free each plane of the bitmap */
	for (i = 0; i < ilbm->ir_Depth; i++)
	{
	    if (bm->Planes[i])
	    {
		/* Free the raster */
		FreeRaster (bm->Planes[i], ilbm->ir_Width, ilbm->ir_Height);
		bm->Planes[i] = NULL;
	    }
	}

	/* Free the ILBM record */
	FreeVec ((APTR) ilbm);
    }
}
