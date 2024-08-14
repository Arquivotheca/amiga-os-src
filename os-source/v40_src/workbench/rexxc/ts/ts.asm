* === ts.asm ===========================================================
*
* Copyright © 1986,1987,1988,1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* Sets the global 'TRACE' flag bit.

         INCLUDE  "rexx/storage.i"
         INCLUDE  "rexx/rxslib.i"
         INCLUDE  "rexx/errors.i"

         XREF     _AbsExecBase

         XLIB     Output
         XLIB     Write

         XLIB     CloseLibrary
         XLIB     FindPort
         XLIB     OpenLibrary

start:   movea.l  _AbsExecBase,a6
         moveq    #0,d6

         ; Open the DOS library

         lea      DOSLib(pc),a1
         moveq    #0,d0
         CALLSYS  OpenLibrary
         move.l   d0,d5

         ; Look for the REXX port

         lea      RexxName(pc),a1
         CALLSYS  FindPort
         tst.l    d0                   ; port found?
         beq.s    NoPort               ; no

         ; Open the REXX library

         lea      RXSLib(pc),a1
         moveq    #RXSVERS,d0
         CALLSYS  OpenLibrary
         tst.l    d0                   ; opened?
         beq.s    NoREXX               ; no ... error
         move.l   d0,a5

         ; Set the global trace flag

         bset     #RLFB_TRACE,rl_Flags(a5)
         bra.s    CloseREXX

         ; REXX port not found 

NoPort:  lea      ErrMsg1(pc),a0
         bsr.s    WriteMsg
         moveq    #RC_ERROR,d6
         bra.s    CloseDOS

         ; REXX library not found ... print error message and exit.

NoREXX:  lea      ErrMsg2(pc),a0       ; message
         bsr.s    WriteMsg
         moveq    #RC_ERROR,d6
         bra.s    CloseDOS

         ; Close the REXX library

CloseREXX:
         movea.l  a5,a1
         CALLSYS  CloseLibrary

         ; Close the DOS library

CloseDOS:
         movea.l  d5,a1
         CALLSYS  CloseLibrary

         move.l   d6,d0                ; return code
         rts

* ===========================     WriteMsg     =========================
* Display an error message
* Registers:   A0 -- string
WriteMsg:
         movem.l  d2/d3,-(sp)
         exg      d5,a6

         move.l   a0,d2                ; message
1$:      tst.b    (a0)+                ; null?
         bne.s    1$                   ; loop back
         subq.l   #1,a0                ; back up
         move.l   a0,d3
         sub.l    d2,d3                ; length
         CALLSYS  Output
         move.l   d0,d1                ; output stream
         CALLSYS  Write

         exg      d5,a6
         movem.l  (sp)+,d2/d3
         rts

         ; String constants

DOSLib   dc.b     'dos.library',0
RXSLib   RXSNAME
RexxName dc.b     'REXX',0

ErrMsg1  dc.b     'ARexx server not active',10,0
ErrMsg2  dc.b     'File LIBS:rexxsyslib.library not found',10,0
         CNOP     0,2

         END
