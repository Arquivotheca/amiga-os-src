

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
        INCLUDE "exec/semaphores.i"
        INCLUDE "exec/ables.i"
        INCLUDE "hardware/intbits.i"
        INCLUDE "devices/trackdisk.i"
        INCLUDE "libraries/configregs.i"
        INCLUDE "libraries/configvars.i"

        INCLUDE "defs.i"
        INCLUDE "cd.i"
        INCLUDE "cdprivate.i"
        INCLUDE "cdgs_hw.i"
        INCLUDE "cd_rev.i"

        OPT     p=68020

*
************************************************************************
***
***  External References
***
************************************************************************

        XREF    CDTask
        XREF    IntrProc
        XREF    XLIntrProc
        XREF    InitIntr
        XREF    InitPort
        XREF    AllocAligned

        XREF    NoCommand      
        XREF    CmdReset       
        XREF    CmdRead        
        XREF    CmdWrite       
        XREF    CmdUpdate      
        XREF    CmdClear       
        XREF    CmdStop        
        XREF    CmdStart       
        XREF    CmdFlush       
        XREF    CmdMotor       
        XREF    CmdSeek        
        XREF    CmdFormat      
        XREF    CmdRemove      
        XREF    CmdChangeNum   
        XREF    CmdChangeState 
        XREF    CmdProtStatus  
        XREF    CmdGetDriveType
        XREF    CmdGetNumTracks
        XREF    CmdAddChangeInt
        XREF    CmdRemChangeInt
        XREF    CmdGetGeometry 
        XREF    CmdEject       
        XREF    CmdInfo        
        XREF    CmdConfig
        XREF    CmdTOCMSF
        XREF    CmdTOCLSN      
        XREF    CmdReadXL      
        XREF    CmdPlayTrack   
        XREF    CmdPlayMSF
        XREF    CmdPlayLSN
        XREF    CmdPause       
        XREF    CmdSearch
        XREF    CmdQCodeMSF
        XREF    CmdQCodeLSN    
        XREF    CmdAttenuate   
        XREF    CmdAddFrameInt 
        XREF    CmdRemFrameInt 

        XDEF    DevName
        XDEF    DevFuncs

        XREF    PutHex
        XREF    PutChar

        INT_ABLES

*
************************************************************************
***
***  Preliminaries
***
***********************************************************************/

                rts

                dc.b    'CD Device Driver',0
                dc.b    'Copyright (c) 1991 Commodore Electronics Ltd.',0
                ds.w    0

************************************************************************
***
***  Resident Tag
***
************************************************************************

        XDEF    ResTag
        XREF    EndCode

ResTag:         dc.w    RTC_MATCHWORD
                dc.l    ResTag
                dc.l    EndCode
                dc.b    RTF_COLDSTART
                dc.b    VERSION
                dc.b    NT_DEVICE
                dc.b    8
                dc.l    DevName
                dc.l    ModIdent
                dc.l    InitDevice

************************************************************************
***
***  Module Strings
***
************************************************************************

ModIdent:       VSTRING
                VERSTAG
                ds.w    0

DevName:        dc.b    'cd.device',0
                ds.w    0

*
************************************************************************
***
***  DEVICE DRIVER INITIALIZATION
***
***     Create and initialize all data structures and hardware.
***
************************************************************************

InitDevice:
                movem.l a2/a3/db/hb,-(sp)

                move.l  #CDHARDWARE,hb                                  ; Hardware base address

                move.b  #0,CDCOMRXCMP(hb)                               ; Reset hardware
                move.b  #0,CDCOMTXCMP(hb)

                move.l  #db_SizeOf,d0                                   ; Create device (library) node
                clear   d1
                lea     DevFuncs(pc),a0
                move.l  d1,a1
                move.l  d1,a2
                exec    MakeLibrary
                tst.l   d0
                beq     99$
                move.l  d0,db
                move.b  #NT_DEVICE,LN_TYPE(db)
                lea     DevName(pc),a0
                move.l  a0,LN_NAME(db)
                move.b  #LIBF_SUMUSED|LIBF_CHANGED,LIB_FLAGS(db)
                move.w  #VERSION,LIB_VERSION(db)
                move.w  #REVISION,LIB_REVISION(db)
                lea     ModIdent(pc),a0
                move.l  a0,LIB_IDSTRING(db)

                move.w  #$7FFF,db_Attenuation(db)                       ; Set volume attenuation
                move.w  #$7FFF,db_TargetAttenuation(db)
                move.w  #$0500,db_DriveBits(db)

                move.l  #STACK_SIZE,d0                                  ; Create I/O task stack
                clear   d1
                exec    AllocMem
                tst.l   d0
                beq     2$
                move.l  d0,a0
                lea     db_Task(db),a1
                move.l  a0,TC_SPLOWER(a1)
                add     #STACK_SIZE,a0
                move.l  a0,TC_SPUPPER(a1)
                move.l  db,-(a0)
                move.l  a0,TC_SPREG(a1)

                move.l  #CDROMPAGESIZE,d0                               ; Allocate ROM data page
                move.l  #MEMF_24BITDMA,d1
                jsr     AllocAligned
                beq     99$
                move.l  d0,db_CDROMPage(db)
                move.l  d0,CDROMHIGH(hb)

                move.l  d0,a0                                           ; Set Invalid mark (status long) in buffers
                clr.l   d0
