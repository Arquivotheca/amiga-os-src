
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* |_o_o|\\ Copyright (c) 1988 The Software Distillery.  All Rights Reserved *
* |. o.| ||	     Written by Doug Walker				    *
* | .  | ||	     The Software Distillery				    *
* | o  | ||	     235 Trillingham Lane				    *
* |  . |//	     Cary, NC 27513					    *
* ======	     BBS:(919)-471-6436                                     *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "netcomm.h"
#include "proto.h"

#include <exec/interrupts.h>
#include <resources/misc.h>
#include <hardware/cia.h>

int InitRDevice   U_ARGS((GLOBAL));
int TermRDevice   U_ARGS((GLOBAL));
int ParRead       U_ARGS((void *, int));
int ParWrite      U_ARGS((int, void *, int));
int ParReadV      U_ARGS((void *, int, void *, int, void *, int));
int ParWriteV     U_ARGS((int, void *, int, void *, int, void *, int));

int ReSync        U_ARGS((GLOBAL, APTR));

void ParAddress   U_ARGS((int));
long ParDataReady U_ARGS((void));
long AutoAllocMiscResource U_ARGS((long));
long AutoFreeMiscResource  U_ARGS((long));
void VRemICRVector  U_ARGS((long, long, void *));
long VAddICRVector  U_ARGS((long, long, void *));

int DestAddr = 0;
int Broke = 0;

extern void *VTask;		    /*	from wakeup.asm     */
extern long  VMask;		    /*	from wakeup.asm     */
extern void  VInt U_ARGS((void));   /*  from wakeup.ams     */

struct Interrupt VInterrupt = {
    {
	NULL, NULL,
	NT_INTERRUPT,
	-126,
	"net: server"
    },
    NULL,
    VInt
};

#if DEBUG

#ifdef ISHANDLER
char *dbgwind = "CON:0/0/640/160/NETPAR-HANDLER/a";
#endif

#ifdef ISSERVER
char *dbgwind = "CON:0/0/640/160/NETPAR-SERVER/a";
#endif

#endif /* DEBUG */

InitRDevice(global)
GLOBAL global;
{
#ifdef ISHANDLER
    struct NetNode *tmpnode;
#endif

    if(!global->RP.DataP)
       SetPacketBuffer(&global->RP, NETBUFSIZE);

#ifdef ISSERVER
    ParAddress(1);
    DestAddr = 2;
#else
    ParAddress(2);
    DestAddr = 1;
#endif

    /*
     *	Allocate resources and CIAA FLAG interrupt
     */

   if (AutoAllocMiscResource(MR_PARALLELPORT) != 0) 
   {
      return(1);
   }
   if (AutoAllocMiscResource(MR_PARALLELBITS) != 0) 
   {
      AutoFreeMiscResource(MR_PARALLELPORT);
      return(1);
   }

   VTask = (void *)FindTask(NULL);
   VMask = SIGBREAKF_CTRL_F;
   {
      long res = VAddICRVector(0, CIAICRB_FLG, &VInterrupt);
      if (res) 
      {
         AutoFreeMiscResource(MR_PARALLELPORT);
         AutoFreeMiscResource(MR_PARALLELBITS);
         return(1);
      }
   }

#ifdef ISSERVER

   return(0);

#endif

#ifdef ISHANDLER

#if 0
   global->RP.DataP = NULL;

   ResetPacketBuffer(&global->RP);
#endif

   for (tmpnode=global->netchain.next; tmpnode; tmpnode=tmpnode->next) 
   {
      BUG(("InitRDevice: Trying to resync '%s'\n", tmpnode->devname));
      NetRestart(global, &tmpnode->RootLock);
   }

#if 0
    ClearPacketBuffer(&global->RP);
#endif

    BUGP("InitRDevice: Exit")

    return(0);
#endif
}

int TermRDevice(global)
GLOBAL global;
{
#ifdef ISHANDLER
    struct NetNode *netnode;

    for (netnode = global->netchain.next; netnode; netnode=netnode->next) {
	if (netnode->ioptr) {
	    netnode->status = NODE_DEAD;
	    /* close io */
	    netnode->ioptr = NULL;
	}
    }
#endif

    VRemICRVector(0, CIAICRB_FLG, &VInterrupt);
    AutoFreeMiscResource(MR_PARALLELPORT);
    AutoFreeMiscResource(MR_PARALLELBITS);

    ClearPacketBuffer(&global->RP);

    return(0);
}

#ifdef ISSERVER
int ReSync(global, ioptr)
GLOBAL global;
APTR ioptr;
{
    if (Broke)
	return(1);      /*  can't resync, user break    */
    return(0);
}
#endif

