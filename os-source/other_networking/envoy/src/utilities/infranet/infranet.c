/***********************************************************************
 *                                                                     *
 *                            COPYRIGHTS                               *
 *                                                                     *
 *   Copyright (c) 1991  Commodore-Amiga, Inc.  All Rights Reserved.   *
 *                                                                     *
 ***********************************************************************
 *
 * $Id: InfraNet.c,v 1.1 91/08/18 14:12:04 ewout Exp Locker: ewout $
 *
 * InfraNet.c - INet Remote Control connectionless server.
 * Pure code if pragmas are used.
 * Friday, 16-Aug-91 21:18:21, Ewout
 *
 * Compiled with SAS/C 5.10a:
 * lc -cfis -v -d0 -b0 InfraNet.c
 * blink from InfraNet.o to InfraNet
 *
 */
#include <exec/memory.h>
#include <dos/dosextens.h>
#include <dos/rdargs.h>
#include <dos/dosasl.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <appn/nipc.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/nipc_protos.h>

/* undef PRAGMAS if you don't have them */
#define PRAGMAS
#ifdef PRAGMAS
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/nipc_pragmas.h>
#else
struct ExecBase *SysBase;
struct DosLibrary *DOSBase;
#endif
struct Library *NIPCBase;

static UBYTE *VersTag = "\0$VER: InfraNet 37.1 (16.08.91) Copyright © 1991 Ewout Walraven";

LONG main(VOID);

LONG main(VOID)
{
#ifdef PRAGMAS
    struct DosLibrary *DOSBase;
#endif
    struct MsgPort *msgport;
    struct IOStdReq *iostdreq;
    struct Entity *entity;
    struct Transaction *event_trans;
    ULONG event, netevent,mysig;
    LONG error = 0, rc = 0;

#ifndef PRAGMAS
    /* set up SysBase */
    SysBase = (*((struct Library **) 4));
#endif

    if (DOSBase = (struct DosLibrary *) OpenLibrary("dos.library", 33))
    {
	if (DOSBase->dl_lib.lib_Version > 36)
	{
	    if (NIPCBase = OpenLibrary("nipc.library", 0))
	    {
	    	if (entity = CreateEntity(ENT_Name,"RemoteControl",ENT_Public,TRUE,ENT_AllocSignal,&mysig))
	        {
		    /*
		     * Set return code to FAIL just in case. The
		     * only way	out is by breaking now
		     * (RETURN_WARN)
		     */
		    rc = RETURN_FAIL;
		    /*
		     * Make sure we're ready before we go on the
		     * air
		     */
		    if (msgport	= CreateMsgPort())
		    {
			if (iostdreq = CreateIORequest(msgport,	sizeof(struct IOStdReq)))
			{
			    if (!(OpenDevice("input.device", 0L, (struct IORequest *) iostdreq,	0L)))
			    {
				/* initialize last of iorequest	*/
				iostdreq->io_Command = IND_WRITEEVENT;
				iostdreq->io_Length = sizeof(struct InputEvent);

				netevent = 1L << mysig;
				SetTaskPri(FindTask(NULL), 19);
				for (;;)
				{
				    event = Wait(netevent | SIGBREAKF_CTRL_C);
				    if (event &	netevent)
				    {
					while(event_trans=GetTransaction(entity))
					{
					    iostdreq->io_Data =	(APTR) event_trans->trans_RequestData;
					    DoIO(iostdreq);
					    ReplyTransaction(event_trans);
					}
					SetSignal(0L, netevent);
				    }
				    if (event &	SIGBREAKF_CTRL_C)
				    {
					error =	ERROR_BREAK;
					break;
				    }
				}
				CloseDevice(iostdreq);
			    }
			    else
				PutStr("Can't open input.device");
			    DeleteIORequest(iostdreq);
			}
			else
			    PutStr("Can't create I/O Request\n");
			DeleteMsgPort(msgport);
		    }
		    else
			PutStr("Can't create message port\n");
		    DeleteEntity(entity);
		}
		else
		{
		    PutStr("Can't Set up Entity\n");
		    rc = RETURN_FAIL;
		}
		CloseLibrary(NIPCBase);
	    }
	    else
	    {
		PutStr("Can't open socket.library\n");
		rc = RETURN_FAIL;
	    }

	    SetIoErr(error);
	    if (error)
	    {
		PrintFault(error, NULL);
		if (error = ERROR_BREAK)
		    rc = RETURN_WARN;
		else
		    error = RETURN_FAIL;
	    }
	}
	else
	{
	    PutStr("Kickstart 2.0 required\n");
	    rc = RETURN_FAIL;
	}
	CloseLibrary((struct Library *) DOSBase);
    }
    return (rc);
}
