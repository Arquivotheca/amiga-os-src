/*
 * SADAPI -- Simple function calls to maintain the client side
 *           of a SAD session.
 * $Id: sadapi.c,v 1.8 93/11/05 15:01:40 jesup Exp $
 *
 */

/* #define SAD_PARALLEL to get the parallel.device version we're using
 * internally to debug prototype hardware without a working serial port.
 */

#ifdef SAD_PARALLEL
#define IOExtStruct IOExtPar
#define IOExt IOPar
#else
#define IOExtStruct IOExtSer
#define IOExt IOSer
#endif

#include        <exec/types.h>
#include        <exec/memory.h>
#include        <exec/devices.h>

#include        <dos/dos.h>
#include        <dos/dostags.h>

#include        <devices/timer.h>
#include        <devices/serial.h>
#include        <devices/parallel.h>

#include        <clib/timer_protos.h>
#include        <pragmas/timer_pragmas.h>
#include        <clib/exec_protos.h>
#include        <pragmas/exec_pragmas.h>
#include        <clib/alib_protos.h>
#include        <clib/dos_protos.h>
#include        <pragmas/dos_pragmas.h>

#include        "sadapi_proto.h"

extern struct Task *WackTask;

/*
 * Global structure that contains all of our valuable variables
 *
 */

struct SADAPIVars
{
    struct Library      *ExecBase;      /* Library and device */
    struct Library      *DBase;         /* bases ... */
    struct Device       *SerialBase;
    struct Unit         *SerialUnit;
    struct Device       *TBase;
    struct Unit         *TimerUnit;
    struct MsgPort      *GPPort;        /* Our ReplyPort ... */
    struct Process      *KeepAliveTask; /* Ptr to Task structure of 2nd process */
    struct MsgPort      *KeepAlivePort; /* MsgPort to send 'die' message to for 2nd process */
    struct MsgPort      *KeepAlivePort2;/* MsgPort for 2nd process IO reply port */
    struct timeval       LastAction;    /* time of last packet sent */
    UBYTE                Flags;         /* Defined below */
    UBYTE                LastAck;       /* Last ACK byte received */
};

#define SADAPIB_V40     0               /* If set, the server is a V40 or greater SAD, */
#define SADAPIF_V40     1               /* with working READWORDs and WRITEBYTEs */

/* message used to pass global ptr to second process */
struct PassMessage
{
    struct Message       PMessage;
    struct SADAPIVars   *PGlobVars;
};

/* Externals ... */
extern struct Library *AbsExecBase;

/* Redirect pragmas through our global struct */
#define DOSBase   (g->DBase)
#define TimerBase (g->TBase);

/* Temporary define until we have the global data area set up ... */
#define SysBase AbsExecBase

/* Max time that the 2nd process should wait before nudging SAD
 * with a NOP to insist that we're still alive.  In seconds.
 */
#define MAXWAITTIME     8

/* Max time that it should wait for SAD to respond to a command packet */
#define ACKTIMEOUT      2

/* Max time allowed between bytes read in return data */
#define DATATIMEOUT     2


void
SADSetVersion( struct SADAPIVars *g )
{
#ifdef SAD_PARALLEL
    g->Flags |= SADAPIF_V40;
    /* Our caller, pingSAD, expects this function to return with
     * a prompt ready, since we immediately call SAD_ObtainValidMem().
     * This should generate a prompt.
     */
    SADNOP( g );
#else
    /* To figure out what version we're running, send a command
     * which turns out to be:
     * READ LONG 0xF80000 (V39)
     * READ WORD 0xF80000 (V40)
     */
    UBYTE cmd[6]={0xaf, 0x05, 0x00, 0xf8, 0x00, 0x00};
    ULONG result;

    g->Flags &= ~SADAPIF_V40;
    if ( SADCmd(g,(void *)&cmd,6,TRUE,TRUE,2,&result,4,NULL,0) )
    {
	if ( g->LastAck == 0x05 )
	{
	    g->Flags |= SADAPIF_V40;
	}
    }
#endif
}

/****** SADAPI/SADInit *****************************************************
*
*   NAME
*     SADInit -- Set up devices, processes, etc., needed to use SADAPI funcs
*
*   SYNOPSIS
*     globvar = SADInit(devicename, bpsrate, unitnumber)
*
*     ULONG SADInit(STRPTR, ULONG, ULONG);
*
*   FUNCTION
*     Opens the serial.device, sets the device parameters, opens the
*     timer device, allocates a global data area, and starts a secondary
*     process used to keep the target SAD from timing out.  This must be
*     called before any other SADAPI functions are used.
*
*   INPUTS
*     devicename --    String name of serial.device workalike
*                      (or, obviously, "serial.device")
*     bpsrate    --    The speed at which you want communication to take
*                      place.  (Almost certainly 9600.)
*     unitnumber --    Unit number of above device to use for all
*                      communication
*
*   RESULT
*     globvar --       A 'handle' that defines the specific SAD session in
*                      use.  Must be passed to all subsequent SADAPI calls.
*                      NULL if SADInit() failed.
*
*   EXAMPLE
*
*   NOTES
*     The SADInit() call and all subsequent SADAPI function calls must be
*     made on the same context, as a Message Port with an allocated signal
*     bit ties use to the context where the SADInit() was done.
*
*   BUGS
*
*   SEE ALSO
*       SADDeinit()
*
******************************************************************************
*
*/

struct SADAPIVars *SADInit(STRPTR devicename, ULONG bpsrate, ULONG unitnumber)
{
    register struct SADAPIVars *g;  /* global pointer to our struct */

