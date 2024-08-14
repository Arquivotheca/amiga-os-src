/* dispatch.c
 *
 */

#include "classbase.h"

/*****************************************************************************/

typedef UBYTE PIXEL;

/*****************************************************************************/

#define	SWAPW(a)	(WORD)(((UWORD)a>>8)+((((UWORD)a&0xff)<<8)))
#define	SWAPU(a)	(UWORD)(((UWORD)a>>8)+((((UWORD)a&0xff)<<8)))
#define	SWAPL(a)	(LONG)(((ULONG)a>>24)+(((ULONG)a&0xff0000)>>8)+(((ULONG)a&0xff00)<<8)+(((ULONG)a&0xff)<<24))

/*****************************************************************************/

#define	BMP_ID	0x424D

/*****************************************************************************/

struct BitMapFile
{
    UWORD bfType;
    LONG  bfSize;
    UWORD bfReserved1;
    UWORD bfReserved2;
    LONG  bfOffbits;
};

#define	BFSIZE	(sizeof (struct BitMapFile))

/*****************************************************************************/

struct BitMapInfo
{
    LONG biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    LONG biCompression;
    LONG biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    LONG biClrUsed;
    LONG biClrImportant;
};

#define	BISIZE	(sizeof (struct BitMapInfo))

/*****************************************************************************/

struct RGBQuad
{
    UBYTE rgbBlue;
    UBYTE rgbGreen;
    UBYTE rgbRed;
    UBYTE rgbReserved;
};

#define	QSIZE	(sizeof (struct RGBQuad))

/*****************************************************************************/

ULONG setdtattrs (struct ClassBase * cb, Object * o, ULONG data,...)
{

    return (SetDTAttrsA (o, NULL, NULL, (struct TagItem *) & data));
}

/*****************************************************************************/

ULONG getdtattrs (struct ClassBase * cb, Object * o, ULONG data,...)
{

    return (GetDTAttrsA (o, (struct TagItem *) & data));
}

/*****************************************************************************/

Class *initClass (struct ClassBase * cb)
{
    Class *cl;

    /* Create our class.  Note that this particular class doesn't have any instance data */
    if (cl = MakeClass (BMPDTCLASS, PICTUREDTCLASS, NULL, NULL, 0L))
    {
	cl->cl_Dispatcher.h_Entry = Dispatch;
	cl->cl_UserData = (ULONG) cb;
	AddClass (cl);
    }

    return (cl);
}

/*****************************************************************************/

ULONG ASM Dispatch (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct ClassBase *cb = (struct ClassBase *) cl->cl_UserData;
    ULONG retval;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (retval = DoSuperMethodA (cl, o, msg))
	    {
		if (!GetBMP (cb, cl, (Object *) retval, ((struct opSet *) msg)->ops_AttrList))
		{
		    CoerceMethod (cl, (Object *) retval, OM_DISPOSE);
		    return NULL;
		}
	    }
	    break;

	    /* Let the superclass handle everything else */
	default:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}

/*****************************************************************************/

