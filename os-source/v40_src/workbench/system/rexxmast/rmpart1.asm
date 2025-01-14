* === rmpart1.asm ======================================================
*
* Copyright � 1986, 1987-1990 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* The resident process for the REXX interpreter.  Opens the "REXX" public
* message port and maintains system resources.

         XDEF     RMStart
         XDEF     CheckTimer
         XDEF     CloseConsole
         XDEF     OpenConsole
         XDEF     WaitLoop

         ; The stack frame for the RexxMaster

         STRUCTURE StackFrame,0
         STRUCT   sf_TPort,MP_SIZE     ; timer port
         UWORD    sf_MsgCount          ; messages out
         STRUCT   sf_TMsg,IOTV_SIZE    ; timer message
         LABEL    sf_SIZEOF

* RexxMaster internal state flags
CLOSING  EQU      15                   ; closing down
TIMEROUT EQU      16                   ; message still out
QUIET    EQU      17                   ; inhibit timer
CLOSETC  EQU      18                   ; close tracing console
ALLDONE  EQU      31                   ; ready to exit

* Register assignments:
*  D4 -- console filehandle
*  D5 -- DOS Base
*  D6 -- state flags
*  D7 -- wait mask
*  A4 -- task ID
*  A5 -- REXX base
*  A6 -- EXEC base
*  A7 -- stack pointer

STACKBF  SET      sf_SIZEOF
RMStart  movea.l  _AbsExecBase,a6      ; Exec library base
         lea      -STACKBF(sp),sp      ; stack frame
         movea.l  ThisTask(a6),a4      ; RexxMaster task
         movea.l  sp,a3

         moveq    #0,d4                ; clear console
         moveq    #0,d7                ; clear mask

         ; Clear the activation structure.

         movea.l  a3,a0
         moveq    #STACKBF/2-1,d1      ; loop count
1$:      clr.w    (a0)+
         dbf      d1,1$                ; loop back

         ; Wait for our startup message

         lea      pr_MsgPort(a4),a0    ; our processid
         CALLSYS  WaitPort
         lea      pr_MsgPort(a4),a0    ; our processid
         CALLSYS  GetMsg               ; D0=Message
         move.l   d0,d3                ; save message

         ; Open the DOS library (guaranteed)

         lea      DOSLib(pc),a1        ; library name
         moveq    #0,d0                ; any version
         CALLSYS  OpenLibrary
         move.l   d0,d5                ; "guaranteed"

         ; Open the REXX systems library (guaranteed)

         lea      RXSLib(pc),a1        ; library name
         moveq    #0,d0                ; any version
         CALLSYS  OpenLibrary
         move.l   d0,a5                ; "guaranteed"

         CALLSYS  Forbid               ; no switching

         ; Open the timer device, using "UNIT_VBLANK" mode.

         lea      TName(pc),a0         ; timer device name
         lea      sf_TMsg(a3),a1       ; timer message packet
         moveq    #1,d0                ; 'UNIT_VBLANK'
         moveq    #0,d1
         CALLSYS  OpenDevice           ; D0=error code
         move.l   d0,d6                ; any errors?
         bne      Error1               ; yes

         move.l   a3,(sf_TMsg+MN_REPLYPORT)(a3)
         move.w   #TR_ADDREQUEST,(sf_TMsg+IO_COMMAND)(a3)

         ; Make sure the REXX public port doesn't already exist ...

         movea.l  rl_REXX(a5),a2       ; REXX name (string structure)
         addq.l   #ns_Buff,a2          ; name string
         movea.l  a2,a1                ; port name
         CALLSYS  FindPort             ; D0=port
         tst.l    d0                   ; port found?
         bne      Error2               ; yes -- error

         ; Initialize the REXX public message port.

         lea      rl_RexxPort(a5),a0   ; REXX message port
         movea.l  a2,a1                ; port name
         exg      a5,a6
         CALLSYS  InitPort             ; D0=signal A1=port
         exg      a5,a6
         bset     d0,d7                ; set signal mask
         CALLSYS  AddPort              ; make it public
         clr.b    rl_Flags(a5)         ; clear flags

         ; Initialize and open the timer (ASYNC) port.

         movea.l  a3,a0                ; timer port
         lea      ASYNName(pc),a1      ; name
         exg      a5,a6                ; EXEC=>A5 , REXX=>A6
         CALLSYS  InitPort             ; D0=signal A1=port
         exg      a5,a6
         bset     d0,d7                ; signal mask
         CALLSYS  AddPort              ; make it public

         ; Build a parameter block to add REXX as a function host ...

         moveq    #RMHOSTP,d0          ; load priority
         clr.l       -(sp)             ; push version
         clr.l       -(sp)             ; push offset
         move.l   d0,-(sp)             ; push priority
         move.l   a2,-(sp)             ; push name

         ; Allocate and link the function node.

         movea.l  sp,a0                ; parameter block
         movea.l  a2,a1                ; node name
         moveq    #RRT_HOST,d0         ; node type
         bsr      AddLib               ; ... add the node
         lea      4*4(sp),sp           ; restore stack

         ; Reply the startup message and restore multitasking ...

         moveq    #0,d0                ; all OK
         bsr      Reply                ; reply startup
         CALLSYS  Permit               ; restore multitasking

         ; Send off a message to start the timer interval.

