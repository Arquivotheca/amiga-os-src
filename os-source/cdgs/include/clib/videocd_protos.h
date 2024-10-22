#ifndef  CLIB_VIDEOCD_PROTOS_H
#define  CLIB_VIDEOCD_PROTOS_H

/*
**	$Id: videocd_protos.h,v 40.0 93/09/14 15:22:00 davidj Exp Locker: davidj $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  EXEC_LISTS_H
#include <exec/lists.h>
#endif
#ifndef  UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif
#ifndef  REXX_STORAGE_H
#include <rexx/storage.h>
#endif
/*--- functions in V37 or higher (Release 2.04) ---*/

/* Public entries */

LONG GetCDTypeA( STRPTR pathName, struct TagItem *attrs );
APTR ObtainCDHandleA( STRPTR pathName, struct TagItem *attrs );
APTR ObtainCDHandle( STRPTR pathName, Tag Tag1, ... );
void ReleaseCDHandle( APTR handle );
struct TagItem *GetVideoCDInfoA( APTR handle, unsigned long seqNum,
	struct TagItem *attrs );
struct TagItem *GetVideoCDInfo( APTR handle, unsigned long seqNum, Tag Tag1,
	... );
void FreeVideoCDInfo( struct TagItem *attrs );
#endif   /* CLIB_VIDEOCD_PROTOS_H */
