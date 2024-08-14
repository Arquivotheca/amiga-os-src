/******************************************************************************
*
*	$Id: waitbovp.c,v 42.0 93/06/16 11:15:49 chrisg Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/libraries.h>
#include <exec/interrupts.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <graphics/monitor.h>
#include "/macros.h"
#include "c.protos"

/* steel hard coded BLITTERSIGNAL */
#define TOF_SIGNAL	4
#define DOWNCODE

#ifndef DOWNCODE /* { */
waittof()
{
	struct Node node;

	node.ln_Name = (char *)FindTask(0);	/* cheat */
	node.ln_Type = 0;	/* Wait TOF type node */
	Disable();
	SetSignal(0,1<<TOF_SIGNAL);
	AddTail(&GBASE->TOF_WaitQ,&node);
	Wait(1<<TOF_SIGNAL);		/* Wait for vertical blank to */
	Remove(&node);
	Enable();				/* broadcast signal */
}
#endif /* } DOWNCODE waittof */

/* bart - 10.03.86 - added a useful function */

#ifndef DOWNCODE
#ifdef UNDEFINED
waitbovp(viewport)
struct ViewPort *viewport;
{
    int target;
    struct View *view;
    struct ViewExtra *ve = 0;
    struct MonitorSpec *mspc =0;

    if(view = GBASE->ActiView)
    {
	mspc = GBASE->default_monitor;

	if (view->Modes & EXTEND_VSTRUCT)
	{
	    if((ve = (struct ViewExtra *) GfxLookUp(view)) && ve->Monitor)
	    {
		mspc = ve->Monitor;
	    }
	}

	do
	{
	    target = ((mspc->ms_Flags & REQUEST_A2024)? 0x2c : view->DyOffset)
		     + viewport->DyOffset + viewport->DHeight;
	    {
		short localmax;
		if (mspc)
		{
		    localmax = mspc->total_rows-1;
		}
		else
		{
		    localmax = GBASE->MaxDisplayRow;
		}
		if (target > localmax) target = localmax;
	    } 
	} while ( target >= vbeampos()) ;
    }
}
#endif

waittovp(viewport,offset)
struct ViewPort *viewport;
SHORT offset;
{
    int target;
    struct View *view;
    struct ViewExtra *ve = 0;
    struct MonitorSpec *mspc =0;


    if(view = GBASE->ActiView)
    {

	mspc = GBASE->default_monitor;

	if (view->Modes & EXTEND_VSTRUCT)
	{
	    if((ve = (struct ViewExtra *) GfxLookUp(view)) && ve->Monitor)
	    {
		mspc = ve->Monitor;
	    }
	}

	if (offset > viewport->DHeight) offset = viewport->DHeight;

	do
	{
	    target = ((mspc->ms_Flags & REQUEST_A2024)? 0x2c : view->DyOffset)
		     + viewport->DyOffset + offset;
	    {
		short localmax;
		if (mspc)
		{
		    localmax = mspc->total_rows-1;
		}
		else
		{
		    localmax = GBASE->MaxDisplayRow;
		}
		if (target > localmax) target = localmax;
	    }
	} while( target >= vbeampos()) ;
    }
}
#endif

