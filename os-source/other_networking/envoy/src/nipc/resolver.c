/**************************************************************************
**
** resolver.c   - NIPC Simple Host Name Resolver functions
**
** Copyright 1992, Commodore-Amiga, Inc.
**
** $Id: resolver.c,v 1.47 93/10/06 19:23:09 kcd Exp $
**
***************************************************************************

/*------------------------------------------------------------------------*/

#include "nipcinternal.h"
#include "externs.h"
#include <exec/execbase.h>
#include <graphics/gfxbase.h>
#include <utility/hooks.h>
#include <string.h>
#include <clib/exec_protos.h>
#include <clib/utility_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/timer_protos.h>
#include "nipcinternal_protos.h"

#include <pragmas/exec_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/timer_pragmas.h>

/*------------------------------------------------------------------------*/

/* Structures used by the resolver */

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

struct TimedResponse
{
    struct timerequest  tr_TimerIO;
    ULONG               tr_Length;
    ULONG               tr_Destination;
    struct InquiryHeader *tr_Buffer;
};

#define SVCB_ENABLE	0
#define SVCB_EXPUNGE    1

#define SVCF_ENABLE	(1 << SVCB_ENABLE)
#define SVCF_EXPUNGE    (1 << SVCB_EXPUNGE)

extern void dbprint(STRPTR message);

/*------------------------------------------------------------------------*/

