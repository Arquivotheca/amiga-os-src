#ifndef LIBRARIES_NETBUFF_H
#define LIBRARIES_NETBUFF_H 1
/*
**	$Id$
**
**	NetBuff library structure definitions.
**
**	(C) Copyright 1991 Raymond S. Brand
**		All Rights Reserved
**
**	(C) Copyright 1991 Commodore-Amiga Inc.
**		All Rights Reserved
*/


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif /* !EXEC_TYPES_H */
#ifndef EXEC_LISTS_H
#include <exec/lists.h>
#endif /* !EXEC_LISTS_H */


#define NETBUFFNAME	"netbuff.library"


struct NetBuffSegment
	{
	struct MinNode Node;	/* segment links               */
	ULONG PhysicalSize;	/* buffer size of this segment */
	ULONG DataOffset;	/* offset to valid data        */
	ULONG DataCount;	/* valid bytes in this segment */
	UBYTE *Buffer;		/* pointer to data buffer area */
	};


struct NetBuff
	{
	struct MinList List;	/* segments                  */
	ULONG Count;		/* amount of data in NetBuff */
	};


#endif	/* LIBRARIES_NETBUFF_H */
