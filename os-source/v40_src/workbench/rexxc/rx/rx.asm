* === rx.asm ===========================================================
*
* Copyright © 1986,1987,1988,1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* Launches REXX programs from Workbench or the CLI.
* CLI Usage: rx command-line
* Workbench: CONSOLE=window-spec COMMAND=command-line STARTUP=command-line

         INCLUDE  "rexx/storage.i"
         INCLUDE  "rexx/rxslib.i"
         INCLUDE  "rexx/errors.i"

         INCLUDE  "libraries/dos.i"
         INCLUDE  "libraries/dosextens.i"

_AbsExecBase EQU  4

         IFND     EXEC_EXECBASE_I
ThisTask EQU      276
         ENDC

         IFND     INTUITION_INTUITION_I
gg_SIZEOF EQU     44
         ENDC

         IFND     WORKBENCH_STARTUP_I
         STRUCTURE WBStartup,MN_SIZE
         APTR     sm_Process
         LONG     sm_Segment
         LONG     sm_NumArgs
         APTR     sm_ToolWindow
         APTR     sm_ArgList
         LABEL    sm_SIZEOF

         STRUCTURE WBArg,0
         LONG     wa_Lock
         APTR     wa_Name
         LABEL    wa_SIZEOF
         ENDC

         STRUCTURE DiskObject,0
         UWORD    do_Magic
         UWORD    do_Version
         STRUCT   do_Gadget,gg_SIZEOF
         UWORD    do_Type
         APTR     do_DefaultTool
         APTR     do_ToolTypes
         LONG     do_CurrentX
         LONG     do_CurrentY
         APTR     do_DrawerData
         APTR     do_ToolWindow
         LONG     do_StackSize
         LABEL    do_SIZEOF

         ; DOS library functions

         XLIB     Close
         XLIB     Delay
         XLIB     Execute
         XLIB     Open
         XLIB     Write

         ; Exec library functions

         XLIB     CloseLibrary
         XLIB     FindPort
         XLIB     Forbid
         XLIB     GetMsg
         XLIB     OpenLibrary
         XLIB     Permit
         XLIB     PutMsg
         XLIB     ReplyMsg
         XLIB     WaitPort

         ; Icon library

         XLIB     FindToolType
         XLIB     FreeDiskObject
         XLIB     GetDiskObject

         ; Our parameter block

         STRUCTURE ParmBlock,0
         APTR     Console              ; console definition
         APTR     CommLine             ; command line
         APTR     StartUp              ; startup command
         STRUCT   ToolBuff,244         ; tooltypes buffer
         LABEL    pb_SIZEOF            ; size: 256 bytes

TOOLBUFF EQU      pb_SIZEOF-ToolBuff   ; buffer size
DELAY    EQU      125                  ; delay time

STACKBF  SET      pb_SIZEOF
start:   movea.l  _AbsExecBase,a6      ; EXEC library base
         lea      -STACKBF(sp),sp      ; stack buffer
         movea.l  ThisTask(a6),a4      ; our task ID
         movea.l  a0,a3                ; save command

         move.l   d0,d3                ; save length
         moveq    #0,d6                ; clear return
         moveq    #0,d7                ; clear message

         ; Initialize the parameter slots.

         movea.l  sp,a0                ; parameter block
         lea      ConDef(pc),a1        ; default console
         move.l   a1,(a0)+             ; ... install it
         clr.l    (a0)+                ; CommLine
         lea      LoadRexx(pc),a1      ; default start-up command
         move.l   a1,(a0)              ; ... install it

         ; Check whether we were started from WB or the CLI ...

         tst.l    pr_CLI(a4)           ; from a CLI?
         bne.s    1$                   ; yes ...

         ; Wait for our startup message from Workbench.

         lea      pr_MsgPort(a4),a0    ; our processid
         CALLSYS  WaitPort
         lea      pr_MsgPort(a4),a0    ; our processid
         CALLSYS  GetMsg               ; D0=message
         move.l   d0,d7                ; save it

         ; Open the DOS library.

