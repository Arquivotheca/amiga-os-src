************************************************************************
*     Envoy Parallel Portish Device Driver
*
*     Copyright (c) 1992 Commodore-Amiga, Inc.
*
*     $Id: epar.asm,v 1.2 93/07/21 14:46:36 jesup Exp $
*
*
************************************************************************

TIMEOUT         equ     5

                include "exec/types.i"
                include "exec/devices.i"
                include "exec/io.i"
                include "exec/memory.i"
                include "exec/initializers.i"
                include "exec/resident.i"
                include "exec/errors.i"
                include "libraries/expansion.i"
                include "libraries/configvars.i"
                include "libraries/configregs.i"
                include "intuition/intuition.i"
                include "envoy/nipc.i"
                include "envoy/services.i"
                include "envoy/errors.i"
                include "epar.i"
                include "dos/dos.i"
                include "dos/dosextens.i"
                include "dos/dostags.i"
                include "devices/parallel.i"

* External system stuff that we need:
                xref    _AbsExecBase,_kprintf
                xref    LoadConfig

                XSYS    FreeMem
                XSYS    CopyMem
                XSYS    AddTail
                XSYS    RemHead
                XSYS    Forbid
                XSYS    Permit
                XSYS    Disable
                XSYS    Enable
                XSYS    Remove
                XSYS    OpenLibrary
                XSYS    CloseLibrary
                XSYS    AllocMem
                XSYS    MakeFunctions
                XSYS    NextTagItem
                XSYS    ReplyMsg
                XSYS    CreateNewProc
                XSYS    PutMsg
                XSYS    WaitPort
                XSYS    CreateMsgPort
                XSYS    DeleteMsgPort
                XSYS    GetMsg
                XSYS    ReplyMsg
                XSYS    CreateEntityA
                XSYS    GetTransaction
                XSYS    BeginTransaction
                XSYS    AllocTransactionA
                XSYS    FreeTransaction
                XSYS    FindServiceA
                XSYS    LoseService
                XSYS    Wait
                XSYS    FindTask
                XSYS    ObtainSemaphore
                XSYS    InitSemaphore
                XSYS    ReleaseSemaphore
                XSYS    Signal
                XSYS    DeleteEntity
                XSYS    EasyRequestArgs

*
* In the event that somebody tries to execute this thing, fail.
*

AlwaysFail:
                moveq.l #-1,d0
                rts


*
* The ROMTAG:
*

Romtag:
                dc.w    RTC_MATCHWORD
                dc.l    Romtag
                dc.l    EndCode
                dc.b    RTF_AUTOINIT
                dc.b    VERSION
                dc.b    NT_DEVICE
                dc.b    0
                dc.l    DevName
                dc.l    IdString
                dc.l    InitTable
EndCode:

NIPCName                dc.b    'nipc.library',0
ServicesName            dc.b    'services.library',0
IntName                 dc.b    'intuition.library',0

DevName         dc.b    'envoyprint.device',0
IdString:
                VSTRING
                VERSTAG
                cnop    0,2


InitTable:
                dc.l    DevBaseDataSize
                dc.l    functable
                dc.l    datatable
                dc.l    InitCode

functable:
                dc.l    Open
                dc.l    Close
                dc.l    Expunge
                dc.l    0

                dc.l    BeginIO
                dc.l    AbortIO

                dc.l    -1

datatable:
                INITBYTE        LN_TYPE,NT_DEVICE
                INITLONG        LN_NAME,DevName
                INITBYTE        LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
                INITWORD        LIB_VERSION,VERSION
                INITWORD        LIB_REVISION,REVISION
                INITLONG        LIB_IDSTRING,IdString
                dc.w    0


*
* InitCode:
*  Called when the device is allocated and brought into memory.
*
*  Input:
*        D0    DevBase
*        A0    SegList for the device
*
*  Output:
*        D0    DevBase if successful, or NULL if failed.
*
*

InitCode:
                movem.l d1-d7/a0-a6,-(sp)
                move.l  d0,a6
                move.l  a0,ep_SegList(a6)
                move.l  _AbsExecBase,ep_SysBase(a6)
                moveq.l #0,d0
                move.l  d0,ep_NIPCBase(a6)
                move.l  d0,ep_ServicesBase(a6)
                move.l  d0,ep_DOSBase(a6)
                move.l  d0,ep_IntuitionBase(a6)
                move.l  d0,ep_OtherTask(a6)

                lea.l   ep_Units(a6),a0
                NEWLIST a0

                move.l  a6,-(sp)
                lea.l   ep_StopSign(a6),a0
                move.l  ep_SysBase(a6),a6
                SYS     InitSemaphore
                move.l  (sp),a6
                lea.l   ep_UnitSemaphore(a6),a0
                move.l  ep_SysBase(a6),a6
                SYS     InitSemaphore
                move.l  (sp)+,a6

                bra     1$

                movem.l a5/a6,-(sp)
                move.l  a6,a5
                move.l  ep_SysBase(a5),a6
                move.l  a5,a1
                moveq.l #0,d0
                move.w  LIB_NEGSIZE(a5),d0
                sub.l   d0,a1
                add.w   LIB_POSSIZE(a5),d0
                SYS     FreeMem
                movem.l (sp)+,a5/a6

