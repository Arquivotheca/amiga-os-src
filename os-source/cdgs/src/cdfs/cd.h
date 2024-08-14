#ifndef CDTV_H
#define CDTV_H
/*
**  CD Device Driver Include File
**
**  Copyright 1990 Commodore-Amiga, Inc
**  Currently Confidential and Proprietary
**  FOR REGISTERED CDTV DEVELOPERS ONLY!
**
**  Driver Version: Beta 2
**  By Sassenrath Research, Ukiah, CA
**  Phone: 707-462-4878 FAX: 462-4879
*/

/*
** CD Device Name
**
**  Name passed to OpenDevice to identify the CD device.
*/
#define CD_NAME "cd.device"

/*
** CD Device Driver Commands
*/
#define CD_RESET         1
#define CD_READ          2
#define CD_WRITE         3
#define CD_UPDATE        4
#define CD_CLEAR         5
#define CD_STOP          6
#define CD_START         7
#define CD_FLUSH         8
#define CD_MOTOR         9
#define CD_SEEK         10
#define CD_FORMAT       11
#define CD_REMOVE       12
#define CD_CHANGENUM        13
#define CD_CHANGESTATE      14
#define CD_PROTSTATUS       15

#define CD_GETDRIVETYPE     18
#define CD_GETNUMTRACKS     19
#define CD_ADDCHANGEINT     20
#define CD_REMCHANGEINT     21
#define CD_GETGEOMETRY      22
#define CD_EJECT        23

#define CD_DIRECT       32
#define CD_STATUS       33
#define CD_QUICKSTATUS      34
#define CD_INFO         35
#define CD_ERRORINFO        36
#define CD_ISROM        37
#define CD_OPTIONS      38
#define CD_FRONTPANEL       39
#define CD_FRAMECALL        40
#define CD_FRAMECOUNT       41
#define CD_READXL       42
#define CD_PLAYTRACK        43
#define CD_PLAYLSN      44
#define CD_PLAYMSF      45
#define CD_PLAYSEGSLSN      46
#define CD_PLAYSEGSMSF      47
#define CD_TOCLSN       48
#define CD_TOCMSF       49
#define CD_SUBQLSN      50
#define CD_SUBQMSF      51
#define CD_PAUSE        52
#define CD_STOPPLAY     53
#define CD_POKESEGLSN       54
#define CD_POKESEGMSF       55
#define CD_MUTE         56
#define CD_FADE         57
#define CD_POKEPLAYLSN      58
#define CD_POKEPLAYMSF      59

/*
** CD Errors
*/
#define CDERR_OPENFAIL  -1  /* OpenDevice() failed      */
#define CDERR_ABORTED   -2  /* Command has been aborted */
#define CDERR_NOTVALID  -3  /* IO request not valid     */
#define CDERR_NOCMD -4  /* No such command      */
#define CDERR_BADARG    -5  /* Bad command argument     */
#define CDERR_NODISK    -6  /* No disk is present       */
#define CDERR_WRITEPROT -7  /* Disk is write protected  */
#define CDERR_BADTOC    -8  /* Unable to recover TOC    */
#define CDERR_DMAFAILED -9  /* Read DMA failed      */
#define CDERR_NOROM -10 /* No CD-ROM track present  */

/*
** CD Transfer Lists
**
**  To create transfer lists, use an Exec List or MinList structure
**  and AddHead/AddTail nodes of the transfer structure below.
**  Don't forget to initialize the List/MinList before using it!
**
*/
struct  CDXL
{
    struct  MinNode Node;   /* double linkage   */
    char    *Buffer;    /* data (word aligned)  */
    long    Length;     /* must be even # bytes */
    void    (*DoneFunc)();  /* called when done */
    long    Actual;     /* bytes transferred    */
};

/*
** CD Audio Segment
**
**  To create segment lists, use an Exec List or MinList structure
**  and AddHead/AddTail nodes with the segment structure below.
**  Don't forget to initialize the List/MinList before using it!
*/
struct  CDAudioSeg
{
    struct  MinNode Node;   /* double linkage   */
    long    Start;      /* starting position    */
    long    Stop;       /* stopping position    */
    void    (*StartFunc)(); /* function to call on start */
    void    (*StopFunc)();  /* function to call on stop  */
};

/*
** CD Table of Contents
**
**  The CD_TOCLSN and CD_TOCMSF comands return an array
**  of this structure.
**
**  Notes:
**
**      The first entry (zero) contains special disk info:
**      the Track field indicates the first track number; the
**      LastTrack field contains the last track number, which is
**      only valid for this first entry; and the Position field
**      indicates the last position (lead out area start) on
**      the disk in MSF format.
*/
struct CDTOC
{
    UBYTE   rsvd;       /* not used */
    UBYTE   AddrCtrl;   /* SubQ info    */
    UBYTE   Track;      /* Track number */
    UBYTE   LastTrack;  /* Only for entry zero. See note above. */
    ULONG   Position;
};

/*
** TOC AddrCtrl Values
**
**  These values are returned from both TOC and SUBQ commands.
**  Lower 4 bits are boolean flags. Upper 4 bits are numerical.
*/
#define TOCCTLB_PREEMPH 0   /* audio pre-emphasis   */
#define TOCCTLB_COPY    1   /* digital copy ok  */
#define TOCCTLB_DATA    2   /* data track       */
#define TOCCTLB_4CHAN   3   /* 4 channel audio  */

#define TOCADR_NOMODE   0x00    /* no mode info     */
#define TOCADR_POSITION 0x10    /* position encoded */
#define TOCADR_MEDIACAT 0x20    /* media catalog number */
#define TOCADR_ISRC 0x30    /* ISRC encoded     */

/*
** CD Audio SubQ
*/
struct CDSubQ
{
    UBYTE   Status;     /* Audio status */
    UBYTE   AddrCtrl;   /* SubQ info    */
    UBYTE   Track;      /* Track number */
    UBYTE   Index;      /* Index number */
    ULONG   DiskPosition;   /* Position from start of disk  */
    ULONG   TrackPosition;  /* Position from start of track */
    UBYTE   ValidUPC;   /* Flag for product identifer   */
    UBYTE   pad[3];     /* undefined    */
};

/*
** SubQ Status Values
*/
#define SQSTAT_NOTVALID 0x00    /* audio status not valid   */
#define SQSTAT_PLAYING  0x11    /* play operation in progress   */
#define SQSTAT_PAUSED   0x12    /* play is paused       */
#define SQSTAT_DONE 0x13    /* play completed ok        */
#define SQSTAT_ERROR    0x14    /* play stopped from error  */
#define SQSTAT_NOSTAT   0x15    /* no status info       */

/*
** UPC/ISRC Bit Flags
*/
#define SQUPCB_ISRC 0   /* Set for ISRC. Clear for UPC  */
#define SQUPCB_VALID    7   /* Media catalog detected   */

#endif /* CDTV_H */
