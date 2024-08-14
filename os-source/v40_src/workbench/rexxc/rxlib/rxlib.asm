* === rxlib.asm ========================================================
*
* Copyright © 1986,1987,1988,1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* Usage: rxlib [? | name [priority] [offset version]]
* Command to add or remove a library or function host to the Libraries List.

         INCLUDE  "rexx/storage.i"
         INCLUDE  "rexx/rxslib.i"
         INCLUDE  "rexx/errors.i"
         INCLUDE  "rexx/rexxio.i"

         INCLUDE  "libraries/dos.i"
         INCLUDE  "libraries/dosextens.i"

         IFND     EXEC_EXECBASE_I
ThisTask EQU      276
         ENDC

         ; EXEC library routines

         XLIB     CloseLibrary
         XLIB     FindPort
         XLIB     Forbid
         XLIB     GetMsg
         XLIB     OpenLibrary
         XLIB     Permit
         XLIB     PutMsg
         XLIB     WaitPort

         ; DOS library routines

         XLIB     Write

_AbsExecBase EQU  4

MAXNAME  EQU      32

         ; Command string in A0, length in D0

STACKBF  EQU      80
start:   movea.l  _AbsExecBase,a6
         lea      -STACKBF(sp),sp
         movea.l  ThisTask(a6),a4      ; our task ID
         movea.l  a0,a3                ; command line

         move.l   d0,d3                ; command length
         moveq    #0,d6                ; default return code
         moveq    #pr_MsgPort,d7       ; processid offset
         add.l    a4,d7                ; our processid

         ; Exclude task switching while we check for the port.

         CALLSYS  Forbid

         ; Open the DOS library

         lea      DOSLib(pc),a1
         moveq    #0,d0                ; any version
         CALLSYS  OpenLibrary          ; D0=library
         move.l   d0,d5                ; assume success

         ; Look for the REXX server's message port.

         lea      RexxName(pc),a1      ; REXX port name
         CALLSYS  FindPort             ; D0=port
         move.l   d0,d4                ; port found?
         beq      Error1               ; no

         ; Open the ARexx Systems Library

         lea      RXSLib(pc),a1        ; REXX library
         moveq    #0,d0                ; any version
         CALLSYS  OpenLibrary
         tst.l    d0                   ; opened ok?
         beq      Error2               ; no
         move.l   d0,a5

         ; Trim trailing blanks or control characters.

         lea      0(a3,d3.l),a1        ; end of string
         subq.w   #1,d3                ; loop count

1$:      clr.b    (a1)                 ; install a null
         cmpi.b   #' ',-(a1)           ; > blank?
         dbhi     d3,1$                ; loop back

         ; Check for special cases

         addq.w   #1,d3                ; restore length
         beq      ShowList             ; ... display names
         cmpi.b   #'?',(a3)            ; a question mark?
         beq      ShowFmt              ; ... display format

         ; Allocate a message packet 

         movea.l  d7,a0                ; replyport (our processid)
         suba.l   a1,a1                ; no extension
         moveq    #0,d0                ; no host
         exg      a5,a6
         CALLSYS  CreateRexxMsg        ; D0=A0=packet
         exg      a5,a6
         beq      Error3               ; failure ...
         movea.l  d0,a2                ; save packet
         move.l   #RXREMLIB,d2         ; default action code

         ; Parse the command string into name and value tokens.

ParseCommand
         move.l   a3,ARG0(a2)          ; name pointer
         movea.l  a3,a0                ; scan pointer
         exg      a5,a6                ; EXEC=>A5, REXX=>A6
         CALLSYS  StcToken             ; D1=length  A1=token
         clr.b    0(a1,d1.l)           ; install a null

         ; Extract the priority token.

         CALLSYS  StcToken             ; D1=length A1=token
         exg      a5,a6                ; REXX=>A5, EXEC=>A6
         clr.b    0(a1,d1.l)           ; install a null

         ; Check whether a token was found.  If not, delete the entry.

         tst.l    d1                   ; a token?
         beq.s    2$                   ; no
         move.l   #RXADDFH,d2          ; action code

         movea.l  a0,a3                ; save scan
         movea.l  a1,a0
         exg      a5,a6                ; EXEC=>A5, REXX=>A6
         CALLSYS  CVa2i                ; D0=value
         move.l   d0,ARG1(a2)          ; install priority

         ; Extract the offset token ...

         movea.l  a3,a0                ; scan pointer
         CALLSYS  StcToken             ; D1=length  A1=token
         clr.b    0(a1,d1.l)           ; install a null

         ; Check whether the offset token was supplied.

         tst.l    d1                   ; offset token?
         beq.s    1$                   ; no
         move.l   #RXADDLIB,d2         ; action code

         ; Convert to an integer.

1$:      movea.l  a0,a3                ; save scan
         movea.l  a1,a0                ; token pointer
         CALLSYS  CVa2i                ; D0=value
         move.l   d0,ARG2(a2)          ; install offset

         ; Extract the version token and convert it.

         movea.l  a3,a0                ; scan pointer
         CALLSYS  StcToken             ; D1=length A1=token
         clr.b    0(a1,d1.l)           ; install a null
         movea.l  a1,a0                ; version token
         CALLSYS  CVa2i                ; D0=value
         exg      a5,a6                ; REXX=>A5, EXEC=>A6
         move.l   d0,(ARG0+12)(a2)     ; install version

         ; Send the message packet to the REXX server.

