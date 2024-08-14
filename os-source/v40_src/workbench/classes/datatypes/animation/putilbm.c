/* putilbm.c
 *
 */

#include "classbase.h"
#include "classdata.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

/* This macro computes the worst case packed size of a "row" of bytes. */
#define MaxPackedSize(rowSize)  ( (rowSize) + ( ((rowSize)+127) >> 7 ) )

/*****************************************************************************/

/* Given POINTERS to POINTER variables, packs one row, updating the source
 * and destination pointers. Returns the size in bytes of the packed row.
 * ASSUMES destination buffer is large enough for the packed row.
 * See MaxPackedSize. */
extern LONG PackRow (BYTE ** pSource, BYTE ** pDest, LONG rowSize);

/*****************************************************************************/

/* Given POINTERS to POINTER variables, unpacks one row, updating the source
 * and destination pointers until it produces dstBytes bytes (i.e., the
 * rowSize that went into PackRow).
 * If it would exceed the source's limit srcBytes or if a run would overrun
 * the destination buffer size dstBytes, it stops and returns TRUE.
 * Otherwise, it returns FALSE (no error). */
extern BOOL UnPackRow (BYTE ** pSource, BYTE ** pDest, WORD srcBytes, WORD dstBytes);

/*****************************************************************************/

LONG PackRow (BYTE **, BYTE **, LONG);
BOOL UnPackRow (BYTE **, BYTE **, WORD, WORD);

/*****************************************************************************/

static BYTE *PutDump (BYTE *, int);
static BYTE *PutRun (BYTE *, int, int);

/*****************************************************************************/

#define DUMP	0
#define RUN	1

/*****************************************************************************/

#define MinRun 3
#define MaxRun 128
#define MaxDat 128

/*****************************************************************************/

/* When used on global definitions, static means private.
 * This keeps these names, which are only referenced in this
 * module, from conficting with same-named objects in your program.
 */
static LONG putSize;
static char buf[256];		/* [TBD] should be 128?  on stack? */

/*****************************************************************************/

#define GetByte()	(*source++)
#define PutByte(c)	{ *dest++ = (c);   ++putSize; }


/*****************************************************************************/

static BYTE *PutDump (BYTE * dest, int nn)
{
    int i;

    PutByte (nn - 1);
    for (i = 0; i < nn; i++)
	PutByte (buf[i]);
    return (dest);
}

/*****************************************************************************/

static BYTE *PutRun (BYTE * dest, int nn, int cc)
{

    PutByte (-(nn - 1));
    PutByte (cc);
    return (dest);
}

/*****************************************************************************/

#define OutDump(nn)   dest = PutDump(dest, nn)
#define OutRun(nn,cc) dest = PutRun(dest, nn, cc)

/*****************************************************************************/

/*----------- packrow --------------------------------------------------*/
/* Given POINTERS TO POINTERS, packs one row, updating the source and
 * destination pointers.  RETURNs count of packed bytes.
 */
LONG packrow (BYTE ** pSource, BYTE ** pDest, LONG rowSize)
{
    BYTE *source, *dest;
    char c, lastc = '\0';
    BOOL mode = DUMP;
    short nbuf = 0;		/* number of chars in buffer */
    short rstart = 0;		/* buffer index current run starts */

    source = *pSource;
    dest = *pDest;
    putSize = 0;
    buf[0] = lastc = c = GetByte ();	/* so have valid lastc */
    nbuf = 1;
    rowSize--;			/* since one byte eaten. */

    for (; rowSize; --rowSize)
    {
	buf[nbuf++] = c = GetByte ();
	switch (mode)
	{
	    case DUMP:
		/* If the buffer is full, write the length byte, then the data */
		if (nbuf > MaxDat)
		{
		    OutDump (nbuf - 1);
		    buf[0] = c;
		    nbuf = 1;
		    rstart = 0;
		    break;
		}

		if (c == lastc)
		{
		    if (nbuf - rstart >= MinRun)
		    {
			if (rstart > 0)
			    OutDump (rstart);
			mode = RUN;
		    }
		    else if (rstart == 0)
			mode = RUN;	/* no dump in progress, so can't lose by making these 2 a run. */
		}
		else
		    rstart = nbuf - 1;	/* first of run */
		break;

	    case RUN:
		if ((c != lastc) || (nbuf - rstart > MaxRun))
		{
		    /* output run */
		    OutRun (nbuf - 1 - rstart, lastc);
		    buf[0] = c;
		    nbuf = 1;
		    rstart = 0;
		    mode = DUMP;
		}
		break;
	}

	lastc = c;
    }

    switch (mode)
    {
	case DUMP:
	    OutDump (nbuf);
	    break;
	case RUN:
	    OutRun (nbuf - rstart, lastc);
	    break;
    }
    *pSource = source;
    *pDest = dest;
    return (putSize);
}

