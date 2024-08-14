/* dispatch.c
 *
 */

#include "classbase.h"

/*****************************************************************************/

#define	DB(x)	;
#define	DT(x)	;

/*****************************************************************************/

#define	MAX_BMAPS	3

/*****************************************************************************/

struct localData
{
    struct IFFHandle		*lod_IFF;		/* IFF Handle */

    /* Frame Information */
    struct BitMap		*lod_KeyFrame;		/* Key frame */
    UWORD			 lod_Width;
    UWORD			 lod_Height;
    UWORD			 lod_Depth;
    UWORD			 lod_Pad;
    LONG			 lod_CurDelta;		/* The current delta */
    LONG			 lod_NumFrames;		/* Number of frames */

    /* Audio Information */
    UBYTE			*lod_Sample;
    ULONG			 lod_SampleLength;
    ULONG			 lod_Period;

    /* Delta Information */

    LONG			*lod_DBuffer;		/* Delta buffer */
    WORD			*lod_YTable;		/* Y table */
    ULONG			 lod_Current;
    struct BitMap		*lod_CurBMap;		/* The current delta bitmap */
    struct BitMap		*lod_DBMap[MAX_BMAPS];	/* Delta bitmaps */

    struct SignalSemaphore	 lod_Lock;
};

/*****************************************************************************/

#define	G(o)		((struct Gadget *)o)

/*****************************************************************************/

static ULONG setdtattrs (struct ClassBase * cb, Object * o, ULONG data,...)
{
    return (SetDTAttrsA (o, NULL, NULL, (struct TagItem *) & data));
}

/*****************************************************************************/

static ULONG getdtattrs (struct ClassBase * cb, Object * o, ULONG data,...)
{
    return (GetDTAttrsA (o, (struct TagItem *) & data));
}

/*****************************************************************************/

static void copybitmap (struct ClassBase * cb, struct BitMap * bm1, struct BitMap * bm2)
{
    ULONG bpr1 = bm1->BytesPerRow;
    ULONG bpr2 = bm2->BytesPerRow;
    ULONG width;
    UBYTE *src;
    UBYTE *dst;
    LONG r;
    LONG p;

    width = GetBitMapAttr (bm1, BMA_WIDTH) / 8;

    for (p = bm1->Depth - 1; p >= 0; p--)
    {
	src = (BYTE *) bm1->Planes[p];
	dst = (BYTE *) bm2->Planes[p];

	for (r = bm1->Rows; r > 0; r--)
	{
	    CopyMem (src, dst, width);
	    src += bpr1;
	    dst += bpr2;
	}
    }
}

/*****************************************************************************/

static void freebitmap (struct ClassBase *cb, struct BitMap *bm)
{
    ULONG i;

    if (bm)
    {
	/* Free the planes */
	for (i = 0; i < (ULONG)bm->Depth; i++)
	    FreeVec (bm->Planes[i]);

	/* Free the bitmap */
	FreeVec (bm);
    }
}

/*****************************************************************************/

static struct BitMap *allocbitmap (struct ClassBase *cb, ULONG w, ULONG h, ULONG d, ULONG flags, struct BitMap *fbm)
{
    struct BitMap *bm;
    ULONG mflags;
    ULONG msize;
    ULONG i;

    if (bm = AllocVec (sizeof (struct BitMap), MEMF_CLEAR))
    {
	/* We be all */
	mflags = MEMF_CLEAR;
	if (flags & BMF_DISPLAYABLE)
	    mflags |= MEMF_CHIP;

	/* Initialize the bitmap */
	InitBitMap (bm, d, w, h);

	/* Calculate the raster size */
#if 0
	msize = RASSIZE (w, h);
#else
	msize = RASSIZE (((w + 63) & ~63), h);
#endif

	/* Allocate the planes */
	for (i = 0; i < d; i++)
	{
	    if ((bm->Planes[i] = AllocVec (msize, mflags)) == NULL)
	    {
		freebitmap (cb, bm);
		return (NULL);
	    }
	}
    }
    return (bm);
}

