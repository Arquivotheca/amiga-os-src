* == packets.asm =======================================================
*
* Copyright (c) 1986-1990 by William S. Hawes.  All Rights Reserved.
*
* $Id: packets.asm,v 1.4 90/11/02 16:49:09 mks Exp $
*
* $Log:	packets.asm,v $
* Revision 1.4  90/11/02  16:49:09  mks
* Fixed usage of rm_SIZEOF
* 
* Revision 1.3  90/09/05  10:34:36  mks
* Fixed enforcer hit on IsRexxMsg() when LN_NAME is NULL
*
* ======================================================================
*
* =========================     LockRexxBase     =======================
* Obtains a read lock on the specified resource.
* All registers are preserved (except for the CCR).
* Registers:      D0 -- resource
LockRexxBase
         addq.w   #1,rl_ReadLock(a6)
         rts

* =======================     UnlockRexxBase     =======================
* Releases a read lock on the specified resource.  If the resource is
* then unlocked, the manager task is signalled.
* All registers are preserved (except for the CCR).
* Registers:      D0 -- resource
UnlockRexxBase
         subq.w   #1,rl_ReadLock(a6)   ; still locked?
         bne.s    2$                   ; yes

         ; No locks outstanding ... signal the manager

         movem.l  d0/d1/a0/a1/a6,-(sp)
         move.l   (rl_RexxPort+MP_SIGTASK)(a6),d1
         beq.s    1$                   ; ... no task
         movea.l  d1,a1

         move.b   (rl_RexxPort+MP_SIGBIT)(a6),d1
         bmi.s    1$                   ; no signal??
         moveq    #0,d0
         bset     d1,d0                ; signal mask
         movea.l  rl_SysBase(a6),a6    ; EXEC base
         CALLSYS  Signal               ; signal

1$:      movem.l  (sp)+,d0/d1/a0/a1/a6

2$:      rts

* ========================     CreateRexxMsg     ========================
* Allocates and initializes a RexxMsg structure.
* Registers:      A0 -- reply port
*                 A1 -- file extension
*                 D0 -- host address
* Return:         D0 -- message packet or 0
*                 A0 -- same
CreateRexxMsg
         movem.l  d0/a0/a1,-(sp)       ; push host/reply/ext

         moveq    #RMSIZEOF/2,d0       ; 1/2 packet
         add.w    d0,d0                ; packet size
         bsr      GetMem               ; D0=A0=packet
         movem.l  (sp)+,d0/d1/a1       ; pop host/reply/ext
         beq.s    1$                   ; failure??

         ; Install values in the message packet

         move.l   d1,MN_REPLYPORT(a0)
         move.w   #RMSIZEOF,MN_LENGTH(a0)
         movem.l  d0/a1,rm_CommAddr(a0)

1$:      move.l   a0,d0                ; set CCR
CRM001   rts

* =========================     IsRexxMsg     ==========================
* Checks whether a message packet was issued by the interpreter.
* Registers:      A0 -- message packet
* Return:         D0 -- boolean flag
IsRexxMsg
         movea.l  LN_NAME(a0),a0       ; message name
         move.l   a0,d0                ; Is there a name?
         beq.s    CRM001               ; We steal this RTS for our use.
         movea.l  rl_REXX(a6),a1       ; REXX string structure
         addq.w   #ns_Buff,a1          ; offset to string
         moveq    #4+1-1,d1            ; loop count (includes null)
         bra      CmpStringLen         ; D0=boolean

* =======================     DeleteRexxMsg     =======================
* Releases a RexxMsg structure.
* Registers:      A0 -- message packet
DeleteRexxMsg
         moveq    #0,d0
         move.w   MN_LENGTH(a0),d0     ; packet length
         bra      GiveMem

* ========================     ClearRexxMsg     ========================
* Releases the argstrings in a RexxMsg structure and clears the slots.
* Registers:   A0 -- message packet
*              D0 -- count
ClearRexxMsg
         movem.l  d2/a2,-(sp)

         lea      ARG0(a0),a2          ; start of argblock
         moveq    #MAXRMARG+1,d2       ; maximum count
         cmp.w    d0,d2                ; max > count?
         bls.s    2$                   ; no ... use maximum
         move.w   d0,d2                ; use count
         bra.s    2$                   ; jump in

1$:      movea.l  (a2),a0              ; get argstring
         clr.l    (a2)+                ; clear slot
         CALLSYS  DeleteArgstring      ; release it
2$:      dbf      d2,1$                ; loop back

         movem.l  (sp)+,d2/a2
         rts

