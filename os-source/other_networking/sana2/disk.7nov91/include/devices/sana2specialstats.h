#ifndef SANA2_SANA2SPECIALSTATS_H
#define SANA2_SANA2SPECIALSTATS_H 1
/*
**	$Id: sana2specialstats.h,v 1.2 91/10/17 12:59:47 dlarson Exp $
**
**	Defined ids for SANA-II special statistics.
**
**
**	(C) Copyright 1991 Commodore-Amiga Inc.
**		All Rights Reserved
*/


#ifndef SANA2_SANA2DEVICE_H
#include <sana2/sana2device.h>
#endif	/* !SANA2_SANA2DEVICE_H */


/*
** The SANA-II special statistic identifier is an unsigned 32 number.
** The upper 16 bits identify the type of network wire type to which
** the statistic applies and the lower 16 bits identify the particular
** statistic.
**
** If you desire to add a new statistic identifier, contacts CATS.
*/



/*
** defined ethernet special statistics
*/

#define S2SS_ETHERNET_BADMULTICAST	((((S2WireType_Ethernet)&0xffff)<<16)|0x0000)
/*
** This count will record the number of times a received packet tripped
** the hardware's multicast filtering mechanism but was not actually in
** the current multicast table.
*/

#define S2SS_ETHERNET_RETRIES		((((S2WireType_Ethernet)&0xffff)<<16)|0x0001)
/*
** This count records the total number of retries which have resulted
** from transmissions on this board.
*/


#endif	/* SANA2_SANA2SPECIALSTATS_H */
