	IFND	SANA2_SANA2DEVICE_I
SANA2_SANA2DEVICE_I	SET	1
**
**	$Id: sana2device.i,v 1.1 91/10/16 11:22:16 dlarson Exp $
**
**	Structure definitions for SANA-II devices.
**
**	(C) Copyright 1991 Raymond S. Brand
**		All Rights Reserved
**
**	(C) Copyright 1991 Commodore-Amiga Inc.
**		All Rights Reserved
**

**
** Contributions from:
**	Raymond S. Brand,   rsbx@cbmvax.commodore.com,  (215) 431-9100
**	Martin Hunt,      martin@cbmvax.commodore.com,  (215) 431-9100
**	Perry Kivolowitz,           ASDG Incorporated,  (608) 273-6585
**	Dale Luck,                    dale@boing.uucp,  (408) 262-1469
**

	IFND	EXEC_TYPES_I
	INCLUDE	"exec/types.i"
	ENDC	!EXEC_TYPES_I
	IFND	EXEC_PORTS_I
	INCLUDE	"exec/ports.i"
	ENDC	!EXEC_PORTS_I
	IFND	EXEC_IO_I
	INCLUDE	"exec/io.i"
	ENDC	!EXEC_IO_I
	IFND	DEVICES_TIMER_I
	INCLUDE	"devices/timer.i"
	ENDC	!DEVICES_TIMER_I
	IFND	LIBRARIES_NETBUFF_I
	INCLUDE "libraries/netbuff.i"
	ENDC	!LIBRARIES_NETBUFF_I


SANA2_MAX_ADDR_BITS	EQU	128
SANA2_MAX_ADDR_BYTES	EQU	((SANA2_MAX_ADDR_BITS+7)/8)


 STRUCTURE IOSana2Req,0
	STRUCT	S2IO_MESSAGE,MN_SIZE
	APTR	S2IO_DEVICE		; device node pointer
	APTR	S2IO_UNIT		; unit (driver private)

	UWORD	S2IO_COMMAND		; device command
	UBYTE	S2IO_FLAGS		; command flags
	BYTE	S2IO_ERROR		; generic error or warning
	ULONG	S2IO_WIREERROR		; wire type specific error

	APTR	S2IO_PACKETTYPE		; packet type
	STRUCT	S2IO_SRCADDR,SANA2_MAX_ADDR_BYTES	; source address
	STRUCT	S2IO_DSTADDR,SANA2_MAX_ADDR_BYTES	; dest address
	ULONG	S2IO_DATALENGTH		; from header

	STRUCT	S2IO_BODY,NB_SIZE	; packet data
	APTR	S2IO_STATDATA		; statics data pointer
	LABEL	S2IO_SIZE


;
; equates for the S2IO_FLAGS field
;

SANA2IOB_RAW	EQU	7		; raw packet IO requested
SANA2IOF_RAW	EQU	(1<<SANA2IOB_RAW)

SANA2IOB_BCAST	EQU	6		; broadcast packet (received)
SANA2IOF_BCAST	EQU	(1<<SANA2IOB_BCAST)

SANA2IOB_MCAST	EQU	5		; multicast packet (received)
SANA2IOF_MCAST	EQU	(1<<SANA2IOB_MCAST)

SANA2IOB_QUICK	EQU	IOB_QUICK	; quick IO requested (0)
SANA2IOF_QUICK	EQU	IOF_QUICK


;
; equates for OpenDevice()
;

SANA2OPB_MINE	EQU	0		; exclusive access requested
SANA2OPF_MINE	EQU	(1<<SANA2OPB_MINE)

