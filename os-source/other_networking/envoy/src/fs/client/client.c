/*
 * Envoy Filesystem Client V0.0
 *
 *
 * BUGS:
 *
 * If on reconnect, the seek of a FH fails to locate the correct position, set the file
 * size TO at least the last known position.
 * Potentially very dangerous... - REJ
 *
 */

/* REQUESTERTIMEOUT is the # of secs EZ requests are brought up for */
#define REQUESTERTIMEOUT  20
#define FILTERWBFILES 3

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include <dos/exall.h>
#include <intuition/intuition.h>
#include "/fs.h"
#include "efs_rev.h"

#include <clib/alib_protos.h>
#include <pragmas/exec_pragmas.h>
#include <clib/exec_protos.h>
#include <pragmas/dos_pragmas.h>
#include <clib/dos_protos.h>
#include <pragmas/intuition_pragmas.h>
#include <clib/intuition_protos.h>
#include "pragmas/nipc_pragmas.h"
#include "clib/nipc_protos.h"
#include "pragmas/services_pragmas.h"
#include "clib/services_protos.h"
#include <dos.h>
#include <string.h>

static struct Entity *Reconnect(struct MountedFSInfoClient *m,
			 ULONG announce, ULONG *failcode);
static void SaveName(struct CLock *cl,struct TPacket *tp);
static BOOL FixLock(struct Entity *re, struct Entity *e,struct CLock *c,
	     struct Transaction *t,
             struct TPacket *tp, ULONG cnum, struct MountedFSInfoClient *m);
static void KeepLock();
static void KeepFH();
static void NukeFH();
static BOOL FixFH(struct Entity *re, struct Entity *e,struct CFH *c,struct Transaction *t,
           struct TPacket *tp, ULONG cnum, struct MountedFSInfoClient *m);
static void SaveFHName();
static BOOL DoStart(LONG type,struct Transaction *t, struct TPacket *tp,
	     struct MountedFSInfoClient *m);
static BOOL NetDownRequest(UBYTE *machine, UBYTE *volume);
static BOOL MountFailedRequest(UBYTE *machine, UBYTE *volume, ULONG err);
static BOOL CantConnectRequest(UBYTE *machine);
static BOOL GPRequest(ULONG num);
static BOOL DoEnd(LONG type, struct Transaction *t, struct TPacket *tp, struct MountedFSInfoClient *m);
extern UBYTE *clpcpy(UBYTE *d, UBYTE *s, ULONG l);
static LONG TimeOutEasyRequest(struct Window *ref, struct EasyStruct *ez, ULONG id, APTR args, ULONG timeout);
static void free_exnext_data (struct CLock *cl, ULONG, ULONG, UBYTE zombie_type);
static void FreeZombie (struct CLock *cl);
void client(void);
static BOOL FixBString();
LONG do_match(struct ExAllControl *ec, ULONG data, struct ExAllData *ed);
static BOOL StartExAll(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	               struct MountedFSInfoClient *m);
static BOOL StartExamine(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	                 struct MountedFSInfoClient *m);
static BOOL HandleReconnect(struct Transaction *ot, struct DosPacket *dp, ULONG *retrycount,
			    BOOL *networkdown, ULONG *failcode, struct MinList *retrylist,
			    struct Entity **re,
			    struct MountedFSInfoClient *m);
static BOOL StartReadWrite(struct DosPacket *dp, struct MountedFSInfoClient *m);
static void AbortRW(struct RWState *rw, struct MountedFSInfoClient *m);

struct HoldWrapper
{
    struct MinNode hw_Node;
    struct Transaction *hw_Transaction;
};

struct RWState {
	struct MinList	  rw_TransList;	// list of active transactions
	struct DosPacket *rw_dp;	// read or write dospacket - may be redundant
	struct CFH	 *rw_FH;	// the filehandle for this read/write
	ULONG		  rw_Complete;	// # bytes totally done
	ULONG		  rw_Remaining;	// # bytes not started yet
	// complete + remaining + active transactions = size of read/write
};

char *rev=VSTRING;
char *revision = VERSTAG;

#define DEBUGMSG kprintf
#define MIN(x, y) ((x) < (y) ? (x):(y))
#define TOLONGWORD(x)	((void *) ((((ULONG) (x)) + 3) & ~3))

#define	GET_TP(t)	((t)->trans_Flags & TRANSF_REQNIPCBUFF ? \
			 (struct TPacket *) ((struct NIPCBuffEntry *) \
				     ((struct NIPCBuff *) (t)->trans_RequestData)->nbuff_Entries.mlh_Head)->nbe_Data : \
			 (struct TPacket *) (t)->trans_RequestData)

#define CONNECTIONMASK	(~0x80000000)
#define RWABORTMASK	(0x80000000)

extern struct Library * AbsExecBase;


    struct Library *SysBase;
    struct Library *DOSBase;
    struct Library *NIPCBase;
    struct Library *ServicesBase;
    struct Library *IntuitionBase;


void client()
{

    struct MsgPort *localport;
    struct StandardPacket *sp;
    struct DosPacket *dp;
    struct DeviceNode *dn;
//    BSTR   ourname;

    /* NIPC/Services Mgr stuff */
    struct Entity *e, *re;
    struct DosList *vol;
    struct Transaction *t;
    struct TPacket *tp;
    ULONG sigbit=0;
    ULONG cetags[3]={ENT_AllocSignal,0,TAG_DONE};
    ULONG attags[3]={TRN_AllocReqBuffer,TRANS_SIZE+sizeof(struct TPacket),TAG_DONE};
    ULONG retrycount=0;
    UBYTE *startstring;

    UBYTE machine[80],rname[256],lname[80],user[80],password[80], flags[17];

    BOOL networkdown=FALSE;                 /* True if the network appears to be down at the moment */
    BOOL startupstatus=FALSE;
    ULONG mflags = 0L;
    struct MountedFSInfoClient *m;          /* General global structure describing this beast */
    struct FileSysStartupMsg *fssm;
    ULONG goodpackets=0L;
    struct MinList retrylist;

    SysBase = (struct Library *) (*(void **)4L);
    NewList((struct List *) &retrylist);

    cetags[1] = (ULONG) &sigbit;



/* Now, attempt to get everything allocated and initialized */
    DOSBase = (struct Library *) OpenLibrary("dos.library",0L);
    if (DOSBase)
    {
        BOOL startupreturned=FALSE;

    /* Get startup packet */
        localport = &((struct Process *)FindTask(0L))->pr_MsgPort;
        while (!(sp=(struct StandardPacket *)GetMsg(localport)))
            WaitPort(localport);
        dp = (struct DosPacket *) sp->sp_Msg.mn_Node.ln_Name;
//        ourname = (BSTR) dp->dp_Arg1;
        dn = (struct DeviceNode *) BADDR(dp->dp_Arg3);
        fssm = (struct FileSysStartupMsg *) BADDR(dn->dn_Startup);
        dp->dp_Res1 = DOSFALSE; /* Default to handler not coming up at all */

        startstring = (UBYTE *) fssm->fssm_Unit;
        if (startstring=clpcpy(machine,startstring,80))
        {
            if (startstring=clpcpy(rname,startstring,80))
            {
                if (startstring=clpcpy(user,startstring,80))
                {
                    if (startstring=clpcpy(password,startstring,80))
                    {
                        UBYTE *gptmp;
                        int x;
                        gptmp = (UBYTE *) BADDR(dn->dn_Name);
                        if (gptmp[0] < 80)
                        {
                            memcpy(lname,&gptmp[1],gptmp[0]);
                            lname[gptmp[0]]=0;
                            startupstatus = TRUE;
                        }

                        startstring=clpcpy(flags,startstring,16);
                        for (x=0; x < strlen(flags); x++)
                        {
                            if ((flags[x] == 'B') || (flags[x] == 'b'))
                                mflags |= MFSIF_NOBACKDROP;
                            if ((flags[x] == 'D') || (flags[x] == 'd'))
                                mflags |= MFSIF_NODISKINFO;
                            if ((flags[x] == 'R') || (flags[x] == 'r'))
                                mflags |= MFSIF_NOREQUESTERS;
                        }
                    }
                }
            }
        }

        NIPCBase = (struct Library *) OpenLibrary("nipc.library",40L);
        if ((NIPCBase) && (startupstatus))
        {
            ServicesBase = (struct Library *) OpenLibrary("services.library",0L);
            if (ServicesBase)
            {
                e = (struct Entity *) CreateEntityA((struct TagItem *) &cetags);
                if (e)
                {
                    t = (struct Transaction *) AllocTransactionA((struct TagItem *)&attags);
                    if (t)
                    {
                        m = (struct MountedFSInfoClient *) AllocMem(sizeof(struct MountedFSInfoClient),MEMF_PUBLIC|MEMF_CLEAR);
                        if (m)
                        {
                            NewList( (struct List *) &m->mfsi_Locks);
                            NewList( (struct List *) &m->mfsi_FileHandles);

                            m->mfsi_Source = e;
                            m->mfsi_RemotePath = rname;
                            m->mfsi_LocalPath = lname;
                            m->mfsi_Machine = machine;
                            m->mfsi_UserName = user;
                            m->mfsi_Password = password;
                            m->mfsi_LocalPort = localport;
                            m->mfsi_Flags = mflags;
			    m->mfsi_ConnectTrans = t;

                            t->trans_ResponseData = t->trans_RequestData;
                            tp = (struct TPacket *) t->trans_RequestData;
                            t->trans_RespDataLength = t->trans_ReqDataLength;
                            networkdown = TRUE;

                            m->mfsi_Destination = Reconnect(m,1,0L);
                            re = m->mfsi_Destination;
                            if (m->mfsi_Destination)
                            {
                                Forbid();
                                if (!dn->dn_Task)
                                {
                                    dn->dn_Task = localport;
                                    Permit();
                                    ReplyPkt(dp,DOSTRUE,dp->dp_Res2);
                                    startupreturned = TRUE;

                                    if (!strlen(m->mfsi_VolumeName))           /* If no vol name given, use default */
                                        strcpy(m->mfsi_VolumeName,lname);

                                    vol = (struct DosList *) MakeDosEntry(m->mfsi_VolumeName,DLT_VOLUME);
                                    if (vol)
                                    {
                                        ((struct DeviceList *)vol)->dl_Task = localport;
                                        ((struct DeviceList *)vol)->dl_DiskType = ((struct DosEnvec *)BADDR(fssm->fssm_Environ))->de_DosType;

                                        m->mfsi_Volume = vol;


                                        AddDosEntry(vol);

                                        while(TRUE)
                                        {
                                            struct Transaction *rt,*ot;
                                            ULONG failcode=0;

                                            /* DosPackets */
// NOTE: some dospackets that come here are fake packets created by the exnext->exall stuff
// they have this as their replyport, and normally will only go through here if there's a
// failed send and a retry (too hard to rewrite, so we need a packet).  DoStart(), when it
// sees one of these, will just resend the exall transaction.  If we must reply it due to
// total failure, make sure we just free it instead.  The only valid argument is arg1,
// the lock.

					    // first search for transactions to retry.  Must empty
					    // this before checking the fs msgport.
					    ot = (struct Transaction *)
						 RemHead((struct List *) &retrylist);
					    if (ot)
					    {
                                                struct TPacket *otp;

						otp = GET_TP(ot);
//kprintf("have retry, t=$%lx, type=%ld, netdown=%ld, retry=%ld\n",ot,otp->tp_Type,networkdown,retrycount);
						if (!HandleReconnect(ot,otp->tp_DosPacket,
								     &retrycount,&networkdown,&failcode,&retrylist,
								     &re,m))
							continue;	// we failed the packet, and freed the transaction
									// OR we failed to reconnect, but haven't
									// given up yet, in which case ot has been
									// re-added to retrylist.
									// If packet is returned, rw are transactions marked
									// as garbage for ignoring.
									// Modifies retrycount, networkdown, and failcode
									// (its state information).
									// re (destination) may get modified too

						// we've reconnected - try to resend the transaction
						// only resend if it's a read/write!  Others must re-init to pick up
						// the new ServerFH/ServerLocks as needed
                                                if ((otp->tp_Type == ACTION_READ) || (otp->tp_Type == ACTION_WRITE))
						{
//kprintf("restarting r/w trans $%lx\n",ot);
						  // must update fields that might have changed
						  otp->tp_ServerMFSI = m->mfsi_ServerMFSI;  // Reconnect can change this
						  otp->tp_Arg1 = (LONG)((struct CFH *)
								        otp->tp_DosPacket->dp_Arg1)->CFH_ServerFH;
                                                  ot->trans_Timeout  = PACKETTIMEOUT + m->mfsi_DynamicDelay;
						  ot->trans_ClientPrivate |= (m->mfsi_ConnectionNumber &
									      CONNECTIONMASK);

/*
kprintf("Transaction dump:\n"
	"\tSrcEntity:\t$%lx\n"
	"\tDestEntity:\t$%lx\n"
	"\tCommand:\t%ld\n"
	"\tType:\t\t%ld\n"
	"\tError:\t\t%ld\n"
	"\tFlags:\t\t$%lx\n"
	"\tSeq:\t\t%ld\n"
	"\tReqData:\t$%lx\n"
	"\tReqDataLen:\t%ld\n"
	"\tReqDataActual:\t%ld\n"
	"\tRespData:\t$%lx\n"
	"\tRespDataLen:\t%ld\n"
	"\tRespDataActual:\t%ld\n"
	"\tTimeout:\t%ld\n"
	"\tReserved:\t$%lx\n"
	"\tClientPrivate:\t$%lx\n"
	"\tServerPrivate:\t$%lx\n",
ot->trans_SourceEntity,
ot->trans_DestinationEntity,
ot->trans_Command,
ot->trans_Type,
ot->trans_Error,
ot->trans_Flags,
ot->trans_Sequence,
ot->trans_RequestData,
ot->trans_ReqDataLength,
ot->trans_ReqDataActual,
ot->trans_ResponseData,
ot->trans_RespDataLength,
ot->trans_RespDataActual,
ot->trans_Timeout,
ot->trans_Reserved,
ot->trans_ClientPrivate,
ot->trans_ServerPrivate);
*/
						  BeginTransaction(m->mfsi_Destination,e,ot);

						} else {

//kprintf("resubmitting packet $%lx from $%lx\n",otp->tp_DosPacket,ot);
						  // not read/write.  Handle as if a new packet.  Needed to make sure
						  // we pick up things like changed ServerLocks and ServerFH's.
//FIX! what about ACTION_ENVOY_EXAMINE_ALL????  I think it's handled below...

						  sp = (struct StandardPacket *) (otp->tp_DosPacket->dp_Link);
						  FreeTransaction(ot);

						  goto got_packet;	// handle as if it was just coming in on localport
						}
					    } else {

					      // not checked if there's anything in the retry list
                                              sp = (struct StandardPacket *)GetMsg(localport);
                                              if (sp)
                                              {
got_packet:
                                                dp = (struct DosPacket *) sp->sp_Msg.mn_Node.ln_Name;
                                                dp->dp_Link = &(sp->sp_Msg); /* In case they don't do it */
                                                /*
                                                 * If the network is down, attempt to reconnect.  If this attempt
                                                 * fails, return the packet as failed.
                                                 */
                                                if ((networkdown) &&
						    (dp->dp_Type != ACTION_IS_FILESYSTEM))      /* To keep Workbench happy */
                                                {
//kprintf("packet $%lx arrived while network is down\n",dp);
						    if (!HandleReconnect(NULL,dp,
									 &retrycount,&networkdown,&failcode,&retrylist,
									 &re,m))
							continue;	// we failed the packet and returned it,
									// OR we failed to reconnect and requeued it at
									// the head of the packet list.

                                                } // if (networkdown && !IS_FILESYSTEM)...


						// handle read/write immediately (avoid trans allocate).  Exall is in
						// the normal case, even though it usually doesn't use ot, on occasion
						// it needs to call StartFakeLocateObject.
                                                if ((dp->dp_Type == ACTION_READ) || (dp->dp_Type == ACTION_WRITE))
                                                {
                                                    /* Action_Read and Action_Write */
                                                        if (!StartReadWrite(dp,m))
                                                        {
                                                            ReplyPkt(dp,dp->dp_Res1,dp->dp_Res2);
                                                            retrycount =0L;
                                                        }
                                                        if (!m->mfsi_Destination)
                                                            networkdown = TRUE;
                                                }
                                                else
                                                {
                                                    ot = (struct Transaction *) AllocTransactionA((struct TagItem *)&attags);
                                                    if (!ot)
                                                    {
//kprintf("can't get transaction\n");
// FIX! mayt not be needed now that we retry transactions!
							if (dp->dp_Type == ACTION_ENVOY_EXAMINE_ALL)
							{
							    struct CLock *cl = BADDR(dp->dp_Arg1);

							    // we "know" no packets will be outstanding
							    free_exnext_data(cl,FALSE,ERROR_NO_FREE_STORE,ZOMBIE_NOFREE);
							    if (cl->Zombie)
								FreeZombie(cl);
							} else
                                                	    ReplyPkt(dp,DOSFALSE,ERROR_NO_FREE_STORE);
                                                	retrycount =0L;
                                                    }
                                                    else
                                                    {
                                                        struct TPacket *otp;

                                                        otp = (struct TPacket *) ot->trans_RequestData;
                                                        otp->tp_ServerMFSI = m->mfsi_ServerMFSI;
                                                        otp->tp_DosPacket = dp;
                                                        otp->tp_Type = dp->dp_Type;
                                                        ot->trans_ResponseData = ot->trans_RequestData;
                                                        ot->trans_RespDataLength = ot->trans_ReqDataLength;
                                                        ot->trans_ReqDataActual = ot->trans_ReqDataLength;
                                                        ot->trans_Timeout = PACKETTIMEOUT + m->mfsi_DynamicDelay;
                                                        ot->trans_Command = FSCMD_DOSPACKET;
						        ot->trans_ClientPrivate |= (m->mfsi_ConnectionNumber &
										    CONNECTIONMASK);

						        if (dp->dp_Type == ACTION_EXAMINE_ALL)
							{
                                                          if (!DoStart(dp->dp_Type,ot,otp,m))
                                                          {
                                                            FreeTransaction(ot);
                                                            ReplyPkt(dp,dp->dp_Res1,dp->dp_Res2);
                                                            retrycount =0L;
                                                          }
                                                          else
                                                          {
                                                            FreeTransaction(ot);
                                                          }
							  // no need to set networkdown, since we don't touch
							  // mfsi_Destination in exall
							} else {

                                                          if (DoStart(dp->dp_Type,ot,otp,m))
                                                          {
                                                            BeginTransaction(m->mfsi_Destination,e,ot);
                                                          }
                                                          else
                                                          {
							    struct CLock *cl = BADDR(dp->dp_Arg1);

                                                            FreeTransaction(ot);
// FIX! mayt not be needed now that we retry transactions!
							    if (dp->dp_Type == ACTION_ENVOY_EXAMINE_ALL)
							    {
								// we "know" no packets will be outstanding
								free_exnext_data(cl,FALSE,FALSE,ZOMBIE_NOFREE);
								if (cl->Zombie)
									FreeZombie(cl);
							    } else if (dp->dp_Type != ACTION_EXAMINE_NEXT ||
								!cl->ExNextData ||
								(!cl->ExNextData->WaitingPacket &&
								 !cl->ExNextData->NextExnext))
							    {
// i.e. return it unless it was an ExNext packet that got queued somehow (waiting for data or for zombie to clear)
								// don't do this if exnext got queued and doesn't
								// need a transaction.  Really should have three
								// returns from DoStart().
                                                            	ReplyPkt(dp,dp->dp_Res1,dp->dp_Res2);
							    }
                                                            retrycount = 0L;

                                                          }
							} // if type == ACTION_EXAMINE_ALL...else...

                                                    } // if (!(ot = AllocTransactionA()))...else...

                                                } // if (read || write)...else...

                                              } // if (sp = GetMsg(localport))...

					    } // if (remhead(retrylist))...else...

                                            /* Returning Transactions */
                                            rt = GetTransaction(e);
                                            if (rt)
                                            {
                                                struct TPacket *rtp;

						// find the request tp struct - may be in an NIPCBuff
						rtp = GET_TP(rt);

//kprintf("got returning transaction $%lx, tp $%lx, type %ld\n",rt,rtp,rtp->tp_Type);
#if 0
if (rtp->tp_Type == 0)
{int i;
kprintf("trans_respdataactual = $%lx\n",rt->trans_RespDataActual);
kprintf("trans_respdataLength = $%lx\n",rt->trans_RespDataLength);
kprintf("trans_responsedata = $%lx\n",rt->trans_ResponseData);
kprintf("trans_Flags = $%lx\n",rt->trans_Flags);
for (i = 0; i < sizeof(*rtp)/4; i++)
kprintf("%ld: $%08lx\n",i,((ULONG *) rtp)[i]);
}
#endif
                                                if (rt->trans_Error)
                                                {
			// we don't want to throw away transactions for read/write, since
			// they hold the key to identifying the RWState structure.  We can't
			// just restart the entire read/write like Greg tried, since some
			// of the transactions might have succeeded.  
			// We'll add the transaction to a list which we check/empty before we
			// check for incoming packets (the retrylist).  Transactions on the
			// retrylist will be re-submitted until they fail badly enough the be
			// returned with an error (perhaps after use intervention).

//kprintf("trans_Error = %ld\n",rt->trans_Error);
						    if (rt->trans_ClientPrivate & RWABORTMASK)
						    {
							// we got a failure of a read/write transaction we
							// aren't interested in.  Dont do anything.
							FreeTransaction(rt);
						    } else {
							// we care
                                                    	networkdown = TRUE;
                                                    	retrycount++;
						    	AddTail((struct List *) &retrylist,
								&(rt->trans_Msg.mn_Node));
						    }
                                                }
                                                else
                                                {
                                                    goodpackets++;
                                                    if (goodpackets > 200)
                                                    {
                                                        goodpackets -= 200;
                                                        if (m->mfsi_DynamicDelay > PACKETTIMEOUT/2)
                                                            m->mfsi_DynamicDelay -= PACKETTIMEOUT/2;
							else
							    m->mfsi_DynamicDelay = 0;
                                                    }

                                                    if (DoEnd(rtp->tp_Type,rt,rtp,m))
						    {
							// something wants to resend the transaction (end_exnext)
							rt->trans_Timeout = PACKETTIMEOUT + m->mfsi_DynamicDelay;
							rt->trans_ClientPrivate |= (m->mfsi_ConnectionNumber &
										    CONNECTIONMASK);
							BeginTransaction(m->mfsi_Destination,e,rt);

							// don't free it
						    } else
							FreeTransaction(rt);
                                                }
                                            }

					    // Tres bizarre...
                                            if ((!sp) && (!rt) && (retrylist.mlh_Head->mln_Succ == NULL))
                                                Wait( (1 << sigbit) | (1 << (localport->mp_SigBit)) );
                                        }
                                        RemDosEntry(vol);
                                        FreeDosEntry(vol);
                                    }
                                }
                                else
                                {
                                    startupreturned = TRUE;
                                    ReplyPkt(dp,DOSTRUE,dp->dp_Res2);
                                    Permit();
                                }
                            }
                            re = m->mfsi_Destination;
                            FreeMem(m,sizeof(struct MountedFSInfoClient));
                        }
                        FreeTransaction(t);
                    }
                    if (re)
                        LoseService(re);

                    DeleteEntity(e);
                }
                CloseLibrary(ServicesBase);
            }
            CloseLibrary(NIPCBase);
        }
        if (!startupreturned)
        {
            Forbid();
            if (dn->dn_Task)
            {
                ReplyPkt(dp,DOSTRUE,dp->dp_Res2);
            }
            else
            {
                ReplyPkt(dp,DOSFALSE,dp->dp_Res2);
            }
            Permit();
        }
        CloseLibrary(DOSBase);
    }
}

