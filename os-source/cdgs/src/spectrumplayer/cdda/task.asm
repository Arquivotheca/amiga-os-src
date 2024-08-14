 

        INCLUDE "exec/types.i"
        INCLUDE "exec/nodes.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/ports.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/tasks.i"
        INCLUDE "exec/devices.i"
        INCLUDE "exec/memory.i"
        INCLUDE "exec/interrupts.i"
        INCLUDE "exec/resident.i"
        INCLUDE "exec/io.i"
        INCLUDE "exec/errors.i"
        INCLUDE "exec/execbase.i"
        INCLUDE "hardware/intbits.i"
        INCLUDE "devices/trackdisk.i"
        INCLUDE "utility/tagitem.i"
        INCLUDE "exec/ables.i"

        INCLUDE "defs.i"
        INCLUDE "cd.i"
        INCLUDE "cdprivate.i"
        INCLUDE "cdgs_hw.i"

        OPT     p=68020

************************************************************************
***
***  External References
***
************************************************************************

        XREF    MSFtoLSNPOS
        XREF    LSNtoMSFPOS
        XREF    MSFtoLSN
        XREF    LSNtoMSF
        XREF    BCDtoBIN
        XREF    MSFBINtoBCD
        XREF    MSFBCDtoBIN
        XREF    Attenuate
        XREF    Open
        XREF    Close
        XREF    Expunge
        XREF    PerformIO
        XREF    SetSubcodeInterrupt
        XREF    ClearSubcodeInterrupt
        XREF    CauseInterruptList
        XREF    CheckSeekRange
        XREF    CheckPlayRange

        XREF    DoCmd
        XREF    DoPacket
        XREF    SendPacket
        XREF    READTOC
        XREF    ID_PACKET
        XREF    ClearPrefetch
        XREF    ClearPrefetchIfError
        XREF    CmdOpts
        XREF    StopRead
        XREF    TransferSample

        XREF    Enable
        XREF    Disable

        XREF    PutHex
        XREF    PutChar

        XDEF    CDTask

        XDEF    NoCommand      
        XDEF    CmdReset       
        XDEF    CmdRead        
        XDEF    CmdWrite       
        XDEF    CmdUpdate      
        XDEF    CmdClear       
        XDEF    CmdStop        
        XDEF    CmdStart       
        XDEF    CmdFlush       
        XDEF    CmdMotor       
        XDEF    CmdSeek        
        XDEF    CmdFormat      
        XDEF    CmdRemove      
        XDEF    CmdChangeNum   
        XDEF    CmdChangeState 
        XDEF    CmdProtStatus  
        XDEF    CmdGetDriveType
        XDEF    CmdGetNumTracks
        XDEF    CmdAddChangeInt
        XDEF    CmdRemChangeInt
        XDEF    CmdGetGeometry 
        XDEF    CmdEject       
        XDEF    CmdInfo        
        XDEF    CmdConfig
        XDEF    CmdTOCMSF
        XDEF    CmdTOCLSN
        XDEF    CmdReadXL      
        XDEF    CmdPlayTrack   
        XDEF    CmdPlayMSF
        XDEF    CmdPlayLSN      
        XDEF    CmdPause       
        XDEF    CmdSearch
        XDEF    CmdQCodeMSF
        XDEF    CmdQCodeLSN     
        XDEF    CmdAttenuate   
        XDEF    CmdAddFrameInt 
        XDEF    CmdRemFrameInt 
        XDEF    CmdSampleTrack   
        XDEF    CmdSampleMSF
        XDEF    CmdSampleLSN      
        XDEF    CmdSampleXLMSF
        XDEF    CmdSampleXLLSN

        INT_ABLES

*
************************************************************************
*                                                                      *
*   CD DEVICE DRIVER TASK                                              *
*                                                                      *
*       This is the task that runs the device driver.  Its main        *
*       body is a loop that reads requests from two message ports,     *
*       performs commands, and replies to the requests.                *
*                                                                      *
*       Two message ports are used to catagorize the requests into     *
*       two different classes:  Requests that directly access the      *
*       CD, and requests that don't or that only modify the behavior   *
*       of requests that do.  A PAUSE command for example modifies     *
*       the behavior of a PLAY command, but it doesn't effect the      *
*       request itself (aside from making the request finish later).   *
*                                                                      *
************************************************************************

CDTask:
                move.l  4(sp),db                                        ; Get device base arg and init hardware
                move.l  #CDHARDWARE,hb

                clr.l   d0                                              ; Clear all command signals
                move.l  #SIGF_CMDDONE|SIGF_PLAYDONE|SIGF_ABORTDRIVE,d1
                exec    SetSignal

                jsr     ID_PACKET                                       ; Get drive ID and set state open/closed

                jsr     READTOC                                         ; Try and detect a disk

                move.l  db_InitTask(db),a1                              ; Signal initialization task that we are running
                move.b  db_InitSignal(db),d1
                move.l  #1,d0
                lsl.l   d1,d0
                exec    Signal
TaskLoop
                clr.l   d0                                              ; Check for signals first
                move.l  d0,d1
                exec    SetSignal
                move.l  d0,d2

*************** Test Drive Abort Signal ********************************

TestSignals
                btst    #SIGB_ABORTDRIVE,d2                             ; Abort disk command?
                beq     CheckAudSector

                clr.l   d0                                              ; Clear this signal
                move.l  #SIGF_ABORTDRIVE,d1
                exec    SetSignal

                move.w  db_Info+CDINFO_Status(db),d0                    ; If we are playing, play is now complete
                btst    #CDSTSB_PLAYING,d0
                bne     PlayComplete


*************** Test Drive Abort Signal ********************************

CheckAudSector
                btst    #SIGB_AUDPBX,d2                                 ; Audio data from sample available?
                beq     CheckPlaySig

                clr.l   d0                                              ; Clear this signal
                move.l  #SIGF_AUDPBX,d1
                exec    SetSignal

                move.w  db_Info+CDINFO_Status(db),d0                    ; Don't transfer if not playing
                btst    #CDSTSB_PLAYING,d0
                beq     CheckPlaySig

                save    ior                                             ; Transfer the audio data
                move.l  db_ClassDReq(db),ior
                jsr     TransferSample
                restore ior

*************** Play Complete ******************************************

CheckPlaySig
                btst    #SIGB_PLAYDONE,d2                               ; Play command complete?
                beq     CheckDiskChangeSig

                clr.l   d0                                              ; Clear this signal
                move.l  #SIGF_PLAYDONE,d1
                exec    SetSignal
PlayComplete
                move.w  db_Info+CDINFO_Status(db),d0                    ; If our play was aborted by a disk change, don't
                btst    #CDSTSB_PLAYING,d0                              ;   do it again.
                beq     CheckDiskChangeSig

                jsr     ClearSubcodeInterrupt                           ; Turn off all subcode interrupts

                and.l   #-1-(INTF_PBX|INTF_OVERFLOW),CDINT2ENABLE(hb)   ; Turn off audio decode hardware
                and.l   #-1-CF_CDROM,CDCONFIG(hb)

                move.w  db_TargetAttenuation(db),d0                     ; Make sure fade has completed
                cmp.w   db_Attenuation(db),d0
                beq     1$
                move.w  d0,db_Attenuation(db)
                bsr     Attenuate
1$
                clr.w   d0                                              ; Turn off drive light
                move.w  #$0500,db_Packet(db)
                jsr     SendPacket

                move.l  db_ClassDReq(db),ior                            ; If the play errored out, report error
                tst.b   db_PlayStatus(db)
                bpl     2$
                tst.b   IO_ERROR(ior)
                bne     2$
                move.b  #CDERR_BadDataType,IO_ERROR(ior)
2$
                move.b  IO_ERROR(ior),d0                                ; Return status (if error, clear actual)
                cmp.b   #CDERR_ABORTED,d0
                bne     3$

                move.b  #CHCMD_PAUSE,db_Packet(db)                      ; Stop playing audio
                jsr     DoPacket
3$
                and.w   #-1-CDSTSF_PLAYING,db_Info+CDINFO_Status(db)    ; No longer playing audio

                clr.l   db_ClassDReq(db)                                ; Class D command done

                move.l  ior,a1                                          ; Request complete
                exec    ReplyMsg


*************** Disk Changed *******************************************

CheckDiskChangeSig
                btst    #SIGB_DISKCHANGE,d2                             ; Disk change signal?
                beq     ScanMsgPorts

                clr.l   d0                                              ; Clear this signal
                move.l  #SIGF_DISKCHANGE,d1
                exec    SetSignal

                tst.b   db_OpenState(db)                                ; Is the drawer now openned?
                bne     1$

                move.w  #0,db_Info+CDINFO_Status(db)                    ; Door now open.  Nothing about drive is true.

                move.l  #-2,db_SeekAdjustment(db)                       ; - Recalculate seek adjustment when disk is inserted
                clr.b   db_PhotoCD(db)                                  ; - PhotoCD disk not present
                jsr     ClearPrefetch                                   ; - Clear read prefetch buffer
                bra     2$
1$
                or.w    #CDSTSF_CLOSED,db_Info+CDINFO_Status(db)        ; Door is closed

                jsr     READTOC                                         ; Try and detect disk
2$
                addq.l  #1,db_ChgCount(db)                              ; Increment change count

                lea     db_ChgList(db),a0                               ; Cause disk change interrupts
                bsr     CauseInterruptList


*************** Scan Message Ports *************************************

ScanMsgPorts
                lea     db_ClassACmdPort(db),a0                         ; Check for a new non-disk-access type cmd
                exec    GetMsg
                tst.l   d0
                bne     ProcessACmd

                move.l  db_ClassDReq(db),d0                             ; Check status of current disk-access cmd
                bne     1$
                lea     db_ClassDCmdPort(db),a0
                exec    GetMsg
                move.l  d0,db_ClassDReq(db)
                bne     ProcessDCmd
1$
                move.l  #TASK_SIGS,d0                                   ; Wait for something to happen
                exec    Wait
                move.l  d0,d2

                bra     TestSignals


*************** Process Class A Command ********************************

ProcessACmd
                move.l  d0,ior                                          ; Perform the function (Class A)
                jsr     PerformIO

                move.l  ior,a1                                          ; Reply to command
                exec    ReplyMsg

                move.l  #CLASSA_SIGS,d1                                 ; Clear signals that should not be set
                clear   d0
                exec    SetSignal

                bra     TaskLoop


*************** Process Class D Command ********************************

ProcessDCmd
                move.l  d0,ior                                          ; Perform IO operation (Class D)
                jsr     PerformIO

                move.b  IO_ERROR(ior),d0                                ; Could we even start the command?
                beq     1$
                cmp.b   #CDERR_ABORTED,d0
                bne     2$
1$
                move.w  IO_COMMAND(ior),d0                              ; If multitasking command, don't reply yet
                lea     CmdOpts(pc),a1
                add.w   d0,d0
                move.w  0(a1,d0.w),d1
                btst    #OB_MULTITASK,d1
                bne     TaskLoop
2$
                clr.l   db_ClassDReq(db)                                ; Class D command done

                move.l  ior,a1                                          ; Normal command, reply and return
                exec    ReplyMsg

                move.l  #CLASSD_SIGS,d1                                 ; Clear signals that should not be set
                clear   d0
                exec    SetSignal

                bra     TaskLoop


