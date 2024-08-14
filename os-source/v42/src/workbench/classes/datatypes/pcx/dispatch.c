/* dispatch.c
 */

#include "classbase.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

#define	SWAPW(a)	(WORD)(((UWORD)a>>8)+((((UWORD)a&0xff)<<8)))
#define	SWAPU(a)	(UWORD)(((UWORD)a>>8)+((((UWORD)a&0xff)<<8)))
#define	SWAPL(a)	(LONG)(((ULONG)a>>24)+(((ULONG)a&0xff0000)>>8)+(((ULONG)a&0xff00)<<8)+(((ULONG)a&0xff)<<24))

/*****************************************************************************/

/* Header for PCX bitmap files */
struct PCX
{
    UBYTE pcx_Zsoft;		/* PCX file identifier */
    UBYTE pcx_Version;		/* version compatibility level */
    UBYTE pcx_Encoding;		/* encoding method */
    UBYTE pcx_BitsPerPix;	/* bits per pixel, or depth */
    UWORD pcx_Xmin;		/* X position of left edge */
    UWORD pcx_Ymin;		/* Y position of top edge */
    UWORD pcx_Xmax;		/* X position of right edge */
    UWORD pcx_Ymax;		/* Y position of bottom edge */
    UWORD pcx_XDPI;		/* X screen res of source image */
    UWORD pcx_YDPI;		/* Y screen res of source image */
    UBYTE pcx_Palette[16][3];	/* PCX color map */
    UBYTE pcx_Reserved1;	/* should be 0, 1 if std res fax */
    UBYTE pcx_Planes;		/* bit planes in image */
    UWORD pcx_BytesPerLine;	/* byte delta between scanlines  */
    UWORD pcx_PaletteInfo;	/* 0 == undef, 1 == color, 2 == grayscale */
    UWORD pcx_Xsize;		/* Video screen size horizontally */
    UWORD pcx_Ysize;		/* Video screen size vertically */
    UBYTE pcx_Reserved2[54];	/* fill to struct size of 128 */
};

#define PCX_HEADERSIZE		128

#define PCX_LAST_VER		5	/* last acceptable version number */
#define PCX_RLL			1	/* PCX RLL encoding method */

#define PCX_MAGIC		0x0A	/* first byte of a PCX image */
#define PCX_256COLORS_MAGIC	0x0C	/* first byte of a PCX 256 color map */
#define	MAX_DEPTH		8	/* PCX supports up to 8 bits per pixel */
#define PCX_MAXCOLORS		256	/* maximum number of colors in a PCX file */

/*****************************************************************************/

static BOOL Get256CMap (struct ClassBase *cb, BPTR fh, struct ColorRegister *cmap, LONG * cregs)
{
    register LONG i, ch;

    if ((ch = FGetC (fh)) == PCX_256COLORS_MAGIC)
    {
	for (i = 0; i < PCX_MAXCOLORS; i++)
	{
	    if ((ch = FGetC (fh)) >= 0)
		cmap->red = ch;
	    else
		break;

	    if ((ch = FGetC (fh)) >= 0)
		cmap->green = ch;
	    else
		break;

	    if ((ch = FGetC (fh)) >= 0)
		cmap->blue = ch;
	    else
		break;

	    cregs[i * 3 + 0] = cmap->red << 24;
	    cregs[i * 3 + 1] = cmap->green << 24;
	    cregs[i * 3 + 2] = cmap->blue << 24;

	    cmap++;
	}

	if (i == PCX_MAXCOLORS)
	    return TRUE;
    }
    return FALSE;
}

/*****************************************************************************/