StartTimer
         bclr     #QUIET,d6            ; quiescent state?
         bne.s    WaitLoop             ; yes
         bset     #TIMEROUT,d6         ; set flag
         bne.s    WaitLoop             ; already out

         ; Set the timer interval and send it off.

         lea      sf_TMsg(a3),a1       ; timer message
         clr.l    IOTV_TIME+TV_SECS(a1)
         move.l   #TIMEINT_MICROS,IOTV_TIME+TV_MICRO(a1)
         CALLSYS  SendIO               ; start the interval

         ; Sit back and wait for messages to arrive ...

WaitLoop
         move.l   d7,d0                ; signal mask
         CALLSYS  Wait                 ; wait for an event

         ; Make a pass through the messages on the pending list.

         move.w   rl_NumMsg(a5),d2     ; pending count
         bra.s    2$                   ; jump in

1$:      subq.w   #1,rl_NumMsg(a5)     ; decrement pending count
         lea      rl_MsgList(a5),a0    ; list header
         CALLSYS  RemHead              ; D0=node
         bsr      DoMessage            ; reprocess it
2$:      dbf      d2,1$                ; loop back

         ; Check the REXX message port ...

         bra.s    4$                   ; jump in
3$:      bsr      DoMessage            ; process the message
4$:      lea      rl_RexxPort(a5),a0   ; public port
         CALLSYS  GetMsg               ; D0=message
         tst.l    d0                   ; found one?
         bne.s    3$                   ; yes

         bsr      GroomTasks           ; groom the task list

         ; Check whether the global console is to be closed.

         tst.w    rl_TraceCnt(a5)      ; console in use?
         bne.s    5$                   ; yes
         bclr     #CLOSETC,d6          ; close it?
         beq.s    5$                   ; no
         bsr      CloseConsole

         ; Check whether to set the 'closing' flag

5$:      btst     #RLFB_CLOSE,rl_Flags(a5)
         beq.s    CheckTimer

         bset     #CLOSING,d6          ; set 'closing' flag
         bne.s    CheckTimer           ; already set

         ; Forbid task switching during closing process ...

         CALLSYS  Forbid

         ; Check whether the timer message or ASYNC command has arrived ..

CheckTimer
         movea.l  a3,a0                ; (sf_TPort) timer port
         CALLSYS  GetMsg               ; DO=message
         tst.l    d0                   ; a message?
         beq.s    2$                   ; no ...

         lea      sf_TMsg(a3),a0       ; timer message
         cmpa.l   d0,a0                ; the timer?
         bne.s    1$                   ; no ...

         ; Timer message returned ...

         bclr     #TIMEROUT,d6         ; yes -- clear flag
         bsr      AgeList              ; age pending messages

         ; Check for tasks or pending messages.

         move.w   rl_NumTask(a5),d0    ; any tasks?
         or.w     rl_NumMsg(a5),d0     ; any messages?
         or.w     sf_MsgCount(a3),d0   ; ASYNC messages?
         bne.s    CheckTimer           ; yes ...

         ; Nothing pending at the moment.  The timer is inhibited when no
         ; tasks are running in order to lessen the load on the system.

         bset     #QUIET,d6            ; inhibit timer
         tst.w    d6                   ; closing down?
         bpl.s    CheckTimer           ; no
         bset     #ALLDONE,d6          ; ready to exit
         bra.s    CheckTimer

         ; An incoming ASNYC message.

