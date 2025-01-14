/* OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE */
/* OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE */
/* OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE */
/* OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE */

/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <iff/ilbm.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <graphics/gfx.h>
#include <graphics/displayinfo.h>
#include <graphics/view.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <libraries/iffparse.h>
#include <utility/tagitem.h>
#include <workbench/workbench.h>
#include <string.h>

/* prototypes */
#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/icon_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/icon_pragmas.h>

/* application includes */
#include "ieiff.h"
#include "ieutils.h"


/*****************************************************************************/


extern struct Library *DOSBase;
extern struct Library *SysBase;
extern struct Library *IFFParseBase;
extern struct Library *GfxBase;
extern struct Library *IntuitionBase;
extern struct Library *IconBase;


/*****************************************************************************/


static UWORD chip BrushImageData[] =
{
    0x0000,0x0000,0x0000,0x0400,0x0000,0x0000,0x0000,0x0C00,
    0x0000,0x0000,0x0000,0x0C00,0x0067,0xE7F0,0x0000,0x0C00,
    0x0066,0x0600,0x0000,0x0C00,0x0067,0xC7C0,0x0060,0x0C00,
    0x0066,0x0600,0x0180,0x0C00,0x0066,0x0600,0x0600,0x0C00,
    0x0066,0x0600,0x1800,0x0C00,0x0000,0x0000,0x6000,0x0C00,
    0x0000,0x0003,0x8000,0x0C00,0x0000,0x000E,0x0000,0x0C00,
    0x0000,0x0038,0x0000,0x0C00,0x0000,0x00E0,0x0000,0x0C00,
    0x0000,0x0380,0x0000,0x0C00,0x0000,0x0E00,0x0000,0x0C00,
    0x0000,0x7800,0xAA00,0x0C00,0x0003,0xE0AA,0x8000,0x0C00,
    0x00FE,0xAAA0,0x0000,0x0C00,0x0000,0x0000,0x0000,0x0C00,
    0x0000,0x0000,0x0000,0x0C00,0x7FFF,0xFFFF,0xFFFF,0xFC00,

    0xFFFF,0xFFFF,0xFFFF,0xF800,0xD555,0x5555,0x5555,0x5000,
    0xD555,0x5555,0x5555,0x5000,0xD510,0x1005,0x5555,0x5000,
    0xD511,0x5155,0x5555,0x5000,0xD510,0x1015,0x5515,0x5000,
    0xD511,0x5155,0x5455,0x5000,0xD511,0x5155,0x5155,0x5000,
    0xD511,0x5155,0x4555,0x5000,0xD555,0x5555,0x1555,0x5000,
    0xD555,0x5554,0x5555,0x5000,0xD555,0x5551,0x5555,0x5000,
    0xD555,0x5545,0x5555,0x5000,0xD555,0x5515,0x5555,0x5000,
    0xD555,0x5455,0x5555,0x5000,0xD555,0x5155,0x5555,0x5000,
    0xD555,0x7D55,0xFF55,0x5000,0xD557,0xF5FF,0xD555,0x5000,
    0xD5FF,0xFFF5,0x5555,0x5000,0xD555,0x5555,0x5555,0x5000,
    0xD555,0x5555,0x5555,0x5000,0x8000,0x0000,0x0000,0x0000,
};

static struct Image BrushImage =
{
    0, 0,			/* Upper left corner */
    54, 22, 2,			/* Width, Height, Depth */
    BrushImageData,		/* Image data */
    0x0003, 0x0000,		/* PlanePick, PlaneOnOff */
    NULL			/* Next image */
};

static struct DiskObject BrushIcon =
{
    WB_DISKMAGIC,		/* Magic Number */
    WB_DISKVERSION,		/* Version */
    {				/* Embedded Gadget Structure */
	NULL,			/* Next Gadget Pointer */
	0, 0, 54, 23,		/* Left,Top,Width,Height */
	GADGIMAGE | GADGHCOMP,	/* Flags */
	RELVERIFY,		/* Activation Flags */
	BOOLGADGET,		/* Gadget Type */
	(APTR)&BrushImage,	/* Render Image */
	NULL,			/* Select Image */
	NULL,			/* Gadget Text */
	NULL,			/* Mutual Exclude */
	NULL,			/* Special Info */
	0,			/* Gadget ID */
	NULL,			/* User Data */
    },
    WBPROJECT,			/* Icon Type */
    (char *)"SYS:Utilities/Display",/* Default Tool */
    NULL,			/* Tool Type Array */
    NO_ICON_POSITION,		/* Current X */
    NO_ICON_POSITION,		/* Current Y */
    NULL,			/* Drawer Structure */
    NULL,			/* Tool Window */
    0				/* Stack Size */
};


