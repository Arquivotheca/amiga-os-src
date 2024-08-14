/* io.c
 *
 */

#include "pointer.h"
#include <intuition/pointerclass.h>

#define	DR(x)	;
#define	DW(x)	;
#define	DM(x)	;
#define	DB(x)	;

/*****************************************************************************/

/* The PrefHeader structure this editor outputs */
static struct PrefHeader far IFFPrefHeader =
{
    0,				/* version */
    0,				/* type    */
    0				/* flags   */
};

/*****************************************************************************/

/* The IFF chunks known to this prefs editor. IFFPrefChunkCnt says how many
 * chunks there are
 */
#define IFFPrefChunkCnt 2
static LONG far IFFPrefChunks[] =
{
    ID_PREF, ID_PRHD,
    ID_PREF, ID_PNTR,
};

/*****************************************************************************/

struct BitMap *allocbitmap (EdDataPtr ed, WORD width, WORD height, BYTE depth)
{
    ULONG planes, data, msize;
    struct BitMap *bm;
    BYTE i;

    planes = RASSIZE (width, height);
    msize = sizeof (struct BitMap) + (planes * depth);
    if (bm = AllocVec (msize, MEMF_CHIP | MEMF_CLEAR))
    {
	InitBitMap (bm, depth, width, height);
	data = ((void *) ((bm) + 1));
	for (i = 0; i < depth; i++)
	    bm->Planes[i] = (PLANEPTR) (data + i * planes);
    }
    return (bm);
}

/*****************************************************************************/

void freebitmap (EdDataPtr ed, struct BitMap * bm)
{
    FreeVec (bm);
}

/*****************************************************************************/

EdStatus ReadPrefs (EdDataPtr ed, struct IFFHandle * iff, struct ContextNode * cn)
{
    struct ExtPrefs *ep = &ed->ed_PrefsWork;
    EdStatus result = ES_IFFERROR;
    struct PointerPrefs *pp;
    struct PointerPrefs tmp;
    struct BitMap *bm;
    struct RGBTable t;
    BOOL ok = TRUE;
    WORD i, plane;
    WORD ncolors;
    ULONG msize;

    if (cn->cn_ID != ID_PNTR || cn->cn_Type != ID_PREF)
	return (ES_IFF_UNKNOWNCHUNK);

    if (ReadChunkBytes (iff, &tmp, sizeof (struct PointerPrefs)) == sizeof (struct PointerPrefs))
    {
	pp = &ed->ed_PrefsWork.ep_PP[tmp.pp_Which];

	CopyMem (&tmp, pp, sizeof (struct PointerPrefs));

	/* GET THE COLORMAP */
	ncolors = (1 << pp->pp_Depth) - 1;
	DR (kprintf ("read %ld colors\n", (LONG) ncolors));
	for (i = 0; i < ncolors; i++)
	{
	    if (ok)
	    {
		if (ReadChunkBytes (iff, (char *) &t, sizeof (struct RGBTable)) != sizeof (struct RGBTable))
		{
		    DR (kprintf (" couldn't read palette\n"));
		    ok = FALSE;
		}
		else if (i < MAXCOLORS)
		{
		    DR (kprintf (" %ld: r=%ld g=%ld b=%ld\n", (LONG) i, (ULONG) t.t_Red, (ULONG) t.t_Green, (ULONG) t.t_Blue));
		    ep->ep_CMap[(i + 1)] = *(&t);
		}
		else
		{
		    DR (kprintf ("TOO MANY COLORS!!!\n"));
		}
	    }
	    else
	    {
		result = ES_DOSERROR;
		ed->ed_SecondaryResult = IoErr ();
		DR (kprintf (" dos error %ld\n", ed->ed_SecondaryResult));
	    }
	}

	if (bm = allocbitmap (ed, pp->pp_Width, pp->pp_Height, pp->pp_Depth))
	{
	    /* GET THE BITPLANES */
	    msize = bm->Rows * bm->BytesPerRow;
	    for (plane = 0; plane < bm->Depth; plane++)
	    {
		if (plane < pp->pp_Depth)
		{
		    if (ok)
		    {
			if (ReadChunkBytes (iff, (char *) bm->Planes[plane], msize) != msize)
			{
			    DR (kprintf (" couldn't read\n"));
			    ok = FALSE;
			}
			else
			{
			    DR (kprintf (" read %ld bytes\n", msize));
			}
		    }
		    else
		    {
			result = ES_DOSERROR;
			ed->ed_SecondaryResult = IoErr ();
			DR (kprintf (" dos error %ld\n", ed->ed_SecondaryResult));
		    }
		}
		else
		{
		    /* Set additional planes to zero */
		    DR (kprintf (" plane %ld < %ld\n", (ULONG) plane, (ULONG) pp->pp_Depth));
		    memset (bm->Planes[plane], 0, msize);
		}
	    }

	    if (ok)
	    {
		DR (kprintf (" screen=0x%lx\n", ed->ed_Screen));

		SetAPen (&ed->ed_PrefsWork.ep_RPort, 0);
		RectFill (&ed->ed_PrefsWork.ep_RPort,
			  (pp->pp_Which * MAXWIDTH), 0,
			  (pp->pp_Which * MAXWIDTH) + MAXWIDTH - 1, MAXHEIGHT - 1);

		bltbm (bm, 0, 0,
		       ed->ed_PrefsWork.ep_BMap,
		       pp->pp_Which * MAXWIDTH, 0,
		       MIN (pp->pp_Width, MAXWIDTH), MIN (pp->pp_Height, MAXHEIGHT),
		       0xC0, 0xFF, NULL, ed->ed_GfxBase);

		result = ES_NORMAL;

		ed->ed_Size = (pp->pp_Size) ? pp->pp_Size - POINTERXRESN_LORES : 0;
	    }
	    freebitmap (ed, bm);
	}
    }

    return (result);
}