*
******* cd.device/CD_READ ****************************************************
*
*   NAME
*       CD_READ -- read data from disk.
*
*   FUNCTION
*       Reads data from the CD into memory.  Data may be accessed on WORD
*       boundaries (you are not restricted to sector boundaries as with
*       normal disk devices).  Data lengths can also be described in WORD
*       amounts.
*
*   IO REQUEST INPUT
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_READ
*       io_Data         pointer to the buffer where the data should be put
*       io_Length       number of bytes to read, must be a WORD multiple.
*       io_Offset       byte offset from the start of the disk describing
*                       where to read data from, must be a WORD multiple.
*
*   IO REQUEST RESULT
*       io_Error  - 0 for success, or an error code as defined in
*                   <devices/cd.h>
*       io_Actual - if io_Error is 0, number of bytes actually transferred
*
*   NOTES
*       If an error occurs when attempting a CD_READ, the software will
*       retry up to 10 times before giving up on the request.  If the
*       drive is in double-speed and an error occurs, the software will
*       retry once more in double-speed, and if this fails, will retry
*       the next 9 times in single-speed.
*
*   SEE ALSO
*       CD_READXL
*
******************************************************************************

CmdRead:
                move.w  db_Info+CDINFO_Status(db),d0                            ; There must be a ROM track
                btst    #CDSTSB_CDROM,d0
                beq     Read_NoROMErr

                clr.w   db_RetryCount(db)                                       ; Initialize retry counter

                move.l  IO_OFFSET(ior),d0                                       ; These fields must be even
                or.l    IO_LENGTH(ior),d0
                btst    #0,d0
                bne     Read_BadLength

                move.l  IO_DATA(ior),d0                                         ; Must be word aligned
                btst    #0,d0
                bne     Read_BadAddress

                save    d3
                clr.l   d3                                                      ; Current drive speed
                move.w  db_Info+CDINFO_ReadSpeed(db),d3
1$
                move.l  IO_OFFSET(ior),d1                                       ; Convert offset to block
                clr.l   d0
                move.w  db_Info+CDINFO_SectorSize(db),d0
                divul.l d0,d1

                move.l  IO_LENGTH(ior),d2                                       ; Convert length to stop offset
                sub.l   #1,d2
                divul.l d0,d2
                add.l   d1,d2

                jsr     CheckPlayRange                                          ; check range of read
                bne     readinvalidlength

                move.w  #CDCMD_READ,db_CMD(db)                                  ; Read data

                lea     db_XferNode(db),a1                                      ; Create transfer entry
                move.l  a1,db_XferEntry(db)
                move.l  IO_OFFSET(ior),db_ARG1(db)                              ; - Store start block
                move.l  IO_DATA(ior),db_ARG2(db)                                ; - Start of XL node
                move.l  IO_DATA(ior),CDXL_Buffer(a1)
                move.l  IO_LENGTH(ior),CDXL_Length(a1)
                move.l  IO_LENGTH(ior),db_ARG3(db)
                move.l  #$7FFFFFF0,db_ARG5(db)
                move.l  d3,db_ARG4(db)                                          ; - Set desired drive speed
                clr.l   CDXL_IntCode(a1)
                clr.l   IO_ACTUAL(ior)                                          ; - Current actual amount transferred
                clr.l   CDXL_Actual(a1)
                lea     db_ListEnd(db),a0
                clr.l   (a0)
                move.l  a0,MLN_SUCC(a1)
                move.b  #1,db_ECC(db)                                           ; - Enable ECC of data

                jsr     DoCmd                                                   ; Do the read

                beq     6$                                                      ; Read error?
                cmp.b   #CDERR_ABORTED,d0
                beq     6$
                cmp.b   #CDERR_NoDisk,d0
                beq     6$

                jsr     ClearPrefetchIfError                                    ; Clear read prefetch buffer if an error occurred

                add.w   #1,db_RetryCount(db)                                    ; If error, retry
                cmp.w   #2,db_RetryCount(db)
                bne     4$

                move.w  #75,d3                                                  ; Switch to normal speed.  Too many errors
4$
                cmp.w   #10,db_RetryCount(db)
                bhi     6$

                exec    Forbid                                                  ; Cancel error if not aborted
                cmp.b   #CDERR_ABORTED,IO_ERROR(ior)
                beq     5$
                clr.b   IO_ERROR(ior)
5$              exec    Permit

                bra     1$
6$
                move.l  IO_LENGTH(ior),d0                                       ; Set actual
                move.l  d0,IO_ACTUAL(ior)
                restore d3
                rts

readinvalidlength:
                restore d3
                move.b  #IOERR_BADLENGTH,IO_ERROR(ior)                          ; Length invalid
                rts


******* cd.device/CD_MOTOR ***************************************************
*
*   NAME
*       CD_MOTOR -- control the on/off state of a drive motor.
*
*   FUNCTION
*       This command gives control over the spindle motor.  The motor may be
*       turned on or off.
*
*       If the motor is just being turned on, the device will delay the
*       proper amount of time to allow the drive to come up to speed.
*       Turning the motor on or off manually is not necessary, the device does
*       this automatically if it receives a request when the motor is off.
*
*   IO REQUEST INPUT
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_MOTOR
*       io_Length       the requested state of the motor, 0 to turn the motor
*                       off, and 1 to turn the motor on.
*
*   IO REQUEST RESULT
*       io_Error - 0 for success, or an error code as defined in
*                  <devices/cd.h>
*       io_Actual - if io_Error is 0 this contains the previous state of the
*                   drive motor.
*
******************************************************************************

CmdMotor:
                clr.l   IO_ACTUAL(ior)                                          ; io_Actual = motor status
                move.w  db_Info+CDINFO_Status(db),d1
                btst    #CDSTSB_SPIN,d1
                sne.b   IO_ACTUAL+3(ior)
                clr     d0

                tst.l   IO_LENGTH(ior)                                          ; Turn motor on?
                beq.s   1$

                btst    #CDSTSB_SPIN,d1                                         ; Already spinning?
                bne     3$

                move.w  #CDCMD_SPIN,db_CMD(db)                                  ; Spin it up
                bra     2$

1$:             btst    #CDSTSB_SPIN,d1                                         ; Already stopped?
                beq     3$

                move.w  #CDCMD_STOP,db_CMD(db)                                  ; Spin it down
2$:
                bsr     DoCmd                                                   ; Send command and get error
3$:
                rts

*
******* cd.device/CD_SEEK ****************************************************
*
*   NAME
*       CD_SEEK -- position laser at specified location.
*
*   FUNCTION
*       CD_SEEK moves the laser to the approximate position specified.  The
*       io_Offset field should be set to the offset to which the head is
*       to be positioned.
*
*   IO REQUEST INPUT
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_SEEK
*       io_Offset       position where head is to be moved (always LSN format)
*
*   IO REQUEST RESULT
*       io_Error - 0 for success, or an error code as defined in
*                  <devices/cd.h>
*
******************************************************************************

CmdSeek:
                save    d0                                                      ; If we are prefetching, stop it
                clr.w   d0
                jsr     StopRead
                restore d0

                move.w  #CDCMD_SEEK,db_CMD(db)                                  ; Seek

                move.l  IO_OFFSET(ior),d1                                       ; Convert offset to block
                clr.l   d0
                move.w  db_Info+CDINFO_SectorSize(db),d0
                divul.l d0,d1

                jsr     CheckSeekRange                                          ; Check range of seek
                bne     seekinvalidlength

                move.l  d1,db_ARG1(db)                                          ; Seek to here

                bsr     DoCmd                                                   ; Do it
                rts

seekinvalidlength:
                move.b  #IOERR_BADLENGTH,IO_ERROR(ior)                          ; Length invalid
                rts


*
******* cd.device/CD_CHANGENUM ***********************************************
*
*   NAME
*       CD_CHANGENUM -- return the current value of the disk-change counter.
*
*   FUNCTION
*       This command returns the current value of the disk-change counter
*       The disk change counter is incremented each time a disk is inserted
*       or removed from the cd unit.
*
*   IO REQUEST INPUT
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_CHANGENUM
*
*   IO REQUEST RESULT
*       io_Error - 0 for success, or an error code as defined in
*                  <devices/cd.h>
*       io_Actual - if io_Error is 0, this contains the current value of the
*                   disk-change counter.
*
******************************************************************************

CmdChangeNum:
                move.l  db_ChgCount(db),IO_ACTUAL(ior)                          ; Return change count
                rts


******* cd.device/CD_CHANGESTATE *********************************************
*
*   NAME
*       CD_CHANGESTATE -- check if a "valid" disk is currently in a drive.
*
*   FUNCTION
*       This command checks to see if there is a "valid" disk in a drive.
*
*   IO REQUEST INPUT
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_CHANGESTATE
*
*   IO REQUEST RESULT
*       io_Error - 0 for success, or an error code as defined in
*                  <devices/cd.h>
*       io_Actual - 0 means there is a disk while anything else indicates
*                   there is no disk.
*
*   NOTES
*       A "valid" disk is a disk with a readable table of contents.
*
******************************************************************************

CmdChangeState:
                move.w  db_Info+CDINFO_Status(db),d0                            ; Return disk insertion state
                btst    #CDSTSB_TOC,d0
                seq     IO_ACTUAL+3(ior)
                rts


******* cd.device/CD_PROTSTATUS **********************************************
*
*   NAME
*       CD_PROTSTATUS -- return whether the current disk is write-protected.
*
*   FUNCTION
*       This command is used to determine whether the current disk is
*       write-protected.  Currently, this function always returns write-
*       protected status.  If write-once CDs are made available at some point,
*       this may change.
*
*   IO REQUEST INPUT
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_PROTSTATUS
*
*   IO REQUEST RESULT
*       io_Error - 0 for success, or an error code as defined in
*                  <devices/cd.h>
*       io_Actual - 0 means the disk is NOT write-protected, while any other
*                   value indicates it is.
*
******************************************************************************

CmdProtStatus:
                move.l  #1,IO_ACTUAL(ior)                                       ; Always write protected
                rts


*****i* cd.device/CD_GETDRIVETYPE ********************************************
*
*   NAME
*       CD_GETDRIVETYPE -- return the type of disk drive for the unit that was
*                          opened.
*
*   FUNCTION
*       This command returns the type of the disk drive to the user.
*       This number will be a small integer and will come from the set of
*       DRIVEXXX constants defined in <devices/trackdisk.h>.
*
*   IO REQUEST INPUT
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_GETDRIVETYPE
*
*   IO REQUEST RESULT
*       io_Error - 0 for success, or an error code as defined in
*                  <devices/cd.h>
*       io_Actual - if io_Error is 0 this contains the drive type connected to
*                   this unit.
*
*   NOTES
*       The best way to get the drive type is with the CD_GETGEOMETRY command.
*
*   SEE ALSO
*       CD_GETGEOMETRY, <devices/cd.h>, <devices/trackdisk.h>
*
******************************************************************************

CmdGetDriveType:
                move.b  #IOERR_NOCMD,IO_ERROR(ior)                              ; Not supported on the game machine
                rts


*
*****i* cd.device/CD_GETNUMTRACKS ********************************************
*
*   NAME
*       CD_GETNUMTRACKS -- return the number of tracks for the type of disk
*                          drive for the unit that was opened.
*
*   FUNCTION
*       This command returns the number of tracks that are available
*       on the disk unit (always 1).
*
*   IO REQUEST INPUT
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_GETNUMTRACKS
*
*   IO REQUEST RESULT
*       io_Error - 0 for success, or an error code as defined in
*                  <devices/cd.h>
*       io_Actual - if io_Error is 0, this returns the number of tracks on
*                   the CD (the CD always has 1 big track).
*
*   SEE ALSO
*       CD_GETGEOMETRY
*
******************************************************************************

CmdGetNumTracks:
                move.l  #1,IO_ACTUAL(ior)                                       ; 1 Large track
                rts


