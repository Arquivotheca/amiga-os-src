    IFND    ENVOY_NIPC_I
ENVOY_NIPC_I    SET     1

**
**      $Id: nipc.i,v 1.11 92/06/25 16:45:28 kcd Exp $
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

ENT_Dummy           SET TAG_USER+$0B1000
ENT_Name            SET ENT_Dummy+1
ENT_Public          SET ENT_Dummy+2
ENT_Signal          SET ENT_Dummy+3
ENT_AllocSignal     SET ENT_Dummy+4

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
        LABEL       trans_SIZE

** Types
TYPE_REQUEST        SET 0               ** Waiting to be serviced
TYPE_RESPONSE       SET 1               ** Has been serviced
TYPE_SERVICING      SET 2               ** Currently being serviced

** Flags
        BITDEF      TRANS,REQBUFFERALLOC,0      ** Set if FreeTransaction is responsible for freeing the buffer
        BITDEF      TRANS,RESPBUFFERALLOC,1     ** Set if FreeTransaction is responsible for freeing the buffer

* Currently defined tags for NIPCInquiry


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

QUERY_SYSINFO       SET     TAG_USER+$200E
MATCH_SYSINFO       SET     TAG_USER+$200F

QUERY_LIBVERSION    SET     TAG_USER+$2010
MATCH_LIBVERSION    SET     TAG_USER+$2011

        ENDC
