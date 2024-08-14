/* dispatch.c
 *
 */

#include "classbase.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

static ULONG bestmodeid (struct ClassBase * cb, ULONG data,...)
{
    return BestModeIDA ((struct TagItem *) & data);
}

/*****************************************************************************/

static ULONG notifyAttrChanges (struct ClassBase * cb, Object * o, VOID * ginfo, ULONG flags, ULONG tag1,...)
{
    return DoMethod (o, OM_NOTIFY, &tag1, ginfo, flags);
}

/*****************************************************************************/

static void freeExtraInfo (struct ClassBase * cb, struct localData * lod)
{
    if (lod->lod_NumAlloc)
    {
	LONG i;

	for (i = (LONG) (lod->lod_NumAlloc - 1); i >= 0; i--)
	    ReleasePen (lod->lod_ColorMap, lod->lod_Allocated[i]);
	lod->lod_NumAlloc = 0L;
    }

    if ((lod->lod_BMap) && !(lod->lod_Flags & LODF_SAMEBM))
    {
	WaitBlit ();
	FreeBitMap (lod->lod_BMap);
	lod->lod_BMap = NULL;
    }
}

/*****************************************************************************/

static struct ColorMap *GetObjectColorMap (struct ClassBase * cb, Class * cl, Object * o)
{
    struct localData *lod = INST_DATA (cl, o);
    struct ColorMap *cm = NULL;
    ULONG i, r, g, b;

    if (lod->lod_Colors)
    {
	/* Get a color map */
	if (cm = GetColorMap (lod->lod_NumColors))
	{
	    /* Set the colors */
	    for (i = 0; i < lod->lod_NumColors; i++)
	    {
		r = lod->lod_CRegs[i * 3 + 0];
		g = lod->lod_CRegs[i * 3 + 1];
		b = lod->lod_CRegs[i * 3 + 2];
		SetRGB32CM (cm, i, r, g, b);
	    }
	}
    }
    return (cm);
}

/*****************************************************************************/

static ULONG copyMethod (struct ClassBase * cb, Class * cl, Object * o, struct dtGeneral * msg)
{
    struct localData *lod = INST_DATA (cl, o);
    struct IFFHandle *iff;
    BOOL success = FALSE;

    if (iff = AllocIFF ())
    {
	if (iff->iff_Stream = (ULONG) OpenClipboard (0L))
	{
	    InitIFFasClip (iff);
	    success = writeObject (cb, iff, o, lod);
	    CloseClipboard ((struct ClipboardHandle *) iff->iff_Stream);
	}
	FreeIFF (iff);
    }
    return ((ULONG) success);
}

/*****************************************************************************/

static ULONG writeMethod (struct ClassBase * cb, Class * cl, Object * o, struct dtWrite * msg)
{
    struct localData *lod = INST_DATA (cl, o);
    struct IFFHandle *iff;
    BOOL success = FALSE;

    if (iff = AllocIFF ())
    {
	if (iff->iff_Stream = msg->dtw_FileHandle)
	{
	    InitIFFasDOS (iff);
	    success = writeObject (cb, iff, o, lod);
	}
	FreeIFF (iff);
    }
    return ((ULONG) success);
}

/*****************************************************************************/

/* Methods we support */
ULONG m[] =
{
    OM_NEW,
    OM_GET,
    OM_SET,
    OM_UPDATE,
    OM_DISPOSE,

    GM_LAYOUT,
    GM_HITTEST,
    GM_GOACTIVE,
    GM_HANDLEINPUT,
    GM_RENDER,

    DTM_FRAMEBOX,
    DTM_SELECT,
    DTM_CLEARSELECTED,
    DTM_COPY,
    DTM_PRINT,
    DTM_WRITE,

    ~0,
};

/*****************************************************************************/