* =========================     FillRexxMsg     ========================
* Converts string pointers or integers in a RexxMsg structure to argstring
* form.  Bits set in the argument mask specify integer conversion.
* Registers:   A0 -- message packet
*              D0 -- argument count
*              D1 -- argument mask
* Return:      D0 -- boolean success
STACKBF  SET      16*4
FillRexxMsg:
         movem.l  d2-d4/a2-a4,-(sp)
         lea      -STACKBF(sp),sp      ; stack space
         movea.l  a0,a2                ; save message
         move.l   d1,d3                ; save mask

         ; Make sure the count is valid.

         moveq    #MAXRMARG+1,d2       ; maximum count
         cmp.w    d0,d2                ; max > count?
         bls.s    0$                   ; no ... use maximum
         move.w   d0,d2                ; use count

0$:      lea      ARG0(a2),a3          ; argblock
         movea.l  sp,a4                ; stack buffer
         move.w   d2,d4                ; counter
         bra.s    5$                   ; jump in

         ; Check each value in the message packet.

1$:      move.l   (a3)+,d0             ; load the value
         asr.w    #1,d3                ; an integer?
         bcs.s    2$                   ; yes

         ; Convert a (null-terminated) string pointer

         tst.l    d0                   ; a pointer?
         beq.s    4$                   ; no
         movea.l  d0,a0
         CALLSYS  Strlen               ; D0=length
         CALLSYS  CreateArgstring      ; D0=A0=argstring
         bra.s    3$

         ; Convert an integer ...

2$:      moveq    #12,d1               ; maximum digits
         CALLSYS  CVi2arg              ; D0=A0=argstring

         ; Check if the allocation succeeded.

3$:      tst.l    d0                   ; success?
         beq.s    6$                   ; no

         ; Install the value in the stack buffer

4$:      move.l   d0,(a4)+             ; save the result

5$:      dbf      d4,1$                ; loop back
         moveq    #-1,d3               ; success
         bra.s    FRMCopy

6$:      moveq    #0,d3                ; failure ...

         ; Copy the values to the message packet, or release the partials.

FRMCopy: lea      ARG0(a2),a3          ; argblock
         movea.l  sp,a4                ; buffer area
         addq.w   #1,d4                ; remaining count
         sub.w    d4,d2                ; converted count
         bra.s    2$                   ; jump in

1$:      move.l   (a4)+,(a3)+          ; a result?
         beq.s    2$                   ; no
         tst.l    d3                   ; success?
         bne.s    2$                   ; yes

         ; Release the value argstring

         movea.l  -(a3),a0             ; argstring
         CALLSYS  DeleteArgstring
         clr.l    (a3)+                ; clear the slot

2$:      dbf      d2,1$                ; loop back

         move.l   d3,d0                ; boolean return
         lea      STACKBF(sp),sp       ; restore stack
         movem.l  (sp)+,d2-d4/a2-a4
         rts

* =========================     SendRexxMsg     ========================
* Sends a packet to the specified host address and waits for the reply.
* Registers:      A0 -- global data pointer
*                 A1 -- host address (argstring)
*                 D0 -- message packet
* Return:         D0 -- error code or 0
*                 D1 -- return code (if no error)
SendRexxMsg
         movem.l  d2/a2/a3/a6,-(sp)
         movea.l  d0,a2                ; message packet
         movea.l  a0,a3                ; global pointer
         move.l   a1,d2                ; port name

         clr.l    RESULT2(a2)          ; clear result slot

         ; Install the STDIN and STDOUT streams in the message.

         movea.l  rl_STDIN(a6),a1      ; STDIN name
         bsr      FindLogical          ; D0=A0=IoBuff
         beq.s    1$                   ; ... not found
         move.l   iobDFH(a0),d0        ; get the filehandle
1$:      move.l   d0,rm_Stdin(a2)      ; install it

         movea.l  a3,a0                ; global pointer
         movea.l  rl_STDOUT(a6),a1     ; STDOUT name
         bsr      FindLogical          ; D0=A0=IoBuff
         beq.s    2$                   ; ... not found
         move.l   iobDFH(a0),d0        ; get the filehandle
