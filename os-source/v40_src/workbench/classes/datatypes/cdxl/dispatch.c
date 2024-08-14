/* dispatch.c
 * main DataTypes class for XL files
 *
 */

#include "classbase.h"

/*****************************************************************************/

#include "cdxl.h"

/*****************************************************************************/

#define	DB(x)	;
#define	DI(x)	;

/*****************************************************************************/

struct localData
{
    struct FileInfoBlock lod_FIB;
    BPTR		 lod_FH;		/* File handle */

    UBYTE		*lod_Buffer;		/* Frame buffer */
    ULONG		 lod_BufferSize;	/* Size of buffer */

    ULONG		 lod_FrameSize;		/* Frame size */

    struct BitMap	*lod_KeyFrame;		/* Key frame */

    UBYTE		*lod_Sample;
    ULONG		 lod_SampleLength;
    ULONG		 lod_Period;

    struct PanFrame	 lod_Header;		/* Descriptor */
    struct BitMap	 lod_Frame;		/* Current frame bitmap */
};

/*****************************************************************************/

#define	G(o)			((struct Gadget *)o)

#define INTDIV(a, b)		(((a) + ((b) / 2)) / (b))
#define	CDXL_SPEED_BPS		153600
#define	CDTV_SEEK		10
#define	CDTV_READXL		42
#define	DEFAULT_XLSPEED		75
#define	DEFAULT_SECTOR_SIZE	2048

#define	NTSC_RATE		60
#define	NTSC_FREQ		3579545
#define	PAL_FREQ		3546895

/*****************************************************************************/

static void ASM CopyBitMap (
    REG (a6) struct ClassBase *cb,
    REG (a2) struct BitMap *bm1, REG (a3) struct BitMap *bm2, REG (d2) ULONG width)
{
    ULONG bpr1 = bm1->BytesPerRow;
    ULONG bpr2 = bm2->BytesPerRow;
    register UBYTE *src;
    register UBYTE *dst;
    register LONG r;
    register LONG p;

    for (p = bm1->Depth - 1; p >= 0; p--)
    {
	src = (BYTE *) bm1->Planes[p];
	dst = (BYTE *) bm2->Planes[p];

	for (r = bm1->Rows - 1; r >= 0; r--)
	{
	    CopyMem (src, dst, width);
	    src += bpr1;
	    dst += bpr2;
	}
    }
}

/*****************************************************************************/

