/**************************************************************************
**
** center_custom.c  - NIPC Library Process Startup Module
**
** Copyright 1992, Commodore-Amiga, Inc.
**
** $Id: center_custom.c,v 1.31 94/02/07 19:52:53 kcd Exp $
**
***************************************************************************

/*------------------------------------------------------------------------*/

#include <exec/exec.h>
#include <exec/ports.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <dos.h>

#include "nipcinternal.h"

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include "nipcinternal_protos.h"


#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "externs.h"

/*------------------------------------------------------------------------*/

/* Externals */

extern void EntryPoint(void);
extern void SetSysBase(void);

/*------------------------------------------------------------------------*/

struct MyLibrary
{
    struct Library ml_Lib;
    ULONG ml_SegList;
    ULONG ml_Flags;
    APTR ml_ExecBase;           /* pointer to exec base */
    LONG ml_Data;               /* Global data */
};


/*------------------------------------------------------------------------*/

void __asm InitLock(register __a6 struct Library * libbase)
{
    register struct NBase *nb = gb;

    InitSemaphore(&nb->openlock);
    gb->jumpstart = FALSE;
}

/*------------------------------------------------------------------------*/

void __asm StartLibrary(register __a6 struct Library * libbase)
{
    register struct NBase *nb = gb;

    ULONG cptags[9];
    struct MsgPort *port;
    struct TagItem *ctti;
    struct Process **tproc;

    ObtainSemaphore(&nb->openlock);

    if (!gb->jumpstart)
    {

        cptags[0] = NP_Entry;
        cptags[1] = (ULONG) & EntryPoint;
        cptags[2] = NP_Name;
        cptags[3] = (ULONG) "nipc_supervisor";
        cptags[4] = NP_FreeSeglist;
        cptags[5] = TRUE;
        cptags[6] = NP_Priority;
        cptags[7] = 10;
        cptags[8] = TAG_DONE;

        port = CreateMsgPort();

        gb->smsg.sm_Msg.mn_ReplyPort = port;
        gb->smsg.sm_Msg.mn_Length = sizeof(struct StartMessage);
        gb->smsg.sm_LibBase = libbase;

        nb->nb_DOSBase = (struct Library *) OpenLibrary("dos.library", 37);
        if (!DOSBase)
            return;

        ctti = (struct TagItem *) &cptags;
        tproc = (struct Process **)&(gb->nipcprocess);
//        gb->nipcprocess = (struct Process *) CreateNewProc(ctti);
        *tproc = (struct Process *) CreateNewProc(ctti);
        gb->nipcprocess->pr_WindowPtr = (APTR)-1;
        CloseLibrary((struct Library *) nb->nb_DOSBase);
        PutMsg(&(nb->nipcprocess->pr_MsgPort), (struct Message *) & nb->smsg);
        WaitPort(port);
        GetMsg(port);
        DeleteMsgPort(port);

    }

    ReleaseSemaphore(&nb->openlock);

}

/*------------------------------------------------------------------------*/

void KillOffNIPC(void)
{
    register struct NBase *nb = gb;
    struct Task *ns;
    ns = FindTask("nipc_supervisor");
    if (ns)
        Signal(ns,(1 << nb->shutdownbit));
}

/*------------------------------------------------------------------------*/