static UBYTE *
CtoBcpy (UBYTE *dest, UBYTE *src)
{
	UBYTE *orig = dest++;

	while (*src)
		*dest++ = *src++;
	*orig = dest - (orig+1);

	return orig;
}

static BOOL FixBString(UBYTE *cp)
{
    BOOL retval=FALSE;
    UBYTE x;
    for (x = 1; x <= cp[0]; x++)
        if (cp[x]==':')
        {
            cp[0] -= x;
            CopyMem(&cp[x+1],&cp[1],cp[0]);
            x = 0;
            retval = TRUE;
        }
    return(retval);
}

static void SaveFHName(struct CFH *fh,struct TPacket *tp)
{
    UBYTE *n, *s;
    int l;
    n = &(((UBYTE *)tp)[sizeof(struct TPacket)]);
    l = strlen(n)+1;

    s = (UBYTE *) AllocMem(l,MEMF_CLEAR|MEMF_PUBLIC);
    if (s)
    {
        fh->CFH_FullServerPath = s;
        strcpy(s,n);
    }
    else
        fh->CFH_FullServerPath = NULL;
}

static void SaveName(struct CLock *cl,struct TPacket *tp)
{
    UBYTE *n, *s;
    int l;
    n = &(((UBYTE *)tp)[sizeof(struct TPacket)]);
    l = strlen(n)+1;

    s = (UBYTE *) AllocMem(l,MEMF_CLEAR|MEMF_PUBLIC);
    if (s)
    {
        cl->CLock_FullServerPath = s;
        strcpy(s,n);
    }
    else
        cl->CLock_FullServerPath = NULL;
}

static void KeepLock(APTR thelock,struct MountedFSInfoClient *m)
{

    struct ResourcesUsed *ru;

    ru = (struct ResourcesUsed *) AllocMem(sizeof(struct ResourcesUsed),MEMF_CLEAR|MEMF_PUBLIC);
    if (!ru)
        return;
    ru->ru_Resource = thelock;
    AddTail((struct List *)&m->mfsi_Locks,(struct Node *)&ru->ru_link);
}


static void KeepFH(APTR thefh,struct MountedFSInfoClient *m)
{

    struct ResourcesUsed *ru;

    ru = (struct ResourcesUsed *) AllocMem(sizeof(struct ResourcesUsed),MEMF_CLEAR|MEMF_PUBLIC);
    if (!ru)
        return;
    ru->ru_Resource = thefh;
    AddTail((struct List *)&m->mfsi_FileHandles,(struct Node *)&ru->ru_link);
}

static void NukeLock(APTR thelock,struct MountedFSInfoClient *m)
{

    struct ResourcesUsed *r;
    r = (struct ResourcesUsed *) m->mfsi_Locks.mlh_Head;
    while (r->ru_link.ln_Succ)
    {
        if (r->ru_Resource == (APTR) thelock)
        {
            Remove((struct Node *)r);
            FreeMem(r,sizeof(struct ResourcesUsed));
            return;
        }
        r = (struct ResourcesUsed *) r->ru_link.ln_Succ;
    }
}

static void NukeFH(APTR thefh,struct MountedFSInfoClient *m)
{
    struct ResourcesUsed *r;

    r = (struct ResourcesUsed *) m->mfsi_FileHandles.mlh_Head;
    while (r->ru_link.ln_Succ)
    {
        if (r->ru_Resource == (APTR) thefh)
        {
            Remove((struct Node *)r);
            FreeMem(r,sizeof(struct ResourcesUsed));
            return;
        }
        r = (struct ResourcesUsed *) r->ru_link.ln_Succ;
    }
}

// does 2 things: one, check for out-of-date filehandle.  Two, queue packet if read/write active.
static BOOL ValidateFH(struct CFH *cf, struct DosPacket *dp, struct MountedFSInfoClient *m)
{ 
	if (cf->CFH_ConnectionNumber < m->mfsi_ConnectionNumber)
	{
		dp->dp_Res1 = FALSE;
		if ((dp->dp_Type == ACTION_READ) || (dp->dp_Type == ACTION_WRITE) ||
		    (dp->dp_Type == ACTION_SEEK))
		{
			dp->dp_Res1 = -1;	// different error code
		}
        	dp->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
		return FALSE;
	}

	// if a read or write is active, we must queue all packets until it's done
	if (cf->CFH_Flags & CFHF_RW_ACTIVE)
	{
//kprintf("queuing packet $%lx (type %ld) for cfh $%lx\n",dp,dp->dp_Action,cf);
		AddTail((struct List *) &(cf->CFH_Packets),&(dp->dp_Link->mn_Node));
		return FALSE;
	}
	return TRUE;
}

// after a read/write ends, release the packets back onto the incoming port (at the head)
static void ReleasePackets(struct CFH *cf, struct MountedFSInfoClient *m)
{
	struct Message *sp;

	while (sp = (struct Message *) RemHead((struct List *) &(cf->CFH_Packets)))
	{
//kprintf("releasing packet $%lx (type %ld) for cfh $%lx\n",sp->mn_Node.ln_Name,
//((struct DosPacket *) (sp->mn_Node.ln_Name))->dp_Action,cf);
		Disable();
		AddHead(&(m->mfsi_LocalPort->mp_MsgList),&(sp->mn_Node));
		Enable();
	}
	cf->CFH_Flags &= ~CFHF_RW_ACTIVE;
}

static BOOL FixFH(struct Entity *re, struct Entity *e,struct CFH *c,struct Transaction *t,
           struct TPacket *tp, ULONG cnum, struct MountedFSInfoClient *m)
{

 UBYTE *datax;
 int x;
 LONG oldtype;

    oldtype = tp->tp_Type;
   /* This FH is known to be created on a previous connection */

    datax = &((UBYTE *)tp)[sizeof(struct TPacket)];
    datax = TOLONGWORD(datax);

    t->trans_Command = FSCMD_DOSPACKET;
    t->trans_Timeout = PACKETTIMEOUT + m->mfsi_DynamicDelay;
    if (c->CFH_Access == ACTION_FINDINPUT)
        tp->tp_Type = ACTION_FINDINPUT;
    else
        tp->tp_Type = ACTION_FINDUPDATE;
    tp->tp_Arg1 = 0L;
    tp->tp_Arg2 = -1L;
    tp->tp_Arg3= ((ULONG) datax - (ULONG)tp);
    tp->tp_Arg4 = m->mfsi_Flags & FLAGSMASK;
    tp->tp_ServerMFSI = m->mfsi_ServerMFSI;
    tp->tp_DosPacket = (struct DosPacket *) 0xFFFFFFFF; /* Encourage E-hits */
    if (c->CFH_FullServerPath)
    {
        x = strlen(c->CFH_FullServerPath);
        datax[0]=x;
        CopyMem(c->CFH_FullServerPath,&datax[1],x);
        FixBString(datax);

        t->trans_ReqDataActual = t->trans_ReqDataLength;
        DoTransaction(re,e,t);
        t->trans_Timeout = PACKETTIMEOUT + m->mfsi_DynamicDelay;
        tp->tp_Type = oldtype;
        if (!t->trans_Error)
        {
            if (tp->tp_Res1)
            {
                c->CFH_ConnectionNumber = cnum;
                c->CFH_ServerFH = (struct FileLock *) tp->tp_Res1;
                t->trans_Timeout = PACKETTIMEOUT + m->mfsi_DynamicDelay;
                tp->tp_Type = ACTION_SEEK;
                tp->tp_Arg1 = (LONG) c->CFH_ServerFH;
                tp->tp_Arg2 = c->CFH_Pos;
                tp->tp_Arg3 = OFFSET_BEGINNING;
                tp->tp_ServerMFSI = m->mfsi_ServerMFSI;
                tp->tp_DosPacket = (struct DosPacket *) 0xFFFFFFFF;
                DoTransaction(re,e,t);
            }
        }
        else
            return(FALSE);
        return(TRUE);
    }
    return(FALSE);

}


static BOOL FixLock(struct Entity *re, struct Entity *e,struct CLock *c,struct Transaction *t,
             struct TPacket *tp, ULONG cnum, struct MountedFSInfoClient *m)
{
    UBYTE *datax;
    int x;
    LONG oldtype;

    oldtype = tp->tp_Type;

/* This lock is known to be created on a previous connection */

    datax = &((UBYTE *)tp)[sizeof(struct TPacket)];
    datax = TOLONGWORD(datax);

    t->trans_Command = FSCMD_DOSPACKET;
    tp->tp_Type = ACTION_LOCATE_OBJECT;
//    tp->tp_Arg1 = -1L;
    tp->tp_Arg1 = 0L;
    tp->tp_Arg2 = ((ULONG)datax-(ULONG)tp);
    tp->tp_Arg3 = c->CLock_FileLock.fl_Access;
    tp->tp_Arg4 = m->mfsi_Flags & FLAGSMASK;
    tp->tp_ServerMFSI = m->mfsi_ServerMFSI;
    tp->tp_DosPacket = (struct DosPacket *) 0xFFFFFFFF;
    if (c->CLock_FullServerPath)
    {
        x = strlen(c->CLock_FullServerPath);
        datax[0]=x;
        CopyMem(c->CLock_FullServerPath,&datax[1],x);
        FixBString(datax);

        t->trans_ReqDataActual = t->trans_ReqDataLength;
        DoTransaction(re,e,t);
        t->trans_Timeout = PACKETTIMEOUT+m->mfsi_DynamicDelay;
        tp->tp_Type = oldtype;
        if (!t->trans_Error)
        {
            if (tp->tp_Res1)
            {
                c->CLock_ConnectionNumber = cnum;
                c->CLock_ServerLock = (struct FileLock *) tp->tp_Res1;
                c->CLock_FileLock.fl_Key = tp->tp_Res1;
            }
        }
        else
        {
            return(FALSE);
        }
        return(TRUE);
    }
    return(FALSE);
}