static ULONG ASM GetCDXL (REG (a6) struct ClassBase * cb, REG (a0) Class * cl, REG (a2) Object * o, REG (a1) struct TagItem * attrs)
{
    register struct localData *lod = INST_DATA (cl, o);
    register struct PanFrame *pf = &lod->lod_Header;

    struct ColorRegister rgb, *cmap, *ccmap;
    BOOL success = TRUE;
    register ULONG i;
    ULONG mult = 1L;
    ULONG secs, fps;
    UWORD *colors;
    UBYTE *buffer;
    ULONG modeid;
    STRPTR title;
    WORD ncolors;
    ULONG frames;
    ULONG fsize;
    ULONG psize;
    LONG *cregs;
    ULONG speed;
    BPTR fh;

    title = (STRPTR) GetTagData (DTA_Name, NULL, attrs);

    GetDTAttrs (o, DTA_Handle, &fh, TAG_DONE);

    if (lod->lod_FH = fh)
    {
	/* Read the animation header */
	if (Read (fh, pf, PAN_SIZE) == PAN_SIZE)
	{
	    /* Get the file size */
	    if (ExamineFH (lod->lod_FH, &lod->lod_FIB))
	    {
		fsize = (ULONG)lod->lod_FIB.fib_Size;
		Seek (fh, 0L, OFFSET_BEGINNING);
	    }
	    else
	    {
		/* Get the file size */
		Seek (fh, 0L, OFFSET_END);
		fsize = (ULONG) Seek (fh, 0L, OFFSET_BEGINNING);
	    }

	    /* Compute the number of frames */
	    lod->lod_FrameSize = (LONG) FRAME_SIZE (pf);
	    frames = fsize / lod->lod_FrameSize;

	    /* Get the speed */
	    speed = (pf->Reserved + 1) * DEFAULT_XLSPEED * DEFAULT_SECTOR_SIZE;

	    /* Compute the frames per second */
	    if (secs = INTDIV ((fsize * 100), speed))
		fps  = INTDIV ((frames * 10000), secs) / 100;
	    else
		fps  = 10;

#if 0
	    if (fps < 4)
		mult = 4;
	    else if (fps < 10)
		mult = 2;
	    fps *= mult;
#endif

	    /* Allocate the frame buffer */
	    if ((lod->lod_Buffer = AllocVec (lod->lod_FrameSize, NULL)) == NULL)
		return FALSE;

	    /* Allocate the bitmap for the key frame */
	    if ((lod->lod_KeyFrame = AllocBitMap ((ULONG)pf->XSize, (ULONG)pf->YSize, (ULONG)pf->PixelSize,
						  BMF_INTERLEAVED, NULL)) == NULL)
		return FALSE;

	    /* Read the first frame of the animation */
	    if (Read (fh, lod->lod_Buffer, lod->lod_FrameSize) == lod->lod_FrameSize)
	    {
		/* Convert it to a bitmap */
		InitBitMap (&lod->lod_Frame, pf->PixelSize, pf->XSize, pf->YSize);
		buffer = (UBYTE *) ((ULONG) lod->lod_Buffer + PAN_SIZE + CMAP_SIZE (pf));
		psize = lod->lod_Frame.BytesPerRow * pf->YSize;
		for (i = 0; i < (ULONG)pf->PixelSize; i++)
		    lod->lod_Frame.Planes[i] = &buffer[(i * psize)];

		/* Blit the frame to the keyframe */
		CopyBitMap (cb, &lod->lod_Frame, lod->lod_KeyFrame, (GetBitMapAttr (&lod->lod_Frame, BMA_WIDTH) / 8));

		/* Come up with the mode id */
		ncolors = 1 << pf->PixelSize;
		modeid = LORES_KEY;
		if (pf->XSize >= 640)
		    modeid = HIRES;
		if (pf->YSize >= 400)
		    modeid |= LACE;

		switch (PI_VIDEO(pf))
		{
		    case PIV_STANDARD:
			break;

		    case PIV_HAM:
			modeid |= HAM_KEY;
			if (pf->PixelSize == 6)
			{
			    ncolors = 32;
			}
			else if (pf->PixelSize == 8)
			{
			    ncolors = 64;
			}
			else
			{
			    SetIoErr (ERROR_ACTION_NOT_KNOWN);
			    success = FALSE;
			}
			break;

		    default:
			SetIoErr (ERROR_ACTION_NOT_KNOWN);
			success = FALSE;
			break;
		}

		/* Should we continue on */
		if (success)
		{
		    /* Compute audio information */
		    if (secs = INTDIV ((speed * pf->AudioSize), lod->lod_FrameSize))
			lod->lod_Period = INTDIV (NTSC_FREQ, secs) / mult;
		    lod->lod_Sample = (UBYTE *)	((ULONG) lod->lod_Buffer + PAN_SIZE + CMAP_SIZE	(pf) + IMAGE_SIZE(pf));
		    if ((lod->lod_SampleLength = pf->AudioSize) == 0)
			lod->lod_Sample = NULL;

		    /* Tell the	super class about our attributes */
		    SetDTAttrs (o, NULL, NULL,
				DTA_ObjName,	    title,
				DTA_NominalHoriz,   (ULONG) pf->XSize,
				DTA_NominalVert,    (ULONG) pf->YSize,
				ADTA_Width,	    (ULONG) pf->XSize,
				ADTA_Height,	    (ULONG) pf->YSize,
				ADTA_Depth,	    (ULONG) pf->PixelSize,
				ADTA_Frames,	    frames,
				ADTA_KeyFrame,	    lod->lod_KeyFrame,
				ADTA_ModeID,	    modeid,
				ADTA_NumColors,	    (ULONG) ncolors,
				ADTA_FramesPerSecond,fps,

				ADTA_Sample,	    lod->lod_Sample,
				ADTA_SampleLength,  lod->lod_SampleLength,
				ADTA_Period,	    lod->lod_Period,

				TAG_DONE);

		    GetDTAttrs (o,
				ADTA_ColorRegisters,(ULONG)&cmap,
				ADTA_CRegs,	    (ULONG)&cregs,  /* For screen */
				TAG_DONE);

		    colors = (UWORD *) ((ULONG)	lod->lod_Buffer	+ PAN_SIZE);

		    ccmap = cmap;
		    i =	ncolors;
		    while (i--)
		    {
			rgb.red	  = (UBYTE) ((0xF00 & *colors) >> 4);
			rgb.green = (UBYTE) ((0x0F0 & *colors));
			rgb.blue  = (UBYTE) ((0x00F & *colors) << 4);
			*cmap =	*(&rgb);
			colors++;
			cmap++;
		    }

		    cmap = ccmap;
		    for	(i = 0;	i < ncolors; i++)
		    {
			cregs[i	* 3 + 0] = (LONG)cmap[i].red   << 24;
			cregs[i	* 3 + 1] = (LONG)cmap[i].green << 24;
			cregs[i	* 3 + 2] = (LONG)cmap[i].blue  << 24;
		    }

		    return (TRUE);
		}
	    }
	}
    }
    else
	SetIoErr (ERROR_REQUIRED_ARG_MISSING);

    return (FALSE);
}

/*****************************************************************************/

static struct BitMap *loadframe (struct ClassBase *cb, Class *cl, Object *o, struct adtFrame *msg)
{
    register struct localData *lod = INST_DATA (cl, o);
    register struct BitMap *bm;
    UBYTE *sample;
    UBYTE *buffer;

