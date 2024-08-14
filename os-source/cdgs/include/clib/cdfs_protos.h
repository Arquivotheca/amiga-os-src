#ifndef  CLIB_CDFS_PROTOS_H
#define  CLIB_CDFS_PROTOS_H
/*
**	$Id: cdfs_protos.h,v 1.1 92/07/24 14:12:26 jerryh Exp Locker: jerryh $
**
**	C prototypes
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
/* "cdfs.library" */
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
void ResetCDTV( void );
LONG SetDebug( long level );
LONG ValidDisk( void );
void MountFS( void );
struct TMInfo *GetTMInfo( void );
LONG IsNoPrefs( void );
#endif   /* CLIB_CDFS_PROTOS_H */
