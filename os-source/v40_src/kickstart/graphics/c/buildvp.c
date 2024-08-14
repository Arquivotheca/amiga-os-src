/******************************************************************************
*
*	$Id: buildvp.c,v 39.75 93/04/15 16:15:29 spence Exp $
*
******************************************************************************/

#define ECS_SPECIFIC		/* for custom.h */

	#include <hardware/custom.h>
	#include "/copper.h"
	#include "/displayinfo.h"
	#include "/displayinfo_internal.h"
	#include "/view.h"
	#include "/vp_internal.h"
	#include "/monitor.h"
	#include "/macros.h"
	#include "/gfxbase.h"
	#include "/gfx.h"
	#include "/gfxpragmas.h"
	#include "c.protos"
	#include <exec/memory.h>

extern struct Custom custom;
extern UWORD dflt_clrs[];

#define USEERR
/*#define USEFNCOUNT*/
#define USEGBDEBUG
/*#define DEBUG*/
/*#define BADFMODE*/
#define MAKEFMODE

/* Switches to enable chip bug-fixing code */
/* #define VBLANK_BUG - set in lmkfile */
/*#define H0_BUG*/
/*#define ALICE_RHS_BUG*/
#define ALICE_LHS_BUG

/* A possible fix that never got implemented ..... */
/* #define ALICE_LHS_WRAPAROUND */

#ifdef USEGBDEBUG
#define GBDEBUG if (GBASE->Debug)
#else
#define GBDEBUG
#endif

#ifdef DEBUG
#define D(x) {GBDEBUG {x};}
#define DG(x) {x}
#else
#define D(x)
#define DG(x)
#endif

#ifdef WACKDEBUG
#define WACK Debug()
#else
#define WACK
#endif

#ifdef USEFNCOUNT
#define INITERR ULONG  err = (bd->FnCount << 16);
#define INC_FNCOUNT {bd->FnCount++; D(kprintf("err = 0x%lx\n", err); WACK;)}
#else
#define INITERR ULONG err = 0;
#define INC_FNCOUNT
#endif

#ifdef USEERR
#define ERR(x) {err |= x;}
#define GOODERR {err = MVP_OK;}
#define RETERR err
#else
#define INC_FNCOUNT
#define INITERR
#define ERR(x)
#define GOODERR
#define RETERR (0)
#endif

#ifdef BADFMODE
#define FMODE io->pad3b[0]
#define FMODE2 io->fmode
#else
#define FMODE io->fmode
#endif

/* A Collection of routines needed to build the native Amiga display mode's
 * intermediate copper lists.
 *
 * All driver routines take a View, ViewPort and ViewPortExtra pointers as 
 * parameters. They all return a ULONG value. The upper word is the number of the
 * routine in the list in which the error occured, and the low word is
 * dependent on the function and the error.
 * A return of 0L is a GOOD return.
 *
 * These are defined in vp_internal.h
 */


/* InitMVP() - must be the 1st function to be called.
 * It takes a pointer to a pointer to a ViewPortExtra, because
 * if the vpe is NULL, create one, and set a flag to show it's creation. This
 * will be removed in the cleanup code.
 * This is the only function in the list that takes a pointer to the BuildData,
 * which will also be initialised. It also takes a pointer to the ProgInfo.
 */

ULONG InitMVP(struct View *v, struct ViewPort *vp, struct ViewPortExtra **vpe, struct ProgInfo *pinfo, struct BuildData *bd)
{
#ifdef BADFMODE
    struct Custom *io = &custom;
#endif
    struct ViewPortExtra *_vpe = *vpe;
    struct ColorMap *cm = vp->ColorMap;
    ULONG  err;
    UWORD  fmode;

    D(kprintf("In InitMVP()\nView = 0x%lx, ViewPort = 0x%lx, VPExtra = 0x%lx\nProgInfo = 0x%lx\n", v, vp, _vpe, pinfo);)
    D(kprintf("vpe->Flags = 0x%lx, vp->DWidth = 0x%lx, vp->DHeight = 0x%lx\n", (_vpe ? _vpe->Flags : 0), vp->DWidth, vp->DHeight);)
    D(kprintf("vp->DxOffset = 0x%lx vp->DyOffset = 0x%lx, v->DxOffset = 0x%lx, v->DyOffset = 0x%lx\n", vp->DxOffset, vp->DyOffset, v->DxOffset, v->DyOffset);)
    D(kprintf("RasInfo = 0x%lx, BitMap = 0x%lx\n", vp->RasInfo, vp->RasInfo->BitMap);)
    D(kprintf("bm->BytesPerRow = 0x%lx vp->Modes = 0x%lx\n", vp->RasInfo->BitMap->BytesPerRow, vp->Modes);)
    D(kprintf("new_mode(vp) = 0x%lx\n", new_mode(vp));)
    WACK;

    GOODERR
    if ((fmode = CalcFMode(vp)) == (UWORD) -1)
    {
    	err = MVP_NO_DISPLAY;
    }
    else
    {
	if (_vpe == NULL)
	{
		D(kprintf("_vpe == NULL\n");)
		if (_vpe = (struct ViewPortExtra *)gfx_GfxNew(VIEWPORT_EXTRA_TYPE))
		{
			D(kprintf("Allocated a new ViewPortExtra\n"); WACK;)
			_vpe->Flags = VPXF_FREE_ME;
			*vpe = _vpe;
			if ((cm) && (cm->Type))
			{
				cm->cm_vpe = _vpe;
			}
		}
		else
		{
			D(kprintf("GfxNew() failed\n");)
			err = MVP_NO_VPE;
		}		
	}
	if (err == MVP_OK)
	{
		struct ViewExtra *ve = NULL;

		/* initialise the BuildData structure */
		bd->bplcon0 = 0;
		bd->bplcon2 = get_bplcon2(cm);
		bd->bplcon3 = 0;
		bd->Flags = 0;
		bd->Offset = 0;
		bd->Offset2 = 0;
		bd->ToViewY = 0;
		bd->FMode = fmode;
		bd->Index = ((pinfo->ToViewX << GBASE->arraywidth) + fmode);
		bd->RHSFudge = bd->LHSFudge = FALSE;
		D(kprintf("fmode = 0x%lx\n", fmode);)

		if (v->Modes & EXTEND_VSTRUCT)
		{
			ve = (struct ViewExtra *)gfx_GfxLookUp(v);

		}
		if(ve && ve->Monitor)
		{
			bd->mspc = ve->Monitor;
		}
		else
		{
			bd->mspc = GBASE->natural_monitor;
		}

		if ((new_mode(vp) & (DOUBLESCAN | LACE)) == LACE)
		{
			bd->ToViewY = 1;
			bd->bplcon0 |= LACE;
			bd->Flags |= BD_IS_LACE;
		}
		/* Put the BuildData where it can be found */
		_vpe->DriverData[0] = (APTR)bd;
		bd->pinfo = pinfo;

		/* reset the origin offset (y component is currently untouched) */
		_vpe->Origin[0].x = _vpe->Origin[1].x = 0;
	}
    }

#ifdef BADFMODE
    io->copcon = 2;	/* turn on CDANG bit - what a fuck up */
#endif

    D(kprintf("Leaving InitMVP(), err = 0x%lx\n", err); WACK;);

    return(RETERR);
}

ULONG CleanUpMVP(DRIVER_PARAMS)
{
    D(kprintf("In CleanUpMVP()\nView = 0x%lx, ViewPort = 0x%lx, VPExtra = 0x%lx\n", v, vp, vpe);)
    D(kprintf("vpe->Flags = 0x%lx\n", vpe->Flags); WACK;)

    if (vpe)
    {
	if (vpe->Flags & VPXF_FREE_ME)
	{
		/* The vpe was allocated on the fly, so free it */
		D(kprintf("Freeing Allocated VPE 0x%lx\n", vpe); WACK;)
		if ((vp->ColorMap) && (vp->ColorMap->Type))
		{
			vp->ColorMap->cm_vpe = NULL;
		}
		gfx_GfxFree((struct ExtendedNode *)vpe);
	}
    }

    D(kprintf("Leaving CleanUpMVP()\n");)
    return(MVP_OK);
}



/* All the following functions *MUST* be called with valid BuildData and ProgInfo
 * pointers, to reduce error-checking code
 */

#define VIDEOOSCAN_FUDGE 10	/* value from V37 MakeVPort(). */

