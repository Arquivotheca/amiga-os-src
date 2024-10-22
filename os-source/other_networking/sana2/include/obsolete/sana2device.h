#ifndef SANA2_SANA2DEVICE_H
#define SANA2_SANA2DEVICE_H 1
/*
**	$Id: sana2device.h,v 1.1 91/10/16 11:21:56 dlarson Exp Locker: dlarson $
**
**	Structure definitions for SANA-II devices.
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


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif /* !EXEC_TYPES_H */
#ifndef EXEC_PORTS_H
#include <exec/ports.h>
#endif /* !EXEC_PORTS_H */
#ifndef EXEC_IO_H
#include <exec/io.h>
#endif /* !EXEC_IO_H */
#ifndef DEVICES_TIMER_H
#include <devices/timer.h>
#endif /* !DEVICES_TIMER_H */
#ifndef LIBRARIES_NETBUFF_H
#include <libraries/netbuff.h>
#endif /* !LIBRARIES_NETBUFF_H */


#define SANA2_MAX_ADDR_BITS	(128)
#define SANA2_MAX_ADDR_BYTES	((SANA2_MAX_ADDR_BITS+7)/8)


struct IOSana2Req
	{
	struct Message S2io_Message;
	struct Device *S2io_Device;	/* device node pointer		*/
	struct Unit   *S2io_Unit;	/* unit (driver private)	*/

	UWORD S2io_Command;		/* device command		*/
	UBYTE S2io_Flags;		/* command flags		*/
	BYTE  S2io_Error;		/* generic error or warning	*/
	ULONG S2io_WireError;		/* wire type specific error	*/

	struct Sana2PacketType *S2io_PacketType;	/* packet type	*/
	UBYTE S2io_SrcAddr[SANA2_MAX_ADDR_BYTES];	/* source addr	*/
	UBYTE S2io_DstAddr[SANA2_MAX_ADDR_BYTES];	/* dest address	*/
	ULONG S2io_DataLength;		/* from header			*/
	struct NetBuff S2io_Body;	/* packet data			*/
	void *S2io_StatData;		/* statics data pointer		*/
	};


/*
 * defines for the S2io_Flags field
 */

#define SANA2IOB_RAW	(7)		/* raw packet IO requested	*/
#define SANA2IOF_RAW	(1<<SANA2IOB_RAW)

#define	SANA2IOB_BCAST	(6)		/* broadcast packet (received)	*/
#define	SANA2IOF_BCAST	(1<<SANA2IOB_BCAST)

#define	SANA2IOB_MCAST	(5)		/* multicast packet (received)	*/
#define	SANA2IOF_MCAST	(1<<SANA2IOB_MCAST)

#define SANA2IOB_QUICK	(IOB_QUICK)	/* quick IO requested (0)	*/
#define SANA2IOF_QUICK	(IOF_QUICK)


/*
 * defines for OpenDevice()
 */

#define SANA2OPB_MINE	(0)		/* exclusive access requested	*/
#define SANA2OPF_MINE	(1<<SANA2OPB_MINE)

#define SANA2OPB_PROM	(1)		/* promiscuous mode requested	*/
#define SANA2OPF_PROM	(1<<SANA2OPB_PROM)


struct Sana2PacketType
	{
	ULONG CanonicalType;		/* used by higher levels	*/
	ULONG Magic;			/* interpretation code		*/
	ULONG Length;			/* length of match data		*/
	UBYTE *Match;			/* bytes to compare		*/
	UBYTE *Mask;			/* mask for comparison		*/
	};


struct Sana2DeviceQuery
	{
	/*
	 * Standard information
	 */
	ULONG	SizeAvailable;		/* bytes available		*/
	ULONG	SizeSupplied;		/* bytes supplied		*/
	LONG	DevQueryFormat;		/* this is type 0		*/
	LONG	DeviceLevel;		/* this document is level 0	*/
	/*
	 * Common information
	 */
	UWORD	AddrFieldSize;		/* address size in bits		*/
	ULONG	MTU;			/* maximum packet data size	*/
	LONG	bps;			/* line rate (bits/sec)		*/
	LONG	HardwareType;		/* what the wire is		*/
	/*
	 * Format specific information
	 */
	};


/*
 * defined Hardware types
 */

#define S2WireType_Ethernet		1
#define S2WireType_Arcnet		2


struct Sana2PacketTypeStats
	{
	LONG PacketsSent;		/* transmitted count		*/
	LONG PacketsReceived;		/* received count		*/
	LONG BytesSent;			/* bytes transmitted count	*/
	LONG BytesReceived;		/* bytes received count		*/
	LONG PacketsDropped;		/* packets dropped count	*/
	};


struct Sana2SpecialStatRecord
	{
	ULONG Type;			/* statistic identifier		*/
	LONG Count;			/* the statistic		*/
	char *String;			/* statistic name		*/
	};


struct Sana2SpecialStatHeader
	{
	ULONG RecordCountMax;		/* room available		*/
	ULONG RecordCountSupplied;	/* number supplied		*/
	/* struct Sana2SpecialStatRecord[RecordCountMax]; */
	};