// ot will be NULL if this isn't a retry
// FIX! move more of this to mfsi structure!!!!!!

static BOOL HandleReconnect(struct Transaction *ot, struct DosPacket *dp, ULONG *retrycount,
			    BOOL *networkdown, ULONG *failcode, struct MinList *retrylist,
			    struct Entity **re, struct MountedFSInfoClient *m)
{
//kprintf("HandleReconnect: ot $%lx, dp $%lx, retry %ld, netdown %ld\n",ot,dp,*retrycount,*networkdown);
	// if we have to Reconnect, any queued packets are going to be returned with
	// ENVOYERR_ABORTED.  DON'T do another reconnect then!  Notice that they were for an old
	// connection and just retry them quietly.  Also, other queued packets might come back
	// with a timeout as well.
	if (ot && (ot->trans_ClientPrivate & CONNECTIONMASK) != m->mfsi_ConnectionNumber)
	{
		// this is an failure from a previous connection.  Quietly resubmit (if read/write)
		// or re-start if other packet type (maintains ordering!)

		// note: it may be envoyerr_aborted (for transactions still queued at previous
		// reconnect), or a timeout or other error.  In any case, since we've reconnected
		// again since then, we should just quietly restart this one.

//kprintf("got bogus error %ld, old connection %ld, current is %ld\n",
//ot->trans_Error,(ot->trans_ClientPrivate & CONNECTIONMASK),m->mfsi_ConnectionNumber);
		*failcode = 0;
		*retrycount = 0L;
		*networkdown = FALSE;

		return TRUE;
	}

	if (*retrycount > 1)
	{
		BOOL afail=FALSE;

		if (m->mfsi_Flags & MFSIF_NOREQUESTERS)
			afail = TRUE;
		else
		    if (!NetDownRequest(m->mfsi_Machine,m->mfsi_LocalPath))
			afail = TRUE;

		if (afail)
		{
		    ULONG failure,failresp;

		    failure = ERROR_NETWORK_FAILED;
		    if (*failcode == ENVOYERR_NORESOURCES)
			failure = ERROR_NO_FREE_STORE;
		    failresp = DOSFALSE;

		    // read, write, seek have error as -1.  Also must handle other read/write transactions.
		    if (dp->dp_Type == ACTION_SEEK)
		    {
			failresp = -1;
			goto reply_pkt;

		    } else if ((dp->dp_Type == ACTION_READ) ||
			       (dp->dp_Type == ACTION_WRITE))
		    {
			failresp = -1;
			if (ot) {
			    // this is a retry of a failed transaction
			    struct TPacket *tp;
			    struct RWState *rw; 

			    // since we now use NIPCbufs, tp could be in either Request or ResponseData
			    tp = GET_TP(ot);
			    rw = (struct RWState *) tp->tp_Arg4;  // may be garbage if (t->trans_ClientPrivate)

			    if (!(ot->trans_ClientPrivate & RWABORTMASK)) // if !0, then we ignore this transaction
			    {
				AbortRW(rw,m);	// free rw, etc, mark all transactions to be ignored
				goto reply_pkt;
			    } // else this read request had already failed somewhere, packet is already replied

			} else
			    goto reply_pkt;	// an initial read/write while network is down

		    } else if (dp->dp_Type == ACTION_ENVOY_EXAMINE_ALL) {

		        // if fake packet, wind things up specially!
			struct CLock *cl = BADDR(dp->dp_Arg1);

			// we "know" no packets will be outstanding
			free_exnext_data(cl,failresp,failure,ZOMBIE_NOFREE);
			if (cl->Zombie)
				FreeZombie(cl);
			// no reply
		    } else {
reply_pkt:
//kprintf("network failure, returning $%lx (%ld)\n",dp,dp->dp_Type);
		    	ReplyPkt(dp,failresp,failure);
		    }

		    if (ot)
			FreeTransaction(ot);

		    *retrycount = 0L;
		    return FALSE;		// this transaction nuked

		} // if afail...

	} // if *retrycount > 1

	if (!(m->mfsi_Destination =
	      Reconnect(m,(m->mfsi_Destination != 0),failcode)))
	{
		*re = m->mfsi_Destination;
		*networkdown = TRUE;

		/* put it at the head of the msgport or on the transaction retry list */
		if (ot) {
//kprintf("Reconnect fail: putting trans $%lx on retry list (dp $%lx) back on message port\n",ot,dp);
			AddTail((struct List *) retrylist,&(ot->trans_Msg.mn_Node));
		} else {
//kprintf("Reconnect fail: putting $%lx (msg $%lx) back on message port\n",dp,dp->dp_Link);
			Disable();
			AddHead(&(m->mfsi_LocalPort->mp_MsgList),(struct Node *)dp->dp_Link);
			Enable();
		}

		*retrycount += 1;
		m->mfsi_DynamicDelay += PACKETTIMEOUT;
		return FALSE;				// we didn't reconnect, try it again
							// will cause requester the second time
	}
	else
	{
		*failcode = 0;
		*retrycount = 0L;
		*networkdown = FALSE;
		*re = m->mfsi_Destination;
	}
	return TRUE;	// we reconnected, you can resend the transaction
}

static struct Entity *Reconnect(struct MountedFSInfoClient *m, ULONG announce, ULONG *failcode)
{
    ULONG notags[3]={FSVC_Error,0,TAG_DONE};
    struct Transaction *t = m->mfsi_ConnectTrans;
    struct TPacket *tp;
    struct Entity *e=m->mfsi_Source;
    struct Entity *re=m->mfsi_Destination;
    UBYTE *gp;
    LONG old;
    struct ResourcesUsed *rr;
    ULONG ecode=0L;
    notags[1] = (ULONG) &ecode;

    tp=GET_TP(t);
    old = tp->tp_Type;
    if (re)
    {
        LoseService(re);
    }

retry:
    re = (struct Entity *) FindServiceA(m->mfsi_Machine,"Filesystem",e,(struct TagItem *)&notags);
    if (!re)
    {
        if (failcode)
            *failcode = ecode;

        if ((announce) && (announce+1))
            if (!(m->mfsi_Flags & MFSIF_NOREQUESTERS))
                if (CantConnectRequest(m->mfsi_Machine))
                    goto retry; /* Sorry.  I wasn't about to mess the design up just to rid myself of a 'goto' */
        return(0L);
    }
    else
    {
        /* Send a MOUNT packet to the remote server */
        t->trans_Command = FSCMD_MOUNT;

        gp = (UBYTE *) t->trans_RequestData;
        strcpy(gp,m->mfsi_RemotePath);
        gp += strlen(m->mfsi_RemotePath)+1;
        strcpy(gp,m->mfsi_LocalPath);
        gp += strlen(m->mfsi_LocalPath)+1;
        strcpy(gp,m->mfsi_UserName);
        gp += strlen(m->mfsi_UserName)+1;
        strcpy(gp,m->mfsi_Password);
        gp += strlen(m->mfsi_Password)+1;
        gp = (UBYTE *) ( ((ULONG)gp) | 3)+1;
        *((ULONG *) gp) = (ULONG) &( ((struct Process *) FindTask(0L))->pr_MsgPort);
        gp += 4;
        strcpy(gp,"NEW");
        gp += strlen("NEW")+1;

        t->trans_Timeout = PACKETTIMEOUT+m->mfsi_DynamicDelay;
        t->trans_ReqDataActual = t->trans_ReqDataLength;
        t->trans_RespDataLength = t->trans_ReqDataLength;
        DoTransaction(re,e,t);
        t->trans_Command = FSCMD_DOSPACKET;
        t->trans_Timeout = PACKETTIMEOUT+m->mfsi_DynamicDelay;

        if (t->trans_Error)
        {
            if (announce)
                if (!(m->mfsi_Flags & MFSIF_NOREQUESTERS))
                    MountFailedRequest(m->mfsi_Machine, m->mfsi_LocalPath, t->trans_Error);
            tp->tp_Type = old;
            LoseService(re);
            return(0L);
        }
        else
        {
            UBYTE *r;

            r = (UBYTE *) t->trans_ResponseData;
            r = &r[4];
            strcpy(&m->mfsi_VolumeName[0],r);
            tp->tp_Type = old;

            m->mfsi_ServerMFSI = (APTR) (((ULONG *)t->trans_ResponseData)[0]);

            m->mfsi_ConnectionNumber++;

            /* Re-establish all known FH's and Locks.  Luckily, the code already exists for
             * doing exactly this.  Just go through the list!
             */

            rr = (struct ResourcesUsed *) m->mfsi_Locks.mlh_Head;
            while (rr->ru_link.ln_Succ)
            {
                FixLock(re,m->mfsi_Source,(struct CLock *) BADDR(rr->ru_Resource),
                        t,tp,m->mfsi_ConnectionNumber,m);
                rr = (struct ResourcesUsed *) rr->ru_link.ln_Succ;
            }

            rr = (struct ResourcesUsed *) m->mfsi_FileHandles.mlh_Head;
            while (rr->ru_link.ln_Succ)
            {

                FixFH(re,m->mfsi_Source,(struct CFH *) rr->ru_Resource,
                      t,tp,m->mfsi_ConnectionNumber,m);
                rr = (struct ResourcesUsed *) rr->ru_link.ln_Succ;
            }

            return(re);
        }
    }
}



/*
 * StartFind()
 *
 * The 'start' routine for all FINDxxxx packets; FINDINPUT, FINDOUTPUT, FINDUPDATE.
 *
 */
static BOOL StartFIND(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	              struct MountedFSInfoClient *m)
{
    struct CLock *oldlock;
    UBYTE *gp;

    /* There's a possibility that this FIND is depending on a Lock that was
     * created on a previous connection -- one that we may well not have
     * been rebuilt.  Check for this, and fail if true.
     * if the address of the lock is NULL, we're dealing with the special case of
     * the lock referencing the root of the mount.  These are always valid.
     */

    oldlock = BADDR(dp->dp_Arg2);
    if (oldlock)
    {
        if (oldlock->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
        {
            dp->dp_Res1 = DOSFALSE;
            dp->dp_Res2 = ERROR_INVALID_LOCK;
            return(FALSE);
        }
        tp->tp_Arg2 = (LONG) oldlock->CLock_ServerLock;
    }
    else
        tp->tp_Arg2 = 0L;

    /* Bump the general purpose pointer up to longword alignment */
    gp = &((UBYTE *) (t->trans_RequestData))[sizeof(struct TPacket)];
    gp = TOLONGWORD(gp);

    /* Tell the server the offset to get to the filename, from the start of the
     * TPacket */

    tp->tp_Arg3 = ((ULONG)gp - (ULONG) tp);

    tp->tp_Arg4 = (m->mfsi_Flags & FLAGSMASK);

    /* Copy the BSTR over */
    CopyMem(BADDR(dp->dp_Arg3),gp,((UBYTE *)BADDR(dp->dp_Arg3))[0]+1);

    /* Fix the BSTR; yank the stuff up to the colon */
    FixBString(gp);

    return(TRUE);
}


/*
 * EndFind()
 *
 * The 'end' routine for all FINDxxxx packets; FINDINPUT, FINDOUTPUT, FINDUPDATE.
 * All Endxxxx routines are required to return the dospacket, one way or another.
 *
 */
static void EndFind(struct TPacket *tp, struct MountedFSInfoClient *m)
{
    struct CFH *cfile;
    struct DosPacket *dp=tp->tp_DosPacket;


    /* If successful, create a local filehandle */
    if (tp->tp_Res1)
    {
        cfile = (struct CFH *) AllocMem(sizeof(struct CFH),MEMF_PUBLIC);
        if (!cfile)
        {
            ReplyPkt(dp,DOSFALSE,ERROR_NO_FREE_STORE);
            return;
        }
        SaveFHName(cfile,tp);       /* Grab the full pathname of this file */

        cfile->CFH_FH = (struct FileHandle *) BADDR(dp->dp_Arg1);
        cfile->CFH_ServerFH = (APTR) tp->tp_Res1;
        cfile->CFH_Pos = 0L;
        cfile->CFH_Access = dp->dp_Type;
        cfile->CFH_ConnectionNumber = m->mfsi_ConnectionNumber;
	cfile->CFH_Flags = 0;
	NewList((struct List *) &(cfile->CFH_Packets));

        KeepFH(cfile,m);

        ((struct FileHandle *)BADDR(dp->dp_Arg1))->fh_Type = m->mfsi_LocalPort;
        ((struct FileHandle *)BADDR(dp->dp_Arg1))->fh_Arg1 = (LONG) cfile;
    }

    ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);

}


/*
 * StartEND
 *
 */
static BOOL StartEND(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	             struct MountedFSInfoClient *m)
{
    struct CFH *cf;

    cf = (struct CFH *) dp->dp_Arg1;

    // checks for out-of-date fh, AND queues packet if a read/write is active
    if (!ValidateFH(cf,dp,m))
    {
	// it could have failed because it got queued, OR because it's out of date...
      if (cf->CFH_ConnectionNumber < m->mfsi_ConnectionNumber)
      {
/* While I'd really like to let people know that they're trying to free a FH
 * which is considered "out of date", I can't do that AND still free up the
 * FH.  So - I've decided to go for the latter.
 */
        UBYTE *l;

        /* The possibility that the filehandle referenced by this existed on a
         * previous connection, and couldn't be reestablished exists.
         * Thus, we check.
         */

        /* Free up the path string */
        l = (UBYTE *) cf->CFH_FullServerPath;
        if (l)
        {
            FreeMem(l,strlen(l)+1);
        }

        NukeFH(cf,m);
        FreeMem(cf,sizeof(struct CFH));
        dp->dp_Res1 = DOSTRUE;
      }

      return(FALSE);
    }

    tp->tp_Arg1 = (LONG) cf->CFH_ServerFH;

    return(TRUE);

}

/*
 * EndEnd, EndRename, EndSetProtect, EndSetDate, EndDeleteObject, EndSetComment
 *
 *  I love names like that.
 *
 */

static void EndEnd(struct TPacket *tp, struct MountedFSInfoClient *m)
{
    struct DosPacket *dp=tp->tp_DosPacket;

    if (tp->tp_Type == ACTION_END)
    {

        struct CFH *cf;
        UBYTE *l;

        /* The possibility that the filehandle referenced by this existed on a
         * previous connection, and couldn't be reestablished exists.
         * Thus, we check.
         */

        cf = (struct CFH *) dp->dp_Arg1;

        /* Free up the path string */
        l = (UBYTE *) cf->CFH_FullServerPath;
        if (l)
        {
            FreeMem(l,strlen(l)+1);
        }

        NukeFH(cf,m);

        /* Free the fh */
        FreeMem(cf,sizeof(struct CFH));

    }

    ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
}


/*
 * EndSeek
 *
 *
 */

static BOOL EndSeek(struct TPacket *tp, struct MountedFSInfoClient *m)
{
    struct DosPacket *dp=tp->tp_DosPacket;

    switch (dp->dp_Arg3)
    {
        case OFFSET_BEGINNING:
            ((struct CFH *) dp->dp_Arg1)->CFH_Pos = dp->dp_Arg2;
            break;

        case OFFSET_CURRENT:
            ((struct CFH *) dp->dp_Arg1)->CFH_Pos += dp->dp_Arg2;
            break;

        case OFFSET_END:
            /* Needs to insert an ACTION_SEEK from
             * OFFSET_CURRENT w/ offset=0 (specially flagged) in here to find
             * out where the position is after something like this.
             */
	    if (tp->tp_Res1 != -1)
	    {
		if (tp->tp_Arg4)	// we sent this to find out where the end is
		{
			((struct CFH *) dp->dp_Arg1)->CFH_Pos = tp->tp_Res1;
			tp->tp_Res1 = dp->dp_Res1;	// get old position back
		} else {
			dp->dp_Res1 = tp->tp_Res1;	// save old position to return
			tp->tp_Arg2 = 0;
			tp->tp_Arg3 = OFFSET_CURRENT;
			tp->tp_Arg4 = 1;		// flag to ourselves
			return TRUE;
		}
	    }
            break;

    }

    ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
    return FALSE;
}




/* StartSeek
 *
 * ACTION_SEEK
 *
 */
