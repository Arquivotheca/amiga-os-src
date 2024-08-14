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
        INCLUDE "include:exec/semaphores.i"
        INCLUDE "include:exec/ables.i"
        INCLUDE "include:utility/tagitem.i"
        INCLUDE "include:hardware/intbits.i"
        INCLUDE "include:devices/trackdisk.i"
        INCLUDE "include:devices/timer.i"
        INCLUDE "include:libraries/configregs.i"
        INCLUDE "include:libraries/configvars.i"

        INCLUDE "gs:cd/cd.i"
        INCLUDE "cdtv.i"
        INCLUDE "defs.i"
        INCLUDE "cddev.i"
        INCLUDE "cdtv_rev.i"
        INCLUDE "macros.i"

        OPT     p=68020

*
************************************************************************
***
***  External References
***
************************************************************************

        XREF    DevFuncs
        XREF    CmdOpts
        XREF    CmdAborts
        XREF    IntrProc
        XREF    GenStatus
        XREF    GetClass1Error
        XREF    GetClass2Error
        XREF    GetROMTrack
        XREF    PutHex
        XREF    DoCmd
        XREF    CmdReset
        XREF    ResetConfig
        XREF    AbortPlay
        XREF    AbortCmd
        XREF    WimpAllocCD
        XREF    FreeCD
        XREF    NextSeg
        XREF    CDBeginIO

        XREF    PutStr

        XDEF    DevName
        XDEF    Disable
        XDEF    Enable

        INT_ABLES

*
************************************************************************
***
***  Preliminaries
***
***********************************************************************/

                rts

                dc.b    'CDTV Device Driver',0
                dc.b    'Copyright (c) 1991 Commodore Electronics Ltd.',0
                dc.b    'All Rights Reserved.',0
                ds.w    0
                dc.b    'Patent Pending.',0
                dc.b    'Written by Carl Sassenrath, Ukiah, CA',0
                dc.b    'Modified by Jerry Horanoff',0
                ds.w    0

                VERSTAG
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
                dc.b    8                                               ; - priority
                dc.l    DevName
                dc.l    ModIdent
                dc.l    InitDevice

************************************************************************
***
***  Module Strings
***
************************************************************************

ModIdent:       VSTRING
                ds.w    0
cdDevName:      dc.b    'cd.device',0
DevName:        dc.b    'cdtv.device',0
DevTask:        dc.b    'CDTVTask',0
ExpanName:      dc.b    'expansion.library',0
                ds.w    0

ManuProd        dc.l    $202,$3                                         ; ConfigDev Manufacturer/Product


TagListSingle:
                dc.l    TAGCD_READSPEED,75
                dc.l    TAG_END,0

TagListDouble:
                dc.l    TAGCD_READSPEED,150
                dc.l    TAG_END,0


*
************************************************************************
***
***  DEVICE DRIVER INITIALIZATION
***
***     Create and initialize all data structures and hardware.
***
************************************************************************

InitDevice:
                movem.l d1-d7/a0-a6,-(sp)

                move.l  #db_SizeOf,d0                                   ; Create device (library) node
                clear   d1
                lea     DevFuncs(pc),a0
                move.l  d1,a1
                move.l  d1,a2
                exec    MakeLibrary
                tst.l   d0
                beq     4$
                move.l  d0,db
                move.b  #NT_DEVICE,LN_TYPE(db)
                lea     DevName(pc),a0
                move.l  a0,LN_NAME(db)
                move.b  #LIBF_SUMUSED|LIBF_CHANGED,LIB_FLAGS(db)
                move.w  #VERSION,LIB_VERSION(db)
                move.w  #REVISION,LIB_REVISION(db)
                lea     ModIdent(pc),a0
                move.l  a0,LIB_IDSTRING(db)

                clr.w   db_Openned(db)
                move.l  #2048,db_BlockSize(db)

                move.l  db,a1                                           ; Add to device list
                exec    AddDevice
4$
                move.l  db,d0
                movem.l (sp)+,d1-d7/a0-a6
                rts






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

                exec    CreateMsgPort                                   ; Create msg port D
                beq     open_fail
                move.l  d0,db_PortD(db)
                
                move.l  db_PortD(db),a0                                 ; Create I/O request D
                move.l  #IOSTD_SIZE,d0
                exec    CreateIORequest
                beq     open_fail
                move.l  d0,db_IORD(db)

                lea     cdDevName(pc),a0                                ; Open cd.device
                clr.l   d0
                move.l  db_IORD(db),a1
                clr.l   d1
                exec    OpenDevice

                move.l  db_IORD(db),a1                                  ; Setpatch cd.device BeginIO
                move.l  IO_DEVICE(a1),a0
                exec    Forbid
                move.l  -30+2(a0),db_CDBeginIO(db)
                lea     CDBeginIO(pc),a1
                move.l  a1,-30+2(a0)
                exec    Permit
