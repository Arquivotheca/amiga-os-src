#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/nodes.h"
#include "exec/lists.h"
#include "exec/ports.h"
#include "exec/alerts.h"
#include "exec/io.h"
#include "exec/memory.h"
#include "devices/timer.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "global.h"
#include "support.h"
#include "quotes.h"
#include "wbalerts.h"

#define TIMEDELAY	5	/* seconds */

int initTimer ()
{
    struct WorkbenchBase *wb = getWbBase ();
    struct timerequest *tr;
    UWORD *ptr;

    if (!(ptr = myallocmem (sizeof (struct timerequest) + sizeof (UWORD), MEMF_CLEAR)))
    {
	FatalNoMem (AN_WBinitTimer);
    }

    /* magic stuff for ioloop() so it can easily dispatch it */
    ptr[0] = MTYPE_TIMER;
    tr = (struct timerequest *) & ptr[1];

    if (OpenDevice (TIMERNAME, UNIT_VBLANK, (struct IORequest *) tr, 0) || tr->tr_node.io_Error)
    {
	Alert (AN_Workbench | AG_OpenDev | AO_TimerDev | AT_DeadEnd);
	exit (1);
    }

    wb->wb_TimerBase = (struct Library *) tr->tr_node.io_Device;
    tr->tr_node.io_Message.mn_ReplyPort = &wb->wb_WorkbenchPort;
    tr->tr_node.io_Command = TR_ADDREQUEST;
    wb->wb_TimerRequest = tr;
    wb->wb_LastFreeMem = -1;
    TimeTick (tr);
    InitLayerDemon ();
    return (1);
}

void uninitTimer ()
{
    struct WorkbenchBase *wb = getWbBase ();
    struct timerequest *tr = wb->wb_TimerRequest;
    UWORD *ptr;

    UnInitLayerDemon ();
    if (!CheckIO ((struct IORequest *) tr))
    {
	AbortIO ((struct IORequest *) tr);
	WaitIO ((struct IORequest *) tr);
    }

    CloseDevice ((struct IORequest *) tr);

    ptr = ((UWORD *) tr) - 1;
    myfreemem (ptr, sizeof (struct timerequest) + sizeof (UWORD));
}

void TimeTick (tr)
    struct timerequest *tr;
{
    extern UWORD VERSION, REVISION;
    struct WorkbenchBase *wb = getWbBase ();
    int freemem, chipmem, fastmem;

    if (!wb->wb_Closed)
    {
	FindDisks ();
	if (!wb->wb_ErrorDisplayed)
	{
	    freemem = AvailMem (0);
	    if (freemem != wb->wb_LastFreeMem)
	    {
		wb->wb_ErrorDisplayed = 1;
		wb->wb_LastFreeMem = freemem;
		chipmem = AvailMem (MEMF_CHIP);
		fastmem = AvailMem (MEMF_FAST);
		ErrorTitle (wb->wb_ScreenTitle);
	    }
	}
    }
    if (tr)
    {
	tr->tr_time.tv_secs = TIMEDELAY;
	tr->tr_time.tv_micro = 0;
	SendIO ((struct IORequest *) tr);
    }
}

/*****************************************************************************/

#define LAYERDEMONDELAY	1	/* in seconds */

void
 AbortLayerDemon ()		/* abort the current layer timer request (if
				 * any) */
{
    struct WorkbenchBase *wb = getWbBase ();
    struct timerequest *tr = wb->wb_LayerDemonRequest;

    if (!CheckIO ((struct IORequest *) tr))
    {
	AbortIO ((struct IORequest *) tr);
	WaitIO ((struct IORequest *) tr);
    }
}

void
 PostLayerDemon ()		/* post a request to the layer timer request */
{
    struct WorkbenchBase *wb = getWbBase ();
    struct timerequest *tr = wb->wb_LayerDemonRequest;

    AbortLayerDemon ();		/* abort old (if any) */
    tr->tr_time.tv_secs = LAYERDEMONDELAY;
    tr->tr_time.tv_micro = 0;
    SendIO ((struct IORequest *) tr);
}

void
 InitLayerDemon ()		/* initialization for the layer demon */
{
    struct WorkbenchBase *wb = getWbBase ();
    struct timerequest *tr;

    if (!(tr = (struct timerequest *) myallocmem (sizeof (struct timerequest), MEMF_CLEAR)))
    {
	FatalNoMem (AN_WBInitLayerDemon);	/* ZZZ: need better backout
						 * here! */
    }
    if (OpenDevice (TIMERNAME, UNIT_VBLANK, (struct IORequest *) tr, 0) || tr->tr_node.io_Error)
    {
	Alert (AN_Workbench | AG_OpenDev | AO_TimerDev | AT_DeadEnd);
	exit (1);		/* ZZZ: need better backout here! */
    }
    tr->tr_node.io_Message.mn_ReplyPort = &wb->wb_LayerPort;
    tr->tr_node.io_Command = TR_ADDREQUEST;

    /* prevents CheckIO from saying that the request is pending when it is not */
    tr->tr_node.io_Message.mn_Node.ln_Type = NT_REPLYMSG;

    wb->wb_LayerDemonRequest = tr;
    PostLayerDemon ();
}

void
 UnInitLayerDemon ()		/* uninitialization for the layer demon */
{
    struct WorkbenchBase *wb = getWbBase ();
    struct timerequest *tr = wb->wb_LayerDemonRequest;

    AbortLayerDemon ();
    CloseDevice ((struct IORequest *) tr);
    myfreemem (tr, sizeof (struct timerequest));
}

void
 DoLayerDemon ()		/* got a layer demon timer request */
{
    struct WorkbenchBase *wb = getWbBase ();
    struct IntuiMessage msg;

    /*
     * If we EVER get here then the system is presumed to be locked up due to
     * me locking layers as a result of the user drag-selecting or dragging an
     * icon. The solution is to call the same logic that would be called if the
     * user cancelled the operation (hit the right mouse button).  This will
     * unlock the layers and the system should return to normal (sic).
     *
     * I play a REAL nasty trick here!  ClearDragging requires a ptr to an
     * intuimessage as one of its inputs.  It uses this to get the window to
     * operate on (msg->IDCMPWindow). However, I do not have a valid
     * intuimessage at this point. Necessity being the mother of invention; I
     * invent one out of thing air (actually the stack) and initialize the one
     * and only field (important point) that ClearDragging is going to use
     * (namely the IDCMPWindow field).  I get the window from wb_CurrentWindow
     * which just happens to be the window that the DragSelect or Dragging
     * started in.
     */
    msg.IDCMPWindow = wb->wb_CurrentWindow;
    ClearDragging (&msg, 0);	/* clear dragging/drag selecting */
}