#ifdef ISHANDLER
int ReSync(global, ioptr)
GLOBAL global;
APTR ioptr;
{
    struct NetNode *nnode;
    for(nnode=global->netchain.next; nnode; nnode=nnode->next)
    {
	if(nnode->ioptr == ioptr)
	{
	   if(ioptr == NULL) nnode->ioptr = (APTR)1;
	   NetRestart(global, &nnode->RootLock);
	}
    }
	
}
#endif

int PutRPacket(global, ioptr)
NGLOBAL global;
APTR ioptr;
{
   int len;

   BUG(("PutRPacket: type %d, Args %lx %lx %lx %lx\n", global->RP.Type,
       global->RP.Arg1, global->RP.Arg2,
       global->RP.Arg3, global->RP.Arg4));


TOP:
    if (SetSignal(0, SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
	Broke = 1;

    BUG(("PutRPacket: writing %d to %lx. . .", RPSIZEN+global->RP.DLen, ioptr));

    if (global->RP.DLen > 0)
	len = ParWriteV(DestAddr, (char *)&global->RP, RPSIZEN, 
	                global->RP.DataP, global->RP.DLen, NULL, 0);
    else
	len = ParWrite(DestAddr, (char *)&global->RP, RPSIZEN);

    if (len < RPSIZEN+global->RP.DLen) {
	BUG(("**********ERROR - wrote %d instead\n", len));
	BUGR("Net Write Error");
	return(1);
    }
    BUG(("%d written\n", len));

   if (global->n.infoport) {
	struct Message *m;
	while (m = GetMsg(global->n.ntirec.m.mn_ReplyPort)) {
	    if (m == &global->n.ntirec.m)
		global->n.inuse_rec = 0;
	    else
		global->n.inuse_trans = 0;
	}
	global->n.inf_trans += RPSIZEN + global->RP.DLen;

	if (!global->n.inuse_trans) {
	    BUG(("PutRPacket: Writing status info to port %lx: TRANSMIT %ld\n",
		global->n.infoport, global->n.inf_trans))

	    global->n.ntitrans.nti_bytes = global->n.inf_trans;
	    global->n.ntitrans.nti_direction = NTI_TRANSMIT;
	    PutMsg(global->n.infoport, &global->n.ntitrans.m);

	    global->n.inuse_trans = 1;
	    global->n.inf_trans = 0;
	}
#if DEBUG
	else
	    BUG(("PutRPacket: Skipping status write, packet outstanding\n"))
#endif
    }
    return(0);
}

int GetRPacket(global, ioptr)
NGLOBAL global;
APTR ioptr;
{
    int len;

    BUG(("GetRPacket: reading %d from %lx. . .", RPSIZEN, ioptr));

TOP:
    /*
     *	note, no Wait() done if data is ready.
     *	signal occurs when somebody aquires the line (D5),
     *	NOT when the address mark is posted (later).
     */

    while (ParDataReady() != 1) {
	long mask = Wait(SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F);

	if (mask & SIGBREAKF_CTRL_C) {
	    Broke = 1;
	    return(1);
	}
	if (ParDataReady() == 1)
	    break;
    }

#if 0
    SetPacketBuffer(&global->RP, NETBUFSIZE);   /*  so can use ParReadV    */
#endif

    len = ParReadV((char *)&global->RP, RPSIZEN, 
                   global->RP.DataP, NETBUFSIZE, NULL, 0);
    if (len < RPSIZEN || len < RPSIZEN + global->RP.DLen) {
	BUG(("**********ERROR - read %d instead, RP.DLen %d\n", 
	   len, global->RP.DLen));
	BUGR("Net Read Error")
	return(1);
    }

    if(global->RP.DLen == 0){
	global->RP.DataP[0] = '\0';
    }

    BUG(("Done, read %d+%d\n", RPSIZEN, global->RP.DLen));

    if (global->n.infoport) {
	struct Message *m;
	while (m = GetMsg(global->n.ntirec.m.mn_ReplyPort)) {
	    if (m == &global->n.ntirec.m)
		global->n.inuse_rec = 0;
	    else
		global->n.inuse_trans = 0;
	}
	global->n.inf_rec += RPSIZEN + global->RP.DLen;

	if (!global->n.inuse_rec) {
	    BUG(("GetRPacket: Writing status info to port %lx: RECEIVE %ld\n",
		global->n.infoport, global->n.inf_rec));

	    global->n.ntirec.nti_bytes = global->n.inf_rec;
	    global->n.ntirec.nti_direction = NTI_RECEIVE;
	    PutMsg(global->n.infoport, &global->n.ntirec.m);

	    global->n.inuse_rec = 1;
	    global->n.inf_rec = 0;
	}
#if DEBUG
	else
	    BUG(("GetRPacket: Skipping status write, packet outstanding\n"))
#endif
    }
    return(0);
}