    g = (struct SADAPIVars *) AllocMem(sizeof(struct SADAPIVars),0);
    if (g)
    {
        struct IOExtStruct *s;

        /* Make sure the flags variable is cleared */
        g->Flags = 0;

        /* Set up our global idea of SysBase; a later #define changes SAS's
         * notions of this ...
         */
        g->ExecBase = AbsExecBase;
        g->DBase = OpenLibrary("dos.library",37L);
        if (g->DBase)
        {
            /* Create our General Purpose Reply Port ... */
            g->GPPort = CreateMsgPort();
            if (g->GPPort)
            {
                /* Try to get serial.device open */
                s = (struct IOExtStruct *) CreateIORequest(g->GPPort,sizeof(struct IOExtStruct));
                if (s)
                {
                    /* Set up IORequest's replyport */
                    s->IOExt.io_Message.mn_ReplyPort = g->GPPort;
#ifdef SAD_PARALLEL
                    s->io_ParFlags = PARF_SHARED;
#endif

                    /* ... And try the open ... */
                    if (!OpenDevice(devicename,unitnumber,(struct IORequest *)s,0))
                    {
                        struct timerequest *t;
                        /* Save necessary fields of the serial open ... */
                        g->SerialBase = s->IOExt.io_Device;
                        g->SerialUnit = s->IOExt.io_Unit;

#ifndef SAD_PARALLEL
                        /* Force the device to parameters that we need ... */
                        s->io_Baud = bpsrate;
                        s->io_ExtFlags = 0;
                        s->io_RBufLen = 512;
                        s->io_BrkTime = 250000;
                        s->io_ReadLen = 8;
                        s->io_WriteLen = 8;
                        s->io_StopBits = 1;
                        s->io_SerFlags = SERF_XDISABLED;
                        s->IOExt.io_Command = SDCMD_SETPARAMS;
                        DoIO((struct IORequest *)s);

                        /* If that succeeded ... */
                        if (!s->IOExt.io_Error)
#endif
                        {
                            /* Now, build a timerrequest, and try to get timer.device open */
                            t = (struct timerequest *) CreateIORequest(g->GPPort,sizeof(struct timerequest));
                            if (t)
                            {
                                /* Set up IORequest's replyport */
                                t->tr_node.io_Message.mn_ReplyPort = g->GPPort;
                                if (!OpenDevice("timer.device",UNIT_VBLANK,(struct IORequest *)t,0))
                                {
                                    ULONG nptags[8]={NP_Entry,0,NP_Name,0,NP_Priority,4,TAG_DONE,0};

                                    /* Save the necessary fields of the timer open ... */
                                    g->TBase = t->tr_node.io_Device;
                                    g->TimerUnit = t->tr_node.io_Unit;

                                    /* Start up the other process */
                                    nptags[1]=(ULONG)&NewProcTask;
                                    nptags[3]=(ULONG)"SAD Client KeepAlive";

                                    g->KeepAliveTask = CreateNewProc((struct TagItem *)&nptags);
                                    if (g->KeepAliveTask)
                                    {
                                        struct PassMessage p;
                                        p.PGlobVars = g;
                                        p.PMessage.mn_ReplyPort = g->GPPort;
                                        PutMsg(&g->KeepAliveTask->pr_MsgPort,&p.PMessage);
                                        WaitPort(g->GPPort);
                                        GetMsg(g->GPPort);

                                        /* Check to see if the other process started up okay .. */
                                        if (p.PGlobVars)    /* Check result */
                                        {
                                            /* Delete iorequests lying around ... */
                                            DeleteIORequest((struct IORequest *)t);
                                            DeleteIORequest((struct IORequest *)s);
                                            return(g);
                                        }
                                    }
                                    CloseDevice((struct IORequest *)t);
                                }
                                DeleteIORequest((struct IORequest *)t);
                            }
                        }
                        CloseDevice((struct IORequest *) s);
                    }
                    DeleteIORequest((struct IORequest *) s);
                }
                DeleteMsgPort(g->GPPort);
            }
            CloseLibrary(g->DBase);
        }
        FreeMem(g,sizeof(struct SADAPIVars));
    }
    return((struct SADAPIVars *) 0L);
}


/****** SADAPI/SADDeinit ***********************************************
*
*   NAME
*     SADDeinit -- Deallocate all resources allocated at SADInit()
*
*   SYNOPSIS
*     SADDeinit(globvar)
*
*     SADDeinit(struct SADAPIVars *);
*
*   FUNCTION
*     Deinitializes the SADAPI routines; closes devices, stops the
*     secondary process, and returns all allocated resources.
*
*   INPUTS
*     globvar --    Global variable cookie from SADInit()
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*     SADInit()
*
******************************************************************************
*
*/


void SADDeinit(struct SADAPIVars *g)
{

    /* structs are allocated off the stack,
     * rather than from CreateIORequest(), because in a
     * low-memory situation, if CreateIORequest() fails,
     * we're stuck.  This way, we can get the devices closed,
     * and free things up for the next Expunge ...
     */
    struct timerequest t;
    struct IOExtStruct s;
    struct Message m;

    /* Shut down other process */
    m.mn_ReplyPort = g->GPPort;
    PutMsg(g->KeepAlivePort,&m);
    WaitPort(g->GPPort);
    GetMsg(g->GPPort);

    /* Close timer.device */
    t.tr_node.io_Device = g->TBase;
    t.tr_node.io_Unit = g->TimerUnit;
    t.tr_node.io_Message.mn_ReplyPort = g->GPPort;
    CloseDevice((struct IORequest *) &t);

    /* Close serial.device */
    s.IOExt.io_Device = g->SerialBase;
    s.IOExt.io_Unit = g->SerialUnit;
    s.IOExt.io_Message.mn_ReplyPort = g->GPPort;
    CloseDevice((struct IORequest *) &s);

    /* Delete the General-Purpose Msg Port */
    DeleteMsgPort(g->GPPort);

    /* Close the dos.library */
    CloseLibrary(g->DBase);

    /* Free the global variable area */
    FreeMem(g,sizeof(struct SADAPIVars));

}

/*
 * NewProcTask() -- Entry point for secondary process.
 *
 * The purpose of this process is simple -- to prevent the situation of
 * more than MAXWAITTIME elapsing without any form of communication occuring
 * with the SAD host.  The code attempts to keep track of how much time has
 * elapsed between the last command issued and the SAD server timing out.
 *
 * This process is created by SADInit(), and removed by SADDeinit().
 *
 */

void NewProcTask(void)
{
    struct SADAPIVars *g;       /* global data area ptr */
    struct Process *proc;       /* Our process */
    struct MsgPort *p;          /* Our process MsgPort */
    struct PassMessage *m;      /* Message from parent process */

    proc = (struct Process *) FindTask(0L); /* Find our process */
    p = &proc->pr_MsgPort;      /* Find our process MsgPort */

    while (!(m = (struct PassMessage *) GetMsg(p))) /* Get the startup message */
        WaitPort(p);

    g = m->PGlobVars;           /* Find the global pointer */

/* Now, we'll cause Enforcer no further problems . . .*/
#undef SysBase
#define SysBase (g->ExecBase)

    g->KeepAlivePort = CreateMsgPort(); /* Make a port to send killoff msgs to */
    if (g->KeepAlivePort)
    {
        g->KeepAlivePort2 = CreateMsgPort(); /* Make a reply port for IO */
        if (g->KeepAlivePort2)
        {
            struct IOExtStruct *s;
            struct timerequest *t;

            s = (struct IOExtStruct *) CreateIORequest(g->KeepAlivePort2,sizeof(struct IOExtStruct));
            if (s)
            {
                t = (struct timerequest *) CreateIORequest(g->KeepAlivePort2,sizeof(struct timerequest));
                if (t)
                {
                    ULONG waitbits; /* Mask used by Wait() below ... */

                    /* In case we WaitIO() on this, and it's never used,
                     * make WaitIO() understand that it's already completed.
                     */
                    t->tr_node.io_Message.mn_Node.ln_Type = NT_REPLYMSG;

                    /* Announce successful startup! */
                    m->PGlobVars = (struct SADAPIVars *) TRUE;
                    ReplyMsg((struct Message *)m);

                    /* Set up serial io request, so we don't do it
                     * -every- time ...
                     */
                    s->IOExt.io_Device = g->SerialBase;
                    s->IOExt.io_Unit = g->SerialUnit;
                    s->IOExt.io_Message.mn_ReplyPort = g->KeepAlivePort2;

                    /* Queue up first time request */
                    t->tr_time.tv_secs = MAXWAITTIME;
                    t->tr_time.tv_micro = 0;
                    t->tr_node.io_Device = g->TBase;
                    t->tr_node.io_Unit = g->TimerUnit;
                    t->tr_node.io_Message.mn_ReplyPort = g->KeepAlivePort2;
                    t->tr_node.io_Command = TR_ADDREQUEST;
                    SendIO((struct IORequest *)t);

                    /* prebuild wait mask */
                    waitbits = (1 << g->KeepAlivePort->mp_SigBit) |
                               (1 << g->KeepAlivePort2->mp_SigBit);

                    /* Main 2nd process loop */
                    while (TRUE)
                    {
                        struct Message *k;  /* Kill off message */
                        k = GetMsg(g->KeepAlivePort);
                        if (k)
                        {
                            /* If told to exit, clean up and leave */
                            AbortIO((struct IORequest *)t); /* stop any pending timer request */
                            WaitIO((struct IORequest *)t);
                            DeleteIORequest((struct IORequest *)t);
                            DeleteIORequest((struct IORequest *)s);
                            DeleteMsgPort(g->KeepAlivePort2);
                            DeleteMsgPort(g->KeepAlivePort);
                            Forbid();
                            ReplyMsg(k);
                            return;
                        }

                        /* If anything's on the port2, it must be our >lone<
                         * timerequest, as all serial is DoIO()'d.
                         */
                        while (GetMsg(g->KeepAlivePort2))
                        {
                            struct timeval tv;
                            /* Find out how much time has passed since the
                             * last action ...
                             */
                            GetSysTime(&tv);
                            SubTime(&tv,&g->LastAction);

#ifndef SAD_PARALLEL
                            /* Decide whether to send a NOP or not ... */
                            if ((!tv.tv_secs) || (tv.tv_secs > MAXWAITTIME))
                            {
                                /* Yes, send one ... */
                                UBYTE nopdata[2]={0xaf,0x00};
                                s->IOExt.io_Command = CMD_WRITE;
                                s->IOExt.io_Data = &nopdata;
                                s->IOExt.io_Length = 2;
                                DoIO((struct IORequest *)s);
                                /* If it failed, it failed.  Nothing we can do */
                            }
#endif

                            /* Queue up a new timer request */
                            if (tv.tv_secs < MAXWAITTIME)
                                t->tr_time.tv_secs = MAXWAITTIME - tv.tv_secs;
                            else
                                t->tr_time.tv_secs = MAXWAITTIME;

                            SendIO((struct IORequest *)t);
                        }

                        /* Wait for something to happen ... */
                        Wait(waitbits);
                    }
                    DeleteIORequest((struct IORequest *)t);
                }
                DeleteIORequest((struct IORequest *)s);
            }
            DeleteMsgPort(g->KeepAlivePort2);
        }
        DeleteMsgPort(g->KeepAlivePort);
    }
    m->PGlobVars = FALSE;

    Forbid();                  /* So we're gone by the time they */
    ReplyMsg((struct Message *)m);               /* Get the reply ... */
}



