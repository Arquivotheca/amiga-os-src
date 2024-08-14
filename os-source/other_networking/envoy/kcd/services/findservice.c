/*
** $Id: findservice.c,v 37.6 92/12/16 14:26:54 kcd Exp $
**
** Module to find a service on a given machine
**
*/

/*--------------------------------------------------------------------------*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef EXEC_LISTS_H
#include <exec/lists.h>
#endif

#ifndef EXEC_SEMAPHORES_H
#include <exec/semaphores.h>
#endif

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif

#ifndef ENVOY_NIPC_H
#include <envoy/nipc.h>
#endif

#include "services.h"
#include "servicesbase.h"
#include "servicesinternal.h"

#include <dos.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/nipc_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/nipc_pragmas.h>
#include <pragmas/utility_pragmas.h>

/*--------------------------------------------------------------------------*/

APTR ASM FindServiceA(REG(a0) STRPTR remoteHost,
                      REG(a1) STRPTR serviceName,
                      REG(a2) APTR srcEntity,
                      REG(a3) struct TagItem *tagList)
{
    APTR remote_entity;
    APTR local_entity;
    STRPTR remote_host = NULL;
    STRPTR hostnamebuff;
    APTR service_entity = NULL;
    struct Transaction *fs_trans;
    struct FindServiceReq *fsr;
    struct FindServiceAck *fsa;
    struct TagItem *tag;
    ULONG loc_sigs,mgr_error,svc_error,func_error, error;
    ULONG *err;

    func_error=0;

    if(local_entity = CreateEntity(ENT_AllocSignal, &loc_sigs,
                                    ENT_Name, "x",
                                    TAG_DONE, 0))
    {
        if(fs_trans = AllocTransaction(TRN_AllocReqBuffer, sizeof(struct FindServiceReq),
                                        TRN_AllocRespBuffer, sizeof(struct FindServiceAck),
                                        TAG_DONE, 0))
        {
            if(srcEntity)
            {
                if(hostnamebuff = AllocMem(256,MEMF_CLEAR|MEMF_PUBLIC))
                {
                    if(remoteHost)
                    {
                        strcpy(hostnamebuff,remoteHost);
                        remote_host = hostnamebuff;
                        DBMSG(("FindService() HostName: %s\n",hostnamebuff));
                    }

                    if(remote_entity = FindEntity(remote_host,MGR_ENTITY_NAME,local_entity,&mgr_error))
                    {
                        DBMSG(("Found Services Manager!\n"));
                        fsr = (struct FindServiceReq *) fs_trans->trans_RequestData;

                        strcpy(fsr->fsr_Service,serviceName);
                        fsr->fsr_UserName[0] = 0;
                        fsr->fsr_PassWord[0] = 0;

                        DBMSG(("Src: %lx, Dst: %lx\n",serviceName,fsr->fsr_Service));

                        if(tag = FindTagItem( FSVC_UserName, tagList))
                            strcpy(fsr->fsr_UserName,(STRPTR) tag->ti_Data);

                        if(tag = FindTagItem( FSVC_PassWord, tagList))
                            strcpy(fsr->fsr_PassWord,(STRPTR) tag->ti_Data);

                        DBMSG(("serviceName: %s\n",serviceName));
                        DBMSG(("FindService() fsr_Service : %s\n",fsr->fsr_Service));
                        DBMSG(("FindService() fsr_UserName: %s\n",fsr->fsr_UserName));
                        DBMSG(("FindService() fsr_PassWord: %s\n",fsr->fsr_PassWord));

                        fs_trans->trans_Command = MGRCMD_FINDSERVICE;
			fs_trans->trans_Timeout = 10;
                        DoTransaction(remote_entity,local_entity,fs_trans);
                        LoseEntity(remote_entity);

                        fsa = (struct FindServiceAck *) fs_trans->trans_ResponseData;

                        if (!(mgr_error = fs_trans->trans_Error))
                        {
                            DBMSG(("FindService: Server EntityName is %s\n",fsa->fsa_EntityName));
                            service_entity = FindEntity(remote_host,fsa->fsa_EntityName,srcEntity,&svc_error);
                            DBMSG(("FindService: Connected to server!\n"));
                        }
                    }
                    FreeMem(hostnamebuff,256);
                }
            }
            FreeTransaction(fs_trans);
        }
        DeleteEntity(local_entity);
    }

    if(func_error)
    {
        error = func_error;
    }
    else if(mgr_error)
    {
        error = mgr_error;
    }
    else
        error = svc_error;

    if(tag = FindTagItem( FSVC_Error, tagList))
    {
        err =(ULONG *) tag->ti_Data;
        *err = error;
    }

    return(service_entity);
}
