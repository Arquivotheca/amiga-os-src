************************************************************************
***                                                                  ***
***                    -= CDTV DEVICE DRIVER =-                      ***
***                                                                  ***
************************************************************************
***                                                                  ***
***     CONFIDENTIAL and PROPRIETARY                                 ***
***     Copyright (c) 1990 by Commodore-Amiga, Inc.                  ***
***     Created by Carl Sassenrath, Sassenrath Research, Ukiah, CA   ***
***                                                                  ***
***     Modified 6/1/91: Jerry Horanoff                              ***
***                                                                  ***
************************************************************************

        INCLUDE "include:exec/types.i"
        INCLUDE "include:exec/nodes.i"
        INCLUDE "include:exec/lists.i"
        INCLUDE "include:exec/ports.i"
        INCLUDE "include:exec/libraries.i"
        INCLUDE "include:exec/tasks.i"
        INCLUDE "include:exec/devices.i"
        INCLUDE "include:exec/memory.i"
        INCLUDE "include:exec/interrupts.i"
        INCLUDE "include:exec/resident.i"
        INCLUDE "include:exec/io.i"
        INCLUDE "include:exec/errors.i"
        INCLUDE "include:exec/execbase.i"
        INCLUDE "include:hardware/intbits.i"
        INCLUDE "include:devices/trackdisk.i"
        INCLUDE "include:devices/timer.i"
        INCLUDE "include:exec/ables.i"
        INCLUDE "gs:cd/cd.i"
        INCLUDE "cdtv.i"
        INCLUDE "cdtv:src/playerlibrary/playerlibrary.i"
        INCLUDE "defs.i"
        INCLUDE "cddev.i"

        OPT     p=68020

************************************************************************
***
***  External References
***
************************************************************************

        XREF    GenStatus
        XREF    GetROMTrack        
        XREF    GetClass1Error
        XREF    GetClass2Error
        XREF    MSFtoLSN
        XREF    LSNtoMSF
        XREF    MSFtoLSNPOS
        XREF    LSNtoMSFPOS
        XREF    BCDtoBIN
        XREF    Attenuate
        XREF    Open
        XREF    Close
        XREF    Expunge
        XREF    Freserved
        XREF    MakeXL
        XREF    StartXL
        XREF    GetQCode
        XREF    SetSQStat
        XREF    DevName
        XREF    InitIntr

        XREF    Enable
        XREF    Disable

        XREF    PutStr
        XREF    PutChar
        XREF    PutHex

        INT_ABLES

*
************************************************************************
***
***  Internal Macros
***
************************************************************************

DEVFUN          macro
                dc.l    \1
                endm

DEVCOM          macro
MAX_CMD         set     MAX_CMD+1
                dc.l    \1
                endm