/*****************************************************************************/

/* Properties that we can deal with */
static LONG ilbmprops[] =
{
    ID_ILBM, ID_BMHD,
    ID_ILBM, ID_CMAP,
    ID_ILBM, ID_CAMG,
    ID_ILBM, ID_GRAB,
    ID_ILBM, ID_NAME,
};

/*****************************************************************************/

#define	MAXSRCPLANES		24
#define	BPR(w)			((w) + 15 >> 4 << 1)
#define MaxPackedSize(rowSize)  ((rowSize) + (((rowSize)+127) >> 7 ))
#define	RowBytes(w)		((((w) + 15) >> 4) << 1)
#define	ChunkMoreBytes(cn)	(cn->cn_Size - cn->cn_Scan)
#define UGetByte()		(*source++)
#define UPutByte(c)		(*dest++ = (c))

/*****************************************************************************/

#if 1
WORD __asm UnPackRow (register __a0 BYTE ** pSource,
                      register __a1 BYTE ** pDest,
                      register __d0 WORD srcBytes,
                      register __d1 WORD dstBytes);
#else
static BOOL UnPackRow (BYTE ** pSource, BYTE ** pDest, WORD srcBytes0, WORD dstBytes0)
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
#endif

/*****************************************************************************/

static LONG GetBody (struct ClassBase * cb, struct IFFHandle * iff, struct BitMapHeader * bmhd, struct BitMap * bitmap)
{
    BYTE *buf, *nullDest, *nullBuf, **pDest;
    register int iPlane, iRow, nEmpty;
    BYTE *planes[MAXSRCPLANES];			/* array of ptrs to planes & mask */
    struct ContextNode *cn;
    register WORD nFilled;
    STRPTR buffer, anchor;
    UBYTE srcPlaneCnt;
    WORD srcRowBytes;
    WORD destRowBytes;
    LONG bufRowBytes;
    WORD compression;
    LONG error = 0;
    ULONG bufsize;
    int nRows;

    srcRowBytes  = RowBytes (bmhd->bmh_Width);
    bufsize = MaxPackedSize (srcRowBytes) << 4;
    if (anchor = buffer = AllocVec (bufsize, MEMF_CLEAR))
    {
	cn = CurrentChunk (iff);

	compression  = bmhd->bmh_Compression;
	if (compression <= cmpByteRun1)
	{
	    srcPlaneCnt  = bmhd->bmh_Depth;		/* Haven't counted for mask plane yet */
	    destRowBytes = bitmap->BytesPerRow;
	    bufRowBytes  = MaxPackedSize (srcRowBytes);
	    nRows        = bmhd->bmh_Height;

	    /* Complain if client asked for a conversion GetBODY doesn't handle. */
	    if ((srcRowBytes <= bitmap->BytesPerRow) && (bufsize >= bufRowBytes * 2) && (srcPlaneCnt <= MAXSRCPLANES))
	    {
		if (nRows > bitmap->Rows)
		    nRows = bitmap->Rows;

		/* Initialize array "planes" with bitmap ptrs; NULL in empty slots. */
		for (iPlane = 0; iPlane < bitmap->Depth; iPlane++)
		    planes[iPlane] = (BYTE *) bitmap->Planes[iPlane];
		for (; iPlane < MAXSRCPLANES; iPlane++)
		    planes[iPlane] = NULL;

		/* Copy any mask plane ptr into corresponding "planes" slot. */
		if (bmhd->bmh_Masking == mskHasMask)
		{
		    planes[srcPlaneCnt] = NULL;	/* In case more dstPlanes than src. */
		    srcPlaneCnt += 1;	/* Include mask plane in count. */
		}

		/* Setup a sink for dummy destination of rows from unwanted planes. */
		nullDest = buffer;
		buffer += srcRowBytes;
		bufsize -= srcRowBytes;

		/* Read the BODY contents into client's bitmap.
		 * De-interleave planes and decompress rows.
		 * MODIFIES: Last iteration modifies bufsize. */
		buf = buffer + bufsize;	/* Buffer is currently empty. */
		for (iRow = nRows; iRow > 0; iRow--)
		{
		    for (iPlane = 0; iPlane < srcPlaneCnt; iPlane++)
		    {
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
			    CopyMem (buf, buffer, nFilled);	/* Could be moving 0 bytes. */

			    if (nEmpty > ChunkMoreBytes (cn))
			    {
				/* There aren't enough bytes left to fill the buffer. */
				nEmpty = ChunkMoreBytes (cn);
				bufsize = nFilled + nEmpty;	/* heh-heh */
			    }

			    /* Append new data to the existing data. */
			    if (ReadChunkBytes (iff, &buffer[nFilled], nEmpty) < nEmpty)
			    {
				error = DTERROR_INVALID_DATA;
				goto errorOut;
			    }

			    buf = buffer;
			    nFilled = bufsize;
			}

			/* Copy uncompressed row to destination plane. */
			if (compression == cmpNone)
			{
			    if (nFilled < srcRowBytes)
			    {
				error = DTERROR_INVALID_DATA;
				goto errorOut;
			    }

			    CopyMem (buf, *pDest, srcRowBytes);
			    buf += srcRowBytes;
			    *pDest += destRowBytes;
			}
			else
			{
			    /* Decompress row to destination plane. */
			    if (UnPackRow (&buf, pDest, nFilled, srcRowBytes))
			    {
				/* pSource, pDest, srcBytes, dstBytes  */
				error = DTERROR_INVALID_DATA;
				goto errorOut;
			    }
			    else
			    {
				*pDest += (destRowBytes - srcRowBytes);
			    }
			}
		    }
		}
	    }
	    else
		error = DTERROR_INVALID_DATA;
	}
	else
	    error = DTERROR_UNKNOWN_COMPRESSION;

errorOut:
	FreeVec (anchor);
    }
    else
	error = ERROR_NO_FREE_STORE;

    SetIoErr (error);
    return (error);
}