/*
 * SADCmd()
 *
 * Provides the necessary functionality to perform all SAD commands.
 * Accepts various parameters, issues the given command, waits
 * (if told to) for an ACK, waits (if told to) or times out for the
 * command complettion, and stores (if necessary) the appropriate
 * return data from the command.
 * OutputPtr/Length is used if there's data to output _after_ getting the
 * command ack and before the Done ack.
 *
 */
BOOL SADCmd(struct SADAPIVars *g, void  *CommandString, ULONG CommandLength,
            BOOL WaitForAck, BOOL WaitForDone, UWORD TimeoutForDone,
            void *ResultPtr, ULONG ResultLength,
	    void *OutputPtr, ULONG OutputLength)
{

    struct IOExtStruct *s;    /* Used for sending serial data */
    struct IOExtStruct *sr;   /* Used for receiving serial data */
    struct timerequest *t; /* Used for issuing timeouts */
    UBYTE ReceiveByte[1];  /* Single byte read from serial */
    struct MsgPort *myport;

    if ( FindTask(NULL) == WackTask )
    {
        myport = g->GPPort;
    }
    else
    {
        myport = CreateMsgPort();
    }
    if ( myport )
    {
        /* Allocate a timer request for possible timeout use ... */
        t = (struct timerequest *) CreateIORequest(myport,sizeof(struct timerequest));
        if (t)
        {
            sr = (struct IOExtStruct *) CreateIORequest(myport,sizeof(struct IOExtStruct));
            if (sr)
            {
                s = (struct IOExtStruct *) CreateIORequest(myport,sizeof(struct IOExtStruct));
                if (s)
                {
                    /* First and foremost, issue their command ... */
                    /* While we're at it, init the receive request's fields too */
                    sr->IOExt.io_Device = s->IOExt.io_Device = g->SerialBase;
                    sr->IOExt.io_Unit = s->IOExt.io_Unit = g->SerialUnit;
                    sr->IOExt.io_Data = &ReceiveByte;
                    sr->IOExt.io_Length = 1;
                    sr->IOExt.io_Command = CMD_READ;
                    /* So we can AbortIO() it with impunity ... */
                    sr->IOExt.io_Message.mn_Node.ln_Type = NT_REPLYMSG;
                    s->IOExt.io_Command = CMD_WRITE;
                    s->IOExt.io_Length = CommandLength;
                    s->IOExt.io_Data = (APTR) CommandString;
                    /* flag the last time we sent a command ... */
                    GetSysTime(&g->LastAction);
                    /* And send the cmd ... */
                    /* If over V40, bump the CMD # up by one */
                    if (g->Flags & SADAPIF_V40)
                        ((UBYTE *)CommandString)[1]++;
                    DoIO((struct IORequest *)s);

                    /* Make sure command was correctly sent ... */
                    if (!s->IOExt.io_Error)
                    {
                        /* If necessary, wait for the remote SAD to ack our packet */
                        if (WaitForAck)
                        {
                            BOOL state=FALSE; /* state for while() loop below; true if leading 00 is read */
                            BOOL success=FALSE;

                            SendIO((struct IORequest *)sr);

                            t->tr_time.tv_secs = ACKTIMEOUT;
                            t->tr_node.io_Command = TR_ADDREQUEST;
                            t->tr_node.io_Device = g->TBase;
                            t->tr_node.io_Unit = g->TimerUnit;
                            SendIO((struct IORequest *)t);
                            while (TRUE)
                            {
                                struct IORequest *i;
                                i = (struct IORequest *) GetMsg(myport);
                                if (i)
                                {
                                    /* Returned IORequest.  Is it timer or serial? */
                                    if (i->io_Device == g->SerialBase)
                                    {
                                        /* serial */
                                        if (ReceiveByte[0] == 0x00)
                                            state = TRUE;
                                        else
                                        {
                                            if (state)
                                            {
						/* If we received the 2nd byte, the command character, we have an ack ..
						 * Handle V39-style and V40-style responses
						 */
						if ( (ReceiveByte[0] == ((UBYTE *)CommandString)[1]) ||
						    (ReceiveByte[0] == ((UBYTE *)CommandString)[1]+1))
						{
						    success = TRUE;
						    g->LastAck = ReceiveByte[0];
						}
                                            }
                                        }
                                        if (success)
                                            break;

					/* send request for next byte */
                                        sr->IOExt.io_Data = &ReceiveByte[0];
                                        sr->IOExt.io_Length = 1;
                                        SendIO((struct IORequest *)sr);
                                    }
                                    else
                                    {
                                        /* timer */
                                        break;
                                    }
                                }
                                WaitPort(myport);
                            }
                            /* clean up ... */
                            AbortIO((struct IORequest *)t);
                            WaitIO((struct IORequest *)t);
                            /* If the ACK didn't come in time ... */
                            if (!success)
                            {
                                AbortIO((struct IORequest *)sr);
                                WaitIO((struct IORequest *)sr);
                                DeleteIORequest((struct IORequest *)s);
                                DeleteIORequest((struct IORequest *)sr);
                                DeleteIORequest((struct IORequest *)t);
                                if ( FindTask(NULL) != WackTask )
                                {
                                    DeleteMsgPort(myport);
                                }
                                return(FALSE);
                            }
                        }

			if (OutputPtr && OutputLength > 0)
			{
			    /* data to output after command ack */
			    /* mainly for WriteArray */

	                    s->IOExt.io_Command = CMD_WRITE;
	                    s->IOExt.io_Length = OutputLength;
	                    s->IOExt.io_Data = (APTR) OutputPtr;

	                    /* And send the cmd ... */
	                    DoIO((struct IORequest *)s);

	                    /* Make sure data was correctly sent ... */
	                    if (s->IOExt.io_Error)
	                    {
				goto failure;
			    }
			}

                        /* If necessary, wait for the remote SAD to finish our packet */
                        if (WaitForDone)
                        {
                            BOOL state=FALSE; /* state for while() loop below; true if leading 1F is read */
                            BOOL success=FALSE;

			    /* send request for next byte */
                            sr->IOExt.io_Data = &ReceiveByte[0];
                            sr->IOExt.io_Length = 1;
                            SendIO((struct IORequest *)sr);

                            t->tr_time.tv_secs = TimeoutForDone;
                            t->tr_node.io_Command = TR_ADDREQUEST;
                            t->tr_node.io_Device = g->TBase;
                            t->tr_node.io_Unit = g->TimerUnit;
                            SendIO((struct IORequest *)t);
                            while (TRUE)
                            {
                                struct IORequest *i;
                                i = (struct IORequest *) GetMsg(myport);
                                if (i)
                                {
                                    /* Returned IORequest.  Is it timer or serial? */
                                    if (i->io_Device == g->SerialBase)
                                    {
                                        /* serial */
                                        if (ReceiveByte[0] == 0x1F)
                                            state = TRUE;
                                        else
                                        {
                                            if (state)
                                            {
						/* If we received the 2nd byte, the command character, we have an ack ..
						 * Handle V39-style and V40-style responses
						 */
						if ( (ReceiveByte[0] == ((UBYTE *)CommandString)[1]) ||
						    (ReceiveByte[0] == ((UBYTE *)CommandString)[1]+1))
						{
						    success = TRUE;
						}
                                            }
                                        }
                                        if (success)
                                            break;
					/* send request for next byte */
                                        sr->IOExt.io_Data = &ReceiveByte[0];
                                        sr->IOExt.io_Length = 1;
                                        SendIO((struct IORequest *)sr);
                                    }
                                    else
                                    {
                                        /* timer */
                                        break;
                                    }
                                }
                                WaitPort(myport);
                            }
                            /* clean up ... */
                            AbortIO((struct IORequest *)t);
                            WaitIO((struct IORequest *)t);
                            /* If the Done didn't come in time ... */
                            if (!success)
                            {
                                AbortIO((struct IORequest *)sr);
                                WaitIO((struct IORequest *)sr);
                                DeleteIORequest((struct IORequest *)s);
                                DeleteIORequest((struct IORequest *)sr);
                                DeleteIORequest((struct IORequest *)t);
                                if ( FindTask(NULL) != WackTask )
                                {
                                    DeleteMsgPort(myport);
                                }
                                return(FALSE);
                            }
                        }

                        /* If we were expecting data in response, fetch it */
                        /* If we're still waiting for data, and none has arrived in
                         * DATATIMEOUT time, consider this action failed.
                         */
                        if (ResultLength)
                        {
                            ULONG Remaining=ResultLength;
                            UBYTE *StoreResult=(UBYTE *)ResultPtr;
                            BOOL success=FALSE;

			    /* send request for next byte */
                            sr->IOExt.io_Data = &ReceiveByte[0];
                            sr->IOExt.io_Length = 1;
                            SendIO((struct IORequest *)sr);

                            t->tr_time.tv_secs = DATATIMEOUT;
                            t->tr_node.io_Command = TR_ADDREQUEST;
                            t->tr_node.io_Device = g->TBase;
                            t->tr_node.io_Unit = g->TimerUnit;
                            SendIO((struct IORequest *)t);
                            while (TRUE)
                            {
                                struct IORequest *i;
                                i = (struct IORequest *) GetMsg(myport);
                                if (i)
                                {
                                    /* Returned IORequest.  Is it timer or serial? */
                                    if (i->io_Device == g->SerialBase)
                                    {
                                        /* serial */
                                        if (Remaining)
                                        {
                                            *StoreResult=ReceiveByte[0];
                                            StoreResult++;
                                            Remaining--;
                                            if (!Remaining)
                                            {
                                                success = TRUE;
                                                break;
                                            }
                                            /* ^^ Note, this should be modified to read in as-many-characters-as-are-ready */
                                            AbortIO((struct IORequest *)t);
                                            WaitIO((struct IORequest *)t);
                                            t->tr_time.tv_secs = DATATIMEOUT;
                                            SendIO((struct IORequest *)t);
                                        }
                                        sr->IOExt.io_Data = &ReceiveByte[0];
                                        sr->IOExt.io_Length = 1;
                                        SendIO((struct IORequest *)sr);
                                    }
                                    else
                                    {
                                        /* timer */
                                        break;
                                    }
                                }
                                WaitPort(myport);
                            }
                            /* clean up ... */
                            AbortIO((struct IORequest *)t);
                            WaitIO((struct IORequest *)t);
                            /* If the data didn't come in time ... */
                            if (!success)
                            {
                                AbortIO((struct IORequest *)sr);
                                WaitIO((struct IORequest *)sr);
                                DeleteIORequest((struct IORequest *)s);
                                DeleteIORequest((struct IORequest *)sr);
                                DeleteIORequest((struct IORequest *)t);
                                if ( FindTask(NULL) != WackTask )
                                {
                                    DeleteMsgPort(myport);
                                }
                                return(FALSE);
                            }
                        }

                        /* success!  Free up IORequests, exit */
                        DeleteIORequest((struct IORequest *)s);
                        DeleteIORequest((struct IORequest *)sr);
                        DeleteIORequest((struct IORequest *)t);
                        if ( FindTask(NULL) != WackTask )
                        {
                            DeleteMsgPort(myport);
                        }
                        return(TRUE);
                    }
failure:
                    DeleteIORequest((struct IORequest *)s);
                }
                DeleteIORequest((struct IORequest *)sr);
            }
            DeleteIORequest((struct IORequest *)t);
        }
        if ( FindTask(NULL) != WackTask )
        {
            DeleteMsgPort(myport);
        }
    }
    return(FALSE);
}

