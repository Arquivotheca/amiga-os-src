/* animr.c
 *
 */

#include "classbase.h"

/*****************************************************************************/

#define	DB(x)	;
#define	DC(x)	;

/*****************************************************************************/

LONG ASM DecodeVKPlane (REG (a0) unsigned char *in, REG (a2) unsigned char *out, REG (d2) LONG linebytes, REG (a3) WORD * ytable);
LONG ASM DecodeXORPlane (REG (a0) unsigned char *in, REG (a2) unsigned char *out, REG (d2) LONG linebytes, REG (a3) WORD * ytable);
LONG ASM Decode8VKPlane (REG (a0) unsigned char *in, REG (a2) unsigned char *out, REG (d2) LONG BytesPerRow, REG (a3) WORD *ytable, REG (d4) LONG xcount);
void ASM PlayDLTA(REG (a2) struct BitMap *bm, REG (a1) LONG *buff, REG (a3) WORD *table, REG (d4) UWORD ImageWidth, REG (d5) UWORD ImageDepth);

/*****************************************************************************/

LONG LoadDelta (struct ClassBase * cb, struct IFFHandle * iff, WORD * ytable,
		 struct BitMap * bm, struct AnimHeader * ah, LONG * deltalong)
{
    BYTE *deltabyte = (BYTE *) deltalong;
    unsigned char *in, *out;
    struct ContextNode *cn;
    LONG i, x, linebytes;
    LONG retval = 0;
    BOOL xor;

    /* Get a pointer to the current chunk */
    cn = CurrentChunk (iff);

    /* Make sure we have a delta chunk */
    if ((cn->cn_Type == ID_ILBM) && (cn->cn_ID == ID_DLTA))
    {
	/* Read the whole delta into our buffer */
	if (ReadChunkBytes (iff, deltalong, cn->cn_Size) == cn->cn_Size)
	{
	    if (ah->ah_Operation == 8)
	    {
#if 0
#if 0
		/* Get the bytes per line for the destination bitmap */
		linebytes = (LONG) bm->BytesPerRow;

		/* Get the width */
		x = GetBitMapAttr (bm, BMA_WIDTH);

		/* Step through the planes */
		for (i = 0; i < bm->Depth; i++)
		{
		    if (deltalong[i])
		    {
			in  = (unsigned char *) (deltabyte + deltalong[i]);
			out = (unsigned char *) (bm->Planes[i]);

kprintf ("%08lx, %08lx, %ld, %08lx, %ld : ", in, out, linebytes, ytable, x);
			Decode8VKPlane (in, out, linebytes, ytable, x);
kprintf ("done\n");
		    }
		}
#else
		if (ah->ah_Flags & 1)
		{
		    PlayDLTA (bm, deltalong, ytable, ((UWORD)(GetBitMapAttr (bm, BMA_WIDTH))), (UWORD) bm->Depth);
		}
#endif
#endif
	    }
	    else
	    {
		/* Are we XOR or not */
		xor = FALSE;
		/* DPaint anim brush */
		if ((ah->ah_Operation == 5) && (ah->ah_Interleave == 1))
		    xor = TRUE;
		else if ((ah->ah_Operation == 5) && (ah->ah_Flags & 1))
		    xor = TRUE;
		else if (ah->ah_Operation == 1)
		    xor = TRUE;

		/* Get the bytes per line for the destination bitmap */
		linebytes = (LONG) bm->BytesPerRow;

		/* Step through the planes */
		for (i = 0; i < bm->Depth; i++)
		{
		    if (deltalong[i])
		    {
			in = (unsigned char *) (deltabyte + deltalong[i]);

			out = (unsigned char *) (bm->Planes[i]);
			DB (kprintf ("%ld) %08lx (%ld), %08lx %s\n", i, in, deltalong[i], out, ((xor) ? "XOR" : "Opt5")));
			if (xor)
			    DecodeXORPlane (in, out, linebytes, ytable);
			else
			    DecodeVKPlane (in, out, linebytes, ytable);
		    }
		}
		DB (kprintf ("\n"));
	    }
	    retval = 1;
	}
    }
    return (retval);
}

/*****************************************************************************/

WORD *AllocYTable (struct ClassBase * cb, struct BitMap * bm)
{
    WORD linebytes;
    WORD *pt, acc;
    WORD *ytable;
    WORD height;
    ULONG msize;

    height = (WORD) bm->Rows;
    linebytes = (WORD) bm->BytesPerRow;
    msize = (LONG) height * sizeof (WORD);

    if (ytable = AllocVec (msize, MEMF_PUBLIC))
    {
	pt = ytable;
	acc = 0;

	while (--height >= 0)
	{
	    *pt++ = acc;
	    acc += linebytes;
	}
    }
    return (ytable);
}

/*****************************************************************************/

VOID FreeYTable (struct ClassBase * cb, WORD * ytable)
{

    FreeVec (ytable);
}
