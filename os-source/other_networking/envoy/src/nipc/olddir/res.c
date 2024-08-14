/**************************************************************************
**
** resolver.c   - NIPC Simple Host Name Resolver functions
**
** Copyright 1992, Commodore-Amiga, Inc.
**
** $Id: resolver.c,v 1.8 92/03/27 14:14:12 kcd Exp Locker: kcd $
**
***************************************************************************

/*------------------------------------------------------------------------*/

#include "nipcinternal.h"
#include "externs.h"

#include <string.h>

#include <clib/exec_protos.h>
#include <clib/utility_protos.h>
#include "nipcinternal_protos.h"

#include <pragmas/exec_pragmas.h>
#include <pragmas/utility_pragmas.h>

/*------------------------------------------------------------------------*/

/* Structures used by the resolver */

struct NameQueryHeader
{
	UWORD	nqh_ID;
	UWORD	nqh_Class;
	UWORD	nqh_SubClass;
	UBYTE	nqh_RealmLength;
	UBYTE	nqh_HostLength;
	UBYTE	nqh_ServiceLength;
	UBYTE	nqh_EntityLength;
	UBYTE	nqh_Data[1];
};	

struct NameResponseHeader
{
	UWORD					nrh_ID;
	UWORD					nrh_Class;
	UWORD					nrh_SubClass;
	UBYTE					nrh_NumAnswers;
	struct NameQueryHeader *nrh_Data;
};	

struct NameHeader
{
	struct Message	nh_Message;
	UWORD			nh_TTL;
	UWORD			nh_Flags;
};	

#define NQH_Host_Query		1			/* A query about a single host. */
#define NQH_Realm_Query		2			/* A Realm-wide query. */

#define NQH_Name		1				/* Address			-> Zone:Hostname */
#define NQH_Numbers		2				/* Realm:Hostname	-> Address(es) */
#define NQH_Services	3				/* Realm:Hostname	-> Service(s) */
#define NQH_Entities	4				/* Realm:Hostname	-> Entities */

#define NQH_Realm_Names      1			/* Address          -> Names of Realms    */
#define NQH_Realm_Networks   2			/* Realm: -> Address of Realm   */
#define NQH_Realm_Services   3			/* Realm: -> Hosts with Service */
#define NQH_Realm_Entities   4			/* Realm: -> Hosts with Entity */

struct SimpleNameInfo
{
    struct Message  sni_Msg;
    ULONG           sni_IPNum;
    UWORD           sni_TTL;
    UBYTE           sni_Type;
    UBYTE           sni_ID;
};

struct NetResolveName
{
    struct Message          nrn_Msg;
    ULONG                   nrn_IP;
    UBYTE                   nrn_Name[256];
    UBYTE                   nrn_CName[256];
    struct NetResolveName  *nrn_Parent;
    ULONG                   nrn_Timer;
    UWORD                   nrn_ID;
    UWORD                   nrn_ParentID;
    UWORD                   nrn_State;
};

struct NetCache
{
    ULONG   nc_IP;
    UBYTE   nc_CName[256];
};

#define NetRes_NORMAL 1
#define NetRes_SERVER 2
#define NetRes_NESTED 4

#define SNI_REQ 1
#define SNI_DAT 2

extern struct MsgPort * __asm DNSInit(register __d0 ULONG ServerIP);
extern void __asm DNSDeinit(void);
extern void __asm DNSTimer(void);
extern void __asm DoDNS(void);


/*------------------------------------------------------------------------*/

void InitResolver()
{
    gb->ResID = 1;
    InitSemaphore(&gb->HostNamesLock);
    InitSemaphore(&gb->RequestLock);
    InitSemaphore(&gb->DNSNamesLock);

    NewList((struct List *)&gb->SimpleNames);
    NewList((struct List *)&gb->RequestList);
    NewList((struct List *)&gb->DNSNames);

    gb->SimpleNamePort = OpenUDPPort(1,&ResolverInput);
    gb->ResolverPort = CreateMsgPort();
    gb->resolversigmask = (1 << gb->ResolverPort->mp_SigBit);

/*    gb->DNSPort = DNSInit(0x87070A01); */

/*    if(gb->DNSPort)
        gb->resolversigmask |= (1<< gb->DNSPort->mp_SigBit); */
}

/*------------------------------------------------------------------------*/

