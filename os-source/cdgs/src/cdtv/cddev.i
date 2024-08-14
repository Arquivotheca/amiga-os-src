        IFND    CDDEV_I
CDDEV_I SET     1
**
**      $Id: cddev.i,v 1.5 93/03/29 13:40:11 jerryh Exp Locker: jerryh $
**
**      Message structures used for device communication
**
**      (C) Copyright 1985,1986,1987,1988 Commodore-Amiga, Inc.
**          All Rights Reserved
**

    IFND CDTV_I
    INCLUDE "cdtv:include/cdtv/cdtv.i"
    ENDC


************************************************************************
***
***  cddev.i
***
***     CDTV Device Driver Constants and Macros
***
************************************************************************


        INCLUDE "include:exec/interrupts.i"
        INCLUDE "include:exec/lists.i"
        INCLUDE "include:exec/ports.i"
        INCLUDE "include:exec/libraries.i"
        INCLUDE "include:exec/tasks.i"


************************************************************************
***
***  Miscellaneous Definitions
***
************************************************************************

STACK_SIZE      equ     4096            ; task stack size
MAX_BYTES       equ     2048*1024       ; max bytes to transfer
SKIP_BYTES      equ     2048            ; size of skip buffer

BLOCK_SIZE      equ     2048            ; sector size
BLOCK_SHIFT     equ     11              ; bits used in bytes per sector

COLOR0          equ     $DFF180         ; for debugging
PERFORMIO       equ     -42             ; for calling ourself

ior             equr    a4              ; saved I/O request pointer
hb              equr    a5              ; hardware base pointer
db              equr    a6              ; device node pointer


************************************************************************
***
***  Command Options
***
************************************************************************

 BITDEF O,QUEUE,1               ; queue the command
 BITDEF O,DISKACCESS,2          ; This command accesses disk or modifies head position
 BITDEF O,ASYNC,3               ; Other commands may concurrently execute
 BITDEF O,NODISK,4              ; works when no disk in drive
 BITDEF O,NOREPLY,5             ; send no reply message
 BITDEF O,ROM,6                 ; CD-ROM required
 BITDEF O,INVALID,7             ; an invalid command

OF_ABT1 EQU     $0100           ; abort processing function number
OF_ABT2 EQU     $0200
OF_ABT3 EQU     $0300


************************************************************************
***
***  Device Driver Error Codes
***
************************************************************************




MSF_FLAG        equ     2       ; MSF requested flag
SQ_PADEND       equ     13      ; SubQ related

SQ_QCODE                equ     $00
SQ_QPAD                 equ     $00
SQ_QERROR               equ     $01
SQ_QDATA                equ     $02
SQ_Q_ADDRCTRL           equ     $02
SQ_Q_TNO                equ     $03
SQ_Q_POINT              equ     $04
SQ_Q_X                  equ     $04
SQ_Q_RTIME              equ     $05
SQ_Q_ATIME              equ     $09
SQ_Q_PTIME              equ     $09
SQ_QLEVEL               equ     $0C