TaskLoop:
                lea     db_DiskPort(db),a0                              ; Get a message
                exec    GetMsg
                tst.l   d0
                bne     1$

                move.l  #SIGF_DISKPORT,d0                               ; Wait for something to happen
                exec    Wait
                bra     TaskLoop
1$
                bclr    #0,db_abortedflags(db)                          ;should be cleared often -
                                                                        ;only set if CMD_READ is aborting an improgress IO
                                                                        ;request
                move.l  d0,ior
                cmp.w   #CDTV_READXL,IO_COMMAND(ior)
                bne     NotReadXL

                clr.l   d4                                              ; Transfer's IO_ACTUAL

                move.l  db_IORD(db),a1                                  ; Just do a regular READ
                move.w  #CD_READ,IO_COMMAND(a1)

                move.l  IO_LENGTH(ior),d3                               ; D3 = MaxTransfer
                move.l  db_BlockSize(db),d1
                mulu.l  d1,d3

                move.l  IO_OFFSET(ior),d0                               ; Correct offset
                mulu.l  d1,d0
                move.l  d0,IO_OFFSET(a1)

                clr.b   IO_FLAGS(a1)                                    ; Clear out flags

                move.l  IO_DATA(ior),a2                                 ; Get XL node
                FIRSTNODE a2

NextNode:
                tst.l   d3                                              ; is IO_OFFSET exceeded?
                ble     EndOfXL

                clr.l   XL_ACTUAL(a2)

                move.l  XL_LENGTH(a2),d2
                beq     5$                                              ; skip XL node?
1$
                move.l  XL_BUFFER(a2),d0
                bne     2$

                lea     db_Buff(db),a0                                  ; use skip buffer if no buf provided (seek)
                move.l  a0,d0

                move.l  #2048,d1
                cmp.l   d1,d2
                bls     2$

                move.l  d1,d2
2$
                move.l  d0,IO_DATA(a1)

                cmp.l   d2,d3                                           ; bound to IO_OFFSET
                bhs     3$
                move.l  d3,d2
3$
                move.l  d2,IO_LENGTH(a1)

                exec    DoIO                                            ; Do the read

                move.l  db_IORD(db),a1
                tst.b   IO_ERROR(a1)
                bne     EndOfXL

                add.l   d2,IO_OFFSET(a1)                                ; increment offset for next read
                add.l   d2,XL_ACTUAL(a2)
                add.l   d2,d4                                           ; add to total
                sub.l   d2,d3                                           ; subtract for max
                ble     4$                                              ; if exhausted, we are done
                                                                        ; regardless of XL node info
                move.l  XL_LENGTH(a2),d2
                sub.l   XL_ACTUAL(a2),d2
                bne     1$              
4$
                move.l  XL_DONECODE(a2),a0                              ; Call DoneCode function
                cmp.l   #0,a0
                beq     5$
                jsr     (a0)
                move.l  db_IORD(db),a1
5$
                move.l  LN_SUCC(a2),a2
                tst.l   (a2)
                bne     NextNode
                
EndOfXL:
                move.l  d4,IO_ACTUAL(ior)                               ; report error status
                move.b  IO_ERROR(a1),IO_ERROR(ior)
                bra     Reply
NotReadXL:
                move.l  db_IORD(db),a1
                exec    DoIO

                move.l  db_IORD(db),a1
                move.l  IO_ACTUAL(a1),IO_ACTUAL(ior)
                move.b  IO_ERROR(a1),d0
                move.b  d0,IO_ERROR(ior)

                bclr    #0,db_abortedflags(db)
                beq     Reply
                cmp.b   #CDERR_ABORTED,d0                               ; if aborted due to a CMD_READ
                beq     TaskLoop                                        ; no reply desired
Reply:
                move.l  ior,a1
                exec    ReplyMsg
                bra     TaskLoop



