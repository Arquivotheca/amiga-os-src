* === hi.asm ===========================================================
*
* Copyright © 1986,1987,1988,1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* Sets the global 'HALT' flag bit.

         SECTION  hi,CODE

         INCLUDE  "rexx/storage.i"
         INCLUDE  "rexx/rxslib.i"
         INCLUDE  "rexx/errors.i"

         INCLUDE  "libraries/dos.i"
         INCLUDE  "libraries/dosextens.i"

         IFND     EXEC_EXECBASE
ThisTask EQU      276
         ENDC

         XREF     _AbsExecBase

         ; DOS library functions

         XLIB     Write

         ; EXEC library functions

         XLIB     CloseLibrary
         XLIB     FindPort
         XLIB     Forbid
         XLIB     OpenLibrary
         XLIB     Permit
         XLIB     Signal

start:   movea.l  _AbsExecBase,a6
         movea.l  ThisTask(a6),a4      ; our task

         moveq    #RC_ERROR,d6         ; default error

         CALLSYS  Forbid               ; forbid task switching

         ; Open the DOS library

         lea      DOSLib(pc),a1        ; DOS library
         moveq    #0,d0                ; any version
         CALLSYS  OpenLibrary
         move.l   d0,d5                ; "guaranteed"

         ; Look for the REXX port

         lea      REXXName(pc),a1      ; REXX port name
         CALLSYS  FindPort             ; D0=port
         move.l   d0,d4                ; port found?
         beq.s    NoPort               ; no

         ; Open the REXX library

         lea      RXSLib(pc),a1        ; REXX systems library
         moveq    #0,d0                ; any version
         CALLSYS  OpenLibrary
         tst.l    d0                   ; opened?
         beq.s    NoREXX               ; no ... error
         move.l   d0,a5

         ; Set the global HALT flag

         bset     #RLFB_HALT,rl_Flags(a5)

         ; Signal the RexxMaster task.

         movea.l  d4,a0
         movea.l  MP_SIGTASK(a0),a1    ; RexxMaster task
         move.b   MP_SIGBIT(a0),d1     ; signal bit
         moveq    #0,d0
         bset     d1,d0                ; signal mask
         CALLSYS  Signal

         moveq    #0,d6                ; clear error
         bra.s    CloseREXX

         ; REXX port not found 

NoPort   lea      ErrMsg1(pc),a0       ; "server not active"
         bsr.s    WriteMsg
         bra.s    CloseDOS

         ; REXX library not found ... print error message and exit.

NoREXX   lea      ErrMsg2(pc),a0       ; "library not found"
         bsr.s    WriteMsg
         bra.s    CloseDOS

         ; Close the REXX library

CloseREXX
         movea.l  a5,a1
         CALLSYS  CloseLibrary

         ; Close the DOS library

CloseDOS movea.l  d5,a1
         CALLSYS  CloseLibrary

         CALLSYS  Permit               ; reenable task switching
         move.l   d6,d0                ; return code
         rts

* ===========================     WriteMsg     =========================
* Displays an error message on the standard output stream.
* Registers:   A0 -- string
WriteMsg:
         movem.l  d2/d3/a6,-(sp)
         movea.l  d5,a6
         move.l   pr_COS(a4),d1        ; output stream?
         beq.s    2$                   ; no

         move.l   a0,d2                ; message
1$:      tst.b    (a0)+                ; null?
         bne.s    1$                   ; loop back
         subq.l   #1,a0                ; back up
         move.l   a0,d3
         sub.l    d2,d3                ; length
         CALLSYS  Write

2$:      movem.l  (sp)+,d2/d3/a6
         rts

         ; String constants

DOSLib   dc.b     'dos.library',0
RXSLib   RXSNAME
REXXName dc.b     'REXX',0             ; port name

ErrMsg1  dc.b     'ARexx server not active',10,0
ErrMsg2  dc.b     'Can',$27,'t open LIBS:rexxsyslib.library',10,0
         CNOP     0,2

         END