2$:      move.l   d0,rm_Stdout(a2)     ; install it

         ; Check for the special 'COMMAND' host.

         movea.l  rl_COMMAND(a6),a0    ; "COMMAND"
         movea.l  d2,a1                ; host address
         subq.w   #ns_Buff,a1          ; string structure
         CALLSYS  CmpString            ; D0=boolean
         bne.s    3$                   ; matched ... DOS call

         ; Disable task switching, then look for the host's port.

         movea.l  rl_SysBase(a6),a6    ; EXEC base
         CALLSYS  Forbid               ; no switching

         movea.l  d2,a1                ; host address
         CALLSYS  FindPort             ; D0=port
         tst.l    d0                   ; port found?
         beq.s    SRMError             ; no ... error

         ; Set our wait flag and then send the message.

         moveq    #RTFB_WAIT,d2        ; wait flag
         bset     d2,rt_Flags(a3)      ; set flag
         movea.l  d0,a0                ; port address
         movea.l  a2,a1                ; message packet
         CALLSYS  PutMsg               ; send it off
         CALLSYS  Permit               ; reenable switching

         ; Wait for the reply.

         lea      rt_MsgPort(a3),a0    ; our replyport
         CALLSYS  WaitPort             ; D0=message
         lea      rt_MsgPort(a3),a0    ; our replyport
         CALLSYS  GetMsg               ; D0=message
         bclr     d2,rt_Flags(a3)      ; clear flag
         move.l   RESULT1(a2),d1       ; return code
         bra.s    SRMDone

         ; Special 'COMMAND' host ... issue the command directly to DOS.

3$:      movea.l  ARG0(a2),a0          ; command string
         movem.l  rm_Stdin(a2),d0/a1   ; input/output streams
         CALLSYS  DOSCommand           ; D0=boolean D1=return
         beq.s    SRMErr13             ; ... command failed

         ; Command issued successfully ... clear return.

SRMDone  moveq    #0,d0                ; success
         bra.s    SRMOut

         ; Error ... port not found.

SRMError CALLSYS  Permit               ; reenable switching

SRMErr13 moveq    #ERR10_013,d0        ; "host not found"

SRMOut   movem.l  (sp)+,d2/a2/a3/a6
         rts

* ===========================     rxSuspend    =========================
* Checks whether execution to be suspended, checks for a global tracing
* console is to be installed, and returns the interrupt flags.
* Registers:   A0 -- global data pointer
* Return:      D0 -- interrupt flags
DOSBREAK SET      (1<<15)!(1<<14)!(1<<13)!(1<<12)
rxSuspend
         movea.l  (rt_MsgPort+MP_SIGTASK)(a0),a1
         move.l   TC_SIGRECVD(a1),d0   ; task signals
         move.b   rt_Flags(a0),d0      ; external flags
         andi.l   #DOSBREAK!1<<RTFB_TCUSE!1<<RTFB_SUSP!1<<RTFB_HALT,d0
         or.l     rl_TraceFH(a6),d0    ; trace filehandle?
         bne.s    rSStart              ; ... check further
         rts

         ; Possible external event ... check further.

rSStart  movem.l  d2/a3/a5/a6,-(sp)    ; delayed save
         movea.l  a0,a3
         movea.l  rl_SysBase(a6),a5    ; EXEC base

         ; Check whether the task is to be suspended.

         exg      a5,a6                ; REXX=>A5 , EXEC=>A6
1$:      btst     #RTFB_SUSP,rt_Flags(a3)
         beq.s    2$                   ; ... not suspended
         move.b   rt_SigBit(a3),d1     ; signal bit
         moveq    #0,d0
         bset     d1,d0                ; signal mask
         CALLSYS  Wait                 ; wait for an event
         bra.s    1$                   ; loop back to check

2$:      CALLSYS  Forbid               ; no switching
         exg      a5,a6                ; EXEC=>A5 , REXX=>A6

         ; Check whether the global tracing console is open.

         move.l   rl_STDERR(a6),a1     ; STDERR name
         move.l   rl_TraceFH(a6),d1    ; global console?
         beq.s    3$                   ; no

         ; Global tracing console is available ... open up STDERR

         lea      FILELIST(a3),a0      ; our File List
         exg      d1,a1                ; name=>D1, filehandle=>A1
         moveq    #RXIO_EXIST,d0       ; existing filehandle
         addq.l   #ns_Buff,d1          ; name string
         CALLSYS  OpenF                ; D0=A0=IoBuff
         beq.s    4$                   ; failure ...
         bset     #RTFB_TCUSE,rt_Flags(a3)
         bra.s    4$

         ; Global console not open ... close STDERR if necessary.

3$:      bclr     #RTFB_TCUSE,rt_Flags(a3)
         beq.s    4$                   ; ... not opened
         movea.l  a3,a0                ; global pointer
         bsr      FindLogical          ; D0=A0=IoBuff
         beq.s    4$                   ; ... not found
         CALLSYS  CloseF               ; close it

         ; Reenable task switching

