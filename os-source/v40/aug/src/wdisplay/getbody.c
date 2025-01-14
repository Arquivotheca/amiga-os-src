/* getbody.c
 *
 */

#include "wdisplay.h"

LONG GetBody (struct IFFHandle * iff, struct BitMapHeader * bmhd, struct BitMap * bitmap)
{
    LONG error = 1;
    ULONG bufsize;
    BYTE *buffer;

    if (bitmap && bmhd)
    {
	bufsize = MaxPackedSize (RowBytes (bmhd->w)) << 4;

	if (buffer = AllocMem (bufsize, MEMF_CLEAR))
	{
	    error = loadbody2 (iff, bitmap, NULL, bmhd, buffer, bufsize);

	    FreeMem (buffer, bufsize);
	}
	else
	{
	    error = IFFERR_NOMEM;
	}
    }

    return (error);
}

#define	COUNT_EM	FALSE

#if COUNT_EM
UBYTE current_plane;
ULONG plane_use[MAXSRCPLANES+1];
#endif

BOOL unpackrow (BYTE ** pSource, BYTE ** pDest, WORD srcBytes0, WORD dstBytes0)
{
    register WORD srcBytes = srcBytes0;
    register WORD dstBytes = dstBytes0;
    register BYTE *source = *pSource;
    register BYTE *dest = *pDest;
    WORD minus128 = -128;	/* get the compiler to generate a CMP.W */
    BOOL error = TRUE;
    register WORD n;
    register BYTE c;

    while (dstBytes > 0)
    {
	if ((srcBytes -= 1) < 0)
	    goto ErrorExit;
	n = UGetByte ();

	if (n >= 0)
	{
	    n += 1;
	    if ((srcBytes -= n) < 0)
		goto ErrorExit;
	    if ((dstBytes -= n) < 0)
		goto ErrorExit;

#if COUNT_EM
	    plane_use[current_plane] += n;
#endif

	    do
	    {
		UPutByte (UGetByte ());
	    } while (--n > 0);
	}
	else if (n != minus128)
	{
	    n = -n + 1;
	    if ((srcBytes -= 1) < 0)
		goto ErrorExit;
	    if ((dstBytes -= n) < 0)
		goto ErrorExit;
	    c = UGetByte ();

#if COUNT_EM
	    if (c != 0)
	    {
		plane_use[current_plane] += ABS(n);
	    }
#endif

	    do
	    {
		UPutByte (c);
	    } while (--n > 0);
	}
    }
    error = FALSE;		/* success! */

  ErrorExit:
    *pSource = source;
    *pDest = dest;
    return (error);
}

