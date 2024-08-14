#ifndef APPN_NIPC_H
#define APPN_NIPC_H
/*
**      $Id: nipc.h,v 1.8 92/04/05 18:12:07 gregm Exp $
**
**      Tags, types and structures for the nipc.library.
**
**      (C) Copyright 1992 Commodore-Amiga, Inc.
**          All Rights Reserved
*/

#include <exec/types.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <utility/tagitem.h>


#define TRN_Dummy (TAG_USER + 0xB1100)
#define TRN_AllocReqBuffer  (TRN_Dummy + 1)
#define TRN_AllocRespBuffer (TRN_Dummy + 2)

#define ENT_Dummy (TAG_USER + 0xB1000)
#define ENT_Name        (ENT_Dummy + 1)
#define ENT_Public      (ENT_Dummy + 2)
#define ENT_Signal      (ENT_Dummy + 3)
#define ENT_AllocSignal (ENT_Dummy + 4)



struct Transaction
   {
   struct Message trans_Msg;            /* Messages are used as the carrier for local transactions */
   struct Entity *trans_SourceEntity;   /* Filled in by ipc.library */
   struct Entity *trans_DestinationEntity;
                                        /* Filled in by ipc.library */
   UBYTE          trans_Command;        /* A field that's server-proprietary */
   UBYTE          trans_Type;           /* Req, Response */
   ULONG          trans_Error;          /* any error values */
   ULONG          trans_Flags;          /* see below */
   ULONG          trans_Sequence;       /* Used by the library to maintain sequences */
   APTR           trans_RequestData;    /* ptr to request data buffer */
   ULONG          trans_ReqDataLength;  /* The length of data in the above buffer */
   ULONG          trans_ReqDataActual;  /* The length of valid data in the request buffer */
   APTR           trans_ResponseData;   /* ptr to response data buffer */
   ULONG          trans_RespDataLength; /* The size of the buffer above */
   ULONG          trans_RespDataActual; /* When data is returned in a response, the amount of the buffer used */
   UWORD          trans_Timeout;        /* Number of seconds before a timeout */
   };

/* Types */
#define TYPE_REQUEST          0     /* Waiting to be serviced */
#define TYPE_RESPONSE         1     /* Has been serviced */
#define TYPE_SERVICING        2     /* Currently being serviced */

/* Flags */
#define TRANSF_REQBUFFERALLOC  1    /* set if FreeTransaction is responsible for freeing the buffer */
#define TRANSF_RESPBUFFERALLOC 2    /* set if FreeTransaction is responsible for freeing the buffer */
#define TRANSF_NOWAIT          4    /* BeginTransaction isn't allowed to Wait() on this transaction */
#define TRANSF_REQUESTTABLE    8    /* The RequestData for this transaction is a table ptr */
#define TRANSF_RESPONSETABLE  16    /* The ResponseData for this transaction is a table ptr */

#define TRANSB_REQBUFFERALLOC  0    /* set if FreeTransaction is responsible for freeing the buffer */
#define TRANSB_RESPBUFFERALLOC 1    /* set if FreeTransaction is responsible for freeing the buffer */
#define TRANSB_NOWAIT          2    /* BeginTransaction isn't allowed to Wait() on this transaction */
#define TRANSB_REQUESTTABLE    3    /* The RequestData for this transaction is a table ptr */
#define TRANSB_RESPONSETABLE   4    /* The ResponseData for this transaction is a table ptr */

#endif APPN_NIPC_H