1$              move.w  #SECSTSF_INVALID,ROM_STATUS+2(a0,d0.l)
                add.w   #$1000,d0
                cmp.w   #(PBXSIZE*$1000)&$FFFF,d0
                bne     1$

                move.l  #CDCOMPAGESIZE,d0                               ; Allocation communications page
                move.l  #MEMF_24BITDMA|MEMF_REVERSE,d1
                jsr     AllocAligned
                beq     98$
                move.l  d0,CDCOMHIGH(hb)

                move.l  d0,db_CDCOMRXPage(db)
                add.l   #$100,d0
                move.l  d0,db_CDSUBPage(db)
                add.l   #$100,d0
                move.l  d0,db_CDCOMTXPage(db)

                move.w  #75,db_Info+CDINFO_PlaySpeed(db)                ; Initialize CDINFO structure
                move.w  #75,db_Info+CDINFO_ReadSpeed(db)
                move.w  #75,db_Info+CDINFO_ReadXLSpeed(db)
                move.w  #2048,db_Info+CDINFO_SectorSize(db)
                move.w  #1,db_Info+CDINFO_XLECC(db)
                move.w  #150,db_Info+CDINFO_MaxSpeed(db)
                move.w  #1,db_Info+CDINFO_AudioPrecision(db)

                lea     db_ChgList(db),a0                               ; Initialize disk change IOR list
                NEWLIST a0

                lea     db_FrameList(db),a0                             ; Initialize frame interrupt callback list
                NEWLIST a0

                moveq.l #INTB_PORTS,d0                                  ; Initialize interrupt server
                moveq.l #32,d1
                lea     IntrProc(pc),a0
                lea     db_StatIntr(db),a1
                bsr     InitIntr

                lea     XLIntrProc(pc),a0                               ; Initialize XL interrupt server
                lea     db_XLIntr(db),a1
                move.b  #NT_INTERRUPT,LN_TYPE(a1)
                move.b  #32,LN_PRI(a1)
                move.l  db,IS_DATA(a1)
                move.l  a0,IS_CODE(a1)  
                lea     DevName(pc),a0
                move.l  a0,LN_NAME(a1)

                moveq   #SIGB_CMDPORT,d0                                ; Initialize non-disk-access message port
                lea     db_ClassACmdPort(db),a0
                bsr     InitPort

                moveq   #SIGB_DISKCMDPORT,d0                            ; Initialize disk-access message port
                lea     db_ClassDCmdPort(db),a0
                bsr     InitPort

                move.b  CDCOMTXINX(hb),db_ComTXInx(db)                  ; Remember Indexes

                move.l  CDCONFIG(hb),d0                                 ; Init CD-ROM registers
                and.l   #CF_PALNTSC|CF_DBLCAS,d0
                or.l    #CF_CDCOMTX|CF_CDCOMRX|CF_2500Q|CF_PBX,d0
                move.l  d0,CDCONFIG(hb)
                move.b  CDCOMRXINX(hb),d0
                move.b  d0,db_ComRXInx(db)
                add.b   #1,d0
                move.b  d0,CDCOMRXCMP(hb)
                move.b  d0,db_CDCOMRXCMP(db)                            ; - Save in shadow location
                move.b  #0,db_ReceivingCmd(db)
                move.b  #0,CDCOMTXCMP(hb)
                move.l  #INTF_RXDMADONE|INTF_TXDMADONE,CDINT2ENABLE(hb)
                move.b  #$10,db_PacketIndex(db)

                lea     db_NULL(db),a0                                  ; Initialize CD_READ's fake xfer node
                move.l  a0,db_XferNode+MLN_SUCC(db)

                move.l  #-1,d0                                          ; Allocate a signal for us to wait on
                exec    AllocSignal
                move.b  d0,db_InitSignal(db)  

                sub.l   a1,a1                                           ; Find our task
                exec    FindTask
                move.l  d0,db_InitTask(db)

                lea     db_Task(db),a1                                  ; Create I/O Task
                move.b  #NT_TASK,LN_TYPE(a1)
                move.b  #10,LN_PRI(a1)
                lea     DevName(pc),a0
                move.l  a0,LN_NAME(a1)
                lea     CDTask(pc),a2
                sub.l   a3,a3
                exec    AddTask

                move.b  db_InitSignal(db),d1                            ; Wait for task to completely initialize
                move.l  #1,d0
                lsl.l   d1,d0
                exec    Wait

                move.b  db_InitSignal(db),d0                            ; Free signal
                exec    FreeSignal

                move.l  db,a1                                           ; Add to device list
                exec    AddDevice