ULONG CalcDispWindow(DRIVER_PARAMS)
{

    /* CalcDispWindow() calculates the DiwStrt/Stop values of this viewport.
     * It stores the values in ViewPort coordinate resolution.
     */

    struct DisplayInfoRecord *record;
    struct ColorMap *cm = vp->ColorMap;
    struct Rectangle *DClip = &vpe->DisplayClip;
    struct BuildData *bd = (struct BuildData *)vpe->DriverData[0];
    struct ProgInfo *pinfo = bd->pinfo;
    ULONG mode = gfx_GetVPModeID(vp);
    UWORD  clip;
    UWORD  ToViewX = pinfo->ToViewX;
    UWORD  ToViewY = bd->ToViewY;
    WORD   dxstrt, dxstop, dystrt, dystop, dwidth, dheight, vdx;
    INITERR

    D(kprintf("In CalcDispWindow()\nView = 0x%lx, ViewPort = 0x%lx, VPExtra = 0x%lx\nProgInfo = 0x%lx, mode = 0x%lx\n", v, vp, vpe, bd->pinfo, mode); WACK;);

    /* First, let's get the display clip area into cliposcan.
     * If there is no DisplayClip in the ViewPortExtra->DisplayClip, then we
     * don't do any clipping, else use the MAX/MIN of either Coerce or
     * Normal DClips and the vpe->DClip.
     *
     * The DisplayClips are set by the user and/or intuition with
     * VideoControl().
     */

    vdx = v->DxOffset;
    if ((mode >> 16) == 4)
    {
	if (mode == A2024TENHERTZ_KEY)
	{
		D(kprintf("10Hz mode. FMode = 0x%lx\n", bd->FMode);)
		dwidth = 352;
		vdx -= 16;
		/* alignments only allow for 1x in 10hz */
		bd->FMode = BANDWIDTH_1X;
	}
	else
	{
		dwidth = 512;
	}
	dheight = ((GBASE->DisplayFlags & PAL) ? 256 : 200);
	bd->Flags |= (BD_IS_A2024 | BD_IS_LACE);
    }
    else
    {
    	dwidth = vp->DWidth;
	dheight = vp->DHeight;
    }

    clip = ((DClip->MinX) | (DClip->MinY) |
            (DClip->MaxX) | (DClip->MaxY));	/* do we have a DClip? */

    if ((cm && cm->Type && clip) &&
	((record = cm->CoerceDisplayInfo) || (record = cm->NormalDisplayInfo)))
    {
	D(kprintf("Have a record = 0x%lx\n", record);)
	bd->cliposcan.MinX = 
		SHORTMAX(record->rec_ClipOScan.MinX,DClip->MinX);
	bd->cliposcan.MinY = 
		SHORTMAX(record->rec_ClipOScan.MinY,DClip->MinY);
	bd->cliposcan.MaxX = 
		SHORTMIN(record->rec_ClipOScan.MaxX,DClip->MaxX);
	bd->cliposcan.MaxY = 
		SHORTMIN(record->rec_ClipOScan.MaxY,DClip->MaxY);
    }
    else
    {
	/* clear the bd->cliposcan to the contents of vpe->DisplayClip, which
	 * may be all 0's.
	 */
	bd->cliposcan = vpe->DisplayClip;
    }


    /* Calculate the displaywindow values asked for */

    dxstrt = vp->DxOffset;
    dystrt = ((vp->DyOffset + ToViewY) & ~(ToViewY));
    dystop = (dystrt + dheight);

    /* Do we need to clip it?
     * Clipping is in ViewPort resolution.
     */

    if (clip)
    {
	D(kprintf("Clipping! cliposcan at 0x%lx\n", &bd->cliposcan);)
	D(kprintf("(0x%lx, 0x%lx) - (0x%lx, 0x%lx)\n", bd->cliposcan.MinX, bd->cliposcan.MinY, bd->cliposcan.MaxX, bd->cliposcan.MaxY);)
	D(kprintf("dxstrt = 0x%lx, dxstop = ????, dystrt = 0x%lx, dystop = 0x%lx\n", dxstrt, dystrt, dystop); WACK;)
	clip = 1;
	dxstrt = MAX(dxstrt, bd->cliposcan.MinX);
	dxstop = MIN((dxstrt + dwidth), (bd->cliposcan.MaxX + 1));	/* inclusive */
	dystrt = MAX(dystrt, bd->cliposcan.MinY);
    }
    else
    {
	dxstop = (dxstrt + dwidth);		/* vp->DWidth is in VP resolution */
    }
    dystop = MAX(dystrt, dystop);

    if ((pinfo->Flags & (PROGINFO_NATIVE | PROGINFO_SCANDBL)) == (PROGINFO_NATIVE | PROGINFO_SCANDBL))
    {
	bd->Flags |= BD_IS_SDBL;
	dystrt <<= 1;
	dystop <<= 1;
    }

    dxstrt += (vdx << ToViewX);
    dystrt += (v->DyOffset << ToViewY);
    dxstop += (vdx << ToViewX);
    dystop += (v->DyOffset << ToViewY);

    /* Now clip to the hardware limits. */
    {
	struct MonitorSpec *mspc = bd->mspc;
	WORD xdiff, ydiff;

	/* if we have a VideoOScan dclip, do some magical chip fudging. */
	if (dxstop > (mspc->DeniseMaxDisplayColumn << ToViewX))
	{
		bd->Flags |= BD_VIDEOOSCAN;
		bd->DiwStopX = dxstop;	/* keep this for other calculations */
		dxstop = (VIDEOOSCAN_FUDGE << ToViewX);
		if (!(GBASE->ChipRevBits0 & GFXF_HR_DENISE))
		{
			dxstop += (mspc->DeniseMaxDisplayColumn << ToViewX);
		}
	}
	else
	{
		bd->DiwStopX = dxstop;
	}
	/* There are (TOTROWS + 2) rows in every Long Frame. However, there is
	 * no sprite DMA fetches on the very last line (YACB), so we limit
	 * DiwStopY to (TOTROWS + 1).
	 */
	bd->DiwStopY = MIN(dystop, ((mspc->total_rows + 1) << ToViewY));
	bd->DiwStrtX = MAX(dxstrt, (xdiff = (mspc->DeniseMinDisplayColumn << ToViewX)));
	bd->DiwStrtY = MAX(dystrt, (ydiff = (mspc->min_row << ToViewY)));
	if (!clip)
	{
	    	/* create the clip values we need for later */
		xdiff = (dxstrt - xdiff);
		ydiff = (dystrt - ydiff);
    		bd->cliposcan.MinX = (vp->DxOffset - (MIN(0, xdiff)));
	    	bd->cliposcan.MinY = (vp->DyOffset - (MIN(0, ydiff)));
	}
#ifdef ALICE_LHS_WRAPAROUND
	if (GBASE->Bugs & BUG_ALICE_LHS)
	{
		/* mspc->ms_xoffset is the number of colour clocks I move
		 * the View to overcome the LHS scroll problem.
		 */
		UWORD xoffset = (/*bd->mspc->ms_xoffset*/8 << (ToViewX + 1));
		D(kprintf("xoffset = 0x%lx\n", xoffset);)
		bd->DiwStopX += xoffset;
	}
#endif
    }

#ifdef H0_BUG
    if ((GBASE->Bugs & BUG_H0) && (ToViewX == 2))
    {
	/* 35ns modes cannot be positioned on 35ns boundaries */
    	bd->DiwStrtX &= ~1;
    	bd->DiwStopX &= ~1;
    }
#endif

    /* Now calculate the actual DiwStopY values that get written into the
     * copper list. This also simplifies the ScrollVPort() code.
     */
    {
	UWORD diwstopl, diwstops, diff;
	if (!(vpe->Flags & VPXF_LAST))
	{
		bd->DiwStopY &= ~(ToViewY);
	}
	diff = ((bd->DiwStopY - bd->DiwStrtY) & ToViewY);
	diwstopl = diwstops = (((dxstop << pinfo->ToDIWResn) >> 2) & 0xff);
	diwstops |= ((((bd->DiwStopY >> ToViewY) + diff) & 0xff) << 8);
	diwstopl |= (((bd->DiwStopY >> ToViewY) & 0xff) << 8);
	bd->RGADiwStopYL = diwstopl;
	bd->RGADiwStopYS = diwstops;
    }

    /* one last check: make sure that the top of the ViewPort is not below
     * the last visible display line. If so, set a special error code.
     */
    if ((bd->DiwStrtY >> ToViewY) > bd->mspc->total_rows)
    {
	ERR(MVP_OFF_BOTTOM)
    }

    D(kprintf("Leaving CalcDispWindow()\n"); WACK;);
    D(kprintf("dxstrt = 0x%lx, dxstop = 0x%lx, dystrt = 0x%lx, dystop = 0x%lx\n", bd->DiwStrtX, bd->DiwStopX, bd->DiwStrtY, bd->DiwStopY); WACK;);

    INC_FNCOUNT;
    return(RETERR);
}

ULONG CalcADispWindow(DRIVER_PARAMS)
{

    /* CalcADispWindow() calculates the DiwStrt/Stop values of this viewport
     * for the single-A chip set.
     * It stores the values in ViewPort coordinate resolution.
     */

    struct BuildData *bd = (struct BuildData *)vpe->DriverData[0];
    struct MonitorSpec *mspc = bd->mspc;
    UWORD  ToViewY = bd->ToViewY;
    INITERR

    D(kprintf("In CalcADispWindow()\nView = 0x%lx, ViewPort = 0x%lx, VPExtra = 0x%lx\nProgInfo = 0x%lx\n", v, vp, vpe, bd->pinfo); WACK;);

    ERR(CalcDispWindow(v, vp, vpe));		/* Calc for ECS/AA */

    /* The differences between ECS/AA displaywindow values and A displaywindow
     * values is that A displaywindow is open for the maximum possible
     * height.
     */

    /* For 'A', bd->DiwStrtY is used to hold the line number to enable the 
     * display.
     *
     * Was: bd->DiwStrtY = (mspc->min_row << ToViewY); - Jun 30 1992
     */
    bd->DiwStopY = (mspc->total_rows << ToViewY);

    /* Now calculate the actual DiwStopY values that get written into the
     * copper list.
     */
    {
	UWORD diff;
	diff = ((bd->DiwStopY - bd->DiwStrtY) & ToViewY);
	bd->RGADiwStopYS &= 0xff;	/* clear away the Y pos */
	bd->RGADiwStopYS |= ((((bd->DiwStopY - diff) >> ToViewY) & 0xff) << 8);
	bd->RGADiwStopYL = bd->RGADiwStopYS;
    }

    D(kprintf("Leaving CalcADispWindow()\n"); WACK;);
    D(kprintf("dxstrt = 0x%lx, dxstop = 0x%lx, dystrt = 0x%lx, dystop = 0x%lx\n", bd->DiwStrtX, bd->DiwStopX, bd->DiwStrtY, bd->DiwStopY); WACK;);

    INC_FNCOUNT;
    return(RETERR);
}
    