/*****************************************************************************/

struct IFFHeader
{
    UBYTE ih_Type[4];		/* File type (FORM, LIST or CAT) */
    ULONG ih_Length;		/* File length */
    UBYTE ih_Form[4];		/* Form type */
};

#define	IH_SIZE	(sizeof (struct IFFHeader))

/*****************************************************************************/

static LONG CountFrames (struct ClassBase * cb, BPTR fh)
{
    struct IFFHeader ih;
    BOOL going = TRUE;
    LONG frames = -1;
    LONG len;

    /* Finding the file length using this method forces the file system to load
     * the entire chain into memory. */
    if (Seek (fh, 0, OFFSET_END) >= 0)
    {
	if ((len = Seek (fh, 0, OFFSET_BEGINNING)) >= 0)
	{
	    if (Read (fh, &ih, IH_SIZE) == IH_SIZE)
	    {
		if ((Strnicmp (ih.ih_Type, "FORM", 4) == 0) && (Strnicmp (ih.ih_Form, "ANIM", 4) == 0))
		{
		    if (ih.ih_Length + 8 != len)
		    {
			SetIoErr (DTERROR_INVALID_DATA);
			return -1;
		    }

		    frames = 0;
		    while ((Read (fh, &ih, IH_SIZE) == IH_SIZE) && going)
		    {
			if (Seek (fh, ih.ih_Length - sizeof (ULONG), OFFSET_CURRENT) == -1)
			    going = FALSE;
			else
			    frames++;
		    }
		}
		else
		    SetIoErr (DTERROR_INVALID_DATA);
	    }
	    else
		SetIoErr (DTERROR_INVALID_DATA);
	}
    }

    return (frames);
}

/*****************************************************************************/