void __asm __stdargs LibraryProcess(void)
{
    register struct NBase *nb = gb;

    ULONG waitmask, wokemask, shutdownmask;
    ULONG libsize;
    BPTR  ourseglist;
    BPTR  procsegarray, procseglist;
    struct ipproto *ipp;
    BOOL split=FALSE;

    waitmask = 0L;
    NewList((struct List *) &gb->BufferCache);
    NewList((struct List *) &gb->BuffEntryCache);

    {
        struct Node *node;
        UWORD count;

        for(count=0; count<128; count++)
            if(node = AllocMem(sizeof(struct Buffer),MEMF_PUBLIC|MEMF_CLEAR))
                AddHead((struct List *) &nb->BufferCache,node);

        for(count=0; count<256; count++)
            if(node = AllocMem(sizeof(struct BuffEntry),MEMF_PUBLIC|MEMF_CLEAR))
                AddHead((struct List *) &nb->BuffEntryCache,node);

    }

    if(nb->nb_DOSBase = (struct Library *) OpenLibrary("dos.library", 37L))
    {
        if(nb->nb_UtilityBase = (struct Library *) OpenLibrary("utility.library",37L))
        {
            if (InitMemory())
            {
                if(OpenConfig())
                {
                    if(InitSana2())
                    {
                        if(InitRoute())
                        {
                            if(InitARP())
                            {
                                if(InitTimer())
                                {

                                    NewList((struct List *) &gb->ProtocolList);

                                    if(init_icmp())
                                    {
                                        if(rdp_init())
                                        {
                                            if(UDP_Init())
                                            {
                                                if(InitANMP())
                                                {
                                                    if(InitResolver())
                                                    {
                                                        nb->shutdownbit = AllocSignal(-1L);
                                                        if (gb->shutdownbit != -1)
                                                        {
                                                            shutdownmask = (1 << gb->shutdownbit);
                                                            CloseConfig();
                                                            gb->jumpstart = TRUE;
                                                            ReplyMsg((struct Message *) & nb->smsg);
                                                            waitmask |= (gb->s2sigmask | gb->timersigmask | gb->resolversigmask | shutdownmask);

                                                            while (TRUE)
                                                            {
                                                                wokemask = Wait(waitmask);

                                                                if (wokemask & shutdownmask)
                                                                {
                                                                    break;
                                                                }
                                                                if (wokemask & gb->s2sigmask)
                                                                {
                                                                    ONTIMER(31);
                                                                    PollPort();         /* stuff on the gb->replyport for the s2 devices */
                                                                    OFFTIMER(31);
                                                                }
                                                                if (wokemask & gb->resolversigmask)
                                                                {
                                                                    DoResolver();
                                                                }
                                                                if (wokemask & gb->timersigmask)
                                                                {
                                                                    ONTIMER(9);
                                                                    TimeOut();          /* reset the timer */
                                                                    HeartBeat();
                                                                    OFFTIMER(9);
                                                                }
                                                            }

                                                            ourseglist = (BPTR) ((struct NBase *)getreg(REG_A6))->nb_SegList;
                                                            procsegarray = ((struct Process *)FindTask(0L))->pr_SegList;
                                                            procseglist = (BPTR) ((ULONG *)BADDR(procsegarray))[3];

                                                            /* Follow our seglist to the end, and tack them onto it */
                                                            while (((ULONG *) BADDR(ourseglist))[0])
                                                                ourseglist = (BPTR) ((ULONG *)BADDR(ourseglist))[0];
                                                            ((ULONG *) BADDR(ourseglist))[0] = (ULONG) procseglist;

                                                            /* Insert us at the top of the list */
                                                            ((ULONG *)BADDR(procsegarray))[3] =
                                                                               (BPTR) ((struct NBase *)getreg(REG_A6))->nb_SegList;

                                                            split = TRUE;
                                                            FreeSignal(nb->shutdownbit);
                                                        }
                                                        DeinitResolver();
                                                    }
                                                    DeinitANMP();
                                                }
                                            }
                                        }
                                        /* deinit all protocols */
                                        ipp = (struct ipproto *) gb->ProtocolList.mlh_Head;
                                        while (ipp->ipproto_link.mln_Succ)
                                        {
                                            (*ipp->ipproto_Deinit) ();
                                            ipp = (struct ipproto *) ipp->ipproto_link.mln_Succ;
                                        }
                                    }
                                    DeinitTimer();
                                }
                                DeinitARP();
                            }
                            DeinitRoute();
                        }
                        DeinitSana2();
                    }
                    CloseConfig();
                }
                DeinitMemory();
            }
            CloseLibrary(nb->nb_UtilityBase);
        }
        CloseLibrary(nb->nb_DOSBase);
    }
    {
        struct Node *node;

        while(node=RemHead((struct List *)&nb->BufferCache))
            FreeMem(node,sizeof(struct Buffer));

        while(node=RemHead((struct List *)&nb->BuffEntryCache))
            FreeMem(node,sizeof(struct BuffEntry));
    }

    if(nb->smsg.sm_Msg.mn_Node.ln_Type != NT_REPLYMSG)
    {
        nb->nipcprocess = NULL;
        ReplyMsg((struct Message *)&nb->smsg);
    }

    if (split)
    {
        /* Free up the library base */
        libsize = ((struct NBase *)getreg(REG_A6))->LibNode.lib_NegSize +
                  ((struct NBase *)getreg(REG_A6))->LibNode.lib_PosSize;
        FreeMem( (void *) (getreg(REG_A6) - (((struct NBase *)getreg(REG_A6))->LibNode.lib_NegSize)),
                 (LONG) libsize);
    }

}

/*------------------------------------------------------------------------*/

/* The Grand place where all input arrives at */

void ProtocolInput(inpbuff, length, ptype, dev)
struct Buffer *inpbuff;
ULONG length;
UWORD ptype;
struct sanadev *dev;
{

    switch (ptype)
    {

        case (PACKET_IP):
            multiplex_in(inpbuff, dev);
            break;

        case (PACKET_ARP):
            ARPInput(inpbuff, length, dev);
            break;

        default:
            FreeBuffer(inpbuff);
    }

}

/*------------------------------------------------------------------------*/

void HeartBeat()
{

    /*
     * go through each of the protocols attached to IP, and enter their
     * timeout routines.  Everyone will get "buzzed" every time the
     * timer.device finishes a request - currently every 0.1 second.
     */
    struct ipproto *pr;

//      kprintf("TimeCount: %ld\n",(ULONG)gb->TimeCount);

    gb->TimeCount--;
    if(!gb->TimeCount)
    {
        gb->TimeCount = 10;
        pr = (struct ipproto *) gb->ProtocolList.mlh_Head;
        while (pr->ipproto_link.mln_Succ)
        {
            (*pr->ipproto_Timeout) ();
            pr = (struct ipproto *) pr->ipproto_link.mln_Succ;
        }
        ResolverTimeout();
        arp_timeout();
        FragmentTimeout();
        ANMPTimeout();
        DevReqTimeout();
	rdp_ack_timeout();
    }
}

