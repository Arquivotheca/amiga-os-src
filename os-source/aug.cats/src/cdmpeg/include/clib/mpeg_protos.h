#ifndef  CLIB_MPEG_PROTOS_H
#define  CLIB_MPEG_PROTOS_H

/*
**	$Id: mpeg_protos.h,v 1.6 91/01/25 15:46:51 rsbx Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef  DEVICES_MPEG_H
#include <devices/mpeg.h>
#endif
void SetSCR( struct Unit *unit, unsigned long time );
ULONG GetSCR( struct Unit *unit );
#endif   /* CLIB_MPEG_PROTOS_H */