2$
                move.l  db,d0
                movem.l (sp)+,a2/a3/db/hb
                rts
97$
                move.l  #CDCOMPAGESIZE,d0                               ; Free communications page
                move.l  db_CDSUBPage(db),a1
                exec    FreeMem
98$
                move.l  #CDROMPAGESIZE,d0                               ; Free ROM data page
                move.l  db_CDROMPage(db),a1
                exec    FreeMem
99$
                move.l  db,d0
                movem.l (sp)+,a2/a3/db/hb
                rts




*
******* cd.device/OpenDevice *************************************************
*
*   NAME
*       OpenDevice - Open a CD unit for access
*
*   SYNOPSIS
*       error = OpenDevice("cd.device", UnitNumber, IORequest, flags);
*       D0                 A0           D0          A1         D1
*
*   FUNCTION
*       Opens the cd.device and creates an IORequest for use in accessing
*       the CD.
*
*   INPUTS
*       UnitNumber - Normally zero; however, this is described as:
*                    Ones digit      = Unit (SCSI unit number)
*                    Tens digit      = LUN (disk within disk changer)
*                    Hundreds digit  = Card number (SCSI card)
*                    Thousands digit = Reserved (must be zero)
*       IORequest  - Pointer to a struct(IOStdReq)
*       flags      - Should be zero.
*
*   RESULTS
*       error        0 = success, otherwise this is an error.
*
*   NOTES
*
*   SEE ALSO
*       CloseDevice()
*
******************************************************************************

 FUNCTION Open
                tst.l   d0                                                      ; Only allow unit zero requests
                beq     1$
                moveq.l #CDERR_OPENFAIL,d0
                bra     2$
1$
                bclr.b  #LIBB_DELEXP,LIB_FLAGS(db)                              ; Clear possible pending delayed expunge

                addq.w  #1,LIB_OPENCNT(db)                                      ; Bump unit and device open counters

                move.l  #UNIT_PATTERN,IO_UNIT(a1)                               ; Use a pattern to determine valid I/O requests

                clear   d0                                                      ; Open ok
2$
                move.b  d0,IO_ERROR(a1)                                         ; Return Open status
                rts


*
******* cd.device/CloseDevice ************************************************
*
*   NAME
*       CloseDevice - terminate access to the CD
*
*   SYNOPSIS
*       CloseDevice(IORequest);
*                   A1
*
*   FUNCTION
*       This function will terminate access to the unit openned with
*       OpenDevice().
*
*   INPUTS
*       iORequest - pointer to a struct(IOStdReq)
*
*   RESULTS
*
*   NOTES
*
*   SEE ALSO
*       OpenDevice()
*
******************************************************************************

 FUNCTION Close
                cmp.l   #UNIT_PATTERN,IO_UNIT(a1)                               ; Verify that this is a valid I/O Request
                bne     1$

                moveq.l #-1,d0                                                  ; Invalidate various I/O Request fields
                move.l  d0,IO_UNIT(a1)
                move.l  d0,IO_DEVICE(a1)

                subq.w  #1,LIB_OPENCNT(db)                                      ; Decrement unit and device open counters

1$              clear   d0                                                      ; Prevent expunge
                rts

*
************************************************************************
***
***  EXPUNGE DEVICE
***
***     Never expunge.  Driver doesn't take much space.
***
************************************************************************

 FUNCTION Expunge
 FUNCTION fReserved
                clear   d0                                              ; Never expunge
                rts