VOID DeinitResolver()
{
    struct SimpleNameInfo *sni,*n1;
    struct NetResolveName *nrn,*n2;

    sni = (struct SimpleNameInfo *) gb->SimpleNames.mlh_Head;
    while(sni->sni_Msg.mn_Node.ln_Succ)
    {
        n1 = (struct SimpleNameInfo *) sni->sni_Msg.mn_Node.ln_Succ;
        FreeMem(sni, sizeof(struct SimpleNameInfo));
        sni = n1;
    }

    sni = (struct SimpleNameInfo *) gb->RequestList.mlh_Head;
    while(sni->sni_Msg.mn_Node.ln_Succ)
    {
        n1 = (struct SimpleNameInfo *) sni->sni_Msg.mn_Node.ln_Succ;
        FreeMem(sni, sizeof(struct SimpleNameInfo));
        sni = n1;
    }

    nrn = (struct NetResolveName *) gb->DNSNames.mlh_Head;
    while(nrn->nrn_Msg.mn_Node.ln_Succ)
    {
        n2 = (struct NetResolveName *) nrn->nrn_Msg.mn_Node.ln_Succ;
        FreeMem(nrn, sizeof(struct NetResolveName));
        nrn = n2;
    }

    if(gb->DNSPort)
        DNSDeinit();

    if(gb->SimpleNamePort)
        CloseUDPPort(gb->SimpleNamePort);

    if(gb->ResolverPort)
        DeleteMsgPort(gb->ResolverPort);

    return;
}

/*------------------------------------------------------------------------*/

ULONG ResolveName(hostname)
UBYTE *hostname;
{
    struct SimpleNameInfo *sni;
    struct NetResolveName *nrn;
    struct MsgPort *myport;
    ULONG namelen, ipnum, reqlen,nbufflen;


    if (ipnum = lookuphost(hostname))
        return ipnum;

    ObtainSemaphore(&gb->HostNamesLock);
    sni = (struct SimpleNameInfo *) gb->SimpleNames.mlh_Head;

    ipnum = 0;

    while(sni->sni_Msg.mn_Node.ln_Succ)
    {
        if(!Stricmp(sni->sni_Msg.mn_Node.ln_Name,hostname))
        {
            ipnum = sni->sni_IPNum;
            ReleaseSemaphore(&gb->HostNamesLock);
            return ipnum;
        }
        sni = (struct SimpleNameInfo *) sni->sni_Msg.mn_Node.ln_Succ;
    }
    ReleaseSemaphore(&gb->HostNamesLock);

    ObtainSemaphore(&gb->DNSNamesLock);
    nrn = (struct NetResolveName *) gb->SimpleNames.mlh_Head;

    while(nrn->nrn_Msg.mn_Node.ln_Succ)
    {
        if(!Stricmp(nrn->nrn_CName,hostname))
        {
            ipnum = sni->sni_IPNum;
            ReleaseSemaphore(&gb->DNSNamesLock);
            return ipnum;
        }
        nrn = (struct NetResolveName *) nrn->nrn_Msg.mn_Node.ln_Succ;
    }
    ReleaseSemaphore(&gb->DNSNamesLock);

    namelen = strlen(hostname)+2;

    nbufflen = (namelen & 0xfffffffe);

    reqlen = sizeof(struct SimpleNameInfo) + nbufflen;
    if(sni = AllocMem(reqlen,MEMF_CLEAR|MEMF_PUBLIC))
    {
        if(myport=CreateMsgPort())
        {
            sni->sni_IPNum = 0;
            sni->sni_TTL = 10;                  /* 1 second timeout */
            sni->sni_ID  = gb->ResID++;
            sni->sni_Msg.mn_Length = reqlen;
            sni->sni_Msg.mn_Node.ln_Name = (UBYTE *) (ULONG) sni + (ULONG) sizeof(struct SimpleNameInfo);
            sni->sni_Msg.mn_ReplyPort = myport;
            CopyMem(hostname,(UBYTE *) (ULONG) sni + (ULONG) sizeof(struct SimpleNameInfo),nbufflen);
            PutMsg(gb->ResolverPort,(struct Message *)sni);
            WaitPort(myport);
            sni = (struct SimpleNameInfo *) GetMsg(myport);
            if(ipnum = sni->sni_IPNum)
            {
                ObtainSemaphore(&gb->HostNamesLock);
                sni->sni_TTL=600;
                AddHead((struct List *)&gb->SimpleNames,(struct Node *)sni);
                ReleaseSemaphore(&gb->HostNamesLock);
            }
            DeleteMsgPort(myport);
        }
        if(!ipnum) FreeMem(sni,reqlen);
    }

    if((!ipnum) && (gb->DNSPort))
    {
        if(nrn = AllocMem(sizeof(struct NetResolveName),MEMF_CLEAR|MEMF_PUBLIC))
        {
            if(myport = CreateMsgPort())
            {
                nrn->nrn_Msg.mn_ReplyPort = myport;
                strcpy(nrn->nrn_CName,hostname);
                PutMsg(gb->DNSPort,(struct Message *)nrn);
                WaitPort(myport);
                nrn = (struct NetResolveName *) GetMsg(myport);
                if(ipnum = nrn->nrn_IP)
                {
                    ObtainSemaphore(&gb->DNSNamesLock);
                    nrn->nrn_Timer=600;
                    AddHead((struct List *)&gb->DNSNames,(struct Node *)nrn);
                    ReleaseSemaphore(&gb->DNSNamesLock);
                }
                DeleteMsgPort(myport);
            }
            if(!ipnum) FreeMem(nrn,sizeof(struct NetResolveName));
        }
    }

    return ipnum;

}