static ULONG InitializeData (struct ClassBase * cb, Class * cl, Object * o, struct TagItem * attrs)
{
    struct localData *lod = INST_DATA (cl, o);
    struct StoredProperty *sp;
    struct BitMapHeader *bmhd;
    BOOL good = FALSE;
    struct BitMap *bm;
    ULONG modeid = 0;
    Point *point;
    STRPTR title;

    InitSemaphore (&lod->lod_Lock);

    title = (STRPTR) GetTagData (DTA_Name, NULL, attrs);

    getdtattrs (cb, o,
		DTA_Handle, &lod->lod_IFF,
		PDTA_BitMapHeader, &bmhd,
		PDTA_Grab, &point,
		TAG_DONE);

    /* IFF handle is already opened for reading */
    PropChunks (lod->lod_IFF, ilbmprops, 5);
    StopChunk (lod->lod_IFF, ID_ILBM, ID_BODY);

    if (ParseIFF (lod->lod_IFF, IFFPARSE_SCAN) == 0L)
    {
	if (sp = FindProp (lod->lod_IFF, ID_ILBM, ID_BMHD))
	{
	    CopyMem (sp->sp_Data, bmhd, sizeof (struct BitMapHeader));

	    lod->lod_Width  = (UWORD) bmhd->bmh_Width;
	    lod->lod_Height = (UWORD) bmhd->bmh_Height;
	    lod->lod_Depth  = (UWORD) bmhd->bmh_Depth;

	    if (lod->lod_Depth > 8)
	    {
		SetIoErr (ERROR_OBJECT_WRONG_TYPE);
	    }
	    else
	    {
		/* Allocate a bitmap to hold the image */
		if (lod->lod_KeyFrame = bm = allocbitmap (cb, (ULONG) lod->lod_Width, (ULONG) lod->lod_Height, (ULONG) lod->lod_Depth, BMF_DISPLAYABLE, NULL))
		{
		    if (sp = FindProp (lod->lod_IFF, ID_ILBM, ID_NAME))
			title = (STRPTR) sp->sp_Data;

		    /* Prepare the Mode ID */
		    if (sp = FindProp (lod->lod_IFF, ID_ILBM, ID_CAMG))
		    {
			modeid = *((ULONG *) sp->sp_Data);
			if ((!(modeid & MONITOR_ID_MASK)) ||
			    ((modeid & EXTENDED_MODE) &&
			     (!(modeid & 0xFFFF0000))))
			{
			    modeid &= (~(EXTENDED_MODE | SPRITES | GENLOCK_AUDIO | GENLOCK_VIDEO | VP_HIDE));
			}
			if ((modeid & 0xFFFF0000) && (!(modeid & 0x00001000)))
			{
			    modeid = 0L;
			}
		    }

		    if (modeid == 0L)
		    {
			modeid = LORES_KEY;
			if (lod->lod_Width >= 640)
			    modeid = HIRES;
			if (lod->lod_Height >= 400)
			    modeid |= LACE;
		    }

		    /* Announce our plans to the world */
		    setdtattrs (cb, o,
				DTA_ObjName,		title,
				DTA_NominalHoriz,	(ULONG) lod->lod_Width,
				DTA_NominalVert,	(ULONG) lod->lod_Height,
				DTA_TotalHoriz,		(ULONG) lod->lod_Width,
				DTA_TotalVert,		(ULONG) lod->lod_Height,
				ADTA_Width,		(ULONG) lod->lod_Width,
				ADTA_Height,		(ULONG) lod->lod_Height,
				ADTA_Depth,		(ULONG) lod->lod_Depth,
				ADTA_Frames,		(ULONG) 1,
				ADTA_KeyFrame,		lod->lod_KeyFrame,
				ADTA_ModeID,		modeid,

				TAG_DONE);

		    /* Get the color information */
		    if (sp = FindProp (lod->lod_IFF, ID_ILBM, ID_CMAP))
		    {
			struct ColorRegister *rgb, *cmap, *ccmap;
			WORD i, ncolors;
			LONG *cregs;

			i = ncolors = MAX ((sp->sp_Size / 3L), (1 << lod->lod_Depth));

			setdtattrs (cb, o, PDTA_NumColors, ncolors, TAG_DONE);

			getdtattrs (cb, o,
				    PDTA_ColorRegisters, (ULONG) & cmap,
				    PDTA_CRegs, &cregs,
				    TAG_DONE);

			rgb = (struct ColorRegister *) sp->sp_Data;
			ccmap = cmap;
			while (i--)
			{
			    *cmap = *rgb;
			    cmap++;
			    rgb++;
			}

			cmap = ccmap;
			for (i = 0; i < ncolors; i++)
			{
			    cregs[i * 3 + 0] = (LONG) cmap[i].red   << 24;
			    cregs[i * 3 + 1] = (LONG) cmap[i].green << 24;
			    cregs[i * 3 + 2] = (LONG) cmap[i].blue  << 24;
			}
		    }
		    else
		    {
			setdtattrs (cb, o, PDTA_NumColors, (1 << lod->lod_Depth), TAG_DONE);
		    }

		    /* Make sure we can get the body of the picture */
		    if (GetBody (cb, lod->lod_IFF, bmhd, lod->lod_KeyFrame) == 0)
		    {
			/* Allocate work areas */
			lod->lod_DBuffer = AllocVec (64000, NULL);
			lod->lod_YTable  = AllocYTable (cb, bm);

			/* Allocate the delta bitmaps */
			lod->lod_DBMap[0] = allocbitmap (cb, (ULONG) lod->lod_Width, (ULONG) lod->lod_Height, (ULONG) lod->lod_Depth, NULL, NULL);
			lod->lod_DBMap[1] = allocbitmap (cb, (ULONG) lod->lod_Width, (ULONG) lod->lod_Height, (ULONG) lod->lod_Depth, NULL, NULL);

			/* Make sure everything was allocated */
			if (lod->lod_DBMap[0] && lod->lod_DBMap[1] && lod->lod_DBuffer && lod->lod_YTable)
			{
			    struct AnimHeader *ah;
			    LONG fps = 10;		/* Was six */

			    /* Copy the bitmaps */
			    copybitmap (cb, bm, lod->lod_DBMap[0]);
			    copybitmap (cb, bm, lod->lod_DBMap[1]);

			    /* Close in preparation for counting and rewinding */
			    CloseIFF (lod->lod_IFF);

			    /* We want to default to one frame (the keyframe) */
			    lod->lod_NumFrames = 1;

			    /* Go back to the beginning */
			    Seek (lod->lod_IFF->iff_Stream, 0, OFFSET_BEGINNING);
			    if (OpenIFF (lod->lod_IFF, IFFF_READ) == 0)
			    {
				/* Set up the stop & prop chunks */
				StopChunk (lod->lod_IFF, ID_ILBM, ID_BODY);
				PropChunk (lod->lod_IFF, ID_ILBM, ID_ANHD);
				StopChunk (lod->lod_IFF, ID_ILBM, ID_DLTA);

				/* Skip past the body */
				ParseIFF (lod->lod_IFF, IFFPARSE_SCAN);

				if (ParseIFF (lod->lod_IFF, IFFPARSE_SCAN) != IFFERR_EOF)
				{
				    if (sp = FindProp (lod->lod_IFF, ID_ILBM, ID_ANHD))
				    {
				        ah = (struct AnimHeader *) sp->sp_Data;

					/* Jiffies are 1/60 of a second */
					DT (kprintf ("time=%ld\n", ah->ah_RelTime));

					DB (kprintf ("op=%ld, bits=%ld\n", (LONG)ah->ah_Operation, (LONG)ah->ah_Flags));

#if 1
					if (ah->ah_Operation == 5)
					    good = TRUE;
#else
					if ((ah->ah_Operation == 5) || (ah->ah_Operation == 8))
					    good = TRUE;
#endif

					if (good)
					{
					    /* Close in preparation for counting and rewinding */
					    CloseIFF (lod->lod_IFF);

					    /* Count the number of frames */
					    lod->lod_NumFrames = CountFrames (cb, lod->lod_IFF->iff_Stream);

					    /* Reopen that fucking file */
					    OpenIFF (lod->lod_IFF, IFFF_READ);

					    if (lod->lod_NumFrames > 0)
						lod->lod_CurDelta = lod->lod_NumFrames + 1;
					    else
						lod->lod_NumFrames = 1;
					}
				    }
				}
			    }

			    /* Announce the latest developments */
			    setdtattrs (cb, o,
					ADTA_Frames,		lod->lod_NumFrames,
					ADTA_FramesPerSecond,	fps,
					TAG_DONE);

			    return (TRUE);
			}
			else
			    SetIoErr (ERROR_NO_FREE_STORE);
		    }
		}
		else
		    SetIoErr (ERROR_NO_FREE_STORE);
	    }
	}
	else
	    SetIoErr (ERROR_REQUIRED_ARG_MISSING);
    }
    else
	SetIoErr (ERROR_SEEK_ERROR);

    return (FALSE);
}