/* Inquire attribute of an object */
static ULONG getPictureDTAttr (struct ClassBase * cb, Class * cl, Object * o, struct opGet * msg)
{
    struct localData *lod = INST_DATA (cl, o);

    switch (msg->opg_AttrID)
    {
	case DTA_Methods:
	    *msg->opg_Storage = (ULONG) m;
	    break;

	case PDTA_ModeID:
	    *msg->opg_Storage = lod->lod_ModeID;
	    break;

	case PDTA_BitMapHeader:
	    *msg->opg_Storage = (ULONG) & lod->lod_BMHD;
	    break;

#ifdef	PDTA_DestBitMap
	case PDTA_DestBitMap:
#endif
	case PDTA_ClassBitMap:
	case PDTA_BitMap:
	    *msg->opg_Storage = (ULONG) lod->lod_BMap;
	    break;

	case PDTA_ColorRegisters:
	    *msg->opg_Storage = (ULONG) lod->lod_Colors;
	    break;

	case PDTA_CRegs:
	    *msg->opg_Storage = (ULONG) lod->lod_CRegs;
	    break;

	case PDTA_GRegs:
	    *msg->opg_Storage = (ULONG) lod->lod_GRegs;
	    break;

	case PDTA_ColorTable:
	    *msg->opg_Storage = (ULONG) lod->lod_ColorTable;
	    break;

	case PDTA_ColorTable2:
	    *msg->opg_Storage = (ULONG) lod->lod_ColorTable2;
	    break;

	case PDTA_Allocated:
	    *msg->opg_Storage = (ULONG) lod->lod_Allocated;
	    break;

	case PDTA_NumColors:
	    *msg->opg_Storage = (ULONG) lod->lod_NumColors;
	    break;

	case PDTA_NumAlloc:
	    *msg->opg_Storage = (ULONG) lod->lod_NumAlloc;
	    break;

	case PDTA_Grab:
	    *msg->opg_Storage = (ULONG) & lod->lod_Point;
	    break;

	default:
	    return (DoSuperMethodA (cl, o, msg));
    }

    return (1L);
}

/*****************************************************************************/