*
************************************************************************
***
***  DEVICE FUNCTION/COMMAND ADDRESS TABLE
***
***     This table contains the addresses for device functions
***     and command functions.  It is used by MakeLibrary to
***     initialize the device node.
***
************************************************************************

DevFuncs                
                dc.l    Open            ; -6
                dc.l    Close           ; -12
                dc.l    Expunge         ; -18
                dc.l    fReserved       ; -24
                dc.l    BeginIO         ; -30
                dc.l    AbortIO         ; -36
                dc.l    fReserved       ; -42
                dc.l    fReserved       ; -48

                dc.l    CmdReset        ; 1
                dc.l    CmdRead         ; 2
                dc.l    CmdWrite        ; 3
                dc.l    CmdUpdate       ; 4
                dc.l    CmdClear        ; 5
                dc.l    CmdStop         ; 6
                dc.l    CmdStart        ; 7
                dc.l    CmdFlush        ; 8
                dc.l    CmdMotor        ; 9
                dc.l    CmdSeek         ; 10
                dc.l    CmdFormat       ; 11
                dc.l    CmdRemove       ; 12
                dc.l    CmdChangeNum    ; 13
                dc.l    CmdChangeState  ; 14
                dc.l    CmdProtStatus   ; 15
                dc.l    NoCommand       ; 16
                dc.l    NoCommand       ; 17
                dc.l    CmdGetDriveType ; 18
                dc.l    CmdGetNumTracks ; 19
                dc.l    CmdAddChangeInt ; 20
                dc.l    CmdRemChangeInt ; 21
                dc.l    CmdGetGeometry  ; 22
                dc.l    CmdEject        ; 23

                dc.l    NoCommand       ; 24
                dc.l    NoCommand       ; 25
                dc.l    NoCommand       ; 26
                dc.l    NoCommand       ; 27
                dc.l    NoCommand       ; 28
                dc.l    NoCommand       ; 29
                dc.l    NoCommand       ; 30
                dc.l    NoCommand       ; 31

                dc.l    CmdInfo         ; 32
                dc.l    CmdConfig       ; 33
                dc.l    CmdTOCMSF       ; 34
                dc.l    CmdTOCLSN       ; 35

                dc.l    CmdReadXL       ; 36

                dc.l    CmdPlayTrack    ; 37
                dc.l    CmdPlayMSF      ; 38
                dc.l    CmdPlayLSN      ; 39
                dc.l    CmdPause        ; 40
                dc.l    CmdSearch       ; 41

                dc.l    CmdQCodeMSF     ; 42
                dc.l    CmdQCodeLSN     ; 43
                dc.l    CmdAttenuate    ; 44

                dc.l    CmdAddFrameInt  ; 45
                dc.l    CmdRemFrameInt  ; 46

MAX_CMD         equ                       46

                dc.l    -1              ; end of list

*
************************************************************************
***
***  DEVICE COMMAND OPTION TABLE
***
***     This table contains bits to indicate valid options for
***     each of the device command functions.  Options are:
***
************************************************************************

        XDEF CmdOpts