99$
                movem.l (sp)+,d1-d7/a0-a6
                moveq.l #0,d0
                rts

1$              move.l  a6,d0
                movem.l (sp)+,d1-d7/a0-a6
                rts


*
*
* Open:
*  Called on each successive OpenDevice() call.
*
*  Input:
*        D0       Unit Number
*        D1       Flags
*        A1       IORequest
*        A6       DevBase
*
*  Output:
*        D0       00 for no error, else code
*
Open:

                movem.l a2,-(sp)
                addq.w  #1,LIB_OPENCNT(a6)
                bclr    #LIBB_DELEXP,ep_Flags(a6)
                tst.l   d0
                beq     11$
2$              subq.w  #1,LIB_OPENCNT(a6)      * decrement the open count (incremented above)
                moveq.l #IOERR_OPENFAIL,d0      * Failure to open;
                move.b  d0,IO_ERROR(a1)
                move.b  #NT_REPLYMSG,LN_TYPE(a1) * Mark IORequest as having completed
                movem.l (sp)+,a2
                rts
11$

                movem.l a1/a5/a6,-(sp)          * Open the nipc.library
                move.l  a6,a5
                move.l  ep_SysBase(a6),a6
                lea.l   NIPCName,a1
                moveq.l #0,d0
                move.b  #37,d0
                SYS     OpenLibrary
                tst.l   d0
                bne     12$
                moveq.l #0,d0                   * Alert user that nipc didn't open
                jsr     OFReq
14$             movem.l (sp)+,a1/a5/a6
                bra     2$
12$
                move.l  d0,ep_NIPCBase(a5)

                lea.l   ServicesName,a1         * Open services.library
                moveq.l #0,d0
                move.b  #37,d0
                SYS     OpenLibrary
                tst.l   d0
                bne     13$
                moveq.l #1,d0                   * Alert user that services didn't open
                jsr     OFReq
91$             move.l  ep_NIPCBase(a5),a1      * Close NIPC; back out
                SYS     CloseLibrary
                bra     14$
13$             move.l  d0,ep_ServicesBase(a5)

                lea.l   DOSName,a1              * Open DOS
                moveq.l #0,d0
                move.b  #37,d0
                SYS     OpenLibrary
                tst.l   d0
                bne     15$
                moveq.l #2,d0                   * Alert user that DOS didn't open (!)
                jsr     OFReq
92$             move.l  ep_ServicesBase(a5),a1  * Close services; back out
                SYS     CloseLibrary
                bra     91$
15$             move.l  d0,ep_DOSBase(a5)


                SYS     CreateMsgPort           * Create a MsgPort to be used as
                tst.l   d0                      * a replyport for communication with
                bne     16$                     * a secondary process
                moveq.l #3,d0                   * If that fails, alert user that it failed.
                jsr     OFReq
93$             move.l  ep_DOSBase(a5),a1       * Close DOS, back out another level
                SYS     CloseLibrary
                bra     92$
16$
                move.l  d0,-(sp)                * Save MsgPort

                lea.l   IntName,a1              * Open Intuition
                moveq.l #0,d0
                move.b  #37,d0
                SYS     OpenLibrary
                tst.l   d0
                bne     20$
94$             move.l  (sp)+,a0                * Close our ReplyPort, and back out
                move.l  ep_SysBase(a5),a6
                SYS     DeleteMsgPort
                bra     93$
20$             move.l  d0,ep_IntuitionBase(a5)

                lea.l   ep_StopSign(a5),a0      * Lock this section of code against
                SYS     ObtainSemaphore         * several callers at once

                tst.l   ep_OtherTask(a5)        * If 2nd task is already up, don't start it
                bne     18$

                lea.l   NPTags,a0               * Start secondary process;
                move.l  a0,d1
                move.l  ep_DOSBase(a5),a6
                SYS     CreateNewProc
                tst.l   d0
                bne     17$
                moveq.l #4,d0                   * If that failed, notify user
                jsr     OFReq
35$             move.l  ep_IntuitionBase(a5),a1 * Close intuition and back out another leve
                move.l  ep_SysBase(a5),a6
                SYS     CloseLibrary
                bra     94$

