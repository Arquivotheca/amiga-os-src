/**************************************************************************
**
** resolver.c   - NIPC Simple Host Name Resolver functions
**
** Copyright 1992, Commodore-Amiga, Inc.
**
** $Id: resolver.c,v 1.23 92/06/08 09:55:30 kcd Exp $
**
***************************************************************************

/*------------------------------------------------------------------------*/

#include "nipcinternal.h"
#include "externs.h"

#include <utility/hooks.h>
#include <string.h>
#include <clib/exec_protos.h>
#include <clib/utility_protos.h>
#include <clib/iffparse_protos.h>
#include "nipcinternal_protos.h"

#include <pragmas/exec_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

/*------------------------------------------------------------------------*/

/* Structures used by the resolver */

struct NameHeader
{
    ULONG   nh_Reserved0;
    UWORD   nh_ID;
    UBYTE   nh_Type;
    UBYTE   nh_Class;
    UWORD   nh_Length;
    UBYTE   nh_Realms;
    UBYTE   nh_Hosts;
    UBYTE   nh_Services;
    UBYTE   nh_Entities;
};

struct ServicesLib
{
    struct Library           SVCS_Lib;
    APTR                     SVCS_NIPCBase;
    APTR                     SVCS_UtilityBase;
    struct ExecBase         *SVCS_SysBase;
    BPTR                     SVCS_SegList;

    /* Services Library and Services Manager Common */
    struct SignalSemaphore   SVCS_ServicesLock;
    struct MinList           SVCS_Services;
    struct SignalSemaphore   SVCS_OpenLock;
};

struct Service
{
    struct Node     svc_Node;                   /* Chain of services */
    UBYTE           svc_PathName[256];          /* Who to run to start a server */
    UBYTE           svc_SvcName[64];            /* Name of this service */
    ULONG           svc_Flags;                  /* Various Flags */
    UWORD           svc_UID;                    /* User allowed to use this service */
    UWORD           svc_GID;                    /* Group allowed to use this service */
};

#define SVCB_EXPUNGE    1
#define SVCF_EXPUNGE    (1 << SVCB_EXPUNGE)

#define NHType_Query        1           /* A query about a single host. */
#define NHType_Response     2           /* A Realm-wide query. */
#define NHType_Transit      3           /* A query in transit to a realm server. */
#define NHType_Error        255

#define REALM_AUTHORITY 0x01

#define NHClass_Numbers     255         /* Realm:[Hostname]   -> Address(es) */

#define NHClass_Hosts       1           /* [Realm] & (Service|Entity) -> Hosts */
#define NHClass_Services    2           /* [Realm]:Hostname -> Services */
#define NHClass_Entities    3           /* [Realm]:Hostname -> Entities */
#define NHClass_Realms      4           /* If(RealmServer) -> Realms */

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

#define NetRes_NORMAL 1
#define NetRes_SERVER 2
#define NetRes_NESTED 4

#define NQ_Realm 1000
#define NQ_Host  1001
#define NQ_Service 1002
#define NQ_Entity 1003


/*------------------------------------------------------------------------*/