ULONG SADSendDelete(struct SADAPIVars *g, ULONG TimeToWait)
{
    struct IOExtStruct *s;
    struct timerequest *t;
    ULONG retval=0;
    struct MsgPort *myport;

    if ( FindTask(NULL) == WackTask )
    {
        myport = g->GPPort;
    }
    else
    {
        myport = CreateMsgPort();
    }
    if ( myport )
    {
        s = (struct IOExtStruct *) CreateIORequest(myport,sizeof(struct IOExtStruct));
        if (s)
        {
            t = (struct timerequest *) CreateIORequest(myport,sizeof(struct timerequest));
            if (t)
            {
                UBYTE delbyte = 0x7F;

                retval = FALSE;
                s->IOExt.io_Length = 1;
                s->IOExt.io_Device = g->SerialBase;
                s->IOExt.io_Unit = g->SerialUnit;
                s->IOExt.io_Data = &delbyte;
                s->IOExt.io_Command = CMD_WRITE;
                SendIO((struct IORequest *)s);

                t->tr_node.io_Device = g->TBase;
                t->tr_node.io_Unit = g->TimerUnit;
                t->tr_node.io_Command = TR_ADDREQUEST;
                t->tr_time.tv_secs = TimeToWait;
                SendIO((struct IORequest *)t);

                while (TRUE)
                {
                    struct IORequest *i;
                    i = (struct IORequest *) GetMsg(myport);
                    if (i)
                    {
                        /* Check to see if it's a returned serial or timer request */
                        if (i->io_Device == g->SerialBase)
                        {
                            /* Serial */
                            retval = TRUE;
                            break;
                        }
                        else
                        {
                            /* timer */
                            break;
                        }

                    }
                    WaitPort(myport);
                }
                AbortIO((struct IORequest *)s);
                WaitIO((struct IORequest *)s);
                AbortIO((struct IORequest *)t);
                WaitIO((struct IORequest *)t);

                DeleteIORequest((struct IORequest *)t);
            }
            DeleteIORequest((struct IORequest *)s);
        }
        if ( FindTask(NULL) != WackTask )
        {
            DeleteMsgPort(myport);
        }
    }
    return(retval);
}

