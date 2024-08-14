#ifndef CDTV_H
#define CDTV_H
/*
** CDTV Device Driver Include File
**
**	Copyright (c) 1991 Commodore Electronics Ltd.
**	All rights reserved. Confidential and Proprietary.
**	CDTV is a trademark of Commodore Electronics Ltd.
**
**	Written by: Carl Sassenrath
**	            Sassenrath Research, Ukiah, CA
**
**	Version of: March 8, 1991 - Sassenrath
**	            Revised to use new CDTV_ prefix.
**	            Old CD_ prefix still works however.
**
**	Version of: March 11, 1991 - Sassenrath
**	            Various control values added.
**
**	Version of: March 15, 1991 - Szucs
**	            READ_PAD_BYTES added.
**
*/

#ifndef	EXEC_NODES_H
#include <exec/nodes.h>
#endif

/*
** CDTV Device Name
**
**	Name passed to OpenDevice to identify the CDTV device.
*/
#define	CDTV_NAME "cdtv.device"

/*
** CDTV Device Driver Commands
*/
#define	CDTV_RESET		 1
#define	CDTV_READ		 2
#define	CDTV_WRITE		 3
#define	CDTV_UPDATE		 4
#define	CDTV_CLEAR		 5
#define	CDTV_STOP		 6
#define	CDTV_START		 7
#define	CDTV_FLUSH		 8
#define	CDTV_MOTOR		 9
#define	CDTV_SEEK		10
#define	CDTV_FORMAT		11
#define	CDTV_REMOVE		12
#define	CDTV_CHANGENUM		13
#define	CDTV_CHANGESTATE	14
#define	CDTV_PROTSTATUS		15

#define	CDTV_GETDRIVETYPE	18
#define	CDTV_GETNUMTRACKS	19
#define	CDTV_ADDCHANGEINT	20
#define	CDTV_REMCHANGEINT	21
#define	CDTV_GETGEOMETRY	22
#define	CDTV_EJECT		23

#define	CDTV_DIRECT		32
#define	CDTV_STATUS		33
#define	CDTV_QUICKSTATUS	34
#define	CDTV_INFO		35
#define	CDTV_ERRORINFO		36
#define	CDTV_ISROM		37
#define	CDTV_OPTIONS		38
#define	CDTV_FRONTPANEL		39
#define	CDTV_FRAMECALL		40
#define	CDTV_FRAMECOUNT		41
#define	CDTV_READXL		42
#define	CDTV_PLAYTRACK		43
#define	CDTV_PLAYLSN		44
#define	CDTV_PLAYMSF		45
#define	CDTV_PLAYSEGSLSN	46
#define	CDTV_PLAYSEGSMSF	47
#define	CDTV_TOCLSN		48
#define	CDTV_TOCMSF		49
#define	CDTV_SUBQLSN		50
#define	CDTV_SUBQMSF		51
#define	CDTV_PAUSE		52
#define	CDTV_STOPPLAY		53
#define	CDTV_POKESEGLSN		54
#define	CDTV_POKESEGMSF		55
#define	CDTV_MUTE		56
#define	CDTV_FADE		57
#define	CDTV_POKEPLAYLSN	58
#define	CDTV_POKEPLAYMSF	59
#define	CDTV_GENLOCK		60

/*
** CDTV Errors
*/
#define	CDERR_OPENFAIL	(-1)	/* OpenDevice() failed		*/
#define	CDERR_ABORTED	(-2)	/* Command has been aborted	*/
#define	CDERR_NOTVALID	(-3)	/* IO request not valid		*/
#define	CDERR_NOCMD	(-4)	/* No such command		*/
#define	CDERR_BADARG	(-5)	/* Bad command argument		*/
#define	CDERR_NODISK	(-6)	/* No disk is present		*/
#define	CDERR_WRITEPROT (-7)	/* Disk is write protected	*/
#define	CDERR_BADTOC	(-8)	/* Unable to recover TOC	*/
#define	CDERR_DMAFAILED	(-9)	/* Read DMA failed		*/
#define	CDERR_NOROM	(-10)	/* No CD-ROM track present	*/

/*
** LSN/MSF Structures and macros
*/
struct RMSF
{
	UBYTE	Reserved;
	UBYTE	Minute;
	UBYTE	Second;
	UBYTE	Frame;
};