CmdOpts:
        dc.w  OF_INVALID
        dc.w  OF_QUEUE                                              ; Reset
        dc.w  OF_QUEUE|OF_DISK|OF_DISKACCESS|OF_ROM|OF_ABTD         ; Read
        dc.w  OF_DISK                                               ; Write
        dc.w  0                                                     ; Update
        dc.w  0                                                     ; Clear
        dc.w  0                                                     ; Stop
        dc.w  0                                                     ; Start
        dc.w  0                                                     ; Flush
        dc.w  OF_QUEUE|OF_DISKACCESS                                ; Motor
        dc.w  OF_QUEUE|OF_DISK|OF_DISKACCESS                        ; Seek
        dc.w  OF_DISK|OF_INVALID                                    ; Format
        dc.w  OF_INVALID                                            ; Remove
        dc.w  0                                                     ; ChangeNum
        dc.w  0                                                     ; ChangeState
        dc.w  0                                                     ; ProtStatus
        dc.w  OF_INVALID                                            ; NoCommand
        dc.w  OF_INVALID                                            ; NoCommand
        dc.w  0                                                     ; CmdGetDriveType
        dc.w  0                                                     ; CmdGetNumTracks
        dc.w  OF_NOREPLY                                            ; CmdAddChangeInt
        dc.w  0                                                     ; CmdRemChangeInt
        dc.w  0                                                     ; CmdGetGeometry
        dc.w  OF_QUEUE                                              ; CmdEject
        dc.w  OF_INVALID                                            ; NoCommand
        dc.w  OF_INVALID                                            ; NoCommand
        dc.w  OF_INVALID                                            ; NoCommand
        dc.w  OF_INVALID                                            ; NoCommand
        dc.w  OF_INVALID                                            ; NoCommand
        dc.w  OF_INVALID                                            ; NoCommand
        dc.w  OF_INVALID                                            ; NoCommand
        dc.w  OF_INVALID                                            ; NoCommand

        dc.w  0                                                     ; CmdInfo
        dc.w  OF_QUEUE                                              ; CmdConfig
        dc.w  OF_QUEUE|OF_DISK                                      ; CmdTOCMSF
        dc.w  OF_QUEUE|OF_DISK                                      ; CmdTOCLSN
        dc.w  OF_QUEUE|OF_DISK|OF_DISKACCESS|OF_ROM|OF_ABTD         ; CmdReadXL
        dc.w  OF_QUEUE|OF_DISK|OF_DISKACCESS|OF_MULTITASK|OF_ABTD   ; CmdPlayTrack
        dc.w  OF_QUEUE|OF_DISK|OF_DISKACCESS|OF_MULTITASK|OF_ABTD   ; CmdPlayMSF
        dc.w  OF_QUEUE|OF_DISK|OF_DISKACCESS|OF_MULTITASK|OF_ABTD   ; CmdPlayLSN
        dc.w  OF_QUEUE|OF_DISK                                      ; CmdPause
        dc.w  OF_QUEUE|OF_DISK                                      ; CmdSearch
        dc.w  OF_QUEUE|OF_DISK|OF_ABTQ                              ; CmdQCodeMSF
        dc.w  OF_QUEUE|OF_DISK|OF_ABTQ                              ; CmdQCodeLSN
        dc.w  OF_QUEUE                                              ; CmdAttenuate
        dc.w  OF_NOREPLY                                            ; CmdAddFrameInt
        dc.w  0                                                     ; CmdRemFrameInt
        dc.w  0



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
                save    a1/ior/hb                                       ; Load up pointers
                move.l  a1,ior
                move.l  #CDHARDWARE,hb

                move.b  #NT_MESSAGE,LN_TYPE(ior)                        ; Initialize request fields
                clr.l   IO_ACTUAL(ior)

                move.b  #CDERR_BadUnitNum,IO_ERROR(ior)                 ; Check that I/O request has been opened
                cmp.l   #UNIT_PATTERN,IO_UNIT(ior)
                bne     bio_done

                move.b  #IOERR_NOCMD,IO_ERROR(ior)                      ; Check command range
                move.w  IO_COMMAND(ior),d0
                ble.s   bio_done
                cmp.w   #MAX_CMD,d0
                bgt.s   bio_done

                clr.b   IO_ERROR(ior)                                   ; Check command options
                lea     CmdOpts(pc),a1
                add.w   d0,d0
                move.w  0(a1,d0.w),d1

                btst    #OB_INVALID,d1
                bne.s   bio_done

                btst    #OB_QUEUE,d1                                    ; Should this command be queued?
                bne.s   bio_queue

                jsr     PerformIO                                       ; Execute the command now and reply (return)
bio_done:
                move.w  IO_COMMAND(ior),d0                              ; Reply (return) the request
                lea     CmdOpts(pc),a1
                add.w   d0,d0
                move.w  0(a1,d0.w),d0
                btst    #OB_NOREPLY,d0
                bne.s   bio_exit
                btst.b  #IOB_QUICK,IO_FLAGS(ior)
                bne.s   bio_exit
                move.l  ior,a1
                exec    ReplyMsg
                bra.s   bio_exit
bio_queue:
                bclr.b  #IOB_QUICK,IO_FLAGS(ior)                        ; Choose a port and queue the command
                lea     db_ClassACmdPort(db),a0
                btst    #OB_DISKACCESS,d1
                beq.s   1$
                lea     db_ClassDCmdPort(db),a0
1$:             move.l  ior,a1
                exec    PutMsg

