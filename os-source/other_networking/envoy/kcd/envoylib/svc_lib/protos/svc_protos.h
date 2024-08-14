#ifndef  CLIB_SVC_PROTOS_H
#define  CLIB_SVC_PROTOS_H

/*
**	$Id: svc_protos.h,v 37.1 92/03/22 18:29:23 kcd Exp Locker: kcd $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  APPN_NIPC_H
#include <appn/nipc.h>
#endif
/*--- functions in V37 or higher (Release 2.04) ---*/

void RexxReserved( void );
ULONG StartServiceA( struct TagItem *tagList );
ULONG StartService( Tag Tag1, ... );
void GetServiceAttrsA( struct TagItem *tagList );
void GetServiceAttrs( Tag Tag1, ... );
#endif   /* CLIB_SVC_PROTOS_H */
