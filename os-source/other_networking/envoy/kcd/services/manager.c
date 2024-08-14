/*
 * * $Id: manager.c,v 37.8 92/12/16 14:27:59 kcd Exp Locker: kcd $ *
 *
 * (C) Copyright 1992, Commodore-Amiga, Inc. *
 *
 * manager.c - Envoy Services Manager *
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/ports.h>

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <dos/rdargs.h>
#include <envoy/errors.h>

#include <libraries/iffparse.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/nipc_protos.h>
#include <clib/svc_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/utility_protos.h>

extern __stdargs NewList(struct List *list);

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/nipc_pragmas.h>
#include <pragmas/svc_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/utility_pragmas.h>

#define SERVICES_MANAGER oui

#include "services.h"
#include "servicesinternal.h"
#include "servicesbase.h"
#include "manager_rev.h"

#include "string.h"

/*--------------------------------------------------------------------------*/

/* Globals */

struct DosLibrary *DOSBase;
struct Library *NIPCBase;
struct Library *UtilityBase;
struct Library *IFFParseBase;
struct ServicesLib *ServicesBase;
struct Library *SvcBase;
struct ExecBase *SysBase;
struct TagItem svc_entity_tags[4] =
{
    ENT_Public, 0,
    ENT_Name, (ULONG) MGR_ENTITY_NAME,
    ENT_AllocSignal, NULL,
    TAG_DONE, 0
};
UBYTE clientHostName[128];
UBYTE VersionTag[] = VERSTAG;

/*--------------------------------------------------------------------------*/

/* Prototypes */

BOOL LoadServicesInfo(void);
VOID ProcessEntity(APTR svc_entity);
struct Service *LocalFindService(STRPTR svc_name);
LONG manager(void);
BPTR PrefOpen(void);
BOOL MakeServices(struct IFFHandle *iff);

#define ID_ISVC  MAKE_ID('I','S','V','C')
#define ID_PREF  MAKE_ID('P','R','E','F')

struct IFFService
{
    ULONG   is_Flags;
    UWORD   is_UID;
    UWORD   is_GID;
    UBYTE   is_PathName[256];
    UBYTE   is_SvcName[64];
};


/*--------------------------------------------------------------------------*/

/* Main  */

LONG manager()
{
    APTR ServiceEntity;
    ULONG entitymask, waitmask, sigs;
    ULONG entitysigbit = 0;
    struct Message *wbmessage = NULL;
    struct Process *myproc;
    struct MsgPort *smport;

    SysBase = (struct ExecBase *)*((void **)4);
    myproc = (struct Process *)FindTask(0L);
    if(!myproc->pr_CLI)
    {
        WaitPort(&myproc->pr_MsgPort);
        wbmessage = GetMsg(&myproc->pr_MsgPort);
    }

    Forbid();
    if(smport = FindPort("Services Manager"))
    {
    	Signal(smport->mp_SigTask,SIGBREAKF_CTRL_F);
    	smport = NULL;
    }
    else
    {
    	smport = CreateMsgPort();
    	smport->mp_Node.ln_Name = "Services Manager";
    	smport->mp_Node.ln_Pri = -128;
    	AddPort(smport);
    }
    Permit();

    if(smport)
    {
        if (DOSBase = (struct DosLibrary *) OpenLibrary("dos.library", 37L))
        {
            if (UtilityBase = (struct Library *) OpenLibrary("utility.library", 37L))
            {
                if (NIPCBase = (struct Library *) OpenLibrary("nipc.library", 0L))
                {
                    if (ServicesBase = (struct ServicesLib *) OpenLibrary("services.library", 37L))
                    {
                        svc_entity_tags[2].ti_Data = (ULONG) & entitysigbit;

                        DBMSG(("services.library opened.\n"));

                        if (ServiceEntity = CreateEntityA(svc_entity_tags))
                        {
                            if (LoadServicesInfo());
                            {
                                DBMSG(("Services Configuration Loaded.\n"));

                                entitymask = (1 << entitysigbit);

                                waitmask = (entitymask | SIGBREAKF_CTRL_F);

                                while (TRUE)
                                {
                                    sigs = Wait(waitmask);

                                    DBMSG(("Awake! %lx\n",sigs));

                                    ProcessEntity(ServiceEntity);

                                    DBMSG(("Sigs after PE(): %lx\n",sigs));

                                    if (sigs & SIGBREAKF_CTRL_F)
                                    {
                                        DBMSG(("***break: CTRL_F\n"));
                                        break;
                                    }

                                }
                            }
                            DeleteEntity(ServiceEntity);
                        }
                        CloseLibrary((struct Library *) ServicesBase);
                    }
                    CloseLibrary(NIPCBase);
                }
                CloseLibrary(UtilityBase);
            }
            CloseLibrary((struct Library *) DOSBase);
        }
        RemPort(smport);
    }
    if(wbmessage)
    {
        Forbid();
        ReplyMsg(wbmessage);
    }
    return(0L);
}

/*--------------------------------------------------------------------------*/

/* LoadServicesInfo  */

BOOL LoadServicesInfo()
{
    struct ContextNode *top;
    ULONG error;
    BPTR IFFStream;
    struct IFFHandle *iff;
    BOOL status = FALSE, cont = TRUE;

    LONG props[]={ID_PREF,ID_ISVC};

    if(IFFParseBase = OpenLibrary("iffparse.library",37))
    {
        if(IFFStream = PrefOpen())
        {
            if(iff = AllocIFF())
            {
                InitIFFasDOS(iff);

                if(!OpenIFF(iff,IFFF_READ))
                {
                    iff->iff_Stream = (ULONG) IFFStream;
                    if(!CollectionChunks(iff,props,1))
                    {
                        while(cont)
                        {
                            error = ParseIFF(iff,IFFPARSE_STEP);
                            if(!error)
                                continue;
                            if(error != IFFERR_EOC)
                                break;

                            top = CurrentChunk(iff);
                            if((top->cn_Type == ID_PREF) && (top->cn_ID == ID_FORM))
                                status = MakeServices(iff);
                        }
                    }
                    CloseIFF(iff);
                }
                FreeIFF(iff);
            }
            Close(IFFStream);
        }
        CloseLibrary(IFFParseBase);
    }
    return status;
}