bio_exit:       restore a1/ior/hb                                       ; All done
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
                save    ior/hb                                          ; Load up pointers
                move.l  a1,ior
                move.l  #CDHARDWARE,hb

                exec    Forbid                                          ; Forbid task switching

                move.l  ior,a1                                          ; Check if I/O is already done
                exec    CheckIO
                tst.l   d0
                bne     99$

                move.l  ior,a1                                          ; Scan both ports for the request
                lea     db_ClassACmdPort(db),a0
                bsr     FindReq
                tst.l   d0
                bne.s   1$
                lea     db_ClassDCmdPort(db),a0
                bsr     FindReq
                tst.l   d0
                beq.s   2$
1$
                move.b  #CDERR_ABORTED,IO_ERROR(ior)                    ; Remove entry from queue and reply (return)
                exec    Remove
                move.l  ior,a1
                exec    ReplyMsg
                clr.l   d0
                bra     99$
2$
                move.b  #CDERR_ABORTED,IO_ERROR(ior)                    ; Abort it and check return code
                bsr     Abort
                clr.l   d0
99$
                save    d0                                              ; Permit task switching again and return
                exec    Permit
                restore d0
                restore ior/hb
                rts

*
*=======================================================================
*==
*==  FindReq
*==
*==     in:     A0 = I/O Port
*==             A1 = I/O Request
*==
*==    out:     D0 = nonzero if found, NULL if not found
*==             A0 = I/O Request
*==
*=======================================================================

FindReq:
                move.l  MP_MSGLIST(a0),a0                               ; Look at message list of port

1$:             move.l  (a0),d0                                         ; End of list?  Exit.
                beq.s   99$

                cmp.l   a0,a1                                           ; If this is the request, return with it
                beq.s   99$

                move.l  d0,a0                                           ; Next entry
                bra.s   1$
99$:
                rts

*
************************************************************************
***
***  Abort
***
***     Abort an I/O command.
***
***             ior -> command I/O request
***             db  -> device base
***             hb  -> hardware base
***
***     Returns value in d0.
***
************************************************************************

Abort:
                move.w  IO_COMMAND(ior),d0                              ; Get command's options
                lea     CmdOpts(pc),a1

                add.w   d0,d0                                           ; Get abort type
                clr.l   d1
                move.w  0(a1,d0.w),d1
                and.w   #$FF00,d1

                cmp.w   #OF_ABTD,d1                                     ; Abort play or read?
                bne     1$

                SIGNAL  SIGF_ABORTDRIVE
                rts
1$
                cmp.w   #OF_ABTQ,d1                                     ; Abort Q-Code?
                bne     2$
                SIGNAL  SIGF_ABORTQCODE
2$
                rts




************************************************************************
***
***  PERFORMIO
***
***     Dispatch to the appropriate command handling code.
***
************************************************************************

 FUNCTION PerformIO
    IFD DEBUG
                jsr     PrintCommand
    ENDC
                lea     CmdOpts(pc),a1                                  ; Check command options
                move.w  IO_COMMAND(ior),d0
                add.w   d0,d0
                move.w  0(a1,d0.w),d1

                btst    #OB_DISK,d1                                     ; If no disk, should we return an error?
                beq.s   1$
                move.b  #CDERR_NoDisk,IO_ERROR(ior)
                move.w  db_Info+CDINFO_Status(db),d0
                btst    #CDSTSB_DISK,d0
                beq.s   99$
1$
                btst    #OB_ROM,d1                                      ; If no CD-ROM disk, should we return an error?
                beq.s   2$
                move.b  #CDERR_BadDataType,IO_ERROR(ior)
                move.w  db_Info+CDINFO_Status(db),d0
                btst    #CDSTSB_CDROM,d0
                beq.s   99$
2$
                clr.b   IO_ERROR(ior)                                   ; Execute Command
                clr.l   d0
                move.w  IO_COMMAND(ior),d0
                addq.l  #8,d0
                muls    #-6,d0
                jsr     0(db,d0.w)
99$
    IFD DEBUG
                move.b  IO_ERROR(ior),d0
                beq     100$
                and.l   #$000000ff,d0
                jsr     PutHex
100$
                move.b  #10,d0
                jsr     PutChar
    ENDC
                rts


    IFD DEBUG


PrintCommand:
                move.b  #'*',d0
                jsr     PutChar
                jsr     PutChar
                jsr     PutChar
                move.b  #'C',d0
                jsr     PutChar
                move.b  #'M',d0
                jsr     PutChar
                move.b  #'D',d0
                jsr     PutChar
                move.b  #' ',d0
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

    ENDC
