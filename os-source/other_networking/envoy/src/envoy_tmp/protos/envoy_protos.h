#ifndef  CLIB_ENVOY_PROTOS_H
#define  CLIB_ENVOY_PROTOS_H
/*
**	$Id: envoy_protos.h,v 1.8 92/06/25 16:47:18 gregm Exp $
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
/*--- functions in V39 or higher (distributed as Release 3.0) ---*/
BOOL HostRequestA( struct TagItem *tags );
BOOL HostRequest( Tag tag1, ... );
BOOL LoginRequestA( struct TagItem *tags );
BOOL LoginRequest( Tag tag1, ... );
BOOL UserRequestA( struct TagItem *tags );
BOOL UserRequest( Tag tag1, ... );
#endif   /* CLIB_ENVOY_PROTOS_H */