/*------------------------------------------------------------------------*/

void DoResolver()
{
    struct SimpleNameInfo *sni;

    while(sni = (struct SimpleNameInfo *) GetMsg(gb->ResolverPort))
    {
        sni->sni_Msg.mn_Node.ln_Type = SNI_REQ;
        UDP_Output((UBYTE *)sni,sni->sni_Msg.mn_Length,0xffffffff,0,1);
        AddTail((struct List *)&gb->RequestList,(struct Node *)sni);
    };

    DoDNS();
}






/*------------------------------------------------------------------------*/

VOID ResolverInput(conn, buff)
struct UDPConnection *conn;
struct Buffer *buff;
{
    struct BuffEntry *be;
    struct udphdr *udp;
    struct iphdr *ip;
    struct SimpleNameInfo *sni;
    struct SimpleNameInfo *temp;
    struct SimpleNameInfo *next;
    UBYTE *str;
    ULONG our_ip;

    be = (struct BuffEntry *) buff->buff_list.mlh_Head;
    ip = (struct iphdr *) ( (ULONG) be->be_data + (ULONG) be->be_offset);
    udp = (struct udphdr *) ( (ULONG) ip + (ULONG) sizeof(struct iphdr));

    sni = (struct SimpleNameInfo *) ((ULONG) udp + (ULONG) sizeof(struct udphdr));

    str = (UBYTE *) (ULONG) sni + (ULONG) sizeof(struct SimpleNameInfo);

    if(sni->sni_Msg.mn_Length >= sizeof(struct SimpleNameInfo))
    {
        if(sni->sni_Msg.mn_Node.ln_Type == SNI_REQ)
        {
            if(our_ip = lookuphost((UBYTE *) (ULONG) sni + (ULONG) sizeof(struct SimpleNameInfo)))
            {
                sni->sni_IPNum = our_ip;
                sni->sni_TTL = 50;
                sni->sni_Msg.mn_Node.ln_Type = SNI_DAT;
                UDP_Output((UBYTE *)sni,sni->sni_Msg.mn_Length,ip->iph_SrcAddr,0,1);
            }
        }
        else
        {
            if(sni->sni_Msg.mn_Node.ln_Type == SNI_DAT)
            {
                temp = (struct SimpleNameInfo *) gb->RequestList.mlh_Head;
                while(temp->sni_Msg.mn_Node.ln_Succ)
                {
                    next = (struct SimpleNameInfo *) temp->sni_Msg.mn_Node.ln_Succ;
                    if(temp->sni_ID == sni->sni_ID)
                    {
                        if(!Stricmp(str,temp->sni_Msg.mn_Node.ln_Name))
                        {
                            Remove((struct Node *)temp);
                            temp->sni_IPNum = sni->sni_IPNum;
                            temp->sni_TTL = sni->sni_TTL;
                            ReplyMsg((struct Message *)temp);
                        }
                    }
                    temp = next;
                }
            }
        }
    }
    FreeBuffer(buff);
}

/*------------------------------------------------------------------------*/

void ResolverTimeout()
{
    struct SimpleNameInfo *sni;
    struct NetResolveName *nrn;
    APTR next;

    sni = (struct SimpleNameInfo *) gb->RequestList.mlh_Head;
    while(sni->sni_Msg.mn_Node.ln_Succ)
    {
        next = (struct SimpleNameInfo *) sni->sni_Msg.mn_Node.ln_Succ;
        sni->sni_TTL -= 1;
        if(!sni->sni_TTL)
        {
            Remove((struct Node *)sni);
            sni->sni_IPNum = 0;
            ReplyMsg((struct Message *)sni);
        }
        sni = next;
    }

    ObtainSemaphore(&gb->HostNamesLock);

    sni = (struct SimpleNameInfo *) gb->SimpleNames.mlh_Head;
    while(sni->sni_Msg.mn_Node.ln_Succ)
    {
        next = (struct SimpleNameInfo *) sni->sni_Msg.mn_Node.ln_Succ;
        sni->sni_TTL -= 1;
        if(!sni->sni_TTL)
        {
            Remove((struct Node *)sni);
            FreeMem(sni,sni->sni_Msg.mn_Length);
        }
        sni = next;
    }
    ReleaseSemaphore(&gb->HostNamesLock);

    ObtainSemaphore(&gb->DNSNamesLock);

    if(gb->DNSPort)
    {
        nrn = (struct NetResolveName *) gb->DNSNames.mlh_Head;
        while(nrn->nrn_Msg.mn_Node.ln_Succ)
        {
            next = (struct NetResolveName *) nrn->nrn_Msg.mn_Node.ln_Succ;
            nrn->nrn_Timer -= 1;
            if(!nrn->nrn_Timer)
            {
                Remove((struct Node *)nrn);
                FreeMem(nrn,sizeof(struct NetResolveName));
            }
            nrn = next;
        }
        ReleaseSemaphore(&gb->DNSNamesLock);
        DNSTimer();
    }
}

/*------------------------------------------------------------------------*/
