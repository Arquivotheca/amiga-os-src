
*************************************************************************
*                                                                       *
*       Copyright (C) 1992, Commodore Amiga Inc.  All rights reserved.  *
*                                                                       *
*************************************************************************


        INCLUDE "exec/types.i"
        INCLUDE "exec/resident.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "exec/ports.i"
        INCLUDE "exec/lists.i"
        INCLUDE "internal.i"
        INCLUDE "freeanim_rev.i"

        SECTION FreeAnimLibrary

        moveq.l #-1,d0           ; Not executable
        rts


ROMTAG:                          ;STRUCTURE RT,0
        dc.w    RTC_MATCHWORD    ; UWORD RT_MATCHWORD
        dc.l    ROMTAG           ; APTR  RT_MATCHTAG
        dc.l    EndCode          ; APTR  RT_ENDSKIP
        dc.b    RTF_COLDSTART    ; UBYTE RT_FLAGS
        dc.b    VERSION          ; UBYTE RT_VERSION
        dc.b    NT_LIBRARY       ; UBYTE RT_TYPE
        dc.b    0                ; BYTE  RT_PRI
        dc.l    FreeAnimName     ; APTR  RT_NAME
        dc.l    VERSTRING        ; APTR  RT_IDSTRING
        dc.l    InitModule       ; APTR  RT_INIT
                                 ; LABEL RT_SIZE

FreeAnimName:
        dc.b    "freeanim.library",0
        ds.w    0

Startup_Animation:
        dc.b    "Startup Animation",0
        ds.w    0

VERSTRING:
                VSTRING

VERNUM: EQU     VERSION
REVNUM: EQU     REVISION

        ds.w    0



 STRUCTURE FreeLib,LIB_SIZE

    STRUCT FL_ReplyPort,MP_SIZE
    STRUCT FL_Message,MN_SIZE
    LABEL  FL_SIZE



*------ Functions Offsets -------------------------------------

initFunc:
        dc.l    Open
        dc.l    Close
        dc.l    Expunge
        dc.l    Reserved
        dc.l    -1


*------ Initializaton Table -----------------------------------

initStruct:
        INITLONG        LIB_IDSTRING,VERSTRING
        INITLONG        LN_NAME,FreeAnimName
        INITWORD        LIB_VERSION,VERNUM
        INITWORD        LIB_REVISION,REVNUM
        INITBYTE        LN_TYPE,NT_LIBRARY
        INITBYTE        LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
        dc.l            0


InitModule:
        save    a2/a6                               ; Make a library
        lea     initFunc(pc),a0
        lea     initStruct(pc),a1
        suba.l  a2,a2
        move.l  #FL_SIZE,d0
        clr.l   d1
        exec    MakeLibrary
        move.l  d0,a2
        tst.l   d0
        beq.s   1$

        lea     FL_ReplyPort(a2),a1                 ; Initialize message port
        move.b  #NT_MSGPORT,LN_TYPE(a1)
        clr.b   MP_FLAGS(a1)
        lea     MP_MSGLIST(a1),a0
        NEWLIST a0

        lea     FL_Message(a2),a0                   ; Initialize message
        move.l  a1,MN_REPLYPORT(a0)
        clr.w   MN_LENGTH(a0)                       ; - 0 means shutdown

        move.l  a2,a1                               ; Add the library to system
        exec    AddLibrary
1$
        move.l  a2,d0                               ; Return library base
        restore a2/a6
        rts




*
*****i* freeanim.library/Open() ***************************************
*
*   NAME
*       Open -- Allocate player
*
*   SYNOPSIS
*       freeAnimLibrary = Open()
*       D0
*
*       struct freeAnim *Open(void);
*
*   FUNCTION
*       Begin fading animation to black (this may take up to a second)
*
*   RESULTS
*       NULL means library could not be openned (no animation present)
*
*   NOTES
*
*   SEE ALSO
*       Close()
*
***********************************************************************

Open:
        save    a2                                      ; Is the library already openned?
        tst.w   LIB_OPENCNT(a6)
        bne     OpenError

        lea     Startup_Animation(pc),a1                ; Find startup animation message port
        exec    FindPort
        tst.l   d0
        beq     OpenError
        move.l  d0,a2

        move.l  #-1,d0                                  ; Allocate message port signal
        exec    AllocSignal
        cmp.b   #-1,d0
        beq     OpenError

        lea     FL_ReplyPort(a6),a1                     ; Find our task
        move.b  d0,MP_SIGBIT(a1)
        sub.l   a1,a1
        exec    FindTask
        lea     FL_ReplyPort(a6),a1
        move.l  d0,MP_SIGTASK(a1)

        move.w  #1,LIB_OPENCNT(a6)                      ; Library is open

        move.l  a2,a0
        lea     FL_Message(a6),a1                       ; Send message to animation task
        exec    PutMsg

        restore a2                                      ; Return library base
        move.l  a6,d0
        rts

OpenError:
        restore a2                                      ; Could not open library
        clr.l   d0
        rts



*
*****i* freeanim.library/Close() **************************************
*
*   NAME
*       Close -- Wait for animation to finish shutting down.
*
*   SYNOPSIS
*       Close()
*
*       void Close(void);
*
*   FUNCTION
*       Waits for animation to finish shutting down and closes library.
*
*   RESULTS
*
*   NOTES
*
*   SEE ALSO
*       Open()
*
***********************************************************************

Close:
        tst.w   LIB_OPENCNT(a6)                         ; Was a message sent to the animation?
        beq     99$

        lea     FL_ReplyPort(a6),a0                     ; Wait for message to return
        exec    WaitPort
1$
        lea     FL_ReplyPort(a6),a0                     ; Get the message
        exec    GetMsg
        tst.l   d0
        bne     1$

        lea     FL_ReplyPort(a6),a0                     ; Free message port signal
        move.b  MP_SIGBIT(a0),d0
        exec    FreeSignal

        clr.w   LIB_OPENCNT(a6)                         ; Remove the library
        move.l  a6,a1
        exec    Remove

        move.l  a6,a1                                   ; Free the memory used by this library
        clr.l   d0
        move.w  LIB_NEGSIZE(a6),d0
        sub.l   d0,a1
        clr.l   d1
        move.w  LIB_POSSIZE(a6),d1
        add.l   d1,d0
        exec    FreeMem
99$
Expunge:
Reserved:
        clr.l   d0                                      ; Just return
        rts




*************************************************************************
*                                                                       *
*       Copyright (C) 1992, Commodore Amiga Inc.  All rights reserved.  *
*                                                                       *
*************************************************************************


EndCode:

        END