BPTR PrefOpen(void)
{
    BPTR stream;

    if(stream = Open("ENV:envoy/services.prefs",MODE_OLDFILE))
        return stream;
    else if(stream = Open("ENVARC:envoy/services.prefs",MODE_OLDFILE))
        return stream;
    else
        return (BPTR)0L;
}

BOOL MakeServices(struct IFFHandle *iff)
{
    struct Service *new_svc, *old_svc;
    struct IFFService *is;
    struct CollectionItem *ci;

    if(ci = FindCollection(iff,ID_PREF,ID_ISVC))
    {
        old_svc = (struct Service *) ServicesBase->SVCS_Services.mlh_Head;
        while (old_svc->svc_Node.ln_Succ)
        {
            old_svc->svc_Flags |= SVCF_EXPUNGE;
            old_svc = (struct Service *) old_svc->svc_Node.ln_Succ;
        }

        while(ci)
        {
            is = (struct IFFService *)ci->ci_Data;

            if (old_svc = LocalFindService(is->is_SvcName))
            {
                DBMSG(("Updating old Service: %s\n", is->is_SvcName));

                strcpy(old_svc->svc_PathName, is->is_PathName);
                old_svc->svc_Flags = is->is_Flags;
            }
            else if (new_svc = AllocMem(sizeof(struct Service), MEMF_PUBLIC | MEMF_CLEAR))
            {
                DBMSG(("Adding new Service: %s\n", is->is_SvcName));

                strcpy(new_svc->svc_SvcName, (UBYTE *) is->is_SvcName);
                strcpy(new_svc->svc_PathName, (UBYTE *) is->is_PathName);
                new_svc->svc_Flags = is->is_Flags;
                AddTail((struct List *) &ServicesBase->SVCS_Services, (struct Node *) new_svc);
                new_svc->svc_Flags &= ~SVCF_EXPUNGE;
            }
            ci = ci->ci_Next;
        }
        return TRUE;
    }
    return FALSE;
}

/*--------------------------------------------------------------------------*/

/* LocalFindService() */

struct Service *LocalFindService(svc_name)
STRPTR svc_name;
{
    struct Service *svc;

    svc = (struct Service *) ServicesBase->SVCS_Services.mlh_Head;
    while (svc->svc_Node.ln_Succ)
    {
        DBMSG(("LocalFindService: %s == %s ?\n", svc_name, svc->svc_SvcName));
        if (!Stricmp(svc_name, svc->svc_SvcName))
            return (svc);
        svc = (struct Service *) svc->svc_Node.ln_Succ;
    }
    return (NULL);
}

/*--------------------------------------------------------------------------*/

/* ProcessEntity */

VOID ProcessEntity(svc_entity)
APTR svc_entity;
{
    struct Transaction *req_trans;
    struct FindServiceReq *fsr;
    struct Service *svc;
    ULONG error;

    ULONG sstags[9]={SSVC_UserName, 0,
    		     SSVC_Password, 0,
    		     SSVC_EntityName, 0,
    		     SSVC_HostName,0,
    		     TAG_DONE};

    while (req_trans = GetTransaction(svc_entity))
    {
        error = ENVOYERR_BADSTARTSERVICE; /* Assume the worst. */

        if ((req_trans->trans_ReqDataLength >= sizeof(struct FindServiceReq)) &&
            (req_trans->trans_Command == MGRCMD_FINDSERVICE))
        {
            fsr = (struct FindServiceReq *) req_trans->trans_RequestData;

            DBMSG(("Request for Service: %s\n", fsr->fsr_Service));

            if (svc = LocalFindService(fsr->fsr_Service))
            {
                if ((!(svc->svc_Flags & SVCF_EXPUNGE)) && (svc->svc_Flags & SVCF_ENABLE))
                {
                    DBMSG(("Opening %s\n",svc->svc_PathName));

                    if(SvcBase = OpenLibrary(svc->svc_PathName,0L))
                    {
                        DBMSG(("Service Opened okay!\n"));
                        GetHostName(req_trans->trans_SourceEntity,clientHostName,127);

                        sstags[1]=(ULONG)fsr->fsr_UserName;
                        sstags[3]=(ULONG)fsr->fsr_PassWord;
                        sstags[5]=(ULONG)req_trans->trans_ResponseData;
                        sstags[7]=(ULONG)clientHostName;

                        error = StartServiceA((struct TagItem *)sstags);

/*                        error=StartService(SSVC_UserName,fsr->fsr_UserName,
                        		   SSVC_Password,fsr->fsr_PassWord,
                        		   SSVC_EntityName,req_trans->trans_ResponseData,
                        		   SSVC_HostName,clientHostName,
                        		   TAG_DONE); */

                        DBMSG(("StartService() Returned %ld, EntityName: %s\n",error,req_trans->trans_ResponseData));
                        req_trans->trans_RespDataActual = sizeof(struct FindServiceAck);
                        CloseLibrary(SvcBase);
                    }
                    else
                    	error = ENVOYERR_OPENSERVICEFAIL;
                }
                else
                    error = ENVOYERR_UNKNOWNSERVICE;
            }
            else
            	error = ENVOYERR_UNKNOWNSERVICE;
        }
        req_trans->trans_Error = error;
        ReplyTransaction(req_trans);
    }
}

