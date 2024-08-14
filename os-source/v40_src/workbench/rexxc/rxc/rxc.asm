* === rxc.asm ==========================================================
*
* Copyright © 1986,1987,1988,1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* Sends a "CLOSE" command packet to the rexxmaster.  The REXX public port  
* is withdrawn and the RexxMaster closes after the last task finishes.

         INCLUDE  "rexx/storage.i"
         INCLUDE  "rexx/rxslib.i"
         INCLUDE  "rexx/errors.i"

         INCLUDE  "libraries/dos.i"
         INCLUDE  "libraries/dosextens.i"

         IFND     EXEC_EXECBASE_I
ThisTask EQU      276
         ENDC

         XREF     _AbsExecBase

         ; External library routines called

         XLIB     Close
         XLIB     Delay
         XLIB     Open
         XLIB     Output
         XLIB     Write

         XLIB     CloseLibrary
         XLIB     FindPort
         XLIB     Forbid
         XLIB     GetMsg
         XLIB     OpenLibrary
         XLIB     Permit
         XLIB     PutMsg
         XLIB     ReplyMsg
         XLIB     WaitPort

         ; The initial entry point for launching from either the CLI or WB.

start:   movea.l  _AbsExecBase,a6
         movea.l  ThisTask(a6),a4

         moveq    #0,d6                ; error code
         moveq    #0,d7                ; WB message

         ; See whether we were started by a CLI or by WorkBench

         tst.l    pr_CLI(a4)           ; a CLI?
         bne.s    1$                   ; yes

         ; Get the WB startup message

         lea      pr_MsgPort(a4),a0    ; our processid
         CALLSYS  WaitPort
         lea      pr_MsgPort(a4),a0
         CALLSYS  GetMsg               ; return: D0=message
         move.l   d0,d7                ; save the message

         ; Open the DOS library

1$:      lea      DOSLib(pc),a1        ; library name
         moveq    #0,d0                ; any version
         CALLSYS  OpenLibrary          ; D0=library
         move.l   d0,d5                ; assume success

         ; Forbid task switching

         CALLSYS  Forbid               ; no switching ...

         ; Look for the REXX port

         lea      RexxName(pc),a1      ; REXX public port
         CALLSYS  FindPort             ; D0=port
         move.l   d0,d4                ; port found?
         beq.s    NoPort               ; no

         ; Open the REXX library (should always succeed)

         lea      RXSLib(pc),a1
         moveq    #0,d0                ; any version
         CALLSYS  OpenLibrary
         tst.l    d0                   ; library found?
         beq.s    NoREXX               ; no
         move.l   d0,a5

         ; Allocate a message packet.  No replyport is given, since the
         ; message is sent as a "no return" packet.

         suba.l   a0,a0                ; no replyport
         suba.l   a1,a1                ; no file extension
         moveq    #0,d0                ; no host address
         exg      a5,a6
         CALLSYS  CreateRexxMsg        ; D0=A0=message
         exg      a5,a6
         move.l   #RXCLOSE!RXFF_NONRET,ACTION(a0)

         movea.l  a0,a1                ; packet
         movea.l  d4,a0                ; REXX port
         CALLSYS  PutMsg               ; send the message ...
         bra.s    CloseREXX

         ; Error messages

NoPort   lea      ErrMsg1(pc),a0       ; REXX port not open
         bsr.s    WriteMsg
         moveq    #RC_WARN,d6          ; return code
         bra.s    CloseDOS

NoREXX   lea      ErrMsg2(pc),a0       ; Couldn't open REXX library
         bsr.s    WriteMsg
         moveq    #RETURN_ERROR,d6     ; return code
         bra.s    CloseDOS

CloseREXX:
         movea.l  a5,a1
         CALLSYS  CloseLibrary

CloseDOS movea.l  d5,a1
         CALLSYS  CloseLibrary

         CALLSYS  Permit               ; reenable task switching

         ; Check whether a WorkBench message was sent.

         tst.l    d7                   ; WB invocation?
         beq.s    1$                   ; no

         ; Close the output stream

         exg      d5,a6
         moveq    #100,d1              ; timeout
         CALLSYS  Delay
         move.l   pr_COS(a4),d1        ; output stream
         CALLSYS  Close                ; close it
         exg      d5,a6

         ; Return the WorkBench Message

         CALLSYS  Forbid               ; no switching, please
         movea.l  d7,a1                ; message
         CALLSYS  ReplyMsg             ; reply it

1$:      move.l   d6,d0
         rts

* =========================      WriteMsg     =========================
* Writes a message to the console ...
* Registers:   A0 -- message
*              D0 -- length
WriteMsg
         movem.l  d2/d3/a6,-(sp)
         movea.l  d5,a6

         move.l   pr_COS(a4),d0        ; a stream?
         bne.s    1$                   ; yes
         tst.l    d7                   ; WB invocation?
         beq.s    3$                   ; no

         lea      CONDef(pc),a1        ; console
         move.l   a1,d1
         move.l   #MODE_NEWFILE,d2
         move.l   a0,-(sp)             ; push message
         CALLSYS  Open                 ; D0=filehandle
         movea.l  (sp)+,a0             ; pop message
         move.l   d0,pr_COS(a4)        ; install it

1$:      move.l   d0,d1
         move.l   a0,d2                ; buffer

2$:      tst.b    (a0)+                ; null?
         bne.s    2$                   ; loop back
         subq.l   #1,a0
         move.l   a0,d3
         sub.l    d2,d3                ; length
         CALLSYS  Write                ; write it ...

3$:      movem.l  (sp)+,d2/d3/a6
         rts

         ; String constants

DOSLib   dc.b     'dos.library',0
RXSLib   RXSNAME
RexxName dc.b     'REXX',0
CONDef   dc.b     'CON:160/80/320/40/Output/ads',0

ErrMsg1  dc.b     'ARexx server not active',10,0
ErrMsg2  dc.b     'Can',$27,'t open rexxsyslib.library',10,0
         CNOP     0,2

         END