/*****************************************************************************/

/*---------- putbody ---------------------------------------------------*/
/* NOTE: This implementation could be a LOT faster if it used more of the
 * supplied buffer. It would make far fewer calls to IFFWriteBytes (and
 * therefore to DOS Write). */
BOOL PutBody (struct ClassBase * cb, struct IFFHandle * iff,
	      struct BitMapHeader * bmhd, struct BitMap * bm)
{
    LONG rowBytes = (LONG) bm->BytesPerRow;
    UBYTE compression = bmhd->bmh_Compression;
    register LONG packedRowBytes;
    int depth = bmhd->bmh_Depth;
    register int plane, row;
    BOOL retval = FALSE;
    BOOL error = FALSE;
    BYTE *planes[25];
    STRPTR buffer;
    ULONG bufsize;
    BYTE *buf;

    DB (kprintf ("BytesPerRow=%ld, Rows=%ld, Depth=%ld, Flags=%02lx\n",
	     (ULONG) bm->BytesPerRow, (ULONG) bm->Rows, (ULONG)bm->Depth, (ULONG)bm->Flags));

    bufsize = MaxPackedSize (rowBytes) + 2;
    if (buffer = AllocMem (bufsize, MEMF_CLEAR))
    {
	/* Copy the pointers to the bit planes into the local array */
	for (plane = 0; plane < depth; plane++)
	    planes[plane] = (BYTE *) bm->Planes[plane];

	/* Write out a BODY chunk header */
	if (PushChunk (iff, NULL, ID_BODY, IFFSIZE_UNKNOWN) == 0)
	{
	    /* Write out the BODY contents */
	    for (row = bmhd->bmh_Height; (row > 0) && !error; row--)
	    {
		for (plane = 0; (plane < depth) && !error; plane++)
		{
		    /* Write next row. */
		    if (compression == cmpNone)
		    {
			if (WriteChunkBytes (iff, planes[plane], rowBytes) != rowBytes)
			    error = TRUE;
			planes[plane] += rowBytes;
		    }
		    /* Compress and write next row. */
		    else if (compression == cmpByteRun1)
		    {
			buf = buffer;
			packedRowBytes = packrow (&planes[plane], &buf, rowBytes);
			if (WriteChunkBytes (iff, buffer, packedRowBytes) != packedRowBytes)
			    error = TRUE;
		    }
		}
	    }
	    /* Finish the chunk */
	    if (!error && (PopChunk (iff) == 0))
		retval = TRUE;
	}

	/* Free the line buffer */
	FreeMem (buffer, bufsize);
    }
    return (retval);
}

/*****************************************************************************/

BOOL PutColors (struct ClassBase * cb, struct IFFHandle * iff, struct BitMapHeader * bmhd, struct ColorRegister * cmap, ULONG modeid)
{
    UWORD numcolors = 1 << bmhd->bmh_Depth;

    if (cmap == NULL)
	return TRUE;

    if (modeid & 0x800)
	numcolors = 1 << (bmhd->bmh_Depth - 2);
    if (PushChunk (iff, 0, ID_CMAP, IFFSIZE_UNKNOWN) == 0L)
	if ((WriteChunkRecords (iff, cmap, sizeof (struct ColorRegister), numcolors)) == numcolors)
	    if ((PopChunk (iff)) == 0L)
		return TRUE;
    return FALSE;
}