#define FIRSTFETCH 0x18
#define DENISE_OFFSET 1		/* just 1/2 a colour-clock, but times 2 for integers */
#define DIWX_TO_CC(x) ((((x) >> ToViewX) - DENISE_OFFSET) >> 1)

/* This table is designed to be:
 * 1) correct
 * 2) compatible with older DDFSTRT values.
 */
UWORD  StrtFetchMasks[][] = 
{
    {0xfff8, 0xfff0, 0xfff0, 0xffe0},
    {0xfff8, 0xfff8, 0xfff8, 0xfff0},
    {0xfff8, 0xfff8, 0xfff8, 0xfff8}
};

UWORD  StopFetchMasks[][] = 
{
    {0xfff8, 0xfff0, 0xfff0, 0xffe0},
    {0xfff8, 0xfff8, 0xfff8, 0xfff0},
    {0xfff8, 0xfff8, 0xfff8, 0xfff8}
};

WORD Overrun[][] =	/* (-(Number of cycles in the pattern)) */
{
    {0xfff8, 0xfff0, 0xfff0, 0xffe0},
    {0xfffc, 0xfff8, 0xfff8, 0xfff0},
    {0xfffe, 0xfffc, 0xfffc, 0xfff8}
};

WORD RealStops[][] =	/* == the number of bitplane fetches per cycle */
{
    {8, 8, 8, 8},	/* Lores (2x and 4x are 16 and 32 cycles, but only 8 fetches) */
    {4, 8, 8, 8},	/* Hires (2x is really 16 cycles, but only 8 cycles) */
    {2, 4, 4, 8}	/* SHires */
};

ULONG CalcDataFetch(DRIVER_PARAMS)
{

    /* CalcDataFetch() calculates the DDFSTRT/STOP values for the defined
     * display window. At this stage, the Display Window values are in
     * ViewPort resolution coordinates.
     */

    struct BuildData *bd = (struct BuildData *)vpe->DriverData[0];
    struct ProgInfo *pinfo = bd->pinfo;
    WORD  ddfstrt, ddfstop;
    UWORD  realstop = GBASE->RealStops[bd->Index];
    UWORD  maskstrt;
    UWORD  pixels;
    UBYTE  ToViewX = pinfo->ToViewX;
    INITERR

    D(kprintf("In CalcDataFetch()\nView = 0x%lx, ViewPort = 0x%lx, VPExtra = 0x%lx\nProgInfo = 0x%lx\n", v, vp, vpe, bd->pinfo); WACK;)

    /* for ddfstrt, convert the displaywindow values to ColourClocks, and subtract 
     * the DENISE_OFFSET. ie
     *
     * ((((diwstrt >> (ToViewX + 1)) << 1) - DENISE_OFFSET) >> 1) & granularity, or
     *
     * (((diwstrt >> ToViewX) - DENISE_OFFSET) >> 1) & granularity.
     *
     * The granularity is 8 for old chips, or 4 for ECS and AA, though for
     * compatibility, keep to 8.
     * 
     */

    bd->Cycle = GBASE->StrtFetchMasks[bd->Index];
    maskstrt = (pinfo->DDFSTRTMask & bd->Cycle);
    bd->Cycle = -((WORD)bd->Cycle);
    D(kprintf("bd->Cycle = 0x%lx\n", bd->Cycle);)

    ddfstrt = ((((bd->FirstFetch = (DIWX_TO_CC(bd->DiwStrtX) - realstop)) - FIRSTFETCH) & maskstrt) + FIRSTFETCH);
    ddfstrt = MAX(FIRSTFETCH, ddfstrt);

    /* for ddfstop, calculate the number of cycles that are needed, and add it to
     * ddfstrt.
     */

    pixels = ((16 << GBASE->bwshifts[bd->FMode]) * ((realstop < 8) ? (bd->Cycle / realstop) : 1));
    ddfstop = (ddfstrt + (bd->Cycle * (((bd->DiwStopX - bd->DiwStrtX - 1) / pixels) + 1)));

    if (ddfstop >= (bd->mspc->total_colorclocks - realstop))
    {
    	D(kprintf("clipping ddfstop (= 0x%lx) to total_colorclocks (= 0x%lx)\n", ddfstop, bd->mspc->total_colorclocks); WACK;)
    	ddfstop = ((((bd->mspc->total_colorclocks - realstop - DENISE_OFFSET) - FIRSTFETCH) & maskstrt) + FIRSTFETCH);

	/* but in the case of narrow ViewPorts, it is quite possible that ddfstrt
	 * and ddfstop are now the same. This will not do! Easy solution - subtract
	 * a fetch cycle from ddfstrt.
	 * Caveat - on a 'A' machine which does not have fine display window control,
	 * this may cause the early fetch to be visible. But seeing as 'A' doesn't
	 * really work on ViewPorts narrower than 64 pixels, I don't see this as
	 * being a practical problem.
	 */
	 if (ddfstrt == ddfstop)
	 {
	 	ddfstrt -= bd->Cycle;
	 }
    }


    if ((pinfo->Flags & (PROGINFO_NATIVE | PROGINFO_VARBEAM)) == (PROGINFO_NATIVE | PROGINFO_VARBEAM))
    {
	bd->bplcon3 |= BPLCON3_EXTBLNKEN;
    }

    bd->DDFStrt = ddfstrt;
    bd->DDFStop = ddfstop;

    D(kprintf("Leaving CalcDataFetch\n"); WACK;);
    D(kprintf("ddfstrt = 0x%lx, ddfstop = 0x%lx\n", bd->DDFStrt, bd->DDFStop); WACK;);

    INC_FNCOUNT;
    return(RETERR);
}

#define SCROLLRESOLUTION (pinfo->ToViewX + (2 - GBASE->bwshifts[bd->FMode]))

ULONG CalcScroll(DRIVER_PARAMS)
{
    struct BuildData *bd = (struct BuildData *)vpe->DriverData[0];
    struct ProgInfo *pinfo = bd->pinfo;
    struct RasInfo *ri = vp->RasInfo;
    struct BitMap *bm;
    LONG   offset = 0;
    UWORD  diwstrt, datafetch;
    UWORD  resn, chunksize;
    UWORD  m;
    WORD   diff, shift;
    BOOL   needpre = FALSE;
    UBYTE  ToDIWResn = pinfo->ToDIWResn;

    INITERR

    if ((bd->Flags & BD_IS_DPF) && ri->Next)
    {
	ri = ri->Next;
    }
    bm = ri->BitMap;

    D(kprintf("In CalcScroll()\nView = 0x%lx, ViewPort = 0x%lx, VPExtra = 0x%lx\nProgInfo = 0x%lx\n", v, vp, vpe, bd->pinfo);)
    D(kprintf("DxOffset = 0x%lx, RxOffset = 0x%lx, DyOffset = 0x%lx, RyOffset = 0x%lx\n", vp->DxOffset, ri->RxOffset, vp->DyOffset, ri->RyOffset);)
    D(kprintf("bd->Flags = 0x%lx, vp->RasInfo = 0x%lx, ri = 0x%lx\n", bd->Flags, vp->RasInfo, ri); WACK;)

    if (bm)
    {
	m =(pinfo->Offset + GBASE->bwshifts[bd->FMode]);
	/* m is the cycle number:
	 * 0 = 2cycle,
	 * 1 = 4cycle, .....
	 * 4 = 32cycle.
	 */
#ifdef ALICE_RHS_BUG
	if ((GBASE->Bugs & BUG_ALICE_RHS) && (!(vpe->Flags & VPXF_LAST)))
	{
		bd->RHSFudge = TRUE;
	}
#endif
	/* convert the displaywindow coordinates to 35ns */
	diwstrt = (bd->DiwStrtX << ToDIWResn);

	/* now calculate the colourclock on which the last fetch of the
	 * first batch of fetches happens, and convert to 35ns.
	 *
	 * Note that for 16 and 32 cycles, the last fetch is the 8th cycle.
	 */
	datafetch = ((bd->DDFStrt + (2 << MIN(m, 2))) << 3);

	diff = diwstrt - datafetch - 4;	/* Remember the Denise delay */

	/* extra from vp and ri offsets, to 35ns */

	shift = (MIN(0, (vp->DxOffset - bd->cliposcan.MinX)) - ri->RxOffset) << ToDIWResn;	/* to 35ns */

	/* the scroll range, governed by the fetch cycle (32, 16, 8, 4, or 2) and
	 * the fmode.
	 */
	chunksize = SCROLLRESOLUTION;
	resn = (0x00ff >> chunksize) + 1;	/* scroll range for bplcon1 in 35ns */

	/* add it all together */
	diff += shift;
	if ((diff < 0) || (shift > 0))
	{
		needpre = TRUE;
	}
	D(kprintf("Shift = 0x%lx, resn = 0x%lx, diff = 0x%lx\n", shift, resn, diff); WACK;)

	/* shift now has the total number of 35ns pixels to scroll.
	 * How many whole bytes is this (in words)?
	 */

	offset += ((-diff / resn) << (1 + GBASE->bwshifts[bd->FMode]));
	diff += resn;
	diff &= (resn - 1);

	/* Do we need to prefetch? */
	if ((needpre) && (diff))
	{
		WORD prefetch = (pinfo->DDFSTRTMask & GBASE->StrtFetchMasks[bd->Index]);
		WORD realstop = GBASE->RealStops[bd->Index];
		D(kprintf("<<< Reducing ddfstrt for scroll from 0x%lx", bd->DDFStrt);)
		bd->DDFStrt += prefetch;
#ifdef ALICE_LHS_BUG
		/* Will need a better check than this for AA+ */
		if ((WORD)bd->DDFStrt < (WORD)FIRSTFETCH)
		{
			bd->DDFStrt = FIRSTFETCH;
			bd->LHSFudge = TRUE;
			/* Couldn't prefetch, so compensate the offset calculations */
			if (shift <= 0)
			{
				offset += (1 << (1 + GBASE->bwshifts[bd->FMode]));
			}
		}
		else
#endif
		{
			/* Is the difference between the new DDFSTRT and
			 * what it COULD be greater than a whole fetch cycle,
			 * or have we done some nasty scrolling with RxOffset?
			 */
			if ((realstop < 8) || (shift > 0))
			{
				/* Subtract (the number of words fetched per cycle *
				 * the number of whole cycles skipped).
				 */
				offset -= ((1 << (1 + GBASE->bwshifts[bd->FMode])) * 
				           (((-prefetch) >> realstop) + 1));
			}
		}
		D(kprintf(" to 0x%lx\n", bd->DDFStrt); WACK;)
	}

	/* add the vertical offset */
	offset -= (LONG)((MIN(0,(((vp->DyOffset + bd->ToViewY) & ~(bd->ToViewY)) - bd->cliposcan.MinY)) - ri->RyOffset) * bm->BytesPerRow);

	/* start on the correct boundary */
	offset &= (0xfffffffe << GBASE->bwshifts[bd->FMode]);

	bd->Offset = offset;
	bd->Scroll = diff;

    }
    else
    {
	bd->Offset = bd->Scroll = 0;
    }

    GOODERR

    D(kprintf("Leaving CalcScroll()\n"); WACK;);
    D(kprintf("scroll = 0x%lx\n", bd->Scroll); WACK;);
    D(kprintf("Offset = 0x%lx\n", bd->Offset); WACK;);

    INC_FNCOUNT;
    return(RETERR);

}