static BOOL ObtainData (struct ClassBase *cb, Class * cl, Object * o, struct TagItem *attrs)
{
    struct ColorRegister *cmap, *acmap;
    struct BitMapHeader *bmh;
    struct RastPort rp;
    struct BitMap *bm;
    LONG numColors;
    STRPTR title;
    LONG *cregs;
    LONG i, j;
    BPTR fh;

    struct RastPort trp;
    struct BitMap *tbm;
    STRPTR linebuffer;
    STRPTR buffer;
    LONG nbytes;

    BOOL retval = FALSE;
    struct PCX pcx;

    BOOL error = FALSE;
    LONG x, y, p, ch;
    LONG count;
    UBYTE c;

    /* Get the default title */
    title = (STRPTR) GetTagData (DTA_Name, NULL, attrs);

    /* Get the file handle to read from and the BitMapHeader to write to */
    if ((GetDTAttrs (o, DTA_Handle, &fh, PDTA_BitMapHeader, &bmh, TAG_DONE) == 2) && fh)
    {
	/* Make sure we are at the beginning of the file */
	Seek (fh, 0L, OFFSET_BEGINNING);

	/* Read the header */
	if (Read (fh, &pcx, sizeof (struct PCX)) == sizeof (struct PCX))
	{
	    /* Make sure it is a version that we can handle */
	    if ((pcx.pcx_Version == 1) || (pcx.pcx_Version > 5))
	    {
		SetIoErr (DTERROR_INVALID_DATA);
		return FALSE;
	    }

	    /* Make sure it is a compression type that we know */
	    if (pcx.pcx_Encoding != 1)
	    {
		SetIoErr (DTERROR_UNKNOWN_COMPRESSION);
		return FALSE;
	    }

	    /* Make sure the magic number is zero */
	    if (pcx.pcx_Reserved1 != 0)
	    {
		SetIoErr (DTERROR_INVALID_DATA);
		return FALSE;
	    }

	    /* Make sure it is a palette type that we can handle */
	    if (SWAPU (pcx.pcx_PaletteInfo) > 2)
	    {
		SetIoErr (DTERROR_INVALID_DATA);
		return FALSE;
	    }

	    /* Display the information */
	    DB (kprintf ("\nname:            %s\n", title));
	    DB (kprintf ("version:         %ld\n", (ULONG) pcx.pcx_Version));
	    DB (kprintf ("encoding:        %ld\n", (ULONG) pcx.pcx_Encoding));
	    DB (kprintf ("bits per pixel:  %ld\n", (ULONG) pcx.pcx_BitsPerPix));
	    DB (kprintf ("rectangle:       %ld, %ld, %ld, %ld\n",
			 (ULONG) SWAPU (pcx.pcx_Xmin), (ULONG) SWAPU (pcx.pcx_Ymin),
			 (ULONG) SWAPU (pcx.pcx_Xmax), (ULONG) SWAPU (pcx.pcx_Ymax)));
	    DB (kprintf ("dpi:             %ld, %ld\n",
			 (ULONG) SWAPU (pcx.pcx_XDPI), (ULONG) SWAPU (pcx.pcx_YDPI)));
	    DB (kprintf ("planes:          %ld\n", (ULONG) pcx.pcx_Planes));
	    DB (kprintf ("bytes per line:  %ld\n", (ULONG) SWAPU (pcx.pcx_BytesPerLine)));
	    DB (kprintf ("interpretation:  %ld\n", (ULONG) pcx.pcx_PaletteInfo));
	    DB (kprintf ("screen size:     %ld, %ld\n",
			 (ULONG) SWAPU (pcx.pcx_Xsize), (ULONG) SWAPU (pcx.pcx_Ysize)));

	    /* Fill in the bitmap header */
	    bmh->bmh_Width = bmh->bmh_PageWidth = SWAPU (pcx.pcx_Xmax) - SWAPU (pcx.pcx_Xmin) + 1;
	    bmh->bmh_Height = bmh->bmh_PageHeight = SWAPU (pcx.pcx_Ymax) - SWAPU (pcx.pcx_Ymin) + 1;
	    bmh->bmh_Depth = pcx.pcx_Planes * pcx.pcx_BitsPerPix;

	    /* Initialize the temporary bitmap */
	    if (!(tbm = AllocBitMap (bmh->bmh_Width, 1, bmh->bmh_Depth, BMF_CLEAR, NULL)))
	    {
		SetIoErr (ERROR_NO_FREE_STORE);
		return FALSE;
	    }
	    InitRastPort (&trp);
	    trp.BitMap = tbm;

	    /* Allocate the temporary line buffer */
	    i = (((LONG) bmh->bmh_Width + 15) >> 4) << 4;
	    if ((linebuffer = AllocVec (i, MEMF_CLEAR)) == NULL)
	    {
		FreeBitMap (tbm);
		SetIoErr (ERROR_NO_FREE_STORE);
		return FALSE;
	    }

	    /* Get the number of colors */
	    numColors = 1 << bmh->bmh_Depth;

	    /* Set the number of colors */
	    SetDTAttrs (o, NULL, NULL, PDTA_NumColors, numColors, TAG_DONE);

	    /* Get a pointer to the color registers */
	    GetDTAttrs (o, PDTA_ColorRegisters, (ULONG) & acmap, PDTA_CRegs, &cregs, TAG_DONE);
	    cmap = acmap;

	    /* See if we are monochrome */
	    if ((pcx.pcx_BitsPerPix == 1) && (pcx.pcx_Planes == 1))
	    {
		/* First color is white */
		cmap->red = cmap->green = cmap->blue = 0xFF;
		cregs[0] = cregs[1] = cregs[2] = 0xff << 24;
		cmap++;

		/* Second color is black */
		cmap->red = cmap->green = cmap->blue = 0;
		cregs[3] = cregs[4] = cregs[5] = 0;
	    }
	    else if (pcx.pcx_BitsPerPix < 8)
	    {
		j = 0;
		for (i = 0; i < MIN (16, numColors); i++)
		{
		    cmap->red = pcx.pcx_Palette[i][0];
		    cmap->green = pcx.pcx_Palette[i][1];
		    cmap->blue = pcx.pcx_Palette[i][2];
		    cregs[i * 3 + 0] = cmap->red << 24;
		    cregs[i * 3 + 1] = cmap->green << 24;
		    cregs[i * 3 + 2] = cmap->blue << 24;
		    j += !((cmap->red == 0) && (cmap->green == 0) && (cmap->blue == 0));
		    cmap++;
		}

		if (j == 0)
		{
		    /* First color is white */
		    cmap = acmap;
		    cmap->red = cmap->green = cmap->blue = 0xFF;
		    cregs[0] = cregs[1] = cregs[2] = 0xff << 24;
		}
	    }

	    /* Allocate the bitmap */
	    if (bm = AllocBitMap (bmh->bmh_Width, bmh->bmh_Height, bmh->bmh_Depth, BMF_CLEAR, NULL))
	    {
		/* Initialize the temporary rastport */
		InitRastPort (&rp);
		rp.BitMap = bm;

		/* Initialize a few variables */
		y = 0;
		nbytes = (LONG) SWAPU (pcx.pcx_BytesPerLine);
		p = nbytes * bmh->bmh_Height * pcx.pcx_Planes;

		/* We can decompress and write to the bitmap at the same time if it is single plane */
		if (pcx.pcx_Planes == 1)
		{
		    p = (LONG) bmh->bmh_Height;
		    while ((p--) && !error)
		    {
			x = 0;
			i = nbytes;
			while ((i > 0) && !error)
			{
			    if ((ch = FGetC (fh)) >= 0)
			    {
				c = (UBYTE) ch;

				if ((c & 0xC0) == 0xC0)
				{
				    count = (LONG) (c & 0x3F);
				    if (count > nbytes)
					error = TRUE;

				    if ((ch = FGetC (fh)) >= 0)
				    {
					c = (UBYTE) ch;
					i -= count;

					/* Push pixel into line buffer */
					switch (pcx.pcx_BitsPerPix)
					{
					    case 8:
						while (--count >= 0)
						    linebuffer[x++] = c;
						break;

					    case 4:
						while (--count >= 0)
						{
						    linebuffer[x++] = (c >> 4) & 0x0F;
						    linebuffer[x++] = c & 0x0F;
						}
						break;

					    case 2:
						while (--count >= 0)
						{
						    linebuffer[x++] = (c >> 6) & 0x03;
						    linebuffer[x++] = (c >> 4) & 0x03;
						    linebuffer[x++] = (c >> 2) & 0x03;
						    linebuffer[x++] = c & 0x03;
						}
						break;

					    case 1:
						while (--count >= 0)
						{
						    linebuffer[x++] = ((c & 0x80) ? 0 : 1);
						    linebuffer[x++] = ((c & 0x40) ? 0 : 1);
						    linebuffer[x++] = ((c & 0x20) ? 0 : 1);
						    linebuffer[x++] = ((c & 0x10) ? 0 : 1);
						    linebuffer[x++] = ((c & 0x08) ? 0 : 1);
						    linebuffer[x++] = ((c & 0x04) ? 0 : 1);
						    linebuffer[x++] = ((c & 0x02) ? 0 : 1);
						    linebuffer[x++] = ((c & 0x01) ? 0 : 1);
						}
						break;
					}
				    }
				    else
				    {
					error = TRUE;
					break;
				    }
				}
				else
				{
				    i--;

				    /* Push pixel into line buffer */
				    switch (pcx.pcx_BitsPerPix)
				    {
					case 8:
					    linebuffer[x++] = c;
					    break;

					case 4:
					    linebuffer[x++] = (c >> 4) & 0x0F;
					    linebuffer[x++] = c & 0x0F;
					    break;

					case 2:
					    linebuffer[x++] = (c >> 6) & 0x03;
					    linebuffer[x++] = (c >> 4) & 0x03;
					    linebuffer[x++] = (c >> 2) & 0x03;
					    linebuffer[x++] = c & 0x03;
					    break;

					case 1:
					    linebuffer[x++] = ((c & 0x80) ? 0 : 1);
					    linebuffer[x++] = ((c & 0x40) ? 0 : 1);
					    linebuffer[x++] = ((c & 0x20) ? 0 : 1);
					    linebuffer[x++] = ((c & 0x10) ? 0 : 1);
					    linebuffer[x++] = ((c & 0x08) ? 0 : 1);
					    linebuffer[x++] = ((c & 0x04) ? 0 : 1);
					    linebuffer[x++] = ((c & 0x02) ? 0 : 1);
					    linebuffer[x++] = ((c & 0x01) ? 0 : 1);
					    break;
				    }
				}
			    }
			    else
			    {
				error = TRUE;
				break;
			    }
			}
			WritePixelLine8 (&rp, 0, y++, (LONG) bmh->bmh_Width, linebuffer, &trp);
		    }

		    if (pcx.pcx_BitsPerPix == 8)
			Get256CMap (cb, fh, acmap, cregs);
		}
		else if (pcx.pcx_Planes > 4)
		{
		    SetIoErr (DTERROR_INVALID_DATA);
		    error = TRUE;
		}
		/* We have to unpack it */
		else if (buffer = AllocVec (p, MEMF_CLEAR))
		{
		    register STRPTR dstp;

		    /* Decompress it */
		    i = 0;
		    while (p > 0)
		    {
			if ((ch = FGetC (fh)) >= 0)
			{
			    c = (UBYTE) ch;
			    if ((c & 0xC0) == 0xC0)
			    {
				count = (LONG) (c & 0x3F);
				if (count > nbytes)
				    break;

				if ((ch = FGetC (fh)) >= 0)
				{
				    c = (UBYTE) ch;
				    p -= count;

				    while (--count >= 0)
					buffer[i++] = c;
				}
				else
				    break;
			    }
			    else
			    {
				buffer[i++] = c;
				p--;
			    }
			}
			else
			    break;
		    }

#if 0
		    if ((p == 0) && (pcx.pcx_BitsPerPix == 8))
			Get256CMap (cb, fh, acmap, cregs);
#endif

		    if (p == 0)
		    {
			register LONG i, m, nocar, nobit;

			dstp = buffer;
			m = pcx.pcx_Planes;
			p = nbytes * (LONG) pcx.pcx_Planes;
			for (y = 0; y < bmh->bmh_Height; y++)
			{
			    x = 0;
			    while (x < bmh->bmh_Width)
			    {
				c = 0;
				nocar = x / 8;
				nobit = x % 8;
				for (i = 0; i < m; i++)
				    c |= ((dstp[nocar + (i * nbytes)] & (0x80 >> nobit)) ? (1 << i) : 0);
				linebuffer[x++] = c;
			    }
			    WritePixelLine8 (&rp, 0, y, (LONG) bmh->bmh_Width, linebuffer, &trp);
			    dstp += p;
			}
		    }
		    else
			error = TRUE;
		    FreeVec (buffer);
		}
		else
		{
		    SetIoErr (ERROR_NO_FREE_STORE);
		    error = TRUE;
		}

		/* Make sure we successful */
		if (error == FALSE)
		{
		    ULONG modeid = HIRES_KEY | LACE;

		    if (bmh->bmh_Width < 640)
			modeid = LORES_KEY;

		    /* Tell the picture class about everything */
		    SetDTAttrs (o, NULL, NULL,
				DTA_ObjName,		title,
				DTA_NominalHoriz,	bmh->bmh_Width,
				DTA_NominalVert,	bmh->bmh_Height,
				PDTA_BitMap,		bm,
				PDTA_ModeID,		modeid,
				TAG_DONE);

		    /* Indicate success */
		    retval = TRUE;
		}
	    }
	    else
		SetIoErr (ERROR_NO_FREE_STORE);

	    /* Free the line buffer */
	    FreeVec (linebuffer);

	    /* Free the temporary bitmap */
	    FreeBitMap (tbm);
	}
	else
	    SetIoErr (DTERROR_NOT_ENOUGH_DATA);
    }
    else
	SetIoErr (ERROR_REQUIRED_ARG_MISSING);

    return retval;
}

/*****************************************************************************/

static ULONG ASM Dispatch (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct ClassBase *cb = (struct ClassBase *) cl->cl_UserData;
    ULONG retval;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (retval = DoSuperMethodA (cl, o, msg))
	    {
		if (!ObtainData (cb, cl, (Object *) retval, ((struct opSet *) msg)->ops_AttrList))
		{
		    CoerceMethod (cl, (Object *) retval, OM_DISPOSE);
		    return NULL;
		}
	    }
	    return retval;

	    /* Let the superclass handle everything else */
	default:
	    return (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }
}

/*****************************************************************************/

Class *initClass (struct ClassBase *cb)
{
    Class *cl;

    if (cl = MakeClass (PCXDTCLASS, PICTUREDTCLASS, NULL, NULL, 0L))
    {
	cl->cl_Dispatcher.h_Entry = (ULONG (*)())Dispatch;
	cl->cl_UserData = (ULONG) cb;
	AddClass (cl);
    }

    return (cl);
}