    /* See if there is a frame already allocated */
    if (buffer = (UBYTE *)msg->alf_UserData)
    {
	bm     = msg->alf_BitMap;
	sample = msg->alf_Sample;

	if (bm == NULL)
	{
	    bm = (struct BitMap *)((ULONG)buffer + lod->lod_FrameSize);
	    DB (kprintf ("bm was NULL again\n"));
	}
    }
    /* No, there wasn't so allocate a new frame */
    else if (buffer = AllocVec (lod->lod_FrameSize + sizeof (struct BitMap), NULL))
    {
	register struct PanFrame *pf = &lod->lod_Header;
	register ULONG i, psize;
	register char *buff;

	bm = (struct BitMap *)((ULONG)buffer + lod->lod_FrameSize);
	InitBitMap (bm, pf->PixelSize, pf->XSize, pf->YSize);
	buff = (UBYTE *)((ULONG)buffer + PAN_SIZE + CMAP_SIZE (pf));
	psize = bm->BytesPerRow * pf->YSize;
	for (i = 0; i < (ULONG)pf->PixelSize; i++)
	    bm->Planes[i] = &buff[(i * psize)];

	if (lod->lod_SampleLength)
	    sample = (UBYTE *) ((ULONG)buffer + PAN_SIZE + CMAP_SIZE (pf) + IMAGE_SIZE(pf));
	else
	    sample = NULL;
    }
    else
	SetIoErr (ERROR_NO_FREE_STORE);

    /* Clear the return value */
    msg->alf_BitMap = NULL;

    /* Make sure we have a buffer */
    if (buffer)
    {
	/* Remember the buffer */
	msg->alf_UserData = buffer;

        /* Clear the error return value */
        SetIoErr (0);

	/* Seek to the correct location */
	{
	    register LONG cof, offset;

	    /* Seek to the correct position within the file */
	    cof = Seek (lod->lod_FH, 0, OFFSET_CURRENT);
	    if (offset = (msg->alf_Frame * lod->lod_FrameSize) - cof)
		Seek (lod->lod_FH, offset, OFFSET_CURRENT);
	}

        /* Read the frame */
        if (Read (lod->lod_FH, buffer, lod->lod_FrameSize) == lod->lod_FrameSize)
        {
            msg->alf_BitMap       = bm;
            msg->alf_Sample       = sample;
            msg->alf_SampleLength = lod->lod_SampleLength;
            msg->alf_Period       = lod->lod_Period;
        }
	else
	{
	    DB (kprintf ("couldn't read\n"));
	}
    }
    else
    {
	DB (kprintf ("no buffer shithead\n"));
    }

    DI (kprintf ("%ld %08lx\n", msg->alf_Frame, msg->alf_BitMap));

    return msg->alf_BitMap;
}

/*****************************************************************************/

static ULONG unloadframe (struct ClassBase *cb, struct adtFrame *msg)
{
    /* Unloading while playing seems to be a problem... */
    FreeVec (msg->alf_UserData);
    msg->alf_UserData = NULL;
    return 0;
}

/*****************************************************************************/

static ULONG ASM Dispatch (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    register struct ClassBase *cb = (struct ClassBase *) cl->cl_UserData;

    switch (msg->MethodID)
    {
	case ADTM_LOADFRAME:
	    return (ULONG) loadframe (cb, cl, o, (struct adtFrame *)msg);

	case ADTM_UNLOADFRAME:
	    return (ULONG) unloadframe (cb, (struct adtFrame *)msg);

	case DTM_TRIGGER:
	    return (ULONG) DoSuperMethodA (cl, o, msg);

	case OM_NEW:
	    {
		register ULONG retval;

		if (retval = DoSuperMethodA (cl, o, msg))
		{
		    if (!(GetCDXL (cb, cl, (Object *) retval, ((struct opSet *) msg)->ops_AttrList)))
		    {
			CoerceMethod (cl, (Object *) retval, OM_DISPOSE);
			retval = NULL;
		    }
		}
		return retval;
	    }

	case OM_DISPOSE:
	    {
		register struct localData *lod = INST_DATA (cl, o);

		FreeBitMap (lod->lod_KeyFrame);
		FreeVec (lod->lod_Buffer);
	    }

	/* Let the superclass handle everything else */
	default:
	    return (ULONG) DoSuperMethodA (cl, o, msg);
    }
}

/*****************************************************************************/

Class *initClass (struct ClassBase * cb)
{
    register Class *cl;

    if (cl = MakeClass (CDXLDTCLASS, ANIMATIONDTCLASS, NULL, sizeof (struct localData), 0L))
    {
	cl->cl_Dispatcher.h_Entry = (ULONG (*)()) Dispatch;
	cl->cl_UserData = (ULONG) cb;
	AddClass (cl);
    }

    return (cl);
}
