#ifndef SANA2_SANA2PACKETMAGIC_H
#define SANA2_SANA2PACKETMAGIC_H 1
/*
**	$Id: sana2packetmagic.h,v 1.1 91/10/16 11:21:49 dlarson Exp $
**
**	Defined types of magic for the Sana2PacketType structure.
**
**	(C) Copyright 1991 Raymond S. Brand
**		All Rights Reserved
**
**	(C) Copyright 1991 Commodore-Amiga Inc.
**		All Rights Reserved
*/

/*
** Contributions from:
**	Raymond S. Brand,   rsbx@cbmvax.commodore.com,  (215) 431-9100
**	Martin Hunt,      martin@cbmvax.commodore.com,  (215) 431-9100
**	Perry Kivolowitz,           ASDG Incorporated,  (608) 273-6585
**	Dale Luck,                    dale@boing.uucp,  (408) 262-1469
*/

#ifndef SANA2_SANA2DEVICE_H
#include <sana2/sana2device.h>
#endif	/* !SANA2_SANA2DEVICE_H */


/*
 * The SANA-II Sana2PacketType structure Magic value is an unsigned 32
 * number. The upper 16 bits identify the type of network wire to which
 * the Magic value applies and the lower 16 bits identify the particular
 * type of magic.
 */


/*
 * defined types of ethernet magic
 */

#define S2MAGIC_ETHERNET_E2	((((S2WireType_Ethernet)&0xffff)<<16)|0x0000)
/*
 * For ethernet, this is the default case. Mask is ignored, Length is 2,
 * Match points to a UWORD to match against the type field of the packet.
 */

#define S2MAGIC_ETHERNET_8023	((((S2WireType_Ethernet)&0xffff)<<16)|0x0001)
/*
 * The caller is declaring his intent to receive any of the valid 802.3
 * ethernet packet types. The Match and Mask fields are ignored, Length
 * is 0.
 * 
 * SANA-II ethernet driver writers need to support a read queue for 802.3
 * packets separate from packets of any non-802.3 type.
 * 
 * A possible flow through an ethernet driver's read interrupt routine
 * might look like the following:
 * 
 * 	...
 * 
 * 	sniff packet type...
 * 
 * 	if (NoSpecificReadWaiting())
 * 		{
 * 		if (IsAn802() && 802ReadWaiting())
 * 			{
 * 			Return802();
 * 			}
 * 		else
 * 			{
 * 			if (OrphanReadWaiting())
 * 				{
 * 				ReturnOrphan();
 * 				}
 * 			else
 * 				{
 * 				DropPacket();
 * 				}
 * 			}
 * 		}
 * 	else
 * 		{
 * 		ReturnSpecificPacket();
 * 		}
 * 
 * 	...
 * 
 */


#endif	/* SANA2_SANA2PACKETMAGIC_H */
