/*
** $Id: servicesinternal.h,v 37.4 92/06/09 15:15:13 kcd Exp $
**
** (C) Copyright 1992, Commodore-Amiga, Inc.
**
** servicesinternal.h - Structures and definitions for the Envoy Services Manager
**
*/

#include <exec/types.h>
#include <utility/tagitem.h>
#include "services.h"

struct Service
{
    struct Node     svc_Node;                   /* Chain of services */
    UBYTE           svc_PathName[256];          /* Who to run to start a server */
    UBYTE           svc_SvcName[64];            /* Name of this service */
    ULONG           svc_Flags;                  /* Various Flags */
    UWORD           svc_UID;                    /* User allowed to use this service */
    UWORD           svc_GID;                    /* Group allowed to use this service */
};

struct FindServiceReq
{
    UBYTE           fsr_Service[64];            /* The name of the service to find */
    UBYTE           fsr_UserName[16];           /* The user wanting to use the service */
    UBYTE           fsr_PassWord[16];           /* The password of the user */
};

struct FindServiceAck
{
    UBYTE           fsa_EntityName[64];
};

#define MGRCMD_FINDSERVICE  100

#define SVCB_EXPUNGE    1
#define SVCF_EXPUNGE    (1 << SVCB_EXPUNGE)

#define MGR_ENTITY_NAME  "Services Manager"

#if SVCS_DEBUG > 0
#define DBMSG(x) kprintf x
#else
#define DBMSG(x)
#endif