/* Set attributes of an object */
static ULONG setPictureDTAttrs (struct ClassBase * cb, Class * cl, Object * o, struct opSet * msg)
{
    struct TagItem *tags = msg->ops_AttrList;
    struct localData *lod;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG refresh = 0L;
    struct BitMap *obm;
    ULONG tidata;
    WORD ncolors;
    ULONG modeid;

    lod = INST_DATA (cl, o);

    ncolors = lod->lod_NumColors;
    modeid = lod->lod_ModeID;
    obm = lod->lod_SourceBMap;

    /* process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case OBP_Precision:
		lod->lod_Precision = tidata;
		break;

	    case PDTA_Remap:
		if (tidata)
		    lod->lod_Flags |= LODF_REMAP;
		else
		    lod->lod_Flags &= ~LODF_REMAP;
		break;

	    case PDTA_NumSparse:
		lod->lod_NumSparse = (UWORD) tidata;
		if (tidata)
		    lod->lod_Flags |= LODF_SPARSE;
		else
		    lod->lod_Flags &= ~LODF_SPARSE;
		break;

	    case PDTA_SparseTable:
		lod->lod_SparseTable = (UBYTE *) tidata;
		break;

	    case PDTA_FreeSourceBitMap:
		if (tidata)
		    lod->lod_Flags |= LODF_FREESRC;
		else
		    lod->lod_Flags &= ~LODF_FREESRC;
		break;

	    case PDTA_ModeID:
		lod->lod_ModeID = tidata;
		break;

	    case PDTA_NumColors:
		lod->lod_NumColors = (WORD) tidata;
		break;

		/* Remember shared data!!! Our subclass is telling us about the new bitmap. */
	    case PDTA_BitMap:
		lod->lod_OurBMap = (struct BitMap *) tidata;
	    case PDTA_ClassBitMap:
		lod->lod_SourceBMap = (struct BitMap *) tidata;
		break;

	    case PDTA_Screen:
		lod->lod_Screen = (struct Screen *) tidata;
		break;

	    case PDTA_Grab:
		lod->lod_Point = *((Point *) tidata);
		break;

	    case DTA_VisibleVert:
	    case DTA_TotalVert:
	    case DTA_VisibleHoriz:
	    case DTA_TotalHoriz:
		refresh = 1L;
		break;
	}
    }

    if (lod->lod_NumColors != ncolors)
    {
	struct FrameInfo *fri;
	ULONG size1;
	ULONG size2;
	ULONG size3;
	ULONG size4;
	ULONG size5;
	ULONG size6;
	ULONG size7;
	ULONG msize;

	GetAttr (DTA_FrameInfo, o, (ULONG *) & fri);

	ncolors = lod->lod_NumColors;

	if (lod->lod_Colors)
	    FreeVec (lod->lod_Colors);

	/* How big is the histogram table */
	lod->lod_HistoSize = sizeof (ULONG) * ncolors;

	/* Compute amount of memory needed for colors */
	size1 = sizeof (struct ColorRegister) * ncolors + 2;	/* lod_Colors */
	size2 = size1 + (sizeof (LONG) * 3 * ncolors) + 2;	/* lod_CRegs */
	size3 = size2 + (sizeof (LONG) * 3 * ncolors * 2) + 2;	/* lod_GRegs */
	size4 = size3 + (sizeof (UBYTE) * ncolors) + 2;		/* lod_ColorTable */
	size5 = size4 + (sizeof (UBYTE) * ncolors) + 2;		/* lod_ColorTable2 */
	size6 = size5 + (sizeof (UBYTE) * ncolors * 2) + 2;	/* lod_Allocated */
	size7 = size6 + lod->lod_HistoSize + 2;			/* lod_Histogram */
	msize = size7 + lod->lod_HistoSize + 2;			/* lod_Histogram2 */

	if (lod->lod_Colors = AllocVec (msize, MEMF_CLEAR))
	{
	    lod->lod_CRegs       = MEMORY_N_FOLLOWING (lod->lod_Colors, size1);
	    lod->lod_GRegs       = MEMORY_N_FOLLOWING (lod->lod_Colors, size2);
	    lod->lod_ColorTable  = MEMORY_N_FOLLOWING (lod->lod_Colors, size3);
	    lod->lod_ColorTable2 = MEMORY_N_FOLLOWING (lod->lod_Colors, size4);
	    lod->lod_Allocated   = MEMORY_N_FOLLOWING (lod->lod_Colors, size5);
	    lod->lod_Histogram   = MEMORY_N_FOLLOWING (lod->lod_Colors, size6);
	    lod->lod_Histogram2  = MEMORY_N_FOLLOWING (lod->lod_Colors, size7);
	}

	fri->fri_ColorMap = (struct ColorMap *) lod->lod_CRegs;
    }

    if (obm != lod->lod_SourceBMap)
    {
	struct FrameInfo *fri;

	GetAttr (DTA_FrameInfo, o, (ULONG *) & fri);

#if 0
	fri->fri_Dimensions.Width = lod->lod_BMHD.bmh_PageWidth;
	fri->fri_Dimensions.Height = lod->lod_BMHD.bmh_PageHeight;
	fri->fri_Dimensions.Depth = lod->lod_BMHD.bmh_Depth;
#else
	fri->fri_Dimensions.Width = GetBitMapAttr (lod->lod_SourceBMap, BMA_WIDTH);
	fri->fri_Dimensions.Height = GetBitMapAttr (lod->lod_SourceBMap, BMA_HEIGHT);
	fri->fri_Dimensions.Depth = GetBitMapAttr (lod->lod_SourceBMap, BMA_DEPTH);
#endif
    }

    if (modeid != lod->lod_ModeID)
    {
	struct DimensionInfo dim;
	struct DisplayInfo di;
	struct FrameInfo *fri;
	ULONG flags = NULL;

	GetAttr (DTA_FrameInfo, o, (ULONG *) & fri);

	di.NotAvailable = TRUE;
	if (!GetDisplayInfoData (NULL, (APTR) & di, sizeof (struct DisplayInfo), DTAG_DISP, lod->lod_ModeID) || di.NotAvailable)
	{
	    if (lod->lod_ModeID & 0x800)
		flags |= DIPF_IS_HAM;
	    else if (lod->lod_ModeID & 0x80)
		flags |= DIPF_IS_EXTRAHALFBRITE;
	    if ((modeid = bestmodeid (cb,
				      BIDTAG_SourceID, lod->lod_ModeID,
				      BIDTAG_DIPFMustHave, flags,
#if 1
				      BIDTAG_NominalWidth,  lod->lod_BMHD.bmh_PageWidth,
				      BIDTAG_NominalHeight, lod->lod_BMHD.bmh_PageHeight,
#else
				      BIDTAG_NominalWidth,  lod->lod_BMHD.bmh_Width,
				      BIDTAG_NominalHeight, lod->lod_BMHD.bmh_Height,
#endif
				      BIDTAG_DesiredWidth,  lod->lod_BMHD.bmh_Width,
				      BIDTAG_DesiredHeight, lod->lod_BMHD.bmh_Height,
				      BIDTAG_Depth, lod->lod_BMHD.bmh_Depth,
				      TAG_DONE)) != INVALID_ID)
	    {
		lod->lod_ModeID = modeid;
		GetDisplayInfoData (NULL, (APTR) & di, sizeof (struct DisplayInfo), DTAG_DISP, lod->lod_ModeID);
	    }
	}

	if (!di.NotAvailable)
	{
	    GetDisplayInfoData (NULL, (APTR) & dim, sizeof (struct DimensionInfo), DTAG_DIMS, lod->lod_ModeID);
	    fri->fri_Dimensions.Depth = MIN (fri->fri_Dimensions.Depth, dim.MaxDepth);
	    fri->fri_PropertyFlags = di.PropertyFlags;
	    fri->fri_Resolution = *(&di.Resolution);
	    fri->fri_RedBits = di.RedBits;
	    fri->fri_GreenBits = di.GreenBits;
	    fri->fri_BlueBits = di.BlueBits;
	}
    }

    return (refresh);
}

/*****************************************************************************/