************************************************************************
***
***  OPEN DEVICE
***
***     Open access to the device.  Setup the unit field of the
***     I/O request to identify successful open.
***
***     The system calls this function with:
***             D0 == Unit Number (0-3)
***             A1 -> IO Request Structure
***             A6 -> Device Node
***
***     Errors are reported in the IO_ERROR field of IORequest
***
************************************************************************

 FUNCTION Open
                save    d1-d7/a0-a6
                save    a2/ior
                tst.l   d0                                              ; Only allow unit zero requests
                bne     open_fail

                move.l  a1,ior

                bclr.b  #LIBB_DELEXP,LIB_FLAGS(db)                      ; Clear possible pending delayed expunge

                addq.w  #1,LIB_OPENCNT(db)                              ; Bump unit and device open counters

                lea     DevTask(pc),a1
                exec    FindTask
                tst.l   d0
                bne     1$

                move.l  #STACK_SIZE,d0                                  ; Create I/O task stack
                clear   d1
                exec    AllocMem
                tst.l   d0
                beq     open_fail
                move.l  d0,a0
                lea     db_Task(db),a1
                move.l  a0,TC_SPLOWER(a1)
                add     #STACK_SIZE,a0
                move.l  a0,TC_SPUPPER(a1)
                move.l  db,-(a0)
                move.l  a0,TC_SPREG(a1)

                moveq   #SIGB_DISKPORT,d0
                lea     db_DiskPort(db),a0
                bsr     InitPort

                lea     db_Task(db),a1                                  ; Create I/O Task
                move.b  #NT_TASK,LN_TYPE(a1)
                move.b  #100,LN_PRI(a1)
                lea     DevTask(pc),a0
                move.l  a0,LN_NAME(a1)
                lea     CDTask(pc),a2
                sub.l   a3,a3
                exec    AddTask
1$
                move.l  #UNIT_SizeOF,d0
                move.l  #MEMF_PUBLIC,d1
                exec    AllocMem
                move.l  d0,IO_UNIT(ior)
                move.l  d0,a2

                exec    CreateMsgPort                                   ; Create msg port A
                beq     open_fail
                move.l  d0,UNIT_PortA(a2)
                
                move.l  UNIT_PortA(a2),a0                                 ; Create I/O request A
                move.l  #IOSTD_SIZE,d0
                exec    CreateIORequest
                beq     open_fail
                move.l  d0,UNIT_IORA(a2)

                exec    CreateMsgPort                                   ; Create msg port Chg
                beq     open_fail
                move.l  d0,UNIT_PortChg(a2)
                
                move.l  UNIT_PortChg(a2),a0                               ; Create I/O request Chg
                move.l  #IOSTD_SIZE,d0
                exec    CreateIORequest
                beq     open_fail
                move.l  d0,UNIT_IORChg(a2)

                lea     cdDevName(pc),a0                                ; Open cd.device
                clr.l   d0
                move.l  UNIT_IORA(a2),a1
                clr.l   d1
                exec    OpenDevice
                tst.b   d0
                bne     open_fail

                move.l  UNIT_IORA(a2),a0                                  ; Duplicate IORequest
                move.l  UNIT_IORChg(a2),a1
                move.l  IO_DEVICE(a0),IO_DEVICE(a1)
                move.l  IO_UNIT(a0),IO_UNIT(a1)

                clear   d0                                              ; Open ok
open_exit:
                restore a2/ior
                restore d1-d7/a0-a6
                move.b  d0,IO_ERROR(a1)                                 ; Return Open status
                rts

open_fail:      moveq.l #IOERR_OPENFAIL,d0                              ; Open failed
                bra     open_exit


************************************************************************
***
***  CLOSE DEVICE
***
***     Conclude access to a particular device unit.
***
***     The system calls this function with:
***             A1 -> IO Request Structure
***             A6 -> Device Node
***
***     Return zero to prevent any kind of expunge.
***
************************************************************************

 FUNCTION Close
                save    a2/ior
                move.l  a1,ior

                move.l  IO_UNIT(ior),a2

                moveq.l #-1,d0                                          ; Invalidate various I/O Request fields
                move.l  d0,IO_UNIT(ior)
                move.l  d0,IO_DEVICE(ior)

                subq.w  #1,LIB_OPENCNT(db)                              ; Decrement unit and device open counters

                move.l  UNIT_IORA(a2),a1
                exec    CloseDevice

                move.l  UNIT_IORChg(a2),a0
                exec    DeleteIORequest
                move.l  UNIT_PortChg(a2),a0
                exec    DeleteMsgPort

                move.l  UNIT_IORA(a2),a0
                exec    DeleteIORequest
                move.l  UNIT_PortA(a2),a0
                exec    DeleteMsgPort

                move.l  a2,a1
                move.l  #UNIT_SizeOF,d0
                exec    FreeMem

close_exit:
                restore a2/ior
                clear   d0                                              ; Prevent expunge
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
 FUNCTION Freserved
                clear   d0                                              ; Never expunge
                rts


