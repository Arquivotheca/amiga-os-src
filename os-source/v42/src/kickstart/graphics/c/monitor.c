/******************************************************************************
*
*	$Id: monitor.c,v 42.0 93/06/16 11:16:22 chrisg Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/memory.h>
#include <exec/execbase.h>

#include "/macros.h"
#include "/gfxnodes.h"
#include "/monitor.h"
#include "/view.h"
#include "/gfxbase.h"
#include "/d/d.protos"
#include "/gfxpragmas.h"
#include "/displayinfo.h"
#include <clib/utility_protos.h>
#include <pragmas/utility_pragmas.h>

#define NULL_MSPC_FUNCTIONS
#define PROTOTYPE_DISPLAY

#ifdef PROTOTYPE_DISPLAY
#include "/displayinfo_internal.h"
#endif
#include "c.protos"

/*#define USEGBDEBUG*/
/*#define DEBUG*/

#ifdef USEGBDEBUG
#define GBDEBUG if (GBASE->Debug)
#else
#define GBDEBUG
#endif

#ifdef DEBUG
#define D(x) {GBDEBUG {x};}
#else
#define D(x)
#endif

#define STREQ(a,b) (!Stricmp((a),(b)))

/* graphics access routines for monitor specification */


struct MonitorSpec * __asm OpenMonitor(register __a1 char *name, register __d0 id)
{
	struct MonitorSpec *mspc= NULL;
	int open_default = ( ((!name) && (!id)) ||
			     (( name) && (STREQ(name,DEFAULT_MONITOR_NAME))) );

	ObtainSemaphore(GBASE->MonitorListSemaphore);

	if(open_default)
	{
	    mspc = GBASE->default_monitor;
	    D(kprintf("open_monitor: default == 0x%08lx\n", mspc );)
	}
	else
	{
	    if(name)
	    {
		D(kprintf("open_monitor: findname == \"%s\"\n", name );)
		mspc = (struct MonitorSpec *)FindName(&GBASE->MonitorList,name);
	    }
	    else
	    {
		struct RawMonitorInfo querymonitor;
		struct RawMonitorInfo *chunk = &querymonitor;

		if( ((ULONG)(gfx_GetDisplayInfoData(NULL,chunk,sizeof(*chunk),DTAG_MNTR,id)))
		>= ((ULONG)(&((struct RawMonitorInfo *)0)->Mspc)+sizeof(ULONG)))
		{
		    mspc = chunk->Mspc;
		}
	    }
	}

	if(mspc)
	{
	    mspc->ms_OpenCount += 1;
	}

	D(kprintf("open_monitor: mspc   == 0x%08lx\n", mspc  );)

	ReleaseSemaphore(GBASE->MonitorListSemaphore);


	return( mspc );
}


int __asm CloseMonitor(register __a0 struct MonitorSpec *mspc)
{
	int error = FALSE;

	ObtainSemaphore(GBASE->MonitorListSemaphore);

	if (mspc)
	{
	    mspc->ms_OpenCount -= 1;
	}
	else
	{
	    error = TRUE;
	}

	ReleaseSemaphore(GBASE->MonitorListSemaphore);
	
	return( error ); 
}

/* transformation routines for monitor specification */

#ifdef NULL_MSPC_FUNCTIONS

void transform_monitorspec(mspc,src,type,dst)
struct MonitorSpec *mspc;
UWORD type;
Point *src,*dst;
{
    nop_monitorspec(mspc,src,type,dst);
}

void translate_monitorspec(mspc,src,type,dst)
struct MonitorSpec *mspc;
UWORD type;
Point *src,*dst;
{
    nop_monitorspec(mspc,src,type,dst);
}

void nop_monitorspec(mspc,src,type,dst)
struct MonitorSpec *mspc;
UWORD type;
Point *src,*dst;
{
    dst->x = src->x;
    dst->y = src->y;
}

#else

void transform_monitorspec(mspc,src,type,dst)
struct MonitorSpec *mspc;
UWORD type;
Point *src,*dst;
{

    D(kprintf("-------------------------------\n");)
    D(kprintf("transform: mspc == %08lx\n",mspc); )
    D(kprintf("transform: src == %08lx \(%04lx,%04lx\)\n",src,src->x,src->y);)
    D(kprintf("transform: type == %04lx\n",type);)

    translate_monitorspec(mspc,src,TO_MONITOR,dst);
    scale_monitorspec(mspc,dst,type,dst);
    translate_monitorspec(mspc,dst,FROM_MONITOR,dst);

    D(kprintf("transform: dst == %08lx \(%04lx,%04lx\)\n",dst,dst->x,dst->y);)
    D(kprintf("-------------------------------\n");)
}

