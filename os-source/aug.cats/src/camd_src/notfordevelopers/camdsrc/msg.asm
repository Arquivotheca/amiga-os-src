*************************************************************************
*     C. A. M. D.       (Commodore Amiga MIDI Driver)                   *
*************************************************************************
*                                                                       *
* Design & Development  - Roger B. Dannenberg                           *
*                       - Jean-Christophe Dhellemmes                    *
*                       - Bill Barton                                   *
*                       - Darius Taghavy                                *
*                       - Talin & Joe Pearce                            *
*                                                                       *
* Copyright 1990 by Commodore Business Machines                         *
*                                                                       *
*************************************************************************
*
* msg.asm     - MidiMsg handling
*
*************************************************************************

; Notes
; -----
;   1.  The message functions are written in such a way as to not
;       need to LockMidiList during receive functions (GetMidi, GetSysEx,
;       etc).  Sys/ex distribution is the potential weak link.
;       Great care was taken to keep sys/ex and MIDI buffers in sync w/o
;       locking on the receive side.  It works now, but keep an eye
;       on it.
;
;   2.  GetSysEx(), SkipSysEx(), etc, use the embedded EOX byte rather than
;       SysExTail to find the end of the current sys/ex message.  Because
;       of this, GetSysEx(), etc, don't need to LockMidiList().


      nolist
        include "exec/types.i"
        include "exec/macros.i"
        include "midi/mididefs.i"
        include "utility/hooks.i"

        include "camdlib.i"
      list


; DEBUG set 1

COLORBUG	macro
;		move.w  #\1,$00dff180
			endm

        idnt    msg.asm
        blanks  on                  ; tables definitions below contain spaces

            ; public
        xdef    GetMidi
        xdef    GetMidiErr
        xdef    GetSysEx
        xdef    MidiMsgLen
        xdef    MidiMsgType
        xdef    PutMidi
        xdef    PutSysEx
        xdef    SkipSysEx
        xdef    QuerySysEx
        xdef    WaitMidi

            ; internal
        xdef    IPutMidi                        ; function name changed by --DJ
        xdef    PutSysExList
        xdef    SendErrToMI
        xdef    _StripSysEx

            ; external
        xref    SendMsgToUnits
        xref    SendRTToUnits
        xref    SendSXListToUnits
        xref    SendSysExToUnits

    ifd DEBUG
            ; debug
        xdef    PostMsg
        xdef    PostSysEx
        xdef    TestSysExFilter
    endc


; common registers

mir     equr    a2      ; A2 = MidiNode pointer



; Public Functions

; All public functions require A6 = CamdBase unless otherwise noted.

****************************************************************
*
*   PutMidi
*
****************************************************************

    ifgt 0
/****** camd.library/PutMidi ******************************************
*
*   NAME
*       PutMidi -- Send a MidiMsg to an output link
*
*   SYNOPSIS
*       void PutMidi (struct MidiLink *link, ULONG Msg)
*                       a0                  d0
*
*   FUNCTION
*       Sends the a MidiMsg to the output link specified.
*       The message is automatically timestamped.
*
*   INPUTS
*       Msg - mm_Msg long word component of a MidiMsg.
*
*   RESULTS
*       None
*
*   NOTE
*       Although this function doesn't require a MidiNode pointer, the
*       caller must have allocated a MidiNode using CreateMidi() in
*       order for any messages to be distributed.
*
*   SEE ALSO
*       PutMidi()
****************************************************************************
*
*/

/****** camd.library/PutMidiMsg ******************************************
*
*   NAME
*       PutMidiMsg (MACRO) -- Send a MidiMsg.
*
*   SYNOPSIS
*       void PutMidiMsg (struct MidiLink *ml, MidiMsg *msg)
*                            a0                 a1
*
*   FUNCTION
*       Sends a MidiMsg. This is a macro that calls PutMidi().
*       The value in msg->mm_Port is ignored.
*       This macro exists in both C and Assembly.
*
*   INPUTS
*       ml      - pointer to MidiLink.
*       msg - pointer to a MidiMsg to send.  Only the mm_Msg segment is
*         used.
*
*   RESULTS
*       None
*
*   SEE ALSO
*       GetMidi(), PutMidi()
****************************************************************************
*
*/
    endc

; Need to change this to pass the correct registers to IPutMidi

PutMidi
        Push    a5
        exg.l   a6,a5                       ; A5 = CamdBase
        move.l  camd_SysBase(a5),a6         ; A6 = SysBase

        clr.l   -(sp)                       ; push timeless value as mm_Time
        move.l  d0,-(sp)                    ; push mm_Msg
        move.l  a0,a1
        move.l  sp,a0                       ; A0 = MidiMsg pointer
        bsr     IPutMidi
        addq.w  #8,sp                       ; pop MidiMsg off stack

        exg.l   a5,a6
        Pop
        rts

****************************************************************
*
*   GetMidi
*
****************************************************************

    ifgt 0
/****** camd.library/GetMidi ******************************************
*
*   NAME
*       GetMidi -- Get next MidiMsg from buffer.
*
*   SYNOPSIS
*       BOOL GetMidi (struct MidiNode *mn, MidiMsg *msg)
*                             a0                 a1
*
*   FUNCTION
*       Gets the next MidiMsg from mn->MsgQueue.
*
*       It is definitely not safe to call this if there's no MsgQueue.
*
*   INPUTS
*       mn - pointer to MidiNode.
*       msg - pointer to buffer to place MidiMsg removed from
*         queue.
*
*   RESULTS
*       TRUE if a MidiMsg was actually copied in msg.  FALSE
*       if the buffer was empty.
*
*   SEE ALSO
*       WaitMidi()
****************************************************************************
*
*/
    endc

GetMidi
        Push    mir/a3

        ; currently doing this without a lock... so be careful about how
        ; PostMsg works.

        move.l  a0,mir                      ; mir = MidiNode
        move.l  a1,a3                       ; A3 = client's MidiMsg

        bsr     SkipSysEx                   ; skip any unread sys/ex bytes for previous message

        CLEAR   d0                          ; set return value to FALSE
        move.l  mi_MsgQueueHead(mir),a0     ; A0 = MsgQueueHead

gm_loop
        cmp.l   mi_MsgQueueTail(mir),a0     ; is the buffer empty?
        beq.s   gm_ret

        move.b  mm_Status(a0),d1            ; is this a "dead" message (removed by StripSysEx())?
        bmi.s   gm_msgok

        lea     MidiMsg_Size(a0),a0         ; skip message
        cmp.l   mi_MsgQueueEnd(mir),a0
        bcs.s   gm_nowrap1
        move.l  mi_MsgQueue(mir),a0
gm_nowrap1
        move.l  a0,mi_MsgQueueHead(mir)
        bra     gm_loop