17$             move.l  d0,ep_OtherTask(a5)     * post startup message to other process
                add.l   #pr_MsgPort,d0
                move.l  d0,a0
                lea.l   ep_GPMsg(a5),a1
                move.l  (sp),MN_REPLYPORT(a1)
                move.l  ep_SysBase(a5),a6
                move.l  a5,xm_DevBase(a1)
                SYS     PutMsg
                move.l  (sp),a0
                SYS     WaitPort                * And wait for response
                move.l  (sp),a0
                SYS     GetMsg

18$
                lea.l   ep_StopSign(a5),a0
                SYS     ReleaseSemaphore

                movem.l 4(sp),a1/a5/a6
                move.l  a6,a5
                jsr     OpenUnit
                movem.l 4(sp),a1/a5/a6
                move.l  a6,a5
                tst.l   d0
                beq     19$

* Icky!  Shut down the other process.  We do this by sending them a message,
* and waiting for a response to come back.  After that point, we can assume
* that it's safely dead.
                move.w  LIB_OPENCNT(a5),d0
                cmp.w   #1,d0
                bne     19$
                sub.l   #MN_SIZE,sp
                move.l  sp,a1
                move.l  ep_OtherTask(a5),d0
                move.l  #0,ep_OtherTask(a5)     * mark it as "down"
                add.l   #pr_MsgPort,d0
                move.l  d0,a0
                move.l  MN_SIZE(sp),MN_REPLYPORT(a1)

                move.l  ep_SysBase(a5),a6
                SYS     PutMsg
                move.l  MN_SIZE(sp),a0
                SYS     WaitPort
                add.l   #MN_SIZE,sp

                bra     35$                     * Drop back out, closing libs and -stuff- as we go.

19$             move.l  (sp)+,a0
                move.l  ep_SysBase(a5),a6
                SYS     DeleteMsgPort

                movem.l (sp)+,a1/a5/a6


                move.l  a6,IO_DEVICE(a1)

                moveq.l #0,d0
                move.b  d0,IO_ERROR(a1)
                move.b  #NT_REPLYMSG,LN_TYPE(a1)

                movem.l (sp)+,a2
                rts

*
* Close
*  Called every time somebody calls CloseDevice()
*
*  Input:
*        A1       IORequest
*        A6       DevBase
*
*  Output:
*        D0       NULL if the device cannot be unloaded, or
*                 SegList if it has been expunged.
*
Close:

                moveq.l #-1,d0
                move.l  d0,IO_DEVICE(a1)

                moveq.l #0,d0
                sub.w   #1,LIB_OPENCNT(a6)
                bne     29$


* If the open count drops to zero, dump the parallel task.

                movem.l a5/a6,-(sp)
                move.l  a6,a5
                move.l  ep_SysBase(a5),a6

                jsr     CloseUnit

* If I can't allocate a replyport, the parallel process won't stop.
* If it doesn't stop, this library will NOT expunge.

                SYS     CreateMsgPort
                tst.l   d0
                beq     9$
                move.l  d0,-(sp)

                lea.l   ep_StopSign(a5),a0
                SYS     ObtainSemaphore

                tst.w   LIB_OPENCNT(a5)
                bne     9$

                move.l  ep_OtherTask(a5),d0
                beq     9$

                add.l   #pr_MsgPort,d0
                move.l  d0,a0
                lea.l   ep_GPMsg(a5),a1
                move.l  (sp),MN_REPLYPORT(a1)
                SYS     PutMsg

                move.l  (sp),a0
                SYS     WaitPort
                move.l  (sp)+,a0
                SYS     DeleteMsgPort

                moveq.l #0,d0
                move.l  d0,ep_OtherTask(a5)


9$              move.l  ep_SysBase(a5),a6
                lea.l   ep_StopSign(a5),a0
                SYS     ReleaseSemaphore
                movem.l (sp)+,a5/a6

29$             movem.l a5/a6,-(sp)
                move.l  a6,a5
                move.l  ep_SysBase(a5),a6
                move.l  ep_NIPCBase(a5),a1
                SYS     CloseLibrary
                move.l  ep_ServicesBase(a5),a1
                SYS     CloseLibrary
                move.l  ep_DOSBase(a5),a1
                SYS     CloseLibrary
                move.l  ep_IntuitionBase(a5),a1
                SYS     CloseLibrary
                movem.l (sp)+,a5/a6

                btst    #LIBB_DELEXP,ep_Flags(a6)
                beq     1$
                bsr     Expunge
1$
                rts

*
* Expunge
*  Called when the Exec memory system runs low on memory;
*  If nobody has this device open, out it goes.
*
*  Input:
*        A6       DevBase
*
*  Output:
*        D0       NULL if cannot be expunged
*                 SegList if it can.
*
Expunge:


                tst.w   LIB_OPENCNT(a6)
                beq     1$