void translate_monitorspec(mspc,src,type,dst)
struct MonitorSpec *mspc;
UWORD type;
Point *src,*dst;
{
    Point tmp;

    tmp.x = src->x;
    tmp.y = src->y;
	
    if(mspc)
    {
	switch(type)
        {
	    case(TO_MONITOR):
	    {
		tmp.x = src->x - mspc->ms_xoffset;
		tmp.y = src->y - mspc->ms_yoffset;
	    }break;
	    case(FROM_MONITOR):
	    {
		tmp.x = src->x + mspc->ms_xoffset;
		tmp.y = src->y + mspc->ms_yoffset;
	    }break;
	    default: break;
        }
    }

    dst->x = tmp.x;
    dst->y = tmp.y;
}

#endif

void scale_monitorspec(mspc,src,type,dst)
struct MonitorSpec *mspc;
UWORD type;
Point *src,*dst;
{
    Point tmp;

    tmp.x = src->x;
    tmp.y = src->y;
	
    if(mspc)
    {
	switch(type)
        {
	    case(TO_MONITOR):
	    {
		tmp.x = ((src->x * mspc->ratioh) + (RATIO_UNITY >> 1))
			>> RATIO_FIXEDPART;
		tmp.y = ((src->y * mspc->ratiov) + (RATIO_UNITY >> 1))
			>> RATIO_FIXEDPART;
	    }break;
	    case(FROM_MONITOR):
	    {
		tmp.x = ((src->x * mspc->ratiov) + (RATIO_UNITY >> 1))
			>> RATIO_FIXEDPART;
		tmp.y = ((src->y * mspc->ratioh) + (RATIO_UNITY >> 1))
			>> RATIO_FIXEDPART;
	    }break;
	    default: break;
        }
    }

    dst->x = tmp.x;
    dst->y = tmp.y;
}

/* initalisation routine for default monitor specification */
/* assumes monitorspec cleared on entry */

#ifdef  PROTOTYPE_DISPLAY