BOOL ASM GetBMP (REG (a6) struct ClassBase * cb, REG (a0) Class * cl, REG (a2) Object * o, REG (a1) struct TagItem * attrs)
{
    struct BitMapHeader *bmhd;
    BOOL success = FALSE;
    PIXEL *albuff = NULL;
    struct BitMapFile bf;
    struct BitMapInfo bi;
    STRPTR buffer, lbuff;
    struct RastPort trp;
    struct BitMap *tbm;
    struct RGBQuad rgb;
    struct RastPort rp;
    struct BitMap *bm;
    LONG i, j, k, l;
    PIXEL *tlbuff;
    ULONG modeid;
    STRPTR title;
    LONG large;
    LONG w, y;
    BPTR fh;

    struct ColorRegister *cmap;
    WORD n, ncolors;
    LONG *cregs;

    WORD bitcnt;

    /* Get the default title */
    title = (STRPTR) GetTagData (DTA_Name, NULL, attrs);

    /* Get the file handle to read from and the BitMapHeader to write to */
    if ((getdtattrs (cb, o, DTA_Handle, &fh, PDTA_BitMapHeader, &bmhd, TAG_DONE) == 2) && fh)
    {
	/* Make sure we are at the beginning of the file */
	Seek (fh, 0L, OFFSET_BEGINNING);

	/* Read the BitMapFile header */
	if (Read (fh, &bf, BFSIZE) == BFSIZE)
	{
	    /* Make sure we are the right type (even though DataTypes already did it) */
	    if (bf.bfType == BMP_ID)
	    {
		/* Read the BitMapInfo information */
		if (Read (fh, &bi, BISIZE) == BISIZE)
		{
		    /* We can't handle any compression... */
		    if (bi.biCompression)
		    {
			SetIoErr (ERROR_OBJECT_WRONG_TYPE);
			return (FALSE);
		    }

		    /* How many bits per pixel */
		    bitcnt = SWAPW (bi.biBitCount);

		    /* Compute the line size */
		    j = SWAPL (bi.biHeight);
		    w = k = (((SWAPL (bi.biWidth) * (LONG) bitcnt) + 31) & 0xFFFFFFE0) >> 3;

		    /* Fill in the size information */
		    bmhd->bmh_Width  = bmhd->bmh_PageWidth = SWAPL (bi.biWidth);
		    bmhd->bmh_Height = bmhd->bmh_PageHeight = SWAPL (bi.biHeight);
		    bmhd->bmh_Depth  = (LONG) bitcnt;

		    /* We only handle 16 and 256 color modes */
		    if (bitcnt == 4)
		    {
			/* Initialize the temporary line buffer */
			w <<= 1;
			if (!(albuff = AllocVec (w, MEMF_CLEAR)))
			{
			    SetIoErr (ERROR_NO_FREE_STORE);
			    return (FALSE);
			}
		    }
		    else if (bitcnt != 8)
		    {
			SetIoErr (ERROR_OBJECT_WRONG_TYPE);
			return (FALSE);
		    }

		    /* Compute the mode ID */
		    modeid = LORES_KEY;
		    if (bmhd->bmh_Width >= 640)
			modeid = HIRES;
		    if (bmhd->bmh_Height >= 400)
			modeid |= LACE;

		    /* Find the color information */
		    Seek (fh, BFSIZE + SWAPL (bi.biSize), OFFSET_BEGINNING);

		    /* Set the number of colors */
		    ncolors = 1 << bitcnt;
		    setdtattrs (cb, o, PDTA_NumColors, ncolors, TAG_DONE);

		    /* Get the destination for the color information */
		    getdtattrs (cb, o,
				PDTA_ColorRegisters,	(ULONG) &cmap,
				PDTA_CRegs,		&cregs,
				TAG_DONE);

		    /* Set the color information */
		    for (n = 0; n < ncolors; n++)
		    {
			if (Read (fh, &rgb, QSIZE) == QSIZE)
			{
			    /* Set the master color table */
			    cmap->red = rgb.rgbRed;
			    cmap->green = rgb.rgbGreen;
			    cmap->blue = rgb.rgbBlue;
			    cmap++;

			    /* Set the color table used for remapping */
			    cregs[n * 3 + 0] = rgb.rgbRed << 24;
			    cregs[n * 3 + 1] = rgb.rgbGreen << 24;
			    cregs[n * 3 + 2] = rgb.rgbBlue << 24;
			}
			else
			    return (FALSE);
		    }

		    /* Initialize the temporary bitmap */
		    if (!(tbm = AllocBitMap (bmhd->bmh_Width, 1, bmhd->bmh_Depth, BMF_CLEAR, NULL)))
		    {
			SetIoErr (ERROR_NO_FREE_STORE);
			return (FALSE);
		    }
		    InitRastPort (&trp);
		    trp.BitMap = tbm;

		    /* Allocate the bitmap */
		    if (bm = AllocBitMap (bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, BMF_CLEAR, NULL))
		    {
			/* Initialize the temporary bitmap */
			InitRastPort (&rp);
			rp.BitMap = bm;

			/* Allocate a temporary line buffer.  Try to read it into one large buffer. */
			large = (LONG) (k * j);
			if (buffer = AllocVec (large, NULL))
			{
			    if (Read (fh, buffer, large) != large)
			    {
				FreeVec (buffer);
				buffer = NULL;
			    }
			}
			else
			{
			    buffer = AllocVec (k, NULL);
			    large = 0;
			}

			/* Make sure we have the line buffer */
			if (lbuff = buffer)
			{
			    /* Step through the lines, converting chunky to planer */
			    for (i = 0, y = (j - 1); i < j; i++, y--)
			    {
				/* Get the next line's data */
				if (large)
				    lbuff = &buffer[i * k];
				else
				    Read (fh, (lbuff = buffer), k);

				/* 4-bit pixels? */
				if (bitcnt == 4)
				{
				    /* Convert the line from 4-bits to 8-bits */
				    for (l = 0, tlbuff = albuff; l < k; l++)
				    {
					*tlbuff++ = (PIXEL) ((lbuff[l] & 0xF0) >> 4);
					*tlbuff++ = (PIXEL) (lbuff[l] & 0xF);
				    }
				    lbuff = (STRPTR) albuff;
				}

				/* Write the chunky line to the rastport */
				WritePixelLine8 (&rp, 0, y, w, lbuff, &trp);
			    }

			    /* Set the attributes */
			    setdtattrs (cb, o,
					DTA_ObjName,		title,
					DTA_NominalHoriz,	bmhd->bmh_Width,
					DTA_NominalVert,	bmhd->bmh_Height,
					PDTA_BitMap,		bm,
					PDTA_ModeID,		modeid,
					TAG_DONE);

			    /* Indicate success */
			    success = TRUE;
			}
			else
			    SetIoErr (ERROR_NO_FREE_STORE);

			/* Free the bitmaps */
			FreeBitMap (tbm);
			FreeVec (albuff);
			if (!success)
			    FreeBitMap (bm);
		    }
		    else
			SetIoErr (ERROR_NO_FREE_STORE);
		}
	    }
	    else
		SetIoErr (ERROR_OBJECT_WRONG_TYPE);
	}
    }

    return (success);
}