******* cd.device/CD_ADDCHANGEINT ********************************************
*
*   NAME
*       CD_ADDCHANGEINT -- add a disk change software interrupt handler.
*
*   FUNCTION
*       This command lets you add a software interrupt handler to the
*       disk device that gets invoked whenever a disk insertion or removal
*       occurs.
*
*       You must pass in a properly initialized Exec Interrupt structure
*       and be prepared to deal with disk insertions/removals immediately.
*       The interrupt is generated by the exec Cause function, so you must
*       preserve A6.
*
*       To set up the handler, an Interrupt structure must be initialized.
*       This structure is supplied as the io_Data to the CD_ADDCHANGEINT
*       command.  The handler then gets linked into the handler chain and
*       gets invoked whenever a disk change happens.  You must eventually
*       remove the handler before you exit.
*
*       This command only returns when the handler is removed. That is,
*       the device holds onto the IO request until the CD_REMCHANGEINT command
*       is executed with that same IO request.  Hence, you must use SendIO()
*       with this command.
*
*   IO REQUEST INPUT
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_ADDCHANGEINT
*       io_Length       sizeof(struct Interrupt)
*       io_Data         pointer to Interrupt structure
*
*   IO REQUEST RESULT
*       io_Error - 0 for success, or an error code as defined in
*                  <devices/cd.h>
*
*   SEE ALSO
*       CD_REMCHANGEINT, <devices/cd.h>, <exec/interrupts.h>,
*       exec.library/Cause()
*
******************************************************************************

CmdAddChangeInt:
                lea     db_ChgList(db),a0                                       ; Interrupt list
AddRequest:
                exec    Forbid                                                  ; Add request to list
                move.l  ior,a1
                exec    AddHead
                exec    Permit
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)
                rts

******* cd.device/CD_REMCHANGEINT ********************************************
*
*   NAME
*       CD_REMCHANGEINT -- remove a disk change software interrupt handler.
*
*   FUNCTION
*       This command removes a disk change software interrupt added
*       by a previous use of CD_ADDCHANGEINT.
*
*   IO REQUEST INPUT
*       The same IO request used for CD_ADDCHANGEINT.
*
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_REMCHANGEINT
*       io_Length       sizeof(struct Interrupt)
*       io_Data         pointer to Interrupt structure
*
*   IO REQUEST RESULT
*       io_Error - 0 for success, or an error code as defined in
*                  <devices/cd.h>
*
*   SEE ALSO
*       CD_ADDCHANGEINT, <devices/cd.h>
*
******************************************************************************

CmdRemChangeInt:
RemoveRequest:
                exec    Forbid                                                 ; Remove interrupt from list
                move.l  ior,a1
                REMOVE
                exec    Permit
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)
                rts


******* cd.device/CD_GETGEOMETRY *********************************************
*
*   NAME
*       CD_GETGEOMETRY -- return the geometry of the drive.
*
*   FUNCTION
*       This command returns a full set of information about the
*       layout of the drive. The information is returned in the
*       DriveGeometry structure pointed to by io_Data.
*
*   IO REQUEST INPUT
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_GETGEOMETRY
*       io_Data         pointer to a DriveGeometry structure
*       io_Length       sizeof(struct DriveGeometry)
*
*   IO REQUEST RESULT
*       io_Error  - 0 for success, or an error code as defined in
*                   <devices/cd.h>
*       io_Actual - length of data transferred.
*
*   SEE ALSO
*       CD_GETNUMTRACKS, <devices/trackdisk.h>
*
******************************************************************************

CmdGetGeometry:
                move.w  db_Info+CDINFO_Status(db),d0                            ; Make sure TOC is valid first
                btst    #CDSTSB_TOC,d0
                beq     Geom_Error

                move.l  IO_DATA(ior),a1                                         ; 2k sectors
                clr.l   d0
                move.w  db_Info+CDINFO_SectorSize(db),d0
                move.l  d0,dg_SectorSize(a1)

                move.l  db_TOC+TOCS_LeadOut(db),d0                              ; Other miscellaneous reporting
                jsr     MSFtoLSNPOS
                move.l  d0,dg_TotalSectors(a1)
                move.l  #1,dg_Cylinders(a1)
                move.l  d0,dg_CylSectors(a1)
                move.l  #1,dg_Heads(a1)
                move.l  d0,dg_TrackSectors(a1)
                move.l  #MEMF_PUBLIC,dg_BufMemType(a1)

                move.b  #DG_CDROM,dg_DeviceType(a1)                             ; Removable CD-ROM disk
                move.b  #DGF_REMOVABLE,dg_Flags(a1)
                clr.w   dg_Reserved(a1)

                move.l  #dg_SIZEOF,IO_ACTUAL(ior)                               ; DriveGeometry length
                rts
Geom_Error:
                move.b  #CDERR_NoDisk,IO_ERROR(ior)                             ; Nothing to report
                move.l  #0,IO_ACTUAL(ior)
                rts


*
******* cd.device/CD_EJECT ***************************************************
*
*   NAME
*       CD_EJECT -- Open or close the CD's drive door
*
*   IO REQUEST
*       io_Command      CD_EJECT
*       io_Data         NULL
*       io_Length       requested state of drive door (0 == close, 1 == open)
*       io_Offset       0
*
*   RESULTS
*       io_Error        0 for success, or an error code as defined in
*                       <devices/cd.h>
*       io_Actual       previous state of drive door
*
*   FUNCTION
*       This command causes the CD-ROM drive's door to open or close.
*       The desired state of the drive door is placed in io_Length.  The
*       previous state of the drive door is returned in io_Actual.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************

CmdEject:
                move.b  #IOERR_NOCMD,IO_ERROR(ior)                              ; Not supported on the game machine
                rts


*
**************************************** Extended CD Commands ********************************************



******* cd.device/CD_INFO ****************************************************
*
*   NAME
*       CD_INFO -- Return information/status of device
*
*   IO REQUEST
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_INFO
*       io_Data         pointer to CDInfo structure
*       io_Length       sizeof(struct CDInfo)
*
*   RESULTS
*       io_Error        0 for success, or an error code as defined in
*                       <devices/cd.h>
*       io_Actual       length of data transferred
*
*   FUNCTION
*
*       This command returns current configurations and status of the device
*       driver.
*
*   EXAMPLE
*
*       struct CDInfo Info;
*
*       ior->io_Command = CD_INFO;               /* Retrieve drive info.    */
*       ior->io_Data    = (APTR)Info;            /* Here's where we want it */
*       ior->io_Length  = sizeof(struct CDInfo); /* Return whole structure  */
*       DoIO(ior);
*
*       if (!ior->io_Error) {                    /* Command succeeded       */
*
*           if (Info.Status & CDSTSF_PLAYING) printf("Audio is playing\n");
*           else                              printf("Audio not playing\n");
*           }
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       <devices/cd.h>
*
******************************************************************************

CmdInfo:
                lea     db_Info(db),a0                                          ; Set up copy parameters
                move.l  IO_DATA(ior),a1
                move.l  IO_LENGTH(ior),d0
                cmp.l   #CDINFO_SIZE,d0
                bhi     Info_BadLength

                move.l  d0,IO_ACTUAL(ior)                                       ; Report actual data coppied
1$
                move.b  (a0)+,(a1)+                                             ; Do the copy
                subq.l  #1,d0
                bne     1$
                rts
Info_BadLength:
                move.b  #CDERR_BADLENGTH,IO_ERROR(ior)                          ; Length too big
                rts


*
******* cd.device/CD_CONFIG **************************************************
*
*   NAME
*       CD_CONFIG -- Set drive preferences
*
*   IO REQUEST
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_CONFIG
*       io_Data         pointer to first entry of TagList
*       io_Length       0
*
*   RESULTS
*       io_Error        0 for success, or an error code as defined in
*                       <devices/cd.h>
*
*   FUNCTION
*       This command sets one or more of the configuration items.
*       The configuration items are:
*
*       TAGCD_PLAYSPEED                 Default: 75
*       TAGCD_READSPEED                 Default: 75 (do not count on this)
*       TAGCD_READXLSPEED               Default: 75
*       TAGCD_SECTORSIZE                Default: 2048
*       TAGCD_XLECC                     Default: 1 (on)
*       TAGCD_EJECTRESET                Default: can be 0 (off) or 1 (on)
*
*       The speed settings are described in the number of frames (sectors)
*       per second.  All CD-ROM drives are capable of the 75 frames/second
*       rate.  Some drives are capable of 150 frames/second, and some even
*       more.  To determine the maximum frame rate of the drive, use the
*       CD_INFO command.  Valid values for caddyless Commodore CD-ROM drives
*       are 75 and 150 (normal speed and double speed).  All other values are
*       invalid.  You should always make sure the drive is capable of the
*       configuration you are requesting by either using the CD_INFO command,
*       and/or by checking for an error condition after submitting your
*       request.
*
*       There are three different types of CD-ROM sectors.  Mode 1 sectors
*       (2048 bytes), mode 2 form 1 sectors (2048 bytes), and mode 2 form 2
*       sectors (2328 bytes).  Normally, disks are encoded in Mode 1 format.
*       Mode 2 form 1 is basically the same as mode 1; however, the mode 2
*       form 2 sector format contains no CD-ROM error correction information.
*       In order to read information encoded in this sector format, the
*       drive's sector size must be configured to 2328 byte sectors.
*
*       Error correction (ECC) of the READXL command can be turned off or
*       on with this command.  Error correction can be implemented in either
*       hardware or software (depending on the CD-ROM drive).  When ECC is
*       implemented in software, CPU usage can become bursty.  Errors rarely
*       occur on CDs unless they have numerous scratches, but when they do
*       occur, they will cause a loss of CPU bandwith.  When ECC is
*       implemented in hardware, no CPU bandwidth is lost -- in this case,
*       ECC will always be on no matter how you configure the drive because 
*       it is free.  The READXL command is used primarily for displaying
*       movie-like data.  In this case, speed is essential and data integrety
*       is not; however, if the CPU is not being utilized during an XL
*       animation there is no need to disable ECC (since the bandwidth is
*       there to be used).  The only time ECC should be disabled is when you
*       are doing intense calculations in the background of a READXL command,
*       AND your program is time-critical.  Do not forget to change this back
*       when you are done!
*       
*       To make the computer reset when a CD is ejected (for an application
*       that does not exit), use the TAGCD_EJECTRESET tag.  When possible,
*       titles should be able to exit cleanly back to Workbench.  Error
*       conditions should be monitored when doing disk I/O.
*
*   EXAMPLE
*       /* Configure ReadXL for double-speed reading and turn off ECC when */
*       /* the ReadXL command is used.                                     */
*       
*       struct TagItem ConfigList[] = {
*
*           { TAGCD_READXLSPEED, 150 },
*           { TAGCD_XLECC,       0   },
*           { TAG_END,           0   }
*           };
*
*           ior->io_Command = CD_CONFIG;
*           ior->io_Data    = (APTR)&ConfigList;
*           ior->io_Length  = 0;
*           DoIO(ior);
*
*           if (ior->io_Error) printf("Could not be configured\n");
*
*   NOTES
*       Setting the configuration will not modify the behavior of a read or
*       play command already in progress.
*
*       This can be a very dangerous command.  If for instance you set
*       TAGCD_SECTORSIZE to 2328, you will no longer be able to read any
*       data encoded at 2048 byte sectors (e.g. the file system will not be
*       able to read the disk anymore).  After you read any data stored with
*       this sector format, you should immediately configure back to the
*       original default value (even if the read failed -- the disk could
*       be removed in the middle of your read).  You should NEVER use this
*       command if you are not the exclusive owner of your disk.
*
*   BUGS
*       TAG_IGNORE, TAG_MORE, and TAG_SKIP do not work.  Do not use these.
*
*       When switching speeds from single to double (or double to single),
*       If the drive is prefetching in single-speed the data you are going
*       to use in double-speed, the drive will not switch to double-speed
*       (and visa versa).  To avoid this problem, switch to the desired speed,
*       begin reading at least 4k into the data (just read two bytes), then
*       begin reading at the beginning.  This will force the prefetch buffer
*       to clear and issue a new read command with the desired speed.
*       (Fixed in 40.24).
*
*   SEE ALSO
*       CD_INFO, <utility/tagitem.h>
*
******************************************************************************

