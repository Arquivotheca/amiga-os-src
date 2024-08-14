* === tcc.asm ==========================================================
*
* Copyright © 1986,1987,1988,1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* Closes the global tracing console.

         INCLUDE  "rexx/storage.i"
         INCLUDE  "rexx/rxslib.i"
         INCLUDE  "rexx/errors.i"

         XREF     _AbsExecBase

         ; DOS library functions

         XLIB     Output
         XLIB     Write

         ; EXEC library functions

         XLIB     CloseLibrary
         XLIB     Forbid
         XLIB     FindPort
         XLIB     OpenLibrary
         XLIB     Permit
         XLIB     PutMsg

start:   movea.l  _AbsExecBase,a6
         moveq    #0,d6

         CALLSYS  Forbid               ; no task switching

         ; Open the DOS library

         lea      DOSLib(pc),a1
         moveq    #0,d0
         CALLSYS  OpenLibrary
         move.l   d0,d5                ; DOS library

         ; Look for the REXX public port

         lea      RexxName(pc),a1      ; REXX port name
         CALLSYS  FindPort
         move.l   d0,d4                ; port found?
         beq.s    Error1               ; no

         ; Open the REXX library

         lea      RXSLib(pc),a1
         moveq    #0,d0                ; any version
         CALLSYS  OpenLibrary
         tst.l    d0                   ; success?
         beq.s    Error2               ; no
         move.l   d0,a5                ; REXX library

         ; Allocate a message packet (with no return port). 

         suba.l   a0,a0                ; no replyport
         suba.l   a1,a1                ; no fileext
         moveq    #0,d0                ; no host name
         exg      a5,a6
         CALLSYS  CreateRexxMsg        ; D0=A0=message
         exg      a5,a6
         beq.s    Error3
         move.l   #RXTCCLS!RXFF_NONRET,ACTION(a0)

         ; Send it off.  The message has been tagged as 'no return', so
         ; the Rexxmaster will recycle it for us.

         movea.l  a0,a1                ; message
         movea.l  d4,a0                ; REXX port
         CALLSYS  PutMsg
         bra.s    CloseREXX

         ; REXX port not found ...

Error1   lea      ErrMsg1(pc),a0
         bsr.s    WriteMsg
         moveq    #RC_WARN,d6
         bra.s    CloseDOS

Error2   lea      ErrMsg2(pc),a0       ; library not found
         bsr.s    WriteMsg
         moveq    #RC_ERROR,d6
         bra.s    CloseDOS

         ; Message packet not allocated ...

Error3   lea      ErrMsg3(pc),a0
         bsr.s    WriteMsg
         moveq    #RC_ERROR,d6

         ; Close the REXX library

CloseREXX
         movea.l  a5,a1
         CALLSYS  CloseLibrary

         ; Close the DOS library

CloseDOS
         movea.l  d5,a1
         CALLSYS  CloseLibrary

         CALLSYS  Permit               ; reenable task switching
         move.l   d6,d0                ; return code
         rts

* ===========================     WriteMsg     =========================
* Display an error message
* Registers:   A0 -- string
WriteMsg movem.l  d2/d3/a6,-(sp)
         movea.l  d5,a6

         move.l   a0,d2                ; message
         moveq    #-1,d3
1$:      addq.l   #1,d3                ; bump length
         tst.b    (a0)+                ; null?
         bne.s    1$                   ; loop back

         CALLSYS  Output               ; D0=output stream
         move.l   d0,d1
         CALLSYS  Write

         movem.l  (sp)+,d2/d3/a6
         rts

         ; String data

DOSLib   dc.b     'dos.library',0
RXSLib   RXSNAME                       ; library name
RexxName dc.b     'REXX',0             ; REXX port name

         ; String constants

ErrMsg1  dc.b     'ARexx server not active',10,0
ErrMsg2  dc.b     'Unable to open LIBS:rexxsyslib.library',10,0
ErrMsg3  dc.b     'Memory allocation failure',10,0
         CNOP     0,2

         END