BOOL InitResolver()
{
    register struct NBase *nb = gb;
    struct NIPCRealmPrefs *nzp;
    struct NIPCIFFHostPrefs *nhp;
    struct NIPCRoute *route;
    struct CollectionItem *ci;
    struct Realm *realm;

    nb->ResID = 1;
    InitSemaphore(&nb->ResponseCacheLock);
    InitSemaphore(&nb->RequestLock);
    InitSemaphore(&nb->DNSResolverLock);
    InitSemaphore(&nb->RealmListLock);

    NewList((struct List *)&gb->ResponseCache);
    NewList((struct List *)&gb->RequestList);
    NewList((struct List *)&gb->DNSNames);
    NewList((struct List *)&gb->RealmList);

    nb->NameRequestPort = OpenUDPPort(1,&ResolverInput);
    nb->ResolverPort = CreateMsgPort();
    nb->resolversigmask = (1 << gb->ResolverPort->mp_SigBit);
    nb->RIPTimer = -1;
    nb->DNSHook = NULL;

    ci = FindCollection(nb->iff,ID_PREF,ID_NLRM);
    while(ci)
    {
        if(realm = AllocMem(sizeof(struct Realm),MEMF_PUBLIC|MEMF_CLEAR))
        {
            nzp = (struct NIPCRealmPrefs *)ci->ci_Data;
            route = Route(nzp->nzp_RealmAddr,TRUE);
            if(route)
                realm->realm_SubnetMask = route->nr_Mask;
            else
                realm->realm_SubnetMask = NetMask(nzp->nzp_RealmAddr);

            realm->realm_NetworkIP = nzp->nzp_RealmAddr;
            realm->realm_Broadcast = nzp->nzp_RealmAddr | ~realm->realm_SubnetMask;
            strcpy(realm->realm_Name,nzp->nzp_RealmName);
            realm->realm_Flags = REALM_AUTHORITY;
            AddTail((struct List *)&nb->RealmList,(struct Node *)realm);
        }
        ci = ci->ci_Next;
    }
    ci = FindCollection(nb->iff,ID_PREF,ID_NRRM);
    while(ci)
    {
        if(realm = AllocMem(sizeof(struct Realm),MEMF_PUBLIC|MEMF_CLEAR))
        {
            nzp = (struct NIPCRealmPrefs *)ci->ci_Data;
            realm->realm_Server = nzp->nzp_RealmAddr;
            strcpy(realm->realm_Name,nzp->nzp_RealmName);
            AddTail((struct List *)&nb->RealmList,(struct Node *)realm);
        }
        ci = ci->ci_Next;
    }
    if(ci = FindCollection(nb->iff,ID_PREF,ID_HOST))
    {
        nhp = (struct NIPCHostPrefs *)ci->ci_Data;
        strcpy(nb->LocalHostName,nhp->nhp_HostName);
        strcpy(nb->RealmName,nhp->nhp_RealmName);
        nb->RealmServer = nhp->nhp_RealmServAddr;
    }
    return(TRUE);
}

/*------------------------------------------------------------------------*/

VOID DeinitResolver()
{
    register struct NBase *nb = gb;
    struct Node *node;
    struct NameInfo *ni, *ni_tmp;
    struct NameHeader *nh;

    ni = (struct NameInfo *)nb->RequestList.mlh_Head;
    while(ni->ni_Msg.mn_Node.ln_Succ)
    {
        ni_tmp = (struct NameInfo *) ni->ni_Msg.mn_Node.ln_Succ;
        nh = ni->ni_Header;
        FreeMem(nh,nh->nh_Length);
        FreeMem(ni, sizeof(struct NameInfo));
        ni = ni_tmp;
    }

    if(gb->DNSHook)
    {
        CallHookPkt(nb->DNSHook,(void *)-1L,(void *)-1L);
        ObtainSemaphore(&nb->DNSResolverLock);
        ReleaseSemaphore(&nb->DNSResolverLock);
    }

    if(gb->NameRequestPort)
        CloseUDPPort(nb->NameRequestPort);

    if(gb->ResolverPort)
        DeleteMsgPort(nb->ResolverPort);

    while(node = RemHead((struct List *)&nb->RealmList))
        FreeMem(node,sizeof(struct Realm));

    return;
}

/*------------------------------------------------------------------------*/
typedef ULONG (*HOOK_FUNC)();

ULONG ResolveName(hostname)
UBYTE *hostname;
{
    register struct NBase *nb = gb;
    struct Hook rn_hook;
    struct TagItem RNTags[3]={NQ_Realm,0,NQ_Host,0,TAG_DONE,0};
    STRPTR realm_ptr,host_ptr,search_ptr;
    UBYTE nbuff[130];
    ULONG ipnum;

    if(!hostname)
        return 0L;

    if((*hostname == 0) || (strlen(hostname) >128))
        return 0L;

    if(!Stricmp(hostname,nb->LocalHostName))
    {
        return ((struct sanadev *)gb->DeviceList.lh_Head)->sanadev_IPAddress;
    }
    strncpy(nbuff,hostname,128);

    search_ptr = nbuff;
    host_ptr = realm_ptr = NULL;

    while(*search_ptr)
    {
        switch(*search_ptr)
        {
            case ':'    : *search_ptr = 0;
                          host_ptr = search_ptr + 1;
                          realm_ptr = nbuff;
                          break;

            case 0      : break;

            default     : search_ptr++;
        }
    }
    if(!host_ptr)
        host_ptr = nbuff;

    else if(*host_ptr == 0)
        return 0;

    if(realm_ptr)
        if(*realm_ptr == 0)
            realm_ptr = NULL;

    ipnum = 0;

    rn_hook.h_Entry = (HOOK_FUNC) RNHookFunc;
    rn_hook.h_SubEntry = NULL;
    rn_hook.h_Data = &ipnum;

    RNTags[0].ti_Data = (ULONG) realm_ptr;
    RNTags[1].ti_Data = (ULONG) host_ptr;
    NetQueryA(&rn_hook,NHClass_Numbers,50,RNTags);
    return ipnum;
}

