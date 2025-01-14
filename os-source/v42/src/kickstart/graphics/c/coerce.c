/******************************************************************************
*
*	$Id: coerce.c,v 42.0 93/06/16 11:16:49 chrisg Exp $
*
******************************************************************************/


	#include "/modeid.h"
	#include "/coerce.h"
	#include "/displayinfo.h"
	#include "c.protos"
	#include "/gfxpragmas.h"
	#include <utility/tagitem.h>

/****** graphics.library/CoerceMode ********************************************
*
*   NAME
*	CoerceMode -- calculate ViewPort mode coercion (V39)
*
*   SYNOPSIS
*	ID = CoerceMode(RealViewPort, MonitorID, Flags);
*	d0              a0            d0         d1
*
*	ULONG CoerceMode(struct ViewPort *, ULONG, ULONG);
*
*   FUNCTION
*	To determine the best mode in the MonitorID to coerce RealViewPort to,
*	given the restrictions set in Flags.
*
*   INPUTS
*	RealViewPort - ViewPort to coerce
*	MonitorID    - Montor number to coerce to (ie a mode masked with
*	               MONITOR_ID_MASK).
*	Flags        - PRESERVE_COLORS - keep the number of bitplanes
*	               in the ViewPort.
*	               AVOID_FLICKER - do not coerce to an interlace mode
*
*   RESULTS
*	ID      - ID of the best mode to coerce to, or INVALID_ID if could not
*	          coerce (see NOTES).
*
*   NOTES
*	This function takes into account the compatibility of the Monitor
*	being coerced to, and the ViewPort that is being coerced.
*	Incompatibilities will cause a result of INVALID_ID.
*
*   EXAMPLE
*	newmode = CoerceMode(vp, VGA_MONITOR_ID, PRESERVE_COLORS);
*
*   SEE ALSO
*	<graphics/coerce.h> <graphics/displayinfo.h>
*
******************************************************************************/


/*#define DEBUG*/
#ifdef DEBUG
#define D(x) {x;}
#else
#define D(x)
#endif

ULONG __asm CoerceMode(register __a0 struct ViewPort *vp, register __d0 ULONG MonitorID, register __d1 ULONG Flags)
{
    ULONG ID = gfx_GetVPModeID(vp);
    ULONG mntr;
    ULONG must, mustnot;
    int t = 0;
    struct DisplayInfo dinfo;
    struct TagItem tag[9];

    D(kprintf("CoerceMode(). vp = 0x%lx, MonitorID = 0x%lx, Flags = 0x%lx\n", vp, MonitorID, Flags);)
    D(kprintf("ID = 0x%lx\n", ID);)

    mntr = MonitorID & MONITOR_ID_MASK;
    if (((ID >> 16) == 0) && 
        (mntr == ((GBASE->DisplayFlags & PAL) ? PAL_MONITOR_ID : NTSC_MONITOR_ID)))
    {
	/* coercing default monitor to PAL or NTSC */
	D(kprintf("coercing default\n");)
	return(mntr | ID);
    }

    /* If coercing to itself, and this is the first ViewPort, then check
     * the alignment, as intuition uses this feature to promote misaligned
     * screens. If it's not the first ViewPort (as declared by intuition, then
     * let BestModeID() handle monitor compatibility issues.
     */
    if (((ID >> 16) == (MonitorID >> 16)) && (Flags & IGNORE_MCOMPAT) &&
        (CalcFMode(vp) != -1))
    {
	/* coercing to itself */
	D(kprintf("Coercing itself\n");)
	return(ID);
    }

    must = NULL;
    mustnot = SPECIAL_FLAGS;
    tag[t].ti_Tag = BIDTAG_ViewPort;
    tag[t++].ti_Data = (ULONG)vp;
    tag[t].ti_Tag = BIDTAG_MonitorID;
    tag[t++].ti_Data = MonitorID;
    if (gfx_GetDisplayInfoData(NULL, (APTR)&dinfo, sizeof(struct DisplayInfo), DTAG_DISP, ID))
    {
	must = (dinfo.PropertyFlags & SPECIAL_FLAGS);
	mustnot = (SPECIAL_FLAGS & ~must);
	if ((Flags & AVOID_FLICKER) && (!(dinfo.PropertyFlags & DIPF_IS_LACE)))
	{
		/* we don't want lace if AVOID_FLICKER is set, and this
		 * ViewPort is not naturally laced.
		 */
		mustnot |= DIPF_IS_LACE;
	}
	D(kprintf("must = 0x%lx, mustnot = 0x%lx\n", must, mustnot);)

	tag[t].ti_Tag = BIDTAG_RedBits;
	tag[t++].ti_Data = dinfo.RedBits;
	tag[t].ti_Tag = BIDTAG_GreenBits;
	tag[t++].ti_Data = dinfo.GreenBits;
	tag[t].ti_Tag = BIDTAG_BlueBits;
	tag[t++].ti_Data = dinfo.BlueBits;
    }
    tag[t].ti_Tag = BIDTAG_DIPFMustNotHave;
    tag[t++].ti_Data = mustnot;
    tag[t].ti_Tag = BIDTAG_DIPFMustHave;
    tag[t++].ti_Data = must;
    if (!(Flags & IGNORE_MCOMPAT))
    {
	tag[t].ti_Tag = BIDTAG_GfxPrivate;
	tag[t++].ti_Data = BIDTAG_COERCE;
    }
    tag[t].ti_Tag = TAG_DONE;

    return(gfx_BestModeIDA(tag));
}
