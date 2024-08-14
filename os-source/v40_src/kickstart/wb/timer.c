/*
 * $Id: timer.c,v 38.2 93/03/05 12:16:18 mks Exp $
 *
 * $Log:	timer.c,v $
 * Revision 38.2  93/03/05  12:16:18  mks
 * Now uses the timer.device tag
 * 
 * Revision 38.1  91/06/24  11:39:05  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

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
#include <internal/librarytags.h>

#define TIMEDELAY	5	/* seconds */

int initTimer()
{
struct WorkbenchBase *wb = getWbBase();
struct timerequest *tr;
UWORD *ptr;

    MP(("initTimer: enter\n"));
    if (ptr = ALLOCVEC(sizeof(struct timerequest) + sizeof(UWORD), MEMF_CLEAR|MEMF_PUBLIC))
    {
        /* magic stuff for ioloop() so it can easily dispatch it */
        ptr[0] = MTYPE_TIMER;
        tr =  (struct timerequest *)&ptr[1];
        if (OpenDevice(ODTAG_TIMER, UNIT_VBLANK, (struct IORequest *)tr, 0))
        {
            Alert(AN_Workbench | AG_OpenDev | AO_TimerDev | AT_DeadEnd);
        }
        wb->wb_TimerBase = (struct Library *)tr->tr_node.io_Device;
        MP(("\tTimerBase=$%lx\n", wb->wb_TimerBase));
        tr->tr_node.io_Message.mn_ReplyPort = &wb->wb_WorkbenchPort;
        tr->tr_node.io_Command = TR_ADDREQUEST;
        wb->wb_TimerRequest = tr;
        wb->wb_LastFreeMem = -1;
        MP(("\tcalling TimeTick\n"));
        TimeTick(tr);
        MP(("\tcalling InitLayerDemon\n"));
        if (!InitLayerDemon()) ptr=NULL;
    }
    MP(("initTimer: exit\n"));
    return((int)ptr);
}

void uninitTimer()
{
    struct WorkbenchBase *wb = getWbBase();
    struct timerequest *tr = wb->wb_TimerRequest;
    UWORD *ptr;

    if (tr)
    {
        MP(("uninitTimer: enter, calling UnInitLayerDemon\n"));
        UnInitLayerDemon();
        if (!CheckIO((struct IORequest *)tr))
        {
            MP(("\tAbortIO..."));
            AbortIO((struct IORequest *)tr);
            MP(("WaitIO..."));
            WaitIO((struct IORequest *)tr);
            MP(("ok\n"));
        }

        MP(("\tCloseDevice..."));
        CloseDevice((struct IORequest *)tr);
        MP(("ok\n"));

        ptr = ((UWORD *)tr) - 1;
        MP(("\tfreeing %ld bytes @$%lx\n",sizeof(struct timerequest) + sizeof(UWORD), ptr));
        FREEVEC(ptr);
    }
    MP(("uninitTimer: exit\n"));
}

void UpdateMemory(void)
{
struct WorkbenchBase *wb = getWbBase();
int freemem, chipmem, fastmem;

	freemem = AvailMem(0);
	if (freemem != wb->wb_LastFreeMem)
	{
		wb->wb_ErrorDisplayed = 1;
		wb->wb_LastFreeMem = freemem;
		chipmem = AvailMem(MEMF_CHIP);
		fastmem = AvailMem(MEMF_FAST);
		sprintf(wb->wb_ScreenTitle, Quote(Q_SCREEN_TITLE), chipmem, fastmem);
		MessageTitle(wb->wb_ScreenTitle);
	}
}

void TimeTick(tr)
struct timerequest *tr;
{
struct WorkbenchBase *wb = getWbBase();

    MP(("TimeTick: enter, tr=$%lx\n", tr));
    if (!wb->wb_Closed)
    {
        MP(("\tcalling PathTick()\n"));
        PathTick();
	MP(("\tcalling FindDisks()\n"));
	FindDisks();
	if (!wb->wb_ErrorDisplayed) UpdateMemory();
    }

    if (tr)
    {
	tr->tr_time.tv_secs = TIMEDELAY;
	tr->tr_time.tv_micro = 0;
	MP(("tr->tr_time.tv_secs=%ld\n", tr->tr_time.tv_secs));
	MP(("tr->tr_time.tv_micro=%ld\n", tr->tr_time.tv_micro));
	MP(("tr->tr_node.io_Command=$%lx\n", tr->tr_node.io_Command));
	MP(("tr->tr_node.io_Flags=$%lx\n", tr->tr_node.io_Flags));
	MP(("tr->tr_node.io_Error=$%lx\n", tr->tr_node.io_Error));
	MP(("tr->tr_node.io_Message.mn_Node.ln_Type=$%lx\n",tr->tr_node.io_Message.mn_Node.ln_Type));
	DP(("TT: calling SendIO($%lx)...", tr));
	SendIO((struct IORequest *)tr);
	DP(("ok\n"));
	MP(("tr->tr_time.tv_secs=%ld\n", tr->tr_time.tv_secs));
	MP(("tr->tr_time.tv_micro=%ld\n", tr->tr_time.tv_micro));
	MP(("tr->tr_node.io_Command=$%lx\n", tr->tr_node.io_Command));
	MP(("tr->tr_node.io_Flags=$%lx\n", tr->tr_node.io_Flags));
	MP(("tr->tr_node.io_Error=$%lx\n", tr->tr_node.io_Error));
	MP(("tr->tr_node.io_Message.mn_Node.ln_Type=$%lx\n",tr->tr_node.io_Message.mn_Node.ln_Type));
    }
    MP(("TimeTick: exit\n"));
}

