/* dispatch.c
 *
 */

#include "classbase.h"

#define	DB(x)	;
#define	DC(x)	;
#define	DP(x)	;

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
    LONG bfSize;
    UWORD bfReserved1;
    UWORD bfReserved2;
    LONG bfOffbits;
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

ULONG setdtattrs (struct ClassBase *cb, Object * o, ULONG data,...)
{
    return (SetDTAttrsA (o, NULL, NULL, (struct TagItem *) & data));
}

/*****************************************************************************/

ULONG getdtattrs (struct ClassBase *cb, Object * o, ULONG data,...)
{
    return (GetDTAttrsA (o, (struct TagItem *) & data));
}

/*****************************************************************************/

Class *initClass (struct ClassBase *cb)
{
    Class *cl;

    if (cl = MakeClass (BMPDTCLASS, PICTUREDTCLASS, NULL, NULL, 0L))
    {
	cl->cl_Dispatcher.h_Entry = Dispatch;
	cl->cl_UserData           = (ULONG) cb;
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

BOOL ASM GetBMP (REG (a6) struct ClassBase *cb, REG (a0) Class * cl, REG (a2) Object * o, REG (a1) struct TagItem * attrs)
{
    struct BitMapHeader *bmhd;
    ULONG modeid = HIRES_KEY;
    struct BitMapFile bf;
    struct BitMapInfo bi;
    struct RGBQuad rgb;
    struct RastPort rp;
    LONG i, j, k, l, m;
    struct BitMap *bm;
    STRPTR buffer;
    STRPTR title;
    LONG x, y;
    BPTR fh;

    title = (STRPTR) GetTagData (DTA_Name, NULL, attrs);

    if ((getdtattrs (cb, o, DTA_Handle, &fh, PDTA_BitMapHeader, &bmhd, TAG_DONE) == 2) && fh)
    {
	Seek (fh, 0L, OFFSET_BEGINNING);

	    if (Read (fh, &bf, BFSIZE) == BFSIZE)
	    {
		if (bf.bfType == BMP_ID)
		{
		    DB (kprintf ("  file size: %ld\n", SWAPL (bf.bfSize)));
		    DB (kprintf ("   ofs bits: %ld\n", SWAPL (bf.bfOffbits)));

		    if (Read (fh, &bi, BISIZE) == BISIZE)
		    {
			if (bi.biCompression)
			{
			    SetIoErr (ERROR_OBJECT_WRONG_TYPE);
			    return (FALSE);
			}

			bmhd->bmh_Width  = bmhd->bmh_PageWidth  = SWAPL (bi.biWidth);
			bmhd->bmh_Height = bmhd->bmh_PageHeight = SWAPL (bi.biHeight);
			bmhd->bmh_Depth  = (LONG) SWAPW (bi.biBitCount);

			DB (kprintf ("       size: %ld (%ld)\n", SWAPL (bi.biSize), BISIZE));
                        DB (kprintf ("      width: %ld\n", bmhd->bmh_Width));
			DB (kprintf ("     height: %ld\n", bmhd->bmh_Height));
			DB (kprintf ("     planes: %d\n",  SWAPW (bi.biPlanes)));
			DB (kprintf ("  bit count: %d (", SWAPW (bi.biBitCount)));
			switch (SWAPW (bi.biBitCount))
			{
			    case 1:
				DB (kprintf ("monochrome)\n"));
				SetIoErr (ERROR_OBJECT_WRONG_TYPE);
				return (FALSE);
				break;

			    case 4:
				DB (kprintf ("16 color)\n"));
				break;

			    case 8:
				DB (kprintf ("256 color)\n"));
				break;

			    case 24:
				DB (kprintf ("true color)\n"));
				SetIoErr (ERROR_OBJECT_WRONG_TYPE);
				return (FALSE);
				break;
			}

			DB (kprintf ("compression: %ld\n", SWAPL (bi.biCompression)));
			DB (kprintf (" size image: %ld\n", SWAPL (bi.biSizeImage)));
			DB (kprintf ("       XPPM: %ld\n", SWAPL (bi.biXPelsPerMeter)));
			DB (kprintf ("       YPPM: %ld\n", SWAPL (bi.biYPelsPerMeter)));
			DB (kprintf ("colors used: %ld\n", SWAPL (bi.biClrUsed)));
			DB (kprintf ("  important: %ld\n", SWAPL (bi.biClrImportant)));
			DB (kprintf ("\n"));

			Seek (fh, BFSIZE + SWAPL (bi.biSize), OFFSET_BEGINNING);
			if (SWAPW (bi.biBitCount) != 24)
			{
			    struct ColorRegister *cmap;
			    WORD i, ncolors;
			    LONG *cregs;

			    ncolors = 1 << SWAPW (bi.biBitCount);

			    setdtattrs (cb, o, PDTA_NumColors, ncolors, TAG_DONE);

			    getdtattrs (cb, o,
					PDTA_ColorRegisters,	(ULONG)&cmap,
					PDTA_CRegs,		&cregs,
					TAG_DONE);

			    for (i = 0; i < ncolors; i++)
			    {
				if (Read (fh, &rgb, QSIZE) == QSIZE)
				{
				    DC (kprintf ("             0x%02lx) r=0x%02x g=0x%02x b=0x%02x\n", i, rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue));
				    /* Set the master color table */
				    cmap->red   = rgb.rgbRed;
				    cmap->green = rgb.rgbGreen;
				    cmap->blue  = rgb.rgbBlue;
				    cmap++;

				    /* Set the color table used for remapping */
				    cregs[i*3+0] = rgb.rgbRed   << 24;
				    cregs[i*3+1] = rgb.rgbGreen << 24;
				    cregs[i*3+2] = rgb.rgbBlue  << 24;
				}
				else
				{
				    DB (kprintf ("error reading color table!\n"));
				}
			    }
			}

			if (bm = AllocBitMap (bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, BMF_CLEAR, NULL))
			{
			    InitRastPort (&rp);
			    rp.BitMap = bm;

			    j = SWAPL (bi.biHeight);
			    k = (((SWAPL (bi.biWidth) * (LONG) SWAPW (bi.biBitCount)) + 31) & 0xFFFFFFE0) >> 3;
			    DB (kprintf ("%ld byte lines\n", k));

			    setdtattrs (cb, o,
					DTA_ObjName,		title,
					DTA_NominalHoriz,	bmhd->bmh_Width,
					DTA_NominalVert,	bmhd->bmh_Height,
					PDTA_BitMap,		bm,
					PDTA_ModeID,		modeid,
					TAG_DONE);

			    if (buffer = AllocMem (k, MEMF_CLEAR))
			    {
				UBYTE pixel, color;
				WORD bitcnt;

				bitcnt = SWAPW (bi.biBitCount);

				for (i = 0, y = (j - 1); i < j; i++, y--)
				{
				    if (Read (fh, buffer, k) == k)
				    {
					for (l = x = 0; l < k; l++)
					{
					    pixel = (UBYTE) buffer[l];

					    switch (bitcnt)
					    {
						case 4:
						    color = (pixel & 0xF0) >> 4;
						    DP (kprintf ("%lx", (LONG)color));
						    SetAPen (&rp, color);
						    WritePixel (&rp, x, y);
						    x++;

						    color = (pixel & 0xF);
						    DP (kprintf ("%lx", (LONG)color));
						    SetAPen (&rp, color);
						    WritePixel (&rp, x, y);
						    x++;
						    break;

						case 8:
						    SetAPen (&rp, pixel);
						    WritePixel (&rp, x, y);
						    x++;
						    break;
					    }
					}
					DP (kprintf ("\n"));
				    }
				    else
				    {
					DB (kprintf ("out of data at %ld\n", i));
				    }
				}
				FreeMem (buffer, k);
			    }
			    return (TRUE);
			}
			else
			{
			    DB (kprintf ("couldn't allocate bitmap\n"));
			}
		    }
		    else
		    {
			DB (kprintf ("couldn't read information\n"));
		    }
		}
		else
		{
		    DB (kprintf ("not a DIB!\n"));
		}
	    }
	    else
	    {
		DB (kprintf ("couldn't read header\n"));
	    }
    }

    return (FALSE);
}