void __asm RNHookFunc(register __a0 struct Hook *hook,
                      register __a2 struct NameHeader *nh,
                      register __a1 void *nada)
{
    *(ULONG *)hook->h_Data = nh->nh_Reserved0;
}

/****************************************************************************/

void __asm NetQueryA(register __a0 struct Hook *hook,
                     register __d0 ULONG queryClass,
                     register __d1 ULONG maxTime,
                     register __a1 struct TagItem *tagList)
{
    register struct NBase *nb;
    struct NameInfo *ni;
    struct NameHeader *nh;
    struct MsgPort *myport;
    struct TagItem *tag;
    STRPTR Names[4],sp;
    ULONG NTags[4]={NQ_Realm,NQ_Host,NQ_Service,NQ_Entity};
    UBYTE Lengths[4],i;
    nb = gb;
    if(!(hook && tagList && maxTime && queryClass))
    {
        return;
    }
    if((queryClass == NHClass_Realms) && !(gb->RealmServer))
    {
        return;
    }
    for(i=0;i<4;i++)
    {
        Names[i] = NULL;
        Lengths[i]=0;
        if(tag = FindTagItem(NTags[i],tagList))
        {
            if(tag->ti_Data)
            {
                Names[i]=(STRPTR) tag->ti_Data;
                Lengths[i] = strlen(Names[i]);
                if(Lengths[i] > 64)
                    return;
                if(Lengths[i])
                    Lengths[i]++;
            }
        }
    }

    if((!Names[0]) && gb->RealmServer)
    {
        Names[0] = (STRPTR) &gb->RealmName;
        Lengths[0] = strlen(Names[0]) + 1;
    }
    else if((!Names[0][0]) && gb->RealmServer)
    {
        Names[0] = (STRPTR) &gb->RealmName;
        Lengths[0] = strlen(Names[0]) + 1;
    }
    if(myport = CreateMsgPort())
    {
        if(ni = (struct NameInfo *)AllocMem(sizeof(struct NameInfo),MEMF_CLEAR|MEMF_PUBLIC))
        {
            Forbid();
            ni->ni_ID = gb->ResID++;
            Permit();

            ni->ni_Msg.mn_ReplyPort = myport;

            ni->ni_HeaderLength = sizeof(struct NameHeader) + (Lengths[0] + Lengths[1] + Lengths[2] + Lengths[3]);
            if(nh = (struct NameHeader *)AllocMem(ni->ni_HeaderLength,MEMF_CLEAR|MEMF_PUBLIC))
            {
                ni->ni_Header = nh;
                ni->ni_TTL = maxTime >> 3; /* (HACK!) Supposed to be maxTime/10! */
                ni->ni_Hook = hook;
                nh->nh_ID = ni->ni_ID;
                nh->nh_Type = NHType_Query;
                nh->nh_Class = queryClass;
                nh->nh_Length = ni->ni_HeaderLength;
                nh->nh_Realms   = Lengths[0];
                nh->nh_Hosts    = Lengths[1];
                nh->nh_Services = Lengths[2];
                nh->nh_Entities  = Lengths[3];

                sp = (STRPTR) nh + sizeof(struct NameHeader);

                for(i=0;i<4;i++)
                {
                    if(Lengths[i])
                    {
                        strcpy(sp,Names[i]);
                    }
                    sp = (STRPTR) sp + Lengths[i];
                }
                PutMsg(nb->ResolverPort,(struct Message *)ni);
                WaitPort(myport);
                GetMsg(myport);
                FreeMem(ni->ni_Header,ni->ni_HeaderLength);
            }
            FreeMem(ni,sizeof(struct NameInfo));
        }
        DeleteMsgPort(myport);
    }
}
/*------------------------------------------------------------------------*/