/* Fudge will detect the overscan cusp cases (h/w limitations), and alter the
 * FMode and everything else accordingly.
 */

#define HIRES4X ((pinfo->ToViewX == 1) && (fmode == BANDWIDTH_4X))
#define LORES4X ((pinfo->ToViewX == 0) && (fmode == BANDWIDTH_4X))
#define LORES2X ((pinfo->ToViewX == 0) && ((fmode == BANDWIDTH_2XNML) || (fmode == BANDWIDTH_2XDBL)))
#define VARBEAM35 ((pinfo->Flags & PROGINFO_VARBEAM) && (pinfo->ToViewX == 2) && (fmode))
#define VARBEAM70_140 ((pinfo->Flags & PROGINFO_VARBEAM) && (pinfo->ToViewX < 2))

ULONG Fudge(DRIVER_PARAMS)
{
    struct BuildData *bd = (struct BuildData *)vpe->DriverData[0];
    struct ProgInfo *pinfo = bd->pinfo;
    UWORD fmode = bd->FMode;
    UWORD cycles = bd->Cycle;
    BOOL lores4x = LORES4X;

    INITERR

    D(kprintf("In Fudge()\n"); WACK;)

    /* AA - We only need to fudge for Hires4x, Lores4x and Lores2x
     * when ddfstrt == FIRSTFETCH.
     */

#ifdef ALICE_LHS_BUG
    if ((bd->LHSFudge) && (!(pinfo->Flags & PROGINFO_VARBEAM)))
    {
	D(kprintf("LHS Fudging\n"); WACK;)

	if ((HIRES4X) && (bd->Scroll >= 0x70))
	{
		/* only in this certain range */
		D(kprintf("Hires 4x cusp case. Scroll = 0x%lx\n", bd->Scroll); WACK;)
		DG(struct Custom *io = &custom; io->color[0] = 0xf00;)
		fmode = BUS_32;	/* drop to 2x */
		cycles = 8;
	}
	if (lores4x && (bd->Scroll >= 0xf0))
	{
		D(kprintf("Lores 4x -> 1x. Scroll = 0x%lx\n", bd->Scroll); WACK;)
		DG(struct Custom *io = &custom; io->color[0] = 0x0f0;)
		fmode = BANDWIDTH_1X;
		cycles = 8;
	}
	if (lores4x && ((bd->Scroll >= 0x70) && (bd->Scroll < 0x80)))
	{
		D(kprintf("Lores 4x cusp case. Scroll = 0x%lx\n", bd->Scroll); WACK;)
		DG(struct Custom *io = &custom; io->color[0] = 0x0f0;)
		fmode = BANDWIDTH_1X;
		cycles = 8;
	}
	if (LORES2X && (bd->Scroll >= 0x70))
	{
		D(kprintf("Lores 2x cusp case. Scroll = 0x%lx\n\n", bd->Scroll); WACK;)
		DG(struct Custom *io = &custom; io->color[0] = 0x0f0;)
		fmode = BANDWIDTH_1X;
		cycles = 8;
	}
	if ((lores4x) && ((bd->Scroll >= 0x80) && (bd->Scroll < 0xf0)))
	{
		D(kprintf("Lores 4x -> 2x. Scroll = 0x%lx\n", bd->Scroll); WACK;)
		DG(struct Custom *io = &custom; io->color[0] = 0x00f;)
		fmode = BUS_32;
		cycles = 16;
	}
#ifdef COARSE_SCROLL
	/* There is another problem though in programmed beam-sync modes.
	 * They usually start fetching at colourclock 0x18, which limits their
	 * ability to scroll. This causes an ugly blank
	 * band on the LHS of the display. We cannot always drop the bandwidth,
	 * because for example 8 bitplane Productivity MUST have 4x.
	 * To overcome this, we force the display to remain static during smooth
	 * scrolling, and only move coarsly. The difference is stored in the 
	 * vpe->Origin point as an offset from vp->DxOffset. This offset can be
	 * read with VPOrigin().
	 *
	 * This problem also occurs under ECS in 70 and 140ns modes.
	 */
	if ((VARBEAM35) || (VARBEAM70_140))
	{
		UWORD max = (0xff >> SCROLLRESOLUTION);
		UWORD resn = (max >> 2);
		D(kprintf("Varbeam problem, max = 0x%lx resn = 0x%lx\n", max, resn); WACK;)
		/* Store the offset */
		vpe->Origin[0].x = (((max - bd->Scroll) + resn) >> pinfo->ToDIWResn);
		vpe->Origin[1].x = (((max - bd->Scroll2) + resn) >> pinfo->ToDIWResn);
		/* Force it to not change */
//		bd->DiwStrtX += (max - bd->Scroll);
		bd->DiwStopX = ((bd->cliposcan.MaxX << pinfo->ToDIWResn) + (v->DxOffset << pinfo->ToViewX));
		bd->Scroll = bd->Scroll2 = (resn + 1);
		bd->Offset -= (2 << GBASE->bwshifts[bd->FMode]);
		bd->Offset2 -= (2 << GBASE->bwshifts[bd->FMode]);
	}
#endif	/* COARSE_SCROLL */
    }
#endif	/* ALICE_LHS_BUG */

    /* 'A' fudging: */
    if ((bd->DiwStopX < 0x100) && (cycles == 8) && (!(GBASE->ChipRevBits0 & GFXF_HR_DENISE)))
    {
	/* The ddfstop calculations are 1 larger than is sometimes necessary, but
	 * this is cancelled out in the unneccessary situations by closing the
	 * display window at the right point. 'A' cannot do this, so only fetch
	 * what is needed. This fudge is mainly for narrow viewports.
	 */
	bd->DDFStop -= cycles;
    }

    /* We also want to maximise sprite usage. This may involve dropping the
     * bandwidth (if possible).
     *
     * The more complicated algorithm is as follows:
     * If there is more than sprite 0,
     * and ddfstrt < the ddfstrt needed to fetch the highest numbered sprite,
     * then
     *    calculate how many extra cycles we need to increment ddfstrt.
     *    calculate what we need to drop the bandwidth to for that value.
     *
     * Only bother with this test if we haven't fudged the fmode already or
     * we have only dropped the fmode to 2x, and there is more than one sprite
     * active.
     */

    if ((GBASE->SpriteReserved > 1) && ((fmode == bd->FMode) || (fmode)))
    {
	UWORD ddfwanted;
	WORD  diff;
	WORD  need;
	UBYTE sprite = FindMSBSet(GBASE->SpriteReserved);
	ddfwanted = (FIRSTFETCH + (sprite << 2));
	diff = (ddfwanted - bd->DDFStrt);
	need = (bd->FirstFetch - bd->DDFStrt);

	D(kprintf("need >1 sprite. ddfstrt = 0x%lx, max sprite number = 0x%lx\n", bd->DDFStrt, sprite);)
	D(kprintf("ddfwanted = 0x%lx, ccstrt = 0x%lx, cycles = 0x%lx\n", ddfwanted, bd->FirstFetch, cycles);)
	/* We must be a 16 or 32 cycle to be able to help, as we cannot fetch any later
	 * anyway. If we are a 16 or 32 cycle, then dropping the bandwidth means
	 * we can just add half the current cycle to ddfstrt, if the colour clock
	 * we need to start fetching by is within that range.
	 */
	if (diff && (cycles > 8))
	{
		/* can we drop the bandwidth? */
		if (need >= (cycles >>= 1))
		{
			fmode = ((fmode == BANDWIDTH_4X) ? BANDWIDTH_2XNML : BANDWIDTH_1X);
			need -= cycles;
			/* and further still? */
			if (((bd->DDFStrt + cycles) < ddfwanted) && (cycles > 8) && (need >= (cycles >>= 1)))
			{
				fmode = BANDWIDTH_1X;
			}
			D(kprintf("Dropped the bw to %lxx, ddfstrt = 0x%lx\n", fmode, bd->DDFStrt);)
		}
	}
    }

    if ((bd->FudgedFMode = fmode) != bd->FMode)	/* used later by BuildAAColors and ScrollVPort() */
    {
	bd->Index = ((pinfo->ToViewX << GBASE->arraywidth) + fmode); /* OK to change this, as we don't use it any more. */
	bd->FMode = fmode;
	CalcDataFetch(v, vp, vpe);
	err = CalcScroll(v, vp, vpe);
    }
    bd->Flags |= BD_FUDGEDFMODE;

    D(kprintf("Leaving Fudge(). Flags = 0x%lx\n", bd->Flags); WACK;)

    INC_FNCOUNT;
    return(RETERR);
}