struct Sana2DeviceStats
	{
	LONG packets_received;		/* received count		*/
	LONG packets_sent;		/* transmitted count		*/
	LONG framing_errors;		/* framming errors found	*/
	LONG bad_data;			/* bad packets received		*/
	LONG hard_misses;		/* hardware miss count		*/
	LONG soft_misses;		/* software miss count		*/
	LONG unknown_types_received;	/* orphan count			*/
	LONG fifo_overruns;		/* hardware overruns		*/
	LONG fifo_underruns;		/* hardware underruns		*/
	LONG reconfigurations;		/* network reconfigurations	*/
	struct timeval last_start;	/* time of last online		*/
	};


/*
 * Device Commands
 */

#define SANA2_CMD_START			(CMD_NONSTD)

#define SANA2CMD_DEVICEQUERY		(SANA2_CMD_START+ 0)
#define SANA2CMD_GETSTATIONADDRESS	(SANA2_CMD_START+ 1)
#define SANA2CMD_CONFIGINTERFACE	(SANA2_CMD_START+ 2)
#define SANA2CMD_ADDSTATIONALIAS	(SANA2_CMD_START+ 3)
#define SANA2CMD_DELSTATIONALIAS	(SANA2_CMD_START+ 4)
#define SANA2CMD_ADDMULTICASTADDRESS	(SANA2_CMD_START+ 5)
#define SANA2CMD_DELMULTICASTADDRESS	(SANA2_CMD_START+ 6)
#define SANA2CMD_MULTICAST		(SANA2_CMD_START+ 7)
#define SANA2CMD_BROADCAST		(SANA2_CMD_START+ 8)
#define SANA2CMD_TRACKTYPE		(SANA2_CMD_START+ 9)
#define SANA2CMD_UNTRACKTYPE		(SANA2_CMD_START+10)
#define SANA2CMD_GETTYPESTATS		(SANA2_CMD_START+11)
#define SANA2CMD_GETSPECIALSTATS	(SANA2_CMD_START+12)
#define SANA2CMD_GETGLOBALSTATS		(SANA2_CMD_START+13)
#define SANA2CMD_ONEVENT		(SANA2_CMD_START+14)
#define SANA2CMD_READORPHAN		(SANA2_CMD_START+15)
#define SANA2CMD_ONLINE			(SANA2_CMD_START+16)
#define SANA2CMD_OFFLINE		(SANA2_CMD_START+17)

#define SANA2_CMD_END			(SANA2_CMD_START+18)


/*
 * defined errors for S2io_Error
 */

#define S2ERR_NO_ERROR		0	/* peachy-keen			*/
#define S2ERR_NO_RESOURCES	1	/* resource allocation failure	*/
#define S2ERR_UNKNOWN_ENTITY	2	/* unable to find something	*/
#define S2ERR_BAD_ARGUMENT	3	/* garbage somewhere		*/
#define S2ERR_BAD_STATE		4	/* inappropriate state		*/
#define S2ERR_BAD_ADDRESS	5	/* who?				*/
#define	S2ERR_MTU_EXCEEDED	6	/* too much to chew		*/
#define	S2ERR_BAD_PROTOCOL	7	/* bad packet type structure	*/
#define S2ERR_NOT_SUPPORTED	8	/* command not supported	*/
#define S2ERR_SOFTWARE		9	/* software error detected	*/


/*
 * defined errors for S2io_WireError
 */

#define S2WERR_GENERIC_ERROR	0	/* no specific info available	*/
#define	S2WERR_NOT_CONFIGURED	1	/* unit not configured		*/
#define S2WERR_UNIT_ONLINE	2	/* unit is currently online	*/
#define S2WERR_UNIT_OFFLINE	3	/* unit is currently offline	*/
#define	S2WERR_ALREADY_TRACKED	4	/* protocol already tracked	*/
#define	S2WERR_NOT_TRACKED	5	/* protocol not tracked		*/
#define S2WERR_NETBUFF_ERROR	6	/* netbuff.lib returned error	*/
#define S2WERR_SRC_ADDRESS	7	/* source address problem	*/
#define S2WERR_DST_ADDRESS	8	/* destination address problem	*/
#define S2WERR_BAD_BROADCAST	9	/* broadcast address problem	*/
#define S2WERR_BAD_MULTICAST	10	/* multicast address problem	*/
#define S2WERR_ALIAS_LIST_FULL	11	/* station alias list full	*/
#define S2WERR_BAD_ALIAS	12	/* bad station alias		*/
#define S2WERR_MULTICAST_FULL	13	/* multicast address list full	*/
#define S2WERR_BAD_EVENT	14	/* unsupported event class      */
#define S2WERR_BAD_STATDATA	15	/* statdata failed sanity check */
#define S2WERR_PROTOCOL_UNKNOWN	16	/* unknown protocol type        */
#define S2WERR_IS_CONFIGURED	17	/* attempt to config twice      */
#define S2WERR_NULL_POINTER	18	/* null pointer detected        */


/*
 * defined events
 */

#define	S2EVENT_ERROR		0	/* error catch all		*/
#define	S2EVENT_TX		1	/* transmitter error catch all	*/
#define	S2EVENT_RX		2	/* receiver error catch all	*/
#define	S2EVENT_ONLINE		3	/* unit is in service		*/
#define	S2EVENT_OFFLINE		4	/* unit is not in service	*/
#define	S2EVENT_NETBUF		5	/* NetBuff error catch all	*/
#define S2EVENT_HARDWARE	6	/* hardware error catch all	*/
#define S2EVENT_SOFTWARE	7	/* software error catch all	*/


#endif	/* SANA2_SANA2DEVICE_H */
