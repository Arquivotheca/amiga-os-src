#ifndef  CLIB_FSD_PROTOS_H
#define  CLIB_FSD_PROTOS_H
/*
**	$Id: fsd_protos.h,v 1.2 93/03/10 13:16:09 gregm Exp Locker: gregm $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  ENVOY_NIPC_H
#include <envoy/nipc.h>
#endif
/*--- functions in V37 or higher (distributed as Release 2.04) ---*/

void RexxReserved( void );
ULONG StartServiceA( struct TagItem *tagList );
ULONG StartService( Tag Tag1, ... );
void GetServiceAttrsA( struct TagItem *tagList );
void GetServiceAttrs( Tag Tag1, ... );
void CleanupDeadMount( struct MountedFSInfo *m );
void SetFSMode( unsigned long mode );
#endif   /* CLIB_FSD_PROTOS_H */
