#ifndef  CLIB_BATTCLOCK_PROTOS_H
#define  CLIB_BATTCLOCK_PROTOS_H

/*
**	$Id: battclock_protos.h,v 1.3 90/05/03 11:57:44 rsbx Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
void ResetBattClock( void );
ULONG ReadBattClock( void );
void WriteBattClock( unsigned long time );
#endif   /* CLIB_BATTCLOCK_PROTOS_H */