/*****************************************************************************/

#define LAYERDEMONDELAY	1	/* in seconds */

void
AbortLayerDemon() /* abort the current layer timer request (if any) */
{
    struct WorkbenchBase *wb = getWbBase();
    struct timerequest *tr = wb->wb_LayerDemonRequest;

    DP(("AbortLayerDemon: enter\n"));
    if (!CheckIO((struct IORequest *)tr))
    {
	DP(("\tAbortIO..."));
	AbortIO((struct IORequest *)tr);
	DP(("WaitIO..."));
	WaitIO((struct IORequest *)tr);
	DP(("ok\n"));
    }
    DP(("AbortLayerDemon: exit\n"));
}

void PostLayerDemon() /* post a request to the layer timer request */
{
    struct WorkbenchBase *wb = getWbBase();
    struct timerequest *tr = wb->wb_LayerDemonRequest;

    DP(("PLD: calling AbortLayerDemon\n"));
    AbortLayerDemon(); /* abort old (if any) */
    tr->tr_time.tv_secs = LAYERDEMONDELAY;
    tr->tr_time.tv_micro = 0;
    DP(("PLD: calling SendIO($%lx)...", tr));
    SendIO((struct IORequest *)tr);
    DP(("ok\n"));
}

int InitLayerDemon() /* initialization for the layer demon */
{
struct WorkbenchBase *wb = getWbBase();
struct timerequest *tr;

    DP(("InitLayerDemon: enter\n"));
    if (tr = (struct timerequest *)ALLOCVEC(sizeof(struct timerequest), MEMF_CLEAR|MEMF_PUBLIC))
    {
        if (OpenDevice(ODTAG_TIMER, UNIT_VBLANK, (struct IORequest *)tr, 0))
        {
            Alert(AN_Workbench | AG_OpenDev | AO_TimerDev | AT_DeadEnd);
            /* ZZZ: need better backout here! */
        }
        tr->tr_node.io_Message.mn_ReplyPort = &wb->wb_LayerPort;
        tr->tr_node.io_Command = TR_ADDREQUEST;

        /* prevents CheckIO from saying that the request is pending when it is not */
        tr->tr_node.io_Message.mn_Node.ln_Type = NT_REPLYMSG;

        wb->wb_LayerDemonRequest = tr;
        DP(("\tcalling PostLayerDemon\n"));
        PostLayerDemon();
    }
    DP(("InitLayerDemon: exit\n"));
    return((int)tr);
}

void UnInitLayerDemon() /* uninitialization for the layer demon */
{
    struct WorkbenchBase *wb = getWbBase();
    struct timerequest *tr = wb->wb_LayerDemonRequest;

    if (tr)
    {
        DP(("UnInitLayerDemon: enter\n"));
        AbortLayerDemon();
        DP(("\tCloseDevice..."));
        CloseDevice((struct IORequest *)tr);
        DP(("\tfreeing %ld bytes @$%lx\n", sizeof(struct timerequest), tr));
        FREEVEC(tr);
    }
    DP(("UnInitLayerDemon: exit\n"));
}

void DoLayerDemon() /* got a layer demon timer request */
{
    struct WorkbenchBase *wb = getWbBase();
    struct IntuiMessage msg;
    DP(("DoLayerDemon: enter\n"));
    /*
	If we EVER get here then the system is presumed to
	be locked up due to me locking layers as a result
	of the user drag-selecting or dragging an icon.
	The solution is to call the same logic that would
	be called if the user cancelled the operation
	(hit the right mouse button).  This will unlock the
	layers and the system should return to normal (sic).

	I play a REAL nasty trick here!  ClearDragging requires
	a ptr to an intuimessage as one of its inputs.  It uses
	this to get the window to operate on (msg->IDCMPWindow).
	However, I do not have a valid intuimessage at this point.
	Necessity being the mother of invention; I invent one
	out of thin air (actually the stack) and initialize the
	one and only field (important point) that ClearDragging
	is going to use (namely the IDCMPWindow field).  I get
	the window from wb_CurrentWindow which just happens to
	be the window that the DragSelect or Dragging started in.
    */
    if (msg.IDCMPWindow = wb->wb_CurrentWindow)
    {
        DP(("\twin=$%lx (%s)\n", wb->wb_CurrentWindow,wb->wb_CurrentWindow ? wb->wb_CurrentWindow->Title : "NULL"));
        ClearDragging(&msg, 0); /* clear dragging/drag selecting */
    }
    DP(("DoLayerDemon: exit\n"));
}