/*****************************************************************************/

static BOOL LocateDelta (struct ClassBase *cb, struct localData *lod, LONG delta)
{
    struct IFFHandle *iff = lod->lod_IFF;
    struct StoredProperty *sp;
    struct AnimHeader *ah;
    struct BitMap *bm;
    BOOL going = TRUE;

    /* If they are asking for a delta that is before the current delta, then we need to rewind */
    if (delta < lod->lod_CurDelta)
    {
	/* Initialize the delta buffers */
	copybitmap (cb, lod->lod_KeyFrame, lod->lod_DBMap[0]);
	copybitmap (cb, lod->lod_KeyFrame, lod->lod_DBMap[1]);

	/* Close in preparation for counting and rewinding */
	CloseIFF (iff);

	/* Go back to the beginning */
	Seek (iff->iff_Stream, 0, OFFSET_BEGINNING);
	if (OpenIFF (iff, IFFF_READ) == 0)
	{
	    /* Set up the stop & prop chunks */
	    StopChunk (iff, ID_ILBM, ID_BODY);
	    PropChunk (iff, ID_ILBM, ID_ANHD);
	    StopChunk (iff, ID_ILBM, ID_DLTA);

	    /* Skip past the body */
	    ParseIFF (iff, IFFPARSE_SCAN);
	}

	/* Say that we are before the first delta */
	lod->lod_CurBMap = NULL;
	lod->lod_CurDelta = 0;
    }

    /* Get the required frame */
    while ((lod->lod_CurDelta < delta) && going)
    {
	/* Scan for the next delta */
	if (ParseIFF (iff, IFFPARSE_SCAN) == IFFERR_EOF)
	{
	    SetIoErr (ERROR_NO_MORE_ENTRIES);
	    going = FALSE;
	}
	else if (sp = FindProp (iff, ID_ILBM, ID_ANHD))
	{
	    /* Increment the frame counter */
	    lod->lod_CurDelta++;

	    /* Get the pointers */
	    ah = (struct AnimHeader *) sp->sp_Data;
	    bm = lod->lod_CurBMap = lod->lod_DBMap[lod->lod_Current];
	    if (ah->ah_Interleave == 0)
	    {
		lod->lod_Current++;
		if (lod->lod_Current > 1)
		    lod->lod_Current = 0;
	    }

	    /* Decode the deltas */
	    LoadDelta (cb, iff, lod->lod_YTable, bm, ah, lod->lod_DBuffer);
	}
	else
	{
	    SetIoErr (ERROR_OBJECT_NOT_FOUND);
	    going = FALSE;
	}
    }

    return going;
}