*
************************************************************************
***
*** LSNtoMSFPOS
***
*** in:         D0 = (long)LSN
***
*** out:        D0 = (long)0x00MMSSFF
***
************************************************************************

 FUNCTION LSNtoMSFPOS

        save    d1

        add.l   #150,d0                                                 ; Add on 00:02:00

        move.l  d0,d1                                                   ; Calculate Frame (Block)
        clr.l   d0
        divu    #75,d1
        swap    d1
        move.b  d1,d0

        clr     d1                                                      ; Calculate Minute and Second
        swap    d1
        divu    #60,d1
        swap    d1
        lsl     #8,d1
        or.l    d1,d0

        restore d1
        rts


************************************************************************
***
*** MSFtoLSNPOS
***
*** in:         D0 = (long)0x00MMSSFF
***
*** out:        D0 = (long)LSN
***
************************************************************************

 FUNCTION MSFtoLSNPOS

        save    d1/d2

        swap    d0                                                      ; Calculate sector from minute
        move    d0,d1
        mulu    #60*75,d1

        swap    d0                                                      ; Add in sector from frame
        clr.l   d2
        move.b  d0,d2
        add.l   d2,d1

        lsr     #8,d0                                                   ; Add in sector from second
        mulu    #75,d0
        add.l   d1,d0

        sub.l   #150,d0                                                 ; Subtract off 00:02:00

        restore d1/d2
        rts


*
************************************************************************
***
*** LSNtoMSF
***
*** in:         D0 = (long)LSN
***
*** out:        D0 = (long)0x00MMSSFF
***
************************************************************************

 FUNCTION LSNtoMSF

        save    d1

        move.l  d0,d1                                                   ; Calculate Frame (Block)
        clr.l   d0
        divu    #75,d1
        swap    d1
        move.b  d1,d0

        clr.w   d1                                                      ; Calculate Minute and Second
        swap    d1
        divu    #60,d1
        swap    d1
        lsl     #8,d1
        or.l    d1,d0

        restore d1
        rts


************************************************************************
***
*** MSFtoLSN
***
*** in:         D0 = (long)0x00MMSSFF
***
*** out:        D0 = (long)LSN
***
************************************************************************

 FUNCTION MSFtoLSN

        save    d1/d2

        swap    d0                                                      ; Calculate sector from minute
        move.w  d0,d1
        mulu    #60*75,d1

        swap    d0                                                      ; Add in sector from frame
        clr.l   d2
        move.b  d0,d2
        add.l   d2,d1

        lsr.w   #8,d0                                                   ; Add in sector from second
        mulu    #75,d0
        add.l   d1,d0

        restore d1/d2
        rts


*
************************************************************************
***
*** BCDtoBIN
***
*** in:
***
***     D0 = (byte)BCD
***
*** out:
***
***     D0 = (byte)BIN
***
***
************************************************************************

 FUNCTION BCDtoBIN

        save    d1

        move    d0,d1                                                   ; Isolate one's digit
        and     #$000f,d1

        lsr     #4,d0                                                   ; Isolate ten's digit
        and     #$000f,d0

        mulu    #10,d0                                                  ; Multiply ten's digit

        add     d1,d0                                                   ; Add one's digit

        restore d1
        rts


*
************************************************************************
***
***  InitIntr
***
***     in:     D1 = Priority
***             DB = Data
***             A0 = Code
***             A1 = Node
***
************************************************************************

 FUNCTION InitIntr
                move.b  #NT_INTERRUPT,LN_TYPE(a1)                       ; Set up interrupt node
                move.b  d1,LN_PRI(a1)
                move.l  db,IS_DATA(a1)
                move.l  a0,IS_CODE(a1)  
                lea     DevName(pc),a0
                move.l  a0,LN_NAME(a1)
                rts


************************************************************************
***
***  InitPort
***
***     in:     A0 = Message port
***             D0 = SignalBit
***
************************************************************************

 FUNCTION InitPort
                clr.b   MP_FLAGS(a0)                                    ; Initialize message port
                move.b  d0,MP_SIGBIT(a0)
                lea     db_Task(db),a1
                move.l  a1,MP_SIGTASK(a0)
                lea     MP_MSGLIST(a0),a1
                NEWLIST a1
                rts


************************************************************************
***
***  Disable
***
************************************************************************

 FUNCTION Disable
                save    a6                                              ; Disable interrupts
                DISABLE a6
                restore a6
                rts


************************************************************************
***
***  Enable
***
************************************************************************

 FUNCTION Enable
                save    a6                                              ; Enable interrupts
                ENABLE  a6
                restore a6
                rts


