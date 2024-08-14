**
** A2065 SANA-II Device Driver source
** Copyright (c) 1992 by Commodore-Amiga, Inc.
**
** All rights Reserved.
**


;DEBUG_MODE     EQU     1

        nolist

        include "includes.asm"
        include "globals.i"

        list

        XDEF    GetTime

        STRUCTURE       MyAllocation,0
        STRUCT          TimerRequest,IOTV_SIZE
        STRUCT          MessagePort,MP_SIZE
        LABEL           MY_SIZE


        CNOP    2

;----------------------------------------------------------------------------
; GetTime - Gets A TimeVal In A Completely Self-Contained Way
;----------------------------------------------------------------------------
; IN:   A6      SysBase
;       A0      Pointer To Place To The TimeVal
;
; USE:  A2      Store the pointer which came in in A0
;       A3      Pointer To A TimerRequest Block
;       A4      Pointer To A Message Port
;----------------------------------------------------------------------------
; NOTES:
;----------------------------------------------------------------------------


GetTime movem.l A2-A4,-(SP)

        move.l  A0,A2
        clr.l   TV_SECS(A2)
        clr.l   TV_MICRO(A2)

        ; Attempt to allocate space for a TimerRequest And MessagePort

        move.l  #MEMF_PUBLIC|MEMF_CLEAR,D1
        move.l  #MY_SIZE,D0
        CALL    AllocMem
        tst.l   D0
        beq     999$

        ; Memory for this routine has been allocated.
        ;
        ; A3 will point to the TimerRequest IO Block
        ; A4 will point to the message port.

        move.l  D0,A3
        lea     MessagePort(A3),A4

        ; Allocate a signal to be used for the Message Port

        moveq.l #-1,D0
        CALL    AllocSignal
        cmp.b   #-1,D0
        beq.s   990$

        ; Signal has been allocated - put it into the Message Port

        move.b  D0,MP_SIGBIT(A4)

        ; Initialize the rest of the Message Port

        move.b  #NT_MSGPORT,LN_TYPE(A4)
        move.b  #PA_SIGNAL,MP_FLAGS(A4)
        suba.l  A1,A1
        CALL    FindTask
        move.l  D0,MP_SIGTASK(A4)
        lea     MP_MSGLIST(A4),A0
        NEWLIST A0

        ; The MsgPort is defined, now finish defining the IOB

        move.l  A4,MN_REPLYPORT(A3)
        move.w  #IOTV_SIZE,MN_LENGTH(A3)

        ; The timer device is now ready to be opened. Set up the call.

        lea     TName,A0
        moveq.l #UNIT_VBLANK,D0
        move.l  A3,A1
        moveq.l #0,D1
        CALL    OpenDevice
        tst.l   D0
        bne.s   980$

        ; The Timer is now open, get the SystemTime

        move.w  #TR_GETSYSTIME,IO_COMMAND(A3)
        move.b  #IOF_QUICK,IO_FLAGS(A3)
        move.l  A3,A1
        CALL    DoIO
        tst.b   IO_ERROR(A3)
        bne.s   970$

        move.l  IOTV_TIME(A3),(A2)
        move.l  IOTV_TIME+4(A3),4(A2)

970$;   ; This Closes the TimerDevice

        move.l  A3,A1
        CALL    CloseDevice

980$    ; This frees the signal which was allocated for the port

        moveq.l #0,D0
        move.b  MP_SIGBIT(A4),D0
        CALL    FreeSignal

990$    ; This frees both the TimerRequest and the Message Port

        move.l  #MY_SIZE,D0
        move.l  A3,A1
        CALL    FreeMem

999$

        movem.l (SP)+,A2-A4
        rts

TName   TIMERNAME


        END