1$:      bsr      rxAsync
         bra.s    CheckTimer

         ; Check whether we're still in business.

2$:      tst.l    d6                   ; done yet?
         bpl      StartTimer           ; no ... loop back

         ; Begin cleanup sequence ... delete all resources.

         exg      a5,a6                ; EXEC=>A5 , REXX=>A6
         lea      rl_LibList(a6),a0    ; Library List
         CALLSYS  RemRsrcList

         lea      rl_ClipList(a6),a0   ; Clip List
         CALLSYS  RemRsrcList

         lea      rl_PgmList(a6),a0    ; Program list
         CALLSYS  RemRsrcList
         exg      a5,a6                ; REXX=>A5 , EXEC=>A6

         ; Withdraw the public ports in preparation for closing.  No more
         ; changes can be made to the library base ater the port has closed.

         lea      rl_RexxPort(a5),a1   ; the public port
         CALLSYS  RemPort              ; unlink it ...

         movea.l  a3,a1                ; ASYNC port
         CALLSYS  RemPort

         ; Clean up the port resources ...

         exg      a5,a6
         lea      rl_RexxPort(a6),a0   ; REXX message port
         CALLSYS  FreePort             ; release it

         movea.l  a3,a0                ; timer port
         CALLSYS  FreePort             ; release it
         exg      a5,a6

         ; Close the console, now that the ports are down.

         bsr      CloseConsole         ; close the console
         bra.s    CloseTimer

         ; Errors

Error1   moveq    #1,d0                ; couldn't open timer
         bsr.s    Reply
         bra.s    CloseREXX

Error2   moveq    #2,d0                ; REXX port already open
         bsr.s    Reply

         ; Close the timer device.

CloseTimer
         lea      sf_TMsg(a3),a1       ; timer message
         CALLSYS  CloseDevice          ; close the timer

         ; Close the REXX system library

CloseREXX
         IFND     DEBUG
         exg      a5,a6
         move.l   pr_CLI(a4),d0        ; CLI structure
         CALLSYS  DeleteCLI            ; release it
         exg      a5,a6
         ENDC

         movea.l  a5,a1                ; REXX library base
         CALLSYS  CloseLibrary         ; close it

         ; Release our seglist ... task switching is disabled.

CloseDOS lea      STACKBF(sp),sp       ; restore stack

         lea      (RMStart-4)(pc),a1   ; our seglist
         move.l   a1,d1
         lsr.l    #2,d1                ; convert to BPTR
         exg      d5,a6
         CALLSYS  UnLoadSeg            ; unload it
         exg      d5,a6

         ; Close the DOS library.

         movea.l  d5,a1                ; DOS library base
         jmp      _LVOCloseLibrary(a6) ; no return!

* ===========================     Reply     ============================
* Replies the startup message.
* Registers:      D0 -- error code
Reply    movea.l  d3,a1                ; startup message
         move.l   d0,sm_Error(a1)      ; install error
         jmp      _LVOReplyMsg(a6)

* =========================     CloseConsole     =======================
* Close the global console ...
CloseConsole
         move.l   d4,d1                ; console or 0
         exg      d5,a6
         CALLSYS  Close
         exg      d5,a6
         moveq    #0,d4                ; clear value
         rts

* =========================     OpenConsole     ========================
* Open the global console (for copyright notice or tracing) ...
* Registers:      A0 -- console name
* Return:         D0 -- console filehandle
OpenConsole
         movem.l  d2/a6,-(sp)
         movea.l  d5,a6

         move.l   a0,d1                ; console definition
         move.l   #MODE_OLDFILE,d2     ; mode
         CALLSYS  Open                 ; D0=filehandle
         move.l   d0,d4                ; internal value

         movem.l  (sp)+,d2/a6
         rts

         ; String constants

DOSLib   dc.b     'dos.library',0      ; DOS library 
RXSLib   RXSLIBNAME                    ; REXX Systems Library
TName    TIMERNAME                     ; timer device name
ASYNName ASYNNAME                      ; async port name

CONDef   dc.b     'CON:160/0/320/80/ARexx/a/inactive',0
GlCName  dc.b     'GLOBALCON',0
         CNOP     0,2