void DoResolver()
{
    register struct NBase *nb = gb;
    struct NameInfo *ni;
    ULONG destIP;

    while(ni = (struct NameInfo *) GetMsg(nb->ResolverPort))
    {
        AddTail((struct List *)&nb->RequestList,(struct Node *)ni);
        if(ni->ni_Header->nh_Realms)
        {
            destIP = gb->RealmServer;
            ni->ni_Header->nh_Type = NHType_Transit;
        }
        else
            destIP = 0xffffffff;
        UDP_Output((UBYTE *)ni->ni_Header,ni->ni_HeaderLength,0,destIP,0,1);
    };
}

/*------------------------------------------------------------------------*/

BOOL InvalidRequest(struct NameHeader *nh);
void SendResponse(struct NameHeader *nh_req, struct NameHeader *nh_resp, ULONG len, ULONG DestIP, ULONG SrcIP);

VOID ResolverInput(conn, buff,dev)
struct UDPConnection *conn;
struct Buffer *buff;
struct sanadev *dev;
{
    register struct NBase *nb = gb;
    struct BuffEntry *be;
    struct udphdr *udp;
    struct iphdr *ip;
    struct NIPCRoute *route;
    struct NameHeader *nh,*nh_new;
    struct NameInfo *ni;
    struct Service *svc;
    struct Realm *realm,*realm_ptr;
    struct ServicesLib *ServicesBase = NULL;
    struct Node *node;
    UBYTE *str;
    ULONG len;
    BOOL match = FALSE;
    UtilityBase = (struct Library *)gb->nb_UtilityBase;

    be = (struct BuffEntry *) buff->buff_list.mlh_Head;
    ip = (struct iphdr *) ( (ULONG) be->be_data + (ULONG) be->be_offset);
    udp = (struct udphdr *) ( (ULONG) ip + (ULONG) sizeof(struct iphdr));

    nh = (struct NameHeader *) ((ULONG) udp + (ULONG) sizeof(struct udphdr));