SANA2OPB_PROM	EQU	1		; promiscuous mode requested
SANA2OPF_PROM	EQU	(1<<SANA2OPB_PROM)


 STRUCTURE SANA2PACKETTYPE,0
	ULONG	S2PT_CANONICALTYPE	; used by higher levels
	ULONG	S2PT_MAGIC		; interpretation code
	ULONG	S2PT_LENGTH		; length of match data
	APTR	S2PT_MATCH		; bytes to compare
	APTR	S2PT_MASK		; mask for comparison
	LABEL	S2PT_SIZE


 STRUCTURE SANA2DEVICEQUERY,0
	; Standard information
	ULONG	S2DQ_SIZEAVAILABLE	; bytes available
	ULONG	S2DQ_SIZESUPPLIED	; bytes supplied
	LONG	S2DQ_FORMAT		; this is type 0
	LONG	S2DQ_DEVICELEVEL	; this document is level 0

	; Common information
	UWORD	S2DQ_ADDRSIZE		; address size in bits
	ULONG	S2DQ_MTU		; maximum packet data size
	LONG	S2DQ_BPS		; line rate (bits/sec)
	LONG	S2DQ_HARDWARETYPE	; what the wire is

	; Format specific information
	LABEL	S2DQ_SIZE


;
; defined SANA-II hardware types
;

S2WIRETYPE_ETHERNET		EQU	1
S2WIRETYPE_ARCNET		EQU	2


 STRUCTURE SANA2PACKETTYPESTATS,0
	LONG	S2PTS_TXPACKETS		; transmitted count
	LONG	S2PTS_RXPACKETS		; received count
	LONG	S2PTS_TXBYTES		; bytes transmitted count
	LONG	S2PTS_RXBYTES		; bytes received count
	LONG	S2PTS_PACKETSDROPPED	; packets dropped count
	LABEL	S2PTS_SIZE


 STRUCTURE SANA2SPECIALSTATRECORD,0
	ULONG	S2SSR_TYPE		; statistic identifier
	LONG	S2SSR_COUNT		; the statistic
	APTR	S2SSR_STRING		; statistic name
	LABEL	S2SSR_SIZE


 STRUCTURE SANA2SPECIALSTATHEADER,0
	ULONG	S2SSH_RECORDCOUNTMAX		; room available
	ULONG	S2SSH_RECORDCOUNTSUPPLIED	; number supplied
	LABEL	S2SSH_SIZE


 STRUCTURE SANA2DEVICESTATS,0
	LONG	S2DS_RXPACKETS		; received count
	LONG	S2DS_TXPACKETS		; transmitted count
	LONG	S2DS_FRAMINGERRORS	; framming errors found
	LONG	S2DS_BADDATA		; bad packets received
	LONG	S2DS_HARDMISSES		; hardware miss count
	LONG	S2DS_SOFTMISSES		; software miss count
	LONG	S2DS_RXUNKNOWNTYPES	; orphan count
	LONG	S2DS_FIFOOVERRUNS	; hardware overruns
	LONG	S2DS_FIFOUNDERRUNS	; hardware underruns
	LONG	S2DS_RECONFIGURATIONS	; network reconfigurations
	STRUCT	S2DS_LASTSTART,TV_SIZE	; time of last online
	LABEL	S2DS_SIZE


;
; Device Commands
;

SANA2_CMD_START			EQU	(CMD_NONSTD)

SANA2CMD_DEVICEQUERY		EQU	(SANA2_CMD_START+0)
SANA2CMD_GETSTATIONADDRESS	EQU	(SANA2_CMD_START+1)
SANA2CMD_CONFIGINTERFACE	EQU	(SANA2_CMD_START+2)
SANA2CMD_ADDSTATIONALIAS	EQU	(SANA2_CMD_START+3)
SANA2CMD_DELSTATIONALIAS	EQU	(SANA2_CMD_START+4)
SANA2CMD_ADDMULTICASTADDRESS	EQU	(SANA2_CMD_START+5)
SANA2CMD_DELMULTICASTADDRESS	EQU	(SANA2_CMD_START+6)
SANA2CMD_MULTICAST		EQU	(SANA2_CMD_START+7)
SANA2CMD_BROADCAST		EQU	(SANA2_CMD_START+8)
SANA2CMD_TRACKTYPE		EQU	(SANA2_CMD_START+9)
SANA2CMD_UNTRACKTYPE		EQU	(SANA2_CMD_START+10)
SANA2CMD_GETTYPESTATS		EQU	(SANA2_CMD_START+11)
SANA2CMD_GETSPECIALSTATS	EQU	(SANA2_CMD_START+12)
SANA2CMD_GETGLOBALSTATS		EQU	(SANA2_CMD_START+13)
SANA2CMD_ONEVENT		EQU	(SANA2_CMD_START+14)
SANA2CMD_READORPHAN		EQU	(SANA2_CMD_START+15)
SANA2CMD_ONLINE			EQU	(SANA2_CMD_START+16)
SANA2CMD_OFFLINE		EQU	(SANA2_CMD_START+17)

