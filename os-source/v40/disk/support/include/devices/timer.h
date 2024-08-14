#ifndef DEVICES_TIMER_H
#define DEVICES_TIMER_H 1
/*
**	$VER: timer.h 36.16 (25.1.91)
**	Includes Release 40.15
**
**	Timer device name and useful definitions.
**
**	(C) Copyright 1985-1993 Commodore-Amiga Inc.
**		All Rights Reserved
*/

#include <exec/types.h>
#include <exec/io.h>

/* unit defintions */
#define UNIT_MICROHZ	0
#define UNIT_VBLANK	1
#define UNIT_ECLOCK	2
#define UNIT_WAITUNTIL	3
#define	UNIT_WAITECLOCK	4

#define TIMERNAME	"timer.device"

struct timeval {
    ULONG tv_secs;
    ULONG tv_micro;
};

struct EClockVal {
    ULONG ev_hi;
    ULONG ev_lo;
};

struct timerequest {
    struct IORequest tr_node;
    struct timeval tr_time;
};

/* IO_COMMAND to use for adding a timer */
#define TR_ADDREQUEST	CMD_NONSTD
#define TR_GETSYSTIME	(CMD_NONSTD+1)
#define TR_SETSYSTIME	(CMD_NONSTD+2)

#endif /* DEVICES_TIMER_H */