2$              bset    #LIBB_DELEXP,ep_Flags(a6)
                moveq.l #0,d0
                rts
1$              tst.l   ep_OtherTask(a6)
                bne     2$

                movem.l a4/a5/a6,-(sp)
                move.l  a6,a5
                move.l  ep_SysBase(a6),a6
                move.l  ep_SegList(a5),a4

                move.l  a5,a1
                SYS     Remove


                move.l  a5,a1
                moveq.l #0,d0
                move.w  LIB_NEGSIZE(a5),d0
                sub.l   d0,a1
                add.w   LIB_POSSIZE(a5),d0
                SYS     FreeMem

                move.l  a4,d0
                movem.l (sp)+,a4/a5/a6
                rts



**************
** OpenUnit **
**************
**
** Creates a Unit structure, attempts to do a FindService(),
** and attempts to send a Start command to the server.
**
** Entry:
**      A1 - IORequest used to open
**      A5 - Ptr to Device Base
**
** Exit:
**      D0 - Result
**              00 - Success
**              NZ - Failure
**
OpenUnit:

                movem.l a2/a3/a1/a6,-(sp)
                move.l  #eu_SIZE,d0
                move.l  #MEMF_CLEAR+MEMF_PUBLIC,d1
                move.l  ep_SysBase(a5),a6
                move.l  a1,a2
                SYS     AllocMem
                tst.l   d0
                bne     1$
                moveq.l #1,d0
                bra     99$
1$
                movem.l d0-d1/a0-a1,-(sp)
                lea.l   ep_UnitSemaphore(a5),a0
                SYS     ObtainSemaphore
                movem.l (sp)+,d0-d1/a0-a1


                move.l  d0,a3
                move.l  d0,a1
                move.l  d0,IO_UNIT(a2)
                lea.l   eu_IORequests(a3),a0
                NEWLIST a0

                lea.l   ep_Units(a5),a0
                SYS     AddTail

                movem.l d0-d1/a0-a1,-(sp)
                lea.l   ep_UnitSemaphore(a5),a0
                SYS     ReleaseSemaphore
                movem.l (sp)+,d0-d1/a0-a1

* Unit structure has been created . . .

* Now, try that FindService()!

                movem.l a2/a3,-(sp)
                move.l  ep_ServicesBase(a5),a6
                lea.l   ep_HostName(a5),a0
                lea.l   SName,a1
                move.l  ep_LocalEntity(a5),a2
                move.l  a2,d0
                tst.l   d0
                bne     199$
                movem.l (sp)+,a2/a3
                bra     99$
199$

                sub.l   #4,sp
                move.l  sp,d1
                sub.l   #7*4,sp
                move.l  a0,d0
                move.l  sp,a3

                move.l  #FSVC_UserName,(a3)+
                lea.l   ep_UserName(a5),a0
                move.l  a0,(a3)+
                move.l  #FSVC_PassWord,(a3)+
                lea.l   ep_Password(a5),a0
                move.l  a0,(a3)+
                move.l  #FSVC_Error,(a3)+
                move.l  d1,(a3)+
                move.l  #TAG_DONE,(a3)+
                move.l  sp,a3
                move.l  d0,a0
                SYS     FindServiceA
                add.l   #7*4,sp
                move.l  (sp)+,d1

                movem.l (sp)+,a2/a3
                move.l  d0,eu_RemoteEntity(a3)
                tst.l   d0
                bne     4$
                jsr     EZ
                moveq.l #1,d0
                bra     99$
4$

* Send a Start packet
                move.l  ep_NIPCBase(a5),a6
                lea.l   TRNTags1,a0
                SYS     AllocTransactionA
                tst.l   d0
                bne     2$
9$              moveq.l #1,d0
                bra     9$
2$              move.l  d0,a2
                move.b  #LPCMD_START_PRT,trans_Command(a2)
                move.w  #3,trans_Timeout(a2)
                move.l  eu_RemoteEntity(a3),a0
                bsr     DoContextTransaction

                tst.b   d0
                bne     7$
                tst.l   trans_Error(a2)
                bne     7$
                move.l  trans_ResponseData(a2),a0
                move.l  (a0),eu_Job(a3)

                move.l  a2,a1
                move.l  ep_NIPCBase(a5),a6
                SYS     FreeTransaction

                moveq.l #0,d0
                bra     99$

7$              move.l  a2,a1
                move.l  ep_NIPCBase(a5),a6
                SYS     FreeTransaction

99$             movem.l (sp)+,a1/a2/a3/a6
                rts