1$:      lea      DOSLib(pc),a1        ; library name
         moveq    #0,d0                ; any version
         CALLSYS  OpenLibrary          ; D0=library
         move.l   d0,d5                ; "guaranteed"

         ; Turn off task switching ...

         CALLSYS  Forbid               ; no switching

         ; WorkBench invocation?  Load arguments into the parameter block.

         move.l   d7,d0                ; WB message?
         beq.s    GetCommandLine       ; no
         movea.l  sp,a0                ; parameter block
         bsr      GetWBArgs            ; get arguments

         ; Open the console window and install it in the process structure.

         move.l   (sp),d1              ; console definition
         move.l   #MODE_NEWFILE,d2     ; open mode
         exg      d5,a6
         CALLSYS  Open                 ; D0=filehandle
         exg      d5,a6
         move.l   d0,pr_COS(a4)        ; output stream
         beq.s    2$                   ; ... failure
         move.l   d0,pr_CIS(a4)        ; input stream

         ; Initialize the current console handler slot.

         lsl.l    #2,d0                ; real address
         movea.l  d0,a0
         move.l   fh_Type(a0),pr_ConsoleTask(a4)

         ; Install the command line.

2$:      movea.l  CommLine(sp),a3      ; command line
         bra.s    FindREXXPort

         ; CLI invocation ... trim trailing "white space".

GetCommandLine
         movea.l  a3,a1                ; command line
         adda.w   d3,a1                ; end of line
         subq.w   #1,d3                ; loop count

1$:      clr.b    (a1)                 ; install null
         cmpi.b   #' ',-(a1)           ; > blank?
         dbhi     d3,1$                ; loop back
         addq.w   #1,d3                ; restore length
         beq      ShowFmt              ; no command ...

         ; Look for the REXX public message port.

FindREXXPort
         moveq    #2-1,d4              ; loop count
         bra.s    2$                   ; jump in

         ; Attempt to start the REXX server using the startup command.

1$:      move.l   StartUp(sp),d1       ; startup command?
         beq.s    2$                   ; no ...
         move.l   pr_COS(a4),d3        ; output stream?
         beq.s    2$                   ; no ... can't issue command
         exg      d5,a6
         CALLSYS  Execute              ; issue command
         exg      d5,a6

         ; Look for the REXX public port.

2$:      lea      REXXPort(pc),a1      ; REXX port name
         CALLSYS  FindPort             ; D0=port
         move.l   d0,d2                ; port found?
         dbne     d4,1$                ; loop back
         beq      Error1               ; error ... not found

         ; REXX is active ... open the REXX Systems library.

         lea      RXSLib(pc),a1        ; library name
         moveq    #0,d0                ; any version
         CALLSYS  OpenLibrary
         tst.l    d0                   ; opened?
         beq      Error2               ; no
         movea.l  d0,a5

         ; Allocate a message packet structure.

         lea      pr_MsgPort(a4),a0    ; our processid
         movea.l  d2,a1                ; REXX port
         move.l   LN_NAME(a1),d0       ; Host Address
         lea      FileExt(pc),a1       ; file extension
         exg      a5,a6
         CALLSYS  CreateRexxMsg        ; D0=A0=packet
         exg      a5,a6
         beq      Error3               ; allocation failure ...
         movea.l  a0,a2                ; save packet

         ; Fill in the message packet ...

         move.l   #RXCOMM,ACTION(a2)   ; command code
         move.l   a3,ARG0(a2)          ; command string
         moveq    #1,d0                ; one string
         moveq    #0,d1                ; string conversion
         exg      a5,a6
         CALLSYS  FillRexxMsg          ; D0=boolean
         exg      a5,a6
         beq      Error4               ; ... failure

         ; Send off the message packet

SendMsg  movea.l  d2,a0                ; REXX public port
         movea.l  a2,a1                ; message packet
         CALLSYS  PutMsg               ; send it off

         ; Wait for the packet to return.  Task switching is reenabled.

         lea      pr_MsgPort(a4),a0    ; our processid
         CALLSYS  WaitPort
         lea      pr_MsgPort(a4),a0    ; our processid
         CALLSYS  GetMsg               ; get the message
         move.l   RESULT1(a2),d6       ; return code
         beq.s    4$                   ; ... all OK

         ; Report the error ...

         exg      a5,a6
         movea.l  sp,a0                ; stack buffer
         lea      CommErr(pc),a1       ; message