/* 6 7 3 */
#define	RED_SCALE	6
#define	GREEN_SCALE	7
#define	BLUE_SCALE	3

/*****************************************************************************/

#define	SQ(x)		((x)*(x))
#define	AVGC(i1,i2,c)	((lod->lod_GRegs[((i1) * 3) + (c)]>>1)+(lod->lod_GRegs[((i2) * 3) + (c)]>>1))

/*****************************************************************************/

static ULONG color_error (ULONG r1, ULONG r2, ULONG g1, ULONG g2, ULONG b1, ULONG b2)
{
    r1 >>= 24;
    g1 >>= 24;
    b1 >>= 24;
    r2 >>= 24;
    g2 >>= 24;
    b2 >>= 24;
    return (SQ (r1 - r2) + SQ (g1 - g2) + SQ (b1 - b2));
}

/*****************************************************************************/

static ULONG framePictureDT (struct ClassBase * cb, Class * cl, Object * o, struct dtFrameBox * dtf)
{
    struct FrameInfo *fri;
    ULONG retval = 1L;

    if (!(dtf->dtf_FrameFlags & FRAMEF_SPECIFY))
    {
	/* Tell me about yourself */
	GetAttr (DTA_FrameInfo, o, (ULONG *) & fri);
	CopyMem (fri, dtf->dtf_FrameInfo, MIN (dtf->dtf_SizeFrameInfo, sizeof (struct FrameInfo)));
    }

    return (retval);
}

/*****************************************************************************/

#if 0
void CopyBitMap (struct ClassBase * cb, struct BitMap * bm1, struct BitMap * bm2)
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
#endif

/*****************************************************************************/
/* Clean this up.  It's gone through several evolutions and has gotten dirty */
/*****************************************************************************/