/****** SADAPI/SADWaitForPrompt ****************************************
*
*   NAME
*     SADWaitForPrompt -- Wait for SAD to issue a prompt
*
*   SYNOPSIS
*     prompt = SADWaitForPrompt(globvar, timeout)
*
*     ULONG SADWaitForPrompt(struct SADAPIVars *, ULONG);
*
*   FUNCTION
*     Waits for the remote SAD to issue a SAD prompt.  If received,
*     notes the prompt type, and returns that value.
*
*   INPUTS
*     globvar --    Global variable cookie from SADInit()
*     timeout --    The total number of seconds to wait for the
*                   prompt before giving up.
*
*   RESULT
*     prompt  --    $00 to indicate failure (timeout expired)
*                   or the trailing character of the SAD prompt:
*                   $BF, $3F, or $21.  See the SAD documentation
*                   in the exec.library autodocs for explanations
*                   of these.
*
*   EXAMPLE
*
*   NOTES
*     It's probably a good idea to send a SAD NOP command before calling
*     this routine for the first time.  Otherwise, if a SAD prompt had
*     already been sent (but missed), it might be as long as two seconds
*     before another prompt was sent.
*
*   BUGS
*
*   SEE ALSO
*     exec.library/SAD
*
******************************************************************************
*
*/


ULONG SADWaitForPrompt(struct SADAPIVars *g, ULONG TimeToWait)
{
    struct IOExtStruct *s;
    struct timerequest *t;
    ULONG retval=0;
    struct MsgPort *myport;

    if ( FindTask(NULL) == WackTask )
    {
        myport = g->GPPort;
    }
    else
    {
        myport = CreateMsgPort();
    }
    if ( myport )
    {
        s = (struct IOExtStruct *) CreateIORequest(myport,sizeof(struct IOExtStruct));
        if (s)
        {
            t = (struct timerequest *) CreateIORequest(myport,sizeof(struct timerequest));
            if (t)
            {
                UBYTE ReceiveByte[1];
                UWORD index;
                UBYTE SADPrompt[3]={'S','A','D'};

                index = 0;
                s->IOExt.io_Length = 1;
                s->IOExt.io_Device = g->SerialBase;
                s->IOExt.io_Unit = g->SerialUnit;
                s->IOExt.io_Data = &ReceiveByte;
                s->IOExt.io_Command = CMD_READ;
                SendIO((struct IORequest *)s);

                t->tr_node.io_Device = g->TBase;
                t->tr_node.io_Unit = g->TimerUnit;
                t->tr_node.io_Command = TR_ADDREQUEST;
                t->tr_time.tv_secs = TimeToWait;
                SendIO((struct IORequest *)t);

                while (TRUE)
                {
                    struct IORequest *i;
                    i = (struct IORequest *) GetMsg(myport);
                    if (i)
                    {
                        /* Check to see if it's a returned serial or timer request */
                        if (i->io_Device == g->SerialBase)
                        {
                            /* Serial */
                            if (index == 3)
                            {
                                retval = ReceiveByte[0];
                                break;
                            }

                            if (ReceiveByte[0] == SADPrompt[index])
                            {
                                index++;
                            }
                            else
                            {
                                index = 0;
                                /* handle the case of data matching prompt... */
                                if (ReceiveByte[0] == SADPrompt[index])
                                {
                                    index++;
                                }
                            }
                            s->IOExt.io_Data = &ReceiveByte;
                            s->IOExt.io_Length = 1;
                            SendIO((struct IORequest *)s);
                        }
                        else
                        {
                            /* timer */
                            break;
                        }

                    }
                    WaitPort(myport);
                }
                AbortIO((struct IORequest *)s);
                WaitIO((struct IORequest *)s);
                AbortIO((struct IORequest *)t);
                WaitIO((struct IORequest *)t);

                DeleteIORequest((struct IORequest *)t);
            }
            DeleteIORequest((struct IORequest *)s);
        }
        if ( FindTask(NULL) != WackTask )
        {
            DeleteMsgPort(myport);
        }
    }
    return(retval);
}

/****** SADAPI/SADReadLong *********************************************
*
*   NAME
*     SADReadLong -- Attempt to read a longword from target memory
*
*   SYNOPSIS
*     success = SADReadLong(globvar, pointer, result)
*
*     BOOL SADReadLong(struct SADAPIVars *, void *, ULONG *);
*
*   FUNCTION
*     Issues a SAD READ_LONG command, which attempts to read
*     a longword from the address specified by pointer, and to store that
*     longword in the ULONG pointed to by result.
*
*   INPUTS
*     globvar --    Global variable cookie from SADInit()
*     pointer --    A pointer to the memory location you wish
*                   to write to.
*     result  --    A pointer to the ULONG at which you want the
*                   result stored.
*
*   RESULT
*     success --    TRUE/FALSE on whether the function is perceived
*                   to have succeeded or not.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       exec.library/SAD READ_LONG
*
******************************************************************************
*
*/



BOOL SADReadLong(struct SADAPIVars *g, void *pointer, ULONG *result)
{
    UBYTE cmd[6]={0xaf, 0x05, 0, 0, 0, 0};
    (*((ULONG *)&cmd[2])) = (ULONG) pointer;

    return (SADCmd(g,(void *)&cmd,6,TRUE,TRUE,2,(void *)result,4,NULL,0));
}