gm_msgok
        cmp.b   #MS_SysEx,d1                ; is this a sys/ex message?
        bne.s   gm_notsysex
        bset.b  #MIB_InSysEx,mi_SysFlags(mir)
gm_notsysex
        move.l  (a0)+,(a3)+                 ; copy next MidiMsg in queue to supplied msg buffer
        move.l  (a0)+,(a3)+

        cmp.l   mi_MsgQueueEnd(mir),a0
        bcs.s   gm_nowrap2
        move.l  mi_MsgQueue(mir),a0
gm_nowrap2
        move.l  a0,mi_MsgQueueHead(mir)
        moveq.l #1,d0                       ; return TRUE

gm_ret
        Pop
        rts



****************************************************************
*
*   WaitMidi
*
****************************************************************

    ifgt 0
/****** camd.library/WaitMidi ******************************************
*
*   NAME
*       WaitMidi -- Wait for next MidiMsg and get it.
*
*   SYNOPSIS
*       BOOL WaitMidi (struct MidiNode *mn, MidiMsg *msg)
*                              a0                 a1
*
*   FUNCTION
*       Waits for the next MidiMsg to arrive at the MidiNode.  On reception,
*       the MidiMsg is removed and copied to the supplied buffer.
*
*       WaitMidi() first checks for errors detected at the MidiNode.  If
*       none are detected, GetMidi() is called.  If the buffer was empty,
*       Wait() is called until the MidiNode is signalled.  Once the signal
*       arrives, this process is repeated.
*
*   INPUTS
*       mn  - pointer to MidiNode.
*       msg - pointer to buffer to place MidiMsg removed from queue.
*
*   RESULTS
*       TRUE if a MidiMsg was received.  FALSE if an error was detected.
*
*   SEE ALSO
*       GetMidi()
****************************************************************************
*
*/
    endc

WaitMidi
        Push    mir/a3
        move.l  a0,mir                  ; mir = mn
        move.l  a1,a3                   ; a3 = msg