1$:      move.b   (a1)+,(a0)+          ; copy it
         bne.s    1$                   ; loop back
         subq.w   #1,a0                ; back up

         ; Convert the return code

         move.l   d6,d0                ; return code
         moveq    #12,d1               ; maximum digits
         CALLSYS  CVi2a                ; A0=scan pointer

         ; See if a secondary error was supplied

         move.l   RESULT2(a2),d2       ; secondary code?
         beq.s    3$                   ; no

         move.b   #'/',(a0)+           ; separator
         move.l   d2,d0                ; error code
         moveq    #12,d1               ; maximum digits
         CALLSYS  CVi2a                ; A0=scan pointer

         move.b   #':',(a0)+           ; separator
         move.b   #' ',(a0)+
         move.l   d2,d0                ; error code
         move.l   a0,-(sp)             ; push pointer
         CALLSYS  ErrorMsg             ; A0=error string

         ; Copy the message string to the output buffer.

         lea      ns_Buff(a0),a1       ; message string
         movea.l  (sp)+,a0             ; pop pointer
2$:      move.b   (a1)+,(a0)+          ; copy string
         bne.s    2$                   ; loop back
         subq.w   #1,a0                ; back up

         ; Terminate the display line ...

3$:      move.b   #$0A,(a0)+           ; a newline
         clr.b    (a0)+                ; a null

         ; Display the error message

         exg      a5,a6
         movea.l  sp,a0                ; message buffer
         bsr      WriteMsg             ; display it

         ; Cleanup operations

4$:      movea.l  a2,a0                ; message
         moveq    #1,d0                ; one string
         exg      a5,a6
         CALLSYS  ClearRexxMsg         ; recycle it
         exg      a5,a6
         bra.s    ReleaseMsg

         ; Display command format

ShowFmt  lea      FmtMsg(pc),a0        ; command format
         bsr.s    WriteMsg
         bra.s    CloseDOS

Error1   lea      ErrMsg1(pc),a0       ; "REXX port not found"
         bsr.s    WriteMsg
         moveq    #RETURN_WARN,d6
         bra.s    CloseDOS

Error2   lea      ErrMsg2(pc),a0       ; "can't open library"
         bsr.s    WriteMsg             ; display it
         moveq    #RETURN_ERROR,d6     ; load error
         bra.s    CloseDOS

Error3   lea      ErrMsg3(pc),a0       ; "allocation failure"
         bsr.s    WriteMsg
         moveq    #RETURN_ERROR,d6
         bra.s    CloseREXX

         ; Error ... couldn't allocate the command string

Error4   lea      ErrMsg3(pc),a0       ; "allocation failure"
         bsr.s    WriteMsg
         moveq    #RETURN_ERROR,d6

ReleaseMsg
         movea.l  a2,a0                ; message packet
         exg      a5,a6                ; EXEC=>A5 , REXX=>A6
         CALLSYS  DeleteRexxMsg        ; release it
         exg      a5,a6                ; REXX=>A5 , EXEC=>A6

         ; Close the REXX library

CloseREXX
         movea.l  a5,a1                ; REXX system library
         CALLSYS  CloseLibrary

         ; Reply the WB message and close the DOS library.

CloseDOS tst.l    d7                   ; WB message?
         beq.s    1$                   ; no

         exg      d5,a6
         moveq    #DELAY,d1            ; delay time
         CALLSYS  Delay                ; wait a bit
         move.l   pr_COS(a4),d1        ; filehandle or 0
         CALLSYS  Close                ; close it
         exg      d5,a6

         movea.l  d7,a1                ; message
         CALLSYS  ReplyMsg             ; reply it
         bra.s    2$                   ; (remain forbidden)

         ; Reenable task switching ...

1$:      CALLSYS  Permit

2$:      movea.l  d5,a1                ; DOS library
         CALLSYS  CloseLibrary

         move.l   d6,d0                ; load return code
         lea      STACKBF(sp),sp       ; restore stack
         rts

* ==========================     WriteMsg     ==========================
* Displays an error message on the standard output stream.
* Registers:   A0 -- string
WriteMsg
         movem.l  d2/d3/a6,-(sp)
         movea.l  d5,a6                ; DOS base
         move.l   pr_COS(a4),d0        ; output stream?
         beq.s    2$                   ; no

         move.l   d0,d1                ; filehandle
         move.l   a0,d2                ; message
1$:      tst.b    (a0)+                ; a null?
         bne.s    1$                   ; no
         subq.w   #1,a0                ; back up
         move.l   a0,d3                ; end of buffer
         sub.l    d2,d3                ; length
         CALLSYS  Write

