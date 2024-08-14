
/*
 * Envoy Filesystem Client V0.0
 * $id$
 *
 *
 * BUGS:
 *
 * If on reconnect, the seek of a FH fails to locate the correct position, set the file
 * size TO at least the last known position.
 *
 * I ought to be doing something akin to gp = (UBYTE *) ((ULONG) (gp+3L) & ~3L) instead of the
 * slow little while() I have in there everywhere.  FIXME.
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
#include <intuition/intuition.h>
#include "/fs.h"
#include "efs_rev.h"

#include <pragmas/exec_pragmas.h>
#include <clib/exec_protos.h>
#include <pragmas/nipc_pragmas.h>
#include <clib/nipc_protos.h>
#include <pragmas/services_pragmas.h>
#include <clib/services_protos.h>
#include <dos.h>

struct Entity *Reconnect();
void SaveName();
BOOL FixLock();
void KeepLock();
void NukeLock();
void KeepFH();
void NukeFH();
BOOL FixFH();
void SaveFHName();
BOOL DoStart();
BOOL NetDownRequest(UBYTE *machine, UBYTE *volume);
BOOL MountFailedRequest(UBYTE *machine, UBYTE *volume, ULONG err);
BOOL CantConnectRequest(UBYTE *machine);
BOOL GPRequest(ULONG num);
void DoEnd();
extern UBYTE *clpcpy(UBYTE *d, UBYTE *s, ULONG l);
LONG TimeOutEasyRequest(struct Window *ref, struct EasyStruct *ez, ULONG id, APTR args, ULONG timeout);


char *rev=VSTRING;
char *revision = VERSTAG;

#define DEBUGMSG kprintf
#define MAXSIZE 65536
#define PACKETTIMEOUT 10
#define MIN(x, y) ((x) < (y) ? (x):(y))

extern struct Library * AbsExecBase;
//#define SysBase  AbsExecBase

#define ERROR_NETWORK_FAILED ERROR_SEEK_ERROR

BOOL FixBString();


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
    BSTR   ourname;

    /* NIPC/Services Mgr stuff */
    struct Entity *e, *re;
    struct DosList *vol;
    struct Transaction *t;
    struct TPacket *tp;
    ULONG sigbit=0;
    ULONG cetags[3]={ENT_AllocSignal,0,TAG_DONE};
    ULONG attags[3]={TRN_AllocReqBuffer,512,TAG_DONE};
//    ULONG notags[1]={TAG_DONE};
    ULONG retrycount=0;
    UBYTE *startstring;

    UBYTE machine[80],rname[256],lname[80],user[80],password[80], flags[17];

    BOOL networkdown=FALSE;                 /* True if the network appears to be down at the moment */
    BOOL startupstatus=FALSE;
