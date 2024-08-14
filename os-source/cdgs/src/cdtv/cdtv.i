        IFND    CDTV_I
CDTV_I  SET     1
*
* CDTV Device Driver Include File
*
*       Copyright (c) 1991 Commodore Electronics Ltd.
*       All rights reserved. Confidential and Proprietary.
*       CDTV is a trademark of Commodore Electronics Ltd.
*
*       Written by: Jerry Horanoff
*
*

*
* CDTV Device Name
*
*       Name passed to OpenDevice to identify the CDTV device.
*

DBSTRING        MACRO
                dc.b    \1,0
                ENDM

CDTV_NAME:      MACRO
                DC.B    'cdtv.device',0
                DS.W    0
                ENDM

*
* CDTV Device Driver Commands
*

CDTV_RESET              equ      1
CDTV_READ               equ      2
CDTV_WRITE              equ      3
CDTV_UPDATE             equ      4
CDTV_CLEAR              equ      5
CDTV_STOP               equ      6
CDTV_START              equ      7
CDTV_FLUSH              equ      8
CDTV_MOTOR              equ      9
CDTV_SEEK               equ     10
CDTV_FORMAT             equ     11
CDTV_REMOVE             equ     12
CDTV_CHANGENUM          equ     13
CDTV_CHANGESTATE        equ     14
CDTV_PROTSTATUS         equ     15

CDTV_GETDRIVETYPE       equ     18
CDTV_GETNUMTRACKS       equ     19
CDTV_ADDCHANGEINT       equ     20
CDTV_REMCHANGEINT       equ     21
CDTV_GETGEOMETRY        equ     22
CDTV_EJECT              equ     23

CDTV_DIRECT             equ     32
CDTV_STATUS             equ     33
CDTV_QUICKSTATUS        equ     34
CDTV_INFO               equ     35
CDTV_ERRORINFO          equ     36
CDTV_ISROM              equ     37
CDTV_OPTIONS            equ     38
CDTV_FRONTPANEL         equ     39
CDTV_FRAMECALL          equ     40
CDTV_FRAMECOUNT         equ     41
CDTV_READXL             equ     42
CDTV_PLAYTRACK          equ     43
CDTV_PLAYLSN            equ     44
CDTV_PLAYMSF            equ     45
CDTV_PLAYSEGSLSN        equ     46
CDTV_PLAYSEGSMSF        equ     47
CDTV_TOCLSN             equ     48
CDTV_TOCMSF             equ     49
CDTV_SUBQLSN            equ     50
CDTV_SUBQMSF            equ     51
CDTV_PAUSE              equ     52
CDTV_STOPPLAY           equ     53
CDTV_POKESEGLSN         equ     54
CDTV_POKESEGMSF         equ     55
CDTV_MUTE               equ     56
CDTV_FADE               equ     57
CDTV_POKEPLAYLSN        equ     58
CDTV_POKEPLAYMSF        equ     59
CDTV_GENLOCK            equ     60
CDTV_PLAYMODE           equ     61
CDTV_READMODE           equ     62

*
* CDTV Errors
*
CDERR_BADTOC            equ     -8      ; Unable to recover TOC
CDERR_DMAFAILED         equ     -9      ; Read DMA failed
CDERR_NOROM             equ     -10     ; No CD-ROM track present

*
* LSN/MSF Structures
*

 STRUCTURE  CDPOS,0
    ULONG   CDPOS_RAW
    ULONG   CDPOS_LSN
    STRUCT  CDPOS_MSF,RMSF_SIZE
    LABEL   CDPOS_SIZE

*
* CDTV Transfer Lists
*
*       To create transfer lists, use an Exec List or MinList structure
*       and AddHead/AddTail nodes of the transfer structure below.
*       Don't forget to initialize the List/MinList before using it!
*
*
 STRUCTURE XL,0
        APTR    XL_SUCC
        APTR    XL_PRED
        APTR    XL_BUFFER
        ULONG   XL_LENGTH
        APTR    XL_DONECODE
        ULONG   XL_ACTUAL
        LABEL   XL_SIZEOF

*
* CDTV Audio Segment
*
*       To create segment lists, use an Exec List or MinList structure
*       and AddHead/AddTail nodes with the segment structure below.
*       Don't forget to initialize the List/MinList before using it!
*
************************************************************************
***
***  Audio Segment Structure Definition
***
************************************************************************

 STRUCTURE AS,0
        APTR    AS_SUCC
        APTR    AS_PRED
        ULONG   AS_START
        ULONG   AS_STOP
        APTR    AS_STARTFUNC
        APTR    AS_STOPFUNC
        LABEL   AS_SIZEOF

*
* CDTV Table of Contents
*
*       The CDTV_TOCLSN and CDTV_TOCMSF comands return an array
*       of this structure.
*
*       Notes:
*
*           The first entry (zero) contains special disk info:
*           the Track field indicates the first track number; the
*           LastTrack field contains the last track number, which is
*           only valid for this first entry; and the Position field
*           indicates the last position (lead out area start) on
*           the disk in MSF format.
*
 STRUCTURE TOC,0
        UBYTE   TOC_RESVD1
        UBYTE   TOC_ADDRCTRL
        UBYTE   TOC_TRACK
        UBYTE   TOC_LASTTRACK
        ULONG   TOC_POSITION
        LABEL   TOC_SIZEOF

 STRUCTURE DI,0                 ; TOC Disk Info
        UBYTE   DI_FIRSTTRACK
        UBYTE   DI_LASTTRACK
        UBYTE   DI_ENDMSF
        LABEL   DI_SIZE

