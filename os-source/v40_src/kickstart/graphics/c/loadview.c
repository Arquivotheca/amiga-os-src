/******************************************************************************
*
*	$Id: loadview.c,v 39.27 93/05/17 12:56:45 spence Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/interrupts.h> 
#include <exec/libraries.h>
#include "/monitor.h"
#include "/gfx.h"
#include "/gfxbase.h"
#include "/copper.h"
#include "/view.h"
#include "/display.h"
#include "/vp_internal.h"
#include "gfxprivate.h"
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include "c.protos"
#include "/macros.h"
#include "/gfxpragmas.h"

#define ZD_BP

/*#define DEBUG*/

#ifdef DEBUG
#define D(x) {x};
#else
#define D(x)
#endif


extern struct Custom custom;

void __asm KillOldMonitor(register __a0 struct MonitorSpec *mspc)
{
    void (* __asm killview)(register __a0 struct MonitorSpec *mspc);

    if ((mspc) && (killview = mspc->ms_KillView))
    {
	(*killview)(mspc);
    }
}

void default_monitor(mspc)
struct MonitorSpec *mspc;
{
    struct Custom *io;

    io = &custom;

    /* if we get here we must have indirected through ms_Special.do_monitor */
    {
	int hcenter;

	io->htotal = mspc->total_colorclocks;

    D(kprintf("default_monitor: io->htotal == 0x%08lx\n",mspc->total_colorclocks);)

	io->hbstrt = mspc->ms_Special->hblank.asi_Start;
	io->hbstop = mspc->ms_Special->hblank.asi_Stop;
	io->hsstrt = mspc->ms_Special->hsync.asi_Start;
	io->hsstop = mspc->ms_Special->hsync.asi_Stop;

    D(kprintf("default_monitor: io->hbstrt == 0x%08lx\n",
	     mspc->ms_Special->hblank.asi_Start);)
    D(kprintf("default_monitor: io->hsstrt == 0x%08lx\n",
	     mspc->ms_Special->hsync.asi_Start);)
    D(kprintf("default_monitor: io->hsstop == 0x%08lx\n",
	     mspc->ms_Special->hsync.asi_Stop);)
    D(kprintf("default_monitor: io->hbstop == 0x%08lx\n",
	     mspc->ms_Special->hblank.asi_Stop);)

	hcenter = ( mspc->total_colorclocks >> 1) +
		    mspc->ms_Special->hsync.asi_Start;

	io->hcenter = hcenter;

    D(kprintf("default_monitor: io->hcenter == 0x%08lx\n",hcenter);)
    D(kprintf("default_monitor: io->vtotal == 0x%08lx\n",mspc->total_rows);)

	io->vsstrt = mspc->ms_Special->vsync.asi_Start/mspc->total_colorclocks;
	io->vsstop = mspc->ms_Special->vsync.asi_Stop/mspc->total_colorclocks;

	/*
	 * Calculate vbstrt and vbstop, and store them in GfxBase.
	 */
#ifdef VBLANK_BUG
	/* VBlanks are already disabled */
	if (GBASE->Bugs & BUG_VBLANK)
	{
		GBASE->MonitorVBlank.asi_Start = mspc->total_rows;
		GBASE->MonitorVBlank.asi_Stop = (mspc->min_row - 1);
	}
	else
#endif
	{
		GBASE->MonitorVBlank.asi_Start = (mspc->ms_Special->vblank.asi_Start / mspc->total_colorclocks);
		GBASE->MonitorVBlank.asi_Stop = (mspc->ms_Special->vblank.asi_Stop / mspc->total_colorclocks);
	}

    D(kprintf("default_monitor: io->vbstrt == 0x%08lx\n",
	     mspc->ms_Special->vblank.asi_Start/mspc->total_colorclocks);)
    D(kprintf("default_monitor: io->vsstrt == 0x%08lx\n",
	     mspc->ms_Special->vsync.asi_Start/mspc->total_colorclocks);)
    D(kprintf("default_monitor: io->vsstop == 0x%08lx\n",
	     mspc->ms_Special->vsync.asi_Stop/mspc->total_colorclocks);)
    D(kprintf("default_monitor: io->vbstop == 0x%08lx\n",
	     mspc->ms_Special->vblank.asi_Stop/mspc->total_colorclocks);)

    }