MAX_CMD         set     0


 STRUCTURE XXX,LIB_SIZE           ; Fake device base for cd.device

        ALIGNLONG
        APTR        db_CDROMPage  ; Hardware buffer pointers
        APTR        db_CDCOMRXPage
        APTR        db_CDCOMTXPage
        APTR        db_CDSUBPage
                                  ;                                Default  
        UWORD  xxx_PlaySpeed      ; Audio play speed               (75)        
        UWORD  xxx_ReadSpeed      ; Data-rate of CD_READ command   (Max)       
        UWORD  xxx_ReadXLSpeed    ; Data-rate of CD_READXL command (75)        
        UWORD  xxx_SectorSize     ; Number of bytes per sector     (2048)   
        UWORD  xxx_XLECC          ; CDXL ECC enabled/disabled               
        UWORD  xxx_EjectReset     ; Reset on eject enabled/disabled         
        STRUCT xxx_Reserved1,8    ; Reserved for future expansion           
                                                                           
        UWORD  xxx_MaxSpeed       ; Maximum speed drive can handle (75, 150)
        UWORD  xxx_AudioPrecision ; 0 = no attenuator, 1 = mute only,       
                                  ; other = (# levels - 1)                  
        UWORD  CDStatus

        LABEL  XXX_SIZE

*
************************************************************************
***
***  DEVICE FUNCTION/COMMAND ADDRESS TABLE
***
***     This table contains the addresses for device functions
***     and command functions.  It is used by MakeLibrary to
***     initialize the device node.
***
***     This table could be smaller (word size) but MANX
***     does not like the word size for some reason!!!
***
************************************************************************

        XDEF DevFuncs

DevFuncs
                DEVFUN  Open             ; -6
                DEVFUN  Close            ; -12
                DEVFUN  Expunge          ; -18
                DEVFUN  Freserved        ; -24
                DEVFUN  BeginIO          ; -30
                DEVFUN  AbortIO          ; -36
                DEVFUN  PerformIO        ; -42
                DEVFUN  Freserved        ; -48

                DEVCOM  CmdReset         ; 1 (-54)
                DEVCOM  CmdRead          ; 2
                DEVCOM  CmdWrite         ; 3
                DEVCOM  CmdUpdate        ; 4
                DEVCOM  CmdClear         ; 5
                DEVCOM  CmdStop          ; 6
                DEVCOM  CmdStart         ; 7
                DEVCOM  CmdFlush         ; 8
                DEVCOM  CmdMotor         ; 9
                DEVCOM  CmdSeek          ; 10
                DEVCOM  CmdFormat        ; 11
                DEVCOM  CmdRemove        ; 12
                DEVCOM  CmdChangeNum     ; 13
                DEVCOM  CmdChangeState   ; 14
                DEVCOM  CmdProtStatus    ; 15
                DEVCOM  NoCommand        ; 16
                DEVCOM  NoCommand        ; 17
                DEVCOM  CmdGetDriveType  ; 18
                DEVCOM  CmdGetNumTracks  ; 19
                DEVCOM  CmdAddChangeInt  ; 20
                DEVCOM  CmdRemChangeInt  ; 21
                DEVCOM  CmdGetGeometry   ; 22
                DEVCOM  CmdEject         ; 23

*
                DEVCOM  NoCommand        ; 24
                DEVCOM  NoCommand        ; 25
                DEVCOM  NoCommand        ; 26
                DEVCOM  NoCommand        ; 27
                DEVCOM  NoCommand        ; 28  (SCSI)
                DEVCOM  NoCommand        ; 29
                DEVCOM  NoCommand        ; 30
                DEVCOM  NoCommand        ; 31

                DEVCOM  NoCommand        ; 32   I
                DEVCOM  CmdStatus        ; 33   I
                DEVCOM  CmdQuickStatus   ; 34   I
                DEVCOM  CmdInfo          ; 35
                DEVCOM  CmdErrorInfo     ; 36   I
                DEVCOM  CmdIsROM         ; 37
                DEVCOM  CmdOptions       ; 38   
                DEVCOM  CmdFrontPanel    ; 39   I
                DEVCOM  CmdFrameCall     ; 40
                DEVCOM  CmdFrameCount    ; 41

                DEVCOM  CmdReadXL        ; 42

                DEVCOM  CmdPlayTrack     ; 43   I

                DEVCOM  CmdPlayLSN       ; 44   I T
                DEVCOM  CmdPlayMSF       ; 45   I T

                DEVCOM  CmdPlaySegsLSN   ; 46
                DEVCOM  CmdPlaySegsMSF   ; 47

                DEVCOM  CmdTOCLSN        ; 48   I T
                DEVCOM  CmdTOCMSF        ; 49   I T

                DEVCOM  CmdSubQLSN       ; 50   I T
                DEVCOM  CmdSubQMSF       ; 51   I T

                DEVCOM  CmdPause         ; 52   I T
                DEVCOM  CmdStopPlay      ; 53   I T

                DEVCOM  CmdPokeSegLSN    ; 54
                DEVCOM  CmdPokeSegMSF    ; 55

                DEVCOM  CmdMute          ; 56
                DEVCOM  CmdFade          ; 57

                DEVCOM  CmdPokePlayLSN   ; 58
                DEVCOM  CmdPokePlayMSF   ; 59

                DEVCOM  CmdGenLock       ; 60

                DEVCOM  CmdPlayMode      ; 61
                DEVCOM  CmdReadMode      ; 62

                dc.l    -1               ; end of list

*
************************************************************************
***
***  DEVICE COMMAND OPTION TABLE
***
***     This table contains bits to indicate valid options for
***     each of the device command functions.  Options are:
***
***     NODISK  - execute command even when no disk present
***     QUEUE   - serialize (queue) the command for execution
***
************************************************************************

        XDEF CmdOpts

CmdOpts:
        dc.w  OF_INVALID
        dc.w  OF_NODISK|OF_QUEUE|OF_DISKACCESS                          ; Reset
        dc.w  OF_QUEUE|OF_DISKACCESS|OF_ROM|OF_ABT3                     ; Read
        dc.w  OF_NODISK                                                 ; Write
        dc.w  OF_NODISK                                                 ; Update
        dc.w  OF_NODISK                                                 ; Clear
        dc.w  OF_NODISK                                                 ; Stop
        dc.w  OF_NODISK                                                 ; Start
        dc.w  OF_NODISK                                                 ; Flush
        dc.w  OF_NODISK|OF_QUEUE|OF_DISKACCESS                          ; Motor
        dc.w  OF_QUEUE|OF_DISKACCESS                                    ; Seek
        dc.w  OF_INVALID                                                ; Format
        dc.w  OF_INVALID                                                ; Remove
        dc.w  OF_NODISK                                                 ; ChangeNum
        dc.w  OF_NODISK                                                 ; ChangeState
        dc.w  0                                                         ; ProtStatus
        dc.w  OF_INVALID                                                ; NoCommand
        dc.w  OF_INVALID                                                ; NoCommand
        dc.w  OF_NODISK                                                 ; CmdGetDriveType
        dc.w  0                                                         ; CmdGetNumTracks
        dc.w  OF_NODISK|OF_NOREPLY                                      ; CmdAddChangeInt
        dc.w  OF_NODISK                                                 ; CmdRemChangeInt
        dc.w  0                                                         ; CmdGetGeometry
        dc.w  OF_NODISK|OF_QUEUE                                        ; CmdEject
        dc.w  OF_INVALID                                                ; NoCommand
        dc.w  OF_INVALID                                                ; NoCommand
        dc.w  OF_INVALID                                                ; NoCommand
        dc.w  OF_INVALID                                                ; NoCommand
        dc.w  OF_INVALID                                                ; NoCommand
        dc.w  OF_INVALID                                                ; NoCommand
        dc.w  OF_INVALID                                                ; NoCommand
        dc.w  OF_INVALID                                                ; NoCommand
*
        dc.w  OF_INVALID                                                ; CmdDirect               (no longer supported)
        dc.w  OF_NODISK|OF_QUEUE                                        ; CmdStatus
        dc.w  OF_NODISK                                                 ; CmdQuickStatus
        dc.w  OF_NODISK|OF_QUEUE                                        ; CmdInfo
        dc.w  OF_NODISK|OF_QUEUE                                        ; CmdErrorInfo
        dc.w  OF_QUEUE                                                  ; CmdIsROM
        dc.w  OF_NODISK|OF_QUEUE                                        ; CmdOptions
        dc.w  OF_NODISK|OF_QUEUE                                        ; CmdFrontPanel
        dc.w  OF_NODISK|OF_NOREPLY|OF_ABT1                              ; CmdFrameCall
        dc.w  OF_NODISK                                                 ; CmdFrameCount
        dc.w  OF_QUEUE|OF_DISKACCESS|OF_ROM|OF_ABT3                     ; CmdReadXL
        dc.w  OF_QUEUE|OF_DISKACCESS|OF_ASYNC|OF_ABT2                   ; CmdPlayTrack
        dc.w  OF_QUEUE|OF_DISKACCESS|OF_ASYNC|OF_ABT2                   ; CmdPlayLSN
        dc.w  OF_QUEUE|OF_DISKACCESS|OF_ASYNC|OF_ABT2                   ; CmdPlayMSF
        dc.w  OF_QUEUE|OF_DISKACCESS|OF_ASYNC|OF_ABT2                   ; CmdPlaySegsLSN
        dc.w  OF_QUEUE|OF_DISKACCESS|OF_ASYNC|OF_ABT2                   ; CmdPlaySegsMSF
        dc.w  OF_QUEUE                                                  ; CmdTOCLSN
        dc.w  OF_QUEUE                                                  ; CmdTOCMSF
        dc.w  OF_QUEUE                                                  ; CmdSubQLSN
        dc.w  OF_QUEUE                                                  ; CmdSubQMSF
        dc.w  OF_QUEUE                                                  ; CmdPause
        dc.w  OF_NODISK|OF_QUEUE                                        ; CmdStopPlay
        dc.w  0                                                         ; CmdPokeSegLSN
        dc.w  0                                                         ; CmdPokeSegMSF
        dc.w  OF_NODISK                                                 ; CmdMute
        dc.w  OF_NODISK|OF_QUEUE                                        ; CmdFade
        dc.w  OF_QUEUE                                                  ; CmdPokePlayLSN
        dc.w  OF_QUEUE                                                  ; CmdPokePlayMSF
        dc.w  OF_NODISK                                                 ; CmdGenLock

        dc.w  OF_NODISK|OF_QUEUE                                        ; CmdPlayMode
        dc.w  OF_NODISK|OF_QUEUE                                        ; CmdReadMode

        dc.w  0




CmdPorts:
        dc.w  0
        dc.w  0     ; Reset
        dc.w  1     ; Read
        dc.w  0     ; Write
        dc.w  0     ; Update
        dc.w  0     ; Clear
        dc.w  0     ; Stop
        dc.w  0     ; Start
        dc.w  0     ; Flush
        dc.w  1     ; Motor
        dc.w  1     ; Seek
        dc.w  0     ; Format
        dc.w  0     ; Remove
        dc.w  0     ; ChangeNum
        dc.w  0     ; ChangeState
        dc.w  0     ; ProtStatus
        dc.w  0     ; NoCommand
        dc.w  0     ; NoCommand
        dc.w  0     ; CmdGetDriveType
        dc.w  0     ; CmdGetNumTracks
        dc.w  2     ; CmdAddChangeInt
        dc.w  2     ; CmdRemChangeInt
        dc.w  0     ; CmdGetGeometry
        dc.w  0     ; CmdEject
        dc.w  0     ; NoCommand
        dc.w  0     ; NoCommand
        dc.w  0     ; NoCommand
        dc.w  0     ; NoCommand
        dc.w  0     ; NoCommand
        dc.w  0     ; NoCommand
        dc.w  0     ; NoCommand
        dc.w  0     ; NoCommand
*
        dc.w  -1    ; CmdDirect
        dc.w  -1    ; CmdStatus
        dc.w  -1    ; CmdQuickStatus
        dc.w  -1    ; CmdInfo
        dc.w  -1    ; CmdErrorInfo
        dc.w  -1    ; CmdIsROM
        dc.w  -1    ; CmdOptions
        dc.w  -1    ; CmdFrontPanel
        dc.w  3     ; CmdFrameCall
        dc.w  -1    ; CmdFrameCount
        dc.w  1     ; CmdReadXL
        dc.w  1     ; CmdPlayTrack
        dc.w  1     ; CmdPlayLSN
        dc.w  1     ; CmdPlayMSF
        dc.w  1     ; CmdPlaySegsLSN
        dc.w  1     ; CmdPlaySegsMSF
        dc.w  -1    ; CmdTOCLSN
        dc.w  -1    ; CmdTOCMSF
        dc.w  0     ; CmdSubQLSN
        dc.w  0     ; CmdSubQMSF
        dc.w  -1    ; CmdPause
        dc.w  -1    ; CmdStopPlay
        dc.w  1     ; CmdPokeSegLSN
        dc.w  1     ; CmdPokeSegMSF
        dc.w  -1    ; CmdMute
        dc.w  -1    ; CmdFade
        dc.w  1     ; CmdPokePlayLSN
        dc.w  1     ; CmdPokePlayMSF
        dc.w  -1    ; CmdGenLock

        dc.w  -1    ; CmdPlayMode
        dc.w  -1    ; CmdReadMode

        dc.w  0


TimerDevName:   dc.b    "timer.device",0,0



 FUNCTION CDBeginIO

                save    d0-d1/a1/a2                                     ; Get device base to cdtv.device
                save    a1
                move.l  $4,a0
                lea     DeviceList(a0),a0
                lea     DevName(pc),a1
                exec    FindName
                move.l  d0,a2
                restore a1

                cmp.w   #CD_READ,IO_COMMAND(a1)                         ; If this is a Read command, is a Play command active?
                bne     2$
                move.l  db_IORD(a2),a1
                move.w  IO_COMMAND(a1),d0
                cmp.w   #CD_PLAYTRACK,d0
                beq     1$
                cmp.w   #CD_PLAYMSF,d0
                beq     1$
                cmp.w   #CD_PLAYLSN,d0
                bne     2$
1$              exec    AbortIO                                         ; - Abort the Play command
2$
                move.l  a2,a0
                restore d0-d1/a1/a2

                move.l  db_CDBeginIO(a0),a0                             ; Jump to CD's BeginIO
                jmp     (a0)









*
************************************************************************
***
***  BEGINIO
***
***     Process an I/O request.
***
***     This function is called with:
***             A1 -> IO Request Structure
***             A6 -> Device Node
***
************************************************************************

BeginIO:
                save    a1/a2/ior
                move.l  a1,ior

                jsr     PERFORMIO(db)                                   ; Execute the command now and reply (return)
                restore a1/a2/ior
                rts

*
************************************************************************
***
***  ABORTIO
***
***     Abort an IO Request that is queued or in progress.
***
************************************************************************

AbortIO:
                save    ior
                move.l  a1,ior

;                PRINT   51$,52$,"***CDTV:_ABORTIO"

                move.w  IO_COMMAND(a1),d0                               ; It ain't pretty, but it works.
                cmp.w   #CDTV_FRAMECALL,d0
                bne     2$
                move.l  #-1,db_CallAddress(db)
1$              tst.l   db_CallAddress(db)
                bne     1$

                move.l  ior,a1
                exec    ReplyMsg
                bra     99$
2$
                lsl.l   #1,d0
                lea     CmdPorts(pc),a0
                move.w  0(a0,d0.w),d0

                move.l  IO_UNIT(a1),a1

                tst.w   d0
                bmi     99$

                bne     3$
                move.l  UNIT_IORA(a1),a1
                bra     9$
3$
                cmp.w   #1,d0
                bne     4$
                move.l  db_IORD(db),a1
                bra     9$
4$
                cmp.w   #2,d0
                bne     9$
                move.l  UNIT_IORChg(a1),a1
9$
                exec    AbortIO
99$
                restore ior
                rts

************************************************************************
***
***  PERFORMIO
***
***     Dispatch to the appropriate command handling code.
***
************************************************************************

PerformIO:

;                jsr     PrintCommand

                clr.l   d0                                              ; Execute Command
                move.w  IO_COMMAND(ior),d0

                addq.l  #8,d0
                muls    #-6,d0
                jsr     0(db,d0.w)

;                clr.l   d0
;                move.b  IO_ERROR(ior),d0
;                beq     1$
;                jsr     PutHex
;1$
;                move.b  #10,d0
;                jsr     PutChar
                rts




PrintCommand:
                move.b  #'*',d0
                jsr     PutChar
                jsr     PutChar
                jsr     PutChar
                move.b  #'C',d0
                jsr     PutChar
                move.b  #'D',d0
                jsr     PutChar
                move.b  #'T',d0
                jsr     PutChar
                move.b  #'V',d0
                jsr     PutChar
                move.b  #':',d0
                jsr     PutChar
                move.b  #' ',d0
                jsr     PutChar

                clr.l   d0
                move.w  IO_COMMAND(ior),d0
                jsr     PutHex
                move.l  IO_OFFSET(ior),d0
                jsr     PutHex
                move.l  IO_LENGTH(ior),d0
                jsr     PutHex
                move.l  IO_DATA(ior),d0
                jsr     PutHex

                rts

*
************************************************************************
***
***  DEVICE COMMANDS
***
***     Primary functions to support drive commands.  These are
***     ordered in logical groups below.
***
************************************************************************

******* Reset **********************************************************

CmdReset:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                clr.l   IO_ACTUAL(ior)                                  ; Return results
                clr.b   IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts


*
******* Read ***********************************************************

CmdRead:
                move.l  db_IORD(db),a1
                move.w  IO_COMMAND(a1),d0
                cmp.w   #CD_PLAYTRACK,d0
                beq     1$
                cmp.w   #CD_PLAYMSF,d0
                beq     1$
                cmp.w   #CD_PLAYLSN,d0
                bne     2$
1$
                bset    #0,db_abortedflags(db)
                exec    AbortIO
2$
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  db_IORD(db),a2                                  ; Send new IORequest

                move.w  IO_COMMAND(ior),IO_COMMAND(a2)
                move.l  IO_DATA(ior),IO_DATA(a2)
                move.l  IO_LENGTH(ior),IO_LENGTH(a2)
                move.l  IO_OFFSET(ior),IO_OFFSET(a2)
                move.b  IO_FLAGS(ior),IO_FLAGS(a2)

                lea     db_DiskPort(db),a0
                move.l  ior,a1
                exec    PutMsg
                rts


*
******* ReadXL *********************************************************

CmdReadXL:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                lea     db_DiskPort(db),a0
                move.l  ior,a1
                exec    PutMsg
                rts


*
******* Seek ***********************************************************

CmdSeek:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  db_IORD(db),a2                                  ; Send new IORequest
                move.w  IO_COMMAND(ior),IO_COMMAND(a2)
                move.l  IO_DATA(ior),IO_DATA(a2)
                move.l  IO_LENGTH(ior),IO_LENGTH(a2)
                move.l  IO_OFFSET(ior),IO_OFFSET(a2)
                clr.b   IO_FLAGS(a2)

                lea     db_DiskPort(db),a0
                move.l  ior,a1
                exec    PutMsg
                rts



******* Motor **********************************************************

CmdMotor:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                clr.l   IO_ACTUAL(ior)                                  ; Return results
                clr.b   IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts



*
******* ChangeNum ******************************************************

CmdChangeNum:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  IO_UNIT(ior),a2
                move.l  UNIT_IORA(a2),a2                                  ; Send new IORequest
                move.w  IO_COMMAND(ior),IO_COMMAND(a2)
                move.l  IO_DATA(ior),IO_DATA(a2)
                move.l  IO_LENGTH(ior),IO_LENGTH(a2)
                move.l  IO_OFFSET(ior),IO_OFFSET(a2)
                clr.b   IO_FLAGS(a2)
                move.l  a2,a1
                exec    DoIO

                move.l  IO_ACTUAL(a2),IO_ACTUAL(ior)                    ; Return results
                move.b  IO_ERROR(a2),IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts


******* ChangeState ****************************************************

CmdChangeState:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  IO_UNIT(ior),a2
                move.l  UNIT_IORA(a2),a2                                  ; Send new IORequest
                move.w  IO_COMMAND(ior),IO_COMMAND(a2)
                move.l  IO_DATA(ior),IO_DATA(a2)
                move.l  IO_LENGTH(ior),IO_LENGTH(a2)
                move.l  IO_OFFSET(ior),IO_OFFSET(a2)
                clr.b   IO_FLAGS(a2)
                move.l  a2,a1
                exec    DoIO

                move.l  IO_ACTUAL(a2),IO_ACTUAL(ior)                    ; Return results
                move.b  IO_ERROR(a2),IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts


******* AddChangeInt ***************************************************

NoResetRemove:
                dc.l    TAGCD_EJECTRESET,0
                dc.l    TAG_END,0

ResetRemove:
                dc.l    TAGCD_EJECTRESET,1
                dc.l    TAG_END,0


CmdAddChangeInt:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  IO_UNIT(ior),a2
                move.l  UNIT_IORA(a2),a2                                ; Send new IORequest

                move.w  #CD_CONFIG,IO_COMMAND(a2)
                lea     NoResetRemove(pc),a0
                move.l  a0,IO_DATA(a2)
                move.l  a2,a1
                exec    DoIO

                move.l  IO_UNIT(ior),a2
                move.l  UNIT_IORChg(a2),a2                              ; Send new IORequest
                move.w  IO_COMMAND(ior),IO_COMMAND(a2)
                move.l  IO_DATA(ior),IO_DATA(a2)
                move.l  IO_LENGTH(ior),IO_LENGTH(a2)
                move.l  IO_OFFSET(ior),IO_OFFSET(a2)
                clr.b   IO_FLAGS(a2)
                move.l  a2,a1
                exec    SendIO

                move.b  IO_ERROR(a2),IO_ERROR(ior)
                rts


******* RemChangeInt ***************************************************

CmdRemChangeInt:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  IO_UNIT(ior),a2
                move.l  UNIT_IORA(a2),a2                                ; Send new IORequest

                move.w  #CD_CONFIG,IO_COMMAND(a2)
                lea     ResetRemove(pc),a0
                move.l  a0,IO_DATA(a2)
                move.l  a2,a1
                exec    DoIO

                move.l  IO_UNIT(ior),a2
                move.l  UNIT_IORChg(a2),a2                              ; Send new IORequest
                move.w  #CD_REMCHANGEINT,IO_COMMAND(a2)
                move.l  a2,a1
                exec    DoIO

                move.l  ior,a1
                exec    ReplyMsg
                rts



******* ProtStatus *****************************************************

CmdProtStatus:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  IO_UNIT(ior),a2
                move.l  UNIT_IORA(a2),a2                                  ; Send new IORequest
                move.w  IO_COMMAND(ior),IO_COMMAND(a2)
                move.l  IO_DATA(ior),IO_DATA(a2)
                move.l  IO_LENGTH(ior),IO_LENGTH(a2)
                move.l  IO_OFFSET(ior),IO_OFFSET(a2)
                clr.b   IO_FLAGS(a2)
                move.l  a2,a1
                exec    DoIO

                move.l  IO_ACTUAL(a2),IO_ACTUAL(ior)                    ; Return results
                move.b  IO_ERROR(a2),IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts


******* GetDriveType ***************************************************

CmdGetDriveType:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  IO_UNIT(ior),a2
                move.l  UNIT_IORA(a2),a2                                  ; Send new IORequest
                move.w  IO_COMMAND(ior),IO_COMMAND(a2)
                move.l  IO_DATA(ior),IO_DATA(a2)
                move.l  IO_LENGTH(ior),IO_LENGTH(a2)
                move.l  IO_OFFSET(ior),IO_OFFSET(a2)
                clr.b   IO_FLAGS(a2)
                move.l  a2,a1
                exec    DoIO

                move.l  IO_ACTUAL(a2),IO_ACTUAL(ior)                    ; Return results
                move.b  IO_ERROR(a2),IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts


******* GetNumTracks ***************************************************

CmdGetNumTracks:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  IO_UNIT(ior),a2
                move.l  UNIT_IORA(a2),a2                                  ; Send new IORequest
                move.w  IO_COMMAND(ior),IO_COMMAND(a2)
                move.l  IO_DATA(ior),IO_DATA(a2)
                move.l  IO_LENGTH(ior),IO_LENGTH(a2)
                move.l  IO_OFFSET(ior),IO_OFFSET(a2)
                clr.b   IO_FLAGS(a2)
                move.l  a2,a1
                exec    DoIO

                move.l  IO_ACTUAL(a2),IO_ACTUAL(ior)                    ; Return results
                move.b  IO_ERROR(a2),IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts


******* GetGeometry ****************************************************

CmdGetGeometry:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  IO_UNIT(ior),a2
                move.l  UNIT_IORA(a2),a2                                  ; Send new IORequest
                move.w  IO_COMMAND(ior),IO_COMMAND(a2)
                move.l  IO_DATA(ior),IO_DATA(a2)
                move.l  IO_LENGTH(ior),IO_LENGTH(a2)
                move.l  IO_OFFSET(ior),IO_OFFSET(a2)
                clr.b   IO_FLAGS(a2)
                move.l  a2,a1
                exec    DoIO

                move.l  IO_ACTUAL(a2),IO_ACTUAL(ior)                    ; Return results
                move.b  IO_ERROR(a2),IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts

*
******* PlayTrack ******************************************************

CmdPlayTrack:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  db_IORD(db),a2                                  ; Send new IORequest

                move.w  #CD_PLAYTRACK,IO_COMMAND(a2)
                move.l  IO_DATA(ior),IO_DATA(a2)
                move.l  IO_LENGTH(ior),d0
                bne     1$
                move.l  IO_OFFSET(ior),d0
                add.l   #1,d0
1$              sub.l   IO_OFFSET(ior),d0
                move.l  d0,IO_LENGTH(a2)
                move.l  IO_OFFSET(ior),IO_OFFSET(a2)
                clr.b   IO_FLAGS(a2)

                lea     db_DiskPort(db),a0
                move.l  ior,a1
                exec    PutMsg
                rts

*
******* PlayLSN ********************************************************

CmdPlayLSN:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  db_IORD(db),a2                                  ; Send new IORequest
                move.w  #CD_PLAYLSN,IO_COMMAND(a2)
                move.l  IO_DATA(ior),IO_DATA(a2)
                move.l  IO_LENGTH(ior),IO_LENGTH(a2)
                move.l  IO_OFFSET(ior),IO_OFFSET(a2)
                clr.b   IO_FLAGS(a2)

                lea     db_DiskPort(db),a0
                move.l  ior,a1
                exec    PutMsg
                rts


******* PlayMSF ********************************************************

CmdPlayMSF:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  db_IORD(db),a2                                  ; Send new IORequest

                move.w  #CD_PLAYLSN,IO_COMMAND(a2)
                move.l  IO_DATA(ior),IO_DATA(a2)
                move.l  IO_LENGTH(ior),d0
                jsr     MSFtoLSN
                move.l  d0,d1
                move.l  IO_OFFSET(ior),d0
                jsr     MSFtoLSN
                sub.l   d0,d1
                move.l  d1,IO_LENGTH(a2)
                move.l  IO_OFFSET(ior),d0
                jsr     MSFtoLSNPOS
                move.l  d0,IO_OFFSET(a2)
                clr.b   IO_FLAGS(a2)

                lea     db_DiskPort(db),a0
                move.l  ior,a1
                exec    PutMsg
                rts



******* PokePlayLSN ****************************************************

CmdPokePlayLSN:
                move.l  db_IORD(db),a1
                exec    AbortIO

                bra     CmdPlayLSN



******* PokePlayMSF ****************************************************

CmdPokePlayMSF:
                move.l  db_IORD(db),a1
                exec    AbortIO

                bra     CmdPlayMSF

*
******* StopPlay *******************************************************

CmdStopPlay:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  db_IORD(db),a1
                exec    AbortIO

                clr.l   IO_ACTUAL(ior)                                  ; Return results
                clr.b   IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts


******* PlaySegsLSN ****************************************************

CmdPlaySegsLSN:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  ior,a1
                exec    ReplyMsg
                rts



******* PlaySegsMSF ****************************************************

CmdPlaySegsMSF:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  ior,a1
                exec    ReplyMsg
                rts


*
******* PokeSegLSN *****************************************************

CmdPokeSegLSN:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  ior,a1
                exec    ReplyMsg
                rts


******* PokeSegMSF *****************************************************

CmdPokeSegMSF:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  ior,a1
                exec    ReplyMsg
                rts


*
******* CmdTOCLSN ******************************************************

CmdTOCLSN:
                move.l  IO_UNIT(ior),a2
                move.l  UNIT_IORA(a2),a2                                  ; Send new IORequest
                move.w  #CD_TOCLSN,IO_COMMAND(a2)
                bra     CmdTOC


******* CmdTOCMSF ******************************************************

CmdTOCMSF:
                move.l  IO_UNIT(ior),a2
                move.l  UNIT_IORA(a2),a2                                  ; Send new IORequest
                move.w  #CD_TOCMSF,IO_COMMAND(a2)
CmdTOC:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                          ; Clear IOB_QUICK

                lea     db_TOC(db),a1
                move.l  a1,IO_DATA(a2)
                move.l  IO_LENGTH(ior),d0
                cmp.l   #100,d0
                bls     0$
                move.l  #100,d0
0$              move.l  d0,IO_LENGTH(a2)
                move.l  IO_OFFSET(ior),IO_OFFSET(a2)
                clr.b   IO_FLAGS(a2)
                move.l  a2,a1
                exec    DoIO

                tst.b   IO_ERROR(a2)
                bne     91$

                save    d2
                move.l  IO_ACTUAL(a2),d2
                beq     90$

                lea     db_TOC(db),a0
                move.l  IO_DATA(ior),a1
1$
                cmp.l   IO_ACTUAL(a2),d2                                ; Is this summary information?
                bne     3$
                tst.l   IO_OFFSET(ior)
                bne     3$

                clr.w   (a1)+                                           ; Copy summary information
                move.b  (a0)+,(a1)+
                move.b  (a0)+,(a1)+
                move.l  (a0)+,(a1)+
                bra     4$
3$
                clr.b   (a1)+                                           ; Copy entry information
                move.b  (a0),d0
                lsr.b   #4,d0
                and.b   #$0f,d0
                move.b  (a0)+,d1
                lsl.b   #4,d1
                and.b   #$f0,d1
                or.b    d1,d0
                move.b  d0,(a1)+
                move.b  (a0)+,(a1)+
                clr.b   (a1)+
                move.l  (a0)+,(a1)+
4$
                sub.l   #1,d2
                bne     1$
                restore d2
90$
                move.l  IO_ACTUAL(a2),d0                                ; Calculate IO_ACTUAL

                tst.l   IO_OFFSET(ior)                                  ; Subtract 1 if summary information was returned
                bne     91$
                subq.l  #1,d0

                cmp.l   #1,IO_LENGTH(ior)                               ; If only summary info was requested, return 1
                bne     91$
                move.l  #1,d0
91$
                move.l  d0,IO_ACTUAL(ior)
                move.b  IO_ERROR(a2),IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts



*
******* SubQLSN ********************************************************

CmdSubQLSN:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  IO_UNIT(ior),a2
                move.l  UNIT_IORA(a2),a2                                  ; Send new IORequest
                move.w  #CD_QCODELSN,IO_COMMAND(a2)
                lea     db_QCode(db),a0
                move.l  a0,IO_DATA(a2)
                move.l  IO_LENGTH(ior),IO_LENGTH(a2)
                move.l  IO_OFFSET(ior),IO_OFFSET(a2)
                clr.b   IO_FLAGS(a2)
                move.l  a2,a1
                exec    DoIO

                move.l  IO_DATA(ior),a0
                move.b  #$11,0(a0)
                clr.l   d0
                move.b  db_QCode(db),d0
                move.b  d0,d1
                lsr.w   #4,d1
                lsl.w   #4,d0
                or.b    d0,d1
                move.b  d1,1(a0)
                move.b  db_QCode+1(db),2(a0)
                move.b  db_QCode+2(db),3(a0)
                move.l  db_QCode+4(db),8(a0)
                move.l  db_QCode+8(db),4(a0)
                clr.l   $c(a0)

                move.l  IO_ACTUAL(a2),IO_ACTUAL(ior)                    ; Return results
                move.b  IO_ERROR(a2),IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts


******* SubQMSF ********************************************************

CmdSubQMSF:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  IO_UNIT(ior),a2
                move.l  UNIT_IORA(a2),a2                                  ; Send new IORequest

                move.w  #CD_QCODEMSF,IO_COMMAND(a2)
                lea     db_QCode(db),a0
                move.l  a0,IO_DATA(a2)
                move.l  IO_LENGTH(ior),IO_LENGTH(a2)
                move.l  IO_OFFSET(ior),IO_OFFSET(a2)
                clr.b   IO_FLAGS(a2)
                move.l  a2,a1
                exec    DoIO

                move.l  IO_DATA(ior),a0
                move.b  #$11,0(a0)
                clr.l   d0
                move.b  db_QCode(db),d0
                move.b  d0,d1
                lsr.w   #4,d1
                lsl.w   #4,d0
                or.b    d0,d1
                move.b  d1,1(a0)
                move.b  db_QCode+1(db),2(a0)
                move.b  db_QCode+2(db),3(a0)
                move.l  db_QCode+4(db),8(a0)
                move.l  db_QCode+8(db),4(a0)
                clr.l   $c(a0)

                move.l  IO_ACTUAL(a2),IO_ACTUAL(ior)                    ; Return results
                move.b  IO_ERROR(a2),IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts


*
******* FrameCall ******************************************************

CmdFrameCall:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  IO_DATA(ior),db_CallAddress(db)

                tst.l   db_IORFrm(db)
                bne     97$

                moveq.l #0,d1                                           ; Init interrupt server
                lea     FrameInt(pc),a0
                lea     db_FrameIntStruct(db),a1
                bsr     InitIntr

                exec    CreateMsgPort                                   ; Create msg port Frm
                move.l  d0,db_PortFrm(db)
                beq     98$

                move.l  d0,a1
                lea     db_FrameIntStruct(db),a0
                move.l  a0,MP_SIGTASK(a1)
                move.b  #PA_SOFTINT,MP_FLAGS(a1)

                move.l  db_PortFrm(db),a0                               ; Create I/O request Frm
                move.l  #IOTV_SIZE,d0
                exec    CreateIORequest
                move.l  d0,db_IORFrm(db)
                beq     98$

                lea     TimerDevName(pc),a0
                move.l  #UNIT_MICROHZ,d0
                move.l  db_IORFrm(db),a1
                clr.l   d1
                exec    OpenDevice
                tst.b   d0
                bne     98$
97$
                jsr     SendTimerReq
98$
                rts




************************************************************************
*                                                                      *
*   FrameInt - Frame call interrupt processor                          *
*                                                                      *
************************************************************************

 FUNCTION FrameInt
                save    d2/ior/db                                       ; Load up registers

                move.l  a1,db
                move.l  db_IORFrm(db),ior

                lea     db_PortFrm(db),a0                               ; Remove the request
                exec    GetMsg

                move.l  db_CallAddress(db),d2                           ; This is a bogus way of doing it, but who cares for this
                beq     99$
                cmp.l   #-1,d2
                bne     1$
                clr.l   db_CallAddress(db)
                bra     99$
1$
                add.l   #1,db_FrameCount(db)

                jsr     SendTimerReq                                    ; Start new request

                move.l  d2,a0                                           ; Call interrupt with IOReq in A2
                save    a2
                move.l  ior,a2
                jsr     (a0)
                restore a2
99$
                restore d2/ior/db
                clr.w   d0                                              ; Clear server chain
                rts


************************************************************************
*                                                                      *
*   SendTimerReq - Send a timer interrupt request                      *
*                                                                      *
************************************************************************

 FUNCTION SendTimerReq

                move.l  db_IORFrm(db),a1
                clr.l   IOTV_TIME+TV_SECS(a1)
                move.l  #13333,IOTV_TIME+TV_MICRO(a1)
                move.w  #TR_ADDREQUEST,IO_COMMAND(a1)
                clr.b   IO_ERROR(a1)

                exec    SendIO
                rts



******* FrameCount *****************************************************

CmdFrameCount:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  db_FrameCount(db),IO_ACTUAL(ior)                ; Return results
                clr.b   IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts


*
******* Pause **********************************************************

CmdPause:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  IO_UNIT(ior),a2
                move.l  UNIT_IORA(a2),a2                                  ; Send new IORequest
                move.w  #CD_PAUSE,IO_COMMAND(a2)
                move.l  IO_DATA(ior),IO_DATA(a2)
                move.l  IO_LENGTH(ior),IO_LENGTH(a2)
                move.l  IO_OFFSET(ior),IO_OFFSET(a2)
                clr.b   IO_FLAGS(a2)
                move.l  a2,a1
                exec    DoIO

                move.l  IO_ACTUAL(a2),IO_ACTUAL(ior)                    ; Return results
                move.b  IO_ERROR(a2),IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts


******* Fade ***********************************************************

CmdFade:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  IO_UNIT(ior),a2
                move.l  UNIT_IORA(a2),a2                                  ; Send new IORequest
                move.w  #CD_ATTENUATE,IO_COMMAND(a2)
                move.l  IO_DATA(ior),IO_DATA(a2)
                move.l  IO_LENGTH(ior),IO_LENGTH(a2)
                move.l  IO_OFFSET(ior),IO_OFFSET(a2)
                clr.b   IO_FLAGS(a2)
                move.l  a2,a1
                exec    DoIO

                move.l  IO_ACTUAL(a2),IO_ACTUAL(ior)                    ; Return results
                move.b  IO_ERROR(a2),IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts


*
******* Mute ***********************************************************

CmdMute:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.l  IO_UNIT(ior),a2
                move.l  UNIT_IORA(a2),a2                                ; Send new IORequest
                move.w  #CD_ATTENUATE,IO_COMMAND(a2)
                move.l  IO_DATA(ior),IO_DATA(a2)
                clr.l   IO_LENGTH(a2)
                move.l  IO_OFFSET(ior),IO_OFFSET(a2)
                tst.l   IO_LENGTH(ior)
                bne     1$
                move.l  #-1,IO_OFFSET(a2)
1$              clr.b   IO_FLAGS(a2)
                move.l  a2,a1
                exec    DoIO

                move.l  IO_ACTUAL(a2),IO_ACTUAL(ior)                    ; Return results
                move.b  IO_ERROR(a2),IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts




******* Status *********************************************************

CmdStatus:
CmdQuickStatus:
                move.l  db_IORD(db),a0                                  ; Get drive status
                move.l  IO_DEVICE(a0),a0
                move.w  CDStatus(a0),d1

                move.l  #$09,d0

                btst    #CDSTSB_DISK,d1
                beq     1$
                or.l    #QSF_DISK,d0
1$
                btst    #CDSTSB_SPIN,d1
                beq     2$
                or.l    #QSF_SPIN,d0
2$
                btst    #CDSTSB_PLAYING,d1
                beq     3$
                or.l    #QSF_AUDIO,d0
3$
                move.l  d0,IO_ACTUAL(ior)                                  ; Return results
                clr.b   IO_ERROR(ior)

                btst.b  #IOB_QUICK,IO_FLAGS(ior)
                bne     4$
                move.l  ior,a1
                exec    ReplyMsg
4$
                rts


******* ErrorInfo ******************************************************

CmdErrorInfo:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                clr.l   IO_ACTUAL(ior)                                  ; Return results
                clr.b   IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg

                rts


*
******* Info ***********************************************************

CmdInfo:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                cmp.l   #CDTV_INFO_BLOCK_SIZE,IO_OFFSET(ior)
                bne     1$
                move.l  #2048,IO_ACTUAL(ior)
1$
                cmp.l   #CDTV_INFO_FRAME_RATE,IO_OFFSET(ior)
                bne     2$
                move.l  #75,IO_ACTUAL(ior)
2$
                clr.b   IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts



******* Eject **********************************************************

CmdEject:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                clr.l   IO_ACTUAL(ior)                                  ; Return results
                clr.b   IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts



******* IsROM **********************************************************

CmdIsROM:
                clr.l   IO_ACTUAL(ior)                                  ; Return results
                clr.b   IO_ERROR(ior)

                move.l  db_IORD(db),a0                                  ; Get drive status
                move.l  IO_DEVICE(a0),a0
                move.w  CDStatus(a0),d0
                btst    #CDSTSB_CDROM,d0
                beq     1$
                move.l  #-1,IO_ACTUAL(ior)
1$
                btst.b  #IOB_QUICK,IO_FLAGS(ior)
                bne     2$
                move.l  ior,a1
                exec    ReplyMsg
2$
                rts


*
******* FrontPanel *****************************************************

CmdFrontPanel:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                clr.l   IO_ACTUAL(ior)                                  ; Return results
                clr.b   IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts


******** CmdGenLock ****************************************************

CmdGenLock:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                clr.l   IO_ACTUAL(ior)                                  ; Return results
                clr.b   IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts


*
******* PlayMode *******************************************************

CmdPlayMode:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                clr.l   IO_ACTUAL(ior)                                  ; Return results
                clr.b   IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts


*
******* ReadMode *******************************************************

CmdReadMode:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                clr.l   IO_ACTUAL(ior)                                  ; Return results
                clr.b   IO_ERROR(ior)

                move.l  ior,a1
                exec    ReplyMsg
                rts



******* Illegal Commands ***********************************************

CmdFormat:
CmdWrite:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Clear IOB_QUICK

                move.b  #CDERR_WriteProt,IO_ERROR(ior)                  ; All disks are write protected

                move.l  ior,a1
                exec    ReplyMsg
                rts



TagList2048:
                dc.l    TAGCD_SECTORSIZE,2048
                dc.l    TAG_END,0

TagList2336:
                dc.l    TAGCD_SECTORSIZE,2336
                dc.l    TAG_END,0


CmdOptions:
                move.l  IO_UNIT(ior),a2
                move.l  UNIT_IORA(a2),a2                                  ; Send new IORequest

                move.l  IO_OFFSET(ior),d0
                bne.s   co_bs
                bra.s   co_exit

        ;------ If 1: set block size
co_bs:          subq.w  #1,d0
                bne.s   co_er
                move.l  IO_LENGTH(ior),d0

                cmp.l   #2336,d0
                blo     1$
                lea     TagList2336(pc),a0
                move.l  #2336,d0
                bra     2$
1$
                lea     TagList2048(pc),a0
                move.l  #2048,d0
2$
                move.l  d0,db_BlockSize(db)
                move.w  #CD_CONFIG,IO_COMMAND(a2)
                move.l  a0,IO_DATA(a2)
                move.l  a2,a1
                exec    DoIO

                bra.s   co_exit

        ;------ If 2: set error recovery
co_er:          subq.w  #1,d0
co_exit:
                rts


******* Dummy Commands *************************************************

CmdUpdate:                                                              ; These commands do nothing
CmdClear:
CmdStop:
CmdStart:
CmdFlush:
CmdRemove:
NoCommand:

                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Choose a port and queue the command
                move.l  ior,a1
                exec    ReplyMsg
                rts