***************
** CloseUnit **
***************
** Loses the found service, deletes the Unit structure.
**
** Entry:
**      A1 - Ptr to IORequest
**      A5 - ptr to Device Base
**
** Exit:
**      none
CloseUnit:
                movem.l     a1/a2/a6/a3,-(sp)

                move.l  ep_SysBase(a5),a6
                movem.l d0-d1/a0-a1,-(sp)
                lea.l   ep_UnitSemaphore(a5),a0
                SYS     ObtainSemaphore
                movem.l (sp)+,d0-d1/a0-a1

                move.l  IO_UNIT(a1),a3
                move.l  a1,-(sp)
                move.l  a3,a1
                SYS     Remove
                move.l  (sp)+,a1

                movem.l d0-d1/a0-a1,-(sp)
                lea.l   ep_UnitSemaphore(a5),a0
                SYS     ReleaseSemaphore
                movem.l (sp)+,d0-d1/a0-a1


* Issue a STOP command
                move.l  ep_NIPCBase(a5),a6
                lea.l   TRNTags2,a0
                SYS     AllocTransactionA
                tst.l   d0
                bne     2$
9$              moveq.l #1,d0
                bra     9$
2$              move.l  d0,a2
                move.l  trans_RequestData(a2),a0
                move.l  eu_Job(a3),(a0)
                tst.l   eu_Job(a3)
                beq     75$
                move.b  #LPCMD_END,trans_Command(a2)
                move.w  #3,trans_Timeout(a2)
                move.l  eu_RemoteEntity(a3),a0
                bsr     DoContextTransaction
                tst.b   d0
                bne     7$
                tst.l   trans_Error(a2)
                bne     7$

75$             move.l  a2,a1
                move.l  ep_NIPCBase(a5),a6
                SYS     FreeTransaction
                moveq.l #0,d0
                bra     99$

7$              move.l  a2,a1
                move.l  ep_NIPCBase(a5),a6
                SYS     FreeTransaction
99$

                move.l      ep_ServicesBase(a5),a6
                move.l      eu_RemoteEntity(a3),a0
                SYS         LoseService

                moveq.l     #0,d0
                move.l      d0,eu_RemoteEntity(a3)

                move.l      a3,a1
                move.l      #eu_SIZE,d0
                move.l      ep_SysBase(a5),a6
                SYS         FreeMem

                movem.l     (sp)+,a1/a2/a6/a3
                rts