static BOOL StartSeek(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	              struct MountedFSInfoClient *m)
{

    struct CFH *cf;

    cf = (struct CFH *) dp->dp_Arg1;

    /* The possibility of an old, unreincarnated fh exists. */

    // checks for out-of-date fh, AND queues packet if a read/write is active
    if (!ValidateFH(cf,dp,m))
	return FALSE;

    tp->tp_Arg1 = (LONG) cf->CFH_ServerFH;
    tp->tp_Arg2 = dp->dp_Arg2;
    tp->tp_Arg3 = dp->dp_Arg3;
    tp->tp_Arg4 = 0;			// flag to me that this came from StartSeek
    return(TRUE);
}


/* StartReadWrite
 *
 * ACTION_READ, ACTION_WRITE
 *
 * Darn.  NIPC _shouldn't_ make copies of the data - Sana 2 doesn't require it.  
 * Currently, it make 2 copies of the data for each transaction.  On a slow net,
 * the net can get far enough behind to cause out-of-memory on large files.
 *
 * The _real_ solution is to rewrite nipc to not copy.  However, since that's not
 * really an option at the moment, we'll throttle the read requests to a max of
 * N outstanding at once.  Then as each comes in, if there's more left we'll send
 * it again.  We'll tune N or make it a config parameter.
 *
 * We'll attach a structure to the packets/transactions that holds the state info
 * for this read or write.
 *
 */
static BOOL StartReadWrite(struct DosPacket *dp, struct MountedFSInfoClient *m)
{
    struct CFH *cf;
    ULONG remaining, thispacketsize,numsent;
    struct HoldWrapper *hw;
    struct RWState *rw;

    cf = (struct CFH *) dp->dp_Arg1;

    /* The possibility of an old, unreincarnated fh exists. */

    // checks for out-of-date fh, AND queues packet if a read/write is active
    if (!ValidateFH(cf,dp,m))
	return FALSE;

    /* Find the total amount of data to read */
    dp->dp_Res1 = 0L;           /* Set this to 0 */
    dp->dp_Res2 = 0L;
    remaining = dp->dp_Arg3;
    if (remaining == 0)
    {
        /* dp_Res1 == 0 here - asked for 0 bytes, got 0 bytes */
        return FALSE;
    }

    // make the state structure for this read/write
    if (!(rw = AllocMem(sizeof(*rw),0)))
    {
	dp->dp_Res1 = -1;
	dp->dp_Res2 = ERROR_NO_FREE_STORE;
	return FALSE;
    }
    NewList((struct List *) &(rw->rw_TransList));
    cf->CFH_Flags |= CFHF_RW_ACTIVE;		// queue any packets for this FH until r/w is done
    rw->rw_dp = dp;
    rw->rw_FH = cf;
    rw->rw_Complete = 0;
    rw->rw_Remaining = remaining;

//kprintf("read/write: cfh $%lx, buffer $%lx, size $%lx, rw $%lx\n",cf,dp->dp_Arg2,dp->dp_Arg3,rw);

    numsent = 0;
    while (remaining && numsent < MAX_RW_PACKETS)
    {
	static ULONG rttags[5]={TRN_AllocReqBuffer,sizeof(struct TPacket),TRN_RespDataNIPCBuff,TRUE,TAG_DONE};
	static ULONG wttags[5]={TRN_AllocRespBuffer,sizeof(struct TPacket),TRN_ReqDataNIPCBuff,TRUE,TAG_DONE};
        struct Transaction *t;

        thispacketsize = MIN(remaining,MAXSIZE);        /* Find out how big this packet will be */

        t = AllocTransactionA((struct TagItem *)(dp->dp_Type == ACTION_READ ? &rttags : &wttags));
        if (t)
        {
            struct TPacket *tp;
	    struct NIPCBuff *buf;
	    struct NIPCBuffEntry *entry;

	    buf = AllocNIPCBuff(2);	// one entry for tp, one for data  (read or write)
	    if (!buf)
	    {
                FreeTransaction(t);
                t = NULL;	// so we go to cleanup code

	    } else {

//kprintf("buf $%lx, entry1 = $%lx, entry2 = $%lx\n",buf,buf->nbuff_Entries.mlh_Head,buf->nbuff_Entries.mlh_Head->mln_Succ);

		// the one place we care about read or write...
		if (dp->dp_Type == ACTION_READ)
		{
		    tp = (struct TPacket *) t->trans_RequestData;
		    t->trans_ResponseData = (void *) buf;
		    t->trans_ReqDataActual = t->trans_ReqDataLength;	// FIX? is this needed?
		    t->trans_RespDataLength = sizeof(*tp) + thispacketsize;	// hmmm... see below
		} else {
		    tp = (struct TPacket *) t->trans_ResponseData;
		    t->trans_RequestData = (void *) buf;
		    t->trans_ReqDataActual = sizeof(*tp) + thispacketsize;
		    // respdatalength already set to sizeof(*tp)
		}
		t->trans_Timeout = PACKETTIMEOUT + m->mfsi_DynamicDelay;
		t->trans_Command = FSCMD_DOSPACKET;
		// flag --  highg bit 1 means ignore this packet when returned
		t->trans_ClientPrivate = (m->mfsi_ConnectionNumber & CONNECTIONMASK);

//kprintf("tp = $%lx, thispacketsize = $%lx\n",tp,thispacketsize);
		tp->tp_ServerMFSI = m->mfsi_ServerMFSI;
		tp->tp_DosPacket = dp;
		tp->tp_Type = dp->dp_Type;			// read OR write

		tp->tp_Arg1 = (LONG)((struct CFH *)dp->dp_Arg1)->CFH_ServerFH;
		tp->tp_Arg2 = sizeof(struct TPacket);		// buffer pointer is an offset
		tp->tp_Arg3 = thispacketsize;
		/* When the packets return, I can match up the transaction to the
		 * rwstate by looking at tp_Arg4.  It's a pointer to struct RWState.
		 */
		tp->tp_Arg4 = (LONG) rw;

		// Somehow allocate a struct Buffer so I can have a segmented data buffer
		// first buffer is tp, second buffer is the actual data.
		// Hmmm.  I could just have a 2-longword result instead of passing back
		// the entire tp...

		// I assume offset is 0 since it's freshly allocated
		entry = (struct NIPCBuffEntry *) buf->nbuff_Entries.mlh_Head;
		entry->nbe_Data = (UBYTE *) tp;
		entry->nbe_PhysicalLength = 
		entry->nbe_Length = sizeof(*tp); // hmmmm....

		entry = (struct NIPCBuffEntry *) buf->nbuff_Entries.mlh_Head->mln_Succ;
		entry->nbe_Data = &(((UBYTE *) dp->dp_Arg2)[dp->dp_Arg3 - remaining]);  // where the data goes/comes
		entry->nbe_PhysicalLength = 
		entry->nbe_Length = thispacketsize; // hmmmm....

		hw = (struct HoldWrapper *) AllocMem(sizeof(struct HoldWrapper),0);
		if (hw)
		{
		    hw->hw_Transaction = t;
		    AddTail((struct List *) &(rw->rw_TransList),(struct Node *) &(hw->hw_Node));
//kprintf("sending trans $%lx... ",t);
		    BeginTransaction(m->mfsi_Destination,m->mfsi_Source,t);
//kprintf("sent\n");
		    remaining -= thispacketsize;
		    numsent++;
		}
		else
		{
		    FreeNIPCBuff(buf);		// also frees the entries
		    FreeTransaction(t);
		    t = NULL;	// so we go to cleanup code
		}
	    } // if !buf ...else...
	}  // if t ...

	if (!t)  //     NOTE not else!
	{
//kprintf("Can't allocate RW transaction!\n");
	    while (hw = (struct HoldWrapper *) RemHead((struct List *) &(rw->rw_TransList)) )
	    {
	        hw->hw_Transaction->trans_ClientPrivate |= RWABORTMASK; /* Ignore when it comes back */
	        FreeMem(hw,sizeof(struct HoldWrapper));
	    }
	    FreeMem(rw,sizeof(*rw));

	    cf->CFH_Flags &= ~CFHF_RW_ACTIVE;		// don't queue packets
	    m->mfsi_Destination = Reconnect(m,TRUE,0);
	    dp->dp_Res1 = DOSTRUE;
	    dp->dp_Res2 = ERROR_NO_FREE_STORE;
	    return(FALSE);
	}
/* Because of the potential problems of not being able to get a transaction at this point,
 * I feel the adequate solution is to return FAILED at this point, and also force a
 * reconnect.  I have yet to code this, though.  FIXME!  IMPORTANT! */
// I think he did fix it - REJ
// what to do about position in the file??????????  FIX!
    }
    rw->rw_Remaining = remaining;	// remember where we were

    return(TRUE);
}


static void AbortRW (struct RWState *rw, struct MountedFSInfoClient *m)
{
	struct HoldWrapper *hw;

//kprintf("aborting rw $%lx, cfh $%lx\n",rw,rw->rw_FH);
	// we must take any packets for the filehandle and release them back to the incoming packet port
	ReleasePackets(rw->rw_FH,m);

	while (hw = (struct HoldWrapper *) RemHead((struct List *) &(rw->rw_TransList)) )
	{
		hw->hw_Transaction->trans_ClientPrivate |= RWABORTMASK; 	// Ignore transaction when returned
		FreeMem(hw,sizeof(struct HoldWrapper));
	}
	FreeMem(rw,sizeof(*rw));
}

/*
 *  EndReadWrite
 *
 *  Finishes off each individual ACTION_READ/WRITE . . .
 *
 *  In main loop we grabbed the request tp pointer already (it's both request and response...)
 *
 */

static BOOL EndReadWrite(struct Transaction *t, struct TPacket *tp, struct MountedFSInfoClient *m)
{
    struct RWState *rw = (struct RWState *) tp->tp_Arg4;
    struct DosPacket *dp=tp->tp_DosPacket;	//  Hmm... (above)

//kprintf("end read/write, rw $%lx, trans $%lx, tp $%lx, type %ld, result %ld %ld\n",rw,t,tp,tp->tp_Type,tp->tp_Res1,tp->tp_Res2);
    /*
     * If successful, see if we need to send any more requests.  If done, free it all.
     */
    if (t->trans_ClientPrivate & RWABORTMASK)    /* If set, this T is part of an aborted Read/Write attempt */
        return FALSE;		// fix? do we need to notice if the position in the file changed????

    if (tp->tp_Res1 == -1)
    {

	AbortRW(rw,m);	// mark any other outstanding transactions to be thrown away
			// frees rw, all HoldWrappers.  Note that positioning might be an issue! FIX!?
	ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);

	return FALSE;	// Don't reuse transaction

    } else {
	// NOTE!  Funny things can happen here if we get a partial buffer back, and then
	// another buffer gets data!!!  Might happen if someone else is writing to the
	// file while we're reading it.  Maybe we can fix this on the server side, or
	// better yet, flag it here, and if another packet comes in, memcopy the data
	// to the right place.  FIX!!!!!!!!
	// Fixed.

	// must update position immediately, since we need to re-establish it on a Reconnect
        ((struct CFH *)dp->dp_Arg1)->CFH_Pos += tp->tp_Res1;	// update position in file
	rw->rw_Complete += tp->tp_Res1;

	// Don't send a request for more data if the previous request didn't read/write the
	// entire amount requested - let the application worry about it.  Note that other
	// transactions may be pending...  Some funny things can still happen since we _are_
	// double-buffering it on large requests.  We hope that they end up getting failed...
// Fix?  The only way to avoid that is to not N-buffer requests...  Maybe I should try that.
	if (tp->tp_Res1 != tp->tp_Arg3)
	{
		LONG res = rw->rw_Complete;		// before we free it - after updating it above

		AbortRW(rw,m);				// nuke any outstanding requests
		ReplyPkt(dp,res,0);			// return how much was read/written

		return FALSE;
	}

	if (rw->rw_Remaining)
	{
	    // there's more to send, turn transaction around and reuse.
	    struct NIPCBuff *buf;
	    struct NIPCBuffEntry *entry;
	    ULONG thispacketsize = MIN(rw->rw_Remaining,MAXSIZE);

	    // the one place we care about read or write...
	    if (dp->dp_Type == ACTION_READ)
	    {
	            tp = (struct TPacket *) t->trans_RequestData;
		    buf = (void *) t->trans_ResponseData;
		    t->trans_RespDataLength = sizeof(*tp) + thispacketsize;	// hmmm... see below
	    } else {
	            tp = (struct TPacket *) t->trans_ResponseData;
		    buf = (void *) t->trans_RequestData;
	            t->trans_ReqDataActual = sizeof(*tp) + thispacketsize;
//		    t->trans_RespDataLength = sizeof(*tp);			// paranoia
	    }
            t->trans_Timeout = PACKETTIMEOUT + m->mfsi_DynamicDelay;

//kprintf("reusing for throttle: trans $%lx, tp $%lx, size $%lx\n",t,tp,thispacketsize);
            tp->tp_ServerMFSI = m->mfsi_ServerMFSI;	// I think we can remove this...
	    tp->tp_Arg2 = sizeof(struct TPacket);	// server may overwrite this
            tp->tp_Arg3 = thispacketsize;

	    entry = (struct NIPCBuffEntry *) buf->nbuff_Entries.mlh_Head->mln_Succ;
	    entry->nbe_Data = &(((UBYTE *) dp->dp_Arg2)[dp->dp_Arg3 - rw->rw_Remaining]);  // where the data goes/comes
	    entry->nbe_PhysicalLength = 
	    entry->nbe_Length = thispacketsize; // hmmmm....

	    rw->rw_Remaining -= thispacketsize;

	    return TRUE;	// reuse the transaction

	} else {

	    // code that called DoEnd() will free the transaction for us
	    // we must free the node that holds a pointer to it.
	    struct HoldWrapper *hw;

	    if (dp->dp_Type == ACTION_READ)
		    FreeNIPCBuff((struct NIPCBuff *) t->trans_ResponseData);		// also frees the entries
	    else
		    FreeNIPCBuff((struct NIPCBuff *) t->trans_RequestData);		// also frees the entries

	    for (hw = (struct HoldWrapper *) rw->rw_TransList.mlh_Head;
		 hw->hw_Node.mln_Succ;
		 hw = (struct HoldWrapper *) hw->hw_Node.mln_Succ)
	    {
		if (hw->hw_Transaction == t)
		{
			Remove((struct Node *) &(hw->hw_Node));
			FreeMem(hw,sizeof(*hw));
			break;
		}
	    }
	    if (IsListEmpty((struct List *) &(rw->rw_TransList)))
	    {
		// we're done!  return the packet

		// we must take any packets for the filehandle and release them back to the incoming packet port
		// we can't release them until the ENTIRE read/write is done since on a reconnect the transactions
		// might get executed in the wrong order!
		ReleasePackets(rw->rw_FH,m);

//kprintf("read/wrote %ld bytes\n",rw->rw_Complete);
		ReplyPkt(dp,rw->rw_Complete,0);
		FreeMem(rw,sizeof(*rw));
	    }
	}
    }
//FIX! free NIPCBuff/etc!
    return FALSE;	// the transaction will be freed
}


/* StartRename
 *
 * ACTION_RENAME_OBJECT
 *
 */