CmdConfig:
                tst.l   IO_LENGTH(ior)                                          ; Make sure Length is zero
                bne     99$

                clr.w   d0                                                      ; Make sure TagList is valid
                jsr     SetTagList
                beq     1$
                rts
1$
                move.w  #1,d0                                                   ; TagList ok, assign values
                jsr     SetTagList
                rts
99$
                move.b  #IOERR_BADLENGTH,IO_ERROR(ior)                          ; Length invalid
                rts


*===============================================================================
*=                                                                             =
*=  SetTagList - Test tag list for validity or assign tag list values          =
*=                                                                             =
*=  in: d0.w = 0: just test taglist, 1: assign taglist                         =
*=                                                                             =
*===============================================================================

SetTagList:
                save    d2-d4/a2                                                ; Save flag
                move.w  d0,d4

                move.l  IO_DATA(ior),a0                                         ; Load up registers
                lea     ConfigTable(pc),a1
                lea     db_Info(db),a2
                clr.l   d0
                clr.w   d1
1$
                move.l  0(a0,d0.l),d2                                           ; Check each item to see if it is recognized
                beq     6$

                move.l  0(a1,d1.w),d3                                           ; Start with first item in device's list
                beq     5$

                cmp.l   d2,d3                                                   ; Are the entry Tags equal?
                bne     4$

                move.l  4(a0,d0.l),d2
                cmp.l   4(a1,d1.w),d2                                           ; Is this a valid value to assign?
                beq     3$
                cmp.l   8(a1,d1.w),d2
                beq     3$
                tst.l   12(a1,d1.w)
                beq     2$
                cmp.l   12(a1,d1.w),d2
                beq     3$
2$
                move.b  #IOERR_BADADDRESS,IO_ERROR(ior)
                bra     6$
3$
                tst.w   d4                                                      ; Is this just test mode?
                beq     4$

                move.l  d1,d3                                                   ; Assign the value
                lsr.w   #3,d3
                move.w  d2,0(a2,d3.w)
4$
                add.w   #16,d1                                                  ; Next entry in device's list
                bra     1$
5$
                clr.w   d1                                                      ; Next entry in submitted list
                add.l   #8,d0
                bra     1$
6$
                restore d2-d4/a2                                                ; All done, exit
                rts


ConfigTable:
                dc.l    TAGCD_PLAYSPEED,75,150,0                                ; Valid tags and values
                dc.l    TAGCD_READSPEED,75,150,0
                dc.l    TAGCD_READXLSPEED,75,150,0
                dc.l    TAGCD_SECTORSIZE,2048,2328,2336
                dc.l    TAGCD_XLECC,0,1,0
                dc.l    TAGCD_EJECTRESET,0,1,0
                dc.l    TAG_END,0,0,0


*
******* cd.device/CD_TOCLSN **************************************************
*
*   NAME
*       CD_TOCLSN -- Return table of contents information from CD (LSN form).
*
*   IO REQUEST
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_TOCLSN
*       io_Data         pointer to array where TOC is to be stored
*       io_Length       number of CDTOC entries to be fetched
*       io_Offset       entry to begin at (entry 0 is summary information)
*
*   RESULTS
*       io_Error        0 for success, or an error code as defined in
*                       <devices/cd.h>
*       io_Actual       Actual number of entries copied
*
*   FUNCTION
*       This command returns the table of contents of the disk currently in
*       the drive.  The table of contents consists of up to 100 entries.
*       Entry zero is summary information describing the number of tracks
*       and the total number of minutes on the disk.  Entries 1 through N
*       contain information about each individual track.  All position
*       information will be in LSN format.
*
*       The io_Data field points to an array of CDTOC structures to receive
*       the TOC data.
*
*       The io_Length field specifies the total number of entries to be
*       fetched.  The array pointed to by io_Data must be at least this many
*       elements in size.
*
*       The io_Offset field specifies the entry number at which to start
*       copying TOC data into *io_Data.
*
*       Entry zero (the summary entry) contains the following:
*
*       struct TOCSummary {
*
*           UBYTE        FirstTrack;    /* First track on disk (always 1)   */
*           UBYTE        LastTrack;     /* Last track on disk               */
*           union LSNMSF LeadOut;       /* Beginning of lead-out track      */
*           };
*
*       Track entries (entries 1 through number of tracks) contain:
*
*       struct TOCEntry {
*
*           UBYTE        CtlAdr;        /* Q-Code info                  */
*           UBYTE        Track;         /* Track number                 */
*           union LSNMSF Position;      /* Start position of this track */
*           };
*
*       CDTOC is described as a union between these two structures:
*
*       union CDTOC {
*
*           struct TOCSummary Summary;  /* First entry is summary info.  */
*           struct TOCEntry   Entry;    /* Entries 1-N are track entries */
*           };
*
*
*   EXAMPLE
*       
*       union CDTOC tocarray[100];
*
*       ior->io_Command = CD_TOCLSN;        /* Retrieve TOC information */
*       ior->io_Offset  = 0;                /* Start with summary info  */
*       ior->io_Length  = 100;              /* Max 99 tracks + summary  */
*       ior->io_Data    = (APTR)tocarray;   /* Here's where we want it  */
*       DoIO (ior);
*
*       if (!ior->io_Error) {               /* Command succeeded        */
*
*           firsttrack   = tocarray[0].Summary.FirstTrack;
*           lasttrack    = tocarray[0].Summary.LastTrack;
*           totalsectors = tocarray[0].Summary.LeadOut.LSN -
*                          tocarray[1].Entry.Position.LSN;
*           }
*
*   NOTES
*
*       In the above example, the amount of data on the disk is calculated as
*       being equal to the location of the lead-out track minus the start of
*       the first track (which is never zero).
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************

CmdTOCLSN:
                moveq.b #1,d0
                bra     CmdTOC

*
******* cd.device/CD_TOCMSF **************************************************
*
*   NAME
*       CD_TOCMSF -- Return table of contents information from CD (MSF form).
*
*   IO REQUEST
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_TOCMSF
*       io_Data         pointer to array where TOC is to be stored
*       io_Length       number of CDTOC entries to be fetched
*       io_Offset       entry to begin at (entry 0 is summary information)
*
*   RESULTS
*       io_Error        0 for success, or an error code as defined in
*                       <devices/cd.h>
*       io_Actual       Actual number of entries copied
*
*   FUNCTION
*       This command returns the table of contents of the disk currently in
*       the drive.  The table of contents consists of up to 100 entries.
*       Entry zero is summary information describing the number of tracks
*       and the total number of minutes on the disk.  Entries 1 through N
*       contain information about each individual track.  All position
*       information will be in MSF format.
*
*       The io_Data field points to an array of CDTOC structures to receive
*       the TOC data.
*
*       The io_Length field specifies the total number of entries to be
*       fetched.  The array pointed to by io_Data must be at least this many
*       elements in size.
*
*       The io_Offset field specifies the entry number at which to start
*       copying TOC data into *io_Data.
*
*       Entry zero (the summary entry) contains the following:
*
*       struct TOCSummary {
*
*           UBYTE        FirstTrack;    /* First track on disk (always 1)   */
*           UBYTE        LastTrack;     /* Last track on disk               */
*           union LSNMSF LeadOut;       /* Beginning of lead-out track      */
*           };
*
*       Track entries (entries 1 through number of tracks) contain:
*
*       struct TOCEntry {
*
*           UBYTE        CtlAdr;        /* Q-Code info                  */
*           UBYTE        Track;         /* Track number                 */
*           union LSNMSF Position;      /* Start position of this track */
*           };
*
*       CDTOC is described as a union between these two structures:
*
*       union CDTOC {
*
*           struct TOCSummary Summary;  /* First entry is summary info.  */
*           struct TOCEntry   Entry;    /* Entries 1-N are track entries */
*           };
*
*
*   EXAMPLE
*       
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************

CmdTOCMSF:
                clr.b   d0
CmdTOC:
                save    d2-d5                                                   ; Save LSN/MSF flag
                move    d0,d5

                move.w  db_Info+CDINFO_Status(db),d0                            ; There must be a ROM track
                btst    #CDSTSB_TOC,d0
                beq     tocnodisk

                lea     db_TOC(db),a0
                clr.l   IO_ACTUAL(ior)                                          ; Check bounds of parameters
                move.l  IO_DATA(ior),d0
                beq     tocinvalidaddress
                move.l  d0,a1
                move.l  IO_OFFSET(ior),d1
                cmp.l   #100,d1
                bhs     tocinvalidoffset                                        ; - A1 = Data, D1 = Start, D2 = Stop
                move.l  IO_LENGTH(ior),d2
                beq     tocinvalidlength
                bmi     tocinvalidlength
                add.l   d1,d2
                cmp.l   #100,d2
                bhi     tocinvalidlength

                mulu    #TOCEntry_SIZE,d1                                       ; Copy all requested TOC entries
                mulu    #TOCEntry_SIZE,d2
                clr.l   d3
                move.b  db_TOC+TOCS_LastTrack(db),d3
                addq.l  #1,d3
                mulu    #TOCEntry_SIZE,d3
                clr.l   d4
1$
                move.b  TOCE_CtlAdr(a0,d1.w),TOCE_CtlAdr(a1,d4.w)               ; Get CtlAdr and Track index
                move.b  TOCE_Track(a0,d1.w),TOCE_Track(a1,d4.w)

                move.l  TOCE_Position(a0,d1.w),d0                               ; Get start of track (or start of lead-out)
                tst.b   d5
                beq     2$
                bsr     MSFtoLSNPOS
2$              move.l  d0,TOCE_Position(a1,d4.w)

                add.w   #TOCEntry_SIZE,d4                                       ; Next entry (if available)
                add.w   #TOCEntry_SIZE,d1
                cmp.w   d2,d1
                bhs     3$
                cmp.w   d3,d1
                blo     1$
3$
                divu    #TOCEntry_SIZE,d4                                       ; Actual number of entries transferred
                and.l   #$0000FFFF,d4
                move.l  d4,IO_ACTUAL(ior)
tocexit:
                restore d2-d5
                rts
tocinvalidlength:
tocinvalidoffset:
                move.b  #IOERR_BADLENGTH,IO_ERROR(ior)                          ; Length invalid
                bra     tocexit
tocinvalidaddress:
                move.b  #IOERR_BADADDRESS,IO_ERROR(ior)                         ; Address invalid
                bra     tocexit
tocnodisk:
                move.b  #CDERR_NoDisk,IO_ERROR(ior)                             ; TOC not valid
                bra     tocexit