/*****************************************************************************/


VOID FillInILBM(WindowInfoPtr wi, DynamicImagePtr di, ILBMPtr ir)
{
struct ViewPort *vp = &wi->win->WScreen->ViewPort;
struct Rectangle rect;
WORD i;

    ir->ir_ModeID = GetVPModeID(vp);
    QueryOverscan(ir->ir_ModeID,&rect,OSCAN_TEXT);

    ir->ir_Width     = (rect.MaxX + 1);
    ir->ir_Height    = (rect.MaxY + 1);
    ir->ir_Depth     = di->di_BMap.Depth;
    ir->ir_BMap      = di->di_BMap;
    ir->ir_RPort     = di->di_RPort;
    ir->ir_NumColors = 1 << ir->ir_Depth;

    for (i = 0; i < ir->ir_NumColors; i++)
    {
	ir->ir_CRegs[i] = GetRGB4(vp->ColorMap,i);
    }
}


/*****************************************************************************/


#define VANILLA_COPY	0xC0
#define NO_MASK		0xFF

/* Properties that we can deal with */
LONG ilbmprops[] =
{
    ID_ILBM, ID_BMHD,
    ID_ILBM, ID_CMAP,
    ID_ILBM, ID_GRAB,
    ID_ILBM, ID_CAMG
};

ILBMPtr ReadILBM(BPTR drawer, STRPTR name, struct TagItem *attrs)
{
struct IFFHandle *iff;
ILBMPtr ilbm = NULL;
BPTR oldCD = NULL;

    SetIoErr(0);

    if (drawer)
    {
	oldCD = CurrentDir(drawer);
    }

    if (iff = AllocIFF())
    {
	if (iff->iff_Stream = Open(name,MODE_OLDFILE))
	{
	    InitIFFasDOS(iff);

	    ilbm = GetILBM(iff);

	    Close(iff->iff_Stream);
	}
	else
	{
	    /* Couldn't open file, Result2 already set by Open */
	}

	/* Free the IFF handle */
	FreeIFF (iff);
    }
    else
    {
	SetIoErr(ERROR_NO_FREE_STORE);
    }

    if (oldCD)
    {
	CurrentDir(oldCD);
    }

    return(ilbm);
}


/*****************************************************************************/


BOOL WriteILBM (BPTR drawer, STRPTR name, ILBMPtr ir, BOOL icon)
{
struct IFFHandle *iff;
BOOL result = FALSE;
BPTR oldCD  = NULL;

    SetIoErr(0);

    if (drawer)
    {
	oldCD = CurrentDir(drawer);
    }

    if (iff = AllocIFF())
    {
	if (iff->iff_Stream = Open(name,MODE_NEWFILE))
	{
	    InitIFFasDOS(iff);

	    if (OpenIFF(iff,IFFF_WRITE) == 0)
	    {
		if (PutILBM(iff,ir))
		{
		    result = TRUE;
		}
		else
		{
		    /* Couldn't write ilbm */
		}

		CloseIFF(iff);
	    }
	    else
	    {
		/* Couldn't open IFF for writing */
	    }

	    Close(iff->iff_Stream);

	    if (result)
	    {
		SetProtection(name,FIBF_EXECUTE);

		if (icon)
		{
		    PutDiskObject(name,&BrushIcon);
		}
	    }
	    else
	    {
		DeleteFile(name);
	    }
	}
	else
	{
	    /* Couldn't open file, Result2 already set by Open */
	}

	FreeIFF(iff);
    }
    else
    {
	SetIoErr(ERROR_NO_FREE_STORE);
    }

    if (oldCD)
    {
	CurrentDir(oldCD);
    }

    return(result);
}


/*****************************************************************************/


BOOL AllocBMRast(struct BitMap *bm, BYTE depth, WORD width, WORD height)
{
WORD i,j;

    InitBitMap(bm,depth,width,height);

    for (i = 0; i < depth; i++)
    {
	if (!(bm->Planes[i] = AllocRaster(width,height)))
	{
	    for (j = 0; j < i; j++);
	    {
                FreeRaster(bm->Planes[j],width,height);
            }
            return(FALSE);
	}
    }
    return (TRUE);
}


/*****************************************************************************/


