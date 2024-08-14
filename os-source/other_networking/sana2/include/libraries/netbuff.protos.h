#ifndef LIBRARIES_NETBUFF_PROTOS_H
#define LIBRARIES_NETBUFF_PROTOS_H 1
/*
**	$Id$
**
**	NetBuff library function prototype definitions.
**
**	(C) Copyright 1991 Raymond S. Brand
**		All Rights Reserved
**
**	(C) Copyright 1991 Commodore-Amiga Inc.
**		All Rights Reserved
*/


#ifndef LIBRARIES_NETBUFF_H
#include <netbuff.h>
#endif /* !LIBRARIES_NETBUFF_H */


extern void AllocSegments( ULONG Count, struct MinList *Segment_List );
extern void IntAllocSegments( ULONG Count, struct MinList *Segment_List );
extern void FreeSegments( struct MinList *Segment_List );
extern void IntFreeSegments( struct MinList *Segment_List );
extern LONG SplitNetBuff( struct NetBuff *NetBuff0, LONG Count, struct NetBuff *NetBuff1 );
extern LONG TrimNetBuff( struct NetBuff *NetBuff, LONG Count );
extern LONG CopyToNetBuff( struct NetBuff *NetBuff, LONG Offset, UBYTE *Data, ULONG Count );
extern LONG CopyFromNetBuff( struct NetBuff *NetBuff, LONG Offset, UBYTE *Data, ULONG Count );
extern LONG CopyNetBuff( struct NetBuff *NetBuff0, struct NetBuff *NetBuff1 );
extern LONG CompactNetBuff( struct NetBuff *NetBuff );
extern LONG ReadyNetBuff( struct NetBuff *NetBuff, ULONG Count );
extern LONG IsContiguous( struct NetBuff *NetBuff, LONG Offset, ULONG Count );
extern LONG NetBuffAppend( struct NetBuff *NetBuff0, struct NetBuff *NetBuff1 );
extern LONG PrependNetBuff( struct NetBuff *NetBUff0, struct NetBuff *NetBuff1 );
extern LONG ReclaimSegments( struct NetBuff *NetBuff );


#endif	/* LIBRARIES_NETBUFF_PROTOS_H */