UWORD init_monitorspec(mspc,type)
struct MonitorSpec *mspc;
UWORD type;
{
    /* set the flags passed in */
    mspc->ms_Flags = type;
    /* but clear existing flags until they are explicitly set */
    mspc->ms_Flags &= ~(MSF_REQUEST_PAL|MSF_REQUEST_NTSC|MSF_REQUEST_SPECIAL|MSF_REQUEST_A2024);

    /* reset use count */
    mspc->ms_OpenCount = -1;

    /* default values */
    mspc->ratioh = RATIO_UNITY;
    mspc->ratiov = RATIO_UNITY;
    mspc->total_rows = STANDARD_NTSC_ROWS;
    mspc->total_colorclocks = STANDARD_COLORCLOCKS;
    mspc->DeniseMaxDisplayColumn = STANDARD_DENISE_MAX;
    mspc->DeniseMinDisplayColumn = STANDARD_DENISE_MIN;
    mspc->BeamCon0 = STANDARD_NTSC_BEAMCON;
    mspc->min_row = MIN_NTSC_ROW;

    /* standard view position */
    mspc->ms_LegalView.MinX = STANDARD_VIEW_X;
    mspc->ms_LegalView.MinY = STANDARD_VIEW_Y;
    mspc->ms_LegalView.MaxX = STANDARD_VIEW_X;
    mspc->ms_LegalView.MaxY = STANDARD_VIEW_Y;

    /* allow default special_ntsc, special_pal requests */
    {
	if(type & MSF_REQUEST_SPECIAL) 
	{
	    struct SpecialMonitor *smp;

	    if(   smp = mspc->ms_Special? 
		  mspc->ms_Special:
	          (struct SpecialMonitor *) gfx_GfxNew(SPECIAL_MONITOR_TYPE) )
	    {

		/* special_broadcast defaults */

		smp->hblank.asi_Start = STANDARD_HBSTRT;
		smp->hblank.asi_Stop = STANDARD_HBSTOP;

		smp->hsync.asi_Start = STANDARD_HSSTRT;
		smp->hsync.asi_Stop = STANDARD_HSSTOP;

		smp->vblank.asi_Start = STANDARD_VBSTRT;
		smp->vblank.asi_Stop = STANDARD_VBSTOP;

		smp->vsync.asi_Start = STANDARD_VSSTRT;
		smp->vsync.asi_Stop = STANDARD_VSSTOP;

		smp->do_monitor = default_monitor;

		mspc->ms_Special = smp;
		mspc->ms_Flags |= MSF_REQUEST_SPECIAL;
		mspc->BeamCon0 |= SPECIAL_BEAMCON;

#ifdef DEBUG
		if(mspc->ms_Node.xln_Library) /* debugging */
		{
		    struct GfxBase *GB = mspc->ms_Node.xln_Library;
		    if(GB->Debug & 0x07)
		    {
			smp->hblank.asi_Start = BROADCAST_HBSTRT;
			smp->hblank.asi_Stop = BROADCAST_HBSTOP;

			smp->hsync.asi_Start = BROADCAST_HSSTRT;
			smp->hsync.asi_Stop = BROADCAST_HSSTOP;

			smp->vblank.asi_Start = BROADCAST_VBSTRT;
			smp->vblank.asi_Stop = BROADCAST_VBSTOP;

			smp->vsync.asi_Start = BROADCAST_VSSTRT;
			smp->vsync.asi_Stop = BROADCAST_VSSTOP;
			mspc->BeamCon0 &= ~SPECIAL_BEAMCON;
			mspc->BeamCon0 |= BROADCAST_BEAMCON;
		    }
		}
#endif
	    }
	    else
	    {
		mspc->ms_Flags &= ~MSF_REQUEST_SPECIAL;
	    }
	}
	else
	{
	    if(mspc->ms_Special)
	    {
	          gfx_GfxFree(mspc->ms_Special);
		  mspc->ms_Special = NULL;
	    }
	}

	if(type & MSF_REQUEST_NTSC)
	{
	    mspc->ms_Flags |= MSF_REQUEST_NTSC;
	}
	else /* can't have both pal and ntsc set */
	if(type & MSF_REQUEST_PAL)
	{
	    mspc->BeamCon0 |= 	STANDARD_PAL_BEAMCON;
	    mspc->total_rows = 	STANDARD_PAL_ROWS;
	    mspc->min_row = 	MIN_PAL_ROW;
	    mspc->ms_Flags |= MSF_REQUEST_PAL;
	} 

	if(type & MSF_REQUEST_A2024)
	{
	    mspc->ms_Flags |= MSF_REQUEST_A2024;
	}
    }

    NewList(&mspc->DisplayInfoDataBase);
    InitSemaphore(&mspc->DisplayInfoDataBaseSemaphore);

    return(mspc->ms_Flags);
}

#else