ILBMPtr GetILBM(struct IFFHandle *iff)
{
ILBMPtr             ilbm;
struct BitMapHeader bmhd;

    if (!(OpenIFF(iff,IFFF_READ)))
    {
	PropChunks(iff,ilbmprops,4);
	StopChunk(iff,ID_ILBM,ID_BODY);

	if (ParseIFF(iff,IFFPARSE_SCAN) == 0L)
	{
	    if (ilbm = AllocVec(sizeof(struct ILBM),MEMF_CLEAR|MEMF_PUBLIC))
	    {
		if (GetBMHD(iff,&bmhd))
		{
		struct BitMap *bm = &(ilbm->ir_BMap);

		    ilbm->ir_Width  = bmhd.w;
		    ilbm->ir_Height = bmhd.h;
		    ilbm->ir_Depth  = bmhd.nplanes;

		    if (AllocBMRast(bm,ilbm->ir_Depth,ilbm->ir_Width,ilbm->ir_Height))
		    {
		    struct ColorRegister pp_Colors[MAXCOLORS];
		    struct StoredProperty *sp;
		    ULONG mode = NULL;

			InitRastPort(&(ilbm->ir_RPort));
			ilbm->ir_RPort.BitMap = bm;
			SetRast(&(ilbm->ir_RPort),0);

			if (sp = FindProp(iff,ID_ILBM,ID_CAMG))
			{
			    mode  = *((ULONG *) sp->sp_Data);
			}

			if (((mode & (0xffff0000)) && (!(mode & 0x00001000))) ||
			(mode == NULL))
			{
			    mode = LORES_KEY;
			    if (ilbm->ir_Width >= 640)
				mode = HIRES;
			    if (ilbm->ir_Height >= 400)
				mode |= LACE;
			    if (ilbm->ir_Depth == 6)
				mode |= HAM;
			}

			ilbm->ir_ModeID = mode;

			if (ilbm->ir_NumColors = GetColors(iff,&(pp_Colors[0])))
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
			else
			{
			    /* Couldn't get colors */
			}

			if (GetBody(iff,&bmhd,bm))
			{
			    CloseIFF(iff);
			    return(ilbm);
			}
			else
			{
			    /* couldn't get body */
			}
		    }
		    else
		    {
			/* couldn't allocate bitmap */
		    }
		}
		else
		{
		    /* no bitmap header */
		}

		FreeILBM(ilbm);
	    }
	    else
	    {
		/* not enough memory */
	    }
	}
	else
	{
	    /* scan error */
	}

	CloseIFF(iff);
    }
    else
    {
	/* couldn't open stream */
    }

    return(NULL);
}


/*****************************************************************************/


BOOL PutILBM(struct IFFHandle *iff, ILBMPtr ilbm)
{
BOOL result = FALSE;
struct BitMapHeader bmhd;

    if (iff && ilbm)
    {
	if (!(OpenIFF(iff,IFFF_WRITE)))
	{
	    if (PushChunk(iff,ID_ILBM,ID_FORM,-1) == 0)
	    {
                if (PutBMHD(iff,&bmhd,ilbm))
                {
                    if (ilbm->ir_NumColors > 0)
                    {
                        struct ColorRegister pp_Colors[MAXCOLORS];
                        WORD i, r, g, b;

                        for (i = 0; i < (ilbm->ir_NumColors); i++)
                        {
                            r = (ilbm->ir_CRegs[i] & 0xF00) >> 4;
                            g = (ilbm->ir_CRegs[i] & 0xF0);
                            b = (ilbm->ir_CRegs[i] << 4);

                            pp_Colors[i].red = r;
                            pp_Colors[i].green = g;
                            pp_Colors[i].blue = b;
                        }

                        PutColors(iff,&bmhd,pp_Colors);
                    }

                    if (PutBody(iff,&bmhd,&(ilbm->ir_BMap)))
                    {
                        result = TRUE;
                    }
                }
		PopChunk(iff);
	    }
	    else
	    {
		/* Couldn't start clip */
	    }
	    CloseIFF(iff);
	}
	else
	{
	    /* couldn't open for writing */
	}
    }

    return(result);
}


/*****************************************************************************/


BOOL GetBMHD(struct IFFHandle *iff, struct BitMapHeader *bmhd)
{
struct StoredProperty *sp;

    if (sp = FindProp(iff,ID_ILBM,ID_BMHD))
    {
	*bmhd = *((struct BitMapHeader *) sp->sp_Data);
	return(TRUE);
    }

    return(FALSE);
}


/*****************************************************************************/


BOOL PutBMHD(struct IFFHandle *iff, struct BitMapHeader *bmhd, ILBMPtr ir)
{
struct BitMap *bm = &(ir->ir_BMap);

    bmhd->w                = bm->BytesPerRow * 8;
    bmhd->h                = bm->Rows;
    bmhd->x                = 0;
    bmhd->y                = 0;
    bmhd->nplanes          = bm->Depth;
    bmhd->Masking          = mskHasTransparentColor;
    bmhd->Compression      = cmpNone;
    bmhd->pad1             = 0;
    bmhd->TransparentColor = 0;
    bmhd->XAspect          = 10;
    bmhd->YAspect          = 11;
    bmhd->PageWidth        = ir->ir_Width;
    bmhd->PageHeight       = ir->ir_Height;

    if ((PushChunk(iff,0,ID_BMHD,sizeof(struct BitMapHeader))) == 0)
    {
	if ((WriteChunkBytes(iff,bmhd,sizeof(struct BitMapHeader))) == sizeof(struct BitMapHeader))
	{
	    if ((PopChunk(iff)) == 0)
	    {
		return(TRUE);
	    }
	}
    }

    return(FALSE);
}


