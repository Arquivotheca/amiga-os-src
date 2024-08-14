#ifndef  CLIB_MISC_PROTOS_H
#define  CLIB_MISC_PROTOS_H
/*
**	$Id: misc_protos.h,v 36.2 90/11/07 14:46:52 bryce Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
UBYTE *AllocMiscResource( unsigned long unitNum, UBYTE *name );
void FreeMiscResource( unsigned long unitNum );
#endif   /* CLIB_MISC_PROTOS_H */