*
******* cd.device/CD_READXL **************************************************
*
*   NAME
*       CD_READXL -- Read from CD-ROM into memory via transfer list.
*
*   IO REQUEST
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_READXL
*       io_Data         pointer to transfer list (i.e. struct List *).
*       io_Length       maximum transfer length (WORD multiple) or 0.
*       io_Offset       byte offset from the start of the disk describing
*                       where to read data from, must be a WORD multiple.
*
*   RESULTS
*       io_Error        0 for success, or an error code as described in
*                       <devices/cd.h>
*       io_Actual       if io_Error is 0, number of bytes actually transferred
*
*   FUNCTION
*       This command starts reading data off the disk at the specified
*       location and deposits it into memory according to the nodes in a
*       transfer list.  The pointer to the list of transfer nodes is placed
*       in io_Data.  If you have a non-circular transfer list, simply set
*       io_Length to 0 (0 is special and means ignore io_Length) -- your
*       transfer will end when your transfer list has been exhausted.  If you
*       have a circular transfer list, the list will never end.  In this case,
*       the transfer will terminate when io_Length bytes have been
*       transferred.
*
*       The fields in the CDXL node structure are:
*
*       struct  CDXL {
*
*           struct MinNode  Node;         /* double linkage                */
*           char           *Buffer;       /* data destination              */
*           LONG            Length;       /* must be even # bytes          */
*           LONG            Actual;       /* bytes transferred             */
*           APTR            IntData;      /* interrupt server data segment */
*           VOID            (*IntCode)(); /* interrupt server code entry   */
*           };
*
*       The philosophy here is that you set up the buffers you want filled,
*       create CDXL nodes describing the locations and sizes of these
*       buffers, link all the nodes together in the order that you'd like
*       (even make a circular list for animations), and execute the command.
*       The data will be streamed into the appropriate buffers until the
*       list has been exhausted, an entry with a Length of zero is
*       encountered, io_Length bytes have been transferred (if io_Length is
*       non-zero), or the command is aborted with AbortIO().
*
*       If you fill in the (*IntCode)() field with a pointer to an interrupt
*       routine, your routine will be called when the transfer for the node
*       is complete.  Your code will be called before the driver proceeds to
*       the next node.  The interrupt should follow the same rules as standard
*       interrupts (see AddIntServer of Exec autodocs).  Register A2 will
*       contain a pointer to the node just completed.  You may manipulate the
*       list from within the interrupt. Your code must be brief (this is an
*       interrupt).  When returning from this interrupt, D0 should be cleared
*       and an RTS instruction should be used to return.
*
*       Servers are called with the following register conventions:
*
*           D0 - scratch
*           D1 - scratch
*
*           A0 - scratch
*           A1 - server is_Data pointer (scratch)
*           A2 - pointer to CDXL node just completed
*
*           A5 - jump vector register (scratch)
*
*           all other registers must be preserved
*
*   EXAMPLE
*
*   NOTES
*       Try to make sure that small buffers are not overused.  Each time
*       a node is completed, an interrupt is generated.  If you find that
*       your computer is acting sluggish, or the CD_READXL command is
*       aborting, you are probably generating too many interrupts.  It is
*       not efficient to have more than a few of these interrupts generated
*       within a vertical blank.
*
*       Unlike the READ command, the READXL command will not retry a sector
*       if there is an error.  Since the READXL command's purpose is primarily
*       for animations, data streaming is considered more important than the
*       data itself.  An error will be returned in io_Error if a data error
*       did occur.  This command will never drop to a lower speed in the event
*       of an error.
*
*   BUGS
*
*   SEE ALSO
*       CMD_READ, CD_SEEK, Autodocs - AddIntServer
*
******************************************************************************

CmdReadXL:
                move.w  db_Info+CDINFO_Status(db),d0                            ; There must be a ROM track
                btst    #CDSTSB_CDROM,d0
                beq     Read_NoROMErr

                move.l  IO_OFFSET(ior),d0                                       ; These fields must be even
                btst    #0,d0
                bne     Read_BadLength
                move.l  IO_LENGTH(ior),d0
                btst    #0,d0
                bne     Read_BadLength

                move.l  IO_OFFSET(ior),d1                                       ; Convert offset to block for testing

                clr.l   d0
                move.w  db_Info+CDINFO_SectorSize(db),d0
                divul.l d0,d1

                jsr     CheckSeekRange                                          ; Check range of seek (not play)
                bne     Read_BadLength

                jsr     ClearPrefetchIfError                                    ; Clear read prefetch buffer if an error occurred

                move.l  IO_OFFSET(ior),db_ARG1(db)                              ; Store start byte

                clr.l   IO_ACTUAL(ior)                                          ; Current actual transfer length

                move.w  #CDCMD_READ,db_CMD(db)                                  ; Read data

                move.l  IO_DATA(ior),a1                                         ; Empty list?                                         
                move.l  LH_HEAD(a1),a1
                tst.l   LN_SUCC(a1)
                beq     Read_BadLength
                                                                                ; Start custom transfer list
                move.l  a1,db_XferEntry(db)
                move.l  IO_OFFSET(ior),db_ARG1(db)
                move.l  CDXL_Buffer(a1),db_ARG2(db)
                move.l  CDXL_Length(a1),db_ARG3(db)
                clr.l   d0                                                      ; - Set desired drive speed
                move.w  db_Info+CDINFO_ReadXLSpeed(db),d0
                move.l  d0,db_ARG4(db)
                move.l  #$7FFFFFF0,db_ARG5(db)
                move.l  IO_LENGTH(ior),d0
                beq     1$
                move.l  d0,db_ARG5(db)
1$              clr.l   IO_ACTUAL(ior)
                clr.l   CDXL_Actual(a1)
                move.w  db_Info+CDINFO_XLECC(db),d0                             ; Configure ECC of data
                move.b  d0,db_ECC(db)

                jsr     DoCmd                                                   ; Do the read
                rts

Read_NoROMErr:
                moveq   #CDERR_BadDataType,d0                                   ; No CD-ROM
                bra.s   Read_ReadErr
Read_BadLength:
                moveq   #CDERR_BADLENGTH,d0                                     ; Bad arguments
                bra.s   Read_ReadErr
Read_BadAddress:
                moveq   #CDERR_BADADDRESS,d0                                    ; Data not word aligned
Read_ReadErr:
                move.b  d0,IO_ERROR(ior)                                        ; Return error
                rts


*
******* cd.device/CD_PLAYTRACK ***********************************************
*
*   NAME
*       CD_PLAYTRACK -- Play one or more tracks of CD audio.
*
*   IO REQUEST
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_PLAYTRACK
*       io_Data         NULL
*       io_Length       number of tracks to play
*       io_Offset       start playing at beginning of this track
*
*   RESULTS
*       io_Error        0 for success, or an error code as defined in
*                       <devices/cd.h>
*   FUNCTION
*       This command causes the drive to play the specified audio track(s).
*       The command will return when the audio has completed.
*
*       io_Offset specifies the track number (starting from 1).
*
*       io_Length specifies the number of tracks to play (0 is invalid).  
*
*   EXAMPLE
*
*       ior->io_Command = CD_PLAYTRACK;    /* Play audio tracks     */
*       ior->io_Offset  = STARTTRACK;      /* Start with this track */
*       ior->io_Length  = 3;               /* Play three tracks     */
*       DoIO(ior);
*
*   NOTES
*
*       PLAY commands are asynchronous with many other CD commands.
*       Using a separate I/O request, other commands can be sent to the device
*       that can change the behavior of the PLAY command.
*
*   BUGS
*
*   SEE ALSO
*       CD_PLAYMSF, CD_PLAYLSN, CD_PAUSE, CD_SEARCH, CD_ATTENUATE
*
******************************************************************************

CmdPlayTrack:
                save    d0                                                      ; If we are prefetching, stop it
                clr.w   d0
                jsr     StopRead
                restore d0

                save    d2                                                      ; Check validity of io_Offset
                move.l  IO_OFFSET(ior),d1
                cmp.b   db_TOC+TOCS_FirstTrack(db),d1
                blo     trackinvalidoffset
                cmp.b   db_TOC+TOCS_LastTrack(db),d1
                bhi     trackinvalidoffset

                move.l  IO_LENGTH(ior),d0                                       ; Check validity of io_Length
                bmi     trackinvalidlength
                move.l  d1,d2
                add.l   d0,d2
                sub.w   #1,d2
                cmp.b   db_TOC+TOCS_LastTrack(db),d2
                bhi     trackinvalidlength
                add.w   #1,d2

                lea     db_TOC(db),a0                                           ; Prepare to access TOC
                mulu    #TOCEntry_SIZE,d1
                mulu    #TOCEntry_SIZE,d2

                move.l  TOCE_Position(a0,d1.w),d0                               ; Start at this track
                jsr     MSFtoLSNPOS
                move.l  d0,d1

                move.l  TOCE_Position(a0,d2.w),d0                               ; Stop before this point
                jsr     MSFtoLSNPOS
                move.l  d0,d2

                move.w  #CDCMD_PLAY,db_CMD(db)                                  ; Play
                move.l  d1,db_ARG1(db)
                move.l  d2,db_ARG2(db)

                bsr     DoCmd                                                   ; Start play
trackexit:
                restore d2
                rts

trackinvalidlength:
trackinvalidoffset:
                move.b  #IOERR_BADLENGTH,IO_ERROR(ior)                          ; Length invalid
                bra     trackexit

*
******* cd.device/CD_PLAYLSN *************************************************
*
*   NAME
*        CD_PLAYLSN -- Play a selected portion of CD audio (LSN form).
*
*   IO REQUEST
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_PLAYLSN
*       io_Data         NULL
*       io_Length       length of play
*       io_Offset       starting position
*
*   RESULTS
*       io_Error        0 for success, or an error code as defined in
*                       <devices/cd.h>
*
*   FUNCTION
*       This command causes the drive to start playing CD audio from the
*       specified position until the specified length has passed.
*
*       io_Offset specifies the starting position.  io_Length contains
*       the amount of time to play.  All data is specified in LSN format.
*
*       A DoIO() will not return until the requested number of sectors
*       have been played.  A SendIO() will return as soon as the PLAY
*       has been started.  At this time other commands can be sent (like
*       CD_PAUSE).  To stop a play before the specified length has been
*       reached, use AbortIO().
*
*   EXAMPLE
*       /* Play two minutes, ten seconds of audio starting at 20 minutes, */
*       /* 58 seconds, and 10 frames.                                     */
*
*       ior->io_Command = CD_PLAYLSN;   /* Play CD audio           */
*       ior->io_Offset  = 94360;        /* 20*(60*75) + 58*75 + 10 */
*       ior->io_Length  = 9750;         /* 02*(60*75) + 10*75 + 00 */
*       DoIO (ior);
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       CD_PLAYTRACK, CD_PAUSE, CD_SEARCH, CD_ATTENUATE
*
******************************************************************************

CmdPlayLSN:
                move.b  #1,d0
                bra     CmdPlay

*
******* cd.device/CD_PLAYMSF *************************************************
*
*   NAME
*        CD_PLAYMSF -- Play a selected portion of CD audio (MSF form).
*
*   IO REQUEST
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_PLAYMSF
*       io_Data         NULL
*       io_Length       length of play
*       io_Offset       starting position
*
*   RESULTS
*       io_Error        0 for success, or an error code as defined in
*                       <devices/cd.h>
*
*   FUNCTION
*       This command causes the drive to start playing CD audio from the
*       specified position until the specified length has passed.
*
*       io_Offset specifies the starting position.  io_Length contains
*       the amount of time to play.  All data is specified in MSF format.
*
*       A DoIO() will not return until the requested number of sectors
*       have been played.  A SendIO() will return as soon as the PLAY
*       has been started.  At this time other commands can be sent (like
*       CD_PAUSE).  To stop a play before the specified length has been
*       reached, use AbortIO().
*
*   EXAMPLE
*       /* Play two minutes, ten seconds of audio starting at 20 minutes, */
*       /* 58 seconds, and 10 frames.                                     */
*
*       ior->io_Command = CD_PLAYMSF;   /* Play CD audio          */
*       ior->io_Offset  = 0x00143A0A;   /* $14=20, $3A=58, $0A=10 */
*       ior->io_Length  = 0x00020A00;   /* $02=02, $0A=10, $00=00 */
*       DoIO (ior);
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       CD_PLAYTRACK, CD_PAUSE, CD_SEARCH, CD_ATTENUATE
*
******************************************************************************