/*****************************************************************************/


WORD GetColors(struct IFFHandle *iff, struct ColorRegister *cmap)
{
struct ColorRegister *rgb;
struct StoredProperty *sp;
WORD i, ncolors = 0;

    if (sp = FindProp(iff,ID_ILBM,ID_CMAP))
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

    return(ncolors);
}


/*****************************************************************************/


BOOL PutColors(struct IFFHandle *iff, struct BitMapHeader *bmhd, struct ColorRegister *cmap)
{
    if ((cmap != NULL) && (PushChunk(iff,0,ID_CMAP,IFFSIZE_UNKNOWN) == 0L))
    {
        if ((WriteChunkRecords(iff,cmap,sizeof(struct ColorRegister), (1 << bmhd->nplanes))) == (1 << bmhd->nplanes))
        {
            if ((PopChunk(iff)) == 0L)
            {
                return(TRUE);
            }
        }
    }

    return (FALSE);
}


/*****************************************************************************/


BOOL GetBody(struct IFFHandle *iff, struct BitMapHeader *bmhd, struct BitMap *bm)
{
BOOL result = FALSE;
WORD i, n, p;
UWORD *srcline, *destline;
struct BitMap mapcopy, *destmap;
UBYTE *linebuf, *csrcline;
WORD srcw, destw;           /* src and dest width in BYTES */
WORD srch, desth;           /* src and dest height (rows) */
WORD srcd, destd;           /* src and dest depth */
WORD deep, rows, mod;

    if ((bmhd->Compression != cmpNone) && (bmhd->Compression != cmpByteRun1))
    {
	return(FALSE);
    }

    destmap = bm;
    mapcopy = *destmap;

    srcw  = BPR (bmhd->w);
    srch  = bmhd->h;
    srcd  = bmhd->nplanes;
    destw = mapcopy.BytesPerRow;
    desth = mapcopy.Rows;
    destd = mapcopy.Depth;
    rows  = MIN(srch,desth);

    mod = destw - srcw;
    if (mod < 0)
	mod = -mod;

    if (bmhd->Masking == mskHasMask)
	srcd++;

    deep = MIN(srcd,destd);

    if (linebuf = AllocVec(srcw*srcd,MEMF_CLEAR|MEMF_PUBLIC))
    {
	for (i = rows; i--;)
	{
	    if (!(result = GetLine(iff,linebuf,srcw,srcd,bmhd->Compression)))
		break;

	    srcline = (UWORD *) linebuf;
	    for (p = 0; p < deep; p++)
	    {
		destline  = (UWORD *) mapcopy.Planes[p];
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

	FreeVec(linebuf);
	result = TRUE;
    }

    return(result);
}


/*****************************************************************************/


BOOL GetLine(struct IFFHandle *iff, UBYTE *buf, WORD wide, WORD deep, UBYTE cmptype)
{
    if (cmptype == cmpNone)
    {
	LONG big = wide * deep;

	if (ReadChunkBytes(iff,buf,big) != big)
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
		return(FALSE);
	    }
	}
    }
    return(TRUE);
}


/*****************************************************************************/


BOOL PutBody(struct IFFHandle *iff, struct BitMapHeader *bmhd, struct BitMap *bitmap)
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

		WriteChunkBytes(iff,&pptr[i * bitmap->BytesPerRow], 2 * ((bmhd->w + 15) / 16));
	    }
	}

	if ((PopChunk(iff)) == 0L)
	{
	    return(TRUE);
	}
    }

    return(FALSE);
}


/*****************************************************************************/


VOID FreeILBM(ILBMPtr ilbm)
{
struct BitMap *bm;
WORD i;

    if (ilbm)
    {
	bm = &(ilbm->ir_BMap);

	for (i = 0; i < ilbm->ir_Depth; i++)
	{
	    if (bm->Planes[i])
	    {
		FreeRaster(bm->Planes[i],ilbm->ir_Width,ilbm->ir_Height);
	    }
	}

	FreeVec(ilbm);
    }
}

/* OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE */
/* OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE */
/* OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE */
/* OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE */
/* OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE * OBSOLETE */
