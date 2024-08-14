#ifndef  CLIB_SERVICES_PROTOS_H
#define  CLIB_SERVICES_PROTOS_H
/*
**	$Id: services_protos.h,v 37.3 92/06/11 14:08:56 kcd Exp $
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
#ifndef  ENVOY_SERVICES_H
#include <envoy/services.h>
#endif
/*--- functions in V37 or higher (distributed as Release 2.04) ---*/

APTR FindServiceA( STRPTR remoteHost, STRPTR serviceName, APTR srcEntity,
	struct TagItem *tagList );
APTR FindService( STRPTR remoteHost, STRPTR serviceName, APTR srcEntity,
	Tag Tag1, ... );
void LoseService( APTR entity );
#endif   /* CLIB_SERVICES_PROTOS_H */