CmdPlayMSF:
                clr.b   d0
CmdPlay:
                save    d0                                                      ; If we are prefetching, stop it
                clr.w   d0
                jsr     StopRead
                restore d0

                save    d2/d3                                                   ; Save LSN/MSF flag
                move.b  d0,d3

                move.l  IO_OFFSET(ior),d1                                       ; Retrieve start and stop positions
                move.l  IO_LENGTH(ior),d2

                tst.b   d3                                                      ; Convert LSN to MSF?
                bne     1$
                move.l  d1,d0                                                   ; - Convert Start
                bsr     MSFtoLSNPOS
                move.l  d0,d1
                move.l  d2,d0
                bsr     MSFtoLSN
                move.l  d0,d2
1$              add.l   d1,d2

                jsr     CheckPlayRange                                          ; Check play range
                bne     playinvalidlength

                move.w  #CDCMD_PLAY,db_CMD(db)                                  ; Play
                move.l  d1,db_ARG1(db)
                move.l  d2,db_ARG2(db)

                bsr     DoCmd                                                   ; Start play
playexit:
                restore d2/d3
                rts

playinvalidlength:
                move.b  #IOERR_BADLENGTH,IO_ERROR(ior)                          ; Length invalid
                bra     playexit

*
******* cd.device/CD_SAMPLETRACK *********************************************
*
*   NAME
*       CD_SAMPLETRACK -- Sample one or more tracks of CD audio.
*
*   IO REQUEST
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_SAMPLETRACK
*       io_Data         NULL
*       io_Length       number of tracks to sample
*       io_Offset       start sampling at beginning of this track
*
*   RESULTS
*       io_Error        0 for success, or an error code as defined in
*                       <devices/cd.h>
*   FUNCTION
*       This command causes the drive to sample the specified audio track(s).
*       The command will return when the audio has completed.
*
*       io_Offset specifies the track number (starting from 1).
*
*       io_Length specifies the number of tracks to sample (0 is invalid).  
*
*   EXAMPLE
*
*       ior->io_Command = CD_SAMPLETRACK;  /* Sample audio tracks   */
*       ior->io_Offset  = STARTTRACK;      /* Start with this track */
*       ior->io_Length  = 3;               /* Sample three tracks   */
*       DoIO(ior);
*
*   NOTES
*
*       SAMPLE commands are asynchronous with many other CD commands.
*       Using a separate I/O request, other commands can be sent to the device
*       that can change the behavior of the SAMPLE command.
*
*   BUGS
*
*   SEE ALSO
*       CD_SAMPLEMSF, CD_SAMPLELSN, CD_PAUSE, CD_SEARCH, CD_ATTENUATE
*
******************************************************************************

CmdSampleTrack:
                save    d0                                                      ; If we are prefetching, stop it
                clr.w   d0
                jsr     StopRead
                restore d0

                save    d2                                                      ; Check validity of io_Offset
                move.l  IO_OFFSET(ior),d1
                cmp.b   db_TOC+TOCS_FirstTrack(db),d1
                blo     strackinvalidoffset
                cmp.b   db_TOC+TOCS_LastTrack(db),d1
                bhi     strackinvalidoffset

                move.l  IO_DATA(ior),d0                                         ; Must be word aligned (Set address)
                btst    #0,d0
                bne     sample_BadAddress
                move.l  d0,db_ARG3(db)

                move.l  IO_LENGTH(ior),d0                                       ; Check validity of io_Length
                bmi     strackinvalidlength
                move.l  d1,d2
                add.l   d0,d2
                sub.w   #1,d2
                cmp.b   db_TOC+TOCS_LastTrack(db),d2
                bhi     strackinvalidlength
                add.w   #1,d2

                lea     db_TOC(db),a0                                           ; Prepare to access TOC
                mulu    #TOCEntry_SIZE,d1
                mulu    #TOCEntry_SIZE,d2

                move.l  TOCE_Position(a0,d1.w),d0                               ; Start at this track
                jsr     MSFtoLSNPOS
                move.l  d0,d1

                move.l  TOCE_Position(a0,d2.w),d0                               ; Stop before this point
                jsr     MSFtoLSNPOS
                move.l  d0,d2

                move.w  #CDCMD_SAMPLE,db_CMD(db)                                ; Sample
                move.l  d1,db_ARG1(db)
                move.l  d2,db_ARG2(db)
                move.l  d2,d0
                sub.l   d1,d0
                add.l   #1,d0
                mulu    #2352,d0
                move.l  d0,db_ARG4(db)

                bsr     DoCmd                                                   ; Start sample
strackexit:
                restore d2
                rts

strackinvalidlength:
strackinvalidoffset:
                move.b  #IOERR_BADLENGTH,IO_ERROR(ior)                          ; Length invalid
                bra     strackexit

*
******* cd.device/CD_SAMPLELSN ***********************************************
*
*   NAME
*        CD_SAMPLELSN -- Sample a selected portion of CD audio (LSN form).
*
*   IO REQUEST
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_SAMPLELSN
*       io_Data         NULL
*       io_Length       length of sample (in sectors)
*       io_Offset       starting position (sector)
*
*   RESULTS
*       io_Error        0 for success, or an error code as defined in
*                       <devices/cd.h>
*
*   FUNCTION
*       This command causes the drive to start sampling CD audio from the
*       specified position until the specified length has passed.
*
*       io_Offset specifies the starting position.  io_Length contains
*       the amount of time to sample.  All data is specified in LSN format.
*
*       A DoIO() will not return until the requested number of sectors
*       have been sampled.  A SendIO() will return as soon as the SAMPLE
*       has been started.  At this time other commands can be sent (like
*       CD_PAUSE).  To stop a sample before the specified length has been
*       reached, use AbortIO().  If you pause a sample, the data will be
*       corrupted.
*
*   EXAMPLE
*       /* Sample four seconds of audio starting at 20 minutes,    */
*       /* 58 seconds, and 10 frames.                              */
*
*       ior->io_Command = CD_SAMPLELSN; /* Sample CD audio         */
*       ior->io_Offset  = 94360;        /* 20*(60*75) + 58*75 + 10 */
*       ior->io_Length  = 300;          /* 00*(60*75) + 04*75 + 00 */
*       ior->io_Data    = Buffer;       /* Buffer to store data    */
*       DoIO (ior);
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       CD_SAMPLETRACK, CD_PAUSE, CD_SEARCH, CD_ATTENUATE
*
******************************************************************************

CmdSampleLSN:
                move.b  #1,d0
                bra     CmdSample

*
******* cd.device/CD_SAMPLEMSF ***********************************************
*
*   NAME
*        CD_SAMPLEMSF -- Sample a selected portion of CD audio (MSF form).
*
*   IO REQUEST
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_SAMPLEMSF
*       io_Data         NULL
*       io_Length       length of sample (MSF time)
*       io_Offset       starting position (MSF length in frames)
*
*   RESULTS
*       io_Error        0 for success, or an error code as defined in
*                       <devices/cd.h>
*
*   FUNCTION
*       This command causes the drive to start sampling CD audio from the
*       specified position until the specified length has passed.
*
*       io_Offset specifies the starting position.  io_Length contains
*       the amount of time to sample.  All data is specified in MSF format.
*
*       A DoIO() will not return until the requested number of sectors
*       have been sampled.  A SendIO() will return as soon as the SAMPLE
*       has been started.  At this time other commands can be sent (like
*       CD_PAUSE).  To stop a sample before the specified length has been
*       reached, use AbortIO().  If you pause a sample, the data will be
*       corrupted.
*
*   EXAMPLE
*       /* Sample four seconds of audio starting at 20 minutes,   */
*       /* 58 seconds, and 10 frames.                             */
*
*       ior->io_Command = CD_SAMPLEMSF; /* Sample CD audio        */
*       ior->io_Offset  = 0x00143A0A;   /* $14=20, $3A=58, $0A=10 */
*       ior->io_Length  = 0x00000400;   /* $02=02, $0A=10, $00=00 */
*       ior->io_Data    = Buffer;       /* Buffer to store data   */
*       DoIO (ior);
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       CD_SAMPLETRACK, CD_PAUSE, CD_SEARCH, CD_ATTENUATE
*
******************************************************************************

CmdSampleMSF:
                clr.b   d0
CmdSample:
                save    d0                                                      ; If we are prefetching, stop it
                clr.w   d0
                jsr     StopRead
                restore d0

                save    d2/d3                                                   ; Save LSN/MSF flag
                move.b  d0,d3

                move.l  IO_OFFSET(ior),d1                                       ; Retrieve start and stop positions
                move.l  IO_LENGTH(ior),d2

                move.l  IO_DATA(ior),d0                                         ; Must be word aligned (Set address)
                btst    #0,d0
                bne     sample_BadAddress
                move.l  d0,db_ARG3(db)

                tst.b   d3                                                      ; Convert LSN to MSF?
                bne     1$
                move.l  d1,d0                                                   ; - Convert Start
                bsr     MSFtoLSNPOS
                move.l  d0,d1
                move.l  d2,d0
                bsr     MSFtoLSN
                move.l  d0,d2
1$              add.l   d1,d2

                jsr     CheckPlayRange                                          ; Check sample range
                bne     sampleinvalidlength

                move.w  #CDCMD_SAMPLE,db_CMD(db)                                ; Sample
                move.l  d1,db_ARG1(db)
                move.l  d2,db_ARG2(db)
                move.l  d2,d0
                sub.l   d1,d0
                mulu    #2352,d0
                move.l  d0,db_ARG4(db)

                lea     db_XferNode(db),a1                                      ; Create transfer entry
                move.l  a1,db_XferEntry(db)
                move.l  IO_DATA(ior),CDXL_Buffer(a1)
                move.l  db_ARG4(db),CDXL_Length(a1)
                clr.l   CDXL_IntCode(a1)
                clr.l   IO_ACTUAL(ior)                                          ; - Current actual amount transferred
                clr.l   CDXL_Actual(a1)
                lea     db_ListEnd(db),a0
                clr.l   (a0)
                move.l  a0,MLN_SUCC(a1)

                bsr     DoCmd                                                   ; Start sample
sampleexit:
                restore d2/d3
                rts

sample_BadAddress:
                move.b  #CDERR_BADADDRESS,IO_ERROR(ior)                         ; Data not word aligned
                bra     sampleexit

sampleinvalidlength:
                move.b  #IOERR_BADLENGTH,IO_ERROR(ior)                          ; Length invalid
                bra     sampleexit



