/* dispatch.c
 */

#include "classbase.h"

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

    if (cl = MakeClass (MACPAINTDTCLASS, PICTUREDTCLASS, NULL, NULL, 0L))
    {
	cl->cl_Dispatcher.h_Entry = (ULONG (*)()) Dispatch;
	cl->cl_UserData 	  = (ULONG) cb;
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
		if (!ObtainData (cb, cl, (Object *) retval, ((struct opSet *) msg)->ops_AttrList))
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

#define PTNG_WIDTH	720
#define PTNG_HEIGHT	576

/*****************************************************************************/

struct MacBinaryHeader
{
    UBYTE		 mbh_Dummy;		/* THIS IS TO ALIGN THE mbh_FileCode FIELD */

    UBYTE		 mbh_Version;		/* Always 0 */
    UBYTE		 mbh_NameLength;	/* Length of original MacPaint file name */
    UBYTE		 mbh_Name[63];		/* Valid up to mbh_NameLength */
    ULONG		 mbh_FileCode;		/* PNTG */
    ULONG		 mbh_AuthorTool;	/* Author Tool ID */
    UBYTE		 mbh_Flags1;		/* Finder flags */
    UBYTE		 mbh_Pad1;		/* Always zero */
    ULONG		 mbh_FileXY;
    UWORD		 mbh_FolderID;
    UBYTE		 mbh_Protection;
    UBYTE		 mbh_Pad2;		/* Always zero */
    ULONG		 mbh_DataLength;	/* Length of data fork */
    ULONG		 mbh_RsrcLength;	/* Length of resource fork */
    ULONG		 mbh_CreationDate;	/* Creation date */
    ULONG		 mbh_ModifyDate;	/* Modification date */

    /* The rest of this structure is bogus due to padding by the compiler */

    UWORD		 mbh_Pad3;
    UBYTE		 mbh_Flags2;		/* More Finder flags */
    UBYTE		 mbh_Pad4[14];		/* UNKNOWN */
    ULONG		 mbh_FileLength;	/* [NOT ALIGNED] Unpacked file length */
    UWORD		 mbh_Pad5;		/* Always zero */
    UBYTE		 mbh_MinWVersion;	/* Minimum write version */
    UBYTE		 mbh_MinRVersion;	/* Minimum read version */
    UWORD		 mbh_CRC;		/* CRC of previous 124 bytes */
};

#define	MBH_SIZE	(sizeof (struct MacBinaryHeader))

/*****************************************************************************/

BOOL ASM ObtainData (REG (a6) struct ClassBase *cb, REG (a0) Class * cl, REG (a2) Object * o, REG (a1) struct TagItem * attrs)
{
    struct ColorRegister *cmap;
    struct BitMapHeader *bmhd;
    ULONG modeid = HIRES_KEY;
    LONG w, i, j, k, l;
    LONG *cregs;
    LONG x, y;
    BPTR fh;

    LONG ch;
    UBYTE c;

    struct RastPort rp;
    struct BitMap *bm;

    struct RastPort trp;
    struct BitMap *tbm;
    STRPTR linebuffer;

    struct MacBinaryHeader mbh;
    STRPTR title;

    BOOL retval = FALSE;

    /* Get the default title */
    title = (STRPTR) GetTagData (DTA_Name, NULL, attrs);

    /* Get the file handle to read from and the BitMapHeader to write to */
    if ((getdtattrs (cb, o, DTA_Handle, &fh, PDTA_BitMapHeader, &bmhd, TAG_DONE) == 2) && fh)
    {
	/* Make sure we are at the beginning of the file */
	Seek (fh, 0L, OFFSET_BEGINNING);

	/* Read the MacBinary header */
	if (Read(fh, &mbh.mbh_Version, MBH_SIZE) == MBH_SIZE)
	{
	    /* See if it has a MacBinary header */
	    if ((mbh.mbh_Version == 0) && (mbh.mbh_Pad1 == 0) &&
		(mbh.mbh_NameLength > 0) && (mbh.mbh_NameLength < 63))
	    {
		/* Get the original Macintosh file name */
		mbh.mbh_Name[mbh.mbh_NameLength] = 0;
		title = mbh.mbh_Name;
	    }
	    else
	    {
		/* Must be a plain Macintosh MacPaint file */
		Seek (fh, 0, OFFSET_BEGINNING);
	    }

	    /* Seek to the Bitmap data */
	    if (Seek (fh, 512, OFFSET_CURRENT) >= 0)
	    {
		/* Initialize the BitMapHeader.  MacPaint files come in only one size */
		memset(bmhd, 0, sizeof(struct BitMapHeader));
		bmhd->bmh_Width  = bmhd->bmh_PageWidth	= PTNG_HEIGHT;
		bmhd->bmh_Height = bmhd->bmh_PageHeight = PTNG_WIDTH;
		bmhd->bmh_Depth  = 1;

		/* MacPaint files are always one bitplane */
		setdtattrs (cb, o, PDTA_NumColors, 2, TAG_DONE);

		/* Get a pointer to the color registers */
		getdtattrs (cb, o,
			    PDTA_ColorRegisters,    (ULONG)&cmap,
			    PDTA_CRegs, 	    &cregs,
			    TAG_DONE);

		/* First color is white */
		cmap->red = cmap->green = cmap->blue  = 0xff;
		cregs[0] = cregs[1] = cregs[2] = 0xff << 24;
		cmap++;

		/* Second color is black */
		cmap->red = cmap->green = cmap->blue = 0;
		cregs[3] = cregs[4] = cregs[5] = 0;

		/* Initialize the temporary bitmap */
		if (!(tbm = AllocBitMap (bmhd->bmh_Width, 1, bmhd->bmh_Depth, BMF_CLEAR, NULL)))
		{
		    SetIoErr (ERROR_NO_FREE_STORE);
		    return (FALSE);
		}
		InitRastPort (&trp);
		trp.BitMap = tbm;

		/* Allocate the temporary line buffer */
		w = (LONG) bmhd->bmh_Width;
		i = ((w + 15) >> 4) << 4;
		if ((linebuffer = AllocVec (i, MEMF_CLEAR)) == NULL)
		{
		    FreeBitMap (tbm);
		    SetIoErr (ERROR_NO_FREE_STORE);
		    return (FALSE);
		}

		/* Allocate the bitmap */
		if (bm = AllocBitMap (bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, BMF_CLEAR, NULL))
		{
		    /* Initialize the temporary rastport */
		    InitRastPort (&rp);
		    rp.BitMap = bm;

		    /* We're going to be drawing in black */
		    SetAPen (&rp, 1);

		    /* Step through the body data */
		    l = x = y = 0;
		    while ((l == 0) && ((ch = FGetC (fh)) >= 0))
		    {
			c = (UBYTE) ch;
			k = c;

			if (c < 0x80)
			{
			    for (i=0; i<=k; i++)
			    {
				if ((ch = FGetC (fh)) >= 0)
				{
				    c = (UBYTE) ch;
				    for (j=7; j>=0; j--)
				    {
					if (c & (1 << j))
					    linebuffer[x] = 1;

					if (++x == PTNG_HEIGHT)
					{
					    x = 0;
					    WritePixelLine8 (&rp, 0, y, w, linebuffer, &trp);
					    if (++y == PTNG_WIDTH)
					    {
						l = 1;
						break;
					    }
					}
				    }
				}
				if (l) break;
			    }
			}
			else
			{
			    if ((ch = FGetC (fh)) >= 0)
			    {
				c = (UBYTE)ch;
				for (i=0; i<=(256 - k); i++)
				{
				    for (j=7; j>=0; j--)
				    {
					if (c & (1 << j))
					    linebuffer[x] = 1;

					if (++x == PTNG_HEIGHT)
					{
					    x = 0;
					    WritePixelLine8 (&rp, 0, y, w, linebuffer, &trp);
					    if (++y == PTNG_WIDTH)
					    {
						l = 1;
						break;
					    }
					}
				    }
				    if (l) break;
				}
			    }
			}
		    }

		    /* Tell the picture class about everything */
		    setdtattrs (cb, o,
				DTA_ObjName,		title,
				DTA_NominalHoriz,	bmhd->bmh_Width,
				DTA_NominalVert,	bmhd->bmh_Height,
				PDTA_BitMap,		bm,
				PDTA_ModeID,		modeid,
				TAG_DONE);

		    retval = TRUE;
		}
		else
		    SetIoErr (ERROR_NO_FREE_STORE);

		/* Free the line buffer */
		FreeVec (linebuffer);

		/* Free the temporary bitmap */
		FreeBitMap (tbm);
	    }
	}
    }
    else
	SetIoErr (ERROR_REQUIRED_ARG_MISSING);

    return (retval);
}