SANA2_CMD_END			EQU	(SANA2_CMD_START+18)


;
; defined errors for S2IO_ERROR
;

S2ERR_NO_ERROR		EQU	0	; peachy-keen
S2ERR_NO_RESOURCES	EQU	1	; resource allocation failure
S2ERR_UNKNOWN_ENTITY	EQU	2	; unable to find something
S2ERR_BAD_ARGUMENT	EQU	3	; garbage somewhere
S2ERR_BAD_STATE		EQU	4	; inappropriate state
S2ERR_BAD_ADDRESS	EQU	5	; who?
S2ERR_MTU_EXCEEDED	EQU	6	; too much to chew
S2ERR_BAD_PROTOCOL	EQU	7	; bad packet type structure
S2ERR_NOT_SUPPORTED	EQU	8	; command not supported
S2ERR_SOFTWARE		EQU	9	; software error detected


;
; defined errors for S2IO_WIREERROR
;

S2WERR_GENERIC_ERROR	EQU	0	; no specific info available
S2WERR_NOT_CONFIGURED	EQU	1	; unit not configured
S2WERR_UNIT_ONLINE	EQU	2	; unit is currently online
S2WERR_UNIT_OFFLINE	EQU	3	; unit is currently offline
S2WERR_ALREADY_TRACKED	EQU	4	; protocol already tracked
S2WERR_NOT_TRACKED	EQU	5	; protocol not tracked
S2WERR_NETBUFF_ERROR	EQU	6	; netbuff.lib returned error
S2WERR_SRC_ADDRESS	EQU	7	; source address problem
S2WERR_DST_ADDRESS	EQU	8	; destination address problem
S2WERR_BAD_BROADCAST	EQU	9	; broadcast address problem
S2WERR_BAD_MULTICAST	EQU	10	; multicast address problem
S2WERR_ALIAS_LIST_FULL	EQU	11	; station alias list full
S2WERR_BAD_ALIAS	EQU	12	; bad station alias
S2WERR_MULTICAST_FULL	EQU	13	; multicast address list full
S2WERR_BAD_EVENT	EQU	14	; unsupported event class
S2WERR_BAD_STATDATA	EQU	15	; statdata failed sanity check
S2WERR_PROTOCOL_UNKNOWN	EQU	16	; unknown protocol type
S2WERR_IS_CONFIGURED	EQU	17	; attempt to config twice
S2WERR_NULL_POINTER	EQU	18	; null pointer detected


;
; defined events
;

S2EVENT_ERROR		EQU	0	; error catch all
S2EVENT_TX		EQU	1	; transmitter error catch all
S2EVENT_RX		EQU	2	; receiver error catch all
S2EVENT_ONLINE		EQU	3	; unit is in service
S2EVENT_OFFLINE		EQU	4	; unit is not in service
S2EVENT_NETBUF		EQU	5	; NetBuff error catch all
S2EVENT_HARDWARE	EQU	6	; hardware error catch all
S2EVENT_SOFTWARE	EQU	7	; software error catch all


	ENDC	SANA2_SANA2DEVICE_I
