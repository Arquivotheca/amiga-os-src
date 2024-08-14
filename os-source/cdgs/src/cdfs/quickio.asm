************************************************************************
****************                                        ****************
****************        -=   CD FILE SYSTEM   =-        ****************
****************                                        ****************
************************************************************************
***                                                                  ***
***   Written by Bryce Nesbitt for Commodore International, Ltd.     ***
***   Copyright (C) 1991 Commodore-Amiga, Inc. All rights reserved.  ***
***                                                                  ***
***         Confidential and Proprietary System Source Code          ***
***                                                                  ***
************************************************************************

************************************************************************
***
*** "Quick IO" module.  Handles Read & Seek commands on context
*** of caller, saving at least two task switches per transfer.
***
************************************************************************

    INCLUDE "exec/types.i"
    INCLUDE "exec/execbase.i"
    INCLUDE "exec/tasks.i"
    INCLUDE "exec/ports.i"
    INCLUDE "exec/ables.i"
    INCLUDE "dos/dosextens.i"

    XDEF    _Quick_SigTask

    XREF    _QuickSeekFile
    XREF    _QuickReadFile

_LVOPermit  EQU -138        ;!@#$@&* Manx
_LVOSignal  EQU -324        ;!@#$@&* Manx
_intena     EQU $dff09a     ;!@#$@&* Manx

    INT_ABLES   

*
*   We set the filesystem's message port's SigTask to point
*   to our code, and set the port arrival action to "secret direct
*   jump".
*
*   This code is called under DISABLE.  On entry A0 points to our code.
*   A1 points to the message _port_, which is assumed to be embeded in
*   the Process structure of the FileSystem.  A6 points to Exec.
*
*   We have to locate the pointer to the message (it will be the
*   last one on the message list).  If we're the only message, and
*   the task is in state TC_WAIT, we can procede.
*
*   We will almost always pass the only message/task in wait test,
*   so we check packet types first.  Bailing out quickly is important;
*   we can take a few extra cycles in each quick packet type and still
*   be ahead.
*
*   !WARNING! You may have to preserve A6 for calling Manx functions.
*   Some of the assembly functions in the Manx modules seem to assume
*   you can destroy A6!
*
*   !WARNING! PutMsg will do ENABLE after this returns!
*
        ;[A0-DosPacket *]
        ;[A1-MsgPort *]
qk_ACTION_READ: move.l  MP_MSGLIST+LH_TAILPRED(a1),d1    ;Last Msg is ours
        cmp.l   MP_MSGLIST+LH_HEAD(a1),d1    ;Any other msgs at
        bne.s   qk_signalD           ;port? Yes=bail...
        cmp.b   #TS_WAIT,TC_STATE-pr_MsgPort(a1) ;Task active?
        bne.s   qk_signalD           ;Yes=bail...

        ;[A0-Packet *]
        ;[A1-MsgPort *]
        ;[D1-Message]
        FORBID
        ENABLE
        movem.l a1/a4,-(sp)

* Now we need to figure out if the data is in the cache and finished.
* This must be as fast as possible - check SourLock last, as it almost
* never will fail.
* Must track changes to cdio.c! 
        move.l  TC_Userdata-pr_MsgPort(a1),a4    ;Manx global data
        move.l  a0,-(sp)             ;DosPacket *
        jsr _QuickReadFile
        addq.w  #4,sp
        movem.l (sp)+,a1/a4
        tst.l   d0               ;did it hit?
        beq.s   qk_signalE           ;failure (a1 valid)
        DISABLE
        JMP _LVOPermit(a6)  ;exit & take your task switch

* Putting this in the middle lets us use bcc.s

_Quick_SigTask: move.l  MP_MSGLIST+LH_TAILPRED(a1),a0   ;Last Msg is ours   
        move.l  LN_NAME(a0),a0          ;(DosPacket *)
        move.l  dp_Type(a0),d0          ;Randell says "long"
        moveq   #ACTION_READ,d1
        cmp.l   d1,d0
        beq.s   qk_ACTION_READ      
        cmp.l   #ACTION_SEEK,d0
        beq.s   qk_ACTION_SEEK      

qk_signalD: ;prevent task switches, then ENABLE to prevent MIDI problems
        FORBID
        ENABLE

qk_signalE: ;must have already Forbid/Enabled
        MOVE.B  MP_SIGBIT(A1),D1
        SUB.W   #pr_MsgPort,A1  ;Recover our task address
        MOVEQ.L #0,D0
        BSET.L  D1,D0
        JSR _LVOSignal(a6)  ;(A1,D0)
        DISABLE
        JMP _LVOPermit(a6)  ;exit & take your task switch

        ;[A0-DosPacket *]
        ;[A1-MsgPort *]
qk_ACTION_SEEK: ; move.w    #$0f00,$dff180  ;Flash Red

        move.l  MP_MSGLIST+LH_TAILPRED(a1),d1    ;Last Msg is ours
        cmp.l   MP_MSGLIST+LH_HEAD(a1),d1    ;Any other msgs at
        bne.s   qk_signalD           ;port? Yes=bail...
        cmp.b   #TS_WAIT,TC_STATE-pr_MsgPort(a1) ;Task active?
        bne.s   qk_signalD           ;Yes=bail...

        ;[A0-Packet *]
        ;[A1-MsgPort *]
        ;[D1-Message]
        FORBID
        ENABLE
        movem.l a1/a4,-(sp)
        move.l  TC_Userdata-pr_MsgPort(a1),a4    ;Manx global data
        move.l  a0,-(sp)             ;DosPacket *
        jsr _QuickSeekFile
        addq.l  #4,sp
        movem.l (sp)+,a1/a4
        tst.l   d0
        beq.s   qk_signalE          ; failed! in WaitIO!
        DISABLE
        JMP _LVOPermit(a6)  ;exit & take your task switch