static ULONG layoutMethod (struct ClassBase * cb, Class * cl, Object * o, struct gpLayout * gpl)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct localData *lod = INST_DATA (cl, o);
    ULONG errLevel = RETURN_OK;
    struct Screen *scr = NULL;
    ULONG errNumber = 0L;
    struct IBox *domain;
    BOOL remap = FALSE;
    ULONG retval = 1L;
    STRPTR title;

    LONG testr, testg, testb;
    struct TagItem tags[2];
    struct BitMap *fbm;
    ULONG table[3];
    ULONG odepth;
    ULONG depth;
    LONG i, j;
    LONG err1;
    LONG err2;
    LONG got;

    BOOL send = FALSE;

    /* Obtain the layout lock */
    ObtainSemaphore (&si->si_Lock);

    /* Set the layout flag */
    si->si_Flags |= DTSIF_LAYOUT;

    /* Get the name and the domain */
    GetAttr (DTA_ObjName, o, (ULONG *) & title);
    GetAttr (DTA_Domain, o, (ULONG *) & domain);

    /* Get a screen pointer */
    if (lod->lod_Screen)
	scr = lod->lod_Screen;
    else if (gpl && gpl->gpl_GInfo)
	scr = gpl->gpl_GInfo->gi_Screen;

    /* Need to remap the bitmap to the destination screen */
    if (gpl->gpl_Initial && lod->lod_SourceBMap)
    {
	/* Do we need to send any information */
	send = TRUE;

	/* Free the extra information */
	freeExtraInfo (cb, lod);

	/* Clear the histogram table */
	memset (lod->lod_Histogram, 0, lod->lod_HistoSize);

	/* We have to have a screen */
	if (scr)
	{
	    /* We only do remap if we have color information */
	    if (lod->lod_Colors)
	    {
		/* Set up tags */
		tags[0].ti_Tag = OBP_Precision;
		tags[0].ti_Data = lod->lod_Precision;
		tags[1].ti_Tag = TAG_DONE;

		/* Time to do major conversion */
		lod->lod_ColorMap = scr->ViewPort.ColorMap;
		depth = MAX (scr->RastPort.BitMap->Depth, (ULONG) lod->lod_BMHD.bmh_Depth);

		/* Get a pointer to the friend bitmap */
		fbm = &(scr->BitMap);

#if 0
		/* Fill the histogram table */
		TallyBitMap (lod->lod_SourceBMap, lod->lod_Histogram, lod->lod_BMHD.bmh_Width);
#endif

		/* Do we have a sparse table? */
		if ((lod->lod_Flags & LODF_SPARSE) &&
		    (lod->lod_SparseCM = GetColorMap (lod->lod_NumSparse)))
		{
		    for (i = 0; i < lod->lod_NumSparse; i++)
		    {
			GetRGB32 (scr->ViewPort.ColorMap, lod->lod_SparseTable[i], 1, (ULONG *) table);
			SetRGB32CM (lod->lod_SparseCM, i, table[0], table[1], table[2]);
		    }
		}

		/* If they asked us to remap, or they told us to be like a screen
		 * which is deeper then the picture, then we have to allocate a work bitmap. */
		if ((lod->lod_Flags & LODF_REMAP) || (lod->lod_Screen && (lod->lod_BMHD.bmh_Depth < lod->lod_Screen->RastPort.BitMap->Depth)))
		{
		    lod->lod_BMap = AllocBitMap ((ULONG) lod->lod_BMHD.bmh_Width, (ULONG) lod->lod_BMHD.bmh_Height, depth, BMF_CLEAR, fbm);
		    lod->lod_Flags &= ~LODF_SAMEBM;
		}
		else
		{
		    lod->lod_Flags |= LODF_SAMEBM;
		}

		if (lod->lod_Flags & LODF_REMAP)
		{
		    if (lod->lod_BMap)
		    {
			/* Fill the histogram table */
			TallyBitMap (lod->lod_SourceBMap, lod->lod_Histogram, lod->lod_BMHD.bmh_Width);

			for (i = 0; i < lod->lod_NumColors; i++)
			{
			    if (lod->lod_Histogram[i])
			    {
				if (lod->lod_Flags & LODF_SPARSE)
				{
				    got = FindColor (lod->lod_SparseCM, lod->lod_CRegs[i * 3 + 0], lod->lod_CRegs[i * 3 + 1], lod->lod_CRegs[i * 3 + 2], lod->lod_NumSparse);
				    got = lod->lod_SparseTable[got];
				}
				else
				{
				    got = ObtainBestPenA (lod->lod_ColorMap, lod->lod_CRegs[i * 3 + 0], lod->lod_CRegs[i * 3 + 1], lod->lod_CRegs[i * 3 + 2], tags);
				    GetRGB32 (lod->lod_ColorMap, got, 1, (lod->lod_GRegs + (lod->lod_NumAlloc * 3)));
				    lod->lod_Allocated[lod->lod_NumAlloc++] = got;
				}
				if (got != i)
				    remap = TRUE;
				lod->lod_ColorTable[i] = lod->lod_ColorTable2[i] = got;
			    }
			}

			if (remap)
			{
			    for (i = 0; i < lod->lod_NumColors; i++)
			    {
				err1 = color_error (
						       lod->lod_CRegs[i * 3 + 0], lod->lod_GRegs[i * 3 + 0],
						       lod->lod_CRegs[i * 3 + 1], lod->lod_GRegs[i * 3 + 1],
						       lod->lod_CRegs[i * 3 + 2], lod->lod_GRegs[i * 3 + 2]);

				if (err1 > 2000)
				{
				    for (j = 0; j < lod->lod_NumAlloc; j++)
				    {
					testr = AVGC (i, j, 0);
					testg = AVGC (i, j, 1);
					testb = AVGC (i, j, 2);

					err2 = color_error (
							       lod->lod_CRegs[i * 3 + 0], testr,
							       lod->lod_CRegs[i * 3 + 1], testg,
							       lod->lod_CRegs[i * 3 + 2], testb);

					if (err2 < err1)
					{
					    err1 = err2;
					    lod->lod_ColorTable2[i] = lod->lod_Allocated[j];
					}
				    }
				}
			    }

			    odepth = lod->lod_SourceBMap->Depth;
			    lod->lod_SourceBMap->Depth = MIN (odepth, 8);
			    XLateBitMap (lod->lod_SourceBMap, lod->lod_BMap, lod->lod_ColorTable, lod->lod_ColorTable2, lod->lod_BMHD.bmh_Width);
			    lod->lod_SourceBMap->Depth = odepth;
			}
		    }
		    else
		    {
			errLevel = RETURN_FAIL;
			errNumber = ERROR_NO_FREE_STORE;
			retval = 0L;
		    }
		}

		if (lod->lod_SparseCM)
		    FreeColorMap (lod->lod_SparseCM);
	    }

	    if (!remap && lod->lod_BMap && !(lod->lod_Flags & LODF_SAMEBM))
	    {
#if 0
		/* So that we can do fast bitmaps */
		CopyBitMap (cb, lod->lod_SourceBMap, lod->lod_BMap);
#else
		BltBitMap (lod->lod_SourceBMap, 0, 0,
			   lod->lod_BMap, 0, 0, lod->lod_BMHD.bmh_Width, lod->lod_BMHD.bmh_Height,
			   0xC0, 0xFF, NULL);
#endif
	    }
	}
	else if (!(lod->lod_Flags & LODF_REMAP))
	{
	    lod->lod_Flags |= LODF_SAMEBM;
	}

	if (lod->lod_Flags & LODF_SAMEBM)
	{
	    lod->lod_Flags &= ~LODF_FREESRC;
	    lod->lod_BMap = lod->lod_SourceBMap;
	}

	WaitBlit ();

	if (lod->lod_Flags & LODF_FREESRC)
	{
	    FreeBitMap (lod->lod_OurBMap);
	    lod->lod_OurBMap = NULL;
	}
    }

    /* Clear the layout flag */
    si->si_Flags &= ~DTSIF_LAYOUT;

    si->si_VisVert  = (LONG) domain->Height;
    si->si_TotVert  = (LONG) lod->lod_BMHD.bmh_Height;

    si->si_VisHoriz = (LONG) domain->Width;
    si->si_TotHoriz = (LONG) lod->lod_BMHD.bmh_Width;

    if (send)
    {
	/* Tell everyone about the changes that we made */
	notifyAttrChanges (cb, o, gpl->gpl_GInfo, NULL,
			   GA_ID,		G (o)->GadgetID,
			   DTA_NominalVert,	(LONG) lod->lod_BMHD.bmh_Height,
			   DTA_NominalHoriz,	(LONG) lod->lod_BMHD.bmh_Width,
			   DTA_ErrorLevel,	errLevel,
			   DTA_ErrorNumber,	errNumber,
			   DTA_Title,		title,
			   TAG_DONE);
    }

    /* Release the layout lock */
    ReleaseSemaphore (&si->si_Lock);

    return (retval);
}