    if(nh->nh_Type == NHType_Transit)
    {
        ObtainSemaphore(&nb->RealmListLock);
        if(IsListEmpty((struct List *)&gb->RealmList))
        {
            nh->nh_Type = NHType_Error;
            UDP_Output((UBYTE *)nh,sizeof(struct NameHeader),0,ip->iph_SrcAddr,0,1);
        }
        else
        {
            realm = (struct Realm *) gb->RealmList.mlh_Head;
            str = (STRPTR) nh + sizeof(struct NameHeader);
            while(realm->realm_Node.mln_Succ)
            {
                if(!Stricmp(str,realm->realm_Name))
                {
                    if(realm->realm_Flags & REALM_AUTHORITY)
                    {
                        nh->nh_Type = NHType_Query;
                        UDP_Output((UBYTE *)nh,nh->nh_Length,ip->iph_SrcAddr,realm->realm_Broadcast,0,1);
                    }
                    else
                    {
                        nh->nh_Type = NHType_Transit;
                        UDP_Output((UBYTE *)nh,nh->nh_Length,ip->iph_SrcAddr,realm->realm_Server,0,1);
                        break;
                    }
                }
                realm = (struct Realm *) realm->realm_Node.mln_Succ;
            }
        }
        ReleaseSemaphore(&nb->RealmListLock);
    }
    else if(nh->nh_Type == NHType_Query)
    {
        switch(nh->nh_Class)
        {
            case NHClass_Numbers:   str = (STRPTR) nh + sizeof(struct NameHeader) + nh->nh_Realms;
                                    if(!Stricmp(nb->LocalHostName,str))
                                    {
                                        if(route = Route(ip->iph_SrcAddr, TRUE))
                                        {
                                            nh->nh_Reserved0 = route->nr_Device->sanadev_IPAddress;
                                            nh->nh_Type = NHType_Response;
                                            UDP_Output((UBYTE *)nh,nh->nh_Length,0,ip->iph_SrcAddr,0,1);
                                        }
                                    }
                                    break;

            case NHClass_Hosts:     if(nh->nh_Realms)
                                    {
                                        str = (STRPTR) nh + sizeof(struct NameHeader);
                                        if(!gb->RealmServer)
                                            break;
                                        if(Stricmp(str,(STRPTR)&nb->RealmName))
                                            break;
                                    }

                                    if(nh->nh_Hosts)
                                        break;

                                    if(nh->nh_Services)
                                    {
                                        str = (STRPTR) nh + sizeof(struct NameHeader) + nh->nh_Realms;
                                        Forbid();
                                        if(nb->nb_ServicesBase)
                                            ServicesBase = (struct ServicesLib *)OpenLibrary("services.library",0L);
                                        Permit();
                                        if(ServicesBase)
                                        {
                                            ObtainSemaphore(&ServicesBase->SVCS_ServicesLock);
                                            svc = (struct Service *)ServicesBase->SVCS_Services.mlh_Head;
                                            while(svc->svc_Node.ln_Succ)
                                            {
                                                if(!Stricmp((STRPTR)&svc->svc_SvcName,str))
                                                {
                                                    if(!(svc->svc_Flags & SVCF_EXPUNGE))
                                                    {
                                                        match = TRUE;
                                                        break;
                                                    }
                                                }
                                                svc = (struct Service *)svc->svc_Node.ln_Succ;
                                            }
                                            ReleaseSemaphore(&ServicesBase->SVCS_ServicesLock);
                                            CloseLibrary((struct Library *)ServicesBase);
                                        }
                                    }
                                    else if(nh->nh_Entities)
                                    {
                                        match = FALSE;
                                        str = (STRPTR) nh + sizeof(struct NameHeader) + nh->nh_Realms;
                                        ObtainSemaphore(&nb->ANMPELSemaphore);
                                        if(FindName((struct List *)&nb->ANMPEntityList,str))
                                            match = TRUE;
                                        ReleaseSemaphore(&nb->ANMPELSemaphore);
                                    }
                                    else
                                        match = TRUE;

                                    if(match)
                                    {
                                      /*  if(nb->RealmServer)
                                            len = strlen(nb->RealmName) + 1;
                                        else */
                                            len = 0;
                                        len = len + strlen(nb->LocalHostName) + 1;
                                        if(nh_new = AllocMem((sizeof(struct NameHeader) + len),MEMF_CLEAR|MEMF_PUBLIC))
                                        {
                                            str = (STRPTR) nh_new + sizeof(struct NameHeader);
                                         /*   if(nb->RealmServer)
                                            {
                                                strcpy(str,nb->RealmName);
                                                str+= strlen(nb->RealmName);
                                                *str=':';
                                                str++;
                                            } */
                                            strcpy(str,nb->LocalHostName);
                                            SendResponse(nh,nh_new,sizeof(struct NameHeader) + len,ip->iph_SrcAddr,dev->sanadev_IPAddress);
                                        }
                                    }
                                    break;

            case NHClass_Services:
                                    if(InvalidRequest(nh))
                                        break;

                                    Forbid();
                                    if(nb->nb_ServicesBase)
                                        ServicesBase = (struct ServicesLib *)OpenLibrary("services.library",0L);
                                    Permit();
                                    if(ServicesBase)
                                    {
                                        ObtainSemaphore(&ServicesBase->SVCS_ServicesLock);
                                        svc = (struct Service *)ServicesBase->SVCS_Services.mlh_Head;
                                        len = 0;
                                        while(svc->svc_Node.ln_Succ)
                                        {
                                            len = len + strlen((STRPTR)&svc->svc_SvcName) +1;
                                            svc = (struct Service *)svc->svc_Node.ln_Succ;
                                        }
                                        if(nh_new = AllocMem((sizeof(struct NameHeader) + len),MEMF_CLEAR|MEMF_PUBLIC))
                                        {
                                            str = (STRPTR) nh_new + sizeof(struct NameHeader);
                                            svc = (struct Service *)ServicesBase->SVCS_Services.mlh_Head;
                                            while(svc->svc_Node.ln_Succ)
                                            {
                                                if(!(svc->svc_Flags & SVCF_EXPUNGE))
                                                {
                                                    strcpy(str,(STRPTR)&svc->svc_SvcName);
                                                    str+=(1 + strlen((STRPTR)&svc->svc_SvcName));
                                                    nh_new->nh_Services++;
                                                }
                                                svc = (struct Service *)svc->svc_Node.ln_Succ;
                                            }
                                        }
                                        ReleaseSemaphore(&ServicesBase->SVCS_ServicesLock);
                                        if(nh_new->nh_Services)
                                        {
                                            SendResponse(nh,nh_new,(ULONG)(str - (STRPTR)nh_new),ip->iph_SrcAddr,dev->sanadev_IPAddress);
                                        }
                                        CloseLibrary((struct Library *)ServicesBase);
                                    }
                                    else
                                    {
                                        if(nh_new = AllocMem((sizeof(struct NameHeader)),MEMF_CLEAR|MEMF_PUBLIC))
                                        {
                                            SendResponse(nh,nh_new,sizeof(struct NameHeader),ip->iph_SrcAddr,dev->sanadev_IPAddress);
                                        }
                                    }
                                    break;
            case NHClass_Entities:
                                    if(InvalidRequest(nh))
                                        break;

                                    ObtainSemaphore(&nb->ANMPELSemaphore);
                                    node = (struct Node *)gb->ANMPEntityList.mlh_Head;
                                    len = 0;
                                    while(node->ln_Succ)
                                    {
                                        if(((struct Entity *)node)->entity_Flags & ENTF_PUBLIC)
                                            len = len + strlen(node->ln_Name) + 1;
                                        node = node->ln_Succ;
                                    }
                                    if(nh_new = AllocMem((sizeof(struct NameHeader) + len),MEMF_CLEAR|MEMF_PUBLIC))
                                    {
                                        str = (STRPTR) nh_new + sizeof(struct NameHeader);
                                        node = (struct Node *)gb->ANMPEntityList.mlh_Head;
                                        while(node->ln_Succ)
                                        {
                                            if(((struct Entity *)node)->entity_Flags & ENTF_PUBLIC)
                                            {
                                                strcpy(str,node->ln_Name);
                                                str+=(1 + strlen(node->ln_Name));
                                                nh_new->nh_Entities++;
                                            }
                                            node = node->ln_Succ;
                                        }
                                    }
                                    ReleaseSemaphore(&nb->ANMPELSemaphore);
                                    if(nh_new)
                                    {
                                        SendResponse(nh,nh_new,(ULONG)(str - (STRPTR)nh_new),ip->iph_SrcAddr,dev->sanadev_IPAddress);
                                    }
                                    break;

            case NHClass_Realms:    if(!IsListEmpty((struct List *)&gb->RealmList))
                                    {
                                        realm_ptr = (struct Realm *)gb->RealmList.mlh_Head;
                                        len = 0;
                                        while(realm_ptr->realm_Node.mln_Succ)
                                        {
                                            len = len + strlen((STRPTR)&realm_ptr->realm_Name) + 1;
                                            realm_ptr = (struct Realm *)realm_ptr->realm_Node.mln_Succ;
                                        }
                                        if(nh_new = AllocMem((sizeof(struct NameHeader) + len),MEMF_CLEAR|MEMF_PUBLIC))
                                        {
                                            ObtainSemaphore(&nb->RealmListLock);

                                            realm_ptr = (struct Realm *)gb->RealmList.mlh_Head;
                                            str = (STRPTR) nh_new + sizeof(struct NameHeader);
                                            while(realm_ptr->realm_Node.mln_Succ)
                                            {
                                                strcpy(str,(STRPTR)&realm_ptr->realm_Name);
                                                str+=(1 + strlen(realm_ptr->realm_Name));
                                                nh_new->nh_Realms++;
                                                realm_ptr = (struct Realm *)realm_ptr->realm_Node.mln_Succ;
                                            }
                                            ReleaseSemaphore(&nb->RealmListLock);
                                            SendResponse(nh,nh_new,(ULONG)(str - (STRPTR)nh_new),ip->iph_SrcAddr,dev->sanadev_IPAddress);
                                        }
                                    }

        }
    }
    else if((nh->nh_Type == NHType_Response) || (nh->nh_Type == NHType_Error))
    {
        ni = (struct NameInfo *) gb->RequestList.mlh_Head;
        while(ni->ni_Msg.mn_Node.ln_Succ)
        {
            if(ni->ni_ID == nh->nh_ID)
            {
                if(nh->nh_Type == NHType_Response)
                {
                    CallHookPkt(ni->ni_Hook,nh,NULL);
                    if((nh->nh_Class == NHClass_Numbers) || (nh->nh_Class == NHClass_Entities) || (nh->nh_Class == NHClass_Services))
                    {
                        Remove((struct Node *)ni);
                        ReplyMsg((struct Message *)ni);
                    }
                    break;
                }
                else if(nh->nh_Type == NHType_Error)
                {
                    Remove((struct Node *)ni);
                    ni->ni_Error = 1;
                    ReplyMsg((struct Message *)ni);
                }
            }
            ni = (struct NameInfo *)ni->ni_Msg.mn_Node.ln_Succ;
        }
    }
    FreeBuffer(buff);
}
/*------------------------------------------------------------------------*/

