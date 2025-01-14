* === inimast.asm ======================================================
*
* Copyright � 1986, 1987-1990 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* Initialization routine for the REXX resident process.

         IDNT     RexxMaster
         SECTION  Init,CODE

*DEBUG    SET      1

         INCLUDE  "rexx/storage.i"
         INCLUDE  "rexx/rxslib.i"
         INCLUDE  "rexx/rexxio.i"
         INCLUDE  "rexx/errors.i"

         INCLUDE  "devices/timer.i"

         INCLUDE  "dos/dos.i"
         INCLUDE  "dos/dosextens.i"

         XREF     _AbsExecBase

         ; DOS library entry points

         XREF     _LVOClose
         XREF     _LVOCreateProc
         XREF     _LVODelay
         XREF     _LVODupLock
         XREF     _LVOOpen
         XREF     _LVOUnLoadSeg
         XREF     _LVOUnLock
         XREF     _LVOWrite

         ; EXEC library entry points

         XREF     _LVOAddPort
         XREF     _LVOAddTail
         XREF     _LVOAllocMem
         XREF     _LVOCloseDevice
         XREF     _LVOCloseLibrary
         XREF     _LVOEnqueue
         XREF     _LVOFindName
         XREF     _LVOFindPort
         XREF     _LVOForbid
         XREF     _LVOFreeMem
         XREF     _LVOFreeSignal
         XREF     _LVOGetMsg
         XREF     _LVOOpenDevice
         XREF     _LVOOpenLibrary
         XREF     _LVOPermit
         XREF     _LVOPutMsg
         XREF     _LVORemHead
         XREF     _LVORemove
         XREF     _LVORemPort
         XREF     _LVOReplyMsg
         XREF     _LVOSendIO
         XREF     _LVOSetTaskPri
         XREF     _LVOSignal
         XREF     _LVOWait
         XREF     _LVOWaitPort


         IFND     EXEC_EXECBASE_I
ThisTask EQU      276
         ENDC

CALLSYS  MACRO    * FunctionName
         jsr      _LVO\1(a6)
         ENDM

RMNAME   MACRO                         ; name for RexxMaster
         dc.b     'RexxMaster',0
         ENDM

ASYNNAME MACRO                         ; asynch port name
         dc.b     'AREXX',0
         ENDM

RMPRI    EQU      4                    ; RexxMaster task priority
RMSTACK  EQU      2048                 ; stack size for master

* Miscellaneous constants
RMHOSTP  EQU      -60                  ; REXX function host priority
RMWARN   EQU      -1                   ; warning error code
MAXLEN   EQU      65535                ; maximum string length
TIMEINT_MICROS     EQU 500000          ; timer interval (microseconds)
RM_MINIMUM_VERSION EQU 33              ; minimum REXX library version

         ; A startup message for the RexxMaster

         STRUCTURE StartMessage,MN_SIZE
         LONG     sm_Error             ; error code
         LABEL    sm_SIZEOF            ; size: 24 bytes

         ; The initial entry point for launching from either the CLI or WB.

STACKBF  SET      sm_SIZEOF
InitSeg  movea.l  _AbsExecBase,a6      ; Exec library base
         lea      -STACKBF(sp),sp      ; stack frame
         movea.l  ThisTask(a6),a4      ; our task ID

         moveq    #0,d6                ; WB message
         moveq    #pr_MsgPort,d7       ; offset to processid
         add.l    a4,d7                ; processid

         move.l   d7,MN_REPLYPORT(sp)  ; install replyport

         ; See whether we were started by a CLI or by WorkBench

         tst.l    pr_CLI(a4)           ; a CLI?
         bne.s    1$                   ; yes

         ; Get the WB startup message

         movea.l  d7,a0                ; our processid
         CALLSYS  WaitPort
         movea.l  d7,a0                ; our processid
         CALLSYS  GetMsg               ; D0=message
         move.l   d0,d6                ; save message

         ; Attempt to inherit WB's CLI structure.

         movea.l  d0,a0
         move.l   MN_REPLYPORT(a0),d1  ; a replyport?
         beq.s    1$                   ; no
         movea.l  d1,a1
         tst.b    MP_FLAGS(a1)         ; PA_SIGNAL?
         bne.s    1$                   ; no
         move.l   MP_SIGTASK(a1),a1    ; WB task
         move.l   pr_CLI(a1),pr_CLI(a4); inherit CLI structure

         ; Open the DOS library

1$:      lea      IniDOS(pc),a1        ; library name
         moveq    #0,d0                ; any version
         CALLSYS  OpenLibrary
         move.l   d0,d5                ; save it

         ; Open a display console if we're running from WorkBench ...

         tst.l    d6                   ; WB invocation?
         beq.s    2$                   ; no
         lea      IniCON(pc),a1        ; console definition
         move.l   a1,d1                ; filename
         move.l   #MODE_OLDFILE,d2     ; mode
         exg      d5,a6
         CALLSYS  Open                 ; D0=filehandle
         exg      d5,a6
         move.l   d0,pr_COS(a4)        ; install it

         ; Make sure the IEEE library can be opened.

