/******************************************************************************
*
*	$Id: bestid.c,v 39.11 93/04/15 11:47:17 spence Exp $
*
******************************************************************************/

	#include <exec/types.h>
	#include "/modeid.h"
	#include "/view.h"
	#include "/displayinfo.h"
	#include "/gfx.h"
	#include "/gfxbase.h"
	#include "/displayinfo_internal.h"
	#include "/macros.h"
	#include "/gfxpragmas.h"
	#include "c.protos"
	#include "/d/d.protos"
	#include <clib/utility_protos.h>
	#include <pragmas/utility_pragmas.h>

/****** graphics.library/BestModeIDA *******************************************
*
*   NAME
*	BestModeIDA -- calculate the best ModeID with given parameters (V39)
*	BestModeID  -- varargs stub for BestModeIDA()
*
*   SYNOPSIS
*	ID = BestModeIDA(TagItems)
*	d0               a0
*
*	ULONG BestModeIDA(struct TagItem *);
*
*	ID = BestModeID(Tag1, ...)
*
*	ULONG BestModeID(ULONG, ...);
*
*   FUNCTION
*	To determine the best ModeID to fit the parameters set in the TagList.
*
*   INPUTS
*	TagItems - A pointer to an array of TagItems.
*
*   TAGS
*	BIDTAG_DIPFMustHave (ULONG) - Mask of DIPF_ flags
*	       (from DisplayInfo->PropertyFlags) that the returned ModeID
*	       must have.
*	       Default - NULL
*
*	BIDTAG_DIPFMustNotHave (ULONG) - Mask of DIPF_ flags that the
*	       returned ModeID must not have.
*	       Default - SPECIAL_FLAGS
*
*	BIDTAG_ViewPort (struct ViewPort *) - ViewPort for which a best-fit
*	       ModeID is sought.
*	       Default - NULL
*
*	BIDTAG_NominalWidth (UWORD),
*	BIDTAG_NominalHeight (UWORD) - together make the aspect ratio.
*	       These values override the vp->DWidth and vp->DHeight values
*	       in the given ViewPort.
*	       Default - SourceID NominalDimensionInfo if BIDTAG_SourceID is
*	       passed, or vp->DWidth and vp->DHeight if BIDTAG_ViewPort is 
*	       passed, or 640 x 200.
*
*	BIDTAG_DesiredWidth (UWORD),
*	BIDTAG_DesiredHeight (UWORD) - Used to distinguish between two
*	       mode IDs with identical aspect ratios.
*	       Default - same values as NominalWidth and NominalHeight.
*
*	BIDTAG_Depth (UBYTE) - minimum the returned ModeID must support.
*	       Default - vp->RasInfo->BitMap->Depth if BIDTAG_ViewPort is
*	       passed, else 1.
*
*	BIDTAG_MonitorID (ULONG) - returned ModeID must use this monitor.
*	       Default - will not restrict the search to any particular monitor
*
*	BIDTAG_SourceID (ULONG) - Use this ModeID instead of a ViewPort.
*	       If specified, the DIPFMustHave mask is made up of the
*	       ((DisplayInfo->PropertyFlags of this ID & SPECIAL_FLAGS) |
*	        DIPFMustHave flags).
*	       Default - VPModeID(vp) if BIDTAG_ViewPort was passed, else the 
*	       DIPFMustHave and DIPFMustNotHave masks are left unchanged.
*
*	BIDTAG_RedBits (UBYTE),
*	BIDTAG_BlueBits (UBYTE),
*	BIDTAG_Greenits (UBYTE) - Minimum bits per gun the resultant
*	       ModeID must support.
*	       Default - 4 bits per gun.
*
*   RESULTS
*	ID      - ID of the best mode to use, or INVALID_ID if a match could
*	          not be found.
*
*   NOTES
*	This function takes into account the Compatability of the Monitor
*	being matched to, and the source ViewPort or ModeID.
*	Incompatibilitys will cause a result of INVALID_ID.
*
*	BIDTAG_NominalWidth, BIDTAG_NominalHeight, 
*	BIDTAG_DesiredWidth, BIDTAG_DesiredHeight, must all be non-0.
*
*	The comparisons are made against the DimensionInfo->Nominal values.
*	ie, this will not return a best fit against overscan dimensions.
*
*   EXAMPLE
*	IFF Display Program with a HAM image, to be displayed in the same
*	monitor type as the Workbench ViewPort.
*
*	ID = BestModeID(BIDTAG_NominalWidth, IFFImage->Width,
*	                BIDTAG_NominalHeight, IFFImage->Height,
*	                BIDTAG_Depth, IFFImage->Depth,
*	                BIDTAG_DIPFMustHave, DIPF_IS_HAM,
*	                BIDTAG_MonitorID, (GetVPModeID(WbVP) & MONITOR_ID_MASK),
*	                TAG_END);
*
*	To make an interlace version of a ViewPort:
*
*	ID = BestModeID(BIDTAG_ViewPort, ThisViewPort,
*	                BIDTAG_MustHave, DIFP_IS_LACE,
*	                TAG_END);
*
*   SEE ALSO
*	<graphics/modeid.h> <graphics/displayinfo.h>
*
******************************************************************************/