union LSNMSF
{
	ULONG	Raw;	
	ULONG	LSN;		/* Logical Sector Number	*/	
	struct	RMSF MSF;	/* Minute, Second, Frame	*/
};

typedef union LSNMSF CDPOS;
#define	TOMSF(m,s,f) (((ULONG)(m)<<16)+((s)<<8)+(f))

/*
** CDTV Transfer Lists
**
**	To create transfer lists, use an Exec List or MinList structure
**	and AddHead/AddTail nodes of the transfer structure below.
**	Don't forget to initialize the List/MinList before using it!
**
*/
struct	CDXL
{
	struct	MinNode Node;	/* double linkage	*/
	char	*Buffer;	/* data (word aligned)	*/
	LONG	Length;		/* must be even # bytes	*/
	void	(*DoneFunc)();	/* called when done	*/
	LONG	Actual;		/* bytes transferred	*/
};

/*
** CDTV Audio Segment
**
**	To create segment lists, use an Exec List or MinList structure
**	and AddHead/AddTail nodes with the segment structure below.
**	Don't forget to initialize the List/MinList before using it!
*/
struct	CDAudioSeg
{
	struct	MinNode Node;	/* double linkage	*/
	CDPOS	Start;		/* starting position	*/
	CDPOS	Stop;		/* stopping position	*/
	void	(*StartFunc)();	/* function to call on start */
	void	(*StopFunc)();	/* function to call on stop  */
};

/*
** CDTV Table of Contents
**
**	The CDTV_TOCLSN and CDTV_TOCMSF comands return an array
**	of this structure.
**
**	Notes:
**
**	    The first entry (zero) contains special disk info:
**	    the Track field indicates the first track number; the
**	    LastTrack field contains the last track number, which is
**	    only valid for this first entry; and the Position field
**	    indicates the last position (lead out area start) on
**	    the disk in MSF format.
*/
struct CDTOC
{
	UBYTE	rsvd;		/* not used	*/
	UBYTE	AddrCtrl;	/* SubQ info	*/
	UBYTE	Track;		/* Track number	*/
	UBYTE	LastTrack;	/* Only for entry zero. See note above. */
	CDPOS	Position;
};

/*
** AddrCtrl Values
**
**	These values are returned from both TOC and SUBQ commands.
**	Lower 4 bits are boolean flags. Upper 4 bits are numerical.
*/
#define	ADRCTLB_PREEMPH	0	/* audio pre-emphasis	*/
#define	ADRCTLB_COPY	1	/* digital copy ok	*/
#define	ADRCTLB_DATA	2	/* data track		*/
#define	ADRCTLB_4CHAN	3	/* 4 channel audio	*/

#define	ADRCTLF_PREEMPH	1
#define	ADRCTLF_COPY	2
#define	ADRCTLF_DATA	4
#define	ADRCTLF_4CHAN	8

#define	ADRCTL_NOMODE	0x00	/* no mode info		*/
#define	ADRCTL_POSITION	0x10	/* position encoded	*/
#define	ADRCTL_MEDIACAT	0x20	/* media catalog number	*/
#define	ADRCTL_ISRC	0x30	/* ISRC encoded		*/
#define	ADRCTL_MASK	0xF0

/*
** CDTV Audio SubQ
*/
struct CDSubQ
{
	UBYTE	Status;		/* Audio status */
	UBYTE	AddrCtrl;	/* SubQ info	*/
	UBYTE	Track;		/* Track number	*/
	UBYTE	Index;		/* Index number	*/
	CDPOS	DiskPosition;	/* Position from start of disk  */
	CDPOS	TrackPosition;	/* Position from start of track */
	UBYTE	ValidUPC;	/* Flag for product identifer	*/
	UBYTE	pad[3];		/* undefined	*/
};

/*
** SubQ Status Values
*/
#define	SQSTAT_NOTVALID	0x00	/* audio status not valid	*/
#define	SQSTAT_PLAYING	0x11	/* play operation in progress	*/
#define	SQSTAT_PAUSED	0x12	/* play is paused		*/
#define	SQSTAT_DONE	0x13	/* play completed ok		*/
#define	SQSTAT_ERROR	0x14	/* play stopped from error	*/
#define	SQSTAT_NOSTAT	0x15	/* no status info		*/

