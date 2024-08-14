    IFND    ENVOY_NIPC_I
ENVOY_NIPC_I    SET     1

**
**      $Id: nipc.i,v 1.1 93/09/03 18:18:41 kcd Exp $
**
**      Tags, types and structures for the nipc.library.
**
**      (C) Copyright 1992 Commodore-Amiga, Inc.
**          All Rights Reserved
**

        include     "exec/types.i"
        include     "exec/lists.i"
        include     "exec/ports.i"
        include     "utility/tagitem.i"

TRN_Dummy           SET TAG_USER+$0B1100
TRN_AllocReqBuffer  SET TRN_Dummy+1
TRN_AllocRespBuffer SET TRN_Dummy+2
TRN_ReqDataNIPCBuff SET TRN_Dummy+3
TRN_RespDataNIPCBuff SET TRN_Dummy+4

ENT_Dummy           SET TAG_USER+$0B1000
ENT_Name            SET ENT_Dummy+1
ENT_Public          SET ENT_Dummy+2
ENT_Signal          SET ENT_Dummy+3
ENT_AllocSignal     SET ENT_Dummy+4
ENT_TimeoutLinks    SET ENT_Dummy+5
ENT_NameLength      SET ENT_Dummy+6
ENT_Release         SET ENT_Dummy+7
ENT_Inherit         SET ENT_Dummy+8

    STRUCTURE       Transaction,0
        STRUCT      trans_Msg,MN_SIZE   ** Messages are used as the carrier for local Transactions
        APTR        trans_SourceEntity  ** Filled in by nipc.library
        APTR        trans_DestinationEntity ** Filled in by nipc.library
        UBYTE       trans_Command       ** A field that's server-proprietary
        UBYTE       trans_Type          ** Request, Servicing, Response
        ULONG       trans_Error         ** Error code
        ULONG       trans_Flags         ** See below
        ULONG       trans_Sequence      ** Used internally
        APTR        trans_RequestData   ** Pointer to Request data buffer
        ULONG       trans_ReqDataLength ** The length of the above buffer
        ULONG       trans_ReqDataActual ** The length of valid data in above buffer
        APTR        trans_ResponseData  ** Pointer to Response data buffer
        ULONG       trans_RespDataLength ** The length of the above buffer
        ULONG       trans_RespDataActual ** The length of valid data in above buffer
        UWORD       trans_Timeout       ** Number of seconds before a timeout
        UWORD       trans_Reserved      ** Reserved for future use
        ULONG       trans_ClientPrivate ** Client specific user data field
        ULONG       trans_ServerPrivate ** Server specific user data field
        LABEL       trans_SIZE

** Types
TYPE_REQUEST        SET 0               ** Waiting to be serviced
TYPE_RESPONSE       SET 1               ** Has been serviced
TYPE_SERVICING      SET 2               ** Currently being serviced

** Flags
        BITDEF      TRANS,REQBUFFERALLOC,0      ** Set if FreeTransaction is responsible for freeing the buffer
        BITDEF      TRANS,RESPBUFFERALLOC,1     ** Set if FreeTransaction is responsible for freeing the buffer
        BITDEF      TRANS,NOWAIT,2              ** BeginTransaction isn't allowed to Wait() on this transaction
        BITDEF      TRANS,REQUESTTABLE,3        ** The RequestData for this Transaction is a table ptr (obsolete)
        BITDEF      TRANS,RESPONSETABLE,4       ** The ResponseData for this Transaction is a table ptr (obsolete)
        BITDEF      TRANS,REQNIPCBUFF,5         ** trans_RequestData points to a struct NIPCBuff
        BITDEF      TRANS,RESPNIPC,6            ** trans_ResponseData points to a struct NIPCBuff

    STRUCTURE       NIPCBuff,0
        STRUCT      nbuff_Link,MLN_SIZE
        STRUCT      nbuff_Entries,MLH_SIZE

        ; No size here...this is nipc private.

    STRUCTURE       NIPCBuffEntry,0
        STRUCT      nbe_Link,MLN_SIZE
        ULONG       nbe_Offset
        ULONG       nbe_Length
        ULONG       nbe_PhysicalLength
        APTR        nbe_Data

        ; No size here either. Hands off.

* Currently defined tags for NIPCInquiry.  Please see nipc.h for descriptions of these.


QUERY_IPADDR        SET     TAG_USER+$2000
MATCH_IPADDR        SET     TAG_USER+$2001

QUERY_REALMS        SET     TAG_USER+$2002
MATCH_REALMS        SET     TAG_USER+$2003

QUERY_HOSTNAME      SET     TAG_USER+$2004
MATCH_HOSTNAME      SET     TAG_USER+$2005

QUERY_SERVICE       SET     TAG_USER+$2006
MATCH_SERVICE       SET     TAG_USER+$2007

QUERY_ENTITY        SET     TAG_USER+$2008
MATCH_ENTITY        SET     TAG_USER+$2009

QUERY_OWNER         SET     TAG_USER+$200A
MATCH_OWNER         SET     TAG_USER+$200B

QUERY_MACHDESC      SET     TAG_USER+$200C
MATCH_MACHDESC      SET     TAG_USER+$200D

QUERY_ATTNFLAGS     SET     TAG_USER+$200E
MATCH_ATTNFLAGS     SET     TAG_USER+$200F

** QUERY LIBVERSION and MATCH_LIBVERSION are not yet implemented *
QUERY_LIBVERSION    SET     TAG_USER+$2010
MATCH_LIBVERSION    SET     TAG_USER+$2011
** QUERY LIBVERSION and MATCH_LIBVERSION are not yet implemented *

QUERY_CHIPREVBITS   SET     TAG_USER+$2012
MATCH_CHIPREVBITS   SET     TAG_USER+$2013

QUERY_MAXFASTMEM    SET     TAG_USER+$2014
MATCH_MAXFASTMEM    SET     TAG_USER+$2015

QUERY_AVIALFASTMEM  SET     TAG_USER+$2016
MATCH_AVIALFASTMEM  SET     TAG_USER+$2017

QUERY_MAXCHIPMEM    SET     TAG_USER+$2018
MATCH_MAXCHIPMEM    SET     TAG_USER+$2019

QUERY_AVAILCHIPMEM  SET     TAG_USER+$2020
MATCH_AVAILCHIPMEM  SET     TAG_USER+$2021

QUERY_KICKVERSION   SET     TAG_USER+$2022
MATCH_KICKVERSION   SET     TAG_USER+$2023

QUERY_WBVERSION     SET     TAG_USER+$2024
MATCH_WBVERSION     SET     TAG_USER+$2025

QUERY_NIPCVERSION   SET     TAG_USER+$2026
MATCH_NIPCVERSION   SET     TAG_USER+$2027


        ENDC
