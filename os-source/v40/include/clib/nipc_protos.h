#ifndef  CLIB_NIPC_PROTOS_H
#define  CLIB_NIPC_PROTOS_H

/*
**	$Id: nipc_protos.h,v 1.10 93/03/04 13:32:58 gregm Exp Locker: kcd $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif
#ifndef  ENVOY_NIPC_H
#include <envoy/nipc.h>
#endif
/*--- functions in V39 or higher (Release 3) ---*/
struct Transaction *AllocTransactionA( struct TagItem *tags );
struct Transaction *AllocTransaction( Tag tag1, ... );
APTR FreeTransaction( struct Transaction *transaction );
/*---------------- Entities ---------------------------------------------------*/
struct Entity *CreateEntityA( struct TagItem *tags );
struct Entity *CreateEntity( Tag tag1, ... );
void DeleteEntity( struct Entity *entity );
struct Entity *FindEntity( STRPTR hostname, STRPTR entityname,
	struct Entity *src_entity, ULONG *detailerror );
void LoseEntity( struct Entity *entity );
/*---------------- NIPC I/O ---------------------------------------------------*/
BOOL DoTransaction( struct Entity *dest_entity, struct Entity *src_entity,
	struct Transaction *transaction );
BOOL BeginTransaction( struct Entity *dest_entity, struct Entity *src_entity,
	struct Transaction *transaction );
struct Transaction *GetTransaction( struct Entity *entity );
void ReplyTransaction( struct Transaction *transaction );
BOOL CheckTransaction( struct Transaction *transaction );
void AbortTransaction( struct Transaction *transaction );
void WaitTransaction( struct Transaction *transaction );
void WaitEntity( struct Entity *entity );
/*---------------- Network Information ----------------------------------------*/
BOOL GetEntityName( struct Entity *entity, STRPTR string,
	unsigned long maxlen );
BOOL GetHostName( struct Entity *entity, STRPTR string,
	unsigned long maxlen );
BOOL NIPCInquiryA( struct Hook *hook, unsigned long maxTime,
	unsigned long maxResponses, struct TagItem *tagList );
BOOL NIPCInquiry( struct Hook *hook, unsigned long maxTime,
	unsigned long maxResponses, Tag tag1, ... );
ULONG PingEntity( struct Entity *pingtarget, unsigned long maxTime );
ULONG GetEntityAttrsA( struct Entity *entity, struct TagItem *tagList );
ULONG GetEntityAttrs( struct Entity *entity, Tag tag1, ... );
void SetEntityAttrsA( struct Entity *entity, struct TagItem *tagList );
void SetEntityAttrs( struct Entity *entity, Tag tag1, ... );
/*---------------- NIPC Buffer Management Routines ----------------------------*/
/*--- functions in V40 or higher (Release 3.01) ---*/
struct NIPCBuff *AllocNIPCBuff( unsigned long entries );
struct NIPCBuffEntry *AllocNIPCBuffEntry( void );
ULONG CopyNIPCBuff( struct NIPCBuff *src_buff, struct NIPCBuff *dest_buff,
	unsigned long offset, unsigned long length );
ULONG CopyToNIPCBuff( UBYTE *src_data, struct NIPCBuff *dest_buff,
	unsigned long length );
ULONG CopyFromNIPCBuffer( struct NIPCBuff *src_buff, UBYTE *dest_data,
	unsigned long length );
void FreeNIPCBuff( struct NIPCBuff *buff );
void FreeNIPCBuffEntry( struct NIPCBuffEntry *entry );
ULONG NIPCBuffLength( struct NIPCBuff *buff );
void AppendNIPCBuff( struct NIPCBuff *first, struct NIPCBuff *second );
UBYTE *NIPCBuffPointer( struct NIPCBuff *buff, struct NIPCBuffEntry **,
	unsigned long offset );
#endif   /* CLIB_NIPC_PROTOS_H */