/*
** UPC/ISRC Bit Flags
*/
#define	SQUPCB_ISRC	0	/* Set for ISRC. Clear for UPC	*/
#define	SQUPCB_VALID	7	/* Media catalog detected	*/

#define	SQUPCF_ISRC	0x01
#define	SQUPCF_VALID	0x80

/*
**  Quick-Status Bits
**
**	Bits returned in IO_ACTUAL of the CDTV_QUICKSTATUS.
*/
#define	QSB_READY	0
#define	QSB_AUDIO	2
#define	QSB_DONE	3
#define	QSB_ERROR	4
#define	QSB_SPIN	5
#define	QSB_DISK	6
#define	QSB_INFERR	7

#define	QSF_READY	0x01
#define	QSF_AUDIO	0x04
#define	QSF_DONE	0x08
#define	QSF_ERROR	0x10
#define	QSF_SPIN	0x20
#define	QSF_DISK	0x40
#define	QSF_INFERR	0x80

/*
** CDTV_GENLOCK values (io_Offset)
*/
#define CDTV_GENLOCK_REMOTE	0	/* Remote control	*/
#define CDTV_GENLOCK_AMIGA	1	/* Amiga video out	*/
#define CDTV_GENLOCK_EXTERNAL	2	/* External video out	*/
#define CDTV_GENLOCK_MIXED	3	/* Amiga over external video */

/*
** CDTV_INFO Command values (io_Offset)
*/
#define CDTV_INFO_DMAC		0	/* private		*/
#define CDTV_INFO_TRIPORT	1	/* private		*/
#define CDTV_INFO_BLOCK_SIZE	2	/* CD-ROM block size	*/
#define CDTV_INFO_FRAME_RATE	3	/* CD-ROM frame rate	*/

/*
** CDTV_OPTIONS Command values (io_Offset)
*/
#define CDTV_OPTIONS_BLOCK_SIZE 1	/* CD-ROM block size	*/
#define CDTV_OPTIONS_ERROR_TYPE	2	/* Error recovery type	*/


/*
** Other CDTV Constants
*/
#define	READ_PAD_BYTES		8	/* # bytes at end of read */

/*
** Old CDTV Definitions
**
**	Obsolete - Do not use these.
*/
#define	CD_NAME	"cdtv.device"
#define	CD_RESET		 1
#define	CD_READ			 2
#define	CD_WRITE		 3
#define	CD_UPDATE		 4
#define	CD_CLEAR		 5
#define	CD_STOP			 6
#define	CD_START		 7
#define	CD_FLUSH		 8
#define	CD_MOTOR		 9
#define	CD_SEEK			10
#define	CD_FORMAT		11
#define	CD_REMOVE		12
#define	CD_CHANGENUM		13
#define	CD_CHANGESTATE		14
#define	CD_PROTSTATUS		15

#define	CD_GETDRIVETYPE		18
#define	CD_GETNUMTRACKS		19
#define	CD_ADDCHANGEINT		20
#define	CD_REMCHANGEINT		21
#define	CD_GETGEOMETRY		22
#define	CD_EJECT		23

#define	CD_DIRECT		32
#define	CD_STATUS		33
#define	CD_QUICKSTATUS		34
#define	CD_INFO			35
#define	CD_ERRORINFO		36
#define	CD_ISROM		37
#define	CD_OPTIONS		38
#define	CD_FRONTPANEL		39
#define	CD_FRAMECALL		40
#define	CD_FRAMECOUNT		41
#define	CD_READXL		42
#define	CD_PLAYTRACK		43
#define	CD_PLAYLSN		44
#define	CD_PLAYMSF		45
#define	CD_PLAYSEGSLSN		46
#define	CD_PLAYSEGSMSF		47
#define	CD_TOCLSN		48
#define	CD_TOCMSF		49
#define	CD_SUBQLSN		50
#define	CD_SUBQMSF		51
#define	CD_PAUSE		52
#define	CD_STOPPLAY		53
#define	CD_POKESEGLSN		54
#define	CD_POKESEGMSF		55
#define	CD_MUTE			56
#define	CD_FADE			57
#define	CD_POKEPLAYLSN		58
#define	CD_POKEPLAYMSF		59
#define	CD_GENLOCK		60

#endif	/* CDTV_H */
