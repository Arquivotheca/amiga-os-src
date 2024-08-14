#ifndef  CLIB_TIMER_PROTOS_H
#define  CLIB_TIMER_PROTOS_H
/*
**	$Id: timer_protos.h,v 1.6 91/01/25 15:46:51 rsbx Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
#ifndef  DEVICES_TIMER_H
#include <devices/timer.h>
#endif
void AddTime( struct timeval *dest, struct timeval *src );
void SubTime( struct timeval *dest, struct timeval *src );
LONG CmpTime( struct timeval *dest, struct timeval *src );
ULONG ReadEClock( struct EClockVal *dest );
void GetSysTime( struct timeval *dest );
#endif   /* CLIB_TIMER_PROTOS_H */