2$:      lea      IniIEEE(pc),a1       ; library name
         moveq    #0,d0                ; any version
         CALLSYS  OpenLibrary          ; D0=base
         lea      IniMsg1(pc),a0       ; "couldn't open library"
         tst.l    d0                   ; success?
         beq      3$                   ; no
         movea.l  d0,a1                ; library base
         CALLSYS  CloseLibrary         ; close it

         ; Make sure the REXX library can be opened.

         lea      IniREXX(pc),a1       ; library name
         moveq    #RM_MINIMUM_VERSION,d0
         CALLSYS  OpenLibrary          ; D0=base
         lea      IniMsg2(pc),a0       ; "couldn't open library"
         tst.l    d0                   ; success?
         beq      3$                   ; no
         movea.l  d0,a5

         ; Post the copyright notice ...

         movea.l  rl_Notice(a5),a0     ; copyright notice text
         bsr      WriteMsg             ; display it

         ; Get the second seglist node and launch the new process ...

         lea      (InitSeg-4)(pc),a3   ; BPTR to next node

         lea      IniName(pc),a1       ; task name
         move.l   a1,d1
         moveq    #RMPRI,d2            ; priority
         move.l   (a3),d3              ; segment
         move.l   #RMSTACK,d4          ; required stack

         IFD      DEBUG
         clr.l    (a3)                 ; ** debug **
         move.l   sp,a1                ; message
         clr.l    MN_REPLYPORT(a1)
         movea.l  d7,a0                ; our processid
         CALLSYS  PutMsg
         movem.l  d2-d7/a2-a6,-(sp)    ; save all regs
         jsr      RMStart              ; call directly
         movem.l  (sp)+,d2-d7/a2-a6    ; restore regs
         bra.s    IRMCloseREXX         ; ** debug **
         ENDC

         ; Create the rexxmaster process.

         exg      d5,a6
         CALLSYS  CreateProc           ; D0=processid
         exg      d5,a6
         lea      IniMsg3(pc),a0       ; "couldn't create process"
         tst.l    d0                   ; success?
         beq.s    4$                   ; no
         movea.l  d0,a2                ; save processid
         clr.l    (a3)                 ; split the seglist

         ; Create a CLI structure for the RexxMaster process ...

         move.l   pr_CLI(a4),d0        ; parent CLI
         exg      a5,a6
         CALLSYS  CreateCLI            ; D0=BPTR to CLI
         exg      a5,a6
         move.l   d0,(pr_CLI-pr_MsgPort)(a2)
         clr.l    (pr_ConsoleTask-pr_MsgPort)(a2)

         ; Send the startup message ...

         move.l   sp,a1                ; message
         movea.l  a2,a0                ; processid
         CALLSYS  PutMsg

         ; ... and wait for the reply.

         movea.l  d7,a0                ; our processid
         CALLSYS  WaitPort
         movea.l  d7,a0
         CALLSYS  GetMsg               ; D0=message

         ; Check for an error return ...

         move.l   sm_Error(sp),d2      ; load error
         beq.s    IRMCloseREXX         ; ... all OK

         ; Decode the error return.

         lea      IniMsg4(pc),a0       ; "couldn't open timer"
         subq.l   #1,d2                ; error?
         beq.s    4$                   ; yes
         lea      IniMsg5(pc),a0       ; "REXX port already open"
         bra.s    4$

         ; Error messages

3$:      bsr.s    WriteMsg             ; display message
         moveq    #RETURN_FAIL,d2      ; load error
         bra.s    IRMCloseDOS

4$:      bsr.s    WriteMsg
         moveq    #RETURN_FAIL,d2

         ; Close the REXX library.

IRMCloseREXX
         movea.l  a5,a1                ; REXX base
         CALLSYS  CloseLibrary         ; close it

         ; Close the output stream, if one was opened.

IRMCloseDOS
         tst.l    d6                   ; a WB message?
         beq.s    1$                   ; no ...

         exg      d5,a6
         moveq    #125,d1              ; wait a bit
         CALLSYS  Delay
         move.l   pr_COS(a4),d1        ; output stream
         CALLSYS  Close                ; close it
         exg      d5,a6

         ; Close the DOS library.

1$:      movea.l  d5,a1
         CALLSYS  CloseLibrary

         lea      STACKBF(sp),sp       ; restore stack

         ; Return the WorkBench message, if one was sent.

         tst.l    d6                   ; a WB message?
         bne.s    2$                   ; yes
         move.l   d2,d0                ; return code
         rts

2$:      movea.l  d6,a1                ; startup message
         jmp      _LVOReplyMsg(a6)     ; no return!

* Writes a message to STDOUT.
WriteMsg
         movem.l  d2/d3/a6,-(sp)
         movea.l  d5,a6
         move.l   pr_COS(a4),d1        ; a stream?
         beq.s    2$                   ; no

         move.l   a0,d2
1$:      tst.b    (a0)+                ; null?
         bne.s    1$
         subq.l   #1,a0
         move.l   a0,d3                ; end of string
         sub.l    d2,d3                ; length
         CALLSYS  Write

2$:      movem.l  (sp)+,d2/d3/a6
         rts

         ; String constants

IniDOS   dc.b     'dos.library',0
IniIEEE  dc.b     'mathieeedoubbas.library',0
IniREXX  dc.b     'rexxsyslib.library',0
IniName  dc.b     'RexxMaster',0
IniCON   dc.b     'CON:160/60/320/80/Starting REXX .../ads/inactive',0

	include 'rexxmast_rev.i'
Version  VERSTAG
;Version  dc.b     '$VER: rexxmast 36.4 (04-APR-91)',0

IniMsg1  dc.b     'Can',$27,'t open mathieeedoubbas.library',10,0
IniMsg2  dc.b     'Can',$27,'t open rexxsyslib.library',10,0
IniMsg3  dc.b     'Failed to create process',10,0
IniMsg4  dc.b     'Can',$27,'t open timer.device',10,0
IniMsg5  dc.b     'REXX server already active',10,0
         CNOP     0,2

* The next section remains resident with the created process.  It is the
* second hunk of the segment list.

         SECTION  RexxMast,CODE

         INCLUDE  "rmpart1.asm"
         INCLUDE  "rmpart2.asm"

         END