/****** SADAPI/SADReadWord *********************************************
*
*   NAME
*     SADReadWord -- Attempt to read a word from target memory
*
*   SYNOPSIS
*     success = SADReadWord(globvar, pointer, result)
*
*     BOOL SADReadWord(struct SADAPIVars *, void *, UWORD *);
*
*   FUNCTION
*     Issues a SAD READ_WORD command, which attempts to read
*     a word from the address specified by pointer, and to store that
*     word in the UWORD pointed to by result.
*
*   INPUTS
*     globvar --    Global variable cookie from SADInit()
*     pointer --    A pointer to the memory location you wish
*                   to write to.
*     result  --    A pointer to the UWORD at which you want the
*                   result stored.
*
*   RESULT
*     success --    TRUE/FALSE on whether the function is perceived
*                   to have succeeded or not.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*     Because SAD in V39 has an incorrect READ_WORD command, this operation
*     is emulated via READ_ARRAY.  Since READ_ARRAY does -two byte reads-,
*     the result may well not be what the caller expects, especially on
*     reading custom chip registers.
*
*   SEE ALSO
*       exec.library/SAD READ_WORD
*
******************************************************************************
*
*/



BOOL SADReadWord(struct SADAPIVars *g, void *pointer, UWORD *result)
{

// Because V39's SAD has an incorrect ReadWord, we emulate it with
// ReadArray().  Under V40, we use the correct (working) ReadWord.

    if (!(g->Flags & SADAPIF_V40))
        return(SADReadArray(g,pointer,(void *)result,2));
    else
    {
        UBYTE cmd[6]={0xaf, 0x04, 0, 0, 0, 0};
        (*((ULONG *)&cmd[2])) = (ULONG) pointer;

        return (SADCmd(g,(void *)&cmd,6,TRUE,TRUE,2,(void *)result,2,NULL,0));
    }
}

/****** SADAPI/SADReadByte *********************************************
*
*   NAME
*     SADReadByte -- Attempt to read a byte from target memory
*
*   SYNOPSIS
*     success = SADReadByte(globvar, pointer, result)
*
*     BOOL SADReadByte(struct SADAPIVars *, void *, UBYTE *);
*
*   FUNCTION
*     Issues a SAD READ_BYTE command, which attempts to read
*     a byte from the address specified by pointer, and to store that
*     byte in the UBYTE pointed to by result.
*
*   INPUTS
*     globvar --    Global variable cookie from SADInit()
*     pointer --    A pointer to the memory location you wish
*                   to write to.
*     result  --    A pointer to the UBYTE at which you want the
*                   result stored.
*
*   RESULT
*     success --    TRUE/FALSE on whether the function is perceived
*                   to have succeeded or not.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       exec.library/SAD READ_BYTE
*
******************************************************************************
*
*/

BOOL SADReadByte(struct SADAPIVars *g, void *pointer, UBYTE *result)
{
    UBYTE cmd[6]={0xaf, 0x03, 0, 0, 0, 0};

    (*((ULONG *)&cmd[2])) = (ULONG) pointer;

    return (SADCmd(g,(void *)&cmd,6,TRUE,TRUE,2,(void *)result,1,NULL,0));
}

/****** SADAPI/SADReadArray *********************************************
*
*   NAME
*     SADReadArray -- Attempt to read an array of bytes from target memory
*
*   SYNOPSIS
*     success = SADReadArray(globvar, pointer, result, length)
*
*     BOOL SADReadArray(struct SADAPIVars *, void *, UBYTE *, ULONG);
*
*   FUNCTION
*     Issues a SAD READ_ARRAY command, which attempts to read
*     length bytes from the address specified by pointer, and to return
*     that data in the array pointed to by result.
*
*   INPUTS
*     globvar --    Global variable cookie from SADInit()
*     pointer --    A pointer to the memory location you wish
*                   to write to.
*     result  --    A pointer to the array at which you want the
*                   result stored.
*     length  --    The number of bytes in the array.
*
*   RESULT
*     success --    TRUE/FALSE on whether the function is perceived
*                   to have succeeded or not.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       exec.library/SAD READ_ARRAY
*
******************************************************************************
*
*/


BOOL SADReadArray(struct SADAPIVars *g, void *pointer, UBYTE *resultptr, ULONG length)
{
    UBYTE cmd[10]={0xaf, 0x0e, 0, 0, 0, 0, 0, 0, 0, 0};

    (*((ULONG *)&cmd[2])) = (ULONG) pointer;

    (*((ULONG *)&cmd[6])) = (ULONG) length;

    return (SADCmd(g,(void *)&cmd,10,TRUE,TRUE,2,(void *)resultptr,length,NULL,0));

}


/****** SADAPI/SADWriteLong *********************************************
*
*   NAME
*     SADWriteLong -- Attempt to write a byte to target memory
*
*   SYNOPSIS
*     success = SADWriteLong(globvar, pointer, data)
*
*     BOOL SADWriteLong(struct SADAPIVars *, void *, ULONG);
*
*   FUNCTION
*     Issues a SAD WRITE_LONG command, which attempts to write
*     the given data longword to the address specified by pointer, on
*     the target machine.
*
*   INPUTS
*     globvar --    Global variable cookie from SADInit()
*     pointer --    A pointer to the memory location you wish
*                   to write to.
*     data    --    Longword to write at address given by pointer.
*
*   RESULT
*     success --    TRUE/FALSE on whether the function is perceived
*                   to have succeeded or not.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       exec.library/SAD WRITE_LONG
*
******************************************************************************
*
*/



BOOL SADWriteLong(struct SADAPIVars *g, void *pointer, ULONG data)
{
    UBYTE cmd[10]={0xaf, 0x02, 0, 0, 0, 0, 0, 0, 0, 0};

    (*((ULONG *)&cmd[2])) = (ULONG) pointer;

    (*((ULONG *)&cmd[6])) = (ULONG) data;

    return (SADCmd(g,(void *)&cmd,10,TRUE,TRUE,2,(void *)0,0,NULL,0));
}


/****** SADAPI/SADWriteWord *********************************************
*
*   NAME
*     SADWriteWord -- Attempt to write a word to target memory
*
*   SYNOPSIS
*     success = SADWriteWord(globvar, pointer, data)
*
*     BOOL SADWriteWord(struct SADAPIVars *, void *, UWORD);
*
*   FUNCTION
*     Issues a SAD WRITE_WORD command, which attempts to write
*     the given data word to the address specified by pointer, on
*     the target machine.
*
*   INPUTS
*     globvar --    Global variable cookie from SADInit()
*     pointer --    A pointer to the memory location you wish
*                   to write to.
*     data    --    Word to write at address given by pointer.
*
*   RESULT
*     success --    TRUE/FALSE on whether the function is perceived
*                   to have succeeded or not.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       exec.library/SAD WRITE_WORD
*
******************************************************************************
*
*/



BOOL SADWriteWord(struct SADAPIVars *g, void *pointer, UWORD data)
{
    UBYTE cmd[8]={0xaf, 0x01, 0, 0, 0, 0, 0, 0};

    (*((ULONG *)&cmd[2])) = (ULONG) pointer;
    (*((UWORD *)&cmd[6])) = (UWORD) data;

    return (SADCmd(g,(void *)&cmd,8,TRUE,TRUE,2,(void *)0,0,NULL,0));
}