/*****************************************************************************/

static UWORD ghost_pattern[] = {0x4444, 0x1111, 0x4444, 0x1111};

/*****************************************************************************/

static ULONG renderMethod (struct ClassBase * cb, Class * cl, Object * o, struct gpRender * msg)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct localData *lod = INST_DATA (cl, o);
    struct IBox *domain;
    struct RastPort *rp;
    LONG tx, ty, bx, by;
    LONG width, height;
    struct IBox *sel;

    if (!lod->lod_BMap || (si->si_Flags & DTSIF_LAYOUT) || !AttemptSemaphoreShared (&si->si_Lock))
	return (0);

    GetAttr (DTA_Domain, o, (ULONG *) & domain);
    GetAttr (DTA_SelectDomain, o, (ULONG *) & sel);

    width = (ULONG) MIN (lod->lod_BMHD.bmh_Width, domain->Width);
    height = (ULONG) MIN (lod->lod_BMHD.bmh_Height, domain->Height);

    /* Get a pointer to the rastport */
    if (rp = msg->gpr_RPort)
    {
	BltBitMapRastPort (lod->lod_BMap, (ULONG) si->si_TopHoriz, (ULONG) si->si_TopVert,
			   rp, domain->Left, domain->Top, width, height, 0xC0);
	if (sel)
	{
	    tx = (LONG) sel->Left - si->si_TopHoriz;
	    ty = (LONG) sel->Top - si->si_TopVert;
	    bx = tx + (LONG) sel->Width - 1;
	    by = ty + (LONG) sel->Height - 1;

	    if ((tx < width) && (ty < height) && (bx > 0) && (by > 0))
	    {
		if (tx < 0)
		    tx = 0;
		if (ty < 0)
		    ty = 0;
		if (bx > width - 1)
		    bx = width - 1;
		if (by > height - 1)
		    by = height - 1;

		SetAfPt (rp, ghost_pattern, 2);
		SetDrMd (rp, JAM1);
		SetAPen (rp, 2);
		WaitBlit ();
		RectFill (rp, domain->Left + tx, domain->Top + ty, domain->Left + bx, domain->Top + by);
	    }
	}

	si->si_OTopVert = si->si_TopVert;
	si->si_OTopHoriz = si->si_TopHoriz;
    }
    ReleaseSemaphore (&si->si_Lock);
    return (1L);
}

/*****************************************************************************/

