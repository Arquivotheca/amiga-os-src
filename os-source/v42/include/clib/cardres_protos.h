#ifndef  CLIB_CARDRES_PROTOS_H
#define  CLIB_CARDRES_PROTOS_H
/*
**	$Id: cardres_protos.h,v 1.1 91/12/04 12:29:38 darren Exp Locker: darren $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  EXEC_RESIDENT_H
#include <exec/resident.h>
#endif
#ifndef  RESOURCES_CARD_H
#include <resources/card.h>
#endif
struct CardHandle *OwnCard( struct CardHandle *handle );
void ReleaseCard( struct CardHandle *handle, unsigned long flags );
struct CardMemoryMap *GetCardMap( void );
BOOL BeginCardAccess( struct CardHandle *handle );
BOOL EndCardAccess( struct CardHandle *handle );
UBYTE ReadCardStatus( void );
BOOL CardResetRemove( struct CardHandle *handle, unsigned long flag );
UBYTE CardMiscControl( struct CardHandle *handle,
	unsigned long control_bits );
ULONG CardAccessSpeed( struct CardHandle *handle, unsigned long nanoseconds );
LONG CardProgramVoltage( struct CardHandle *handle, unsigned long voltage );
BOOL CardResetCard( struct CardHandle *handle );
BOOL CopyTuple( struct CardHandle *handle, UBYTE *buffer,
	unsigned long tuplecode, unsigned long size );
ULONG DeviceTuple( UBYTE *tuple_data, struct DeviceTData *storage );
struct Resident *IfAmigaXIP( struct CardHandle *handle );
BOOL CardForceChange( void );
ULONG CardChangeCount( void );
ULONG CardInterface( void );
#endif   /* CLIB_CARDRES_PROTOS_H */