/****** SADAPI/SADWriteArray ********************************************
*
*   NAME
*     SADWriteArray -- Attempt to write an array of bytes to target memory
*
*   SYNOPSIS
*     success = SADWriteArray(globvar, pointer, datapointer, length)
*
*     BOOL SADWriteArray(struct SADAPIVars *, void *, UBYTE *, ULONG)
*
*   FUNCTION
*     Issues a SAD WRITE_ARRAY command, which attempts to write
*     the given array of bytes to the address specified by pointer on
*     the target machine.
*
*   INPUTS
*     globvar --    Global variable cookie from SADInit()
*     pointer --    A pointer to the memory location you wish
*                   to write to.
*     datapointer --
*                   A pointer to the array of bytes that you want to write.
*     length  --    Number of bytes in the array.
*
*   RESULT
*     success --    TRUE/FALSE on whether the function is perceived
*                   to have succeeded or not.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       exec.library/SAD WRITE_ARRAY
*
******************************************************************************
*
*/



BOOL SADWriteArray(struct SADAPIVars *g, void *pointer, UBYTE *dataptr, ULONG datalength)
{
    BOOL res=FALSE;
    UBYTE cmd[10]={0xaf, 0x0d, 0, 0, 0, 0, 0, 0, 0, 0};

    (*((ULONG *)&cmd[2])) = (ULONG) pointer;

    (*((ULONG *)&cmd[6])) = (ULONG) datalength;

    res = SADCmd(g,(void *)cmd,10,TRUE,TRUE,2,(void *)0,0,dataptr,datalength);

    return(res);
}

/****** SADAPI/SADWriteByte *********************************************
*
*   NAME
*     SADWriteByte -- Attempt to write a byte to target memory
*
*   SYNOPSIS
*     success = SADWriteByte(globvar, pointer, data)
*
*     BOOL SADWriteByte(struct SADAPIVars *, void *, UBYTE);
*
*   FUNCTION
*     Issues a SAD WRITE_BYTE command, which attempts to write
*     the given data byte to the address specified by pointer, on
*     the target machine.
*
*   INPUTS
*     globvar --    Global variable cookie from SADInit()
*     pointer --    A pointer to the memory location you wish
*                   to write to.
*     data    --    Byte to write at address given by pointer.
*
*   RESULT
*     success --    TRUE/FALSE on whether the function is perceived
*                   to have succeeded or not.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       exec.library/SAD WRITE_BYTE
*
******************************************************************************
*
*/

BOOL SADWriteByte(struct SADAPIVars *g, void *pointer, UBYTE byte)
{
    if (!(g->Flags & SADAPIF_V40))
        return(SADWriteArray(g,pointer,&byte,1));
    else
    {
        UBYTE cmd[8]={0xaf, 0x00, 0, 0, 0, 0, 0};

        (*((ULONG *)&cmd[2])) = (ULONG) pointer;
        (*((UBYTE *)&cmd[6])) = (UBYTE) byte;

        return (SADCmd(g,(void *)&cmd,7,TRUE,TRUE,2,(void *)0,0,NULL,0));

    }
}

/****** SADAPI/SADGetContextFrame ***************************************
*
*   NAME
*     SADGetContextFrame -- Find a pointer to the target's context frame
*
*   SYNOPSIS
*     success = SADGetContextFrame(globvar, resultptr)
*
*     BOOL SADGetContextFrame(struct SADAPIVars *, ULONG *);
*
*   FUNCTION
*     Issues a SAD GET_CONTEXT_FRAME command, which fetches the
*     address of the target machine's context frame.
*
*   INPUTS
*     globvar   --  Global variable cookie from SADInit()
*     resultptr --  A pointer to a ULONG, where you wish the
*                   Context frame pointer to be stored.
*
*   RESULT
*     success --    TRUE/FALSE on whether the function is perceived
*                   to have succeeded or not.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       exec.library/SAD GET_CONTEXT_FRAME
*
******************************************************************************
*
*/


BOOL SADGetContextFrame(struct SADAPIVars *g, ULONG *resultptr)
{
    UBYTE cmd[2]={0xaf, 0x08};

    return (SADCmd(g,(void *)&cmd,2,TRUE,TRUE,2,(void *)resultptr,4,NULL,0));
}

/****** SADAPI/SADReturnToSystem ****************************************
*
*   NAME
*     SADReturnToSystem -- Causes the target machine to exit SAD
*
*   SYNOPSIS
*     success = SADReturntoSystem(globvar)
*
*     BOOL SADReturnToSystem(struct SADAPIVars *);
*
*   FUNCTION
*     Issues a SAD RETURN_TO_SYSTEM command to the target machine.
*     This causes the remote SAD to exit from SAD and return
*     to the state from which it was started.  If used with
*     single-stepping, this command allows execution of the next
*     instruction.
*
*   INPUTS
*     globvar --    Global variable cookie from SADInit()
*
*   RESULT
*     success --    TRUE/FALSE on whether the function is perceived
*                   to have succeeded or not.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       exec.library/SAD RETURN_TO_SYSTEM
*
******************************************************************************
*
*/



BOOL SADReturnToSystem(struct SADAPIVars *g)
{
    UBYTE cmd[6]={0xaf,0x07,0x00,0x00,0x00,0x00};
    return (SADCmd(g,(void *)&cmd,6,TRUE,FALSE,0,0,0,NULL,0));

}

/****** SADAPI/SADCallFunction ****************************************
*
*   NAME
*     SADCallFunction -- Causes the target machine call a subroutine
*
*   SYNOPSIS
*     success = SADCallFunction(globvar,address)
*
*     BOOL SADCallFunction(struct SADAPIVars *, APTR);
*
*   FUNCTION
*     Issues a SAD CALL ADDRESS command to the target machine.
*     This causes the remote SAD to call a function at the
*     given address.
*     No parameters will be set up in registers.  a0-a1/d0-d1
*     are scratch, as usual.  No return code is available.
*
*   INPUTS
*     globvar --    Global variable cookie from SADInit()
*     address --    Address on the target machine to call
*
*   RESULT
*     success --    TRUE/FALSE on whether the function is perceived
*                   to have sucessfully called or not.  It is NOT
*		    the value returned by the function.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       exec.library/SAD CALL_FUNCTION
*
******************************************************************************
*
*/


BOOL SADCallFunction(struct SADAPIVars *g, APTR addr)
{
    UBYTE cmd[6]={0xaf,0x06,0x00,0x00,0x00,0x00};
    *((ULONG *) &cmd[2]) = (ULONG) addr;
    return(SADCmd(g,(void *)&cmd,6,TRUE,TRUE,60,0,0,NULL,0));

}

/****** SADAPI/SADNOP ****************************************************
*
*   NAME
*     SADNOP -- Does nothing more than tell SAD not to time out yet
*
*   SYNOPSIS
*     success = SADNOP(globvar)
*
*     BOOL SADNOP(struct SADAPIVars *);
*
*   FUNCTION
*     Issues a SAD NOP command.
*
*   INPUTS
*     globvar --    Global variable cookie from SADInit()
*
*   RESULT
*     success --    TRUE/FALSE on whether the function is perceived
*                   to have succeeded or not.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       exec.library/SAD NOP
*
******************************************************************************
*
*/



BOOL SADNOP(struct SADAPIVars *g)
{
    UBYTE cmd[2]={0xaf,0x00};
    return(SADCmd(g,(void *)&cmd,2,FALSE,FALSE,0,0,0,NULL,0));
}