*
******* cd.device/CD_SAMPLEXLLSN *********************************************
*
*   NAME
*        CD_SAMPLEXLLSN -- Sample a selected portion of CD audio (LSN form).
*
*   IO REQUEST
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_SAMPLEXLLSN
*       io_Data         pointer to transfer list (i.e. struct List *).
*       io_Length       length of sample (in sectors)
*       io_Offset       starting position (sector)
*
*   RESULTS
*       io_Error        0 for success, or an error code as described in
*                       <devices/cd.h>
*       io_Actual       if io_Error is 0, number of bytes actually transferred
*
*   FUNCTION
*       This command causes the drive to start sampling CD audio from the
*       specified position until the specified length has passed.
*
*       io_Offset specifies the starting position.  io_Length contains
*       the amount of time to sample.  All data is specified in MSF format.
*
*       A DoIO() will not return until the requested number of sectors
*       have been sampled.  A SendIO() will return as soon as the SAMPLE
*       has been started.  At this time other commands can be sent (like
*       CD_PAUSE).  To stop a sample before the specified length has been
*       reached, use AbortIO().  If you pause a sample, the data will be
*       corrupted.
*
*       This command starts reading data off the disk at the specified
*       location and deposits it into memory according to the nodes in a
*       transfer list.  The pointer to the list of transfer nodes is placed
*       in io_Data.  The sample will continue playing audio until io_Length
*       has been reached (even if the transfer list has completed).
*
*       The fields in the CDXL node structure are:
*
*       struct  CDXL {
*
*           struct MinNode  Node;         /* double linkage                */
*           char           *Buffer;       /* data destination              */
*           LONG            Length;       /* must be even # bytes          */
*           LONG            Actual;       /* bytes transferred             */
*           APTR            IntData;      /* interrupt server data segment */
*           VOID            (*IntCode)(); /* interrupt server code entry   */
*           };
*
*       If you fill in the (*IntCode)() field with a pointer to an interrupt
*       routine, your routine will be called when the transfer for the node
*       is complete.  Your code will be called before the driver proceeds to
*       the next node.  The interrupt should follow the same rules as standard
*       interrupts (see AddIntServer of Exec autodocs).  Register A2 will
*       contain a pointer to the node just completed.  You may manipulate the
*       list from within the interrupt. Your code must be brief (this is an
*       interrupt).  When returning from this interrupt, D0 should be cleared
*       and an RTS instruction should be used to return.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       CD_READXL, CD_SAMPLEXLMSF, CD_PAUSE, CD_SEARCH, CD_ATTENUATE
*
******************************************************************************

CmdSampleXLLSN:
                move.b  #1,d0
                bra     CmdSampleXL

*
******* cd.device/CD_SAMPLEXLMSF *********************************************
*
*   NAME
*        CD_SAMPLEXLMSF -- Sample a selected portion of CD audio (MSF form).
*
*   IO REQUEST
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_SAMPLEXLMSF
*       io_Data         pointer to transfer list (i.e. struct List *).
*       io_Length       length of sample (MSF length in frames)
*       io_Offset       starting position (MSF time)
*
*   RESULTS
*       io_Error        0 for success, or an error code as defined in
*                       <devices/cd.h>
*       io_Actual       if io_Error is 0, number of bytes actually transferred
*
*   FUNCTION
*       This command causes the drive to start sampling CD audio from the
*       specified position until the specified length has passed.
*
*       io_Offset specifies the starting position.  io_Length contains
*       the amount of time to sample.  All data is specified in MSF format.
*
*       A DoIO() will not return until the requested number of sectors
*       have been sampled.  A SendIO() will return as soon as the SAMPLE
*       has been started.  At this time other commands can be sent (like
*       CD_PAUSE).  To stop a sample before the specified length has been
*       reached, use AbortIO().  If you pause a sample, the data will be
*       corrupted.
*
*       This command starts reading data off the disk at the specified
*       location and deposits it into memory according to the nodes in a
*       transfer list.  The pointer to the list of transfer nodes is placed
*       in io_Data.  The sample will continue playing audio until io_Length
*       has been reached (even if the transfer list has completed).
*
*       The fields in the CDXL node structure are:
*
*       struct  CDXL {
*
*           struct MinNode  Node;         /* double linkage                */
*           char           *Buffer;       /* data destination              */
*           LONG            Length;       /* must be even # bytes          */
*           LONG            Actual;       /* bytes transferred             */
*           APTR            IntData;      /* interrupt server data segment */
*           VOID            (*IntCode)(); /* interrupt server code entry   */
*           };
*
*       If you fill in the (*IntCode)() field with a pointer to an interrupt
*       routine, your routine will be called when the transfer for the node
*       is complete.  Your code will be called before the driver proceeds to
*       the next node.  The interrupt should follow the same rules as standard
*       interrupts (see AddIntServer of Exec autodocs).  Register A2 will
*       contain a pointer to the node just completed.  You may manipulate the
*       list from within the interrupt. Your code must be brief (this is an
*       interrupt).  When returning from this interrupt, D0 should be cleared
*       and an RTS instruction should be used to return.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       CD_READXL, CD_SAMPLEXLLSN, CD_PAUSE, CD_SEARCH, CD_ATTENUATE
*
******************************************************************************

CmdSampleXLMSF:
                clr.b   d0
CmdSampleXL:
                save    d0                                                      ; If we are prefetching, stop it
                clr.w   d0
                jsr     StopRead
                restore d0

                save    d2/d3                                                   ; Save LSN/MSF flag
                move.b  d0,d3

                move.l  IO_OFFSET(ior),d1                                       ; Retrieve start and stop positions
                move.l  IO_LENGTH(ior),d2

                tst.b   d3                                                      ; Convert LSN to MSF?
                bne     1$
                move.l  d1,d0                                                   ; - Convert Start
                bsr     MSFtoLSNPOS
                move.l  d0,d1
                move.l  d2,d0
                bsr     MSFtoLSN
                move.l  d0,d2
1$              add.l   d1,d2

                jsr     CheckPlayRange                                          ; Check sample range
                bne     sampleXLinvalidlength

                move.w  #CDCMD_SAMPLE,db_CMD(db)                                ; Sample
                move.l  d1,db_ARG1(db)
                move.l  d2,db_ARG2(db)

                move.l  IO_DATA(ior),a1                                         ; Empty list?                                         
                move.l  LH_HEAD(a1),a1
                tst.l   LN_SUCC(a1)
                beq     sampleXLinvalidlength
                                                                                ; Start custom transfer list
                move.l  a1,db_XferEntry(db)
                move.l  CDXL_Buffer(a1),db_ARG3(db)
                move.l  CDXL_Length(a1),db_ARG4(db)
                clr.l   IO_ACTUAL(ior)
                clr.l   CDXL_Actual(a1)

                bsr     DoCmd                                                   ; Start sample
sampleXLexit:
                restore d2/d3
                rts

sampleXLinvalidlength:
                move.b  #IOERR_BADLENGTH,IO_ERROR(ior)                          ; Length invalid
                bra     sampleexit


*
******* cd.device/CD_PAUSE ***************************************************
*
*   NAME
*       CD_PAUSE -- Pause or unPause play command.
*
*   IO REQUEST
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_PAUSE
*       io_Data         NULL
*       io_Length       pausemode : 1 = pause play; 0 = do not pause play;
*       io_Offset       0
*
*   RESULTS
*       io_Actual - if io_Error is 0, this contains the previous pause state.
*
*   FUNCTION
*       This command will place the CD in, or take the CD out of pause mode.
*       The desired pause state is placed in io_Length.  This command only
*       effects play commands.  When the audio is playing and the pausemode
*       is set, this command will immediately pause the audio output
*       suspending the play command until the play is unpaused.  When audio
*       is not playing and the pausemode is set, this command will set the
*       pause mode (having no immediate effect).  When a play command is
*       submitted, the laser will seek to the appropriate position and pause
*       at that spot.  The play command will be suspended until the play is
*       unpaused (or the play is aborted).
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************

CmdPause:
                clr.l   IO_ACTUAL(ior)                                          ; Calculate IO_ACTUAL
                move.w  db_Info+CDINFO_Status(db),d0
                btst    #CDSTSB_PAUSED,d0
                sne.b   IO_ACTUAL+3(ior)

                tst.l   IO_LENGTH(ior)                                          ; Pause or resume?
                bne.s   1$

                tst.l   IO_ACTUAL(ior)                                          ; Already resumed?
                beq     2$

                move.w  #CDCMD_RESUME,db_CMD(db)                                ; Resume play if we were playing
                bsr     DoCmd
                rts
1$
                tst.l   IO_ACTUAL(ior)                                          ; Already paused?
                bne     2$

                move.w  #CDCMD_PAUSE,db_CMD(db)                                 ; Pause play if we are playing
                bsr     DoCmd
2$              rts


*
******* cd.device/CD_SEARCH **************************************************
*
*   NAME
*       CD_SEARCH -- configure the mode in which PLAY commands play
*
*   IO REQUEST
*       io_Command      CD_SEARCH
*       io_Data         NULL
*       io_Length       searchmode
*       io_Offset       0
*
*   RESULTS
*       io_Actual - if io_Error is 0, this contains the previous search mode.
*
*   FUNCTION
*       This command causes a play command to play in fast-forward,
*       fast-reverse, or normal play mode.  These modes are defined as:
*
*       CDMODE_NORMAL   0   Normal play (current speed setting)
*       CDMODE_FFWD     1   Play in fast forward mode  
*       CDMODE_FREV     2   Play in fast reverse mode
*
*       The search mode can be set before the play command is sent, or during
*       a play.  If CD_SEARCH is sent before a play command is sent, the
*       mode is set and the command immediately returns.  If the mode is set
*       to REV mode, when the play command is sent the play will begin at the
*       requested end position and work backwards to the start position.
*
*       If CD_SEARCH is sent during a play, the play will automatically
*       switch to the desired mode and continue playing until the original
*       play command is completed.  If REV mode is set and the beginning of
*       the play is encountered before switching back to forward play, the
*       play command will terminate with no error.
*
*   EXAMPLE
*       /* Search in fast forward mode. */
*       ior->io_Command = CD_SEARCH;
*       ior->io_Data    = NULL;
*       ior->io_Offset  = 0;
*       ior->io_Length  = CDMODE_FFWD;
*       DoIO(ior);
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************

CmdSearch:
                cmp.l   #3,IO_LENGTH(ior)                                       ; Is the requested mode valid?
                blo     1$
                move.b  #IOERR_BADLENGTH,IO_ERROR(ior)
                rts
1$
                move.l  #CDMODE_NORMAL,d1                                       ; Report previous search mode
                move.w  db_Info+CDINFO_Status(db),d0
                btst    #CDSTSB_SEARCH,d0
                beq     2$
                move.l  #CDMODE_FFWD,d1
                btst    #CDSTSB_DIRECTION,d0
                beq     2$
                move.l  #CDMODE_FREV,d1
2$              move.l  d1,IO_ACTUAL(ior)

                move.w  #CDCMD_SETPLAYMODE,db_CMD(db)                           ; Set PlayMode...
                move.l  IO_LENGTH(ior),db_ARG1(db)

                bsr     DoCmd                                                   ; ... please.
                rts


*
******* cd.device/CD_QCODELSN ************************************************
*
*   NAME
*       CD_QCODELSN -- Report current disk position.
*
*   IO REQUEST
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_QCODELSN
*       io_Data         pointer to QCode structure
*       io_Length       0 - MUST be zero (for future compatability)
*
*   RESULTS
*       io_Error        0 for success, or an error code as defined in
*                       <devices/cd.h>
*
*   FUNCTION
*       This command reports current subcode Q channel time information.  This
*       command only returns data when CD Audio is playing (or paused).  At
*       any other time, an error is returned.  The Q-Code packet consists of:
*
*       struct QCode {
*
*           UBYTE        CtlAdr;        /* Data type / QCode type           */
*           UBYTE        Track;         /* Track number                     */
*           UBYTE        Index;         /* Track subindex number            */
*           UBYTE        Zero;          /* The "Zero" byte of Q-Code packet */
*           union LSNMSF TrackPosition; /* Position from start of track     */
*           union LSNMSF DiskPosition;  /* Position from start of disk      */
*           };
*
*   EXAMPLE
*
*       struct QCode qcode;
*
*       ior->io_Command = CD_QCODELSN;  /* Retrieve TOC information */
*       ior->io_Length  = 0;            /* MUST be zero             */
*       ior->io_Data    = (APTR)qcode;  /* Here's where we want it  */
*       DoIO (ior);
*
*       if (!ior->io_Error) {           /* Command succeeded        */
*
*           printf("Current position is: %ld\n", qcode.DiskPosition.LSN);
*           }
*
*   NOTES
*       This function may not return immediately.  It may take several frames
*       to pass by before a valid Q-Code packet can be returned.  Use SendIO()
*       and CheckIO() if response time is critical, and the information is
*       not.
*
*   BUGS
*
*   SEE ALSO
*       CD_PLAYMSF, CD_PLAYLSN, CD_PLAYTRACK, <devices/cd.h>
*
******************************************************************************

