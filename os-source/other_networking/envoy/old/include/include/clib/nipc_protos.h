#ifndef  CLIB_NIPC_PROTOS_H
#define  CLIB_NIPC_PROTOS_H
/*
**	$Id: nipc_protos.h,v 1.8 92/06/25 16:47:18 kcd Exp $
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
struct Transaction *AllocTransactionA( struct Tagitem *tags );
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
#endif   /* CLIB_NIPC_PROTOS_H */