************************************************************************
***
***  CDTV Device Driver Base Data Segment
***
************************************************************************

 STRUCTURE CDDB,LIB_SIZE
        STRUCT  db_Task,TC_SIZE
        STRUCT  db_StatIntr,IS_SIZE
        STRUCT  db_DiskPort,MP_SIZE     ; Port for disk access type commands

        ULONG   db_BlockSize            ; V35.4
        ULONG   db_ChgCount
        ULONG   db_FadeTable
        ULONG   db_FadeTarget
        ULONG   db_FadeLength
        ULONG   db_FadeStepSize
        ULONG   db_PlayIOR
        ULONG   db_CDBeginIO

        ULONG   db_FrameReq
        ULONG   db_FramePort
        ULONG   db_PortFrm
        ULONG   db_IORFrm
        ULONG   db_CallAddress
        ULONG   db_FrameCount

        APTR    db_PortD
        APTR    db_IORD

        APTR    db_InitTask                     ; Initialization task pointer

        UWORD   db_Openned

        UWORD   db_Attenuation
        UWORD   db_StatTimer            ; timeout for status commands
        UWORD   db_StatAbort            ; status timeout abort counter (stats)
        UWORD   db_Class2Opts           ; active class-2 options

        UBYTE   db_ReadAbortPlay        ; AbortPlay flag (Read automatically aborts play)
        UBYTE   db_ROMTrack             ; CES V34.3
        UBYTE   db_Class1Err            ; Class 1 command error (or initial Class 2 error)
        UBYTE   db_Class2Err            ; Class 2 command error
        UBYTE   db_DiskState            ; current insertion status
        UBYTE   db_CDStatus
        UBYTE   db_StatLen              ; bytes to read for status
        UBYTE   db_GenStat              ; TRUE for general status event
        UBYTE   db_AckWidth
        UBYTE   db_SegType
        UBYTE   db_SQStat               ; subq status byte
        UBYTE   db_AudioStat            ; audio bit of status
        UBYTE   db_ReadMode             ; normal/double speed modes b0=read b1=readxl
        UBYTE   db_RetryCount           ; read retry counter
        UBYTE   db_NOSTAT               ; SQStatus contains no usefull information
        UBYTE   db_abortedflags

        STRUCT  db_XferList,MLH_SIZE+XL_SIZEOF*3
        STRUCT  db_ChgList,LH_SIZE
        STRUCT  db_FrameList,LH_SIZE
        STRUCT  db_FrameIntStruct,IS_SIZE
        STRUCT  db_preQCode,14
        STRUCT  db_QCode,14
        STRUCT  db_PrintBuf,64

        STRUCT  db_TOC,101*8

        ALIGNLONG

        STRUCT  db_Buff,2336

        LABEL   db_SizeOf


 STRUCTURE CDUNIT,0

        APTR    UNIT_PortA                        ; Reply port A 
        APTR    UNIT_PortChg                      ; Reply port Chg
        APTR    UNIT_IORA                         ; I/O request A 
        APTR    UNIT_IORChg                       ; I/O request Chg

        LABEL   UNIT_SizeOF


************************************************************************
***
***  Miscellaneous Macro Definitions
***
************************************************************************

FIRSTNODE       MACRO
                tst.l   4(\1)           ; is it a list header?
                bne.s   0$              ; no
                move.l  (\1),\1         ; fetch first entry
0$
                ENDM


************************************************************************
***
***  Task Internal Signal Related Definitions
***
************************************************************************

 BITDEF SIG,DISKPORT,20

SIGNAL          MACRO
                move.l  #\1,d0
                lea     db_Task(db),a1
                exec    Signal
                ENDM


************************************************************************
***
***  Old CDTV Hardware definitions needed for compatibility
***
************************************************************************

CTLADR_PREEMPH          equ     4
CTLADR_COPY             equ     5
CTLADR_DATA             equ     6
CTLADR_4CHAN            equ     7


************************************************************************
***
***  Status Command Bit Definitions
***
************************************************************************

CDSF_READY      equ     $01
CDSF_AUDIO      equ     $04
CDSF_DONE       equ     $08
CDSF_ERROR      equ     $10
CDSF_SPINUP     equ     $20
CDSF_DISKIN     equ     $40
CDSF_INFERR     equ     $80

CDSB_READY      equ     0
CDSB_AUDIO      equ     2
CDSB_DONE       equ     3
CDSB_ERROR      equ     4
CDSB_SPINUP     equ     5
CDSB_DISKIN     equ     6
CDSB_INFERR     equ     7

PRINT           MACRO

                save    a0
                lea     \1(pc),a0
                jsr     PutStr
                bra     \2
\1              dc.b    \3,10,0
                ds.w    0
\2             
                restore a0
                ENDM

                ENDC