**************************
** DoContextTransaction **
**************************
**
** This function effectively performs a DoTransaction(), but also
** works with the mn_ReplyPort mechanism.  So -- even though the
** Source Entity is owned by another task, we ask that all
** transactions get returned to our own message port, rather than
** to the Entity itself.
**
**
** Entry:
**          A0 - Remote Entity
**          A2 - Transaction
**          A5 - Device Base
**
** Exit:    D0 - Result
**                  00 - Success
**                  NZ - Failure (couldn't allocate memory?)
**                  trans_Error will also be valid.
**
DoContextTransaction:

            movem.l     a4/a6,-(sp)
            move.l      a0,a4
            move.l      ep_SysBase(a5),a6
            SYS         CreateMsgPort
            tst.l       d0
            beq         9$
            move.l      d0,-(sp)
            move.l      d0,MN_REPLYPORT(a2)     * Tell trans to come back HERE.
            move.l      ep_NIPCBase(a5),a6
            move.l      a4,a0
            move.l      ep_LocalEntity(a5),a1
            SYS         BeginTransaction

1$          moveq.l     #0,d0
            move.l      d0,d1
            move.l      (sp),a0
            move.b      MP_SIGBIT(a0),d1
            bset        d1,d0
            move.l      ep_SysBase(a5),a6
            SYS         Wait

            move.l      (sp),a0
            SYS         GetMsg
            tst.l       d0
            beq         1$

            move.l      (sp)+,a0
            SYS         DeleteMsgPort
            moveq.l     #0,d0
            bra         99$

9$          moveq.l     #1,d0
99$         movem.l     (sp)+,a6/a4
            rts




********************
** PrinterProcess **
********************
**
** The entry point of the parallel printer process.
** The printer process basically sits around with the sole
** purpose of GetTransaction()'ing and FreeTransaction()'ing
** returned transactions.  (And IORequests.)
**
** NOTE:  While a process, because I send messages (specifically the
** shutdown message) through the Process port, THIS CODE MAY NOT USE
** DOS CALLS.
**
PrinterProcess:

* First, Wait for the startup message, and get the device
* base from there.

                sub.l   #lv_SIZE,sp
                move.l  sp,a4
                moveq.l #0,d0
                move.l  d0,lv_LocalEntity(a4)

                move.l  _AbsExecBase,a6
                moveq.l #0,d0
                move.l  d0,a1
                SYS     FindTask
                move.l  d0,lv_OurTask(a4)
                add.l   #pr_MsgPort,d0
                move.l  d0,a0
                move.l  a0,-(sp)
87$             SYS     WaitPort
                move.l  (sp),a0
                SYS     GetMsg
                tst.l   d0
                bne     88$
                move.l  (sp),a0
                bra     87$
88$             addq.l  #4,sp
                move.l  d0,a1
                move.l  d0,-(sp)
                move.l  xm_DevBase(a1),a5

                jsr     LoadConfig

* Create local Entity
                sub.l   #4,sp
                move.l  ep_NIPCBase(a5),a6
                lea.l   ETags,a0
                move.l  sp,4(a0)
                SYS     CreateEntityA
                move.l  (sp)+,d1
                move.l  d1,lv_SigBit(a4)
                tst.l   d0
                bne     1$
                move.l  (sp),a1
                move.l  #0,xm_DevBase(a1)
                bra     9$
1$              move.l  d0,ep_LocalEntity(a5)
                move.l  d0,lv_LocalEntity(a4)
* Startup
                move.l  ep_NIPCBase(a5),lv_NIPCBase(a4)
                move.l  ep_SysBase(a5),lv_SysBase(a4)


                move.l  (sp),a1
                move.l  #-1,xm_DevBase(a1)
9$              move.l  (sp)+,a1
                move.l  xm_DevBase(a1),d4
                move.l  ep_SysBase(a5),a6
                SYS     ReplyMsg
                tst.l   d4
                beq     999$
* No longer permitted to touch the device base now that the ReplyMsg has occurred.

100$
                move.l  lv_SysBase(a4),a6
                moveq.l #0,d0
                moveq.l #0,d1
                move.l  lv_OurTask(a4),a0
                move.b  MP_SIGBIT+pr_MsgPort(a0),d1
                move.l  d1,d2
                bset    d1,d0
                move.l  lv_SigBit(a4),d1
                move.l  d1,d3
                bset    d1,d0
                SYS     Wait
                btst    d2,d0           * Was it a MsgPort signal?
                bne     101$
                btst    d3,d0           * Was it an Entity signal?
                beq     100$
101$

800$            move.l  lv_NIPCBase(a4),a6
                move.l  lv_LocalEntity(a4),a0
                SYS     GetTransaction
                tst.l   d0
                beq     900$

* Got a returned transaction!  Try to figure out who the owner is . . .
* Need to:
* Go through the Unit list, checking every open unit's list of iorequests
* for one with a matching transaction ptr (faked in IO_ACTUAL).  When
* found, yank that IORequest, and return the darned thing.
* This could get slow under VERY heavy use.

                move.l  d0,a3
                lea.l   ep_UnitSemaphore(a5),a0
                move.l  lv_SysBase(a4),a6
                SYS     ObtainSemaphore

                move.l  ep_Units+LH_HEAD(a5),a0
598$            tst.l   LN_SUCC(a0)
                beq     600$

                move.l  eu_IORequests(a0),a1
597$            tst.l   LN_SUCC(a1)
                beq     599$

                move.l  IO_ACTUAL(a1),d1

                cmp.l   a3,d1
                beq     550$

                move.l  LN_SUCC(a1),a1
                bra     597$

599$            move.l  LN_SUCC(a0),a0
                bra     598$


550$            move.l  a1,a2
                SYS     Remove
                move.l  a5,IO_ACTUAL(a2)
                move.b  #0,IO_ERROR(a2)

                move.l  trans_Error(a3),d0
                beq     551$

                move.b  #ParErr_LineErr,IO_ERROR(a2)

551$            movem.l a4-a6,-(sp)
                move.l  a5,a6
                move.l  a2,a1
                jsr     ReplyRequest
                movem.l (sp)+,a4-a6

600$

                lea.l   ep_UnitSemaphore(a5),a0
                move.l  lv_SysBase(a4),a6
                SYS     ReleaseSemaphore
                bra     800$


900$            move.l  ep_SysBase(a5),a6
                move.l  lv_OurTask(a4),a0
                add.l   #pr_MsgPort,a0
                SYS     GetMsg

                tst.l   d0
                beq     100$
                move.l  d0,a2

                move.l  lv_LocalEntity(a4),a0
                move.l  lv_NIPCBase(a4),a6
                SYS     DeleteEntity

                move.l  lv_SysBase(a4),a6
                SYS     Forbid
                move.l  a2,a1
                SYS     ReplyMsg

999$
                add.l   #lv_SIZE,sp
                moveq.l #0,d0
                rts





*************
** BeginIO **
*************
**
** The only entry point for the IORequests.
** If QuickIO is requested, and the IORequest can be immediately
** serviced, BeginIO() will return with the IORequest
** completed.  If not, the quick IO bit will be cleared on
** exit, and the caller should expect the iorequest to return on
** the replyport.
**
** Entry:
**          A1 - ptr to the IORequest
**          A6 - ptr to the epar device's device base
**
** Exit:
**
BeginIO:

                bclr.b  #IOB_QUICK,IO_FLAGS(a1)
                move.b  #NT_MESSAGE,LN_TYPE(a1)

                moveq.l #0,d0
                move.w  IO_COMMAND(a1),d0

                cmp.w   #CMD_FLUSH,d0
                bhi     1$
                lsl.l   #2,d0
                lea.l   CommandTable,a0
                add.l   d0,a0
                tst.l   (a0)
                beq     1$
                move.l  (a0),a0
                move.b  #0,IO_ERROR(a1)
                jmp     (a0)
1$              move.b  #IOERR_NOCMD,IO_ERROR(a1)
11$             bra     ReplyRequest



******************
** ReplyRequest **
******************
** Return an IORequest to the caller.
** Takes care not to ReplyMsg IORequests that were QuickIO.
**
** Entry:
**          A1 - ptr to IORequest
**          A6 - ptr to DeviceBase
**
** Exit:
**          none
**
ReplyRequest:
                move.l  IO_LENGTH(a1),IO_ACTUAL(a1)
                move.b  #NT_REPLYMSG,LN_TYPE(a1)
                btst.b  #IOB_QUICK,IO_FLAGS(a1)
                bne     1$
                move.l  a6,-(sp)
                move.l  ep_SysBase(a6),a6
                SYS     ReplyMsg
                move.l  (sp)+,a6
1$
                rts


AbortAll:
                rts


*************
** AbortIO **
*************
**
** Abort an IORequest.
**
** Entry:
**          A1 - ptr to IORequest
**          A6 - ptr to device base
**
** Exit:
**          none
**
AbortIO:

                move.l  a6,-(sp)
                move.l  ep_SysBase(a6),a6
                move.l  a1,-(sp)
                SYS     Disable
                move.l  (sp),a1
                cmp.b   #NT_MESSAGE,LN_TYPE(a1)
                bne     1$
                SYS     Remove
                move.l  (sp)+,a1
                move.b  #IOERR_ABORTED,IO_ERROR(a1)
                SYS     ReplyMsg
                bra     2$
1$              addq.l  #4,sp
2$              SYS     Enable
                move.l  (sp)+,a6
                rts


***********
** Flush **
***********
**
** Abort all IORequests.
**
** Entry:
**          A1 - ptr to iorequest
**
** Exit:
**          none
**
Flush:
                movem.l a2/a1/a6,-(sp)
                move.l  IO_UNIT(a1),a2
                move.l  ep_SysBase(a6),a6
                bsr     AbortAll
                movem.l (sp)+,a1/a2/a6
                jsr     ReplyRequest
                rts

CommandTable:
                dc.l    0
                dc.l    0
                dc.l    0
                dc.l    Write
                dc.l    0
                dc.l    0
                dc.l    0
                dc.l    0
                dc.l    Flush


***********
** Write **
***********
** Handles all CMD_WRITE's by allocating a Transaction (if possible) for
** the IORequest, filling in the appropriate fields from the Unit structure,
** and BeginTransaction()'ing it from the Entity owned by the secondary
** process, to the entity found at OpenDevice() time.
**
** Entry:
**          A1  - IORequest
**          A6  - Device Base
**
** Exit:
**          io_Error becomes valid
**
**
Write:

                movem.l a2/a3/a5/a6,-(sp)
                move.l  IO_UNIT(a1),a3
                tst.l   eu_Job(a3)
                bne     82$
                move.l  a1,a3
                move.l  a6,a5
                bra     81$
82$             move.l  IO_DEVICE(a1),a6
                move.l  a1,a3
                move.l  a6,a5
                move.l  ep_NIPCBase(a5),a6
                move.l  IO_LENGTH(a1),d0
                addq.l  #4,d0
                sub.l   #12,sp
                move.l  sp,a0
                move.l  #TRN_AllocReqBuffer,0(a0)
                move.l  d0,4(a0)
                move.l  #TAG_DONE,8(a0)
                SYS     AllocTransactionA
                add.l   #12,sp
                tst.l   d0
                bne     80$
81$             move.b  #ParErr_LineErr,IO_ERROR(a3)
                bra     90$
80$
                move.l  IO_DATA(a3),a0
                move.l  d0,a2
                move.l  trans_RequestData(a2),a1
                move.l  a0,-(sp)
                move.l  IO_UNIT(a3),a0
                move.l  eu_Job(a0),(a1)
                move.l  (sp)+,a0
                add.l   #4,a1
                move.l  IO_LENGTH(a3),d0
                addq.l  #4,d0
                move.l  d0,trans_ReqDataActual(a2)
                subq.l  #4,d0
                move.l  ep_SysBase(a5),a6
                SYS     CopyMem
                move.b  #LPCMD_DATA,trans_Command(a2)
                move.w  #TIMEOUT,trans_Timeout(a2)

* link the structs together . . .
                move.l  a2,IO_ACTUAL(a3)            * I'll replace this value later

* Add this IORequest to the Unit list

                movem.l d0-d1/a0-a1,-(sp)
                lea.l   ep_UnitSemaphore(a5),a0
                SYS     ObtainSemaphore

                move.l  IO_UNIT(a3),a0
                lea.l   eu_IORequests(a0),a0
                move.l  a3,a1
                SYS     AddTail

                lea.l   ep_UnitSemaphore(a5),a0
                SYS     ReleaseSemaphore
                movem.l (sp)+,d0-d1/a0-a1

* Send off the Transaction
                move.l  IO_UNIT(a3),a0
                move.l  eu_RemoteEntity(a0),a0
                move.l  ep_LocalEntity(a5),a1
                move.l  ep_NIPCBase(a5),a6
                SYS     BeginTransaction
                bra     99$

90$             move.l  a3,a1
                move.l  a5,a6
                jsr     ReplyRequest
99$             movem.l (sp)+,a2/a3/a5/a6

                rts

EZ:
                movem.l d0-d7/a0-a6,-(sp)
                sub.l   #es_SIZEOF,sp
                move.l  sp,a0
                move.l  #es_SIZEOF,es_StructSize(a0)
                move.l  #0,es_Flags(a0)
                lea.l   EZName,a1
                move.l  a1,es_Title(a0)
                lea.l   EZText1,a1
                cmp.w   #ENVOYERR_TIMEOUT,d1
                bhi     1$
                cmp.w   #ENVOYERR_UNKNOWNHOST,d1
                blo     1$
                lea.l   EZText,a1
1$              move.l  a1,es_TextFormat(a0)
                lea.l   EZGadget,a1
                move.l  a1,es_GadgetFormat(a0)

                moveq.l #0,d0
                move.l  a0,a1
                move.l  d0,a0
                move.l  ep_IntuitionBase(a5),a6
                SYS     EasyRequestArgs
                add.l   #es_SIZEOF,sp
                movem.l (sp)+,d0-d7/a0-a6
                rts

OFReq:
                movem.l d0-d7/a0-a6,-(sp)
                move.l  d0,d2
                move.l  ep_SysBase(a5),a6
                lea.l   IntName,a1
                moveq.l #0,d0
                SYS     OpenLibrary
                tst.l   d0
                beq     999$
                move.l  d0,a6
                moveq.l #0,d0

                sub.l   #es_SIZEOF,sp
                move.l  sp,a1
                move.l  #es_SIZEOF,es_StructSize(a1)
                move.l  #0,es_Flags(a1)
                lea.l   OFTitle,a0
                move.l  a0,es_Title(a1)
                lea.l   OFText,a0
                asl.l   #2,d2
                move.l  0(a0,d2.l),a0
                move.l  a0,es_TextFormat(a1)
                lea.l   OFGadget,a0
                move.l  a0,es_GadgetFormat(a1)

                moveq.l #0,d0
                move.l  d0,a0
                SYS     EasyRequestArgs
                add.l   #es_SIZEOF,sp

                move.l  a6,a1
                move.l  ep_SysBase(a5),a6
                SYS     CloseLibrary

999$            movem.l (sp)+,d0-d7/a0-a6
                rts


OFTitle         dc.b    'EnvoyPrint.device',0

                cnop    0,2
OFText          dc.l    0$,1$,2$,3$,4$,5$

0$              dc.b    'Couldn''t open nipc.library',0
1$              dc.b    'Couldn''t open services.library',0
2$              dc.b    'Couldn''t open dos.library',0
3$              dc.b    'Couldn''t CreateMsgPort()',0
4$              dc.b    'Couldn''t CreateNewProc()',0
5$              dc.b    'Printer Process Error',0


EZName          dc.b    'Envoy Printer',0
EZText1         dc.b    'Permission denied to',10
                dc.b    'access that printer',10,0
EZText          dc.b    'Failed to contact',10
                dc.b    'remote printer server',0
OFGadget:
EZGadget        dc.b    'OK',0


                cnop    0,2
NPTags          dc.l    NP_Entry,PrinterProcess,NP_Name,NPName,TAG_DONE,0
NPName          dc.b    'EnvoyPrinter',0
                cnop    0,2
ETags           dc.l    ENT_AllocSignal,0,TAG_DONE,0
DOSName         dc.b    'dos.library',0
                cnop    0,2
TRNTags1        dc.l    TRN_AllocRespBuffer,4,TAG_DONE,0
TRNTags2        dc.l    TRN_AllocReqBuffer,4,TAG_DONE,0
SName           dc.b    'Print Spooler',0

                end