CmdQCodeLSN:
                move.b  #1,d0
                bra     CmdQCode

*
******* cd.device/CD_QCODEMSF ************************************************
*
*   NAME
*       CD_QCODEMSF -- Report current disk position.
*
*   IO REQUEST
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_QCODEMSF
*       io_Data         pointer to QCode structure
*       io_Length       0 - MUST be zero (for future compatability)
*
*   RESULTS
*       io_Error        0 for success, or an error code as defined in
*                       <devices/cd.h>
*
*   FUNCTION
*       This command reports current subcode Q channel time information.  This
*       command only returns data when CD Audio is playing (or paused).  At 
*       any other time, an error is returned.  The Q-Code packet consists of:
*
*       struct QCode {
*
*           UBYTE        CtlAdr;        /* Data type / QCode type           */
*           UBYTE        Track;         /* Track number                     */
*           UBYTE        Index;         /* Track subindex number            */
*           UBYTE        Zero;          /* The "Zero" byte of Q-Code packet */
*           union LSNMSF TrackPosition; /* Position from start of track     */
*           union LSNMSF DiskPosition;  /* Position from start of disk      */
*           };
*
*   EXAMPLE
*
*       struct QCode qcode;
*
*       ior->io_Command = CD_QCODEMSF;  /* Retrieve TOC information */
*       ior->io_Length  = 0;            /* MUST be zero             */
*       ior->io_Data    = (APTR)qcode;  /* Here's where we want it  */
*       DoIO (ior);
*
*       if (!ior->io_Error) {           /* Command succeeded        */
*
*           printf("Current position is: %02d:%02d:%02d\n",
*               qcode.DiskPosition.MSF.Minute,
*               qcode.DiskPosition.MSF.Second,
*               qcode.DiskPosition.MSF.Frame);
*           }
*
*   NOTES
*       This function may not return immediately.  It may take several frames
*       to pass by before a valid Q-Code packet can be returned.  Use SendIO()
*       and CheckIO() if response time is critical, and the information is
*       not.
*
*   BUGS
*
*   SEE ALSO
*       CD_PLAYMSF, CD_PLAYLSN, CD_PLAYTRACK, <devices/cd.h>
*
******************************************************************************

CmdQCodeMSF:
                clr.b   d0
CmdQCode:
                save    d2                                                      ; Save LSN/MSF flag
                move.b  d0,d2

                move.w  #CDCMD_QCODE,db_CMD(db)                                 ; Get a valid time-encoded packet
                jsr     DoCmd
                bne     1$

                move.l  IO_DATA(ior),a1                                         ; Status byte

                move.b  db_QCode+3(db),(a1)+                                    ; Store CTLADR byte

                move.b  db_QCode+4(db),d0                                       ; Store track
                jsr     BCDtoBIN
                move.b  d0,(a1)+

                move.b  db_QCode+5(db),d0                                       ; Store subindex
                jsr     BCDtoBIN
                move.b  d0,(a1)+

                move.b  db_QCode+9(db),(a1)+                                    ; Store zero byte


                move.l  db_QCode+5(db),d0                                       ; Get relative time - convert BCD to BIN format
                and.l   #$00ffffff,d0
                jsr     MSFBCDtoBIN
                move.l  d0,(a1)+

                move.l  db_QCode+9(db),d0                                       ; Get absolute time - convert BCD to BIN format
                and.l   #$00ffffff,d0
                jsr     MSFBCDtoBIN
                move.l  d0,(a1)+

                tst.b   d2                                                      ; Convert to LSN format if requested
                beq     1$
                
                move.l  IO_DATA(ior),a1                                         ; Convert track position to LSN format
                move.l  QCODE_TrackPosition(a1),d0
                jsr     MSFtoLSNPOS
                move.l  d0,QCODE_TrackPosition(a1)

                move.l  QCODE_DiskPosition(a1),d0                               ; Convert disk position to LSN format
                jsr     MSFtoLSNPOS
                move.l  d0,QCODE_DiskPosition(a1)
1$
                restore d2
                rts


*
******* cd.device/CD_ATTENUATE ***********************************************
*
*   NAME
*       CD_ATTENUATE -- Attenuate CD audio volume (immediately or gradually)
*
*   IO REQUEST
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_ATTENUATE
*       io_Data         NULL
*       io_Length       duration of volume fade in frames
*       io_Offset       target volume level (0 - 0x7FFF) (-1 = status only)
*
*   RESULTS
*       io_Error        Returns an error if drive does not support attenuation
*       io_Actual       current volume level (fade may be monitored)
*
*   FUNCTION
*       This command will ramp the CD audio volume up or down from its
*       current value to the value contained in io_Offset.  The range is 0
*       (silence) to 0x7FFF (full volume).  If -1 is specified as the target,
*       the attenuation will not be modified; the current attenuation value
*       will be returned in io_Actual.
*
*       io_Length contains the duration of the fade.  In seconds, this is
*       io_Length divided by the current frame rate (usually 75).
*
*       Note that this command returns before the fade has completed.  Thus,
*       once started, a fade cannot be aborted.  You can, however, send a
*       new CD_ATTENUATE command, which will immediately override any fade
*       currently in progress.  An io_Length of zero means attenuate
*       immediately.
*
*       If a gradual attenuation command is sent before the play command, the
*       fade will begin as soon as the play command is sent.
*
*   EXAMPLE
*
*   NOTES
*       This command has no effect on Amiga audio volume, only CD audio.
*
*       If the drive does not support volume attenuation, but does support
*       mute, a value of under $0800 should be considered mute, and equal
*       to or above should be full volume.  If chunky attenuation is
*       supported, the drive should do the best it can.  If the drive does
*       not support volume attenuation at all, an error should be returned.
*       Even if only mute is supported, if gradual attenuation is requested,
*       the device should still emulate the fade command and mute based on
*       the $0800 boundary.
*
*   BUGS
*
*   SEE ALSO
*       CD_INFO
*
******************************************************************************

CmdAttenuate:
                save    d2                                                      ; Return previous attenuation
                clr.l   IO_ACTUAL(ior)
                move.w  db_Attenuation(db),IO_ACTUAL+2(ior)

                move.l  IO_OFFSET(ior),d0                                       ; Set target attenuation
                cmp.l   #-1,d0
                beq     2$

                cmp.l   #$7FFF,d0                                               ; Cap off at $7FFF
                bls     0$
                move.l  #$7FFF,d0
0$              move.l  d0,d2

                move.l  IO_LENGTH(ior),d1
                and.l   #$7FFF,d1                                               ; Set attenuation now?
                beq.s   3$

                sub.w   db_TargetAttenuation(db),d0                             ; Calculate attenuation per step
                ext.l   d0
                divs    d1,d0

                tst.w   d0                                                      ; Minimum fade step = 1
                bne.s   1$
                move.w  #1,d0
1$              move.w  d0,db_FadeStepSize(db)

                move.w  d2,db_TargetAttenuation(db)                             ; Save our desired attenuation value

                move.b  #1,db_AutoFade(db)                                      ; Enable frame interrupts (when playing)
                jsr     SetSubcodeInterrupt
2$              restore d2
                rts
3$
                clr.w   db_FadeStepSize(db)                                     ; Cancel out any fade and set attenuation now
                move.w  d0,db_TargetAttenuation(db)
                move.w  d0,db_Attenuation(db)

                move.w  db_Info+CDINFO_Status(db),d0                            ; If we are playing audio, attenuate now
                btst    #CDSTSB_PLAYING,d0
                beq     4$
                bsr     Attenuate
4$
                restore d2
                rts


*
******* cd.device/CD_ADDFRAMEINT *********************************************
*
*   NAME
*       CD_ADDFRAMEINT -- add a CD-frame software interrupt handler.
*
*   IO REQUEST
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_ADDFRAMEINT
*       io_Length       sizeof(struct Interrupt)
*       io_Data         pointer to Interrupt structure
*
*   RESULTS
*       io_Error        0 for success, or an error code as defined in
*                       <devices/cd.h>
*
*   FUNCTION
*       This command lets you add a software interrupt handler to the
*       disk device that gets invoked whenever a new frame is encountered
*       while CD audio is being played.
*
*       You must pass in a properly initialized Exec Interrupt structure
*       and be prepared to deal with frame interrupts immediately.
*       The interrupt is generated by the exec Cause function, so you must
*       preserve A6.
*
*       To set up the handler, an Interrupt structure must be initialized.
*       This structure is supplied in io_Data of the CD_ADDFRAMEINT
*       command.  The handler then gets linked into the handler chain and
*       gets invoked whenever a frame event occurs.  You must eventually
*       remove the handler before you exit.
*
*       This command only returns when the handler is removed. That is,
*       the device holds onto the IO request until the CD_REMFRAMEINT command
*       is executed with that same IO request.  Hence, you must use SendIO()
*       with this command.
*
*   NOTES
*       The interrupt handler can be added before or after a play command is
*       sent.  Interrupts will only be generated while CD audio is playing.
*       Interrupts will not be generated when audio is paused.
*
*   SEE ALSO
*       CD_REMFRAMEINT, <devices/cd.h>, <exec/interrupts.h>,
*       exec.library/Cause()
*
******************************************************************************

CmdAddFrameInt:
                lea     db_FrameList(db),a0                                     ; Add request to frame list
                jsr     AddRequest

                move.b  #1,db_AutoFrame(db)                                     ; Enable frame interrupts (when playing)
                jsr     SetSubcodeInterrupt
                rts


*
******* cd.device/CD_REMFRAMEINT *********************************************
*
*   NAME
*       CD_REMFRAMEINT -- remove a CD-frame interrupt handler.
*
*   IO REQUEST
*       The same IO request used for CD_ADDFRAMEINT.
*
*       io_Device       preset by the call to OpenDevice()
*       io_Unit         preset by the call to OpenDevice()
*       io_Command      CD_REMFRAMEINT
*       io_Length       sizeof(struct Interrupt)
*       io_Data         pointer to Interrupt structure
*
*   RESULTS
*       io_Error        0 for success, or an error code as defined in
*                       <devices/cd.h>
*
*   FUNCTION
*       This command removes a CD-frame software interrupt added
*       by a previous use of CD_ADDFRAMEINT.
*
*   BUGS
*
*   SEE ALSO
*       CD_ADDFRAMEINT, <devices/cd.h>
*
******************************************************************************

CmdRemFrameInt:
                jsr         RemoveRequest                                       ; Remove request from frame list

                lea         db_FrameList(db),a0                                 ; If list is empty, disable frame interrupts
                IFNOTEMPTY  a0,1$
                clr.b       db_AutoFrame(db)
                jsr         SetSubcodeInterrupt
1$              rts



*****i* Dummy Commands *******************************************************

CmdReset:
CmdUpdate:                                                                      ; These commands do nothing
CmdClear:
CmdStop:
CmdStart:
CmdFlush:
CmdRemove:
NoCommand:
                rts


*****i* Write Protected ******************************************************

CmdFormat:
CmdWrite:
                move.b  #CDERR_WriteProt,IO_ERROR(ior)                          ; All disks are write protected
                rts