/*****************************************************************************/

BOOL WriteBMHD (struct ClassBase * cb, struct IFFHandle * iff, struct BitMapHeader * bmhd)
{
    ULONG msize = sizeof (struct BitMapHeader);

    if (PushChunk (iff, 0, ID_BMHD, msize) == 0)
	if (WriteChunkBytes (iff, bmhd, msize) == msize)
	    if (PopChunk (iff) == 0)
		return (TRUE);
    return (FALSE);
}

/*****************************************************************************/

BOOL WriteCAMG (struct ClassBase * cb, struct IFFHandle * iff, ULONG modeid)
{
    ULONG msize = sizeof (ULONG);

    if (PushChunk (iff, 0, ID_CAMG, msize) == 0)
	if (WriteChunkBytes (iff, (char *) &modeid, msize) == msize)
	    if (PopChunk (iff) == 0)
		return (TRUE);
    return (FALSE);
}

/*****************************************************************************/

BOOL WriteNAME (struct ClassBase * cb, struct IFFHandle * iff, STRPTR name)
{
    ULONG msize;

    if (!name)
	return (TRUE);
    msize = strlen (name) + 1;

    if (PushChunk (iff, 0, ID_NAME, msize) == 0)
	if (WriteChunkBytes (iff, name, msize) == msize)
	    if (PopChunk (iff) == 0)
		return (TRUE);
    return (FALSE);
}

/*****************************************************************************/

BOOL writeObject (struct ClassBase * cb, struct IFFHandle * iff, Object * o, struct localData * lod)
{
    struct BitMapHeader bmhd;
    BOOL success = FALSE;
    STRPTR title = NULL;
    struct BitMap *bm;
    struct IBox *sel;

    GetAttr (DTA_SelectDomain, o, (ULONG *) & sel);
    GetAttr (DTA_ObjName, o, (ULONG *) & title);

    /* Copy the bitmap */
    bmhd = *(&lod->lod_BMHD);

    /* Set it up */
    bmhd.bmh_Left = bmhd.bmh_Top = 0;
    if (sel)
    {
	bmhd.bmh_Left = sel->Left;
	bmhd.bmh_Top = sel->Top;
	bmhd.bmh_Width = sel->Width;
	bmhd.bmh_Height = sel->Height;
    }

    /* Compress that baby */
    bmhd.bmh_Compression = cmpByteRun1;

    /* Create a temporary bitmap */
    if (bm = AllocBitMap ((ULONG) bmhd.bmh_Width, (ULONG) bmhd.bmh_Height, (ULONG) bmhd.bmh_Depth, NULL, NULL))
    {
	/* Clip to it (lod->lod_SourceBMap) */
	BltBitMap (lod->lod_BMap, bmhd.bmh_Left, bmhd.bmh_Top,
		   bm, 0, 0, bmhd.bmh_Width, bmhd.bmh_Height,
		   0xC0, 0xFF, NULL);
	WaitBlit ();

	if (OpenIFF (iff, IFFF_WRITE) == 0)
	{
	    if (PushChunk (iff, ID_ILBM, ID_FORM, IFFSIZE_UNKNOWN) == 0)
	    {
		/* Write the title of the object */
		WriteNAME (cb, iff, title);

		if (WriteBMHD (cb, iff, &bmhd))
		    if (WriteCAMG (cb, iff, lod->lod_ModeID))
			if (PutColors (cb, iff, &bmhd, lod->lod_Colors, lod->lod_ModeID))
			    success = PutBody (cb, iff, &bmhd, bm);
		PopChunk (iff);
	    }
	    CloseIFF (iff);
	}
	FreeBitMap (bm);
    }

    return (success);
}