ULONG CalcModulos(DRIVER_PARAMS)
{
    struct BuildData *bd = (struct BuildData *)vpe->DriverData[0];
    struct ProgInfo *pinfo = bd->pinfo;
    struct RasInfo *ri = vp->RasInfo;
    struct BitMap *bm = ri->BitMap;
    UBYTE shift = GBASE->bwshifts[bd->FudgedFMode];
    INITERR
    WORD  wpr, wpd;

    D(kprintf("In CalcModulos()\nView = 0x%lx, ViewPort = 0x%lx, VPExtra = 0x%lx\nProgInfo = 0x%lx\n", v, vp, vpe, bd->pinfo); WACK;);
    D(kprintf("Flags = 0x%lx\n", pinfo->Flags);)

    if (bd->Flags & BD_IS_DPF)
    {
	bm = ri->Next->BitMap;
    }

    /* The modulo is the number of words-per-row in the bitmap - number of
     * words fetched each scanline.
     *
     * The number of words fetched per scanline is the number of colourclocks
     * between ddfstop and ddfstrt, converted to pixels, /16, =
     * ((DDFSTOP - DDFSTRT) * bandwidth) + bandwidth
     *  ------------------    ~~~~~~~~~    ~~~~~~~~~
     *  Fetches per cycle     (^ 1, 2 or 4        ^)
     *
     */

    if (bm)
    {
	WORD overrun = -(GBASE->Overrun[(pinfo->ToViewX << GBASE->arraywidth) + bd->FudgedFMode]);
	wpr = (bm->BytesPerRow + 1) >> 1;	/* to words */
	wpd = (((bd->DDFStop - bd->DDFStrt) << shift) / overrun);

	/* DDFSTOP is where to START STOPPING. The number of fetch cycles after
	 * DDFSTOP (the final fetch) is always a minimum of 8. eg a 4 cycle mode
	 * will do 2 whole fetches.
	 * In the current rev of Alice (8374R3), the maximum count for the last
	 * fetch is 8 (ie a 32 cycle is truncated to 8 cycles).
	 */

	wpd += ((8 / MIN(overrun, 8)) * (1 << shift));
	D(kprintf("ddfstop = 0x%lx, wpr = 0x%lx, wpd = 0x%lx\n", bd->DDFStop, wpr, wpd); WACK;)

	bd->Modulo = ((wpr - wpd) << 1);	/* to bytes */
	bd->Modulo &= (0xfffe << shift);

	if (bd->Flags & BD_IS_LACE)
	{
		D(kprintf("Lace. Was 0x%lx\n", bd->Modulo); WACK;)
		bd->Modulo += bm->BytesPerRow;
	}

	if (bd->Flags & BD_IS_SDBL)
	{
		/* Refetch the last line (use the number of bytes actually fetched) */
		D(kprintf("Scandouble modulos\n");)
		bd->Modulo2 = bd->Modulo;
		bd->Modulo = -(wpd << 1);
		bd->Modulo &= (0xfffe << shift);
	}
	else
	{
		if (!(bd->Flags & BD_IS_DPF))
		{
			D(kprintf("Not DPF\n");)
			bd->Modulo2 = bd->Modulo;
		}
	}
    }
    else
    {
	bd->Modulo = 0;
    }
    
    GOODERR

    D(kprintf("Leaving CalcModulos()\n"); WACK;)
    D(kprintf("Modulo1 = 0x%lx Modulo2 = 0x%lx\n", bd->Modulo, bd->Modulo2); WACK;)

    INC_FNCOUNT;
    return(RETERR);
}

ULONG CalcDPFScrollMods(DRIVER_PARAMS)
{
    struct BuildData *bd = (struct BuildData *)vpe->DriverData[0];
    struct RasInfo *ri = vp->RasInfo;
    UWORD ddfstrtorig = bd->DDFStrt, ddfstrt1, ddfstrt2 = ddfstrtorig;
    LONG offset;
    BOOL isdpf = FALSE;

    INITERR

    D(kprintf("In CalcDPFScroll()\n"); WACK;)

    /* Problem with DualPlayfield - we must use the minimum ddfstrt value that
     * *both* playfields need.
     * CalcScroll() alters the ddfstrt value already calculated to minimise
     * bitplane DMA usage, so we need to keep track.
     */

    if ((ri->Next) && (ri->Next->BitMap))
    {
	isdpf = TRUE;
	bd->Flags |= BD_IS_DPF;
	CalcScroll(v, vp, vpe);		/* assume no error returned */
	ddfstrt2 = bd->DDFStrt;		/* store the (possibly altered) new ddfstrt */
	bd->DDFStrt = ddfstrtorig;	/* restore the original */
	bd->Offset2 = bd->Offset;	/* these are the values we want for plfd 2 */
	bd->Scroll2 = bd->Scroll;	
        bd->Flags &= ~BD_IS_DPF;	/* calculate scroll for plfd 1 */
    }
    CalcScroll(v, vp, vpe);

    D(kprintf("After 2 CalcScrolls, ddfstrt1 = 0x%lx, ddfstrt2 = 0x%lx\n", bd->DDFStrt, ddfstrt2); WACK;)

    /* use the minimum ddfstrt value of the two */
    if ((ddfstrt1 = bd->DDFStrt) != ddfstrt2)
    {
	/* handle the fact that the two ddfstrt values are different */
	offset = (2 << GBASE->bwshifts[bd->FMode]);
	if (ddfstrt1 < ddfstrt2)
	{
		/* we need to use this new ddfstrt value, and so alter
		 * playfield 2's offset accordingly.
		 */
		D(kprintf("Altering Offset2 by 0x%lx\n", offset); WACK;)
		bd->Offset2 -= offset;
	}
	else /* (ddfstrt2 < ddfstrt1) */
	{
		/* change the ddfstrt value, and the 1st playfield's
		 * offset.
		 */
		D(kprintf("Altering Offset1 by 0x%lx, changing ddfstrt to 0x%lx\n", offset, ddfstrt2); WACK;)
		bd->DDFStrt = ddfstrt2;
		bd->Offset -= offset;
	}
    }

    Fudge(v, vp, vpe);

    /* now calculate the modulo values */
    CalcModulos(v, vp, vpe);
    if (isdpf)
    {
	UWORD mods;
	bd->Flags |= BD_IS_DPF;
	CalcModulos(v, vp, vpe);
	mods = bd->Modulo;		/* swap the modulos over */
	bd->Modulo = bd->Modulo2;
	bd->Modulo2 = mods;
//	bd->Flags &= ~BD_IS_DPF;
    }
    
    D(kprintf("Leaving CalcDPFScroll()\n"); WACK;)

    INC_FNCOUNT;
    return(RETERR);
}

#define BPLCON3_BRDSPRT (1<<1)
#define KILLEHB 0x200
#define ECSENA_LOCKS (BPLCON3_EXTBLNKEN | BPLCON3_BRDSPRT | BPLCON3_ZDCLKEN | BPLCON3_BRDNTRAN | BPLCON3_BRDNBLNK)