/*****************************************************************************/

EdStatus OpenPrefs (EdDataPtr ed, STRPTR name)
{
    EdStatus result;

    DR (kprintf ("OpenPrefs %s\n", name));
    if ((result = ReadIFF (ed, name, IFFPrefChunks, IFFPrefChunkCnt, ReadPrefs)) == ES_NORMAL)
    {
	/* Map up */
	DM (kprintf ("OpenPrefs : map up\n"));
	ReMap (ed, &ed->ed_PrefsWork, 1);
    }

    /* Update the sketchpad */
    UpdateSketch (ed);

    return (result);
}

/*****************************************************************************/

EdStatus WritePrefs (EdDataPtr ed, struct IFFHandle * iff, struct ContextNode * cn)
{
    struct ExtPrefs *ep = &ed->ed_PrefsWork;
    EdStatus result = ES_NORMAL;
    UWORD depth = ed->ed_Depth;
    struct PointerPrefs *pp;
    struct BitMap *bm;
    BOOL ok = TRUE;
    WORD ncolors;
    ULONG msize;
    WORD i, j;

    if (ed->ed_Screen)
	depth = MIN (ed->ed_Depth, ed->ed_Screen->BitMap.Depth);

    for (i = 0; i < 2; i++)
    {
	pp = &ed->ed_PrefsWork.ep_PP[i];
	pp->pp_Which = i;
	if (!(PushChunk (iff, 0, ID_PNTR, IFFSIZE_UNKNOWN)))
	{
	    DW (kprintf ("save pointer %ld\n", (LONG) i));
	    pp->pp_Depth  = 2;
	    pp->pp_Width  = ed->ed_Width;
	    pp->pp_Height = ed->ed_Height;
	    pp->pp_Size   = ed->ed_Size + POINTERXRESN_LORES;
	    if (WriteChunkBytes (iff, pp, sizeof (struct PointerPrefs)) == sizeof (struct PointerPrefs))
	    {
		/* PUT THE COLORMAP */
		ncolors = (1 << pp->pp_Depth) - 1;
		for (j = 0; j < ncolors; j++)
		{
		    if (ok)
		    {
			if (WriteChunkBytes (iff, (char *) &ep->ep_CMap[(j + 1)], sizeof (struct RGBTable)) != sizeof (struct RGBTable))
			{
			    ok = FALSE;
			}
		    }
		    else
		    {
			result = ES_DOSERROR;
			ed->ed_SecondaryResult = IoErr ();
			DW (kprintf (" dos error %ld\n", ed->ed_SecondaryResult));
		    }
		}

		DB (kprintf ("sprite is %ld x %ld x %ld\n", (LONG)pp->pp_Width, (LONG)pp->pp_Height, (LONG)pp->pp_Depth));
		if (bm = allocbitmap (ed, pp->pp_Width, pp->pp_Height, pp->pp_Depth))
		{
		    /* Copy the image to the temporary BitMap */
		    bltbm (ed->ed_PrefsWork.ep_BMap,
			   (i * MAXWIDTH), 0,
			   bm, 0, 0, pp->pp_Width, pp->pp_Height,
			   0xC0, 0xFF, NULL, ed->ed_GfxBase);

#if 0
bltbm (bm, 0, 0,
	&ed->ed_Screen->BitMap, 0, (i * MAXHEIGHT), pp->pp_Width, pp->pp_Height,
	0xC0, 0xFF, NULL, ed->ed_GfxBase);

bltbm (ed->ed_PrefsWork.ep_BMap, (i * MAXWIDTH), 0,
	&ed->ed_Screen->BitMap, pp->pp_Width, (i * MAXHEIGHT), pp->pp_Width, pp->pp_Height,
	0xC0, 0xFF, NULL, ed->ed_GfxBase);
#endif

		    /* PUT THE BODY */
		    msize = bm->Rows * bm->BytesPerRow;
		    for (j = 0; j < pp->pp_Depth; j++)
		    {
			if (ok)
			{
			    if (WriteChunkBytes (iff, (char *) bm->Planes[j], msize) != msize)
			    {
				ok = FALSE;
			    }
			}
			else
			{
			    result = ES_DOSERROR;
			    ed->ed_SecondaryResult = IoErr ();
			    DW (kprintf (" dos error %ld\n", ed->ed_SecondaryResult));
			}
		    }

		    freebitmap (ed, bm);
		}

		PopChunk (iff);
	    }
	}
    }
    return (result);
}