BOOL InvalidRequest(struct NameHeader *nh)
{
    register struct NBase *nb = gb;
    STRPTR str;

    if(nh->nh_Realms)
    {
        str = (STRPTR) nh + sizeof(struct NameHeader);
        if(!gb->RealmServer)
            return TRUE;
        if(Stricmp(str,(STRPTR)&nb->RealmName))
            return TRUE;
    }
    if(!nh->nh_Hosts)
    {
        return TRUE;
    }
    str = (STRPTR) nh + sizeof(struct NameHeader) + nh->nh_Realms;
    if(Stricmp(str,nb->LocalHostName))
    {
        return TRUE;
    }
    if(nh->nh_Services || nh->nh_Entities)
    {
        return TRUE;
    }
    return FALSE;
}

void SendResponse(struct NameHeader *nh_req, struct NameHeader *nh_resp, ULONG len, ULONG DestIP, ULONG SrcIP)
{
    nh_resp->nh_Reserved0 = SrcIP;
    nh_resp->nh_ID = nh_req->nh_ID;
    nh_resp->nh_Class = nh_req->nh_Class;
    nh_resp->nh_Type = NHType_Response;
    nh_resp->nh_Length = len;
    UDP_Output((UBYTE *)nh_resp,len,0,DestIP,0,1);
    FreeMem(nh_resp,len);
}