/****** SADAPI/SADAllocateMemory *****************************************
*
*   NAME
*     SADReleaseMemory -- Attempt to allocate memory on the target machine
*
*   SYNOPSIS
*     success = SADAllocateMemory(globvar, length, type)
*
*     BOOL SADAllocateMemory(struct SADAPIVars *, ULONG, ULONG);
*
*   FUNCTION
*     This function causes the target machine's SAD to attempt to
*     allocate memory with AllocVec(), given the length and type of
*     memory provided.
*
*   INPUTS
*     globvar --    Global variable cookie from SADInit()
*     length  --    Size of memory block requested
*     type    --    Type of memory needed
*
*   RESULT
*     success --    TRUE/FALSE on whether the function is perceived
*                   to have succeeded or not.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       exec.library/SAD ALLOCATE_MEMORY
*
******************************************************************************
*
*/

BOOL SADAllocateMemory(struct SADAPIVars *g, ULONG length, ULONG type, ULONG *result)
{

    UBYTE cmd[10]={0xaf, 0x09, 0, 0, 0, 0, 0, 0, 0, 0};

    (*((ULONG *)&cmd[2])) = (ULONG) length;

    (*((ULONG *)&cmd[6])) = (ULONG) type;

    return(SADCmd(g,(void *)&cmd,10,TRUE,TRUE,2,(void *) result,4,NULL,0));
}

/****** SADAPI/SADReleaseMemory *****************************************
*
*   NAME
*     SADReleaseMemory -- Free up any memory allocated by SADAllocMemory
*
*   SYNOPSIS
*     success = SADReleaseMemory(globvar, address)
*
*     BOOL SADReleaseMemory(struct SADAPIVars *, void *);
*
*   FUNCTION
*     Issues a SAD RELEASE_MEMORY command, which causes the target
*     machine's SAD to attempt to FreeVec() the address provided.
*
*   INPUTS
*     globvar --    Global variable cookie from SADInit()
*     address --    A pointer to the memory vector returned by
*                   SADAllocMemory()
*
*   RESULT
*     success --    TRUE/FALSE on whether the function is perceived
*                   to have succeeded or not.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       exec.library/SAD RELEASE_MEMORY
*
******************************************************************************
*
*/


BOOL SADReleaseMemory(struct SADAPIVars *g, void *ptr)
{

    UBYTE cmd[6]={0xaf, 0x0a, 0, 0, 0, 0};

    (*((ULONG *)&cmd[2])) = (ULONG) ptr;

    return(SADCmd(g,(void *)&cmd,6,TRUE,TRUE,2,0,0,NULL,0));
}

/****** SADAPI/SADReset **************************************************
*
*   NAME
*     SADReset -- Cause the target machine to soft-reset itself
*
*   SYNOPSIS
*     success = SADReset(globvar)
*
*     BOOL SADCallAddress(struct SADAPIVars *);
*
*   FUNCTION
*     Issues a SAD RESET command, which causes the target machine's
*     SAD to reset the target machine.
*
*   INPUTS
*     globvar --    Global variable cookie from SADInit()
*
*   RESULT
*     success --    TRUE/FALSE on whether the function is perceived
*                   to have succeeded or not.
*
*   EXAMPLE
*
*   NOTES
*     Once this command is issued, don't expect the remote SAD to exist
*     any longer.
*
*   BUGS
*
*   SEE ALSO
*       exec.library/SAD RESET
*
******************************************************************************
*
*/



BOOL SADReset(struct SADAPIVars *g)
{
    UBYTE cmd[6]={0xaf,0x0f};
    return (SADCmd(g,(void *)&cmd,2,TRUE,FALSE,0,0,0,NULL,0));
}

/****** SADAPI/SADCallAddress *****************************************
*
*   NAME
*     SADCallAddress -- Cause the target machine to JSR to an address
*
*   SYNOPSIS
*     success = SADCallAddress(globvar, address, timeout)
*
*     BOOL SADCallAddress(struct SADAPIVars *, void *, ULONG);
*
*   FUNCTION
*     Issues a SAD CALL_ADDRESS command, which causes the target machine's
*     SAD to attempt to JSR to the address you've provided.
*
*   INPUTS
*     globvar --    Global variable cookie from SADInit()
*     address --    The address you wish to JSR to
*     timeout --    Amount of time (in seconds) you want SADCallAddress()
*                   to wait for the JSR to complete.
*
*   RESULT
*     success --    TRUE/FALSE on whether the function is perceived
*                   to have succeeded or not.
*
*   EXAMPLE
*
*   NOTES
*     Since various routines can take various amounts of time, the timeout
*     field needs to be set by the caller in this case.
*
*
*   BUGS
*
*   SEE ALSO
*       exec.library/SAD CALL_ADDRESS
*
******************************************************************************
*
*/



BOOL SADCallAddress(struct SADAPIVars *g, void *address, ULONG timeout)
{
    UBYTE cmd[6]={0xaf, 0x06, 0, 0, 0, 0};

    (*((ULONG *)&cmd[2])) = (ULONG) address;

    return (SADCmd(g,(void *)&cmd,6,TRUE,TRUE,timeout,0,0,NULL,0));
}

/****** SADAPI/SADTurnOnSingleStepping *************************************
*
*   NAME
*     SADTurnOnSingleStepping -- Turn on Single stepping
*
*   SYNOPSIS
*     success = SADTurnOnSingleStepping(globvar, pointer)
*
*     BOOL SADTurnOffSingleStepping(struct SADAPIVars *, ULONG *);
*
*   FUNCTION
*     Issues a SAD TURN_ON_SINGLE command, and turns on single stepping
*     on the remote machine.
*
*   INPUTS
*     globvar --    Global variable cookie from SADInit()
*     pointer --    A pointer to a ULONG where you want the previous
*                   exception vector stored.  This must be passed to
*                   SADTurnOffSingleStepping() to disable stepping.
*
*   RESULT
*     success --    TRUE/FALSE on whether the function is perceived
*                   to have succeeded or not.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       exec.library/SAD TURN_ON_SINGLE
*
******************************************************************************
*
*/

BOOL SADTurnOnSingleStepping(struct SADAPIVars *g, ULONG *ptr)
{
    UBYTE cmd[2]={0xaf,0x0b};

    return(SADCmd(g,(void *)&cmd,2,TRUE,TRUE,2,ptr,4,NULL,0));
}

/****** SADAPI/SADTurnOffSingleStepping ************************************
*
*   NAME
*     SADTurnOffSingleStepping -- Turn off Single stepping
*
*   SYNOPSIS
*     success = SADTurnOffSingleStepping(globvar, vector)
*
*     BOOL SADTurnOffSingleStepping(struct SADAPIVars *, ULONG);
*
*   FUNCTION
*     Issues a SAD TURN_OFF_SINGLE command, and turns off single stepping
*     on the remote machine.
*
*   INPUTS
*     globvar --    Global variable cookie from SADInit()
*     vector  --    The exception vector as returned by
*                   SADTurnOnSingleStepping()
*
*   RESULT
*     success --    TRUE/FALSE on whether the function is perceived
*                   to have succeeded or not.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       exec.library/SAD TURN_OFF_SINGLE
*
******************************************************************************
*
*/

BOOL SADTurnOffSingleStepping(struct SADAPIVars *g, ULONG vector)
{
    UBYTE cmd[6]={0xaf, 0x0c, 0, 0, 0, 0};

    (*((ULONG *)&cmd[2])) = (ULONG) vector;

    return(SADCmd(g,(void *)&cmd,6,TRUE,TRUE,2,0,0,NULL,0));
}