UWORD init_monitorspec(mspc,type)
struct MonitorSpec *mspc;
UWORD type;
{
    /* clear existing flags */
    mspc->ms_Flags &= ~(MSF_REQUEST_PAL|MSF_REQUEST_NTSC|MSF_REQUEST_SPECIAL);

    /* reset use count */
    mspc->ms_OpenCount = -1;

    /* default values */
    mspc->ratioh = RATIO_UNITY;
    mspc->ratiov = RATIO_UNITY;
    mspc->total_rows = STANDARD_NTSC_ROWS;
    mspc->total_colorclocks = STANDARD_COLORCLOCKS;
    mspc->DeniseMaxDisplayColumn = STANDARD_DENISE_MAX;
    mspc->DeniseMinDisplayColumn = STANDARD_DENISE_MIN;
    mspc->BeamCon0 = STANDARD_NTSC_BEAMCON;
    mspc->min_row = MIN_NTSC_ROW;

    /* standard view position */
    mspc->ms_LegalView.MinX = STANDARD_VIEW_X;
    mspc->ms_LegalView.MinY = STANDARD_VIEW_Y;
    mspc->ms_LegalView.MaxX = STANDARD_VIEW_X;
    mspc->ms_LegalView.MaxY = STANDARD_VIEW_Y;

    /* allow default special_ntsc, special_pal requests */
    {
	if(type & MSF_REQUEST_SPECIAL) 
	{
	    struct SpecialMonitor *smp;

	    if(   smp = mspc->ms_Special? 
		  mspc->ms_Special:
	          (struct SpecialMonitor *) gfx_GfxNew(SPECIAL_MONITOR_TYPE) )
	    {

		/* special_broadcast defaults */

		smp->hblank.asi_Start = STANDARD_HBSTRT;
		smp->hblank.asi_Stop = STANDARD_HBSTOP;

		smp->hsync.asi_Start = STANDARD_HSSTRT;
		smp->hsync.asi_Stop = STANDARD_HSSTOP;

		smp->vblank.asi_Start = STANDARD_VBSTRT;
		smp->vblank.asi_Stop = STANDARD_VBSTOP;

		smp->vsync.asi_Start = STANDARD_VSSTRT;
		smp->vsync.asi_Stop = STANDARD_VSSTOP;

		smp->do_monitor = (int (*()))default_monitor;

		mspc->ms_Special = smp;
		mspc->ms_Flags |= MSF_REQUEST_SPECIAL;
		mspc->BeamCon0 |= SPECIAL_BEAMCON;

#ifdef DEBUG
		if(mspc->ms_Node.xln_Library) /* debugging */
		{
		    struct GfxBase *GB = mspc->ms_Node.xln_Library;
		    if(GB->Debug & 0x07)
		    {
			smp->hblank.asi_Start = BROADCAST_HBSTRT;
			smp->hblank.asi_Stop = BROADCAST_HBSTOP;

			smp->hsync.asi_Start = BROADCAST_HSSTRT;
			smp->hsync.asi_Stop = BROADCAST_HSSTOP;

			smp->vblank.asi_Start = BROADCAST_VBSTRT;
			smp->vblank.asi_Stop = BROADCAST_VBSTOP;

			smp->vsync.asi_Start = BROADCAST_VSSTRT;
			smp->vsync.asi_Stop = BROADCAST_VSSTOP;
			mspc->BeamCon0 &= ~SPECIAL_BEAMCON;
			mspc->BeamCon0 |= BROADCAST_BEAMCON;
		    }
		}
#endif
	    }
	    else
	    {
		mspc->ms_Flags &= ~MSF_REQUEST_SPECIAL;
	    }
	}
	else
	{
	    if(mspc->ms_Special)
	    {
	          gfx_GfxFree(mspc->ms_Special);
		  mspc->ms_Special = NULL;
	    }
	}

	if(type & MSF_REQUEST_NTSC)
	{
	    mspc->ms_Flags |= MSF_REQUEST_NTSC;
	}
	else /* can't have both pal and ntsc set */
	if(type & MSF_REQUEST_PAL)
	{
	    mspc->BeamCon0 |= 	STANDARD_PAL_BEAMCON;
	    mspc->total_rows = 	STANDARD_PAL_ROWS;
	    mspc->min_row = 	MIN_PAL_ROW;
	    mspc->ms_Flags |= MSF_REQUEST_PAL;
	} 
    }

    NewList(&mspc->DisplayInfoDataBase);
    InitSemaphore(&mspc->DisplayInfoDataBaseSemaphore);

    return(mspc->ms_Flags);
}

#endif

/**i*** graphics.library/SetDefaultMonitor***********************************
*
*   NAME
*	SetDefaultMonitor -- to PAL or NTSC or the ScanDbl equivalent (V39)
*
*   SYNOPSIS
*	result = SetDefaultMonitor(MonitorNumber);
*	d0                         d0
*
*	BOOL SetDefaultMonitor(UWORD);
*
*   FUNCTION
*	To set the default monitor to be either PAL, NTSC, DOUBLEPAL or
*	DOUBLENTSC.
*
*   INPUT
*	MonitorNumber - (xxx_MONITOR_ID >> 16)
*
*   RESULT
*	result - TRUE if OK, else FALSE.
*
*   SEE ALSO
*	<graphics/modeid.h>
*
******************************************************************************/