4$:      movea.l  a5,a6                ; EXEC=>A6
         CALLSYS  Permit               ; reenable switching

         ; Clear 'break' signals and determine if any interrupts are set.

         move.l   #DOSBREAK,d2         ; break signal mask
         moveq    #0,d0                ; clear all
         move.l   d2,d1                ; signal mask
         CALLSYS  SetSignal            ; D0=signals
         and.l    d2,d0                ; break signals
         moveq    #(SIGBREAKB_CTRL_C-INB_BREAK_C),d1
         lsr.l    d1,d0                ; align flags

         ; See if the external HALT flag is set ...

         bclr     #RTFB_HALT,rt_Flags(a3)
         beq.s    5$                   ; not set
         bset     #INB_HALT,d0         ; set 'HALT' flag

5$:      tst.l    d0                   ; set CCR
         movem.l  (sp)+,d2/a3/a5/a6
         rts

* ======================     OpenPublicPort     ========================
* Opens a public message port (as a resource node), and links it into
* the system ports list.
* Registers:   A0 -- header
*              A1 -- name
* Return:      D0 -- port resource node or 0
*              A0 -- same
OpenPublicPort:
         movem.l  a2/a6,-(sp)

         ; Allocate a message port and link it into the resource list.

         moveq    #rmp_SIZEOF,d0
         CALLSYS  AddRsrcNode          ; D0=A0=node
         beq.s    2$                   ; allocation failure

         ; Install fields in the resource node.  Ports are "auto-delete",
         ; so ClosePublicPort will be called when the node is removed.

         movea.l  a0,a2
         move.b   #RRT_PORT,RRTYPE(a2)
         move.l   a6,rr_Base(a2)
         move.w   #_LVOClosePublicPort,rr_Func(a2)

         ; Initialize the replylist (for messages awaiting reply).

         lea      rmp_ReplyList(a2),a0
         CALLSYS  InitList

         ; Initialize the message port, then make it public.

         lea      rmp_Port(a2),a0
         movea.l  RRNAME(a2),a1        ; name (saved in node)
         CALLSYS  InitPort             ; D0=signal  A1=port
         bpl.s    1$                   ; signal allocated ...

         ; Signal couldn't be allocated, so the node is released.

         movea.l  a2,a0
         CALLSYS  ClosePublicPort
         moveq    #0,d0                ; NULL return
         bra.s    2$

         ; Make the message port public ...

1$:      movea.l  rl_SysBase(a6),a6
         CALLSYS  AddPort              ; link it in
         move.l   a2,d0                ; return node

2$:      movea.l  d0,a0
         movem.l  (sp)+,a2/a6
         rts

* =======================     ClosePublicPort     =======================
* Closes a named message port and releases the node.  This function is
* called automatically when a port resource node is released.
* Registers:   A0 -- port node
ClosePublicPort:
         movem.l  a2/a5,-(sp)
         movea.l  a0,a2
         movea.l  rl_SysBase(a6),a5

         ; Unlink the port from the system list, if it was linked.

         exg      a5,a6
         lea      rmp_Port(a2),a1      ; message port
         tst.l    (a1)                 ; ever linked?
         beq.s    1$                   ; no
         CALLSYS  RemPort              ; unlink it

         ; Check for any queued messages ...

1$:      lea      rmp_Port(a2),a0
         CALLSYS  GetMsg               ; D0=message
         tst.l    d0                   ; a message?
         bne.s    2$                   ; yes

         ; Check for messages in the replylist ...

         lea      rmp_ReplyList(a2),a0
         CALLSYS  RemTail              ; D0=node
         tst.l    d0                   ; a message?
         beq.s    3$                   ; no -- all done

         ; Reply the message, setting the return code.

2$:      movea.l  d0,a1                ; message packet
         move.l   #RC_ERROR,RESULT1(a1)
         CALLSYS  ReplyMsg             ; reply it
         bra.s    1$

         ; Free the port resources, and mark it as "closed".

3$:      exg      a5,a6
         tst.b    (rmp_Port+MP_SIGBIT)(a2)   ; signal bit?
         bmi.s    4$                         ; no
         lea      rmp_Port(a2),a0
         CALLSYS  FreePort

         ; Recycle the node, unless this is an auto-delete call.

4$:      tst.l    rr_Base(a2)          ; auto-delete?
         beq.s    5$                   ; yes
         clr.l    rr_Base(a2)          ; clear it
         movea.l  a2,a0
         CALLSYS  RemRsrcNode

5$:      movem.l  (sp)+,a2/a5
         rts