    D(kprintf("default_monitor: beamcon0 == 0x%08lx\n",mspc->BeamCon0);)

    newdefras(GBASE,mspc->ratioh,mspc->ratiov);
}

void loadview(d)
struct View *d;
{
    typedef volatile struct Custom vcustom;
    vcustom *io;
    struct MonitorSpec *oldmspc = GBASE->current_monitor;
    BOOL vbstrt = FALSE;

    io = &custom;

    D(kprintf("loadview: view == 0x%08lx. oldmspc = 0x%lx\n",d, oldmspc);)

    /* single thread access to ActiView hardware copper list */

    D(kprintf("Getting the Semaphore\n");)

    ObtainSemaphore(GBASE->ActiViewCprSemaphore);

    if( (d) && ( (GBASE->Modes&INTERLACE) && (!(d->Modes&INTERLACE))) )
    {
	/* Changing from Lace to non-lace */
	vbstrt = TRUE;
    }
    GBASE->SpecialCounter++;
    io->intena = BITCLR|INTF_VERTB;

    if (GBASE->ActiView = d)
    {
	struct	ViewExtra *ve = NULL;
	struct	MonitorSpec *mspc = NULL;
	int	tot_rows = 0;
	int	tot_cclks = 0;
	UWORD	old_dma;
	UWORD	topline = TOPLINE;
	char	mode_change = FALSE;

	D(kprintf("ActiView and View are the same\n");)

	if (d->LOFCprList == 0)
	{
		D(kprintf("LOFCprList = NULL\n");)
		goto no_cpr_list;
	}

	GBASE->Modes = d->Modes;
	GBASE->LOFlist = d->LOFCprList->start;

	mspc = GBASE->natural_monitor;
	D(kprintf("natural_monitor = 0x%lx\n", mspc);)

	if (d->Modes & EXTEND_VSTRUCT)
	{
	    if((ve = (struct ViewExtra *) gfx_GfxLookUp(d)) && ve->Monitor)
	    {
		mspc = ve->Monitor;
		D(kprintf("Use ve->mspc = 0x%lx, ve = 0x%lx\n", mspc, ve);)
	    }
	    if ((GBASE->ChipRevBits0 & GFXF_AA_LISA) && ve)
	    {
		topline = ve->TopLine;
	    }
	}

	D(kprintf("topline = 0x%lx, GBASE->TopLine = 0x%lx\n", topline, GBASE->TopLine);)
	if (topline != GBASE->TopLine)
	{
		GBASE->TopLine = topline;
		GBASE->copinit->wait14[0] = (COPPER_WAIT | topline);	/* risky */
		if (GBASE->copinit->norm_hblank[0] & COPPER_WAIT)
		{
			GBASE->copinit->norm_hblank[0] = (COPPER_WAIT | topline);
		}
	}

	if (d->SHFCprList)
	{
		/* LPEN bit is set in bplcon0, swap the long frame and short
		 * frame.
		 */
		if ((GBASE->system_bplcon0 & 0x8) &&
		    (GBASE->DisplayFlags & LPEN_SWAP_FRAMES))
		{
			GBASE->SHFlist = GBASE->LOFlist;
			GBASE->LOFlist = d->SHFCprList->start;
		}
		else
		{
			GBASE->SHFlist = d->SHFCprList->start;
		}
	}

	if(!(mspc == oldmspc))
	{
	    mode_change = TRUE; /* force change */
	}

	GBASE->current_monitor = mspc; /* update current */
	update_top_color(mspc);     /* update topmost */

	D(kprintf("loadview: mspc == 0x%08lx\n",mspc);)
	D(kprintf("mspc->ms_Flags = 0x%lx\n", mspc->ms_Flags);)
	if (mspc->ms_Flags)
	{
		/* found new 1.3 stuff */
		tot_rows = mspc->total_rows;
		tot_cclks = mspc->total_colorclocks;
	}
	else	
	{
	    io->beamcon0 = beamconpal(GBASE);	/* going to old mode */
	    D(kprintf("old mode: beamcon0 == %04lx\n",beamconpal(GBASE));)
	}

	old_dma = io->dmaconr;

	/* changing modes? */

	if ( ( tot_rows != GBASE->current_tot_rows) ||
	     (tot_cclks != GBASE->current_tot_cclks)  )
	{
		LONG tmp;
		/* turn off all dma that may blow up */

		io->dmacon = DMAF_COPPER|DMAF_SPRITE;

		/* ensure DDFSTOP will never screw us.
		 * Wait for the line count to change, the write legal values
		 * into the ddfstrt/stop registers, wait for another line, so
		 * we know the datafetch completed, and then turn of raster
		 * DMA.
		 *
		 * At this point, VBLANK interrupts are disable, so we
		 * cannot get preempted out, although we could get hit by
		 * a higher priority interrupt.
		 */
		tmp = gfx_VBeamPos();
		while (gfx_VBeamPos() == tmp);
		io->ddfstrt = 0x38;
		io->ddfstop = 0x58;
		tmp = gfx_VBeamPos();
		while (gfx_VBeamPos() == tmp);
		io->dmacon = DMAF_RASTER;

		mode_change = TRUE;

//		io->beamcon0 = beamconpal(GBASE);
		D(kprintf("mode_change: beamcon0 == %04lx\n",beamconpal(GBASE));)
	}

	if(mode_change)
	{
	    D(kprintf("Mode_change\n");)
	    if ( ( tot_rows || tot_cclks ) && (d->LOFCprList) )
	    {
		if(!(mspc->ms_Flags & REQUEST_SPECIAL))
		{
		    D(kprintf("not special: beamcon0 == %04lx\n",mspc->BeamCon0);)
		    D(kprintf("@beamcon0 == %04lx\n",&mspc->BeamCon0);)
		    io->beamcon0 = mspc->BeamCon0;
		}
		else
		{
		    /* call special monitor routine */
		    if((mspc->ms_Special) && (mspc->ms_Special->do_monitor))
		    {
			(*mspc->ms_Special->do_monitor)(mspc); /* indirect */
			vbstrt = TRUE;
		    }
		    else 
		    { 
			io->beamcon0 = 0x0000; /* stub error hardcode */ 
		    }
		}
		GBASE->MaxDisplayRow = mspc->total_rows - 1; /* for vbasm */
		GBASE->MaxDisplayColumn = mspc->DeniseMaxDisplayColumn; 
		GBASE->MinDisplayColumn = mspc->DeniseMinDisplayColumn;
	    }
	    else
	    {
		GBASE->MaxDisplayRow = ( ( GBASE->DisplayFlags & PAL ) ?
		                        STANDARD_PAL_ROWS :
		                        STANDARD_NTSC_ROWS )
		                        - 1; /* for vbasm */
		GBASE->MaxDisplayColumn = STANDARD_DENISE_MAX; /* bart */
		GBASE->MaxDisplayColumn = STANDARD_DENISE_MIN; /* bart */
	    }
	    /* Do any special kill code for the old monitor */
	    KillOldMonitor(oldmspc);
	}
	GBASE->current_tot_rows = tot_rows;
	GBASE->current_tot_cclks = tot_cclks;
	GBASE->MonitorFlags = mspc->ms_Flags;
	if (mode_change) io->dmacon = old_dma | DMAF_SETCLR;
    }
    else
    {
no_cpr_list:

	D(kprintf("no_cpr_list\n");)
	KillOldMonitor(GBASE->current_monitor);
	GBASE->MonitorFlags = NULL;
	GBASE->current_monitor = NULL; /* no view, so no monitor either */
	GBASE->hedley_flags = 0;
	GBASE->LOFlist = GBASE->copinit->wait_forever;
	GBASE->Modes = 0;
	defras(GBASE);
	if (! d)
		GBASE->copinit->fm0[1]=0;	/* turn off sprite scan doubling and anything else */
    }

    if (vbstrt)
    {
	D(kprintf("vbstrt\n");)
	GBASE->Bugs |= CHANGE;	/* vblank should change the beamcon0 */
	io->vbstrt = 0;
    }
    io->intena = BITSET|INTF_VERTB;

    ReleaseSemaphore(GBASE->ActiViewCprSemaphore);
    D(kprintf("Released the Semaphore\n");)
    D(kprintf("ByeBye loadview()\n");)

}