static ULONG printMethod (struct ClassBase * cb, Class * cl, Object * o, struct dtPrint * msg)
{
    struct localData *lod = INST_DATA (cl, o);
    struct GadgetInfo *gi = msg->dtp_GInfo;
    struct RastPort *prp;
    union printerIO *pio;
    UWORD width, height;
    struct ColorMap *cm;
    struct RastPort rp;
    ULONG retval = 0L;
    BOOL going = TRUE;
    struct IBox *sel;
    UWORD left, top;
    UWORD iospecial;
    LONG destcols;
    LONG destrows;
    ULONG sigr;
    ULONG psig;
    ULONG tmpl;

    if (pio = msg->dtp_PIO)
    {
	/* What bit are we going to wait on? */
	psig = 1L << pio->ios.io_Message.mn_ReplyPort->mp_SigBit;

	if (GetAttr (DTA_SelectDomain, o, (ULONG *) & sel) && sel)
	{
	    left   = sel->Left;
	    top    = sel->Top;
	    width  = sel->Width;
	    height = sel->Height;
	}
	else
	{
#if 1
	    left   = 0;
	    top    = 0;
	    width  = lod->lod_BMHD.bmh_Width;
	    height = lod->lod_BMHD.bmh_Height;
#else
	    /* Fill in the window rectangle */
	    QueryOverscan (lod->lod_ModeID, &rect, OSCAN_VIDEO);
	    left   = 0;
	    top    = 0;
	    width  = lod->lod_BMHD.bmh_PageWidth;	/* rect.MaxX - rect.MinX + 1; */
	    height = lod->lod_BMHD.bmh_PageHeight;	/* rect.MaxY - rect.MinY + 1; */
#endif
	}

	/* Get a color map */
	if (cm = GetObjectColorMap (cb, cl, o))
	{
	    /* Prepare the RastPort */
	    InitRastPort (&rp);
	    rp.BitMap = lod->lod_SourceBMap;
	    prp = (struct RastPort *) GetTagData (DTA_RastPort, (ULONG)&rp, msg->dtp_AttrList);

	    destcols = destrows = 0;
#if 0
	    iospecial = SPECIAL_ASPECT;
#else
	    /* Compute the flags & size */
	    if ((lod->lod_BMHD.bmh_Width == width) && (lod->lod_BMHD.bmh_Height == height))
	    {
		iospecial = SPECIAL_FULLCOLS | SPECIAL_ASPECT;
	    }
	    else
	    {
		iospecial = SPECIAL_FRACCOLS | SPECIAL_ASPECT;
		tmpl = lod->lod_BMHD.bmh_Width;
		tmpl = tmpl << 16;
		destcols = (tmpl / width) << 16;
	    }
#endif

	    destcols  = (LONG)  GetTagData (DTA_DestCols, (ULONG)destcols,  msg->dtp_AttrList);
	    destrows  = (LONG)  GetTagData (DTA_DestRows, (ULONG)destrows,  msg->dtp_AttrList);
	    iospecial = (UWORD) GetTagData (DTA_Special,  (ULONG)iospecial, msg->dtp_AttrList);

	    /* Prepare to dump the RastPort */
	    pio->iodrp.io_Command = PRD_DUMPRPORT;
	    pio->iodrp.io_RastPort = prp;
	    pio->iodrp.io_ColorMap = cm;
	    pio->iodrp.io_Modes = lod->lod_ModeID;
	    pio->iodrp.io_SrcX = left;
	    pio->iodrp.io_SrcY = top;
	    pio->iodrp.io_SrcWidth = width;
	    pio->iodrp.io_SrcHeight = height;
	    pio->iodrp.io_DestCols = destcols;
	    pio->iodrp.io_DestRows = destrows;
	    pio->iodrp.io_Special = iospecial;
	    SendIO ((struct IORequest *) pio);

	    while (going)
	    {
		/* Wait for something to happen */
		sigr = Wait (psig | SIGBREAKF_CTRL_C);

		/* Did they try to abort us? */
		if (sigr & SIGBREAKF_CTRL_C)
		{
		    /* Abort the IO */
		    AbortIO ((struct IORequest *) pio);
		    WaitIO ((struct IORequest *) pio);

		    /* Reset the printer (otherwise they get garbage the next time they print) */
		    pio->ios.io_Command = CMD_WRITE;
		    pio->ios.io_Data = "\033c";
		    pio->ios.io_Length = -1L;
		    DoIO ((struct IORequest *) pio);
		    going = FALSE;
		}

		/* Is the print complete? */
		if (sigr & psig)
		{
		    while (GetMsg (pio->ios.io_Message.mn_ReplyPort)) ;
		    going = FALSE;
		}
	    }

	    /* Return the return value from the print request */
	    if ((retval = (ULONG) pio->ios.io_Error) == 0)
	    {
		DoMethod (o, DTM_CLEARSELECTED, gi);
	    }

	    /* Free the temporary color map */
	    FreeColorMap (cm);
	}
    }

    return retval;
}

/*****************************************************************************/