static BOOL StartRename(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	                struct MountedFSInfoClient *m)
{

    UBYTE *gp;

    /* Either of the two given locks may be old, and invalid.  Check.  */
    /* If not a root lock, fill in the TPacket version */

    /* Src lock */
    if (dp->dp_Arg1)
    {
        struct CLock *ol;
        ol = (struct CLock *) BADDR(dp->dp_Arg1);

        if (ol->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
        {
            dp->dp_Res1 = DOSFALSE;
            dp->dp_Res2 = ERROR_INVALID_LOCK;
            return (FALSE);
        }

        tp->tp_Arg1 = (LONG) ol->CLock_ServerLock;
    }
    else
        tp->tp_Arg1 = 0L;


    /* Dst Lock */
    if (dp->dp_Arg3)
    {
        struct CLock *ol;
        ol = (struct CLock *) BADDR(dp->dp_Arg3);
        if (ol->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
        {
            dp->dp_Res1 = DOSFALSE;
            dp->dp_Res2 = ERROR_INVALID_LOCK;
            return (FALSE);
        }
        tp->tp_Arg3 = (LONG) ol->CLock_ServerLock;
    }
    else
        tp->tp_Arg3 = 0L;


    /* Fill in the "from" filepath */
    /* bump gp up to longword boundries */
    gp = &(((UBYTE *) t->trans_RequestData)[sizeof(struct TPacket)]);
    gp = TOLONGWORD(gp);

    tp->tp_Arg2 = ((ULONG) gp - (ULONG) tp);

    CopyMem(BADDR(dp->dp_Arg2),gp,((UBYTE *)BADDR(dp->dp_Arg2))[0]+1);
    FixBString(gp);

    /* Fill in the "to" filepath */
    /* bump gp up to longword boundries */
    gp = &gp[((UBYTE *)BADDR(dp->dp_Arg2))[0]+1];
    gp = TOLONGWORD(gp);

    tp->tp_Arg4 = ((ULONG) gp - (ULONG) tp);

    CopyMem(BADDR(dp->dp_Arg4),gp,((UBYTE *)BADDR(dp->dp_Arg4))[0]+1);
    FixBString(gp);
    gp = &gp[((UBYTE *)BADDR(dp->dp_Arg4))[0]+1];

    return(TRUE);

}


/* StartSetProtect
 *
 * ACTION_SET_PROTECT
 *
 */
static BOOL StartSetProtect(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	                    struct MountedFSInfoClient *m)
{

    UBYTE *gp;
    struct CLock *cl;

/* The given lock may be old */
    if (dp->dp_Arg2)
    {
        cl = (struct CLock *) BADDR(dp->dp_Arg2);
        if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
        {
            dp->dp_Res1 = DOSFALSE;
            dp->dp_Res2 = ERROR_INVALID_LOCK;
            return(FALSE);
        }
        tp->tp_Arg2 = (LONG) cl->CLock_ServerLock;
    }
    else
        tp->tp_Arg2 = 0L;

    gp = &(((UBYTE *) t->trans_RequestData)[sizeof(struct TPacket)]);
    gp = TOLONGWORD(gp);

    tp->tp_Arg3 = ((ULONG) gp - (ULONG) tp);

    CopyMem(BADDR(dp->dp_Arg3),gp,((UBYTE *)BADDR(dp->dp_Arg3))[0]+1);
    FixBString(gp);
    gp = &gp[((UBYTE *)BADDR(dp->dp_Arg3))[0]+1];

    tp->tp_Arg4 = dp->dp_Arg4;

    return(TRUE);

}

/* StartSetComment
 *
 * ACTION_SET_COMMENT
 *
 */
static BOOL StartSetComment(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	                    struct MountedFSInfoClient *m)
{

    UBYTE *gp;
    struct CLock *cl;

/* The given lock may be old */
    if (dp->dp_Arg2)
    {
        cl = (struct CLock *) BADDR(dp->dp_Arg2);
        if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
        {
            dp->dp_Res1 = DOSFALSE;
            dp->dp_Res2 = ERROR_INVALID_LOCK;
            return(FALSE);
        }
        tp->tp_Arg2 = (LONG) cl->CLock_ServerLock;
    }
    else
        tp->tp_Arg2 = 0L;

    gp = &(((UBYTE *) t->trans_RequestData)[sizeof(struct TPacket)]);
    gp = TOLONGWORD(gp);

    tp->tp_Arg3 = ((ULONG) gp - (ULONG) tp);

    CopyMem(BADDR(dp->dp_Arg3),gp,((UBYTE *)BADDR(dp->dp_Arg3))[0]+1);
    FixBString(gp);

    gp = &gp[((UBYTE *)BADDR(dp->dp_Arg3))[0]+1];
    gp = TOLONGWORD(gp);

    tp->tp_Arg4 = ((ULONG) gp - (ULONG) tp);

    CopyMem(BADDR(dp->dp_Arg4),gp,((UBYTE *)BADDR(dp->dp_Arg4))[0]+1);
//    FixBString(gp);  /* Jeeze, Greg ... Don't strip colons off of COMMENTS! */
    gp = &gp[((UBYTE *)BADDR(dp->dp_Arg4))[0]+1];

    return(TRUE);

}

static void
FreeCLock (struct CLock *cl, struct MountedFSInfoClient *m)
{
/* Like ACTION_END, while I'd really like to let people know that they're
 * trying to free an old lock, I'd also like to free the memory associated.
 * I can't do both.  So, I'm telling them "aok", and freeing the memory.
 */
            NukeLock((APTR)MKBADDR(cl),m);

	    // we were in the middle of an exnext and unlocked it...
	    // the other side has reset, so we're just dumping this lock.

	    // or...

	// we were in the middle of an exnext and unlocked it...
	// the server will wind up any exalls on the other end.  When we get this back
	// we know the exalls are done and any transactions asking for exall buffers
	// have been replied.
	// we're only called after the other end has completed the unlock, so no more
	// exall data blocks should be coming.

//kprintf("freeing lock $%lx with exnext data still attached\n",cl);

	    // replies dp if needed, sets cl->Zombie if needed
	    free_exnext_data(cl,FALSE,ERROR_NO_MORE_ENTRIES,ZOMBIE_FREE);

	    // if it's a zombie, we still have a transaction out there...
	    // if so, it'll get freed when the transaction returns.
	    if (!(cl->Zombie))
	    {
		if (cl->CLock_FullServerPath)
		    FreeMem(cl->CLock_FullServerPath,strlen(cl->CLock_FullServerPath)+1);

		FreeMem(cl,sizeof(struct CLock));
	    }
}

/* StartFreeLock
 *
 * ACTION_FREE_LOCK
 *
 */
static BOOL StartFreeLock(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	                  struct MountedFSInfoClient *m)
{

    struct CLock *cl;

/* The given lock may be old */
    if (dp->dp_Arg1)
    {
        cl = (struct CLock *) BADDR(dp->dp_Arg1);
        if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
        {
//            dp->dp_Res1 = DOSFALSE;
//            dp->dp_Res2 = ERROR_INVALID_LOCK;

	    FreeCLock(cl,m);

            dp->dp_Res1 = DOSTRUE;
            return(FALSE);
        }
        tp->tp_Arg1 = (LONG) cl->CLock_ServerLock;

	// we were in the middle of an exnext and unlocked it...
	// the other side has reset, so we're just dumping this lock.

	// or...

	// we were in the middle of an exnext and unlocked it...
	// the server will wind up any exalls on the other end.  When we get this back
	// we know the exalls are done and any transactions asking for exall buffers
	// have been replied.
	// we're only called after the other end has completed the unlock, so no more
	// exall data blocks should be coming.

	// replies dp if needed, sets cl->Zombie if needed
	free_exnext_data(cl,FALSE,ERROR_NO_MORE_ENTRIES,ZOMBIE_FREE);
    }
    else
        tp->tp_Arg1 = 0L;

    return(TRUE);

}

/*
 *  EndFreeLock
 *
 */

static void EndFreeLock(struct TPacket *tp, struct MountedFSInfoClient *m)
{
    struct DosPacket *dp=tp->tp_DosPacket;
    struct CLock *cl;
    cl = (struct CLock *) BADDR(dp->dp_Arg1);

    FreeCLock(cl,m);

    ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
}



/* StartSetDate
 *
 * ACTION_SET_DATE
 *
 */
static BOOL StartSetDate(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	                 struct MountedFSInfoClient *m)
{
    UBYTE *gp;
    struct CLock *cl;

/* The given lock may be old */
    cl = (struct CLock *) BADDR(dp->dp_Arg2);

    if (dp->dp_Arg2)
    {
        if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
        {
            dp->dp_Res1 = DOSFALSE;
            dp->dp_Res2 = ERROR_INVALID_LOCK;
            return(FALSE);
        }
        tp->tp_Arg2 = (LONG) cl->CLock_ServerLock;
    }
    else
        tp->tp_Arg2 = 0L;

    gp = &(((UBYTE *) t->trans_RequestData)[sizeof(struct TPacket)]);
    gp = TOLONGWORD(gp);

    tp->tp_Arg3 = ((ULONG) gp - (ULONG) tp);

    CopyMem(BADDR(dp->dp_Arg3),gp,((UBYTE *)BADDR(dp->dp_Arg3))[0]+1);
    FixBString(gp);

    gp = &gp[((UBYTE *)BADDR(dp->dp_Arg3))[0]+1];
    gp = TOLONGWORD(gp);

    tp->tp_Arg4 = ((ULONG) gp - (ULONG) tp);
    CopyMem((APTR)dp->dp_Arg4,gp,sizeof(struct DateStamp));

    gp = &gp[sizeof(struct DateStamp)];

    return(TRUE);

}


/* StartParent, StartCopyDir, StartExAllEnd
 *
 * ACTION_PARENT
 *
 */
static BOOL StartParent(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	                struct MountedFSInfoClient *m)
{
    struct CLock *cl;

/* The given lock may be old */
    cl = (struct CLock *) BADDR(dp->dp_Arg1);

    if (cl)
    {
        if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
        {
            dp->dp_Res1 = DOSFALSE;
            dp->dp_Res2 = ERROR_INVALID_LOCK;
            return(FALSE);
        }
        tp->tp_Arg1 = (LONG) cl->CLock_ServerLock;
    }
    else
        tp->tp_Arg1 = 0L;

    return(TRUE);

}

/* StartParentFH
 *
 * ACTION_PARENT_FH
 *
 */
static BOOL StartParentFH(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	                  struct MountedFSInfoClient *m)
{
    struct CFH *cf;

    cf = (struct CFH *) dp->dp_Arg1;

    /* The possibility of an old, unreincarnated fh exists. */

    // checks for out-of-date fh, AND queues packet if a read/write is active
    if (!ValidateFH(cf,dp,m))
	return FALSE;

    tp->tp_Arg1 = (LONG)cf->CFH_ServerFH;

    return(TRUE);

}


/*
 * EndParent, EndCopyDir, EndLocateObject, EndCreateDir
 *
 */

static BOOL EndParent(struct Transaction *t, struct TPacket *tp, struct MountedFSInfoClient *m)
{
    struct DosPacket *dp=tp->tp_DosPacket;
    struct CLock *cl;

    if (tp->tp_Res1)        /* Successful! */
    {
	// handle delayed resolution of locks on remote root directories
	// tp_Arg5 is a flag
	if (tp->tp_Type == ACTION_LOCATE_OBJECT && tp->tp_Arg5)
	{
		cl = BADDR(dp->dp_Arg1);
		cl->CLock_ServerLock = (struct FileLock *) tp->tp_Res1;

		// get tp set up again
		tp->tp_Type = dp->dp_Action;

		// the rest will get re-setup
		// restart the action which caused all this to start
		if (dp->dp_Action == ACTION_EXAMINE_ALL)
			return StartExAll(t,tp,dp,m);
		else // ACTION_EXAMINE
			return StartExamine(t,tp,dp,m);
	}

        cl = (struct CLock *) AllocMem(sizeof(struct CLock),MEMF_CLEAR|MEMF_PUBLIC);
        if (!cl)
        {
            ReplyPkt(dp,DOSFALSE,ERROR_NO_FREE_STORE);
            return FALSE;
        }
        cl->CLock_ConnectionNumber = m->mfsi_ConnectionNumber;
        cl->CLock_ServerLock = (struct FileLock *) tp->tp_Res1;
        cl->CLock_FileLock.fl_Key = tp->tp_Res1;
        if (dp->dp_Type == ACTION_LOCATE_OBJECT)
            cl->CLock_FileLock.fl_Access = dp->dp_Arg3;
        else
        {
            if ( (dp->dp_Arg1) && ( (dp->dp_Type != ACTION_COPY_DIR_FH) && (dp->dp_Type != ACTION_PARENT_FH) ) )
            {
                cl->CLock_FileLock.fl_Access = ((struct CLock *) BADDR(dp->dp_Arg1))->CLock_FileLock.fl_Access;
            }
            else
                cl->CLock_FileLock.fl_Access = SHARED_LOCK;
        }
        cl->CLock_FileLock.fl_Volume = MKBADDR(m->mfsi_Volume);
        cl->CLock_FileLock.fl_Task = m->mfsi_LocalPort;
        dp->dp_Res1 = (LONG) MKBADDR(cl);
        SaveName(cl,tp);
        KeepLock((APTR)dp->dp_Res1,m);

        ReplyPkt(dp,dp->dp_Res1,dp->dp_Res2);
        return FALSE;
    }
    else                    /* Not successful, or Parent reached root */
    {
        ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
    }
    return FALSE;
}


/* StartDeleteObject
 *
 * ACTION_DELETE_OBJECT
 *
 */
static BOOL StartDeleteObject(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	                      struct MountedFSInfoClient *m)
{
    UBYTE *gp;
    struct CLock *cl;

/* The given lock may be old */
    cl = (struct CLock *) BADDR(dp->dp_Arg1);

    if (dp->dp_Arg1)
    {
        if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
        {
            dp->dp_Res1 = DOSFALSE;
            dp->dp_Res2 = ERROR_INVALID_LOCK;
            return(FALSE);
        }
        tp->tp_Arg1 = (LONG) cl->CLock_ServerLock;
    }
    else
        tp->tp_Arg1 = 0L;

    gp = &(((UBYTE *) t->trans_RequestData)[sizeof(struct TPacket)]);
    gp = TOLONGWORD(gp);

    tp->tp_Arg2 = ((ULONG) gp - (ULONG) tp);

    CopyMem(BADDR(dp->dp_Arg2),gp,((UBYTE *)BADDR(dp->dp_Arg2))[0]+1);
    FixBString(gp);
    gp = &gp[((UBYTE *)BADDR(dp->dp_Arg2))[0]+1];

    return(TRUE);

}



/* StartFakeLocateObject
 *
 * ACTION_LOCATE_OBJECT
 *
 *	Done to resolve locks that were left as NULL serverlocks for performance.
 *	Send a fake packet to get the lock, then do the original action again.
 *
 */
static BOOL StartFakeLocateObject(struct Transaction *t, struct TPacket *tp,
				   struct DosPacket *dp, struct MountedFSInfoClient *m)
{
	UBYTE *gp;
	struct CLock *cl;

	// we know cl is non-null before we're called
	cl = (struct CLock *) BADDR(dp->dp_Arg1);
        if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
        {
            dp->dp_Res1 = DOSFALSE;
            dp->dp_Res2 = ERROR_INVALID_LOCK;
            return(FALSE);
        }

	// ok, we need to make ServerLock real.  Basically, we need to do a LOCATE_OBJECT
	// and after that is done, resume doing this.

	gp = &(((UBYTE *) t->trans_RequestData)[sizeof(struct TPacket)]);
	gp = TOLONGWORD(gp);
	*gp = '\0';					// "" BSTR
	gp += 1;

	tp->tp_Type = ACTION_LOCATE_OBJECT;
	tp->tp_Arg1 = NULL;			// root of the mounted volume
	tp->tp_Arg2 = ((ULONG) gp - (ULONG) tp);
	tp->tp_Arg3 = SHARED_LOCK;
	tp->tp_Arg4 = m->mfsi_Flags & FLAGSMASK;
	tp->tp_Arg5 = TRUE;			// this is a fake locate object
	// tp_DosPacket already set

	t->trans_ReqDataActual = ((ULONG)gp - (ULONG)tp);

	return(TRUE);
}


/* StartLocateObject
 *
 * ACTION_LOCATE_OBJECT
 *
 */
static BOOL StartLocateObject(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	                      struct MountedFSInfoClient *m)
{
    UBYTE *gp;
    struct CLock *cl;
    int slen;

/* The given lock may be old */
    cl = (struct CLock *) BADDR(dp->dp_Arg1);

    if (dp->dp_Arg1)
    {
        if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
        {
            dp->dp_Res1 = DOSFALSE;
            dp->dp_Res2 = ERROR_INVALID_LOCK;
            return(FALSE);
        }
        tp->tp_Arg1 = (LONG) cl->CLock_ServerLock;
    }
    else
        tp->tp_Arg1 = 0L;

    gp = &(((UBYTE *) t->trans_RequestData)[sizeof(struct TPacket)]);
    gp = TOLONGWORD(gp);

    tp->tp_Arg2 = ((ULONG) gp - (ULONG) tp);

    CopyMem(BADDR(dp->dp_Arg2),gp,((UBYTE *)BADDR(dp->dp_Arg2))[0]+1);
    FixBString(gp);
    slen = gp[0];
    gp += gp[0]+1;

    tp->tp_Arg3 = dp->dp_Arg3;
    tp->tp_Arg4 = m->mfsi_Flags & FLAGSMASK;
    tp->tp_Arg5 = FALSE;			// this is a real locate object

    t->trans_ReqDataActual = ((ULONG)gp - (ULONG)tp);

/* Special casing for Mike Sinz */
/* If the caller is attempting to get a lock on the root of a
 * volume, indicated by a null string length (after the : and
 * previous is removed), and null reference lock, create a
 * lock structure for them immediately, and don't bother to
 * contact the server.
 */

// This helps performance in that WB does a Lock()/SameLock() on each
// volume until it gets a match whenever a PutDiskObject() is done.

    if ((!cl) && (!slen) && (dp->dp_Arg3 == SHARED_LOCK))
    {
        struct CLock *ncl;
        ncl = (struct CLock *) AllocMem(sizeof(struct CLock),MEMF_CLEAR|MEMF_PUBLIC);
        if (ncl)
        {
            ncl->CLock_ConnectionNumber = m->mfsi_ConnectionNumber;
            ncl->CLock_ServerLock = 0L;
            ncl->CLock_FileLock.fl_Access = SHARED_LOCK;

            ncl->CLock_FileLock.fl_Volume = MKBADDR(m->mfsi_Volume);
            ncl->CLock_FileLock.fl_Task = m->mfsi_LocalPort;
            dp->dp_Res1 = (LONG) MKBADDR(ncl);
            ncl->CLock_FullServerPath = AllocMem(1,MEMF_PUBLIC);
            if (!ncl->CLock_FullServerPath)
            {
                FreeMem(ncl,sizeof(struct CLock));
                dp->dp_Res1 = FALSE;
                dp->dp_Res2 = ERROR_NO_FREE_STORE;
            }
            else
            {
                ((STRPTR)ncl->CLock_FullServerPath)[0]=0;
                KeepLock((APTR)dp->dp_Res1,m);

                return(FALSE);
            }
        }
    }

    return(TRUE);
}


// Free any exnext-related data associated with the lock
static void
free_exnext_data (struct CLock *cl, ULONG res1, ULONG res2, UBYTE zombie_type)
{
	struct ExNextBlock *block;
	struct ExNextData  *edata = cl->ExNextData;
	BOOL running = FALSE;

	// nothing to do if we're not exalling, or if we've already done this.
	if (!edata)
		return;
	if (cl->Zombie)
	{
//kprintf("free_exnext_data on Zombie! $%lx\n",cl);
		// if the new zombie type requires a free of the clock, upgrade it.
		if (zombie_type == ZOMBIE_FREE)
			cl->Zombie = zombie_type;
		return;
	}

	FreeVec(edata->CurrentBlock);
	edata->CurrentBlock = NULL;

	while (block = (struct ExNextBlock *) RemHead((struct List *) &(edata->BlockList)))
	{
		running |= (!(block->Available) & !(block->Error));
		FreeVec(block);
	}

	if (edata->WaitingPacket)
	{
		ReplyPkt(edata->WaitingPacket,res1,res2);
		edata->WaitingPacket = NULL;
	}

	if (!running)
	{
		// the exall had finished already, free everything
		if (edata->FakePacket)
			FreeDosObject(DOS_STDPKT,edata->FakePacket);

		FreeVec(edata);

		cl->ExNextData = NULL;
	} else {
//kprintf("made $%lx into Zombie (%ld)\n",cl,zombie_type);
		// we have outstanding exall transactions.  Wait for them to finish
		// or error out.  Mark cl as a Zombie, so we know to free it.
		// Zombie is 0 (normal), 1 (free when possible), or 2 (free exnext
		// data only when possible, lock still active).
		cl->Zombie = zombie_type;
	}
}

static void
FreeZombie (struct CLock *cl)
{
	struct ExNextData  *edata = cl->ExNextData;

	// the exall had finished already, free everything
	if (edata->FakePacket)
		FreeDosObject(DOS_STDPKT,edata->FakePacket);

	// Since we restart NextExnext packets on a returning exall packet, this
	// must be some more catastrophic error.  We'll just return it with an
	// error.
	if (edata->NextExnext)
	{
//kprintf("returning ExNext $%lx for lock $%lx - failed\n",edata->NextExnext,cl);
		ReplyPkt(edata->NextExnext,FALSE,ERROR_OBJECT_NOT_FOUND);	// kinda random
	}

	FreeVec(edata);
	cl->ExNextData = NULL;

	if (cl->Zombie == ZOMBIE_FREE)
		FreeMem(cl,sizeof(*cl));

	cl->Zombie = 0;	// not a zombie any more
}

static BOOL StartExAll(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	               struct MountedFSInfoClient *m)
{
  { // so we can override *t...
    struct CLock *cl = BADDR(dp->dp_Arg1);
    ULONG rttags[5]={TRN_AllocReqBuffer,sizeof(struct TPacket),TRN_AllocRespBuffer,0,TAG_DONE};
    struct Transaction *t2;
    struct ExAllControl *ec = (void *) dp->dp_Arg5;

    // envoy returns a null server lock on LOCATE_OBJECT of the root with a NULL lock.
    // All well and good, but this causes a problem with exall, since it needs a real
    // lock.  Probably on a NULL server lock we should return action not known, and then
    // dos will emulate it with exnext (slowly).
    if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
    {
       	dp->dp_Res1 = DOSFALSE;
        dp->dp_Res2 = ERROR_INVALID_LOCK;
        return(FALSE);
    }
    if (!(cl->CLock_ServerLock))		// handle evil internal performance hack
    {
	// this will send an ACTION_LOCATE_OBJECT, lock the root, and then call us again
	return (StartFakeLocateObject(t,tp,dp,m));
    }

    // this code basically cloned from Read
    rttags[1]=sizeof(struct TPacket);
    rttags[3]=sizeof(struct TPacket) + dp->dp_Arg3;
    if (ec->eac_MatchString)
	rttags[1] += strlen(ec->eac_MatchString) + 1;

    t2 = AllocTransactionA((struct TagItem *)&rttags);
    if (t2)
    {
	    struct TPacket *tp;

	    t2->trans_ReqDataActual = t2->trans_ReqDataLength;
	    t2->trans_Timeout = PACKETTIMEOUT+m->mfsi_DynamicDelay;
	    t2->trans_Command = FSCMD_DOSPACKET;
	    t2->trans_ClientPrivate = (m->mfsi_ConnectionNumber & CONNECTIONMASK);

	    tp = (struct TPacket *) t2->trans_RequestData;
	    tp->tp_ServerMFSI = m->mfsi_ServerMFSI;
	    tp->tp_DosPacket = dp;
	    tp->tp_Type = dp->dp_Type;

	    tp->tp_Arg1 = (LONG) cl->CLock_ServerLock;
	    tp->tp_Arg2 = sizeof(struct TPacket);  // offset to buffer
	    tp->tp_Arg3 = dp->dp_Arg3;		   // size
	    tp->tp_Arg4 = dp->dp_Arg4;		   // ED_xxx
	    tp->tp_Arg5 = (m->mfsi_Flags & FLAGSMASK); // eac_entries coming back
	    tp->tp_Arg6 = 0;			   // offset to matchstring or NULL

	    if (ec->eac_MatchString)
	    {
		tp->tp_Arg6 = sizeof(struct TPacket);	// pointer to parsepattern if any
		strcpy((char *) (((ULONG) tp) + tp->tp_Arg6),ec->eac_MatchString);
	    }

//kprintf("sending exall $%lx, $%lx, $%lx, %ld, $%lx, '%s'\n",tp->tp_Arg1,tp->tp_Arg2,tp->tp_Arg3,
//tp->tp_Arg4,tp->tp_Arg5,tp->tp_Arg6 ? ((ULONG) tp) + tp->tp_Arg6 : "");

	    BeginTransaction(m->mfsi_Destination,m->mfsi_Source,t2);
    } else {
	    dp->dp_Res1 = DOSTRUE;
	    dp->dp_Res2 = ERROR_NO_FREE_STORE;
	    return(FALSE);
    }

    return(TRUE);
  }
}

/* size of fixed info for different exall_data sizes */
static UBYTE exall_size[] = {
	8,12,16,20,32,36,40, 44,48,52  // last 3 are assumptions, just in case
};

static void
EndExAll (struct TPacket *tp, struct MountedFSInfoClient *m)
{
	struct DosPacket *dp = tp->tp_DosPacket;
	struct ExAllData *ed,*prev_ed;
	struct ExAllControl *ec = (void *) dp->dp_Arg5;
	UBYTE *dest;
	ULONG offset,i;

//kprintf("Exall returned, %ld:%ld\n",tp->tp_Res1,tp->tp_Res2);
	// mark block of data as available, update the buffer in the block
	if (tp->tp_Res1 || tp->tp_Res2 == ERROR_NO_MORE_ENTRIES)	// successful
	{
		// copy exall data to final buffer
		dest = (UBYTE *) dp->dp_Arg2;
		CopyMem(&((UBYTE *)tp)[sizeof(struct TPacket)],dest,dp->dp_Arg3);

		// fix the ed_Next, ed_Name, ed_Comment pointers.
		// 0 is NULL, not the beginning of the buffer
		// tp_Arg5 is flags going out, eac_Entries coming back...
		offset = (ULONG) dest;
		ed = (struct ExAllData *) dest;
		ec->eac_Entries = tp->tp_Arg5;

		for (i = 0; i < tp->tp_Arg5; i++)
		{
//kprintf("$%lx: next %ld, name %ld, comm %ld\n",ed,ed->ed_Next,ed->ed_Name,ed->ed_Comment);
			if (ed->ed_Next)
			    ed->ed_Next	= (void *) (((ULONG) ed->ed_Next) + offset);
			ed->ed_Name	= (void *) (((ULONG) ed->ed_Name) + offset);

			if (dp->dp_Arg4 >= ED_COMMENT && ed->ed_Comment)
			    ed->ed_Comment = (void *) (((ULONG) ed->ed_Comment) + offset);
//kprintf("$%lx:\t%s\n",ed,ed->ed_Name);

			ed = ed->ed_Next;
		}

		// ugh.  Now that we have the entries, we must allow him to sniff them if he wants
		if (ec->eac_MatchFunc)
		{
			ed = (struct ExAllData *) dest;
			prev_ed = NULL;
			for (i = 0; i < tp->tp_Arg5; i++)
			{
//kprintf("domatch: %s\n",ed->ed_Name);
				if (!do_match(ec,dp->dp_Arg4,ed))
				{
//kprintf("rejected\n");
					ec->eac_Entries--;

					// rejected.  remove it
					if (ed != (struct ExAllData *) dest)
					{
						prev_ed->ed_Next = ed->ed_Next;
						ed = ed->ed_Next;
					} else if (ed->ed_Next) {
						// must move down an entry, argh
						CopyMem(ed->ed_Next,ed,exall_size[dp->dp_Arg4]);
						// don't touch ed
					}
				} else
					ed = ed->ed_Next;
			}
		}
	}
	ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
}

static void
fill_fib (struct DosPacket *dp, struct ExAllData *ed, struct ExNextBlock *block)
{
	struct FileInfoBlock *fib = BADDR(dp->dp_Arg2);

//kprintf("filling fib from $%lx with %s, type %ld\n",ed,ed->ed_Name,block->Type);
	CtoBcpy(fib->fib_FileName,ed->ed_Name);
	if (ed->ed_Comment)
		CtoBcpy(fib->fib_Comment,ed->ed_Comment);
	else
		fib->fib_Comment[0] = '\0';

	fib->fib_EntryType 	= 
	fib->fib_DirEntryType	= ed->ed_Type;
	fib->fib_Size		= ed->ed_Size;
	fib->fib_Protection	= ed->ed_Prot;
	fib->fib_Date.ds_Days	= ed->ed_Days;
	fib->fib_Date.ds_Minute	= ed->ed_Mins;
	fib->fib_Date.ds_Tick	= ed->ed_Ticks;
	if (block->Type >= ED_OWNER)
	{
		// V37 filesystems/dos won't give us owner info
		fib->fib_OwnerUID	= ed->ed_OwnerUID;
		fib->fib_OwnerGID	= ed->ed_OwnerGID;
	}
	fib->fib_DiskKey	= 0;			// FIX!!!!!!!
	// that will break some programs!
	fib->fib_NumBlocks	= ((ed->ed_Size + 511) >> 9) + 1;
				  // divide by 512 rounded up + 1 for hdr

	dp->dp_Res1 = DOSTRUE;
	dp->dp_Res2 = 0;
}

// most of these parameters are in case we want to change how the caching is done
// internally.

static BOOL
next_entry (struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	    struct CLock *cl, struct MountedFSInfoClient *m)
{
	struct ExNextData *edata = cl->ExNextData;

//kprintf("next_entry for $%lx: edata $%lx\n",cl,edata);
	if ((edata->CurrentEntry = edata->CurrentEntry->ed_Next) != NULL)
	{
		fill_fib(dp,edata->CurrentEntry,edata->CurrentBlock);

	} else {
//kprintf("	exall block $%lx empty\n",edata->CurrentBlock);
		// current block is empty.  free it.  If another block is coming, wait.
		FreeVec(edata->CurrentBlock);
	
		if (edata->CurrentBlock = (void *)
					  RemHead((struct List *) &(edata->BlockList)))
		{
			if (edata->CurrentBlock->Available)
			{
				edata->CurrentEntry = (void *) edata->CurrentBlock->buffer;
				fill_fib(dp,edata->CurrentEntry,edata->CurrentBlock);
			} else {
				// FIX! error if somone already waiting!!!!!!
				if (edata->CurrentBlock->Error)
				{
					dp->dp_Res1 = FALSE;
					dp->dp_Res2 = edata->CurrentBlock->Res2;
					// won't be a zombie because nothing is outstanding
					free_exnext_data(cl,FALSE,edata->CurrentBlock->Res2,ZOMBIE_NOFREE);
				}
//if (edata->WaitingPacket)
//kprintf("illegal, can't wait for $%lx, already waiting for $%lx\n",dp,edata->WaitingPacket);
//else
				edata->WaitingPacket = dp;
			}
		} else {
//kprintf("	no more blocks, all done\n");
			dp->dp_Res1 = FALSE;
			dp->dp_Res2 = ERROR_NO_MORE_ENTRIES;
			free_exnext_data(cl,FALSE,ERROR_NO_MORE_ENTRIES,ZOMBIE_NOFREE);
			// will try to free currentblock/blocklist
		}
	}
	return FALSE;	// since we never want to start a transaction
}

static void
send_exnext_request (struct Transaction *t, struct TPacket *tp,
		     struct CLock *cl, struct MountedFSInfoClient *m)
{
	UBYTE *gp;

//kprintf("sending exall request for $%lx\n",cl);
	gp = &(((UBYTE *) t->trans_RequestData)[sizeof(struct TPacket)]);
	gp = TOLONGWORD(gp);

	// ENVOY_EXAMINE_ALL is an internal packet to the server, never seen outside
	// exallcontrol struct is attached to the lock at the server - we might keep
	// it here for statelessness, but that could cause problems, since we aren't
	// supposed to muddle with it ever.

	tp->tp_Type = ACTION_ENVOY_EXAMINE_ALL;
        tp->tp_Arg1 = (LONG) cl->CLock_ServerLock;
	tp->tp_Arg2 = ((ULONG) gp - (ULONG) tp);	// offset of arg buffer
	tp->tp_Arg3 = TRANS_SIZE;
	tp->tp_Arg4 = ED_OWNER;
	tp->tp_Arg5 = (m->mfsi_Flags & FLAGSMASK);
	tp->tp_Arg6 = 0;				// no matchstring
	tp->tp_DosPacket = cl->ExNextData->FakePacket;	// so the bloody retry will work
}

static BOOL
start_exnext_data (struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
		   struct CLock *cl, struct MountedFSInfoClient *m)
{
	struct ExNextData *edata;
	struct ExNextBlock *block = NULL;
	struct DosPacket *newdp;

	// the lock has been examined, get (first) exall data for it
	if (!(edata = AllocVec(sizeof(*edata),MEMF_CLEAR)))
		goto cleanup;
	if (!(block = AllocVec(sizeof(*block),0)))
		goto cleanup;
	if (!(newdp = AllocDosObject(DOS_STDPKT,NULL)))
		goto cleanup;
	
	newdp->dp_Action = ACTION_ENVOY_EXAMINE_ALL;
	newdp->dp_Arg1   = dp->dp_Arg1;		// points to cl
//newdp->dp_Port = (void *) -1L;		// for debugging, make sure it's never replied

	block->Available = FALSE;

	edata->FakePacket = newdp;
	edata->CurrentBlock = block;
	edata->WaitingPacket = dp;
	NewList((struct List *) &(edata->BlockList));

	cl->ExNextData = edata;

	// send packet to server asking for first batch of exall data
	send_exnext_request(t,tp,cl,m);

	return TRUE;	// send the transaction

cleanup:
	FreeVec(block);	// NULL is ok
	FreeVec(edata); // cl->ExNextData never set

	dp->dp_Res1 = FALSE;
	dp->dp_Res2 = ERROR_NO_FREE_STORE;
	return FALSE;
}

static BOOL
end_exnext_data (struct Transaction *t, struct TPacket *tp, struct MountedFSInfoClient *m)
{
	struct DosPacket *dp = tp->tp_DosPacket;
	struct CLock *cl;
	struct ExNextData  *edata;
	struct ExAllData   *ed;
	struct ExNextBlock *block;
        UBYTE *efrom;
	ULONG offset,i;

	// NOTE! dp is a fake packet!  Not real!  it's merely there to make retry/etc
	// work.  dp_Arg1 is the only valid field.

	cl = (struct CLock *) BADDR(dp->dp_Arg1);
	edata = cl->ExNextData;

	if (cl->Zombie)
	{
		// after we sent this request, we unlocked the lock.  All we need to do is clean up.
		// OR we did another examine.

		// The server will get cleaned up on the next Examine or on UnLock (RemExAll)

//kprintf("exall data back for Zombie $%lx (%ld)\n",cl,cl->Zombie);
		// Restart NextExnext packets on a returning exall packet.
		// We just throw them into the front of the packet queue and let
		// them re-execute now that cl is no longer a Zombie.
		if (edata->NextExnext)
		{
//kprintf("restarting ExNext $%lx for lock $%lx\n",edata->NextExnext,cl);
			AddHead(&(m->mfsi_LocalPort->mp_MsgList),
				&(edata->NextExnext->dp_Link->mn_Node));
			edata->NextExnext = NULL;
		}

		FreeZombie(cl);
		return FALSE;
	}

//kprintf("end_exnext_data: %ld, %ld\n",tp->tp_Res1,tp->tp_Res2);

	// FIX!? do I need to check CLock_ConnectionNumber?
	if (!edata)
		return FALSE;	// FIX!  is this needed?  do we need to check cl for validity?

	if (!(edata->CurrentBlock->Available))
		block = edata->CurrentBlock;
	else {
		for (block = (void *) edata->BlockList.mlh_Head;
		     block->Node.mln_Succ;
		     block = (void *) block->Node.mln_Succ)
		{
			if (!(block->Available))
				break;
		}
		if (!(block->Node.mln_Succ))
		{
//kprintf("couldn't find any empty blocks!\n");
			return FALSE;	/* must have come through a time warp... */
		}
	}

//kprintf("found block for data $%lx, type %ld, number %ld\n",block,tp->tp_Arg4,tp->tp_Arg5);
	// mark block of data as available, update the buffer in the block
	if (tp->tp_Res1 || tp->tp_Res2 == ERROR_NO_MORE_ENTRIES)	// successful
	{
		// in case the server filesystem doesn't support ED_OWNER...
		block->Type = tp->tp_Arg4;
		block->Available = TRUE;

	        efrom = &((UBYTE *)tp)[sizeof(struct TPacket)];
		efrom = TOLONGWORD(efrom);
	        CopyMem(efrom,block->buffer,TRANS_SIZE);

		// fix the ed_Next, ed_Name, ed_Comment pointers.
		// 0 is NULL, not the beginning of the buffer
		// tp_Arg5 is flags going out, eac_Entries coming back...
		offset = (ULONG) block->buffer;
		ed = (struct ExAllData *) block->buffer;
		for (i = 0; i < tp->tp_Arg5; i++)
		{
//kprintf("$%lx: next %ld, name %ld, comm %ld($%02lx)\n",
//ed,ed->ed_Next,ed->ed_Name,ed->ed_Comment,*((UBYTE *) offset + (ULONG) ed->ed_Comment));
			if (ed->ed_Next)
			    ed->ed_Next	= (void *) (((ULONG) ed->ed_Next) + offset);
			if (ed->ed_Comment)
			    ed->ed_Comment	= (void *) (((ULONG) ed->ed_Comment) + offset);
			ed->ed_Name	= (void *) (((ULONG) ed->ed_Name) + offset);
//kprintf("$%lx:\t%s (%s)\n",ed,ed->ed_Name,ed->ed_Comment);

			ed = ed->ed_Next;
		}
	} else {
//kprintf("exall failed! %ld, %ld\n",tp->tp_Res1,tp->tp_Res2);
		// exall failed.  mark somehow and return waiting packet if any with
		// error!
		block->Error = TRUE;
		block->Res2 = tp->tp_Res2;

		if (dp = edata->WaitingPacket)
		{
			edata->WaitingPacket = NULL;
			ReplyPkt(dp,FALSE,tp->tp_Res2);
		}
		return FALSE;
	}

	// if packet is waiting, fill it and return it.
	// Note: we set currententry here, since that's the only case where it won't
	// be set correctly already.  next_entry moved the block to CurrentBlock when
	// it left dp in WaitingPacket.
	if (tp->tp_Arg5 == 0)
	{
//kprintf("no entries...\n");
		// server returned 0 entries.  Wierd, but within spec.  We're done with
		// the buffer.  Free it here.
		// If it's the CurrentBlock, then when next_entry is called it will get freed.
		// We could have taught next_entry how to deal with empty blocks instead.
		if (block == edata->CurrentBlock)
		{
			edata->CurrentEntry = (void *) block->buffer;
			edata->CurrentEntry->ed_Next = NULL;	// make it look empty!
		} else {
			Remove((struct Node *) &(block->Node));
			FreeVec(block);
		}
	}

	// Don't check for waitingpacket if there weren't any entries...  unless this is
	// the end.
	if ((dp = edata->WaitingPacket) != NULL &&
	    (tp->tp_Arg5 != 0 || !tp->tp_Res1))
	{
		edata->WaitingPacket = NULL;

		// is there any data in the buffer?
		if (tp->tp_Arg5)
		{
			edata->CurrentEntry  = (void *) edata->CurrentBlock->buffer;
			fill_fib(dp,edata->CurrentEntry,edata->CurrentBlock);
		} else {
			// the last block must have had no entries, AND we were waiting.
			// CurrentBlock must be freed
			FreeVec(edata->CurrentBlock);
			edata->CurrentBlock = NULL;	// paranoia

			dp->dp_Res1 = FALSE;
			dp->dp_Res2 = ERROR_NO_MORE_ENTRIES;
		}
//kprintf("returning exnext $%lx, current = $%lx\n",dp,edata->CurrentEntry);
		// since it won't reply a packet for us here..
		ReplyPkt(dp,dp->dp_Res1,dp->dp_Res2);
	}

	// if not last batch of exall data, send another packet to server asking for more
	// NOTE: the fact that the next block has been requested is a flag that there's
	// more to come to next_entry().
	if (tp->tp_Res1)
	{
		struct ExNextBlock *block;

//kprintf("more blocks needed\n");
		if (!(block = AllocVec(sizeof(*block),0)))
		{
			// exall failed.  mark somehow and return waiting packet if any with
			// error!  No...
			// when we exnext, we'll find no more blocks, and assume we were done exalling.
			// eventually we'll unlock the lock or reuse it.
			return FALSE;
		}
		block->Available = FALSE;
		block->Error = 0;
		if (edata->CurrentBlock)
			AddTail((struct List *) &(edata->BlockList),(struct Node *) &(block->Node));
		else {
			edata->CurrentBlock = block;
			edata->CurrentEntry = (void *) block->buffer;
		}
		// end_exnext_data stuffs the data into the first free block
		send_exnext_request(t,tp,cl,m);

		// we'll be returning TRUE;
	}
	return (BOOL) (tp->tp_Res1 != 0);	// TRUE means resend, FALSE means no more data to fetch
}

// this only gets entered when an exall packet is retried via a fake packet.
// dp is the fake packet here.  Don't reply it!  transaction response will go to end_exnext_data.

static BOOL StartFakeExamine(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	                     struct MountedFSInfoClient *m)
{
    struct CLock *cl;

/* The given lock may be old */
    cl = (struct CLock *) BADDR(dp->dp_Arg1);

	// FIX! is this needed?
//    if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
//    {
//	dp->dp_Res1 = DOSFALSE;
//	dp->dp_Res2 = ERROR_INVALID_LOCK;
//	return(FALSE);
//    }

    // if the lock has been unlocked, instead of retrying we should make sure we free it
    if (cl->Zombie)
    {
	FreeZombie(cl);
	return FALSE;
    }

    // resend the exnext request...
    send_exnext_request(t,tp,cl,m);

    return TRUE;
}

/* StartExamine
 *
 * ACTION_Examine, ExNext, ExamineFH
 *
 */
static BOOL StartExamine(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	                 struct MountedFSInfoClient *m)
{
    UBYTE *gp;
    struct CLock *cl;
    struct CFH *cf;

/* The given lock may be old */
    if (dp->dp_Type != ACTION_EXAMINE_FH)
    {
        cl = (struct CLock *) BADDR(dp->dp_Arg1);

	// we sometimes create root locks with NULL for serverlock.  This screws up the
	// server to no end since ExAll needs to hang off a real lock.  By doing this here,
	// we do skip the connection check, but with a NULL lock it's irrelevant anyways.
	// we also don't set cl->Examined, but with a NULL lock we don't care.  Fall through
	// and send an Examine/exnext packet.
        if (cl && cl->CLock_ServerLock != NULL)
        {
	    if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
	    {
        	dp->dp_Res1 = DOSFALSE;
	        dp->dp_Res2 = ERROR_INVALID_LOCK;
	        return(FALSE);
	    }

	    if (dp->dp_Action == ACTION_EXAMINE_NEXT)
	    {
		if (cl->ExNextData)
		{
		  if (cl->Zombie)
		  {
			// Hmm.  He managed to stop a previous exnext, examine and exnext again
			// before the first exnext sequence got cleared. by a returning 
			// ENVOY_EXAMINE_ALL.  We need to wait for that to come back, and _then_
			// kick off the exnext.
//kprintf("$%lx needs to wait for zombie to clear, lock $%lx\n",dp,cl);
			cl->ExNextData->NextExnext = dp;
			return FALSE;	// code at DoStart() call will avoid sending dp back...
		  }
		  if (cl->ExNextData->WaitingPacket)
		  {
			// This is illegal (queuing exnexts).  The reason
			// is that the key field wouldn't be copied to following exnexts.
			dp->dp_Res1 = FALSE;
			dp->dp_Res2 = ERROR_OBJECT_IN_USE;
			return FALSE;
//kprintf("dp = $%lx, edata->waitingpacket = $%lx\n",dp,cl->ExNextData->WaitingPacket);
		  }

		  // edata has the exnext data for this lock
		  // replies packet now or later if data isn't available yet
		  return next_entry(t,tp,dp,cl,m);

		} else {
		  // either this is the first exnext, or someone kept calling it
		  // after no_more_entries
		  if (cl->Examined)
		  {
//kprintf("first exnext for $%lx\n",cl);
			cl->Examined = FALSE;
			return start_exnext_data(t,tp,dp,cl,m);
		  } else {
//kprintf("too many exnexts ($%lx)\n",cl);
			dp->dp_Res1 = FALSE;
			dp->dp_Res2 = ERROR_NO_MORE_ENTRIES;
			return FALSE;
		  }
		}

	    } else {
		free_exnext_data(cl,FALSE,FALSE,ZOMBIE_NOFREE);	// in case he had been exnexting

		cl->Examined = TRUE;
		tp->tp_Arg1 = (LONG) cl->CLock_ServerLock;
	    }
	} else {
	    if (dp->dp_Type == ACTION_EXAMINE_OBJECT && cl && 
		!(cl->CLock_ServerLock))		// handle evil internal performance hack
	    {
		// this will send an ACTION_LOCATE_OBJECT, lock the root, and then call us again
		return (StartFakeLocateObject(t,tp,dp,m));
	    }
	    tp->tp_Arg1 = 0L;
	}
    }
    else
    {
	// ExamineFH

        cf = (struct CFH *) dp->dp_Arg1;

        /* The possibility of an old, unreincarnated fh exists. */

        // checks for out-of-date fh, AND queues packet if a read/write is active
        if (!ValidateFH(cf,dp,m))
	    return FALSE;

        tp->tp_Arg1 = (LONG)cf->CFH_ServerFH;

    }


    gp = &(((UBYTE *) t->trans_RequestData)[sizeof(struct TPacket)]);
    gp = TOLONGWORD(gp);

    tp->tp_Arg2 = ((ULONG) gp - (ULONG) tp);
    tp->tp_Arg4 = (m->mfsi_Flags & FLAGSMASK);

    // no need to send full fib for examine_fh (or examine, for that matter)
    // however, that would require setting the ReqDataActual and RespDataActual differently,
    // among other things.

    // we can still end up here for exnext on NULL locks.
    if (dp->dp_Action == ACTION_EXAMINE_NEXT)
	CopyMem(BADDR(dp->dp_Arg2),gp,sizeof(struct FileInfoBlock));

    t->trans_ReqDataActual =  (ULONG)(&gp[sizeof(struct FileInfoBlock)]) - (ULONG) tp;

    return(TRUE);
}


/*
 *  EndExamine
 *
 */

static void EndExamine(struct TPacket *tp, struct MountedFSInfoClient *m)
{
    struct DosPacket *dp=tp->tp_DosPacket;

    if (tp->tp_Res1)                /* If sucessful */
    {
        UBYTE *efrom;
        efrom = &((UBYTE *)tp)[sizeof(struct TPacket)];
	efrom = TOLONGWORD(efrom);

        CopyMem(efrom,BADDR(dp->dp_Arg2),sizeof(struct FileInfoBlock));
    }

    ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
}


/* StartSameLock
 *
 * ACTION_SAME_LOCK
 *
 */
static BOOL StartSameLock(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	                  struct MountedFSInfoClient *m)
{

    /* Either of the two given locks may be old, and invalid.  Check.  */
    /* If not a root lock, fill in the TPacket version */

    /* Src lock */
    if (dp->dp_Arg1)
    {
        struct CLock *ol;
        ol = (struct CLock *) BADDR(dp->dp_Arg1);
        if (ol->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
        {
            dp->dp_Res1 = DOSFALSE;
            dp->dp_Res2 = ERROR_INVALID_LOCK;
            return (FALSE);
        }
        tp->tp_Arg1 = (LONG) ol->CLock_ServerLock;
    }
    else
        tp->tp_Arg1 = 0L;


    /* Dst Lock */
    if (dp->dp_Arg2)
    {
        struct CLock *ol;
        ol = (struct CLock *) BADDR(dp->dp_Arg2);
        if (ol->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
        {
            dp->dp_Res1 = DOSFALSE;
            dp->dp_Res2 = ERROR_INVALID_LOCK;
            return (FALSE);
        }
        tp->tp_Arg2 = (LONG) ol->CLock_ServerLock;
    }
    else
        tp->tp_Arg2 = 0L;

    return(TRUE);
}


/* StartInfo
 *
 * ACTION_INFO
 *
 */
static BOOL StartInfo(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	              struct MountedFSInfoClient *m)
{

    /* Either of the two given locks may be old, and invalid.  Check.  */
    /* If not a root lock, fill in the TPacket version */

    /* Src lock */
    if (dp->dp_Arg1)
    {
        struct CLock *ol;
        ol = (struct CLock *) BADDR(dp->dp_Arg1);
        if (ol->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
        {
            dp->dp_Res1 = DOSFALSE;
            dp->dp_Res2 = ERROR_INVALID_LOCK;
            return (FALSE);
        }
        tp->tp_Arg1 = (LONG) ol->CLock_ServerLock;
    }
    else
        tp->tp_Arg1 = 0L;

    tp->tp_Arg2 = sizeof(struct TPacket);

    return(TRUE);
}

/* StartChangeMode
 *
 * ACTION_CHANGE_MODE
 *
 */
static BOOL StartChangeMode(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	                    struct MountedFSInfoClient *m)
{

    /* Either of the two given locks may be old, and invalid.  Check.  */
    /* If not a root lock, fill in the TPacket version */

    if (dp->dp_Arg1 == CHANGE_LOCK)
    {
        /* Src lock */
        if (dp->dp_Arg2)
        {
            struct CLock *ol;
            ol = (struct CLock *) BADDR(dp->dp_Arg2);
            if (ol->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
            {
                dp->dp_Res1 = DOSFALSE;
                dp->dp_Res2 = ERROR_INVALID_LOCK;
                return (FALSE);
            }
            tp->tp_Arg2 = (LONG) ol->CLock_ServerLock;
        }
    }
    else if (dp->dp_Arg1 == CHANGE_FH)
    {
        struct CFH *cf;
        cf = (struct CFH *) dp->dp_Arg2;

        /* The possibility of an old, unreincarnated fh exists. */

        // checks for out-of-date fh, AND queues packet if a read/write is active
        if (!ValidateFH(cf,dp,m))
	    return FALSE;

        tp->tp_Arg2 = (LONG)cf->CFH_ServerFH;
    }

    return(TRUE);
}



/*
 *  EndInfo
 *
 */

static void EndInfo(struct TPacket *tp, struct MountedFSInfoClient *m)
{
    struct DosPacket *dp=tp->tp_DosPacket;

    if (tp->tp_Res1)                /* If sucessful */
    {
        UBYTE *efrom;
        struct InfoData *i;
        efrom = &((UBYTE *)tp)[sizeof(struct TPacket)];
	efrom = TOLONGWORD(efrom);

        CopyMem(efrom,BADDR(dp->dp_Arg2),sizeof(struct InfoData));
        i = (struct InfoData *) BADDR(dp->dp_Arg2);
        i->id_DiskType = ID_FFS_DISK;
        i->id_VolumeNode = MKBADDR(m->mfsi_Volume);
    }

    ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
}




/* StartFlush
 *
 * ACTION_FLUSH
 *
 */
static BOOL StartFlush(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	               struct MountedFSInfoClient *m)
{
    return(TRUE);
}




/* StartDiskInfo
 *
 * ACTION_DISK_INFO
 *
 */
static BOOL StartDiskInfo(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	                  struct MountedFSInfoClient *m)
{

    tp->tp_Arg1 = sizeof(struct TPacket);

    return(TRUE);
}


static BOOL StartToID(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	              struct MountedFSInfoClient *m)
{
    STRPTR w;
    tp->tp_Arg1 = sizeof(struct TPacket);
    w = (STRPTR) ((ULONG)(tp) + sizeof(struct TPacket));
    strncpy(w,(char *) dp->dp_Arg1,33);
    return(TRUE);
}

static BOOL StartToInfo(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	                struct MountedFSInfoClient *m)
{

    tp->tp_Arg1 = dp->dp_Arg1;
    tp->tp_Arg2 = sizeof(struct TPacket);
    return(TRUE);
}

static void EndToInfo(struct TPacket *tp, struct MountedFSInfoClient *m)
{
    struct DosPacket *dp=tp->tp_DosPacket;

    if (tp->tp_Res1)
    {
        APTR q;
        ULONG sz;
        q = (APTR) ((ULONG) tp + sizeof(struct TPacket));
        if (dp->dp_Type == ACTION_UID_TO_USERINFO)
            sz = sizeof(struct UserInfo);
        else
            sz = sizeof(struct GroupInfo);

        CopyMem(q,(APTR)dp->dp_Arg2,sz);
    }
    ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);

}


/*
 *  EndDiskInfo
 *
 */

static void EndDiskInfo(struct TPacket *tp, struct MountedFSInfoClient *m)
{
    struct DosPacket *dp=tp->tp_DosPacket;

    if (tp->tp_Res1)                /* If sucessful */
    {
        UBYTE *efrom;
        struct InfoData *i;
        efrom = &((UBYTE *)tp)[sizeof(struct TPacket)];
	efrom = TOLONGWORD(efrom);

        CopyMem(efrom,BADDR(dp->dp_Arg1),sizeof(struct InfoData));
        i = (struct InfoData *) BADDR(dp->dp_Arg1);
        i->id_VolumeNode = MKBADDR(m->mfsi_Volume);
        i->id_DiskType = ID_FFS_DISK;
    }

    ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
}

/*
 *  StartOpenFromLock
 *
 */

static BOOL StartOpenFromLock(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	                      struct MountedFSInfoClient *m)
{

    struct CLock *cl;

/* The given lock may be old */
    cl = (struct CLock *) BADDR(dp->dp_Arg2);

    if (dp->dp_Arg2)
    {
        if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
        {
            dp->dp_Res1 = DOSFALSE;
            dp->dp_Res2 = ERROR_INVALID_LOCK;
            return(FALSE);
        }
        tp->tp_Arg2 = (LONG) cl->CLock_ServerLock;
    }
    else
        tp->tp_Arg2 = 0L;


    return(TRUE);
}

/*
 * StartLockFromFH
 *
 * ACTION_COPY_DIR_FH
 *
 */
static BOOL StartLockFromFH(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	                    struct MountedFSInfoClient *m)
{
    struct CFH *cf;

    cf = (struct CFH *) dp->dp_Arg1;

    /* The possibility of an old, unreincarnated fh exists. */

    // checks for out-of-date fh, AND queues packet if a read/write is active
    if (!ValidateFH(cf,dp,m))
	return FALSE;

    tp->tp_Arg1 = (LONG) cf->CFH_ServerFH;

    return(TRUE);

}


/*
 * StartReadLink
 *
 * ACTION_READ_LINK
 *
 */
static BOOL StartReadLink(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
	                  struct MountedFSInfoClient *m)
{
//    struct CLock *cl;
//    UBYTE *gp;
    dp->dp_Res1 = -1L;
    dp->dp_Res2 = ERROR_ACTION_NOT_KNOWN;

    return(FALSE);
}




static BOOL DoStart(LONG type,struct Transaction *t, struct TPacket *tp, struct MountedFSInfoClient *m)
{
    struct DosPacket *dp;
    dp = (struct DosPacket *) tp->tp_DosPacket;
//kprintf("Start %ld...",dp->dp_Action);
    switch(type)
    {
// Handled as special case before calling DoStart...
//        case ACTION_READ:
//        case ACTION_WRITE:
//            return(StartReadWrite(dp,m));
//
//        case ACTION_EXAMINE_ALL:
//            return(StartExAll(t,tp,dp,m));

        case ACTION_EXAMINE_FH:
        case ACTION_EXAMINE_NEXT:
        case ACTION_EXAMINE_OBJECT:
            return(StartExamine(t,tp,dp,m));

        case ACTION_FINDINPUT:
        case ACTION_FINDOUTPUT:
        case ACTION_FINDUPDATE:
            return(StartFIND(t,tp,dp,m));

        case ACTION_ENVOY_EXAMINE_ALL:
            return(StartFakeExamine(t,tp,dp,m));

        case ACTION_FH_FROM_LOCK:
            return(StartOpenFromLock(t,tp,dp,m));

        case ACTION_COPY_DIR_FH:
            return(StartLockFromFH(t,tp,dp,m));

        case ACTION_SEEK:
            return(StartSeek(t,tp,dp,m));

        case ACTION_END:
            return(StartEND(t,tp,dp,m));

        case ACTION_RENAME_OBJECT:
            return(StartRename(t,tp,dp,m));

        case ACTION_SET_OWNER:
        case ACTION_SET_PROTECT:
            return(StartSetProtect(t,tp,dp,m));

        case ACTION_SET_COMMENT:
            return(StartSetComment(t,tp,dp,m));

        case ACTION_FREE_LOCK:
            return(StartFreeLock(t,tp,dp,m));

        case ACTION_SET_DATE:
            return(StartSetDate(t,tp,dp,m));

        case ACTION_PARENT:
        case ACTION_COPY_DIR:
	case ACTION_EXAMINE_ALL_END:		// we only care about passing the lock
            return(StartParent(t,tp,dp,m));

        case ACTION_DELETE_OBJECT:
            return(StartDeleteObject(t,tp,dp,m));

        case ACTION_CREATE_DIR:
        case ACTION_LOCATE_OBJECT:
            return(StartLocateObject(t,tp,dp,m));

        case ACTION_SAME_LOCK:
            return(StartSameLock(t,tp,dp,m));

        case ACTION_SET_FILE_SIZE:
        case ACTION_FLUSH:
            return(StartFlush(t,tp,dp,m));

        case ACTION_CHANGE_MODE:
            return(StartChangeMode(t,tp,dp,m));

        case ACTION_USERNAME_TO_UID:
        case ACTION_GROUPNAME_TO_GID:
            return(StartToID(t,tp,dp,m));

        case ACTION_UID_TO_USERINFO:
        case ACTION_GID_TO_GROUPINFO:
            return(StartToInfo(t,tp,dp,m));

        case ACTION_PARENT_FH:
            return(StartParentFH(t,tp,dp,m));

        case ACTION_IS_FILESYSTEM:
        {
            dp->dp_Res1 = DOSTRUE;
            return(FALSE);
        }

        case ACTION_CURRENT_VOLUME:
        {
            dp->dp_Res1 = MKBADDR(m->mfsi_Volume);
            return(FALSE);
        }
        case ACTION_EFS_INFO:
        {
            char *targethost, *targetpath;
            dp->dp_Res1 = DOSTRUE;
            targethost = (char *) dp->dp_Arg1;
            targetpath = (char *) dp->dp_Arg2;
            strncpy(targethost,m->mfsi_Machine,dp->dp_Arg3);
            strncpy(targetpath,m->mfsi_RemotePath,dp->dp_Arg4);
            return(FALSE);
        }

        case ACTION_READ_LINK:
            return(StartReadLink(t,tp,dp,m));

        case ACTION_INFO:
            return(StartInfo(t,tp,dp,m));

        case ACTION_DISK_INFO:
            return(StartDiskInfo(t,tp,dp,m));

        default:
case ACTION_RENAME_DISK:
case ACTION_ADD_NOTIFY:
case ACTION_REMOVE_NOTIFY:
case ACTION_MAKE_LINK:
case ACTION_LOCK_RECORD:
case ACTION_FREE_RECORD:
case ACTION_INHIBIT:
case ACTION_MORE_CACHE:
        {
            dp->dp_Res1 = DOSFALSE;
            dp->dp_Res2 = ERROR_ACTION_NOT_KNOWN;
            return(FALSE);
            break;
        }
    }
}





static BOOL DoEnd(LONG type, struct Transaction *t, struct TPacket *tp, struct MountedFSInfoClient *m)
{
//kprintf("  End %ld\n",type);
    switch(type)
    {

        case ACTION_READ:
	case ACTION_WRITE:
            return EndReadWrite(t,tp,m);                /* Special case! */

//        case ACTION_READ_LINK:
//            EndReadLink(tp,m);
//	      break;

	case ACTION_ENVOY_EXAMINE_ALL:
	    return end_exnext_data (t,tp,m);	// may return TRUE, meaning it wants
						// to reuse the transaction.
	case ACTION_EXAMINE_ALL:
	    EndExAll(tp,m);
	    break;

        case ACTION_EXAMINE_FH:
        case ACTION_EXAMINE_NEXT:
        case ACTION_EXAMINE_OBJECT:
            EndExamine(tp,m);
	    break;

        case ACTION_FH_FROM_LOCK:
        case ACTION_FINDINPUT:
        case ACTION_FINDOUTPUT:
        case ACTION_FINDUPDATE:
            EndFind(tp,m);
	    break;

        case ACTION_SEEK:
            return EndSeek(tp,m);	// may want to send another command

        case ACTION_END:
        case ACTION_CHANGE_MODE:
        case ACTION_USERNAME_TO_UID:
        case ACTION_GROUPNAME_TO_GID:
        case ACTION_RENAME_OBJECT:
        case ACTION_SET_PROTECT:
        case ACTION_SET_OWNER:
        case ACTION_SET_DATE:
        case ACTION_DELETE_OBJECT:
        case ACTION_SET_COMMENT:
        case ACTION_SAME_LOCK:
        case ACTION_SET_FILE_SIZE:
	case ACTION_EXAMINE_ALL_END:
            EndEnd(tp,m);		// just return results, except for ACTION_END.
	    break;

        case ACTION_FREE_LOCK:
            EndFreeLock(tp,m);
	    break;

        case ACTION_PARENT_FH:
        case ACTION_PARENT:
        case ACTION_COPY_DIR:
        case ACTION_COPY_DIR_FH:
        case ACTION_CREATE_DIR:
        case ACTION_LOCATE_OBJECT:
            return EndParent(t,tp,m);	// may need to reuse transaction after NULL
					// serverlock resolution.
        case ACTION_FLUSH:
            EndEnd(tp,m);
	    break;

        case ACTION_INFO:
            EndInfo(tp,m);
	    break;

        case ACTION_DISK_INFO:
            EndDiskInfo(tp,m);
	    break;

        case ACTION_UID_TO_USERINFO:
        case ACTION_GID_TO_GROUPINFO:
            EndToInfo(tp,m);
	    break;

        default:
        {
            break;
        }
    }
    return FALSE;
}

/*
 * NetDownRequest()
 *
 * Returns TRUE for "retry", and FALSE for "cancel".
 *
 */

static BOOL NetDownRequest(UBYTE *machine, UBYTE *volume)
{
    BOOL returnval = FALSE;
    ULONG args[2];

    IntuitionBase = OpenLibrary("intuition.library",0L);
    if (IntuitionBase)
    {
        struct EasyStruct ers={sizeof(struct EasyStruct),0L,"Envoy Network Problem",
                                "Host '%s'\ndidn't respond regarding\nvolume '%s:'","Retry|Cancel"};
        args[0] = (ULONG) machine;
        args[1] = (ULONG) volume;
        returnval = (BOOL) TimeOutEasyRequest(0L,&ers,0L,args,REQUESTERTIMEOUT);

        CloseLibrary(IntuitionBase);
    }

    return(returnval);
}



/*
 * MountFailedRequest()
 *
 * Returns TRUE for "retry", and FALSE for "cancel".
 *
 */

static BOOL MountFailedRequest(UBYTE *machine, UBYTE *volume, ULONG err)
{
    BOOL returnval = FALSE;

    IntuitionBase = OpenLibrary("intuition.library",0L);
    if (IntuitionBase)
    {
        ULONG args[3];
        UBYTE *reason;
        struct EasyStruct ers={sizeof(struct EasyStruct),0L,"Envoy Network Problem",
                "Host '%s'\nrejected connection attempt for\nvolume '%s:'\n%s","OK"};


        switch (err)
        {
            case FSERR_REJECTED_USER:
                reason = "User rejected";
                break;
            case FSERR_REJECTED_NOMOUNT:
                reason = "Volume does not exist";
                break;
            case ENVOYERR_TIMEOUT:
                reason = "Timed out trying to obtain a mount";
                break;
            case ENVOYERR_CANTDELIVER:
                reason = "Server is refusing our data";
                break;
            default:
                reason = "Unknown error!";
                break;
        }
        args[0] = (ULONG) machine;
        args[1] = (ULONG) volume;
        args[2] = (ULONG) reason;
        returnval = (BOOL) TimeOutEasyRequest(0L,&ers,0L,args,REQUESTERTIMEOUT);

        CloseLibrary(IntuitionBase);
    }

    return(returnval);
}

/*
 * CantConnectRequest()
 *
 * Returns TRUE for "retry", and FALSE for "cancel".
 *
 */

static BOOL CantConnectRequest(UBYTE *machine)
{
    BOOL returnval = FALSE;

    IntuitionBase = OpenLibrary("intuition.library",0L);
    if (IntuitionBase)
    {
        struct EasyStruct ers={sizeof(struct EasyStruct),0L,"Envoy Network Problem",
                               "Cannot connect to\nhost '%s'","Retry|Cancel"};
        returnval = (BOOL) TimeOutEasyRequest(0L,&ers,0L,&machine,REQUESTERTIMEOUT);

        CloseLibrary(IntuitionBase);
    }

    return(returnval);
}



/*
 * GPRequest()
 *
 * Returns TRUE for "retry", and FALSE for "cancel".
 *
 */

static BOOL GPRequest(ULONG num)
{
    BOOL returnval = FALSE;

    IntuitionBase = OpenLibrary("intuition.library",0L);
    if (IntuitionBase)
    {
        struct EasyStruct ers={sizeof(struct EasyStruct),0L,"Envoy Network Problem",
                               "Error code: %lx","OK"};
        returnval = (BOOL) TimeOutEasyRequest(0L,&ers,0L,&num,REQUESTERTIMEOUT);

        CloseLibrary(IntuitionBase);
    }

    return(returnval);
}


static LONG TimeOutEasyRequest(struct Window *ref, struct EasyStruct *ez, ULONG id, APTR args, ULONG timeout)
{
    LONG resp;
    struct Window *reqwin;
    reqwin = (struct Window *) BuildEasyRequestArgs(ref,ez,id,args);
    if (reqwin)
    {
        struct MsgPort *rp;
        rp = (struct MsgPort *) CreateMsgPort();
        if (rp)
        {
            struct timerequest *tio;
            tio = (struct timerequest *) CreateIORequest(rp,sizeof(struct timerequest));
            if (tio)
            {
                if (!OpenDevice("timer.device", UNIT_VBLANK, (struct IORequest *) tio, 0))
                {
                    tio->tr_node.io_Command = TR_ADDREQUEST;
                    tio->tr_node.io_Flags = 0;
                    tio->tr_time.tv_secs = timeout;
                    tio->tr_time.tv_micro = 0;
                    SendIO((struct IORequest *) tio);
                    while (TRUE)
                    {
                        ULONG waitmask;

                        waitmask = ( (1 << rp->mp_SigBit) | (1 << reqwin->UserPort->mp_SigBit) );
                        Wait(waitmask);
                        if (GetMsg(rp))
                        {
                            resp = 0; /* negative response on timeout */
                            break;
                        }
                        else
                        {
                            resp = SysReqHandler(reqwin, &id, FALSE);
                            if ((resp+1) >= 0)
                            {
                                AbortIO((struct IORequest *) tio);
                                WaitIO((struct IORequest *) tio);
                                break;
                            }
                        }
                    }
                    CloseDevice((struct IORequest *) tio);
                }
                DeleteIORequest(tio);
            }
            DeleteMsgPort(rp);
        }
        FreeSysRequest(reqwin);
    }
    return(resp);
}