wm_loop
        CLEAR   d0
        tst.b   mi_ErrFlags(mir)        ; if error flags, exit loop
        bne.s   wm_ret

        move.l  mir,a0
        move.l  a3,a1
        bsr     GetMidi
        tst.w   d0                      ; if TRUE, exit loop
        bne.s   wm_ret

        CLEAR   d0
        move.b  mi_ReceiveSigBit(mir),d1
        bmi.s   wm_ret                  ; catches ReceiveSigBit==-1 (the case for send-only MI's)
        bset.l  d1,d0                   ; wait for midi signal mask
        move.l  a6,-(sp)
        move.l  camd_SysBase(a6),a6
        JSRLIB  Wait
        move.l  (sp)+,a6

        bra     wm_loop

wm_ret
        Pop
        rts



****************************************************************
*
*   PutSysExToPort
*
****************************************************************

    ifgt 0
/****** camd.library/PutSysEx ******************************************
*
*   NAME
*       PutSysEx -- Send a Sys/Ex Message to a link.
*
*   SYNOPSIS
*       void PutSysEx (struct MidiLink *ml, UBYTE *Buffer)
*                              a0          a1
*
*   FUNCTION
*       Sends the sys/ex message to the specified port.
*
*       Sys/Ex messages are sent to hardware units regardless of transmit
*	buffer size.  Distribution pauses until the entire sys/ex message
*	is placedin the transmit queue.
*
*       Distribution to MidiNodes is a bit different.  If the sys/ex message
*       is greater than the MidiNode's sys/ex buffer, then the message will
*       not be sent to the MidiNode.  If the message is smaller than the
*	MidiNode's sys/ex buffer, but there's not enough room left in the
*	buffer to contain the sys/ex message, CMEF_SysExFull will be sent
*	to the MidiNode. Otherwise the sys/ex message will be sent.
*
*   INPUTS
*       ml     - MidiLink to send Sys/Ex to
*       Buffer - Pointer to a Sys/Ex message beginning with MS_SysEx and
*            ending with MS_EOX.
*
*   RESULTS
*       None
*
*   NOTE
*       Although this function doesn't require a MidiNode pointer, the
*       caller must have allocated a MidiNode using CreateMidi() in
*       order for any messages to be distributed.
*
*   SEE ALSO
*       PutSysEx()
****************************************************************************
*
*/
    endc

; register inputs
; a0 -- MidiLink
; a1 -- Buffer

; register usage
; d0 = scratch
; d1
; d2 = save address of dest pointer to update msg queue
; d3 = length of sysex
; d4 = pointer to original MidiLink

; a0 = scratch
; a1 = scr
; a2 = mir
; a3 = buffer pointer (storage only, not used as address)
; a4 = receiver MidiLink
; a5 = CAMDBase
; a6 = ExecBase
; a7 = stack

PutSysEx
        Push    mir/d2-d4/a3-a5
        exg.l   a6,a5                       ; A5 = CamdBase
        move.l  camd_SysBase(a5),a6         ; A6 = SysBase

        move.l  a0,d4                       ; d4 <-- Sender MidiLink

            ; set up MidiMsg on the stack
        clr.l   -(sp)                       ; push timeless value as mm_Time
        subq    #4,sp                       ; push space for MidiMsg
        move.b  (a1),(sp)                   ; copy 1st 3 bytes of msg to MidiMsg
        move.b  1(a1),1(sp)
        move.b  2(a1),2(sp)

            ; sanity check of messages
        cmp.b   #MS_SysEx,(a1)              ; is 1st byte MS_SysEx?
        bne     pstp_err

        move.l  a1,a3                       ; a3 <-- SysEx Buffer address
        bsr     SysExLen
        move.l  d0,d3                       ; d3 <-- SysEx Length

        moveq   #3,d0                       ; test length (must be >= 3 bytes)
        cmp.l   d0,d3                       ; if sysex length < 3
        bcs     pstp_err                    ; then error

            ; send to Interfaces
        LockMidiList                        ; lock MidiList

        move.l  d4,a4                       ; a4 <-- Sender MidiLink
        move.l  ml_Location(a4),d0          ; d0 = cluster address
        beq     pstp_intdone                ; if point to air, then done
        move.l  d0,a4                       ; a4 <-- cluster address
        lea     mcl_Receivers(a4),a4        ; a4 <-- address of receiver list

pstp_intloop
        move.l  (a4),a4                     ; next node
        tst.l   (a4)                        ; is end of list?
        beq     pstp_intdone

            ; check type filter -- see if we are taking SysEx calls today
            ; note -- we're testing the upper byte here...
        btst.b  #CMB_SysEx&7,ml_EventTypeMask+(31-CMB_SysEx)/8(a4)
        beq     pstp_intloop

            ; check sys/ex filter
        move.l  a4,a2                       ; a2 <-- pointer to link...
        move.l  a3,a1                       ; a1 = SysEx message pointer
        bsr     TestSysExFilter             ; returns Z=set on match
        bne.s   pstp_intloop

        move.l  ml_MidiNode(a4),mir         ; a2 <-- address of MidiNode

            ; call hook (if any)
        move.l  mi_ReceiveHook(mir),d0  ; d0 <-- address of receive hook
        beq.s   pstp_nohook             ; if no hook, skip

        move.l  sp,a1                   ; A1 = MidiMsg
        move.l  d0,a0                   ; a0 <-- address of hook
        move.l  a4,a2                   ; a2 <-- address of MidiLink?
        move.l  d3,d0                   ; D0 = length (a3 already has buffer)
        pea     pstp_donehook
        move.l  h_Entry(a0),-(sp)
        rts

pstp_donehook
        move.l  ml_MidiNode(a4),mir         ; a2 <-- address of MidiNode
pstp_nohook
            ; check error flags
        move.b  mi_ErrFlags(mir),d0
        and.b   #CMEF_BufferFull!CMEF_SysExFull,d0
        bne.s   pstp_intloop

            ; check buffer size
            ; NOTE - this should also handle the case where there is no buffer
        cmp.l   mi_SysExQueueSize(mir),d3
        beq.s   pstp_intloop            ; ok to not have a SysExQueue
        bcc.s   pstp_toobig                 ; if len >= bufsize, send TooBig error

            ; check for buffer overflow
        move.l  mi_SysExQueueHead(mir),d0   ; D0 = head-tail
        sub.l   mi_SysExQueueTail(mir),d0
        bhi.s   pstp_ovwrap
        add.l   mi_SysExQueueSize(mir),d0   ; if occupied part of buffer doesn't wrap
                                            ; complement by buffer size (end-tail+head-buff)
pstp_ovwrap
        cmp.l   d0,d3                       ; will this message fit in the remaining buffer?
        bcc.s   pstp_overflow               ; if len >= empty space, skip (takes into account overflow pad)

            ; post sysex to SysExQueue
        move.l  a3,a0                       ; A0 = src buffer
        move.l  mi_SysExQueueTail(mir),a1   ; A1 = dest buffer
        move.l  d3,d0                       ; D0 = length
        bsr     PostSysEx
        move.l  a1,d2                       ; save dest ptr in D2 (temp storage)

        move.l  sp,a1                       ; A1 = MidiMsg
                                            ; a2 = mir
                                            ; a4 = MidiLink
        bsr     PostMsg
        tst.w   d0                          ; posted ok?
        beq.s   pstp_intloop                ; if not, don't set Tail

        move.l  d2,mi_SysExQueueTail(mir)   ; store buffer point (loaded into D2 above)
        bra.s   pstp_intloop

pstp_overflow
        moveq   #CMEF_SysExFull,d1
        bra.s   pstp_senderr

pstp_toobig
        moveq   #CMEF_SysExTooBig,d1
pstp_senderr
        move.l  ml_MidiNode(a4),mir         ; a2 <-- address of MidiNode

        bsr     SendErrToMI
        bra     pstp_intloop

pstp_intdone
        UnlockMidiList
        bra.s   pstp_ret

pstp_err            ; Send Err to sender node
        moveq   #CMEF_MsgErr,d1
        move.l  d3,mir
        move.l  ml_MidiNode(mir),mir
        bsr     SendErrToMI

pstp_ret
        addq.w  #8,sp                   ; pop MidiMsg off stack
        exg.l   a5,a6
        Pop
        rts

****************************************************************
*
*   GetSysEx
*
****************************************************************

    ifgt 0
/****** camd.library/GetSysEx ******************************************
*
*   NAME
*       GetSysEx -- Read bytes from Sys/Ex buffer.
*
*   SYNOPSIS
*       ULONG GetSysEx (struct MidiNode *mn, UBYTE *Buf, ULONG Len)
*                              a0              a1          d0
*
*   FUNCTION
*       Reads bytes from the Sys/Ex buffer for the current sys/ex message
*       into the supplied buffer.  This function will not read past the end
*       of the current sys/ex message.  The actual number of bytes read is
*       returned.  0 is returned when there are no more bytes to be read for
*       the current sys/ex message.  If the current message is not a sys/ex
*       message, this function will return 0.
*
*   INPUTS
*       mn      - Pointer to MidiNode.
*       Buf - Output buffer pointer.
*       Len - Max number of bytes to read.
*
*   RESULTS
*       Actual number of bytes read.
*
*   SEE ALSO
*       GetMidi(), SkipSysEx(), QuerySysEx()
****************************************************************************
*
*   Note that OutputFunc assumes GetSysEx does NOT need CamdBase in a6.
*   If this changes, OutputFunc must be changed.
*
****************************************************************************
*/
    endc

GetSysEx
        Push    mir/d2

        move.l  a0,mir                      ; mir = MidiNode

        move.l  a1,-(sp)                    ; save original buffer ptr for length calc

        btst.b  #MIB_InSysEx,mi_SysFlags(mir)   ; are we in a sys/ex message?
        beq.s   gsx_ret

        move.l  mi_SysExQueueHead(mir),a0   ; A0 = src buffer (A1 = dest buffer, D0 = dest length)

        move.l  mi_SysExQueueEnd(mir),d1
        sub.l   a0,d1                       ; D1 = bytes before wrap
        move.l  d0,d2
        sub.l   d1,d2                       ; D2 = bytes after wrap
        bls.s   gsx_copysimple              ; if <= 0, simple copy

                    ; fragmented copy
        subq.l  #1,d1                       ; bytes before wrap is never 0
        move.l  d1,d0
        swap    d0
gsx_loop1
        move.b  (a0)+,(a1)
        cmp.b   #MS_EOX,(a1)+
        dbeq    d1,gsx_loop1
        dbeq    d0,gsx_loop1
        beq.s   gsx_done                    ; if not EOX, then we're at a buffer wrap

        move.l  mi_SysExQueue(mir),a0
        move.l  d2,d0

gsx_copysimple      ; simple copy
        subq.l  #1,d0                       ; avoid entering dbeq loop w/ 0 count
        bcs.s   gsx_more
        move.l  d0,d1
        swap    d0
gsx_loop2
        move.b  (a0)+,(a1)
        cmp.b   #MS_EOX,(a1)+
        dbeq    d1,gsx_loop2
        dbeq    d0,gsx_loop2
        bne.s   gsx_more                    ; if not EOX, output buffer expired

gsx_done
        bclr.b  #MIB_InSysEx,mi_SysFlags(mir)

gsx_more
        cmp.l   mi_SysExQueueEnd(mir),a0
        bcs.s   gsx_nowrap
        move.l  mi_SysExQueue(mir),a0
gsx_nowrap
        move.l  a0,mi_SysExQueueHead(mir)   ; store src buffer pointer

gsx_ret
        move.l  a1,d0                       ; D0 = current dest buffer pointer
        sub.l   (sp)+,d0                    ; subtract original -> D0 = bytes copied into dest buffer
        Pop
        rts



****************************************************************
*
*   QuerySysEx
*
****************************************************************

    ifgt 0
/****** camd.library/QuerySysEx ******************************************
*
*   NAME
*       QuerySysEx -- Get number of bytes remaining in current Sys/Ex
*                 message.
*
*   SYNOPSIS
*       ULONG QuerySysEx (struct MidiNode *mn)
*                                a0
*
*   FUNCTION
*       Returns the number of bytes remaining in the current sys/ex message.
*
*   INPUTS
*       mn - pointer to MidiNode
*
*   RESULTS
*       Remaining bytes in sys/ex message.      0 is returned if the last
*       message read from GetMidi() wasn't a sys/ex message.
*
*   SEE ALSO
*       GetSysEx(), GetMidi()
****************************************************************************
*
*/
    endc

QuerySysEx
        Push    d2-d3
        CLEAR   d0                          ; clear result

        btst.b  #MIB_InSysEx,mi_SysFlags(a0)    ; in sys/ex?
        beq.s   qsx_ret

        move.b  #MS_EOX,d1                  ; D1 = EOX byte for comparison
        move.l  mi_SysExQueueHead(a0),a1    ; A1 = next sys/ex byte

        move.l  mi_SysExQueueEnd(a0),d2
        sub.l   a1,d2                       ; D2 = End-Head
        subq.l  #1,d2                       ; End-Head is always > 0
        move.w  d2,d3
        swap    d2
qsx_loop1
        cmp.b   (a1)+,d1
        dbeq    d3,qsx_loop1
        dbeq    d2,qsx_loop1

        suba.l  mi_SysExQueueHead(a0),a1    ; A1 = byte count from loop1 (suba doesn't affect flags)

        beq.s   qsx_done                    ; if not EOX, then we're at a buffer wrap

        move.l  a1,d0                       ; D0 = byte count from loop1
        move.l  mi_SysExQueue(a0),a1        ; A1 = beginning of Queue

qsx_loop2
        cmp.b   (a1)+,d1
        bne.s   qsx_loop2

        sub.l   mi_SysExQueue(a0),a1        ; A1 = byte count from loop2

qsx_done
        add.l   a1,d0                       ; add byte count to length

qsx_ret
        Pop
        rts



****************************************************************
*
*   SkipSysEx
*
****************************************************************

    ifgt 0
/****** camd.library/SkipSysEx ******************************************
*
*   NAME
*       SkipSysEx -- Skip the next sys/ex message in the Sys/Ex buffer.
*
*   SYNOPSIS
*       void SkipSysEx (struct MidiNode *mn)
*                              a0
*
*   FUNCTION
*       Skips the remaining bytes of the current sys/ex message.  Nothing
*       happens if the last message read from GetMidi() wasn't a sys/ex
*       message.
*
*   INPUTS
*       mn - Pointer to MidiNode.
*
*   RESULTS
*       None.
*
*   SEE ALSO
*       GetSysEx()
****************************************************************************
*
*/
    endc

SkipSysEx
        bclr.b  #MIB_InSysEx,mi_SysFlags(a0)    ; in sys/ex?
        beq.s   ssx_ret

        Push    d2

        move.b  #MS_EOX,d0                  ; D0 = EOX byte for comparison
        move.l  mi_SysExQueueHead(a0),a1    ; A1 = next sys/ex byte

        move.l  mi_SysExQueueEnd(a0),d1
        sub.l   a1,d1                       ; D1 = End-Head

        subq.l  #1,d1                       ; End-Head is always > 0
        move.w  d1,d2
        swap    d1
ssx_loop1
        cmp.b   (a1)+,d0
        dbeq    d2,ssx_loop1
        dbeq    d1,ssx_loop1
        beq.s   ssx_done                    ; if not EOX, then we're at a buffer wrap

        move.l  mi_SysExQueue(a0),a1
ssx_loop2
        cmp.b   (a1)+,d0
        bne.s   ssx_loop2

ssx_done
        cmp.l   mi_SysExQueueEnd(a0),a1
        bcs.s   ssx_nowrap
        move.l  mi_SysExQueue(a0),a1
ssx_nowrap
        move.l  a1,mi_SysExQueueHead(a0)    ; save Head

        Pop
ssx_ret
        rts


****************************************************************
*
*   GetMidiErr
*
****************************************************************

    ifgt 0
/****** camd.library/GetMidiErr ******************************************
*
*   NAME
*       GetMidiErr -- Read accumulated MIDI error flags.
*
*   SYNOPSIS
*       UBYTE GetMidiErr (struct MidiNode *mn)
*                                 a0
*
*   FUNCTION
*       Returns the current MIDI error flags from the MidiNode.  The error
*       flags are cleared after reading.  Some error flags, such as
*       CMEF_BufferFull, prevent additional MIDI reception and must be
*       cleared in order to restart MIDI reception.
*
*       Only bits enabled by SetMidiErrFilter() are returned as 1 bits.
*
*   INPUTS
*       mn - MidiNode to check.
*
*   RESULTS
*       CMEF_ error flags or 0 if no errors were present.  The upper 24 bits
*       are cleared for the caller's convenience.
*
*   SEE ALSO
*       SetMidiErrFilter(), WaitMidi()
****************************************************************************
*
*/
    endc


GetMidiErr
        CLEAR   d0                          ; clear upper bits (sign extends result)
        move.b  mi_ErrFlags(a0),d0          ; read pending error flags
        eor.b   d0,mi_ErrFlags(a0)          ; clear flags we just read
        rts



****************************************************************
*
*   MidiMsgType
*
****************************************************************

    ifgt 0
/****** camd.library/MidiMsgType ******************************************
*
*   NAME
*       MidiMsgType -- Determine the type of a MIDI message
*
*   SYNOPSIS
*       WORD MidiMsgType (MidiMsg *Msg)
*                          a0
*
*   FUNCTION
*       Returns a type bit number (CMB_) for supplied MidiMsg.
*
*   INPUTS
*       Msg - Pointer to a MidiMsg.
*
*   RESULTS
*       CMB_ bit number of message type.  -1 if message is undefined.  Also
*       -1 is returned for MS_EOX since it's not legal to be passed through
*       PutMidi().  The result is sign extended to 32 bits for assembly
*       programmers.
*
*   SEE ALSO
*       MidiMsgLen()
****************************************************************************
*
*/
    endc

; A6 doesn't actually need to point to CamdBase for this function
; !!! assumes structure of MidiMsg (i.e. doesn't use mm_ members)

MidiMsgType
        Push

        CLEAR   d1                  ; D1 = temp register
        moveq   #-1,d0              ; D0 = result

        move.b  (a0)+,d1            ; get status byte (advance pointer to first data byte)
        bpl.s   mt_ret              ; return 0 if not a status byte

        cmp.b   #MS_System,d1       ; system msg?
        bcc.s   mt_sys

        and.b   #$f0,d1             ; mask off channel bits
        cmp.b   #MS_Ctrl,d1         ; a controller?
        beq.s   mt_ctrl

mt_ch                       ; channel msg
        lsr.b   #4,d1               ; get upper nibble
        lea     ChFilterTbl-8,a1    ; get table address -8 to compensate for status byte flag
        bra.s   mt_fetch

mt_sys                      ; system msg
        lea     SysFilterTbl-MS_System,a1   ; get table address -$f0 to compensate
        bra.s   mt_fetch

mt_ctrl                     ; ctrl, subdivide ctrl types
        move.b  (a0),d1             ; get ctrl # (first data byte)
        lea     CtrlFilterTbl,a1    ; get table address

mt_fetch
        move.b  0(a1,d1.w),d0       ; get type bit
        ext.w   d0                  ; sign extend to long just to be nice
        ext.l   d0

mt_ret
        Pop
        rts


ChFilterTbl
        dc.b    CMB_Note,       CMB_Note        ; 80, 90
        dc.b    CMB_PolyPress,  -1              ; A0, B0
        dc.b    CMB_Prog,       CMB_ChanPress   ; C0, D0
        dc.b    CMB_PitchBend                   ; E0

SysFilterTbl
        dc.b    CMB_SysEx,      CMB_SysCom      ; F0, F1
        dc.b    CMB_SysCom,     CMB_SysCom      ; F2, F3
        dc.b    -1,             -1              ; F4, F5
        dc.b    CMB_SysCom,     -1              ; F6, F7
        dc.b    CMB_RealTime,   -1              ; F8, F9
        dc.b    CMB_RealTime,   CMB_RealTime    ; FA, FB
        dc.b    CMB_RealTime,   -1              ; FC, FD
        dc.b    CMB_RealTime,   CMB_RealTime    ; FE, FF

CtrlFilterTbl
        dcb.b   $20,CMB_CtrlMSB     ;  0 - 1F
        dcb.b   $20,CMB_CtrlLSB     ; 20 - 3F
        dcb.b   $10,CMB_CtrlSwitch  ; 40 - 4F
        dcb.b   $10,CMB_CtrlByte    ; 50 - 5F
        dcb.b   $06,CMB_CtrlParam   ; 60 - 65
        dcb.b   $12,CMB_CtrlUndef   ; 66 - 77
        dcb.b   $08,CMB_Mode        ; 78 - 7F

        ds.w    0       ; force even alignment


****************************************************************
*
*   MidiMsgLen
*
****************************************************************

    ifgt 0
/****** camd.library/MidiMsgLen ******************************************
*
*   NAME
*       MidiMsgLen -- Determine the length of a MIDI message
*
*   SYNOPSIS
*       WORD MidiMsgLen (ULONG StatusByte)
*                          d0
*
*   FUNCTION
*       Returns the length in bytes of the MIDI message described by the
*       supplied status byte.  The message length includes the status byte.
*
*   INPUTS
*       StatusByte - UBYTE containing status byte.
*
*   RESULTS
*       length - length of the message in bytes.  For valid messages this
*            will be at least 1.  0 is returned for invalid messages,
*            MS_SysEx and MS_EOX.
*
*            The result is sign extended to 32 bits for assembly
*            programmers.
*
*   SEE ALSO
*       MidiMsgType()
****************************************************************************
*
*/
    endc


; A6 doesn't actually need to point to CamdBase for this function
; ParseMidi depends on the upper word of the return value being 0.

MidiMsgLen
        Push

        CLEAR   d1                  ; D1 = temp
        move.b  d0,d1
        CLEAR   d0                  ; D0 = result.  clear upper bits (sign extends result)

        tst.b   d1
        bpl.s   ml_ret              ; return 0 if not a status byte

        cmp.b   #MS_System,d1
        bcc.s   ml_sys

ml_ch
        lsr.b   #4,d1               ; get upper nibble
        lea     ChLenTbl-8,a1       ; get table address, -8 to account for extra junk in d1
        bra.s   ml_fetch

ml_sys
        lea     SysLenTbl-$f0,a1    ; get table address, -$f0 to account for extra junk in d1

ml_fetch
        move.b  0(a1,d1.w),d0       ; get length

ml_ret
        Pop
        rts


ChLenTbl      ; 8n 9n An Bn Cn Dn En
        dc.b    3, 3, 3, 3, 2, 2, 3

SysLenTbl     ; F0 F1 F2 F3 F4 F5 F6 F7 F8 F9 FA FB FC FD FE FF
        dc.b    0, 2, 3, 2, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1      ; 0 for undefined (or unknown for F0)

        ds.w    0       ; force even alignment



****************************************************************
*
* Internal Functions
*
****************************************************************

****************************************************************
*
*   StripSysEx
*
*   SYNOPSIS
*       void StripSysEx (struct XMidiNode *mn)
*                                  a1
*
*   FUNCTION
*       Strips Sys/Ex messages out of MsgQueue.  Called from
*       C.  Assumes caller has called LockMidiList().
*
*   INPUTS
*       mn - pointer to MidiNode to affect
*
*   RESULTS
*       None
*
****************************************************************

_StripSysEx         ; called from C
        move.l  mi_MsgQueueHead(a1),a0      ; A0 = MsgHead

stsx_loop
        cmp.l   mi_MsgQueueTail(a1),a0      ; is the buffer empty?
        beq.s   stsx_ret

        cmp.b   #MS_SysEx,mm_Status(a0)     ; is this a sys/ex message?
        bne.s   stsx_skip
        clr.b   mm_Status(a0)               ; kill this message (make status byte non-negative)
stsx_skip
        lea     MidiMsg_Size(a0),a0         ; skip message
        cmp.l   mi_MsgQueueEnd(a1),a0
        bcs.s   stsx_loop
        move.l  mi_MsgQueue(a1),a0
        bra.s   stsx_loop

stsx_ret
        rts

****************************************************************
*
*   IPutMidi
*
*   FUNCTION
*       Send a MidiMsg.
*
*   INPUTS
*       A0 - pointer to MidiMsg
*       A1 - pointer to MidiLink            ; Added by --DJ
*       A5 - CamdBase
*       A6 - SysBase
*
*   RESULTS
*       None
*
****************************************************************

IPutMidi
        Push    mir/d2/d3/a4
        move.l  a0,a4                       ; A4 = MidiMsg
        move.l  a1,d3                       ; picked new register later -jp

        move.l  mm_Msg(a4),d0               ; d0 = mm_Msg component of MidiMsg
        and.l   #$80808000,d0               ; valid message structure in D0?
        neg.l   d0                          ; if not $80000000, post error
        bvc.s   pmm_err                     ; (only $80000000 causes NEG.L to set V=1)

        lea     mm_Msg(a4),a0
        bsr     MidiMsgType                 ; get msg type
        move.b  d0,d2                       ; d2 = type bit.  if it's <0, then we got an undefined message
        bmi.s   pmm_err

        cmp.b   #CMB_SysEx,d2               ; can't do sys/ex here!
        beq.s   pmm_err

        LockMidiList                        ; lock list

; Added by --DJ

        move.l  d3,a1                       ; picked new register later -jp
        move.l  ml_Location(a1),d0          ; d0 <-- Cluster address
        beq.s   pmm_intdone                 ; if point to air, don't send
        move.l  d0,a1                       ; a1 <-- Cluster address
        lea     mcl_Receivers(a1),a1        ; a1 <-- address of receiver list

pmm_intloop
        move.l  (a1),a1                     ; a1 <-- next node in receiver list
        tst.l   (a1)                        ; is this the end of list?
        beq.s   pmm_intdone                 ; if so, go to done

            ; check type filter
        move.l  ml_EventTypeMask(a1),d1        ; d1 = TypeFilter for MidiNode
        btst.l  d2,d1                       ; d2 = Type bit
        beq.s   pmm_intloop

            ; check channel filter
        move.b  mm_Status(a4),d0            ; d0 = status byte
        cmp.b   #MS_System,d0
        bcc.s   pmm_sysmsg                  ; skip channel filter if a system msg

        and.b   #$f,d0                      ; leave only channel bits behind
        move.w  ml_ChannelMask(a1),d1       ; d1 = ChannelMask for MidiNode
        btst.l  d0,d1
        beq.s   pmm_intloop
pmm_sysmsg

        move.l  ml_MidiNode(a1),mir         ; a2 <-- address of MidiNode

            ; set port ID

        move.b  ml_PortID(a1),mm_Port(a4)   ; set the port number

; Stack save inserted as temporary kludge by --DJ until we get registers
; optimized
        exg.l   a4,a1                       ; A1 = MidiMsg pointer
        bsr     PostMsg                     ; post the message
        exg.l   a4,a1                       ; A1 = MidiLink (again)
        bra     pmm_intloop

pmm_intdone
        UnlockMidiList
        bra.s   pmm_ret

pmm_err            ; Send Err

; REM: We need to propagate error _BACKWARDS_ to sender's midi node.
; to do that, we need the Interface location of the original sender link.
; (Then we can just call SendErrToMI).
; We'll need a register to store that in...

        move.l  d3,mir
        move.l  ml_MidiNode(mir),mir
        moveq   #CMEF_MsgErr,d1
        bsr     SendErrToMI

pmm_ret
        Pop
        rts

****************************************************************
*
*   PutSysExList
*
*   FUNCTION
*       Send SysEx List to a Port.
*
*   INPUTS
*       A3 - ParserData
*               . assumes pd_CurMsg is completely initialized
*                 (including mm_Time)
*               . assumes sys/ex list contains EOX byte.
*       A5 - CamdBase
*       A6 - SysBase
*       D4 - MidiList
*
*   RESULTS
*       None
*
****************************************************************

; REM: JOE: This function is for sending sysex blocks that aren't in a
; continguous hunk of memory, but rather in a linked list of blocks.
; The only place this function is called is by ParseMidi...Because
; ParseMidi will actually allocate new memory blocks if the incoming sysex
; message overflows it's block.

PutSysExList
        Push    mir/d3/d5/d6/a4

            ; calculate length -> d3
        move.l  pd_SXBCur(a3),d3
        sub.l   pd_SXBuff(a3),d3            ; d3 = BCur - Buff
        CLEAR   d0
        move.w  pd_NSXNodes(a3),d0          ; d0 = node count
        beq.s   psxl_short

        add.l   #PDSX_ShortBufSize,d3       ; d3 += ShortBufSize
        sub.w   #1,d0

        swap    d0
        lsr.l   #16-PDSX_LongBufExp,d0
        add.l   d0,d3                       ; d3 += (nodes-1) * LongBufSize

psxl_short
        moveq   #3,d0                       ; test length (must be >= 3 bytes)
        cmp.l   d0,d3
        bcs     psxl_err

            ; send to Interfaces
        LockMidiList                        ; lock MidiList

        move.l  d4,a1
        move.l  ml_Location(a1),d0          ; d0 <-- Cluster address
        beq     psxl_intdone                ; if point to air, don't send
        move.l  d0,a1                       ; a1 <-- Cluster address
        lea     mcl_Receivers(a1),a1        ; a1 <-- address of receiver list
        bra.s   psxl_intgo

psxl_intloop
        move.l  d6,a1
psxl_intgo
        move.l  (a1),a1                     ; a1 <-- next node in receiver list
        tst.l   (a1)                        ; is this the end of list?
        beq     psxl_intdone
        move.l  a1,d6

            ; check type filter
        btst.b  #CMB_SysEx&7,ml_EventTypeMask+(31-CMB_SysEx)/8(a1)
        beq.s   psxl_intloop

            ; check sys/ex filter
        lea     pd_SXShortBuffer(a3),a0     ; a0 = sys/ex message pointer for TestSysExFilter
        move.l  a1,a2
        bsr     TestSysExFilter             ; returns Z=set on match
        move.l  a2,a1                       ; doesn't affect flags
        bne.s   psxl_intloop

        move.l  ml_MidiNode(a1),mir         ; a2 <-- address of MidiNode

        move.l  mi_ReceiveHook(mir),d0      ; is there a receive hook?
        beq.s   psxl_nohook                 ; no, skip

        bsr     psxl_dohook                 ; call hook
        move.l  d6,a1                       ; A1 = MidiLink pointer
        move.l  ml_MidiNode(a1),mir         ; a2 <-- address of MidiNode

psxl_nohook
            ; check error flags
        move.b  mi_ErrFlags(mir),d0
        and.b   #CMEF_BufferFull!CMEF_SysExFull,d0
        bne.s   psxl_intloop

            ; check buffer size
        cmp.l   mi_SysExQueueSize(mir),d3   ; is the buffer big enough to hold this sys/ex?
        bcc.s   psxl_toobig                 ; if len >= bufsize, send sys/ex toobig error

            ; check for buffer overflow
        move.l  mi_SysExQueueHead(mir),d0   ; D0 = head-tail
        sub.l   mi_SysExQueueTail(mir),d0
        bhi.s   psxl_ovwrap
        add.l   mi_SysExQueueSize(mir),d0   ; if occupied part of buffer doesn't wrap
                                            ; complement by buffer size
psxl_ovwrap
        cmp.l   d0,d3                       ; will this message fit in the remaining buffer?
        bcc.s   psxl_overflow               ; if len >= empty space, skip (takes into account overflow pad)

        ; we now use a1, confident it will be restored at top of loop

        move.l  mi_SysExQueueTail(mir),a1   ; A1 = dest buffer
        move.w  pd_NSXNodes(a3),d5          ; d5 = node count
        beq.s   psxl_lastbuf

        lea     pd_SXShortBuffer(a3),a0     ; A0 = src
        move.l  #PDSX_ShortBufSize,d0       ; D0 = length
        bsr     PostSysEx

        sub.w   #1,d5                       ; d5-- (count-1)
        move.l  pd_SXList(a3),a4            ; walk SXList, A4 = SXNode
        bra.s   psxl_sxnloopend

psxl_sxnloop
        lea     psxn_Buffer(a4),a0          ; A0 = src
        move.l  #PDSX_LongBufSize,d0        ; D0 = length
        bsr     PostSysEx
        move.l  (a4),a4
psxl_sxnloopend
        dbra    d5,psxl_sxnloop

psxl_lastbuf
        move.l  pd_SXBuff(a3),a0            ; A0 = src
        move.l  pd_SXBCur(a3),d0
        sub.l   a0,d0                       ; D0 = length
        bsr     PostSysEx

        move.l  a1,d5                       ; save dest ptr in d5 (temp storage)

        lea     pd_CurMsg(a3),a1            ; A1 = MidiMsg pointer
        move.l  d6,a4                       ; A4 = MidiLink pointer
        move.b  ml_PortID(a4),mm_Port(a1)   ; set the port number
        bsr     PostMsg
        tst.w   d0                          ; posted ok?
        beq     psxl_intloop                ; if not, don't set Tail

        move.l  d5,mi_SysExQueueTail(mir)   ; store buffer point (loaded into d5 above)
        bra     psxl_intloop

psxl_overflow
        moveq   #CMEF_SysExFull,d1
        bsr     SendErrToMI
        bra     psxl_intloop

psxl_toobig
        moveq   #CMEF_SysExTooBig,d1
        bsr     SendErrToMI
        bra     psxl_intloop

psxl_intdone
        UnlockMidiList
        bra.s   psxl_ret

psxl_err
        move.l  d4,a1
        move.l  ml_MidiNode(a1),mir
        moveq   #CMEF_MsgErr,d1
        bsr     SendErrToMI

psxl_ret
        Pop
        rts

psxl_dohook
        ; we now use a1, confident it will be restored at top of loop
        movem.l a5/d7,-(sp)                 ; get a register
        move.l  a3,a5                       ; put ParserData there

        move.l  mi_ReceiveHook(mir),d7      ; d7 <-- address of receive hook

        lea     pd_CurMsg(a5),a1            ; A1 = MidiMsg pointer
        move.l  d6,a2                       ; A2 = MidiLink pointer
        move.b  ml_PortID(a2),mm_Port(a1)   ; set the port number

        move.w  pd_NSXNodes(a5),d5          ; d5 = node count
        beq.s   psxl_hklastbuf

        lea     pd_SXShortBuffer(a5),a3     ; A3 = src
        move.l  #PDSX_ShortBufSize,d0       ; D0 = length
        pea     psxl_donehook1
        move.l  d7,a0                       ; a0 <-- address of receive hook
        move.l  h_Entry(a0),-(sp)
        rts

psxl_donehook1
        sub.w   #1,d5                       ; d5-- (count-1)
        move.l  pd_SXList(a5),a4            ; walk SXList, A4 = SXNode
        bra.s   psxl_hksxnloopend

psxl_hksxnloop
        lea     psxn_Buffer(a4),a3          ; A3 = src
        move.l  #PDSX_LongBufSize,d0        ; D0 = length
        lea     pd_CurMsg(a5),a1            ; A1 = MidiMsg pointer
        move.l  d6,a2                       ; A2 = MidiLink pointer
        pea     psxl_donehook2
        move.l  d7,a0                       ; a0 <-- address of receive hook
        move.l  h_Entry(a0),-(sp)
        rts

psxl_donehook2
        move.l  (a4),a4
psxl_hksxnloopend
        dbra    d5,psxl_hksxnloop

psxl_hklastbuf
        move.l  pd_SXBuff(a5),a3            ; A0 = src
        move.l  pd_SXBCur(a5),d0
        sub.l   a3,d0                       ; D0 = length
        lea     pd_CurMsg(a5),a1            ; A1 = MidiMsg pointer
        move.l  d6,a2                       ; A2 = MidiLink pointer
        pea     psxl_donehook3
        move.l  d7,a0                       ; a0 <-- address of receive hook
        move.l  h_Entry(a0),-(sp)
        rts

psxl_donehook3
        move.l  a5,a3                       ; restore ParserData
        movem.l (sp)+,a5/d7                 ; restore registers
        rts


****************************************************************
*
*   PostSysEx
*
*   FUNCTION
*       Post sys/ex data to MidiNode->SysExQueue
*
*   INPUTS
*       A0 - source buffer
*       A1 - dest buffer
*       D0 - length
*       A2 - MidiNode (mir)
*       A6 - SysBase
*
*   RESULTS
*       A1 - resulting destination pointer
*
****************************************************************

; REM: The overhead of calling CopyMem may be greater than the
; savings of it's efficiency -- most SysEx messages should be 256
; bytes or shorter. --DJ

PostSysEx
        Push    d2

        move.l  mi_SysExQueueEnd(mir),d1
        sub.l   a1,d1                       ; D1 = bytes before SysExEnd
        move.l  d0,d2
        sub.l   d1,d2                       ; D2 = bytes after SysExEnd
        bls.s   psx_copysimple              ; if <= 0, simple copy

                    ; fragmented copy
        move.l  d1,d0
        CopyMem

        move.l  mi_SysExQueue(mir),a1
        move.l  d2,d0
psx_copysimple      ; simple copy
        CopyMem

        cmp.l   mi_SysExQueueEnd(mir),a1
        bcs.s   psx_nowrap
        move.l  mi_SysExQueue(mir),a1
psx_nowrap

        Pop
        rts



****************************************************************
*
*   PostMsg
*
*   FUNCTION
*       Post MidiMsg to MidiNode->MsgQueue
*
*   INPUTS
*       A1 - MidiMsg
*       A2 - MidiNode (mir)
*       A4 - MidiLink receiving the message
*       A5 - CamdBase (for internal hook to use)
*       A6 - SysBase
*
*   RESULTS
*       (WORD) TRUE if message was posted.  FALSE on error.
*
*   NOTE
*       Assumes caller has MidiList locked.
*
****************************************************************

PostMsg
        COLORBUG $0ff0
        Push    mir/a1/d2/d3                                    ; preserve MidiMsg (a1)
        move.l  a1,d2                       ; d2 <-- copy of message dest (for hook)

        CLEAR   d3                          ; default return value (FALSE)

            ; if MidiNode has no message queue, then skip this one

        tst.l   mi_MsgQueue(mir)            ; check to see if has msg queue
        beq.s   pm_nosig                    ; if not, then skip queue processing

            ; check error flags

        btst.b  #CMEB_BufferFull,mi_ErrFlags(mir)
        bne.s   pm_nosig

        move.l  mi_MsgQueueTail(mir),a0     ; a0 = buffer tail (location of next byte to fill)
        cmp.l   mi_MsgQueueHead(mir),a0     ; is the buffer currently empty?
        seq     d0                          ; d0 = TRUE if buffer is empty (used below)

        move.l  (a1)+,(a0)+                 ; copy MidiMsg
        move.l  (a1),(a0)+                  ; A1 is no longer valid

            ; Set timestamp of posted message

        move.l  mi_TimeReference(mir),d1    ; see if there's a timestamp vector
        beq.s   pm_nostamp                  ; if so, then
        move.l  d1,a1                       ; a1 = address of timestamp
        move.l  (a1),-4(a0)                 ; set timestamp
pm_nostamp:

        cmp.l   mi_MsgQueueEnd(mir),a0
        bcs.s   pm_nowrap
        move.l  mi_MsgQueue(mir),a0
pm_nowrap

        cmp.l   mi_MsgQueueHead(mir),a0     ; is the buffer about to overflow?
        bne.s   pm_settail

        COLORBUG $0f00

        moveq   #CMEF_BufferFull,d1     ; set error flag and signal
        bsr.s   SendErrToMI
        bra.s   pm_nosig                        ; and don't update MsgTail pointer

pm_settail
        move.l  a0,mi_MsgQueueTail(mir)
        tst.b   d0                      ; was the buffer empty?
        beq.s   pm_nosig                ; is so, then fall thru here to signal

        CLEAR   d0
        move.b  mi_ReceiveSigBit(mir),d1   ; d1 = signal bit
        bmi.s   pm_nosig                ; if no signal, ignore
        bset.l  d1,d0                   ; d0 = signal mask
        move.l  mi_SigTask(mir),a1      ; a1 = task to signal
        JSRLIB  Signal
        moveq   #1,d3                   ; return TRUE
        COLORBUG $0fff

pm_nosig
; REM: Need to call the hook here, but we need the MidiLink, and the
; MidiMsg address...
; REM: What if we say that the receiver hook works regardless
; of buffering?

        move.l  mi_ReceiveHook(mir),d0  ; d0 <-- address of receive hook
        beq.s   pm_ret                  ; if no hook, then return

        move.l  d2,a1                   ; a1 <-- address of message (dest)

        cmpi.b  #MS_SysEx,(a1)          ; don't call hook for SysEx
        beq.s   pm_ret

        move.l  d0,a0                   ; a0 <-- address of hook
        move.l  a4,a2                   ; a2 <-- address of MidiLink?
        pea     pm_ret
        move.l  h_Entry(a0),-(sp)
        rts

pm_ret
        move.l  d3,d0
        Pop
        rts



****************************************************************
*
*   SysExLen
*
*   FUNCTION
*       Calculate length of sys/ex message from F0 to F7.
*
*   INPUTS
*       A1 - sys/ex message
*
*   RESULTS
*       D0 - Length
*
****************************************************************

; Input register changed to A1 by --DJ

SysExLen
        move.b  #MS_EOX,d0              ; D0 = EOX byte for comparison
        move.l  a1,a0                   ; A0 = buffer pointer

sxl_loop
        cmp.b   (a1)+,d0
        bne.s   sxl_loop

        move.l  a1,d0
        sub.l   a0,d0                   ; D0 = byte count from loop
        rts


****************************************************************
*
*   TestSysExFilter
*
*   FUNCTION
*       Test sys/ex message against MidiLink SysExFilter.
*
*   INPUTS
*       A0 - sys/ex message
*       A2 - MidiLink
*
*   RESULTS
*       Z - set on match, clear otherwise
*
*   NOTE
*       This makes assumptions about the SysExFilter structure
*       and SXF modes.
*
****************************************************************

TestSysExFilter
        addq.l  #1,a0                       ; offset to manuf. id in message
        lea     ml_SysExFilter(a2),a1

        move.b  (a1)+,d1                    ; !!! assumes structure of SysExFilter
        beq.s   tsxf_done                   ; quick test for 0 here: match all (Z is also set)

        cmp.b   #SXFM_3Byte,d1              ; a 3-byte id?
        bcc.s   tsxf_ext

            ; match a 1-byte id
        and.w   #SXF_CountBits,d1           ; D1 = count
        subq.w  #1,d1                       ; always is >= 1 if we got past 0 check above
        move.b  (a0),d0                     ; D0 = match byte (from sys/ex message)
tsxf_1loop
        cmp.b   (a1)+,d0
        dbeq    d1,tsxf_1loop
        rts

tsxf_ext    ; match a 3-byte id
        moveq   #2,d1                       ; D1 = count
tsxf_3loop
        cmp.b   (a0)+,(a1)+
        dbne    d1,tsxf_3loop
tsxf_done
        rts

        ifne 0
****************************************************************
*
*   SendErrToPort
*
*   FUNCTION
*       Distribute error flags to all MidiNodes listening
*       to the specified Port.
*
*   INPUTS
*       A2  - MidiNode
*       D1.b - error flags
*       A5 - CamdBase
*       A6 - SysBase
*
*   RESULTS
*       None
*
****************************************************************

SendErrToPort
        Push    mir/d2-d3

        move.b  d0,d2                       ; D2 = Port #
        move.b  d1,d3                       ; D3 = Error flags

        LockMidiList

        lea     camd_RecvList(a5),mir       ; walk CamdBase->camd_RecvList, mir = MidiNode
setp_intloop
        move.l  (mir),mir
        tst.l   (mir)
        beq.s   setp_intdone

        move.l  mi_PortFilter(mir),d0
        btst.l  d2,d0
        beq     setp_intloop

        move.b  d3,d1
        bsr.s   SendErrToMI

        bra     setp_intloop

setp_intdone
        UnlockMidiList

        Pop
        rts

        endc

****************************************************************
*
*   SendErrToMI
*
*   FUNCTION
*       Distribute error flags to a MidiNodes.
*
*   INPUTS
*       D1.b - error flags
*       A2 - MI pointer (mir)
*       A5 - CamdBase
*       A6 - SysBase
*
*   RESULTS
*       None
*
*   NOTE
*       Assumes caller has the MidiList locked.
*
****************************************************************

SendErrToMI
        and.b   mi_ErrFilter(mir),d1        ; set only those bits that are requested by the client
        bne.s   setm_signal                 ; if no bits left, don't set signal
        rts

setm_signal
        or.b    d1,mi_ErrFlags(mir)         ; set error flags
        CLEAR   d0                          ; Signal task
        move.b  mi_ReceiveSigBit(mir),d1       ; d1 = signal bit
        bset.l  d1,d0                       ; d0 = signal mask
        move.l  mi_SigTask(mir),a1          ; a1 = task to signal
        JMPLIB  Signal

        end