void ResolverTimeout()
{
    struct NameInfo *ni;
    struct RIPHeader *rip;
    APTR next;
    if(gb->RIPTimer != -1)
    {
            /* HACK ALERT!!!!!! */
        if((!gb->RIPTimer) || (gb->RIPTimer >30))
        {
            gb->RIPTimer = 30;
            if(rip = (struct RIPHeader *) AllocMem(sizeof(struct RIPHeader),MEMF_CLEAR|MEMF_PUBLIC))
            {
                rip->rip_Command = 2;
                rip->rip_Version = 1;
                rip->rip_AddFam = 2;
                rip->rip_DestIP = 0x87070000;
                rip->rip_Metric = 1;
                UDP_Output((UBYTE *)rip,sizeof(struct RIPHeader),0,0xffffffff,520,520);
                FreeMem(rip,sizeof(struct RIPHeader));
            }
        }
        gb->RIPTimer--;
    }

    ni = (struct NameInfo *) gb->RequestList.mlh_Head;
    while(ni->ni_Msg.mn_Node.ln_Succ)
    {
        next = (struct NameInfo *) ni->ni_Msg.mn_Node.ln_Succ;
        ni->ni_TTL -= 1;
        if(!ni->ni_TTL)
        {
            Remove((struct Node *)ni);
            ReplyMsg((struct Message *)ni);
        }
        ni = next;
    }

}

/*------------------------------------------------------------------------*/