/*****************************************************************************/

EdStatus SavePrefs (EdDataPtr ed, STRPTR name)
{
    EdStatus result;

    /* Map down */
    DM (kprintf ("SavePrefs : map down\n"));
    ReMap (ed, &ed->ed_PrefsWork, -1);

    DW (kprintf ("SavePrefs %s\n", name));
    result = WriteIFF (ed, name, &IFFPrefHeader, WritePrefs);

    /* Map up */
    DM (kprintf ("SavePrefs : map up\n"));
    ReMap (ed, &ed->ed_PrefsWork, 1);

    return (result);
}

/*****************************************************************************/

UWORD chip RomPointer0[] =
{
    0xC000,
    0x7000,
    0x3C00,
    0x3F00,

    0x1FC0,
    0x1FC0,
    0x0F00,
    0x0D80,

    0x04C0,
    0x0460,
    0x0020,
    0x0000,

    0x0000,
    0x0000,
    0x0000,
    0x0000,

};

UWORD chip RomPointer1[] =
{
    0x4000,
    0xB000,
    0x4C00,
    0x4300,

    0x20C0,
    0x2000,
    0x1100,
    0x1280,

    0x0940,
    0x08A0,
    0x0040,
    0x0000,

    0x0000,
    0x0000,
    0x0000,
    0x0000,
};

UWORD chip RomBusyPointer0[] =
{
    0x0400,
    0x0000,
    0x0100,
    0x0000,

    0x07C0,
    0x1FF0,
    0x3FF8,
    0x3FF8,

    0x7FFC,
    0x7EFC,
    0x7FFC,
    0x3FF8,

    0x3FF8,
    0x1FF0,
    0x07C0,
    0x0000,
};

UWORD chip RomBusyPointer1[] =
{
    0x07C0,
    0x07C0,
    0x0380,
    0x07E0,

    0x1FF8,
    0x3FEC,
    0x7FDE,
    0x7FBE,

    0xFF7F,
    0xFFFF,
    0xFFFF,
    0x7FFE,

    0x7FFE,
    0x3FFC,
    0x1FF8,
    0x07E0,
};

UBYTE PDefCols[4][3] =
{
    {0x00, 0x00, 0x00},
    {0xE0, 0x40, 0x40},
    {0x00, 0x00, 0x00},
    {0xE0, 0xE0, 0xC0}
};

/*****************************************************************************/

VOID CopyFromDefault (EdDataPtr ed)
{
    struct ExtPrefs *ep = &ed->ed_PrefsDefaults;
    struct BitMap bm;
    register WORD i;

    ep->ep_PP[0].pp_X     =  ep->ep_PP[0].pp_Y    =  0;
    ep->ep_PP[0].pp_Size  = ep->ep_PP[1].pp_Size  = 0;
    ep->ep_PP[0].pp_Depth = ep->ep_PP[1].pp_Depth = 2;

    for (i = 0; i < 4; i++)
    {
	ep->ep_CMap[i].t_Red   = PDefCols[i][0];
	ep->ep_CMap[i].t_Green = PDefCols[i][1];
	ep->ep_CMap[i].t_Blue  = PDefCols[i][2];
    }

    InitBitMap (&bm, 2, 16, 16);
    bm.Planes[0] = RomPointer0;
    bm.Planes[1] = RomPointer1;
    bltbm (&bm, 0, 0, ep->ep_BMap, 0, 0, 16, 16, 0xC0, 0xFF, NULL, ed->ed_GfxBase);

    InitBitMap (&bm, 2, 16, 16);
    bm.Planes[0] = RomBusyPointer0;
    bm.Planes[1] = RomBusyPointer1;
    bltbm (&bm, 0, 0, ep->ep_BMap, MAXWIDTH, 0, 16, 16, 0xC0, 0xFF, NULL, ed->ed_GfxBase);

    ep->ep_PP[1].pp_X  = -5;
    ep->ep_PP[1].pp_Y  =  0;

    /* Map up */
    DM (kprintf ("Default : map up\n"));
    ReMap (ed, ep, 1);
}