/*****************************************************************************/

static struct BitMap *loadframe (struct ClassBase *cb, Class *cl, Object *o, struct adtFrame *msg)
{
    struct localData *lod = INST_DATA (cl, o);

    ObtainSemaphore (&lod->lod_Lock);

    /* Clear the return value */
    msg->alf_BitMap = NULL;

    /* Locate the delta */
    if (LocateDelta (cb, lod, msg->alf_Frame))
    {
	if (msg->alf_Frame == 0)
	{
	    if (msg->alf_UserData != lod->lod_KeyFrame)
		freebitmap (cb, msg->alf_UserData);
	    msg->alf_UserData = msg->alf_BitMap = lod->lod_KeyFrame;
	}
	else if (lod->lod_CurBMap)
	{
	    /* Make sure we have a bitmap to copy to */
	    if ((msg->alf_UserData == NULL) || (msg->alf_UserData == lod->lod_KeyFrame))
		msg->alf_UserData = allocbitmap (cb, (ULONG) lod->lod_Width, (ULONG) lod->lod_Height, (ULONG) lod->lod_Depth, NULL, NULL);

	    /* Copy the bitmap */
	    if (msg->alf_UserData)
		copybitmap (cb, lod->lod_CurBMap, (struct BitMap *) msg->alf_UserData);
	    msg->alf_BitMap = msg->alf_UserData;
	}
    }
    else
    {
	freebitmap (cb, msg->alf_UserData);
	msg->alf_UserData = NULL;
    }