static ULONG ASM Dispatch (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct ClassBase *cb = (struct ClassBase *) cl->cl_UserData;
    struct DTSpecialInfo *si;
    struct localData *lod;
    Object *newobj;
    ULONG retval;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (newobj = (Object *) DoSuperMethodA (cl, o, msg))
	    {
		lod = INST_DATA (cl, newobj);
		lod->lod_ModeID = INVALID_ID;
		lod->lod_Flags |= LODF_REMAP;
		lod->lod_Precision = PRECISION_IMAGE;

		si = (struct DTSpecialInfo *) G (newobj)->SpecialInfo;
		si->si_VertUnit = si->si_HorizUnit = 1;
		si->si_OTopVert = si->si_OTopHoriz = -1;
		setPictureDTAttrs (cb, cl, newobj, (struct opSet *) msg);
	    }
	    retval = (ULONG) newobj;
	    break;

	case OM_GET:
	    retval = getPictureDTAttr (cb, cl, o, (struct opGet *) msg);
	    break;

	case OM_UPDATE:
	case OM_SET:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    retval += setPictureDTAttrs (cb, cl, o, (struct opSet *) msg);
	    if (retval)
	    {
		struct RastPort *rp;
		struct gpRender gpr;

		/* Get a pointer to the rastport */
		if (rp = ObtainGIRPort (((struct opSet *) msg)->ops_GInfo))
		{
		    /* Force a redraw */
		    gpr.MethodID = GM_RENDER;
		    gpr.gpr_GInfo = ((struct opSet *) msg)->ops_GInfo;
		    gpr.gpr_RPort = rp;
		    gpr.gpr_Redraw = GREDRAW_UPDATE;
		    DoMethodA (o, &gpr);

		    /* Release the temporary rastport */
		    ReleaseGIRPort (rp);
		}

		/* Clear the return value */
		retval = 0L;
	    }
	    break;

	case GM_LAYOUT:
	    notifyAttrChanges (cb, o, ((struct gpLayout *) msg)->gpl_GInfo, NULL,
			       GA_ID, G (o)->GadgetID,
			       DTA_Busy, TRUE,
			       TAG_DONE);
	    retval = DoSuperMethodA (cl, o, msg);
	    retval += DoAsyncLayout (o, (struct gpLayout *) msg);
	    break;

	case DTM_PROCLAYOUT:
	    notifyAttrChanges (cb, o, ((struct gpLayout *) msg)->gpl_GInfo, NULL,
			       GA_ID, G (o)->GadgetID,
			       DTA_Busy, TRUE,
			       TAG_DONE);
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);

	case DTM_ASYNCLAYOUT:
	    retval += layoutMethod (cb, cl, o, (struct gpLayout *) msg);
	    break;

	case GM_RENDER:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    retval += renderMethod (cb, cl, o, (struct gpRender *) msg);
	    break;

	case DTM_FRAMEBOX:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    retval += framePictureDT (cb, cl, o, (struct dtFrameBox *) msg);
	    break;

	case DTM_CLEARSELECTED:
	    ((struct DTSpecialInfo *) (G (o)->SpecialInfo))->si_Flags &= ~DTSIF_HIGHLIGHT;
	case DTM_SELECT:
	    {
		struct dtSelect *dts = (struct dtSelect *) msg;
		struct RastPort *rp;

		/* Get a pointer to the rastport */
		if (rp = ObtainGIRPort (dts->dts_GInfo))
		{
		    struct gpRender gpr;

		    /* Force a redraw */
		    gpr.MethodID = GM_RENDER;
		    gpr.gpr_GInfo = dts->dts_GInfo;
		    gpr.gpr_RPort = rp;
		    gpr.gpr_Redraw = GREDRAW_REDRAW;
		    DoMethodA (o, &gpr);

		    /* Release the temporary rastport */
		    ReleaseGIRPort (rp);
		}
		/* Clear the return value */
		retval = 0L;
	    }
	    break;

	case DTM_PRINT:
	    retval = printMethod (cb, cl, o, (struct dtPrint *) msg);
	    break;

	case DTM_COPY:
	    retval = copyMethod (cb, cl, o, (struct dtGeneral *) msg);
	    break;

	case DTM_WRITE:
	    retval = writeMethod (cb, cl, o, (struct dtWrite *) msg);
	    break;

	case OM_DISPOSE:
	    /* Wait for any blits to finish */
	    WaitBlit ();

	    /* Get a pointer to the instance data */
	    lod = INST_DATA (cl, o);

	    freeExtraInfo (cb, lod);

	    if (lod->lod_Colors)
		FreeVec (lod->lod_Colors);

	    /* As soon as we have data sharing get rid of this!!! */
	    if (lod->lod_OurBMap)
		FreeBitMap (lod->lod_OurBMap);

	    /* Let the superclass handle everything else */
	default:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}

/*****************************************************************************/

Class *initClass (struct ClassBase * cb)
{
    Class *cl;

    if (cl = MakeClass (PICTUREDTCLASS, DATATYPESCLASS, NULL, sizeof (struct localData), 0L))
    {
	cl->cl_Dispatcher.h_Entry = (ULONG (*)()) Dispatch;
	cl->cl_UserData = (ULONG) cb;
	AddClass (cl);
    }
    return (cl);
}