*
* AddrCtrl Values
*
*       These values are returned from both TOC and SUBQ commands.
*       Lower 4 bits are boolean flags. Upper 4 bits are numerical.
*
ADRCTLB_PREEMPH         equ     0       ; audio pre-emphasis
ADRCTLB_COPY            equ     1       ; digital copy ok
ADRCTLB_DATA            equ     2       ; data track
ADRCTLB_4CHAN           equ     3       ; 4 channel audio

ADRCTLF_PREEMPH         equ     1
ADRCTLF_COPY            equ     2
ADRCTLF_DATA            equ     4
ADRCTLF_4CHAN           equ     8

ADRCTL_NOMODE           equ     $00     ; no mode info
ADRCTL_POSITION         equ     $10     ; position encoded
ADRCTL_MEDIACAT         equ     $20     ; media catalog number
ADRCTL_ISRC             equ     $30     ; ISRC encoded
ADRCTL_MASK             equ     $F0

*
* SubQ Status Values
*
SQSTAT_NOTVALID         equ     $00     ; audio status not valid
SQSTAT_PLAYING          equ     $11     ; play operation in progress
SQSTAT_PAUSED           equ     $12     ; play is paused
SQSTAT_DONE             equ     $13     ; play completed ok
SQSTAT_ERROR            equ     $14     ; play stopped from error
SQSTAT_NOSTAT           equ     $15     ; no status info

*
* UPC/ISRC Bit Flags
*
SQUPCB_ISRC             equ     0       ; Set for ISRC. Clear for UPC
SQUPCB_VALID            equ     7       ; Media catalog detected

SQUPCF_ISRC             equ     $01
SQUPCF_VALID            equ     $80

*
*  Quick-Status Bits
*
*       Bits returned in IO_ACTUAL of the CDTV_QUICKSTATUS.
*
QSB_READY               equ     0
QSB_AUDIO               equ     2
QSB_DONE                equ     3
QSB_ERROR               equ     4
QSB_SPIN                equ     5
QSB_DISK                equ     6
QSB_INFERR              equ     7

QSF_READY               equ     $01
QSF_AUDIO               equ     $04
QSF_DONE                equ     $08
QSF_ERROR               equ     $10
QSF_SPIN                equ     $20
QSF_DISK                equ     $40
QSF_INFERR              equ     $80

*
* CDTV_GENLOCK values (io_Offset)
*
CDTV_GENLOCK_REMOTE     equ     0       ; Remote control
CDTV_GENLOCK_AMIGA      equ     1       ; Amiga video out
CDTV_GENLOCK_EXTERNAL   equ     2       ; External video out
CDTV_GENLOCK_MIXED      equ     3       ; Amiga over external video

*
* CDTV_INFO Command values (io_Offset)
*
CDTV_INFO_BLOCK_SIZE    equ     2       ; CD-ROM block size
CDTV_INFO_FRAME_RATE    equ     3       ; CD-ROM frame rate

*
* CDTV_FRONTPANEL Command values (io_Length)
*
CDTV_PANEL_DISABLED             equ     0       ; Disable front panel control of CD (here for compatibliity)

CDTV_PANEL_ENABLED              equ     1       ; Enable internal player control of CD
CDTV_PANEL_NUMERIC              equ     2       ; Enable internal player's use of numeric keypad
CDTV_PANEL_NOAMIGA              equ     4       ; Do not send controller keypresses to Amiga
CDTV_PANEL_PLAY_EJECT           equ     8       ; STOP/EJECT means EJECT even when playing CD audio

*
* CDTV_PLAYMODE Command values (io_Length)
*
CDTV_PLAYMODE_NORMAL            equ     0       ; Play at normal speed
CDTV_PLAYMODE_FF                equ     1       ; Play in fast forward mode
CDTV_PLAYMODE_REW               equ     2       ; Play in fast reverse mode
CDTV_PLAYMODE_DOUBLE            equ     3       ; Play in double-speed mode

CDTV_PLAYMODE_NOACTION          equ     -1      ; Return status in IO_Actual only

*
* CDTV_PLAYMODE Return values (io_Actual)
*
*      The following values are ORed with the previous PLAYMODE values
*      and returned in IO_ACTUAL.  For example, a return value of
*      0x00000011 means the drive is PLAYING in FF mode.
*
CDTV_PLAYMODE_NOTPLAYING        equ     $00     ; No play is in progress
CDTV_PLAYMODE_PLAYING           equ     $10     ; Play is in progress
CDTV_PLAYMODE_PAUSED            equ     $20     ; Paused within a play

*
* CDTV_READMODE Command values (io_Length)
*
CDTV_READMODE_RDDS              equ     1       ; Read CD-ROM data at double speed
CDTV_READMODE_XLDS              equ     2       ; ReadXL CD-ROM data at double speed

*
* CDTV_OPTIONS Command values (io_Offset)
*
CDTV_OPTIONS_BLOCK_SIZE         equ     1       ; CD-ROM block size
CDTV_OPTIONS_ERROR_TYPE         equ     2       ; Error recovery type


*
* Other CDTV Constants
*
READ_PAD_BYTES                  equ     8       ; # bytes at end of read

        ENDC    ; CDTV_I