2$:      move.l   d2,ACTION(a2)        ; action code
         move.l   #4,RESULT1(a2)       ; timeout count
         movea.l  d4,a0                ; REXX port
         movea.l  a2,a1                ; message packet
         CALLSYS  PutMsg               ; send it

         ; Wait for the packet at our processid.

         movea.l  d7,a0                ; our processid
         CALLSYS  WaitPort
         movea.l  d7,a0                ; our processid
         CALLSYS  GetMsg
         move.l   RESULT1(a2),d6       ; return code
         bra      CleanUp2

         ; Show the libraries currently available.

ShowList exg      a5,a6
         moveq    #0,d0                ; all resources
         CALLSYS  LockRexxBase         ; lock structure
         move.l   rl_LibList(a6),d2    ; first node

1$:      movea.l  d2,a2                ; current node
         move.l   (a2),d2              ; successor?
         beq.s    6$                   ; no

         moveq    #MAXNAME-1,d0        ; max name length
         movea.l  LN_NAME(a2),a0       ; name string
         movea.l  sp,a1                ; stack buffer

2$:      move.b   (a0)+,(a1)+          ; copy the string
         dbeq     d0,2$                ; loop back
         beq.s    4$                   ; all copied

         ; Name too long ... insert an ellipsis.

         lea      Ellipsis(pc),a0
3$:      move.b   (a0)+,(a1)+          ; copy
         bne.s    3$                   ; loop back

4$:      move.b   #' ',-1(a1)
         lea      LibType(pc),a0
         cmpi.b   #RRT_LIB,LN_TYPE(a2) ; library?
         beq.s    5$                   ; yes
         lea      HostType(pc),a0

5$:      move.b   (a0)+,(a1)+          ; copy
         bne.s    5$                   ; loop back
         move.b   #$0A,-1(a1)          ; a newline
         clr.b    (a1)

         ; Display the string.

         movea.l  sp,a0                ; buffer
         bsr      WriteMsg             ; display it
         bra.s    1$                   ; loop back

6$:      moveq    #0,d0                ; all resources
         CALLSYS  UnlockRexxBase       ; unlock structure
         exg      a5,a6
         bra.s    CleanUp1

         ; Display the command format ...

ShowFmt  lea      FmtMsg(pc),a0        ; command format
         bsr.s    WriteMsg
         bra.s    CleanUp1

         ; Error messages

Error1:  lea      ErrMsg1(pc),a0       ; REXX port not found
         bsr.s    WriteMsg
         moveq    #RC_ERROR,d6
         bra.s    CloseDOS

Error2:  lea      ErrMsg2(pc),a0       ; couldn't open library
         bsr.s    WriteMsg
         moveq    #RC_ERROR,d6
         bra.s    CloseDOS

Error3:  lea      ErrMsg3(pc),a0       ; allocation failure
         bsr.s    WriteMsg
         moveq    #RC_ERROR,d6
         bra.s    CleanUp2

Error4:  lea      ErrMsg4(pc),a0       ; invalid integer
         bsr.s    WriteMsg
         moveq    #RC_ERROR,d6

         ; Recycle the message packet.

CleanUp2 movea.l  a2,a0                ; message packet
         exg      a5,a6
         CALLSYS  DeleteRexxMsg
         exg      a5,a6

         ; Close the REXX library

CleanUp1 movea.l  a5,a1
         CALLSYS  CloseLibrary

         ; Close the DOS library

CloseDOS movea.l  d5,a1                ; DOS library
         CALLSYS  CloseLibrary

         ; Reenable task switching

         CALLSYS  Permit
         move.l   d6,d0                ; return code
         lea      STACKBF(sp),sp
         rts

* ==========================      WriteMsg     =========================
* Writes a message to the output stream.
* Registers:   A0 -- message
*              D0 -- length
WriteMsg:
         movem.l  d2/d3/a6,-(sp)
         movea.l  d5,a6                ; DOS base
         move.l   pr_COS(a4),d1        ; a stream?
         beq.s    2$                   ; no

         move.l   a0,d2                ; message
1$:      tst.b    (a0)+
         bne.s    1$
         subq.w   #1,a0                ; back up
         move.l   a0,d3                ; end of string
         sub.l    d2,d3                ; length
         CALLSYS  Write

2$:      movem.l  (sp)+,d2/d3/a6
         rts

         ; String data

DOSLib   dc.b     'dos.library',0
RXSLib   RXSNAME
RexxName dc.b     'REXX',0
HostType dc.b     '(host)',0
LibType  dc.b     '(library)',0
Ellipsis dc.b     '...',0

FmtMsg   dc.b     'Usage: RXLIB NAME PRIORITY [OFFSET] [VERSION]',10,0
ErrMsg1  dc.b     'ARexx server not active',10,0
ErrMsg2  dc.b     'Can',$27,'t open rexxsyslib.library',10,0
ErrMsg3  dc.b     'Memory allocation failure',10,0
ErrMsg4  dc.b     'Invalid integer value',10,0
         CNOP     0,2

         END