BOOL __asm SetDefaultMonitor(register __d0 UWORD mon)
{
	struct MonitorSpec *mspc = NULL;
	struct MonitorSpec *oldmspc = NULL;
	BOOL waspal = (GBASE->DisplayFlags & PAL);
	BOOL ispal = FALSE;
	BOOL isdbl = FALSE;
	BOOL result = FALSE;
	BOOL change = FALSE;

	D(kprintf("SDM() mon = 0x%lx\n", mon);)

	ObtainSemaphore(GBASE->MonitorListSemaphore);

	/* For DBLNTSC and DBLPAL, the monitors must have already been added
	 * in the normal startup-sequence.
	 */
	switch (mon)
	{
		case (DBLNTSC_MONITOR_ID >> 16) :
		{
			isdbl = TRUE;
			mspc = gfx_OpenMonitor(NULL, DBLNTSCLORES_KEY);
		}
		break;
		case (DBLPAL_MONITOR_ID >> 16) :
		{
			isdbl = TRUE;
			ispal = TRUE;
			mspc = gfx_OpenMonitor(NULL, DBLPALLORES_KEY);
		}
		break;
		case (NTSC_MONITOR_ID >> 16) :
		{
			if ((oldmspc = mspc = gfx_OpenMonitor(NULL, (NTSC_MONITOR_ID | LORES_KEY))) == NULL)
			{
				D(kprintf("Could not open NTSC monitor\n");)
				mspc = new_monitorspec(GBASE, MSF_REQUEST_NTSC, NTSC_MONITOR_NAME);
			}
			GBASE->natural_monitor = mspc;
		}
		break;
		case (PAL_MONITOR_ID >> 16) :
		{
			if ((oldmspc = mspc = gfx_OpenMonitor(NULL, (PAL_MONITOR_ID | LORES_KEY))) == NULL)
			{
				D(kprintf("Could not open PAL monitor\n");)
				mspc = new_monitorspec(GBASE, MSF_REQUEST_PAL, PAL_MONITOR_NAME);
			}
			ispal = TRUE;
			GBASE->natural_monitor = mspc;
		}
		default : break;
	}

	if ((mspc) && (!(isdbl && (GBASE->system_bplcon0 & GENLOCK_VIDEO))))
	{
		int unity_scale();

		D(kprintf("have mspec = 0x%lx\n", mspc);)
		mspc->ms_scale = unity_scale; 	/* force unity */

		GBASE->default_monitor = mspc;

		/* Now set up the GBASE variables. */
		if (ispal)
		{
			GBASE->DisplayFlags &= ~NTSC;
			GBASE->DisplayFlags |= PAL;
//			GBASE->MaxDisplayRow = (STANDARD_PAL_ROWS - 1);
			GBASE->NormalDisplayRows = 256;
			GBASE->monitor_id = mon;
//			if (!(isdbl))
			{
				GBASE->NormalDPMX = 1226;	/* 320 X 1000/261mm */
				GBASE->NormalDPMY = 1299;	/* 200 X 1000/197mm */
			}
			((struct ExecBase *)SysBase)->VBlankFrequency = 50;
			if ((!waspal) || (isdbl))
			{
				change = TRUE;
			}
		}
		else
		{
			GBASE->DisplayFlags &= ~PAL;
			GBASE->DisplayFlags |= NTSC;
//			GBASE->MaxDisplayRow = (STANDARD_NTSC_ROWS - 1);
			GBASE->NormalDisplayRows = 200;
			GBASE->monitor_id = mon;
//			if (!(isdbl))
			{
				GBASE->NormalDPMX = 1280;	/* 320 X 1000/250mm */
				GBASE->NormalDPMY = 1098;	/* 200 X 1000/182mm */
			}
			((struct ExecBase *)GBASE->ExecBase)->VBlankFrequency = 60;
			if ((waspal) || (isdbl))
			{
				change = TRUE;
			}
		}

		if ((!(isdbl)) && (oldmspc == NULL))
		{
			/* initialize view range bound */
			vlimit(mspc,&mspc->ms_LegalView,NULL);
			AddHead(&GBASE->MonitorList, mspc);
			activate_mspc((ispal ? (PAL_MONITOR_ID) : (NTSC_MONITOR_ID)), mspc);
		}
		result = TRUE;
        }
	if (change)
	{
		if (!(isdbl))
		{
			/* Go through the database, marking the new default as available. */
			struct DisplayInfo disp;
			ULONG ID = INVALID_ID;

			/* Cause the monitor to be unpacked */
			gfx_FindDisplayInfo((mon << 16) | 0x1000);

			while ((ID = gfx_NextDisplayInfo(ID)) != INVALID_ID)
			{
				ULONG monid = (ID & MONITOR_ID_MASK);

				if ( (((monid == NTSC_MONITOR_ID) && (waspal)) ||
				      ((monid == PAL_MONITOR_ID) && (!(waspal)))) &&
				      (gfx_GetDisplayInfoData(NULL, &disp, sizeof(struct DisplayInfo), DTAG_DISP, ID)) )
				{
					disp.NotAvailable &= ~(DI_AVAIL_NOMONITOR);
					gfx_SetDisplayInfoData(NULL, &disp, sizeof(struct DisplayInfo), DTAG_DISP, ID);
				}
			}
		}
		GBASE->monitor_id = mon;
	}

	ReleaseSemaphore(GBASE->MonitorListSemaphore);

	return(result);
}
