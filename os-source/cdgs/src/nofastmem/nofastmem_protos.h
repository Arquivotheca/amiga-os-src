#ifndef  CLIB_NOFASTMEM_PROTOS_H
#define  CLIB_NOFASTMEM_PROTOS_H
/*
**	$Id: nofastmem_protos.h,v 1.1 94/01/26 10:43:43 jerryh Exp Locker: jerryh $
**
**	C prototypes
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
/* "nofastmem.resource" */
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
void RemoveFastMem( void );
void RestoreFastMem( void );
#endif   /* CLIB_NOFASTMEM_PROTOS_H */