ULONG MakeAABplcon(DRIVER_PARAMS)
{
#define SCROLL(x) ((((x) & 0x003c) >> 2) | (((x) & 0x0003) << 8) | (((x) & 0x00c0) << 4))

    struct BuildData *bd = (struct BuildData *)vpe->DriverData[0];
    struct ProgInfo *pinfo = bd->pinfo;
    struct RasInfo *ri = vp->RasInfo;
    struct BitMap *bm;
    INITERR
    UWORD  bpu = 0;

    D(kprintf("In MakeAABplcon()\nView = 0x%lx, ViewPort = 0x%lx, VPExtra = 0x%lx\nProgInfo = 0x%lx\n", v, vp, vpe, bd->pinfo); WACK;);

    if (bm = ri->BitMap)
    {
	bpu = bm->Depth;
    }

    if ((bd->Flags & BD_IS_DPF) && (ri = ri->Next) && (bm = ri->BitMap))
    {
	bpu += bm->Depth;
    }

    if (bd->Flags & BD_IS_A2024)
    {
	bpu <<= 1;
    }

    bd->bplcon0 |= pinfo->bplcon0 | GBASE->system_bplcon0 | (v->Modes & LACE) | 
                  (((bpu & 7) << 12) | ((bpu & 8) << 1));

    if ((vp->ColorMap) && (vp->ColorMap->Type) && 
		(vp->ColorMap->AuxFlags & CMAF_DUALPF_DISABLE))
	bd->bplcon0 &= 0xfbff;	/* clear dualpf bit */


    /* for bplcon1, remember bd->Scroll is in 35ns values */
    bd->bplcon1 = SCROLL(bd->Scroll) | (SCROLL(((bd->Flags & BD_IS_DPF) ? bd->Scroll2 : bd->Scroll)) << 4);
    bd->bplcon2 = pinfo->bplcon2 | bd->bplcon2;
    bd->bplcon3 |= get_bplcon3(vp->ColorMap);
    bd->bplcon4 = get_bplcon4(vp->ColorMap);

    /* Do we need to turn on ECSENA? */
    if (bd->bplcon3 & (ECSENA_LOCKS))
    {
	bd->bplcon0 |= USE_BPLCON3;
    }

    /* at this point, set the ScanDouble bit in the fmode. bd->FudgedFMode is
     * not used as an index anymore, and this is needed for ScrollVPort().
     */
    if (bd->Flags & BD_IS_SDBL)
    {
	bd->FudgedFMode |= FMF_BSCAN2;	/* bitplane scan double bit */
    }
    if (bd->mspc->ms_Flags & MSF_DOUBLE_SPRITES)
    {
	bd->FudgedFMode |= FMF_SSCAN2;
    }
    /* also, get the sprite fmode. */
    bd->FudgedFMode |= GBASE->SpriteFMode;

    D(kprintf("Leaving MakeAABplcon()\n"); WACK;);

    INC_FNCOUNT;
    return(RETERR);

}

ULONG MakeECSBplcon(DRIVER_PARAMS)
{
#undef SCROLL
#define SCROLL(x) (((x) & 0x003c) >> 2)

    struct BuildData *bd = (struct BuildData *)vpe->DriverData[0];
    struct ProgInfo *pinfo = bd->pinfo;
    struct RasInfo *ri = vp->RasInfo;
    struct BitMap *bm;
    INITERR
    UWORD  bpu = 0;

    D(kprintf("In MakeECSBplcon()\nView = 0x%lx, ViewPort = 0x%lx, VPExtra = 0x%lx\nProgInfo = 0x%lx\n", v, vp, vpe, bd->pinfo); WACK;);

    if (bm = ri->BitMap)
    {
	bpu = bm->Depth;
    }

    if ((bd->Flags & BD_IS_DPF) && (ri = ri->Next) && (bm = ri->BitMap))
    {
	bpu += bm->Depth;
    }

    if (bd->Flags & BD_IS_A2024)
    {
	bpu <<= 1;
    }

    bd->bplcon0 |= pinfo->bplcon0 | GBASE->system_bplcon0 | (v->Modes & LACE) | 
                  (((bpu & 7) << 12));

    if ((vp->ColorMap) && (vp->ColorMap->Type) && 
		(vp->ColorMap->AuxFlags & CMAF_DUALPF_DISABLE))
	bd->bplcon0 &= 0xfbff;	/* clear dualpf bit */

    /* for bplcon1, remember bd->Scroll is in 35ns values */
    bd->bplcon1 = SCROLL(bd->Scroll) | (SCROLL(((bd->Flags & BD_IS_DPF) ? bd->Scroll2 : bd->Scroll)) << 4);
    bd->bplcon2 = ((pinfo->bplcon2 | bd->bplcon2) & (~KILLEHB));
    bd->bplcon3 |= get_bplcon3(vp->ColorMap);

    /* Do we need to turn on ECSENA? */
    if (bd->bplcon3 & (ECSENA_LOCKS))
    {
	bd->bplcon0 |= USE_BPLCON3;
    }

    D(kprintf("Leaving MakeECSBplcon()\n"); WACK;);

    INC_FNCOUNT;
    return(RETERR);

}

/*********************************************************************************/
/*                                                                               */
/* From here on is the code that actually builds the A, ECS and AA copper lists. */
/*                                                                               */
/*********************************************************************************/

#define DPF_OFFSET_PLANE 3
#define DPF_OFFSET_COLOUR (1 << DPF_OFFSET_PLANE)
#define SPR_OFFSET_PLANE 4
#define SPR_OFFSET_COLOUR (1 << SPR_OFFSET_PLANE)

struct CopIns *setplptr(struct CopIns *c, struct CopList **cl, ULONG reg, ULONG value, UWORD diff)
{
    WORD frame = (diff ? CPR_NT_LOF : 0);

    D(kprintf("setpltr(), address = 0x%lx, value = 0x%lx\n", reg, value); WACK;)

    c->OpCode = (frame | COPPER_MOVE); c->DESTADDR = (WORD)reg; c->DESTDATA = (WORD)(value >> 16); CBUMP(cl);
    c->OpCode = (frame | COPPER_MOVE); c->DESTADDR = (WORD)(reg + 2); c->DESTDATA = (WORD)(value & 0xffff); CBUMP(cl);

    if (diff)	/* must be laced */
    {
	D(kprintf("Have a diff of 0x%lx\n", diff); WACK;)
	c->OpCode = (CPR_NT_SHT | COPPER_MOVE); c->DESTADDR = (WORD)reg; c->DESTDATA = (WORD)((value + diff) >> 16); CBUMP(cl);
	c->OpCode = (CPR_NT_SHT | COPPER_MOVE); c->DESTADDR = (WORD)(reg + 2); c->DESTDATA = (WORD)((value + diff) & 0xffff); CBUMP(cl);
	(*cl)->SLRepeat += 2;

    }

    return(c);
}

struct CopIns *DoPlPtrs(struct ViewPort *vp, struct CopIns *c, struct CopList **cl, struct BuildData *bd)
{
    struct RasInfo *ri = vp->RasInfo;
    struct BitMap *bm;
    struct BitMap *bm2 = NULL;
    struct ProgInfo *pinfo = bd->pinfo;
    UWORD diff, diff1;
    UWORD diff2 = 0;
    int depth, totdepth = 0;

    if (bm = ri->BitMap)
    {
	totdepth = bm->Depth;
    }
    if ((bd->Flags & BD_IS_DPF) && (ri->Next) && (bm2 = ri->Next->BitMap))    /* DualPF */
    {
	diff2 = ((bd->Flags & BD_IS_LACE) ? bm2->BytesPerRow : 0);
	totdepth += bm2->Depth;
    }

    if (totdepth)
    {
	/* Aha - some bitplane pointers */
	diff1 = ((bd->Flags & BD_IS_LACE) ? bm->BytesPerRow : 0);
	if (!(bd->Flags & BD_IS_A2024))
	{
		for (depth = 0; depth < totdepth; depth++)
		{
			int plane;
			ULONG addr;
			plane = ((((pinfo->Flags & (PROGINFO_NATIVE | PROGINFO_HAM)) == (PROGINFO_NATIVE | PROGINFO_HAM)) && (bm->Depth >= 7)) ?
				((depth + 6) & 0x7) : 	/* swap the bitplanes */
				depth);
			if (bm2)
			{
				if (depth & 1)
				{
					addr = (ULONG)(bm2->Planes[depth >> 1] + bd->Offset2);
					diff = diff2;
				}
				else
				{
					addr = (ULONG)(bm->Planes[depth >> 1] + bd->Offset);
					diff = diff1;
				}
			}
			else
			{
				addr = (ULONG)(bm->Planes[plane] + bd->Offset);
				diff = diff1;
			}
			c = setplptr(c, cl, REGNUM(bplpt) + (depth<<2), addr, diff);
		}
	}
	else
	{
		for (depth = 0; depth < totdepth; depth++)
		{
			ULONG addr = (ULONG)(bm->Planes[depth] + bd->Offset);

			c = setplptr(c, cl, REGNUM(bplpt) + (depth << 3), addr, 0);
			c = setplptr(c, cl, REGNUM(bplpt) + (((depth << 1) + 1)<<2), (addr + diff1), 0);
		}
	}
    }

    return(c);
}

