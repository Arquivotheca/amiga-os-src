/**************************************************************************
**
** timer.c      - The heartbeat of the nipc.library
**
** Copyright 1992, Commodore-Amiga, Inc.
**
** $Id: timer.c,v 1.11 92/11/02 14:05:11 kcd Exp $
**
***************************************************************************

/*------------------------------------------------------------------------*/

#include <exec/types.h>
#include <devices/timer.h>
#include <exec/io.h>
#include "nipcinternal.h"
#include "externs.h"

#include <clib/exec_protos.h>
#include "nipcinternal_protos.h"

#include <pragmas/exec_pragmas.h>

/*------------------------------------------------------------------------*/

/*
 * this provides the heartbeat to all protocols above IP, as well as to ARP.
 * Currently the timeout is _about_ .1 of a second. (+ the time it takes to
 * queue a new request)
 */

/*------------------------------------------------------------------------*/

BOOL InitTimer()
{
    register struct NBase *nb = gb;

    if(nb->timerport = (struct MsgPort *) CreateMsgPort())
    {
        gb->timersigmask = (1 << gb->timerport->mp_SigBit);
        gb->TimeCount = 10;

        if(nb->treq = (struct timerequest *) CreateIORequest(nb->timerport, sizeof(struct timerequest)))
        {
            if(!OpenDevice("timer.device", UNIT_VBLANK, (struct IORequest *) nb->treq, 0))
            {
                    gb->TimerDevice = (struct Library *) nb->treq->tr_node.io_Device;
                    gb->TimerUnit = nb->treq->tr_node.io_Unit;

                    /* queue the 1st request */

                    gb->treq->tr_node.io_Command = TR_ADDREQUEST;
                    gb->treq->tr_node.io_Flags = 0;
                    gb->treq->tr_time.tv_secs = 0;  /* make it .1 second */
                    gb->treq->tr_time.tv_micro = 100000;
                    SendIO((struct IORequest *) nb->treq);
                    return (TRUE);
            }
            DeleteIORequest((struct IORequest *) nb->treq);
        }
        DeleteMsgPort(nb->timerport);
    }
    return FALSE;
}

/*------------------------------------------------------------------------*/

void TimeOut()
{
    register struct NBase *nb = gb;
    if(nb->treq = (struct timerequest *) GetMsg(nb->timerport))
    {
        gb->treq->tr_time.tv_secs = 0;
        gb->treq->tr_time.tv_micro = 100000;
        SendIO((struct IORequest *) nb->treq);
    }
    return;
}

/*------------------------------------------------------------------------*/

void DeinitTimer()
{
    register struct NBase *nb = gb;
    AbortIO((struct IORequest *) nb->treq);
    WaitIO((struct IORequest *) nb->treq);

    CloseDevice((struct IORequest *) nb->treq);
    DeleteMsgPort(nb->timerport);
    DeleteIORequest((struct IORequest *)nb->treq);

}
/*------------------------------------------------------------------------*/