/* like the old GetBODY */
LONG loadbody2 (iff, bitmap, mask, bmhd, buffer, bufsize)
    struct IFFHandle *iff;
    struct BitMap *bitmap;
    BYTE *mask;
    struct BitMapHeader *bmhd;
    BYTE *buffer;
    ULONG bufsize;
{
    UBYTE srcPlaneCnt = bmhd->nplanes;	/* Haven't counted for mask plane yet */
    WORD srcRowBytes = RowBytes (bmhd->w);
    WORD destRowBytes = bitmap->BytesPerRow;
    LONG bufRowBytes = MaxPackedSize (srcRowBytes);
    int nRows = bmhd->h;
    WORD compression = bmhd->Compression;
    register int iPlane, iRow, nEmpty;
    register WORD nFilled;
    BYTE *buf, *nullDest, *nullBuf, **pDest;
    BYTE *planes[MAXSRCPLANES];	/* array of ptrs to planes & mask */
    struct ContextNode *cn;

    cn = CurrentChunk (iff);
    if (compression > cmpByteRun1)
	return (FALSE);

    /* Complain if client asked for a conversion GetBODY doesn't handle. */
    if (srcRowBytes > bitmap->BytesPerRow ||
	bufsize < bufRowBytes * 2 ||
	srcPlaneCnt > MAXSRCPLANES)
	return (FALSE);

    if (nRows > bitmap->Rows)
	nRows = bitmap->Rows;

    /* Initialize array "planes" with bitmap ptrs; NULL in empty slots. */
    for (iPlane = 0; iPlane < bitmap->Depth; iPlane++)
    {
#if COUNT_EM
	plane_use[iPlane] = 0;
#endif
	planes[iPlane] = (BYTE *) bitmap->Planes[iPlane];
    }
    for (; iPlane < MAXSRCPLANES; iPlane++)
	planes[iPlane] = NULL;

    /* Copy any mask plane ptr into corresponding "planes" slot. */
    if (bmhd->Masking == mskHasMask)
    {
	if (mask != NULL)
	    planes[srcPlaneCnt] = mask;	/* If there are more srcPlanes than
					 * dstPlanes, there will be NULL
					 * plane-pointers before this. */
	else
	    planes[srcPlaneCnt] = NULL;	/* In case more dstPlanes than src. */
	srcPlaneCnt += 1;	/* Include mask plane in count. */
    }

    /* Setup a sink for dummy destination of rows from unwanted planes. */
    nullDest = buffer;
    buffer += srcRowBytes;
    bufsize -= srcRowBytes;

    /*
     * Read the BODY contents into client's bitmap. De-interleave planes and
     * decompress rows. MODIFIES: Last iteration modifies bufsize.
     */

    buf = buffer + bufsize;	/* Buffer is currently empty. */
    for (iRow = nRows; iRow > 0; iRow--)
    {
	for (iPlane = 0; iPlane < srcPlaneCnt; iPlane++)
	{
#if COUNT_EM
	    current_plane = iPlane;
#endif
	    pDest = &planes[iPlane];

	    /* Establish a sink for any unwanted plane. */
	    if (*pDest == NULL)
	    {
		nullBuf = nullDest;
		pDest = &nullBuf;
	    }

	    /* Read in at least enough bytes to uncompress next row. */
	    nEmpty = buf - buffer;	/* size of empty part of buffer. */
	    nFilled = bufsize - nEmpty;	/* this part has data. */
	    if (nFilled < bufRowBytes)
	    {
		/* Need to read more. */

		/* Move the existing data to the front of the buffer. */
		/* Now covers range buffer[0]..buffer[nFilled-1]. */
		movmem (buf, buffer, nFilled);	/* Could be moving 0 bytes. */

		if (nEmpty > ChunkMoreBytes (cn))
		{
		    /* There aren't enough bytes left to fill the buffer. */
		    nEmpty = ChunkMoreBytes (cn);
		    bufsize = nFilled + nEmpty;	/* heh-heh */
		}

		/* Append new data to the existing data. */
		if (ReadChunkBytes (iff, &buffer[nFilled], nEmpty) < nEmpty)
		    return (FALSE);

		buf = buffer;
		nFilled = bufsize;
	    }

	    /* Copy uncompressed row to destination plane. */
	    if (compression == cmpNone)
	    {
		if (nFilled < srcRowBytes)
		    return (IFFERR_MANGLED);
		movmem (buf, *pDest, srcRowBytes);
		buf += srcRowBytes;
		*pDest += destRowBytes;
	    }
	    else
	    {
		/* Decompress row to destination plane. */
		if (unpackrow (&buf, pDest, nFilled, srcRowBytes))
		    /* pSource, pDest, srcBytes, dstBytes  */
		    return (IFFERR_MANGLED);
		else
		    *pDest += (destRowBytes - srcRowBytes);
	    }
	}
    }

#if COUNT_EM
    /* Initialize array "planes" with bitmap ptrs; NULL in empty slots. */
    for (iPlane = 0; iPlane < bitmap->Depth; iPlane++)
    {
	printf ("Plane %2ld: %ld\n", (ULONG)iPlane, plane_use[iPlane]);
    }
    printf ("\n");
#endif

    return (TRUE);
}