2$:      movem.l  (sp)+,d2/d3/a6
         rts

* ==========================     GetWBArgs     =========================
* Gets WB arguments from one or more icon files and installs them in a
* parameter block.
* Registers:   A0 -- parameter block
*              D0 -- message
GetWBArgs
         movem.l  d2-d7/a2/a3/a5,-(sp)
         movea.l  a0,a5                ; save parmblock
         movea.l  d0,a2                ; save message

         ; Open the Icon Library

         lea      IconLib(pc),a1       ; library name
         moveq    #0,d0                ; any version
         CALLSYS  OpenLibrary          ; D0=library base
         move.l   d0,d5                ; success?
         beq.s    GWExit               ; no
         exg      d5,a6                ; EXEC=>D5, Icon=>A6

         ; Get the tool or project parameters 

         moveq    #0,d3                ; buffer length
         move.l   sm_NumArgs(a2),d6    ; argument count
         moveq    #0,d7                ; clear index
         bra.s    5$                   ; jump in

1$:      movea.l  sm_ArgList(a2),a0    ; argument list
         move.l   wa_Lock(a0,d7.w),pr_CurrentDir(a4)
         movea.l  wa_Name(a0,d7.w),a0  ; tool or project name
         move.l   a0,CommLine(a5)      ; save it ...
         addq.w   #wa_SIZEOF,d7        ; advance index
         CALLSYS  GetDiskObject        ; D0=disk object
         tst.l    d0                   ; success?
         beq.s    5$                   ; no
         movea.l  d0,a3                ; save object

         ; Look for the ToolTypes and copy them into the buffer ...

         moveq    #Console-Console,d2  ; parameter index
         moveq    #3-1,d4              ; loop count

2$:      movea.l  do_ToolTypes(a3),a0  ; ToolTypes array
         moveq    #0,d0                ; clear offset
         move.b   Offsets(pc,d4.w),d0  ; offset to name
         lea      Offsets(pc,d0.w),a1  ; ToolTypes name
         CALLSYS  FindToolType         ; D0=value
         tst.l    d0                   ; string found?
         beq.s    4$                   ; no

         ; Copy the tooltype string to the buffer area.

         movea.l  d0,a0                ; tooltype string
         lea      ToolBuff(a5,d3.w),a1 ; buffer area
         move.l   a1,Console(a5,d2.w)  ; install pointer

3$:      clr.b    (a1)                 ; install a null
         cmpi.w   #TOOLBUFF-1,d3       ; at end?
         bcc.s    4$                   ; yes
         addq.w   #1,d3                ; increment length
         move.b   (a0)+,(a1)+          ; copy string
         bne.s    3$

4$:      addq.w   #4,d2                ; index
         dbf      d4,2$                ; loop back

         ; Release the DiskObject structure.

         movea.l  a3,a0                ; disk object
         CALLSYS  FreeDiskObject       ; release it

5$:      dbf      d6,1$                ; loop back

         ; Close the Icon library

         exg      d5,a6
         movea.l  d5,a1                ; Icon base
         CALLSYS  CloseLibrary

GWExit   movem.l  (sp)+,d2-d7/a2/a3/a5
         rts

         ; Offsets to ToolType names

Offsets  dc.b     STARTUP-Offsets      ; last entry
         dc.b     COMMAND-Offsets
         dc.b     CONSOLE-Offsets      ; first entry

         ; ToolType names

CONSOLE  dc.b     'CONSOLE',0
COMMAND  dc.b     'CMD',0
STARTUP  dc.b     'STARTUP',0

         ; String data

DOSLib   dc.b     'dos.library',0
IconLib  dc.b     'icon.library',0
RXSLib   RXSNAME                       ; library name
REXXPort dc.b     'REXX',0             ; REXX port name
FileExt  EQU      REXXPort             ; file extension
LoadRexx dc.b     'rexxmast',0
ConDef   dc.b     'CON://640/100/Rx Output',0

FmtMsg   dc.b     'Usage:  rx filename [arguments]',10,0
ErrMsg1  dc.b     'ARexx server not active',10,0
ErrMsg2  dc.b     'Can',$27,'t open rexxsyslib.library',10,0
ErrMsg3  dc.b     'Allocation failure',10,0

CommErr  dc.b     'Command returned ',0
         CNOP     0,2

         END