ULONG BuildColours(DRIVER_PARAMS)
{
    struct BuildData *bd = (struct BuildData *)vpe->DriverData[0];
    struct ColorMap *cm;
    INITERR

    D(kprintf("In BuildColours()\nView = 0x%lx, ViewPort = 0x%lx, VPExtra = 0x%lx\nProgInfo = 0x%lx\n", v, vp, vpe, bd->pinfo); WACK;);

    cm = vp->ColorMap;
    {
	struct CopList *cl;
	if ((cl = vp->DspIns = copinit(vp->DspIns, DSPINS_COUNTA)) && (cl->MaxCount))
	{
		UWORD *c = (UWORD *) cl->CopIns;
		UWORD  *table;
		int    pen;
		UWORD datamask=0x7fff;

		if ((cm) && (cm->Flags & COLORMAP_TRANSPARENCY))
		    datamask=-1;

		table = (cm ? (UWORD *)cm->ColorTable : dflt_clrs);
		bd->firstwait = (struct CopIns *) c;
		D(kprintf("Have CopList at 0x%lx, CopIns at 0x%lx\n", cl, c); WACK;)
		SUPERWAIT(c, 0, 0); SUPERBUMP(&cl); 
		if (table)
		{
			ULONG colormask[8];
			UWORD scrambled_colors[32];
			create_color_bits(vp,colormask);
			if ( (new_mode(vp) & SUPERHIRES) && (! (GBASE->ChipRevBits0 & GFXF_AA_MLISA)))
			{
			    scramble_colors(table,scrambled_colors);
			    scramble_colors(table+16,scrambled_colors+16);
			    table=scrambled_colors;
			}
			for (pen = 0; pen < 32; pen++)
			{
				if (colormask[0] & (1l<<(31-pen)))
				{
					D(kprintf("color=%ld\n",pen););
					*(c++)=COPPER_MOVE; *(c++)=0x180+pen+pen; *(c++)=table[pen] & datamask; SUPERBUMP(&cl);
				}
			}
		}
		bd->c = (struct CopIns *) c;
	}
	else
	{
		ERR(MVP_NO_DSPINS);
	}
    }

    D(kprintf("Leaving BuildColours()\n"); WACK;);

    INC_FNCOUNT;
    return(RETERR);
}

ULONG BuildAAColours(DRIVER_PARAMS)
{

    struct BuildData *bd = (struct BuildData *)vpe->DriverData[0];
    struct ColorMap *cm;
    struct CopList *cl;
    ULONG colormask[8];			/* mask of colors used */
    UWORD cmcount;
    INITERR

    D(kprintf("In BuildAAColours()\nView = 0x%lx, ViewPort = 0x%lx, VPExtra = 0x%lx\nProgInfo = 0x%lx\n", v, vp, vpe, bd->pinfo); WACK;);

    if (cm = vp->ColorMap)
    {
	cmcount = cm->Count;
    }
    else
    {
	cmcount = 32;
    }
    D(kprintf("cmcount = 0x%lx\n", cmcount);)	

    {
	if ((cl = vp->DspIns = copinit(vp->DspIns, (((cmcount << 1) + (((cmcount >> 5) + 1) << 1)) + DSPINS_COUNTAA))) && (cl->MaxCount))
	{
	    UWORD *c = (UWORD *) cl->CopIns;
	    UWORD  *table, *origtableh, *origtablel;
	    int bank, loct, pen;
	    UWORD datamask=0x7fff;

	    if (cm)
	    {
		if (cm->Flags & COLORMAP_TRANSPARENCY) datamask=-1;
		origtableh = (UWORD *)cm->ColorTable;
		origtablel = (UWORD *)((cm->Type == COLORMAP_TYPE_V39) ? cm->LowColorBits : origtableh);
	    }
	    else
	    {
		origtableh = origtablel = dflt_clrs;
	    }
	    D(kprintf("cl = 0x%lx, c = 0x%lx, cl->Count = %ld, cl->MaxCount = %ld\n", cl, c, cl->Count, cl->MaxCount); WACK;)
	    D(kprintf("origtableh = 0x%lx, origtablel = 0x%lx\n", origtableh, origtablel); WACK;)

	    bd->firstwait = (struct CopIns *) c;

	    D(kprintf("bplcon3 = %lx\n",bd->bplcon3););
	    create_color_bits(vp,colormask);	// create bit array of colors used
	    #ifdef DEBUG
	    {
	    	int i;
	    	for (i=0; i<8; i++)
	    	    D(kprintf("cmask[%ld] = 0x%lx\n", i, colormask[i]));
	    }
	    #endif
	    D(kprintf("colormask[0]=%lx\n",colormask[0]););
	    D(kprintf("cl = 0x%lx, c = 0x%lx\n", cl, c);)
	    SUPERWAIT(c, 0, 0); SUPERBUMP(&cl); 
	    for (bank = 0; bank < 8; bank++)
	    {
		if (colormask[bank])
		{
		    D(kprintf("colormask[%ld] = 0x%lx\n", bank, colormask[bank]); WACK;)
		    for (loct = 0; loct < 2; loct++)	/* MSBs, then LSBs of colours */
		    {
			D(kprintf("loct = 0x%lx\n", loct);)
			table = (loct ? origtablel : origtableh);
			SUPERCMOVE(c, bplcon3, (((bank & 0x7) << 13) | (loct << 9)) | bd->bplcon3); SUPERBUMP(&cl);
			for (pen = 0; pen < 32; pen++)
			{
				if (colormask[bank] & (1l<<(31-pen)))
				{
 					*(c++)=COPPER_MOVE; *(c++)=0x180+pen+pen; *(c++)=table[pen+(bank<<5)] & datamask; SUPERBUMP(&cl);
				}
			}
		     }
		}
	    }
	    SUPERCMOVE(c,bplcon4,bd->bplcon4); SUPERBUMP(&cl);
	    bd->c = (struct CopIns *)c;
	} else
	ERR(MVP_NO_DSPINS);
    }
    D(kprintf("Leaving BuildAAColours()\n"); WACK;);
    D(kprintf("cl->Count = %ld\n", cl->Count);)

    INC_FNCOUNT;
    return(RETERR);
}

struct CopIns *BuildStdAACopList(struct CopList **cl, UWORD *c, struct BuildData *bd, struct ProgInfo *pinfo)
{
    D(kprintf("In BuildStdAACopList() - CopList at 0x%lx, CopIns at 0x%lx, bd at 0x%lx, pinfo at 0x%lx\n", *cl, c, bd, pinfo); WACK;)

    /* DIWSTRT/STOP are in ViewPort resolution. The diwstrt/stop registers now
     * work in 35ns pixel resolution with AA, so some conversion is
     * required.
     */

#ifdef ALICE_LHS_WRAPAROUND
    if (GBASE->Bugs & BUG_ALICE_LHS)
    {
	/* Must ensure that the wrap around value is at least 1, as 0 is illegal */
	bd->DiwStopX %= ((bd->mspc->DeniseMaxDisplayColumn - 1) << pinfo->ToViewX);
    }
#endif

    bd->DiwStopX <<= pinfo->ToDIWResn;
    bd->DiwStrtY >>= bd->ToViewY;

    SUPERCMOVE(c, diwstrt, (((bd->DiwStrtY & 0xff) << 8) |
                            (((bd->DiwStrtX <<= pinfo->ToDIWResn) >> 2) & 0xff))); SUPERBUMP(cl);
    SUPERCMOVE(c, bplcon0, bd->bplcon0); SUPERBUMP(cl);
    SUPERCMOVE(c, bplcon2, bd->bplcon2); SUPERBUMP(cl);
    SUPERCMOVE(c, bplcon3, bd->bplcon3); SUPERBUMP(cl);
    *(c++) = (COPPER_MOVE | CPR_NT_SHT); *(c++) = REGNUM(diwstop); *(c++) = bd->RGADiwStopYL; SUPERBUMP(cl);
    *(c++) = (COPPER_MOVE | CPR_NT_LOF); *(c++) = REGNUM(diwstop); *(c++) = bd->RGADiwStopYS; SUPERBUMP(cl);
    (*cl)->SLRepeat++;
    SUPERCMOVE(c, ddfstrt, bd->DDFStrt); SUPERBUMP(cl);
    SUPERCMOVE(c, ddfstop, bd->DDFStop); SUPERBUMP(cl);
    SUPERCMOVE(c, bplcon1, bd->bplcon1); SUPERBUMP(cl);
    SUPERCMOVE(c, bpl1mod, bd->Modulo); SUPERBUMP(cl);
    SUPERCMOVE(c, bpl2mod, bd->Modulo2); SUPERBUMP(cl);

    D(kprintf("Leaving BuildStdAACopList()\n"); WACK;)
    return((struct CopIns *) c);
}