BOOL InitResolver()
{
    register struct NBase *nb = gb;
    struct NIPCRealmPrefs *nzp;
    struct NIPCHostPrefs *nhp;
    struct NIPCRoute *route;
    struct CollectionItem *ci;
    struct Realm *realm;
    struct EClockVal ev;

    nb->ResID = 1;
    InitSemaphore(&nb->ResponseCacheLock);
    InitSemaphore(&nb->RequestLock);
    InitSemaphore(&nb->RealmListLock);

    NewList((struct List *)&gb->ResponseCache);
    NewList((struct List *)&gb->RequestList);
    NewList((struct List *)&gb->RealmList);

    nb->NameRequestPort = OpenUDPPort(NIP_UDP_PORT,&ResolverInput);
    nb->ResolverPort = CreateMsgPort();
    nb->NameResponsePort = CreateMsgPort();
    nb->resolversigmask = (1 << gb->ResolverPort->mp_SigBit) | (1 << gb->NameResponsePort->mp_SigBit);
    nb->RIPTimer = 65535;
    nb->DNSHook = NULL;

    ReadEClock(&ev);
    nb->TimerSeed = ev.ev_lo;
    nb->IsRealmServer = 1;

    if(ci = FindCollection(nb->iff,ID_PREF,ID_HOST))
    {
        nhp = (struct NIPCHostPrefs *)ci->ci_Data;
        strcpy(nb->LocalHostName,nhp->nhp_HostName);
        strcpy(nb->RealmName,nhp->nhp_RealmName);
        nb->RealmServer = nhp->nhp_RealmServAddr;
        if(ci->ci_Size > 132)
        {
            strcpy(nb->OwnerName,nhp->nhp_OwnerName);
            if(!(nhp->nhp_HostFlags & NHPFLAGF_REALMS))
            {
            	nb->RealmServer = 0L;
            	nb->RealmName[0] = '\0';
            }
            if(!(nhp->nhp_HostFlags & NHPFLAGF_REALMSERVER))
            {
            	nb->IsRealmServer = 0;
            }
        }
    }

    if(nb->IsRealmServer)
    {
        ci = FindCollection(nb->iff,ID_PREF,ID_NLRM);
        while(ci)
        {
            if(realm = AllocMem(sizeof(struct Realm),MEMF_PUBLIC|MEMF_CLEAR))
            {
                nzp = (struct NIPCRealmPrefs *)ci->ci_Data;
                route = Route(nzp->nzp_RealmAddr,TRUE);
                if(route)
                {
                    realm->realm_SubnetMask = route->nr_Mask;
                    realm->realm_SrcIP = route->nr_Device->sanadev_IPAddress;
                }
                else
                {
                    realm->realm_SubnetMask = NetMask(nzp->nzp_RealmAddr);
                    realm->realm_SrcIP = 0xffffffff;
                }

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
    }
    return(TRUE);
}

/*------------------------------------------------------------------------*/

VOID DeinitResolver()
{
    register struct NBase *nb = gb;
    struct Node *node;
    struct InquiryInfo *ii, *ii_tmp;
    struct InquiryHeader *ih;

    ii = (struct InquiryInfo *)nb->RequestList.mlh_Head;
    while(ii->ii_Msg.mn_Node.ln_Succ)
    {
        ii_tmp = (struct InquiryInfo *) ii->ii_Msg.mn_Node.ln_Succ;
        ih = ii->ii_IHeader;
        FreeMem(ih,ih->ih_Length);
        FreeMem(ii, sizeof(struct InquiryInfo));
        ii = ii_tmp;
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
    struct TagItem RNTags[4]={MATCH_REALM,0,
                              MATCH_HOSTNAME,0,
                              QUERY_IPADDR,0,
                              TAG_DONE,0};
    STRPTR realm_ptr,host_ptr,search_ptr;
    UBYTE nbuff[130];
    ULONG ipnum;

    if(!hostname)
        return 0L;

    if((*hostname == 0) || (strlen(hostname) >128))
        return 0L;

    stccpy(nbuff,hostname,128);

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

    if(!Stricmp(host_ptr,(STRPTR)&nb->LocalHostName))
    {
        if(realm_ptr)
        {
            if(!Stricmp(realm_ptr,(STRPTR)&nb->RealmName))
                return(0xffffffff);
        }
        else
            return(0xffffffff);
    }

    ipnum = 0;

    rn_hook.h_Entry = (HOOK_FUNC) RNHookFunc;
    rn_hook.h_SubEntry = (HOOK_FUNC) UtilityBase;
    rn_hook.h_Data = &ipnum;

    RNTags[0].ti_Data = (ULONG) realm_ptr;
    RNTags[1].ti_Data = (ULONG) host_ptr;
    if(!realm_ptr)
    {
        if(gb->RealmServer)
            RNTags[0].ti_Data = (ULONG) &gb->RealmName[0];
        else
            RNTags[0].ti_Tag = TAG_IGNORE;
    }

    SetSignal(0L,SIGF_NET);
    if(NIPCInquiryA(&rn_hook,5,1,RNTags))
        Wait(SIGF_NET);

    return ipnum;
}

#undef SysBase
#undef UtilityBase
#define SysBase (*(struct Library **)4L)

ULONG __asm RNHookFunc(register __a0 struct Hook *hook,
                      register __a2 struct Task *task,
                      register __a1 struct TagItem *tagList)
{
    struct TagItem *tag;
    struct Library *UtilityBase = (struct Library *)hook->h_SubEntry;

    if(tagList)
    {
        if(tag = FindTagItem(QUERY_IPADDR, tagList))
        {
            *(ULONG *)hook->h_Data = tag->ti_Data;
            Signal(task,SIGF_NET);
            return(FALSE);
        }
    }
    else
    {
        Signal(task,SIGF_NET);
        return(FALSE);
    }
    return(TRUE);
}

#undef SysBase
#undef UtilityBase

#define SysBase        (NIPCBASE->nb_SysBase)
#define UtilityBase    (NIPCBASE->nb_UtilityBase)

/****************************************************************************/

BOOL __asm NIPCInquiryA(register __a0 struct Hook *hook,
                     register __d0 ULONG maxTime,
                     register __d1 ULONG maxResponses,
                     register __a1 struct TagItem *tagList)

{
    register struct NBase *nb;
    struct InquiryInfo *ii;
    struct InquiryHeader *ih;
    struct TagItem *tag,*current;

    ULONG querylength,numtags,ptype;
    UBYTE *tagoffset,*dataoffset;

    BOOL addMRTag = FALSE;

    nb = gb;
    if(!(hook && tagList && maxTime ))
        return FALSE;

    if((FindTagItem(QUERY_REALMS,tagList) && !(gb->RealmServer)))
        return FALSE;

    if((FindTagItem(MATCH_REALM,tagList) && !(gb->RealmServer)))
        return FALSE;

    /* Now, let's figure out how much data we'll need. */

    querylength = sizeof(struct InquiryHeader) + 8;
    current = tagList;
    numtags = 1;

    if(gb->RealmServer)
    {
        addMRTag = TRUE;
        ptype = PTYPE_TRANSIT;
    }
    else
        ptype = PTYPE_QUERY;

    while(tag = NextTagItem(&current))
    {
        querylength+=8;
        numtags++;
        switch(tag->ti_Tag)
        {
            case QUERY_REALMS:          ptype = PTYPE_REALMREQ;
                                        addMRTag = FALSE;
                                        break;

            case MATCH_REALM:           addMRTag = FALSE;
            case MATCH_HOSTNAME:
            case MATCH_OWNER:
            case MATCH_ENTITY:
            case MATCH_SERVICE:
            case QUERY_LIBVERSION:
                                        querylength+=strlen((STRPTR)tag->ti_Data)+1;
                                        break;

            case MATCH_LIBVERSION:      querylength+=strlen((STRPTR)((ULONG)tag->ti_Data+4))+5;
                                        break;

        }
    }
    if(addMRTag)
    {
        querylength += (9 + strlen((STRPTR)&nb->RealmName));
        numtags++;
    }
    if(ii = (struct InquiryInfo *) AllocMem(sizeof(struct InquiryInfo),MEMF_CLEAR|MEMF_PUBLIC))
    {
        Forbid();
        ii->ii_ID = gb->ResID++;
        Permit();

        ii->ii_QueryLength = querylength;

        if(ih = (struct InquiryHeader *)AllocMem(ii->ii_QueryLength,MEMF_CLEAR|MEMF_PUBLIC))
        {

            struct TagItem *dtag;

            ii->ii_IHeader = ih;
            ii->ii_Task = FindTask(0L);
            ii->ii_Hook = hook;
            ii->ii_TTL = maxTime + 1;
            ii->ii_MaxResponses = maxResponses;
            ih->ih_PType = ptype;
            ih->ih_Length = querylength;
            ih->ih_HeaderLength = sizeof(struct InquiryHeader);
            ih->ih_ID    = ii->ii_ID;

            tagoffset = (UBYTE *)((ULONG)ih + (ULONG)sizeof(struct InquiryHeader));
            dataoffset = (UBYTE *)((ULONG)tagoffset + (ULONG)(numtags <<3));
            current = tagList;

            while(tag = NextTagItem(&current))
            {
                dtag = (struct TagItem *)tagoffset;
                tagoffset += 8;

                switch(tag->ti_Tag)
                {
                    case MATCH_HOSTNAME:
                    case MATCH_REALM:
                    case MATCH_OWNER:
                    case MATCH_ENTITY:
                    case MATCH_SERVICE:
                    case QUERY_LIBVERSION:	dtag->ti_Tag = tag->ti_Tag;
                                                dtag->ti_Data = (LONG)(dataoffset - (UBYTE *)ih);
                                                strcpy((STRPTR)dataoffset,(STRPTR)tag->ti_Data);
                                                dataoffset += (strlen((STRPTR)tag->ti_Data) + 1);
                                                break;

                    case MATCH_LIBVERSION:      dtag->ti_Tag = tag->ti_Tag;
                                                dtag->ti_Data = (LONG)(dataoffset - (UBYTE *)ih);
                                                *((ULONG *)dataoffset) = *(ULONG *)tag->ti_Data;
                                                strcpy((STRPTR)(dataoffset + 4),(STRPTR)(tag->ti_Data + 4));
                                                dataoffset += (strlen((STRPTR)tag->ti_Data+4) + 5);
                                                break;

                    default:                    dtag->ti_Tag = tag->ti_Tag;
                                                break;
                }
            }
            if(addMRTag)
            {
                dtag = (struct TagItem *)tagoffset;
                dtag->ti_Tag = MATCH_REALM;
                dtag->ti_Data = (LONG)(dataoffset - (UBYTE *)ih);
                strcpy((STRPTR)dataoffset,(STRPTR)&gb->RealmName);
            }

            PutMsg(nb->ResolverPort,(struct Message *)ii);
            return TRUE;
        }
        else
        {
            FreeMem(ii,sizeof(struct InquiryInfo));
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}

/*------------------------------------------------------------------------*/

void QueueNameResponse(struct InquiryHeader *buff, ULONG length, ULONG dest)
{
    struct TimedResponse *tr;

    if(tr = AllocMem(sizeof(struct TimedResponse),MEMF_CLEAR))
    {
        tr->tr_TimerIO.tr_node.io_Device = (struct Device *)gb->TimerDevice;
        tr->tr_TimerIO.tr_node.io_Unit = (struct Unit *)gb->TimerUnit;
        tr->tr_TimerIO.tr_node.io_Message.mn_ReplyPort = gb->NameResponsePort;
        tr->tr_TimerIO.tr_node.io_Command = TR_ADDREQUEST;
        gb->TimerSeed = makefastrand(gb->TimerSeed);
        tr->tr_TimerIO.tr_time.tv_secs=0;
        tr->tr_TimerIO.tr_time.tv_micro = 7 * gb->TimerSeed;
        tr->tr_Length = length;
        tr->tr_Destination = dest;
        tr->tr_Buffer = buff;
        SendIO((struct IORequest *)tr);
    }
}

/*------------------------------------------------------------------------*/

void DoResolver()
{
    register struct NBase *nb = gb;
    struct InquiryInfo *ii;
    struct TimedResponse *tr;
    ULONG destIP;

    while(ii = (struct InquiryInfo *) GetMsg(nb->ResolverPort))
    {
        AddTail((struct List *)&nb->RequestList,(struct Node *)ii);
        if((ii->ii_IHeader->ih_PType == PTYPE_TRANSIT) || (ii->ii_IHeader->ih_PType == PTYPE_REALMREQ))
        {
            destIP = gb->RealmServer;
        }
        else
        {
            ii->ii_IHeader->ih_SourceAddr = ((struct sanadev *) gb->DeviceList.lh_Head->ln_Succ)->sanadev_IPAddress;
            destIP = 0xffffffff;
        }
        UDP_Output((UBYTE *)ii->ii_IHeader,ii->ii_QueryLength,0,destIP,NIP_UDP_PORT,NIP_UDP_PORT);
    };

    while(tr = (struct TimedResponse *) GetMsg(nb->NameResponsePort))
    {
        UDP_Output((UBYTE *)tr->tr_Buffer,tr->tr_Length,0,tr->tr_Destination,NIP_UDP_PORT,NIP_UDP_PORT);
        FreeMem(tr->tr_Buffer,tr->tr_Buffer->ih_Length);
        FreeMem(tr,sizeof(struct TimedResponse));
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
    struct InquiryHeader *ih,*ih_new;
    struct InquiryInfo *ii;
    struct Service *svc;
    struct Realm *realm,*realm_ptr;
    struct ServicesLib *ServicesBase = NULL;
    struct Node *node;
    struct TagItem *tag;

    UBYTE *str;
    BOOL match;

    UtilityBase = (struct Library *)gb->nb_UtilityBase;

    be = (struct BuffEntry *) buff->buff_list.mlh_Head;
    ip = (struct iphdr *) ( (ULONG) be->be_data + (ULONG) be->be_offset);
    udp = (struct udphdr *) ( (ULONG) ip + (ULONG) sizeof(struct iphdr));

    ih = (struct InquiryHeader *) ((ULONG) udp + (ULONG) sizeof(struct udphdr));

    if(ih->ih_PType == PTYPE_TRANSIT)
    {
        if(!ih->ih_SourceAddr && (ip->iph_SrcAddr != 0x7f000001))
            ih->ih_SourceAddr = ip->iph_SrcAddr;

        ObtainSemaphore(&nb->RealmListLock);

        realm = (struct Realm *) gb->RealmList.mlh_Head;
        if(tag = FindTagItem(MATCH_REALM,(struct TagItem *)((ULONG)ih+(ULONG)ih->ih_HeaderLength)))
        {
            str = (STRPTR) tag->ti_Data + (LONG)ih;
            while(realm->realm_Node.mln_Succ)
            {
                if(!Stricmp(str,realm->realm_Name))
                {
                    if(realm->realm_Flags & REALM_AUTHORITY)
                    {
                        ih->ih_PType = PTYPE_QUERY;
                        if(ih->ih_SourceAddr)
                            UDP_Output((UBYTE *)ih,ih->ih_Length,0,realm->realm_Broadcast,NIP_UDP_PORT,NIP_UDP_PORT);
                        else
                        {
                            ih->ih_SourceAddr = realm->realm_SrcIP;
                            UDP_Output((UBYTE *)ih,ih->ih_Length,0,realm->realm_Broadcast,NIP_UDP_PORT,NIP_UDP_PORT);
                            ih->ih_SourceAddr = 0;
                        }
                    }
                    else
                    {
                        ih->ih_PType = PTYPE_TRANSIT;
                        UDP_Output((UBYTE *)ih,ih->ih_Length,0,realm->realm_Server,NIP_UDP_PORT,NIP_UDP_PORT);
                        break;
                    }
                }
                realm = (struct Realm *) realm->realm_Node.mln_Succ;
            }
        }
        ReleaseSemaphore(&nb->RealmListLock);
    }
    else if((ih->ih_PType == PTYPE_QUERY) || (ih->ih_PType == PTYPE_REALMREQ))
    {
        struct TagItem *current;
        ULONG responselength,responsetags;
        STRPTR tmpstr;
        BOOL respond = TRUE;

        if ((ih->ih_PType == PTYPE_REALMREQ) && (!ih->ih_SourceAddr))
            ih->ih_SourceAddr = ip->iph_SrcAddr;

        current = (struct TagItem *)((ULONG)ih + ih->ih_HeaderLength);

        responselength = sizeof(struct InquiryHeader);  /* Don't forget the terminating TAG_DONE TagItem */
        responsetags = 1;

        while(tag = NextTagItem(&current))
        {
            responsetags++;
            switch(tag->ti_Tag)
            {
                case MATCH_HOSTNAME:    tmpstr = (STRPTR)((ULONG)ih + (ULONG)tag->ti_Data);
                                        if(Stricmp(tmpstr,(STRPTR)&nb->LocalHostName))
                                        {
                                            respond = FALSE;
                                        }
                                        responselength += strlen(tmpstr) + 1;
                                        break;

                case QUERY_HOSTNAME:    responselength += strlen((STRPTR)&nb->LocalHostName) + 1;
                                        if(gb->RealmServer)
                                            responselength += strlen((STRPTR)&gb->RealmName) + 1;
                                        break;

                case MATCH_OWNER:	tmpstr = (STRPTR)((ULONG)ih + (ULONG)tag->ti_Data);
                                        if(Stricmp(tmpstr,(STRPTR)&nb->OwnerName))
                                        {
                                            respond = FALSE;
                                        }
                                        responselength += strlen(tmpstr) + 1;
                                        break;

                case QUERY_OWNER:    	responselength += strlen((STRPTR)&nb->OwnerName) + 1;
                                        break;

                case MATCH_REALM:       tmpstr = (STRPTR)((ULONG)ih + (ULONG)tag->ti_Data);
                                        if(Stricmp(tmpstr,(STRPTR)&nb->RealmName))
                                        {
                                            respond = FALSE;
                                        }
                                        responselength += strlen(tmpstr) + 1;
                                        break;

                case QUERY_REALMS:      realm_ptr = (struct Realm *)gb->RealmList.mlh_Head;
                                        while(realm_ptr->realm_Node.mln_Succ)
                                        {
                                            responselength += strlen((STRPTR)&realm_ptr->realm_Name) + 1;
                                            responsetags++;
                                            realm_ptr = (struct Realm *)realm_ptr->realm_Node.mln_Succ;
                                        }
                                        responsetags--;
                                        break;

                case MATCH_IPADDR:
                                        if(!IsLocalDevice(tag->ti_Data))
                                        {
                                            respond = FALSE;
                                        }
                                        break;

                case MATCH_ATTNFLAGS:   if(!(tag->ti_Data & ((struct ExecBase *)SysBase)->AttnFlags))
                                        {
                                            respond = FALSE;
                                        }
                                        break;

                case MATCH_CHIPREVBITS:
                                        {
                                            struct GfxBase *GfxBase;
                                            if(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",37L))
                                            {
                                                if(!(tag->ti_Data & GfxBase->ChipRevBits0))
                                                {
                                                    respond = FALSE;
                                                }
                                                CloseLibrary((struct Library *)GfxBase);
                                            }
                                            else
                                                respond = FALSE;
                                        }
                                        break;

                case MATCH_MAXFASTMEM:
                case MATCH_MAXCHIPMEM:
                case MATCH_AVAILFASTMEM:
                case MATCH_AVAILCHIPMEM:
                                        {
                                            ULONG memflags;

                                            switch(tag->ti_Tag)
                                            {
                                                case MATCH_MAXFASTMEM:  memflags = MEMF_FAST|MEMF_TOTAL;
                                                                        break;
                                                case MATCH_MAXCHIPMEM:  memflags = MEMF_CHIP|MEMF_TOTAL;
                                                                        break;
                                                case MATCH_AVAILFASTMEM: memflags = MEMF_FAST;
                                                                         break;
                                                case MATCH_AVAILCHIPMEM: memflags = MEMF_CHIP;
                                                                         break;
                                            }
                                            if(!(AvailMem(memflags) >= tag->ti_Data))
                                            {
                                                respond = FALSE;
                                            }
                                        }
                                        break;

                case MATCH_KICKVERSION:
                                        {
                                            ULONG version;
                                            version = (SysBase->lib_Version << 16) | ((struct ExecBase *)SysBase)->SoftVer;
                                            if(version < tag->ti_Data)
                                            {
                                                respond = FALSE;
                                            }
                                        }
                                        break;

                case MATCH_WBVERSION:
                                        {
                                            ULONG version;
                                            struct Library *VBase;
                                            if(VBase = OpenLibrary("version.library",0))
                                            {
                                                version = (VBase->lib_Version << 16) | VBase->lib_Revision;
                                                if(version < tag->ti_Data)
                                                {
                                                    respond = FALSE;
                                                }
                                                CloseLibrary(VBase);
                                            }
                                            else
                                                respond = FALSE;
                                        }
                                        break;

                case MATCH_LIBVERSION:
                                        {
                                            ULONG match_version;
                                            ULONG lib_version;
                                            STRPTR lib_name;
                                            struct Library *LibBase;

					    match_version = *(ULONG *)((ULONG)ih + (ULONG)tag->ti_Data);
					    lib_name = (STRPTR)((ULONG)ih + (ULONG)tag->ti_Data + 4);

                                            if(LibBase = OpenLibrary(lib_name,0))
                                            {
                                                lib_version = (LibBase->lib_Version << 16) | LibBase->lib_Revision;
                                                if(lib_version < match_version)
                                                {
                                                    respond = FALSE;
                                                }
                                                else
                                                {
                                                    responsetags++;
                                                    responselength += strlen(lib_name) + 5;
                                                }
                                                CloseLibrary(LibBase);
                                            }
                                            else
                                                respond = FALSE;
                                        }
                                        break;

		case QUERY_LIBVERSION:	tmpstr = (STRPTR)((ULONG)ih + (ULONG)tag->ti_Data);
		                        responselength += strlen(tmpstr) + 5;
					responsetags++;
					break;

		case MATCH_NIPCVERSION:
					{
					    ULONG version;
					    version = (gb->LibNode.lib_Version << 16) | (gb->LibNode.lib_Revision);
					    if(version < tag->ti_Data)
					    {
					    	respond = FALSE;
					    }
					}
					break;

                case QUERY_ATTNFLAGS:
                case QUERY_CHIPREVBITS:
                case QUERY_MAXFASTMEM:
                case QUERY_AVAILFASTMEM:
                case QUERY_MAXCHIPMEM:
                case QUERY_AVAILCHIPMEM:
                case QUERY_KICKVERSION:
                case QUERY_WBVERSION:
                case QUERY_NIPCVERSION:
                                        break;

                case QUERY_IPADDR:
                                        if(!(route = Route(ih->ih_SourceAddr, TRUE)))
                                        {
                                            respond = FALSE;
                                        }
                                        break;

                case MATCH_SERVICE:     match = FALSE;
                                        Forbid();
                                        if(nb->nb_ServicesBase)
                                            ServicesBase = (struct ServicesLib *)OpenLibrary("services.library",0L);
                                        Permit();
                                        if(ServicesBase)
                                        {
                                            ObtainSemaphore(&ServicesBase->SVCS_ServicesLock);
                                            svc = (struct Service *)ServicesBase->SVCS_Services.mlh_Head;
                                            tmpstr = (STRPTR)((ULONG)ih + (ULONG)tag->ti_Data);
                                            while(svc->svc_Node.ln_Succ)
                                            {
                                                if(!Stricmp((STRPTR)&svc->svc_SvcName,tmpstr))
                                                {
                                                    if((!(svc->svc_Flags & SVCF_EXPUNGE)) && (svc->svc_Flags & SVCF_ENABLE))
                                                    {
                                                        responselength += strlen(tmpstr) + 1;
                                                        responsetags++;
                                                        match = TRUE;
                                                        break;
                                                    }
                                                }
                                                svc = (struct Service *)svc->svc_Node.ln_Succ;
                                            }
                                            ReleaseSemaphore(&ServicesBase->SVCS_ServicesLock);
                                            CloseLibrary((struct Library *)ServicesBase);
                                        }
                                        if(!match)
                                        {
                                            respond = FALSE;
                                        }
                                        responsetags--;
                                        break;

                case QUERY_SERVICE:
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
                                                if((!(svc->svc_Flags & SVCF_EXPUNGE)) && (svc->svc_Flags & SVCF_ENABLE))
                                                {
                                                    responselength += strlen((STRPTR)&svc->svc_SvcName) + 1 + 8;
                                                    responsetags++;
                                                }
                                                svc = (struct Service *)svc->svc_Node.ln_Succ;
                                            }
                                            ReleaseSemaphore(&ServicesBase->SVCS_ServicesLock);
                                            CloseLibrary((struct Library *)ServicesBase);
                                        }
                                        responsetags--;
                                        break;

                case MATCH_ENTITY:      tmpstr = (STRPTR)((ULONG)ih + (ULONG)tag->ti_Data);
                                        ObtainSemaphore(&nb->ANMPELSemaphore);
                                        if(FindNameI((struct List *)&nb->ANMPEntityList,tmpstr))
                                        {
                                            responselength += strlen(tmpstr) + 1;
                                        }
                                        else
                                            respond = FALSE;
                                        ReleaseSemaphore(&nb->ANMPELSemaphore);
                                        break;

                case QUERY_ENTITY:      ObtainSemaphore(&nb->ANMPELSemaphore);
                                        node = (struct Node *)gb->ANMPEntityList.mlh_Head;
                                        while(node->ln_Succ)
                                        {
                                            if(((struct Entity *)node)->entity_Flags & ENTF_PUBLIC)
                                            {
                                                responselength += strlen(node->ln_Name) + 1 + 8;
                                                responsetags++;
                                            }
                                            node = node->ln_Succ;
                                        }
                                        ReleaseSemaphore(&nb->ANMPELSemaphore);
                                        break;

            }
            if(!respond)
                break;
        }
        responselength += (responsetags << 3);
EndParse:
        if(respond)
        {
            if(ih_new = AllocMem(responselength,MEMF_CLEAR|MEMF_PUBLIC))
            {
                struct TagItem *dtag;
                UBYTE *newdataoffset;
                UBYTE *newtagoffset;

                ih_new->ih_Length = responselength;
                ih_new->ih_HeaderLength = sizeof(struct InquiryHeader);
                ih_new->ih_PType  = PTYPE_RESPONSE;
                ih_new->ih_ID     = ih->ih_ID;

                newtagoffset = (UBYTE *) (ULONG)ih_new + (ULONG)sizeof(struct InquiryHeader);
                newdataoffset = newtagoffset + (responsetags * 8);

                current = (struct TagItem *)((ULONG)ih + ih->ih_HeaderLength);

                while(tag = NextTagItem(&current))
                {
                    dtag = (struct TagItem *)newtagoffset;
                    dtag->ti_Tag = tag->ti_Tag;

                    switch(tag->ti_Tag)
                    {
                        case MATCH_HOSTNAME:
                        case MATCH_REALM:
                        case MATCH_ENTITY:
                        case MATCH_SERVICE:
                        case MATCH_OWNER:
                        case MATCH_MACHDESC:    dtag->ti_Data = (LONG)((ULONG)newdataoffset - (ULONG)ih_new);
                                                strcpy((STRPTR)newdataoffset,(STRPTR)ih+tag->ti_Data);
                                                newdataoffset += strlen((STRPTR)newdataoffset) + 1;
                                                break;

			case MATCH_LIBVERSION:	dtag->ti_Data = (LONG)((ULONG)newdataoffset - (ULONG)ih_new);
						*((ULONG *)newdataoffset) = *((ULONG *)((LONG)ih+tag->ti_Data));
						strcpy((STRPTR)((ULONG)newdataoffset+4),(STRPTR)ih+tag->ti_Data);
						newdataoffset += strlen((STRPTR)newdataoffset+4) +5;
						break;

                        case QUERY_HOSTNAME:    dtag->ti_Data = (LONG)((ULONG)newdataoffset - (ULONG)ih_new);
                                                if(gb->RealmServer)
                                                {
                                                    strcpy((STRPTR)newdataoffset,(STRPTR)&nb->RealmName);
                                                    newdataoffset += strlen((STRPTR)newdataoffset);
                                                    *newdataoffset=':';
                                                    newdataoffset++;
                                                }
                                                strcpy((STRPTR)newdataoffset,(STRPTR)&nb->LocalHostName);
                                                newdataoffset += strlen((STRPTR)newdataoffset) + 1;
                                                break;

                        case QUERY_OWNER:	dtag->ti_Data = (LONG)((ULONG)newdataoffset - (ULONG)ih_new);
                                                strcpy((STRPTR)newdataoffset,(STRPTR)&nb->OwnerName);
                                                newdataoffset += strlen((STRPTR)newdataoffset) + 1;
                                                break;

                        case MATCH_ATTNFLAGS:
                        case MATCH_CHIPREVBITS:
                        case MATCH_MAXFASTMEM:
                        case MATCH_AVAILFASTMEM:
                        case MATCH_MAXCHIPMEM:
                        case MATCH_AVAILCHIPMEM:
                        case MATCH_KICKVERSION:
                        case MATCH_WBVERSION:
                        case MATCH_NIPCVERSION:
                        case MATCH_IPADDR:      dtag->ti_Data = tag->ti_Data;
                                                break;

                        case QUERY_IPADDR:      route = Route(ih->ih_SourceAddr, TRUE);
                                                dtag->ti_Data = route->nr_Device->sanadev_IPAddress;
                                                break;

                        case QUERY_ENTITY:      ObtainSemaphore(&nb->ANMPELSemaphore);
                                                node = (struct Node *)gb->ANMPEntityList.mlh_Head;
                                                while(node->ln_Succ)
                                                {
                                                    if(((struct Entity *)node)->entity_Flags & ENTF_PUBLIC)
                                                    {
                                                        dtag->ti_Tag = QUERY_ENTITY;
                                                        dtag->ti_Data = (LONG)((ULONG)newdataoffset - (ULONG)ih_new);
                                                        strcpy((STRPTR)newdataoffset,(STRPTR)node->ln_Name);
                                                        newdataoffset += strlen((STRPTR)newdataoffset) +1;
                                                        newtagoffset +=8;
                                                        dtag = (struct TagItem *)newtagoffset;
                                                    }
                                                    node = node->ln_Succ;
                                                }
                                                ReleaseSemaphore(&nb->ANMPELSemaphore);
                                                newtagoffset -= 8;
                                                break;

                        case QUERY_SERVICE:     Forbid();
                                                if(nb->nb_ServicesBase)
                                                    ServicesBase = (struct ServicesLib *)OpenLibrary("services.library",0L);
                                                Permit();
                                                if(ServicesBase)
                                                {
                                                    ObtainSemaphore(&ServicesBase->SVCS_ServicesLock);
                                                    svc = (struct Service *)ServicesBase->SVCS_Services.mlh_Head;
                                                    while(svc->svc_Node.ln_Succ)
                                                    {
                                                        if((!(svc->svc_Flags & SVCF_EXPUNGE)) && (svc->svc_Flags & SVCF_ENABLE))
                                                        {
                                                            dtag->ti_Tag = QUERY_SERVICE;
                                                            dtag->ti_Data = (LONG)((ULONG)newdataoffset - (ULONG)ih_new);
                                                            strcpy((STRPTR)newdataoffset,(STRPTR)&svc->svc_SvcName);
                                                            newdataoffset +=strlen((STRPTR)&svc->svc_SvcName) + 1;
                                                            newtagoffset +=8;
                                                            dtag = (struct TagItem *)newtagoffset;
                                                        }
                                                        svc = (struct Service *)svc->svc_Node.ln_Succ;
                                                    }
                                                    ReleaseSemaphore(&ServicesBase->SVCS_ServicesLock);
                                                    CloseLibrary((struct Library *)ServicesBase);
                                                }
                                                newtagoffset -= 8;
                                                break;

                        case QUERY_REALMS:      node = (struct Node *)gb->RealmList.mlh_Head;
                                                while(node->ln_Succ)
                                                {
                                                    dtag->ti_Tag = QUERY_REALMS;
                                                    dtag->ti_Data = (LONG)((ULONG)newdataoffset - (ULONG)ih_new);
                                                    strcpy((STRPTR)newdataoffset,(STRPTR)((struct Realm *)node)->realm_Name);
                                                    newdataoffset += strlen((STRPTR)newdataoffset) +1;
                                                    newtagoffset +=8;
                                                    dtag = (struct TagItem *)newtagoffset;
                                                    node = node->ln_Succ;
                                                }
                                                newtagoffset -= 8;
                                                break;
                        case QUERY_MAXFASTMEM:
                        case QUERY_MAXCHIPMEM:
                        case QUERY_AVAILFASTMEM:
                        case QUERY_AVAILCHIPMEM:
                                                {
                                                    ULONG memflags;

                                                    switch(tag->ti_Tag)
                                                    {
                                                        case QUERY_MAXFASTMEM:  memflags = MEMF_FAST|MEMF_TOTAL;
                                                                                break;
                                                        case QUERY_MAXCHIPMEM:  memflags = MEMF_CHIP|MEMF_TOTAL;
                                                                                break;
                                                        case QUERY_AVAILFASTMEM: memflags = MEMF_FAST;
                                                                                 break;
                                                        case QUERY_AVAILCHIPMEM: memflags = MEMF_CHIP;
                                                                                 break;
                                                    }
                                                    dtag->ti_Data =  AvailMem(memflags);
                                                }
                                                break;

			case QUERY_ATTNFLAGS:
						dtag->ti_Data = ((struct ExecBase *)SysBase)->AttnFlags;
						break;

                        case QUERY_CHIPREVBITS:
                                                {
                                                    struct GfxBase *GfxBase;
                                                    if(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",37L))
                                                    {
                                                        dtag->ti_Data = GfxBase->ChipRevBits0;
                                                        CloseLibrary((struct Library *)GfxBase);
                                                    }
                                                }
                                                break;
                        case QUERY_KICKVERSION:
                                                dtag->ti_Data = (SysBase->lib_Version << 16) | ((struct ExecBase *)SysBase)->SoftVer;
                                                break;

                        case QUERY_NIPCVERSION:
                        			dtag->ti_Data = (gb->LibNode.lib_Version << 16) | (gb->LibNode.lib_Revision);
                        			break;

                        case QUERY_WBVERSION:
                                                {
                                                    struct Library *VBase;
                                                    if(VBase = OpenLibrary("version.library",0))
                                                    {
                                                        dtag->ti_Data = (VBase->lib_Version << 16) | VBase->lib_Revision;
                                                        CloseLibrary(VBase);
                                                    }
                                                }
                                                break;

                        case QUERY_LIBVERSION:
                                                {
                                                    ULONG lib_version;
                                                    STRPTR lib_name;
                                                    struct Library *LibBase;

                                                    lib_name = (STRPTR)((ULONG)ih + (ULONG)tag->ti_Data);
                                                    lib_version = 0;

                                                    if(LibBase = OpenLibrary(lib_name,0))
                                                    {
                                                        lib_version = (LibBase->lib_Version << 16) | LibBase->lib_Revision;
                                                        CloseLibrary(LibBase);
                                                    }
                                                    dtag->ti_Data = (LONG)((ULONG)newdataoffset - (ULONG)ih_new);
                                                    *((ULONG *)newdataoffset) = lib_version;
                                                    strcpy((STRPTR)((ULONG)newdataoffset+4),lib_name);
                                                    newdataoffset += strlen(lib_name) + 5;
                                                }
                                                break;

                    }
                    newtagoffset += 8;
                }
                dtag = (struct TagItem *)newtagoffset;
                dtag->ti_Tag = TAG_DONE;
                QueueNameResponse((APTR)ih_new, ih_new->ih_Length, ih->ih_SourceAddr);
//                UDP_Output((UBYTE *)ih_new,ih_new->ih_Length,0,ih->ih_SourceAddr,NIP_UDP_PORT,NIP_UDP_PORT);
//                FreeMem(ih_new,ih_new->ih_Length);
            }
        }
    }
    else if(ih->ih_PType == PTYPE_RESPONSE)
    {
        struct InquiryInfo *tmp_ii;
        ii = (struct InquiryInfo *) gb->RequestList.mlh_Head;
        while(ii->ii_Msg.mn_Node.ln_Succ)
        {
            tmp_ii = (struct InquiryInfo *)ii->ii_Msg.mn_Node.ln_Succ;

            if(ii->ii_ID == ih->ih_ID)
            {
                ULONG result;
                struct TagItem *ftag,*current;
                current = (struct TagItem *)((ULONG)ih + ih->ih_HeaderLength);
                while(ftag=NextTagItem(&current))
                {
                    switch(ftag->ti_Tag)
                    {
                        case TAG_DONE:
                        case MATCH_IPADDR:
                        case QUERY_IPADDR:
                        case MATCH_ATTNFLAGS:
                        case QUERY_ATTNFLAGS:
                        case MATCH_CHIPREVBITS:
                        case QUERY_CHIPREVBITS:
                        case MATCH_MAXFASTMEM:
                        case QUERY_MAXFASTMEM:
                        case MATCH_AVAILFASTMEM:
                        case QUERY_AVAILFASTMEM:
                        case MATCH_MAXCHIPMEM:
                        case QUERY_MAXCHIPMEM:
                        case MATCH_AVAILCHIPMEM:
                        case QUERY_AVAILCHIPMEM:
                        case MATCH_KICKVERSION:
                        case QUERY_KICKVERSION:
                        case MATCH_WBVERSION:
                        case QUERY_WBVERSION:
                        case MATCH_NIPCVERSION:
                        case QUERY_NIPCVERSION:
                                                break;

                        default:                ftag->ti_Data += (LONG)ih;
                                                break;
                    }
                }
                result = CallHookPkt(ii->ii_Hook,ii->ii_Task,(APTR)((ULONG)ih + ih->ih_HeaderLength));
                ii->ii_MaxResponses--;
                if(result && !ii->ii_MaxResponses)
                    CallHookPkt(ii->ii_Hook,ii->ii_Task,NULL);

                if(!result || !ii->ii_MaxResponses)
                {
                    Remove((struct Node *)ii);
                    FreeMem(ii->ii_IHeader,ii->ii_QueryLength);
                    FreeMem(ii,sizeof(struct InquiryInfo));
                    break;
                }
                break;
            }
            ii = tmp_ii;
        }
    }
    FreeBuffer(buff);
}
/*------------------------------------------------------------------------*/

void ResolverTimeout()
{
    struct InquiryInfo *ii;
    APTR next;

#ifdef RIP_YES
    struct RIPHeader *rip;

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
#endif

    ii = (struct InquiryInfo *) gb->RequestList.mlh_Head;
    while(ii->ii_Msg.mn_Node.ln_Succ)
    {
        next = (struct InquiryInfo *) ii->ii_Msg.mn_Node.ln_Succ;
        ii->ii_TTL -= 1;
        if(!ii->ii_TTL)
        {
            CallHookPkt(ii->ii_Hook,ii->ii_Task,NULL);
            Remove((struct Node *)ii);
            FreeMem(ii->ii_IHeader,ii->ii_QueryLength);
            FreeMem(ii,sizeof(struct InquiryInfo));
        }
        else
        {
            ULONG destIP;

            if((ii->ii_IHeader->ih_PType == PTYPE_TRANSIT) || (ii->ii_IHeader->ih_PType == PTYPE_REALMREQ))
            {
                destIP = gb->RealmServer;
            }
            else
            {
                ii->ii_IHeader->ih_SourceAddr = ((struct sanadev *) gb->DeviceList.lh_Head->ln_Succ)->sanadev_IPAddress;
                destIP = 0xffffffff;
            }
            UDP_Output((UBYTE *)ii->ii_IHeader,ii->ii_QueryLength,0,destIP,NIP_UDP_PORT,NIP_UDP_PORT);
        }
        ii = next;
    }

}

/*------------------------------------------------------------------------*/