    ReleaseSemaphore (&lod->lod_Lock);

    return msg->alf_BitMap;
}

/*****************************************************************************/

static ULONG unloadframe (struct ClassBase *cb, Class *cl, Object *o, struct adtFrame *msg)
{
    struct localData *lod = INST_DATA (cl, o);

    if (msg->alf_UserData != lod->lod_KeyFrame)
	freebitmap (cb, msg->alf_UserData);
    msg->alf_UserData = NULL;
    return 0;
}

/*****************************************************************************/

static ULONG startMethod (struct ClassBase * cb, Class * cl, Object * o, struct adtStart *asa)
{
    struct localData *lod = INST_DATA (cl, o);

    ObtainSemaphore (&lod->lod_Lock);

    /* Go to where ever they tell us */
    LocateDelta (cb, (struct localData *) INST_DATA (cl, o), asa->asa_Frame);

    ReleaseSemaphore (&lod->lod_Lock);

    return (ULONG) DoSuperMethodA (cl, o, (Msg) asa);
}

/*****************************************************************************/

static ULONG ASM Dispatch (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct ClassBase *cb = (struct ClassBase *) cl->cl_UserData;
    struct localData *lod = INST_DATA (cl, o);
    ULONG retval;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (retval = DoSuperMethodA (cl, o, msg))
	    {
		if (!(InitializeData (cb, cl, (Object *) retval, ((struct opSet *) msg)->ops_AttrList)))
		{
		    CoerceMethod (cl, (Object *) retval, OM_DISPOSE);
		    retval = NULL;
		}
	    }
	    return (retval);

	case ADTM_LOADFRAME:
	    return (ULONG) loadframe (cb, cl, o, (struct adtFrame *)msg);

	case ADTM_UNLOADFRAME:
	    return (ULONG) unloadframe (cb, cl, o, (struct adtFrame *)msg);

	case ADTM_START:
	    return (ULONG) startMethod (cb, cl, o, (struct adtStart *) msg);

	case OM_DISPOSE:
	    WaitBlit ();
	    freebitmap (cb, lod->lod_KeyFrame);
	    lod->lod_KeyFrame = NULL;
	    freebitmap (cb, lod->lod_DBMap[0]);
	    lod->lod_DBMap[0] = NULL;
	    freebitmap (cb, lod->lod_DBMap[1]);
	    lod->lod_DBMap[1] = NULL;
	    FreeVec (lod->lod_DBuffer);
	    lod->lod_DBuffer = NULL;
	    FreeYTable (cb, lod->lod_YTable);
	    lod->lod_YTable = NULL;

	/* Let the superclass handle everything else */
	default:
	    return (ULONG) DoSuperMethodA (cl, o, msg);
    }
}

/*****************************************************************************/

Class *initClass (struct ClassBase * cb)
{
    Class *cl;

    if (cl = MakeClass (ANIMDTCLASS, ANIMATIONDTCLASS, NULL, sizeof (struct localData), 0L))
    {
	cl->cl_Dispatcher.h_Entry = (ULONG (*)()) Dispatch;
	cl->cl_UserData = (ULONG) cb;
	AddClass (cl);
    }

    return (cl);
}