/*#define DEBUG*/
#ifdef DEBUG
#define D(x) {x;}
#else
#define D(x)
#endif

#define DONT_CARE PROTO_MONITOR_ID
#define DFT_MUSTHAVE 0
#define DFT_MUSTNOTHAVE SPECIAL_FLAGS
#define DFT_VP NULL
#define DFT_MID DONT_CARE
#define DFT_SID INVALID_ID
#define DFT_NOMW 640
#define DFT_NOMH 200
#define DFT_DESW 640
#define DFT_DESH 200
#define DFT_DEPTH 1
#define DFT_RBITS 4
#define DFT_GBITS 4
#define DFT_BBITS 4

#define	LABS(x)	(((LONG)(x) < 0) ? (-(LONG)(x)) : (x))

/* Space savings. GetTagDataUser is implemented in misc.asm as: */
ULONG __asm GetTagDataUser(register __d0 Tag tagValue, register __d1 ULONG def, register __a0 struct TagItem *tagList);
#define GetTagData(tag, def, list) GetTagDataUser((tag)&~(TAG_USER),def,list)

ULONG __asm BestModeIDA(register __a0 struct TagItem *tag)
{
    struct DimensionInfo dims;
    struct MonitorInfo srcminfo, dstminfo;
    DisplayInfoHandle srchandle = NULL, dsthandle = NULL;
    struct DisplayInfoRecord *rec;
    struct DisplayInfo *cdi = (struct DisplayInfo *)&srcminfo;
    ULONG nextid = INVALID_ID;
    ULONG DesAspect;
    ULONG BestAspect = 0;
    UWORD bestwidth = 0, bestheight = 0;
    UWORD bestdepth;

    ULONG bestid = INVALID_ID;
    ULONG MustHaveMask;
    ULONG MustNotHaveMask;
    ULONG MID;
    ULONG SID;
    struct ViewPort *vp;
    BOOL dfltmon = FALSE;
    BOOL coercion = FALSE;
    BOOL find = TRUE;
    UWORD NomW;
    UWORD NomH;
    UWORD DesW;
    UWORD DesH;
    UWORD deftw = DFT_NOMW, defth = DFT_NOMH;
    UWORD min_pixelspeed = 35;
    WORD fmode = BANDWIDTH_4X;
    UBYTE RBits;
    UBYTE GBits;
    UBYTE BBits;

    D(kprintf("BestModeID(). TagList = 0x%lx\n", tag);)

    ObtainSemaphore(GBASE->MonitorListSemaphore);

    /* Initialise the data */
    MustHaveMask = GetTagData(BIDTAG_DIPFMustHave, DFT_MUSTHAVE, tag);
    MustNotHaveMask = GetTagData(BIDTAG_DIPFMustNotHave, (DFT_MUSTNOTHAVE & ~(MustHaveMask)), tag);
    vp = (struct ViewPort *)GetTagData(BIDTAG_ViewPort, DFT_VP, tag);
    bestdepth = GetTagData(BIDTAG_Depth, ((vp && vp->RasInfo && vp->RasInfo->BitMap) ? vp->RasInfo->BitMap->Depth : DFT_DEPTH), tag);
    MID = GetTagData(BIDTAG_MonitorID, DFT_MID, tag);
    if ((MID >> 16) == 0)
    {
	MID = ((GBASE->monitor_id << 16) | 0x1000);
	dfltmon = TRUE;
    }
    SID = GetTagData(BIDTAG_SourceID, (vp ? gfx_GetVPModeID(vp) : DFT_SID), tag);
    if (vp)
    {
	deftw = vp->DWidth;
	defth = vp->DHeight;
	if ((fmode = CalcFMode(vp)) == -1)
	{
		/* If CalcFMode() fails, 
		 * we'll just have to find a match in 1x.
		 */
		D(kprintf("fmode failed\n");)
		fmode = BANDWIDTH_1X;
	}
	/* If the ViewPort is not aligned for 4x, then we have to limit the
	 * the type of mode we can return. This is done by specifying a
	 * minimum pixel speed that we can support with the given alignment
	 * and depth.
	 */
	if (fmode != BANDWIDTH_4X)
	{
		if (fmode == BANDWIDTH_1X)
		{
			if (bestdepth > 2)
			{
				min_pixelspeed = 70;
			}
			if (bestdepth > 4)
			{
				min_pixelspeed = 140;
			}
		}
		else
		{
			/* must be 2x */
			if (bestdepth > 4)
			{
				min_pixelspeed = 70;
			}
		}
	}
    }
    if (gfx_GetDisplayInfoData(NULL, (APTR)&dims, sizeof(struct DimensionInfo), DTAG_DIMS, SID))
    {
	deftw = ((dims.Nominal.MaxX - dims.Nominal.MinX) + 1);
	defth = ((dims.Nominal.MaxY - dims.Nominal.MinY) + 1);
    }
    NomW = GetTagData(BIDTAG_NominalWidth, deftw, tag);
    NomH = GetTagData(BIDTAG_NominalHeight, defth, tag);
    DesW = GetTagData(BIDTAG_DesiredWidth, NomW, tag);
    DesH = GetTagData(BIDTAG_DesiredHeight, NomH, tag);
    RBits = GetTagData(BIDTAG_RedBits, DFT_RBITS, tag);
    GBits = GetTagData(BIDTAG_GreenBits, DFT_GBITS, tag);
    BBits = GetTagData(BIDTAG_BlueBits, DFT_BBITS, tag);
    coercion = (BOOL)FindTagItem(BIDTAG_GfxPrivate, tag);

    D(kprintf("Using:\n");)
    D(kprintf("MustHaveMask    = 0x%lx\n", MustHaveMask);)
    D(kprintf("MustNotHaveMask = 0x%lx\n", MustNotHaveMask);)
    D(kprintf("MonitorID = 0x%lx\n", MID);)
    D(kprintf("SourceID  = 0x%lx\n", SID);)
    D(kprintf("ViewPort = 0x%lx\n", vp);)
    D(kprintf("Nominal = (0x%lx, 0x%lx)\n", NomW, NomH);)
    D(kprintf("Desired = (0x%lx, 0x%lx)\n", DesW, DesH);)
    D(kprintf("Depth = 0x%lx\n", bestdepth);)
    D(kprintf("fmode = 0x%lx\n", fmode);)
    D(kprintf("RGB Bits = 0x%lx, 0x%lx, 0x%lx\n", RBits, GBits, BBits);)
    D(kprintf("Coercion = %s\n", (coercion ? "TRUE" : "FALSE"));)
    D(kprintf("min_pixelspeed = %ld\n", min_pixelspeed);)

    /* Now I have all the parameters, find the match */

    srchandle = (DisplayInfoHandle)gfx_FindDisplayInfo(SID);
    if (rec = find_key((struct DisplayInfoRecord *)(SubRecord((struct DisplayInfoRecord *)GBASE->DisplayInfoDataBase)), (MID >> 16), (UWORD)~0))
    {
	dsthandle = SubRecord(rec);
	/* Start the search with this ID */
	nextid = ((((struct DisplayInfoRecord *)dsthandle)->rec_MajorKey << 16) |
	          (((struct DisplayInfoRecord *)dsthandle)->rec_MinorKey));
	D(kprintf("dsthandle = 0x%lx, nextid = 0x%lx\n", dsthandle, nextid);)
    }

    /* Check Monitor compatibility if coercing... */
    if (coercion && 
        gfx_GetDisplayInfoData(srchandle, (APTR)&srcminfo, sizeof(struct MonitorInfo), DTAG_MNTR, SID) &&
        gfx_GetDisplayInfoData(dsthandle, (APTR)&dstminfo, sizeof(struct MonitorInfo), DTAG_MNTR, MID))
    {

	WORD srccomp = srcminfo.Compatibility;
	WORD dstcomp = dstminfo.Compatibility;

	D(kprintf("check compatibilty\n");)
	if (((srccomp == MCOMPAT_NOBODY) || (dstcomp == MCOMPAT_NOBODY)) ||
	     (((srccomp == MCOMPAT_SELF) || (dstcomp == MCOMPAT_SELF)) &&
	      ((SID & MONITOR_ID_MASK) != MID)))
	{
		D(kprintf("Not compatible\n");)
		find = FALSE;
	}
    }
    else
    {
	D(kprintf("No MonitorInfo\n");)
//	find = FALSE;
    }
    
    if (((MID == DONT_CARE) || (find)) && (srchandle ? (gfx_GetDisplayInfoData(srchandle, (APTR)cdi, sizeof(struct DisplayInfo), DTAG_DISP, SID)) : TRUE))
    {
	ULONG MustHave = (srchandle ? ((cdi->PropertyFlags & SPECIAL_FLAGS) | MustHaveMask) : MustHaveMask);
	ULONG MustNotHave = (~(MustHave) & MustNotHaveMask);
	BOOL havewidth = FALSE, haveheight = FALSE;
	/* Use integers to keep the aspect ratio */
	ULONG ThisAspect;
	ULONG BestDiff;
	BOOL BestIsLaced = TRUE;

	DesAspect = ((NomW << 16) / NomH);
	BestDiff = DesAspect;

	D(kprintf("MustHave = 0x%lx, MustNotHave = 0x%lx\n", MustHave, MustNotHave);)
	D(kprintf("DesiredAspect = 0x%lx\n", DesAspect);)

	/* Loop through all the modes in the database.
	 * For each mode, if the appropriate DIPF_ bits are set and cleared, then
	 * calculate the aspect ratio of the mode, and compare that to the closest
	 * aspect ratio we have so far. If this mode's ratio is better, or if it's
	 * identical but a more appropriate dimension, then store this mode as
	 * the best so far.
	 */

	find = TRUE;
	do
	{
		if ((MID != DONT_CARE) && ((nextid & MONITOR_ID_MASK) != MID))
		{
			/* There are no more IDs for this monitor type */
			find = FALSE;
		}
		if ((nextid & MONITOR_ID_MASK) && ((MID == DONT_CARE) || ((nextid & MONITOR_ID_MASK) == MID)))
		{
			DisplayInfoHandle handle = (DisplayInfoHandle)gfx_FindDisplayInfo(nextid);
			D(kprintf("nextid = 0x%lx, handle = 0x%lx\n", nextid, handle);)

			/* Check that this ModeID matches our necessary requirements */

			if (gfx_GetDisplayInfoData(handle, (APTR)cdi, sizeof(struct DisplayInfo), DTAG_DISP, nextid) &&
			    ((cdi->PropertyFlags & MustHave) == MustHave) &&
			    ((cdi->PropertyFlags & MustNotHave) == 0) &&
			     (cdi->NotAvailable == 0) &&
			    (cdi->PixelSpeed >= min_pixelspeed) &&
			    (gfx_GetDisplayInfoData(handle, (APTR)&dims, sizeof(struct DimensionInfo), DTAG_DIMS, nextid)))
			{
				/* we have a possible candidate.
				 * do the checks to increase best match.
				 *
				 * I believe it is more important to preserve the depth
				 * than the width of the coerced mode, so depth takes
				 * priority in any comparisons.
				 */
	
				WORD tnomw = (dims.Nominal.MaxX + 1);
				WORD tnomh = (dims.Nominal.MaxY + 1);

				ThisAspect = ((tnomw << 16) / tnomh);
				D(kprintf("tnomw = 0x%lx, tnomh = 0x%lx, aspect = 0x%lx\n", tnomw, tnomh, ThisAspect);)
				D(kprintf("havewidth = %s, haveheight = %s\n", (havewidth ? "TRUE" : "FALSE"), (haveheight ? "TRUE" : "FALSE"));)

				if (dims.MaxDepth >= bestdepth)
				{
					D(kprintf("Checking RGB\n");)
					if ((cdi->RedBits >= RBits) &&
					    (cdi->GreenBits >= GBits) &&
					    (cdi->BlueBits >= BBits))
					{
						ULONG ThisDiff;
						D(kprintf("BestDiff = 0x%lx\n", BestDiff);)

						/* This revolting conditional is the heart of the matter.
						 * This ModeID is the one if:
						 * 1) First time through.
						 * 2) The aspect ratio of this mode is closer to the desired aspect
						 *    ratio than the current best choice.
						 * 3) If this aspect ratio is better or equal to the current best one,
						 *    and these dimensions are a better fit than the current best.
						 * 4) If this mode has the same dimensions as the current best, but the
						 *    current best is laced (try to reduce interlace).
						 */

						if ( ( ((ThisDiff = (LABS(ThisAspect - DesAspect))) <= BestDiff) &&
						      ((((!(haveheight)) && (LABS(tnomh + bestheight - DesH) <= DesH)) ||
						       ((!(havewidth)) && (LABS(tnomw + bestwidth - DesW) <= DesW))) ||
						       ((haveheight && (tnomh == DesH)) ||
						        (havewidth && (tnomw == DesW)))) )
						     || (bestheight == 0))	/* (bestheight == 0) if there is no match yet */
						{
							D(kprintf("Could be it...");)
							if ((!((tnomw == bestwidth) && (tnomh == bestheight))) ||
							    (BestIsLaced))
							{
								D(kprintf(">>> best so far = 0x%lx\n", nextid);)
								bestdepth = MIN(dims.MaxDepth, bestdepth);
								bestid = nextid;
								BestAspect = ThisAspect;
								BestDiff = LABS(ThisAspect - DesAspect);
								bestwidth = tnomw; bestheight = tnomh;
								havewidth = (tnomw >= DesW); haveheight = (tnomh >= DesH);
								BestIsLaced = (cdi->PropertyFlags & DIPF_IS_LACE);
							}
						}
					}
				}
			}
		}
	}
	while (((nextid = gfx_NextDisplayInfo(nextid)) != INVALID_ID) && (find));
	if (dfltmon)
	{
		bestid &= ~MONITOR_ID_MASK;
	}
    }
    ReleaseSemaphore(GBASE->MonitorListSemaphore);

    D(kprintf("Returning 0x%lx\n", bestid);)
    return(bestid);
}