//    ULONG cnum=0L;                          /* current connection #; merely distinguishes old data vs. new */
    ULONG mflags = 0L;
    struct MountedFSInfoClient *m;          /* General global structure describing this beast */
    struct FileSysStartupMsg *fssm;

    SysBase = (struct Library *) (*(void **)4L);

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
        ourname = (BSTR) dp->dp_Arg1;
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
                        }
                    }
                }
            }
        }

        NIPCBase = (struct Library *) OpenLibrary("nipc.library",0L);
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


                            t->trans_ResponseData = t->trans_RequestData;
                            tp = (struct TPacket *) t->trans_RequestData;
                            t->trans_RespDataLength = t->trans_ReqDataLength;
                            networkdown = TRUE;

                            m->mfsi_Destination = Reconnect(t,m,1,0L);
                            re = m->mfsi_Destination;
                            if (m->mfsi_Destination)
                            {
                                re = m->mfsi_Destination;

                                Forbid();
                                if (!dn->dn_Task)
                                {
                                    dn->dn_Task = localport;
                                    Permit();
                                    ReplyPkt(dp,DOSTRUE,dp->dp_Res2);
                                    startupreturned = TRUE;

                                    if (!strlen(&m->mfsi_VolumeName))           /* If no vol name given, use default */
                                        strcpy(&m->mfsi_VolumeName,lname);

                                    vol = (struct DosList *) MakeDosEntry(&m->mfsi_VolumeName,DLT_VOLUME);
                                    if (vol)
                                    {
                                        ((struct DeviceList *)vol)->dl_Task = localport;
                                        ((struct DeviceList *)vol)->dl_DiskType = ((struct DosEnvec *)BADDR(fssm->fssm_Environ))->de_DosType;

                                        m->mfsi_Volume = vol;


                                        AddDosEntry(vol);

                                        while(TRUE)
                                        {
                                            struct Transaction *rt;
                                            ULONG failcode=0;

                                            /* DosPackets */
                                            sp = (struct StandardPacket *)GetMsg(localport);
                                            if (sp)
                                            {
                                                struct Transaction *ot;
                                                dp = (struct DosPacket *) sp->sp_Msg.mn_Node.ln_Name;
                                                dp->dp_Link = &(sp->sp_Msg); /* In case they don't do it */
                                                /*
                                                 * If the network is down, attempt to reconnect.  If this attempt
                                                 * fails, return the packet as failed.
                                                 */
                                                if ((networkdown) && (dp->dp_Type != ACTION_IS_FILESYSTEM))      /* To keep Workbench happy */
                                                {
                                                    if (retrycount > 1)
                                                    {
                                                        if (!NetDownRequest(m->mfsi_Machine,m->mfsi_LocalPath))
                                                        {
                                                            ULONG failure,failresp;
                                                            failure = ERROR_NETWORK_FAILED;
                                                            if (failcode == ENVOYERR_NORESOURCES)
                                                                failure = ERROR_NO_FREE_STORE;
                                                            failresp = DOSFALSE;
                                                            if ((dp->dp_Type == ACTION_READ) || (dp->dp_Type == ACTION_WRITE))
                                                                failresp = DOSTRUE;
                                                            ReplyPkt(dp,failresp,failure);
                                                            retrycount = 0L;
                                                            continue;
                                                        }
                                                    }
                                                    if (!(m->mfsi_Destination = Reconnect(t,m,(m->mfsi_Destination != 0),&failcode)))
                                                    {
                                                        re = m->mfsi_Destination;
                                                        networkdown = TRUE;
                                                        /* put it at the head of the msgport */
                                                        Disable();
                                                        AddHead(&(localport->mp_MsgList),(struct Node *)sp);
                                                        Enable();
                                                        retrycount++;
                                                        continue;
                                                    }
                                                    else
                                                    {
                                                        failcode = 0;
                                                        re = m->mfsi_Destination;
                                                        retrycount = 0L;
                                                        networkdown = FALSE;
                                                        re = m->mfsi_Destination;
                                                    }
                                                }

                                                ot = (struct Transaction *) AllocTransactionA((struct TagItem *)&attags);
                                                if (!ot)
                                                {
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
                                                    ot->trans_Timeout = PACKETTIMEOUT;
                                                    ot->trans_Command = FSCMD_DOSPACKET;
                                                    /* Some Special case shit right here */
                                                    if ((dp->dp_Type == ACTION_READ) || (dp->dp_Type == ACTION_WRITE))
                                                    {
                                                    /* Action_Read and Action_Write */
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
                                                        if (!m->mfsi_Destination)
                                                            networkdown = TRUE;
                                                    }
                                                    else
                                                    {
                                                        /* Everybody else */
                                                        if (DoStart(dp->dp_Type,ot,otp,m))
                                                        {
                                                            BeginTransaction(m->mfsi_Destination,e,ot);
                                                        }
                                                        else
                                                        {
                                                            FreeTransaction(ot);
                                                            ReplyPkt(dp,dp->dp_Res1,dp->dp_Res2);
                                                            retrycount = 0L;
                                                        }
                                                    }
                                                }
                                            }

                                            /* Returning Transactions */
                                            rt = GetTransaction(e);
                                            if (rt)
                                            {
                                                struct TPacket *rtp;
                                                rtp = (struct TPacket *) rt->trans_RequestData;

                                                if (rt->trans_Error)
                                                {
                                                    struct StandardPacket *ssp;
                                                    rtp = (struct TPacket *) rt->trans_RequestData;
                                                    ssp = (struct StandardPacket *) ((struct DosPacket *)rtp->tp_DosPacket)->dp_Link;
                                                    networkdown = TRUE;
                                                    if  ((((rtp->tp_Type == ACTION_READ) || (rtp->tp_Type == ACTION_WRITE)) && (!rtp->tp_Arg4)) || ((rtp->tp_Type != ACTION_READ) && (rtp->tp_Type != ACTION_WRITE)) )
                                                    {
                                                        /* put it at the head of the msgport, yes - I know this is not a nice thing to do */
                                                        Disable();
                                                        AddHead(&(localport->mp_MsgList),(struct Node *)ssp);
                                                        Enable();
                                                        retrycount++;
                                                    }
                                                }
                                                else
                                                {
                                                    DoEnd(rtp->tp_Type,rt,m);
                                                }
                                                FreeTransaction(rt);
                                            }

                                            if ((!sp) && (!rt))
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

BOOL FixBString(UBYTE *cp)
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

void SaveFHName(struct CFH *fh,struct TPacket *tp)
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
}

void SaveName(struct CLock *cl,struct TPacket *tp)
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
}

void KeepLock(APTR thelock,struct MountedFSInfoClient *m)
{

    struct ResourcesUsed *ru;

    ru = (struct ResourcesUsed *) AllocMem(sizeof(struct ResourcesUsed),MEMF_CLEAR|MEMF_PUBLIC);
    if (!ru)
        return;
    ru->ru_Resource = thelock;
    AddTail((struct List *)&m->mfsi_Locks,(struct Node *)&ru->ru_link);
}


void KeepFH(APTR thefh,struct MountedFSInfoClient *m)
{

    struct ResourcesUsed *ru;

    ru = (struct ResourcesUsed *) AllocMem(sizeof(struct ResourcesUsed),MEMF_CLEAR|MEMF_PUBLIC);
    if (!ru)
        return;
    ru->ru_Resource = thefh;
    AddTail((struct List *)&m->mfsi_FileHandles,(struct Node *)&ru->ru_link);
}

void NukeLock(APTR thelock,struct MountedFSInfoClient *m)
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

void NukeFH(APTR thefh,struct MountedFSInfoClient *m)
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

BOOL FixFH(struct Entity *re, struct Entity *e,struct CFH *c,struct Transaction *t,
           struct TPacket *tp, ULONG cnum, struct MountedFSInfoClient *m)
{

 UBYTE *datax;
 int x;
 LONG oldtype;

    oldtype = tp->tp_Type;
   /* This FH is known to be created on a previous connection */

    datax = &((UBYTE *)tp)[sizeof(struct TPacket)];

    while ( (ULONG) datax & 3)
        datax ++;

    t->trans_Command = FSCMD_DOSPACKET;
    t->trans_Timeout = PACKETTIMEOUT;
    if (c->CFH_Access == ACTION_FINDINPUT)
        tp->tp_Type = ACTION_FINDINPUT;
    else
        tp->tp_Type = ACTION_FINDUPDATE;
    tp->tp_Arg1 = 0L;
    tp->tp_Arg2 = -1L;
    tp->tp_Arg3= ((ULONG) datax - (ULONG)tp);
    tp->tp_Arg4 = m->mfsi_Flags & FLAGSMASK;
    tp->tp_ServerMFSI = m->mfsi_ServerMFSI;
    x = strlen(c->CFH_FullServerPath);
    datax[0]=x;
    CopyMem(c->CFH_FullServerPath,&datax[1],x);
    FixBString(datax);

    t->trans_ReqDataActual = t->trans_ReqDataLength;
    DoTransaction(re,e,t);
    t->trans_Timeout = PACKETTIMEOUT;
    tp->tp_Type = oldtype;
    if (!t->trans_Error)
    {
        if (tp->tp_Res1)
        {
            c->CFH_ConnectionNumber = cnum;
            c->CFH_ServerFH = (struct FileLock *) tp->tp_Res1;
            t->trans_Timeout = PACKETTIMEOUT;
            tp->tp_Type = ACTION_SEEK;
            tp->tp_Arg1 = (LONG) c->CFH_ServerFH;
            tp->tp_Arg2 = c->CFH_Pos;
            tp->tp_Arg3 = OFFSET_BEGINNING;
            tp->tp_ServerMFSI = m->mfsi_ServerMFSI;
            DoTransaction(re,e,t);
        }
    }
    else
        return(FALSE);
    return(TRUE);

}


BOOL FixLock(struct Entity *re, struct Entity *e,struct CLock *c,struct Transaction *t,
             struct TPacket *tp, ULONG cnum, struct MountedFSInfoClient *m)
{
    UBYTE *datax;
    int x;
    LONG oldtype;

    oldtype = tp->tp_Type;

/* This lock is known to be created on a previous connection */

    datax = &((UBYTE *)tp)[sizeof(struct TPacket)];

    while ( (ULONG) datax & 3)
        datax++;

    t->trans_Command = FSCMD_DOSPACKET;
    tp->tp_Type = ACTION_LOCATE_OBJECT;
//    tp->tp_Arg1 = -1L;
    tp->tp_Arg1 = 0L;
    tp->tp_Arg2 = ((ULONG)datax-(ULONG)tp);
    tp->tp_Arg3 = c->CLock_FileLock.fl_Access;
    tp->tp_Arg4 = m->mfsi_Flags & FLAGSMASK;
    tp->tp_ServerMFSI = m->mfsi_ServerMFSI;
    x = strlen(c->CLock_FullServerPath);
    datax[0]=x;
    CopyMem(c->CLock_FullServerPath,&datax[1],x);
    FixBString(datax);

    t->trans_ReqDataActual = t->trans_ReqDataLength;
    DoTransaction(re,e,t);
    t->trans_Timeout = PACKETTIMEOUT;
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

struct Entity *Reconnect(struct Transaction *t, struct MountedFSInfoClient *m, ULONG announce, ULONG *failcode)
{
    ULONG notags[3]={FSVC_Error,0,TAG_DONE};
    struct TPacket *tp=t->trans_RequestData;
    struct Entity *e=m->mfsi_Source;
    struct Entity *re=m->mfsi_Destination;
    UBYTE *gp;
    LONG old;
    struct ResourcesUsed *rr;
    ULONG ecode=0L;
    notags[1] = (ULONG) &ecode;

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
        strcpy(gp,"NEW");
        gp += strlen("NEW")+1;

        t->trans_Timeout = PACKETTIMEOUT;
        t->trans_ReqDataActual = t->trans_ReqDataLength;
        t->trans_RespDataLength = t->trans_ReqDataLength;
        DoTransaction(re,e,t);
        t->trans_Command = FSCMD_DOSPACKET;
        t->trans_Timeout = PACKETTIMEOUT;

        if (t->trans_Error)
        {
            if (announce)
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
BOOL StartFIND(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
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

    gp = &((UBYTE *) (t->trans_RequestData))[sizeof(struct TPacket)];

    /* Bump the general purpose pointer up to longword alignment */
    while ( (ULONG) gp & 3L)
        gp++;

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
void EndFind(struct TPacket *tp, struct MountedFSInfoClient *m)
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
BOOL StartEND(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
               struct MountedFSInfoClient *m)
{

    struct CFH *cf;
    UBYTE *l, *gp;

    /* The possibility that the filehandle referenced by this existed on a
     * previous connection, and couldn't be reestablished exists.
     * Thus, we check.
     */


    cf = (struct CFH *) dp->dp_Arg1;

    if (cf->CFH_ConnectionNumber < m->mfsi_ConnectionNumber)
    {
        dp->dp_Res1 = DOSFALSE;
        dp->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
        return(FALSE);
    }

    /* Free up the path string */
    l = (UBYTE *) cf->CFH_FullServerPath;
    if (l)
    {
        FreeMem(l,strlen(l)+1);
    }

    /* Remove this fh from our lists */
    NukeFH(cf,m);

    /* make the general purpose pointer point to directly past the TPacket */
    gp = &((UBYTE *) t->trans_RequestData)[sizeof(struct TPacket *)];

    tp->tp_Arg1 = (LONG) cf->CFH_ServerFH;

    return(TRUE);

}

/*
 * EndEnd, EndSeek, EndRename, EndSetProtect, EndSetDate, EndDeleteObject, EndSetComment
 *
 *  I love names like that.
 *
 */

void EndEnd(struct TPacket *tp, struct MountedFSInfoClient *m)
{
    struct DosPacket *dp=tp->tp_DosPacket;

    ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
}


/*
 * EndSeek
 *
 *
 */

void EndSeek(struct TPacket *tp, struct MountedFSInfoClient *m)
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
            /* FIXME!  CUrrently broken.  Needs to insert an ACTION_SEEK from
             * OFFSET_BEGINNING w/ offset=0 (specially flagged) in here to find
             * out where the position is after something like this.
             */
            break;

    }

    ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
}




/* StartSeek
 *
 * ACTION_SEEK
 *
 */
BOOL StartSeek(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
               struct MountedFSInfoClient *m)
{

    struct CFH *cf;

    cf = (struct CFH *) dp->dp_Arg1;

    /* The possibility of an old, unreincarnated fh exists. */

    if (cf->CFH_ConnectionNumber < m->mfsi_ConnectionNumber)
    {
        dp->dp_Res1 = DOSFALSE;
        dp->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
        return(FALSE);
    }

    tp->tp_Arg1 = (LONG) cf->CFH_ServerFH;
    tp->tp_Arg2 = dp->dp_Arg2;
    tp->tp_Arg3 = dp->dp_Arg3;

    return(TRUE);
}


/* StartRead
 *
 * ACTION_READ
 *
 */
BOOL StartRead(struct Transaction *x, struct TPacket *xp, struct DosPacket *dp,
               struct MountedFSInfoClient *m)
{

    struct CFH *cf;
    ULONG remaining, thispacketsize;

    cf = (struct CFH *) dp->dp_Arg1;

    /* The possibility of an old, unreincarnated fh exists. */

    if (!dp->dp_Arg3)
    {
        dp->dp_Res1 = 0L;
        dp->dp_Res2 = 0L;
        return(FALSE);
    }

    if (cf->CFH_ConnectionNumber < m->mfsi_ConnectionNumber)
    {
        dp->dp_Res1 = DOSTRUE;
        dp->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
        return(FALSE);
    }

    /* Find the total amount of data to read */
    dp->dp_Res1 = 0L;           /* Set this to 0 */
    remaining = dp->dp_Arg3;
    if (!remaining)
    {
        dp->dp_Res1 = DOSFALSE;
        return(FALSE);
    }

    while (remaining)
    {
        ULONG rttags[5]={TRN_AllocReqBuffer,sizeof(struct TPacket),TRN_AllocRespBuffer,0,TAG_DONE};
        struct Transaction *t;

        thispacketsize = MIN(remaining,MAXSIZE);        /* Find out how big this packet will be */
        remaining -= thispacketsize;
        rttags[3]=sizeof(struct TPacket) + thispacketsize;

        t = AllocTransactionA((struct TagItem *)&rttags);
        if (t)
        {
            struct TPacket *tp;
            tp = (struct TPacket *) t->trans_RequestData;
            tp->tp_ServerMFSI = m->mfsi_ServerMFSI;
            tp->tp_DosPacket = dp;
            tp->tp_Type = dp->dp_Type;

            t->trans_ReqDataActual = t->trans_ReqDataLength;
            t->trans_Timeout = PACKETTIMEOUT;
            t->trans_Command = FSCMD_DOSPACKET;

            tp->tp_Arg1 = (LONG)((struct CFH *)dp->dp_Arg1)->CFH_ServerFH;
            tp->tp_Arg2 = sizeof(struct TPacket);
            tp->tp_Arg3 = thispacketsize;
            /* When the packets return, I can match up the 'last' one to the DosPacket
             * by looking at tp_Arg4.  If 0, that tpacket is the last one -- and the
             * associated dospacket can be returned.
             */
            tp->tp_Arg4 = remaining;
            BeginTransaction(m->mfsi_Destination,m->mfsi_Source,t);
        }
        else
        {
            m->mfsi_Destination = Reconnect(x,m,TRUE,0);
            dp->dp_Res1 = DOSTRUE;
            dp->dp_Res2 = ERROR_NO_FREE_STORE;
            return(FALSE);
        }
    }
    return(TRUE);
}


/* StartWrite
 *
 * ACTION_WRITE
 *
 */
BOOL StartWrite(struct Transaction *x, struct TPacket *xp, struct DosPacket *dp,
                struct MountedFSInfoClient *m)
{

    struct CFH *cf;
    ULONG remaining, thispacketsize;

    cf = (struct CFH *) dp->dp_Arg1;

    /* The possibility of an old, unreincarnated fh exists. */

    if (cf->CFH_ConnectionNumber < m->mfsi_ConnectionNumber)
    {
        dp->dp_Res1 = DOSTRUE;
        dp->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
        return(FALSE);
    }

    /* Find the total amount of data to read */
    remaining = dp->dp_Arg3;
    dp->dp_Res1 = 0L;           /* Set this to 0 */
    while (remaining)
    {
        ULONG rttags[3]={TRN_AllocReqBuffer,0,TAG_DONE};
        struct Transaction *t;
        UBYTE *dataptr;


        thispacketsize = MIN(remaining,MAXSIZE);        /* Find out how big this packet will be */
        rttags[1]=sizeof(struct TPacket) + thispacketsize;

        dataptr = &(((UBYTE *)dp->dp_Arg2)[dp->dp_Arg3-remaining]);
        remaining -= thispacketsize;

        t = AllocTransactionA((struct TagItem *)&rttags);
        if (t)
        {
            struct TPacket *tp;
            tp = (struct TPacket *) t->trans_RequestData;
            tp->tp_ServerMFSI = m->mfsi_ServerMFSI;
            tp->tp_DosPacket = dp;
            tp->tp_Type = dp->dp_Type;

            t->trans_ReqDataActual = t->trans_ReqDataLength;
            t->trans_Timeout = PACKETTIMEOUT;
            t->trans_Command = FSCMD_DOSPACKET;
            t->trans_ResponseData = t->trans_RequestData;
            t->trans_RespDataLength = t->trans_ReqDataLength;

            tp->tp_Arg1 = (LONG)((struct CFH *)dp->dp_Arg1)->CFH_ServerFH;
            tp->tp_Arg2 = sizeof(struct TPacket);
            tp->tp_Arg3 = thispacketsize;

            /* When the packets return, I can match up the 'last' one to the DosPacket
             * by looking at tp_Arg4.  If 0, that tpacket is the last one -- and the
             * associated dospacket can be returned.
             */
            tp->tp_Arg4 = remaining;
            CopyMem(dataptr,&((UBYTE *)tp)[sizeof(struct TPacket)],thispacketsize);
            BeginTransaction(m->mfsi_Destination,m->mfsi_Source,t);
        }
        else
        {
            m->mfsi_Destination = Reconnect(x,m,TRUE,0);
            dp->dp_Res1 = DOSTRUE;
            dp->dp_Res2 = ERROR_NO_FREE_STORE;
            return(FALSE);
        }
/* Because of the potential problems of not being able to get a transaction at this point,
 * I feel the adequate solution is to return FAILED at this point, and also force a
 * reconnect.  I have yet to code this, though.  FIXME!  IMPORTANT! */
    }

    return(TRUE);
}


/*
 *  EndRead
 *
 *  Finishes off each individual ACTION_READ . . .
 *
 */

void EndRead(struct Transaction *t, struct TPacket *tp, struct MountedFSInfoClient *m)
{
    struct DosPacket *dp=tp->tp_DosPacket;

    /*
     * If successful, find out where the return data should go,
     * copy the data there, and be done with it.
     */

    if ((tp->tp_Res1) && (tp->tp_Res1 != -1))
    {
        /* Some data was returned */
        ULONG offset;
        UBYTE *destptr;
        /* the Offset that this block lies at can be calculated by:
         * TotalAskedFor - RemainingAfterThisBlock - SizeRequestedOfThisBlock
         */
        ((struct CFH *)dp->dp_Arg1)->CFH_Pos += tp->tp_Res1;
        offset = dp->dp_Arg3 - tp->tp_Arg4 - tp->tp_Arg3;
        destptr = (UBYTE *) &((UBYTE *) dp->dp_Arg2)[offset];
        CopyMem(&((UBYTE *)tp)[sizeof(struct TPacket)],destptr,tp->tp_Res1);
        dp->dp_Res1 += tp->tp_Res1;
    }
    else
    {
        if (tp->tp_Res1 == -1)
            dp->dp_Res1 = tp->tp_Res1;
    }

    if (!tp->tp_Arg4)                               /* If the last packet, return the dp! */
        ReplyPkt(dp,dp->dp_Res1,tp->tp_Res2);

}


/*
 *  EndWrite
 *
 *  Finishes off each individual ACTION_WRITE . . .
 *
 */

void EndWrite(struct Transaction *t, struct TPacket *tp, struct MountedFSInfoClient *m)
{
    struct DosPacket *dp=tp->tp_DosPacket;

    /*
     * If successful, increment the Res1 counter
     * copy the data there, and be done with it.
     */

    if ( (tp->tp_Res1) && (tp->tp_Res1 != -1) ) /* If a good response */
    {
        dp->dp_Res1 += tp->tp_Res1;
        ((struct CFH *)dp->dp_Arg1)->CFH_Pos += tp->tp_Res1;

    }
    else
    {
        dp->dp_Res1 = tp->tp_Res1;
    }

    if (!tp->tp_Arg4)                               /* If the last packet, return the dp! */
        ReplyPkt(dp,dp->dp_Res1,tp->tp_Res2);

}


/* StartRename
 *
 * ACTION_RENAME_OBJECT
 *
 */
BOOL StartRename(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
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


    gp = &(((UBYTE *) t->trans_RequestData)[sizeof(struct TPacket)]);

    /* Fill in the "from" filepath */
    /* bump gp up to longword boundries */
    while ((ULONG) gp & 3L)
        gp++;

    tp->tp_Arg2 = ((ULONG) gp - (ULONG) tp);

    CopyMem(BADDR(dp->dp_Arg2),gp,((UBYTE *)BADDR(dp->dp_Arg2))[0]+1);
    FixBString(gp);
    gp = &gp[((UBYTE *)BADDR(dp->dp_Arg2))[0]+1];

    /* Fill in the "to" filepath */
    /* bump gp up to longword boundries */
    while ((ULONG) gp & 3L)
        gp++;

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
BOOL StartSetProtect(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
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

    while ((ULONG) gp & 3L)
        gp++;

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
BOOL StartSetComment(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
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

    while ((ULONG) gp & 3L)
        gp++;

    tp->tp_Arg3 = ((ULONG) gp - (ULONG) tp);

    CopyMem(BADDR(dp->dp_Arg3),gp,((UBYTE *)BADDR(dp->dp_Arg3))[0]+1);
    FixBString(gp);
    gp = &gp[((UBYTE *)BADDR(dp->dp_Arg3))[0]+1];

    while ((ULONG) gp & 3L)
        gp++;

    tp->tp_Arg4 = ((ULONG) gp - (ULONG) tp);

    CopyMem(BADDR(dp->dp_Arg4),gp,((UBYTE *)BADDR(dp->dp_Arg4))[0]+1);
    FixBString(gp);
    gp = &gp[((UBYTE *)BADDR(dp->dp_Arg4))[0]+1];

    return(TRUE);

}

/* StartFreeLock
 *
 * ACTION_FREE_LOCK
 *
 */
BOOL StartFreeLock(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
                   struct MountedFSInfoClient *m)
{

    struct CLock *cl;

/* The given lock may be old */
    if (dp->dp_Arg1)
    {
        cl = (struct CLock *) BADDR(dp->dp_Arg1);
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

    cl = (struct CLock *) BADDR(dp->dp_Arg1);

    return(TRUE);

}

/*
 *  EndFreeLock
 *
 */

void EndFreeLock(struct TPacket *tp, struct MountedFSInfoClient *m)
{
    struct DosPacket *dp=tp->tp_DosPacket;
    struct CLock *cl;
    cl = (struct CLock *) BADDR(dp->dp_Arg1);

/*  Free the path string */
    NukeLock((APTR)MKBADDR(cl),m);
    if (cl->CLock_FullServerPath)
        FreeMem(cl->CLock_FullServerPath,strlen(cl->CLock_FullServerPath)+1);

    ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
}



/* StartSetDate
 *
 * ACTION_SET_DATE
 *
 */
BOOL StartSetDate(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
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

    while ((ULONG) gp & 3L)
        gp++;

    tp->tp_Arg3 = ((ULONG) gp - (ULONG) tp);

    CopyMem(BADDR(dp->dp_Arg3),gp,((UBYTE *)BADDR(dp->dp_Arg3))[0]+1);
    FixBString(gp);
    gp = &gp[((UBYTE *)BADDR(dp->dp_Arg3))[0]+1];

    while ((ULONG) gp & 3L)
        gp++;

    tp->tp_Arg4 = ((ULONG) gp - (ULONG) tp);
    CopyMem((APTR)dp->dp_Arg4,gp,sizeof(struct DateStamp));

    gp = &gp[sizeof(struct DateStamp)];

    return(TRUE);

}


/* StartParent, StartCopyDir
 *
 * ACTION_PARENT
 *
 */
BOOL StartParent(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
                  struct MountedFSInfoClient *m)
{
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

    return(TRUE);

}

/* StartParentFH
 *
 * ACTION_PARENT_FH
 *
 */
BOOL StartParentFH(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
                   struct MountedFSInfoClient *m)
{
    struct CFH *cf;

    cf = (struct CFH *) dp->dp_Arg1;

    /* The possibility of an old, unreincarnated fh exists. */

    if (cf->CFH_ConnectionNumber < m->mfsi_ConnectionNumber)
    {
        dp->dp_Res1 = DOSTRUE;
        dp->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
        return(FALSE);
    }

    tp->tp_Arg1 = (LONG)cf->CFH_ServerFH;

    return(TRUE);

}


/*
 * EndParent, EndCopyDir, EndLocateObject, EndCreateDir
 *
 */

void EndParent(struct TPacket *tp, struct MountedFSInfoClient *m)
{
    struct DosPacket *dp=tp->tp_DosPacket;
    struct CLock *cl;

    if (tp->tp_Res1)        /* Successful! */
    {
        cl = (struct CLock *) AllocMem(sizeof(struct CLock),MEMF_CLEAR|MEMF_PUBLIC);
        if (!cl)
        {
            ReplyPkt(dp,DOSFALSE,ERROR_NO_FREE_STORE);
            return;
        }
        cl->CLock_ConnectionNumber = m->mfsi_ConnectionNumber;
        cl->CLock_ServerLock = (struct FileLock *) tp->tp_Res1;
        cl->CLock_FileLock.fl_Key = tp->tp_Res1;
        if (dp->dp_Type == ACTION_LOCATE_OBJECT)
            cl->CLock_FileLock.fl_Access = dp->dp_Arg3;
        else
        {
            if ( (dp->dp_Arg1) && (dp->dp_Type != ACTION_COPY_DIR_FH) )
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
        return;
    }
    else                    /* Not successful, or Parent reached root */
    {
        ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
    }
}


/* StartDeleteObject
 *
 * ACTION_DELETE_OBJECT
 *
 */
BOOL StartDeleteObject(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
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

    while ((ULONG) gp & 3L)
        gp++;

    tp->tp_Arg2 = ((ULONG) gp - (ULONG) tp);

    CopyMem(BADDR(dp->dp_Arg2),gp,((UBYTE *)BADDR(dp->dp_Arg2))[0]+1);
    FixBString(gp);
    gp = &gp[((UBYTE *)BADDR(dp->dp_Arg2))[0]+1];

    return(TRUE);

}



/* StartLocateObject
 *
 * ACTION_LOCATE_OBJECT
 *
 */
BOOL StartLocateObject(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
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

    while ((ULONG) gp & 3L)
        gp++;

    tp->tp_Arg2 = ((ULONG) gp - (ULONG) tp);

    CopyMem(BADDR(dp->dp_Arg2),gp,((UBYTE *)BADDR(dp->dp_Arg2))[0]+1);
    FixBString(gp);
    slen = gp[0];
    gp += gp[0]+1;

    tp->tp_Arg3 = dp->dp_Arg3;
    tp->tp_Arg4 = m->mfsi_Flags & FLAGSMASK;

    t->trans_ReqDataActual = ((ULONG)gp - (ULONG)tp);

/* Special casing for Mike Sinz */
/* If the caller is attempting to get a lock on the root of a
 * volume, indicated by a null string length (after the : and
 * previous is removed), and null reference lock, create a
 * lock structure for them immediately, and don't bother to
 * contact the server.
 */
    if ((!cl) && (!slen))
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


/* StartExamine
 *
 * ACTION_Examine, ExNext
 *
 */
BOOL StartExamine(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
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

    while ((ULONG) gp & 3L)
        gp++;

    tp->tp_Arg2 = ((ULONG) gp - (ULONG) tp);
    tp->tp_Arg4 = (m->mfsi_Flags & FLAGSMASK);

    CopyMem(BADDR(dp->dp_Arg2),gp,sizeof(struct FileInfoBlock));

    t->trans_ReqDataActual =  (ULONG)(&gp[sizeof(struct FileInfoBlock)]) - (ULONG) tp;

    return(TRUE);
}


/*
 *  EndExamine
 *
 */

void EndExamine(struct TPacket *tp, struct MountedFSInfoClient *m)
{
    struct DosPacket *dp=tp->tp_DosPacket;

    if (tp->tp_Res1)                /* If sucessful */
    {
        UBYTE *efrom;
        efrom = &((UBYTE *)tp)[sizeof(struct TPacket)];
        while ((ULONG) efrom & 3L)
            efrom++;

        CopyMem(efrom,BADDR(dp->dp_Arg2),sizeof(struct FileInfoBlock));
    }

    ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
}


/* StartSameLock
 *
 * ACTION_SAME_LOCK
 *
 */
BOOL StartSameLock(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
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
BOOL StartInfo(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
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
BOOL StartChangeMode(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
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

        if (cf->CFH_ConnectionNumber < m->mfsi_ConnectionNumber)
        {
            dp->dp_Res1 = DOSTRUE;
            dp->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
            return(FALSE);
        }
        tp->tp_Arg2 = (LONG)cf->CFH_ServerFH;
    }

    return(TRUE);
}



/*
 *  EndInfo
 *
 */

void EndInfo(struct TPacket *tp, struct MountedFSInfoClient *m)
{
    struct DosPacket *dp=tp->tp_DosPacket;

    if (tp->tp_Res1)                /* If sucessful */
    {
        UBYTE *efrom;
        struct InfoData *i;
        efrom = &((UBYTE *)tp)[sizeof(struct TPacket)];
        while ((ULONG) efrom & 3L)
            efrom++;

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
BOOL StartFlush(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
                struct MountedFSInfoClient *m)
{
    return(TRUE);
}




/* StartDiskInfo
 *
 * ACTION_DISK_INFO
 *
 */
BOOL StartDiskInfo(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
                   struct MountedFSInfoClient *m)
{

    tp->tp_Arg1 = sizeof(struct TPacket);

    return(TRUE);
}


BOOL StartToID(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
               struct MountedFSInfoClient *m)
{
    STRPTR w;
    tp->tp_Arg1 = sizeof(struct TPacket);
    w = (STRPTR) ((ULONG)(tp) + sizeof(struct TPacket));
    strncpy(w,dp->dp_Arg1,33);
    return(TRUE);
}

BOOL StartToInfo(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
                 struct MountedFSInfoClient *m)
{

    tp->tp_Arg1 = dp->dp_Arg1;
    tp->tp_Arg2 = sizeof(struct TPacket);
    return(TRUE);
}

void EndToInfo(struct TPacket *tp, struct MountedFSInfoClient *m)
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

void EndDiskInfo(struct TPacket *tp, struct MountedFSInfoClient *m)
{
    struct DosPacket *dp=tp->tp_DosPacket;

    if (tp->tp_Res1)                /* If sucessful */
    {
        UBYTE *efrom;
        struct InfoData *i;
        efrom = &((UBYTE *)tp)[sizeof(struct TPacket)];
        while ((ULONG) efrom & 3L)
            efrom++;

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

BOOL StartOpenFromLock(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
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
BOOL StartLockFromFH(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
                     struct MountedFSInfoClient *m)
{
    struct CFH *cf;

    cf = (struct CFH *) dp->dp_Arg1;

    /* The possibility of an old, unreincarnated fh exists. */

    if (cf->CFH_ConnectionNumber < m->mfsi_ConnectionNumber)
    {
        dp->dp_Res1 = DOSTRUE;
        dp->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
        return(FALSE);
    }

    tp->tp_Arg1 = (LONG) cf->CFH_ServerFH;

    return(TRUE);

}


/*
 * StartReadLink
 *
 * ACTION_READ_LINK
 *
 */
BOOL StartReadLink(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
                   struct MountedFSInfoClient *m)
{
//    struct CLock *cl;
//    UBYTE *gp;
    dp->dp_Res1 = -1L;
    dp->dp_Res2 = ERROR_ACTION_NOT_KNOWN;

    return(FALSE);
}




BOOL DoStart(LONG type,struct Transaction *t, struct TPacket *tp, struct MountedFSInfoClient *m)
{
    struct DosPacket *dp;
    dp = (struct DosPacket *) tp->tp_DosPacket;
    switch(type)
    {

        case ACTION_READ_LINK:
            return(StartReadLink(t,tp,dp,m));

        case ACTION_INFO:
            return(StartInfo(t,tp,dp,m));

        case ACTION_DISK_INFO:
            return(StartDiskInfo(t,tp,dp,m));

        case ACTION_READ:
            return(StartRead(t,tp,dp,m));

        case ACTION_WRITE:
            return(StartWrite(t,tp,dp,m));

        case ACTION_CHANGE_MODE:
            return(StartChangeMode(t,tp,dp,m));

        case ACTION_EXAMINE_NEXT:
        case ACTION_EXAMINE_OBJECT:
            return(StartExamine(t,tp,dp,m));

        case ACTION_FINDINPUT:
        case ACTION_FINDOUTPUT:
        case ACTION_FINDUPDATE:
            return(StartFIND(t,tp,dp,m));

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

        default:
        {
            dp->dp_Res1 = DOSFALSE;
            dp->dp_Res2 = ERROR_ACTION_NOT_KNOWN;
            return(FALSE);
            break;
        }
    }
}





void DoEnd(LONG type, struct Transaction *t, struct MountedFSInfoClient *m)
{
    struct TPacket *tp;
    tp = (struct TPacket *) t->trans_ResponseData;

    switch(type)
    {

//        case ACTION_READ_LINK:
//            EndReadLink(tp,m);
//            return;

        case ACTION_INFO:
            EndInfo(tp,m);
            return;

        case ACTION_DISK_INFO:
            EndDiskInfo(tp,m);
            return;

        case ACTION_EXAMINE_NEXT:
        case ACTION_EXAMINE_OBJECT:
            EndExamine(tp,m);
            return;

        case ACTION_FH_FROM_LOCK:
        case ACTION_FINDINPUT:
        case ACTION_FINDOUTPUT:
        case ACTION_FINDUPDATE:
            EndFind(tp,m);
            return;

        case ACTION_SEEK:
            EndSeek(tp,m);
            return;

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
            EndEnd(tp,m);
            return;

        case ACTION_FREE_LOCK:
            EndFreeLock(tp,m);
            return;

        case ACTION_PARENT_FH:
        case ACTION_PARENT:
        case ACTION_COPY_DIR:
        case ACTION_COPY_DIR_FH:
        case ACTION_CREATE_DIR:
        case ACTION_LOCATE_OBJECT:
            EndParent(tp,m);
            return;

        case ACTION_READ:
            EndRead(t,tp,m);                /* Special case! */
            return;

        case ACTION_FLUSH:
            EndEnd(tp,m);
            return;

        case ACTION_WRITE:
            EndWrite(t,tp,m);               /* Special case! */
            return;

        case ACTION_UID_TO_USERINFO:
        case ACTION_GID_TO_GROUPINFO:
            EndToInfo(tp,m);
            return;

        default:
        {
            break;
        }
    }
}

/*
 * NetDownRequest()
 *
 * Returns TRUE for "retry", and FALSE for "cancel".
 *
 */

BOOL NetDownRequest(UBYTE *machine, UBYTE *volume)
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

BOOL MountFailedRequest(UBYTE *machine, UBYTE *volume, ULONG err)
{
    BOOL returnval = FALSE;

    IntuitionBase = OpenLibrary("intuition.library",0L);
    if (IntuitionBase)
    {
        ULONG args[3];
        UBYTE *reason="<Unknown reason>";
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

BOOL CantConnectRequest(UBYTE *machine)
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

BOOL GPRequest(ULONG num)
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


LONG TimeOutEasyRequest(struct Window *ref, struct EasyStruct *ez, ULONG id, APTR args, ULONG timeout)
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
                        ULONG waitmask,found;

                        waitmask = ( (1 << rp->mp_SigBit) | (1 << reqwin->UserPort->mp_SigBit) );
                        found = Wait(waitmask);
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