ULONG BuildNmlAACopList(DRIVER_PARAMS)
{

    struct BuildData *bd = (struct BuildData *)vpe->DriverData[0];
    struct ProgInfo *pinfo = bd->pinfo;
    struct CopList *cl;
    UWORD fmode = bd->FudgedFMode;
    UWORD diwhighl, diwhighs;
    UWORD diwstopX;
    WORD ivg;

    INITERR

    D(kprintf("In BuildNmlAACopList()\nView = 0x%lx, ViewPort = 0x%lx, VPExtra = 0x%lx\nProgInfo = 0x%lx\n", v, vp, vpe, bd->pinfo);)
    D(kprintf("bd = 0x%lx CopIns = 0x%lx\n", bd, bd->c); WACK;)

    if (cl = vp->DspIns)    /* should have been set up in BuildAAColours() */
    {
	UWORD *c = (UWORD *) bd->c;

	c = (UWORD *) BuildStdAACopList(&cl, c, bd, pinfo);
	c = (UWORD *) DoPlPtrs(vp, c, &cl, bd);

	diwstopX = ((bd->Flags & BD_VIDEOOSCAN) ? 0 : bd->DiwStopX);

	diwhighl = ((((diwstopX & 0x400) << 3) | (((bd->DiwStrtX & 0x400) >> 5) | ((bd->DiwStrtY & 0x700) >> 8))));
	if (GBASE->ChipRevBits0 & GFXF_AA_ALICE)
	{
		/* For ROM space savings, ECS goes through here too! */

		UWORD mask = ((GBASE->Bugs & BUG_H0) ? 2 : 3);
		diwhighl |= (((bd->DiwStrtX & mask) << 3) | ((bd->DiwStopX & mask) << 11));
	}
	diwhighs = diwhighl;
	diwhighl |= (((bd->DiwStopY + bd->ToViewY) >> bd->ToViewY) & 0x700);
	diwhighs |= ((bd->DiwStopY >>= bd->ToViewY) & 0x700);
	*(c++) = (COPPER_MOVE | CPR_NT_LOF); *(c++) = REGNUM(diwhigh); *(c++)= diwhighl; SUPERBUMP(&cl);
	*(c++) = (COPPER_MOVE | CPR_NT_SHT); *(c++) = REGNUM(diwhigh); *(c++)= diwhighs; SUPERBUMP(&cl);
	cl->SLRepeat++;

	if (GBASE->ChipRevBits0 & GFXF_AA_ALICE)
	{
		/* For ROM space savings, ECS goes through here too! */
		SUPERCMOVE(c, fmode, fmode); SUPERBUMP(&cl);

#ifdef ALICE_RHS_BUG
		/* +++++++++ Fudge Fudge Fudge! ++++++++++++
		 * On the last line of the display, turn off bitplane DMA by 
		 * making BPU in bplcon0 = 0, and set fmode to 0.
		 *
		 * This is only needed for display windows with a 16 or 32 fetch cycle.
		 */

		D(kprintf("cycle = %ld\n", (pinfo->Offset + GBASE->bwshifts[fmode])); WACK;)
		if (bd->RHSFudge)
		{
			SUPERWAIT(c, (vp->DHeight - 1), 0); SUPERBUMP(&cl);
			SUPERCMOVE(c, bplcon0, (bd->bplcon0 & 0x8fef)); SUPERBUMP(&cl);
			SUPERCMOVE(c, fmode, GBASE->SpriteFMode); SUPERBUMP(&cl);
		}
#endif
	}

	SUPERCEND(c); SUPERBUMP(&cl);

        if (bd->firstwait)
        {
		ivg = gfx_CalcIVG(v, vp);
		D(kprintf("IVG = 0x%lx\n", ivg); WACK)
		bd->firstwait->VWAITPOS = -ivg;		/* leave enough space for loading the colours */
        }

	/* Does the IVG straddle line 255?
	 * DiwStrtY must be > 255, && (DiwStrtY - ivg) < 255.
	 */
	if (bd->Flags & BD_IS_SDBL)
	{
		ivg <<= 1;
	}
	else
	{
		if (bd->Flags & BD_IS_LACE)
		{
			ivg >>= 1;
		}
	}
	if (((bd->DiwStrtY & 0xff) && (bd->DiwStrtY > ivg) && ((bd->DiwStrtY & 0xff00) != ((bd->DiwStrtY - ivg) & 0xff00))) ||
	    ((cl->Flags & EXACT_LINE) && (!(bd->DiwStrtY & 0xff))))
	{
		D(kprintf("Straddles. DiwStrtY = 0x%lx, vp = 0x%lx\n", bd->DiwStrtY, vp); WACK;)
		DG(struct Custom *io = &custom; io->color[0] = 0xf00;)
		vpe->Flags |= VPXF_STRADDLES_256;
		if (bd->DiwStrtY >= 512)
		{
			vpe->Flags |= VPXF_STRADDLES_512;
			DG(struct Custom *io = &custom; io->color[0] = 0x0f0;)
		}
	}
	else
	{
		vpe->Flags &= ~(VPXF_STRADDLES_256 | VPXF_STRADDLES_512);
	}
    }
    else
    {
	ERR(MVP_NO_DSPINS);
    }

    D(kprintf("Leaving BuildNmlAACopList()\n"); WACK;);
    D(kprintf("cl->Count = %ld\n", cl->Count);)

    INC_FNCOUNT;
    return(RETERR);
}

ULONG BuildNmlACopList(DRIVER_PARAMS)
{

    struct BuildData *bd = (struct BuildData *)vpe->DriverData[0];
    struct ProgInfo *pinfo = bd->pinfo;
    struct CopList *cl;
    UWORD  ToViewY = bd->ToViewY;
    WORD ivg, enable;

    INITERR

    D(kprintf("In BuildNmlACopList()\nView = 0x%lx, ViewPort = 0x%lx, VPExtra = 0x%lx\nProgInfo = 0x%lx\n", v, vp, vpe, bd->pinfo);)
    D(kprintf("bd = 0x%lx CopIns = 0x%lx\n", bd, bd->c); WACK;)

    if (cl = vp->DspIns)    /* should have been set up in BuildColours() */
    {
	UWORD *c = (UWORD *) bd->c;

	/* DIWSTRT/STOP are in ViewPort resolution. The diwstrt/stop registers now
	 * work in 35ns pixel resolution with AA, so some conversion is
	 * required.
	 */

	bd->DiwStrtX <<= pinfo->ToDIWResn;
//	bd->DiwStopX <<= pinfo->ToDIWResn;
//	bd->DiwStopY >>= ToViewY;	/* not used any more */

	/* clip to the 'A' hardware limits. The display window is restricted to
	 * opening horizontally to the left 3/4 of the display (H8 = 0) and vertically
	 * upper 2/3 of the display (V8 = 0), and closing horizontally to the right
	 * 1/4 (H8 = 1) and vertically to the lower half.
	 */

	bd->DiwStrtX = MIN(bd->DiwStrtX, 0x3ff);

#ifdef UNDEF
	/* Furthermore, on true 'A' machines (those without ECS Denise or Lisa),
	 * for some reason, setting DDFSTRT to 0x20 in lores screens when the 
	 * screen is as far to the left as possible results in about 4 pixels being
	 * lost. This is weird - if I remove the old denise whilst the machine is
	 * running, and replace it with an ECS denise, then the display is fine.
	 * The simplest solution I have found so far is to sniff out this case,
	 * and reduce DDFSTRT by 4, and subtract 2 from the bitplane
	 * offsets and modulos.
	 * Sheesh!
	 */
	if ((bd->DDFStrt == 0x20) && (pinfo->ToViewX == 0))
	{
		/* Note: this also works on ECS and AA in 'A' emulation */
		bd->DDFStrt -= 4;
		bd->Offset -= 2;
		bd->Offset2 -= 2;
		if (bd->DiwStopX >= 0x100)
		{
			bd->DDFStop -= 4;
		}
		else
		{
			bd->Modulo -= 2;
			bd->Modulo2 -= 2;
		}
	}
#endif
	SUPERCMOVE(c, diwstrt, (((bd->mspc->min_row & 0xff) << 8) |
	                        ((bd->DiwStrtX >> 2) & 0xff))); SUPERBUMP(&cl);
	SUPERCMOVE(c, bplcon0, (bd->bplcon0 & 0x0fef)); SUPERBUMP(&cl);
	SUPERCMOVE(c, bplcon2, bd->bplcon2); SUPERBUMP(&cl);
	SUPERCMOVE(c, diwstop, bd->RGADiwStopYL); SUPERBUMP(&cl);
	SUPERCMOVE(c, ddfstrt, bd->DDFStrt); SUPERBUMP(&cl);
	SUPERCMOVE(c, ddfstop, bd->DDFStop); SUPERBUMP(&cl);
	SUPERCMOVE(c, bplcon1, bd->bplcon1); SUPERBUMP(&cl);
	SUPERCMOVE(c, bpl1mod, bd->Modulo); SUPERBUMP(&cl);
	SUPERCMOVE(c, bpl2mod, bd->Modulo2); SUPERBUMP(&cl);

	c = (UWORD *) DoPlPtrs(vp, c, &cl, bd);

	/* We now display the ViewPort by turning on BitPlaneUsage. We always
	 * want to do this at the same Y position (the View->DyOffset).
	 */
	enable = (bd->DiwStrtY - vp->DyOffset - (v->DyOffset << ToViewY));
	/* Turn on... */
	SUPERWAIT(c, enable, 0); SUPERBUMP(&cl);
	SUPERCMOVE(c, bplcon0, bd->bplcon0); SUPERBUMP(&cl);
	/* ... and turn off again, at different points for Long and Short frame */
	{
		UWORD diff;
		diff = ((vp->DHeight & (vp->DyOffset + 1)) & ToViewY);
		D(kprintf("vp->DHeight = 0x%lx, enable = 0x%lx, diff = 0x%lx\n", vp->DHeight, enable, diff);)
		*(c++) = (COPPER_WAIT | CPR_NT_SHT); *(c++) = (vp->DHeight + diff); *(c++) = 0; SUPERBUMP(&cl);
		*(c++) = (COPPER_WAIT | CPR_NT_LOF); *(c++) = (vp->DHeight & ~1); *(c++) = 0; SUPERBUMP(&cl);
		cl->SLRepeat++;
	}
	SUPERCMOVE(c, bplcon0, (bd->bplcon0 & 0x0fef)); SUPERBUMP(&cl);
	/* Calculate InterViewPort gap. */
        if (bd->firstwait)
        {
		ivg = gfx_CalcIVG(v, vp);
		D(kprintf("IVG = 0x%lx\n", ivg); WACK)
		bd->firstwait->VWAITPOS = (enable - ivg);		/* leave enough space for loading the colours */
        }
	SUPERCEND(c); SUPERBUMP(&cl);
    }

    D(kprintf("Leaving BuildNmlACopList\n");)

    INC_FNCOUNT;
    return(RETERR);
}
