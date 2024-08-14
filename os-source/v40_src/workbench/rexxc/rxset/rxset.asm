* === rxset.asm ========================================================
*
* Copyright © 1986, 1987-1990 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* Posts a string to the RexxMaster ClipList.  The command format is
* "rxset name=value", where "name" can be anything and "value" is any
* string.  If the string includes semicolons, it should be enclosed in
* double quotes to prevent the CLI from breaking it up.
* The second parameter may be null, in which case the named string is
* removed from the ClipList.
* If no parameters are supplied, the current contents of the Clip List are
* displayed on the console.
* Usage: rxset [? | name [=] [value]]

         INCLUDE  "rexx/storage.i"
         INCLUDE  "rexx/rxslib.i"
         INCLUDE  "rexx/errors.i"
         INCLUDE  "rexx/rexxio.i"

         INCLUDE  "dos/dos.i"
         INCLUDE  "dos/dosextens.i"

;         INCLUDE  "libdefs.i"

         IFND     EXEC_EXECBASE_I
ThisTask EQU      276
         ENDC

CALLSYS  MACRO    * FunctionName
         jsr      _LVO\1(a6)
         ENDM

* Macro to define an external library entry point (offset)
XLIB     MACRO    * FunctionName
         XREF     _LVO\1
         ENDM

	XLIB	OpenLibrary

MAXNAME  EQU      20                   ; maximum name length
MAXVALUE EQU      50                   ; maximum value length
STACKBF  EQU      80                   ; buffer length

         ; Initial call from CLI has command string in A0, length in D0

start:   movea.l  _AbsExecBase,a6
         lea      -STACKBF(sp),sp      ; stack buffer
         movea.l  ThisTask(a6),a4
         movea.l  a0,a3                ; start of first token

         move.l   d0,d3                ; length
         moveq    #0,d6                ; clear error

         CALLSYS  Forbid               ; exclude task switching

         ; Open the DOS library

         lea      DOSLib(pc),a1
         moveq    #0,d0                ; any version
         CALLSYS  OpenLibrary          ; D0=library
         move.l   d0,d5                ; assume success

         ; Look for the REXX server's public port.

         lea      RexxName(pc),a1      ; REXX port name
         CALLSYS  FindPort             ; D0=port
         move.l   d0,d4                ; port found?
         beq      Error1               ; no

         ; Open the ARexx Systems Library

         lea      RXSLib(pc),a1
         moveq    #0,d0                ; any version
         CALLSYS  OpenLibrary
         tst.l    d0                   ; opened ok?
         beq      Error2               ; no
         move.l   d0,a5

         ; Trim trailing blanks or control characters.

         lea      0(a3,d3.l),a1        ; end of string
         bra.s    2$                   ; jump in

1$:      cmpi.b   #' ',-(a1)           ; character > blank?
         bhi.s    3$                   ; yes
2$:      clr.b    (a1)                 ; install a null
         subq.l   #1,d3                ; any left?
         bcc.s    1$                   ; loop back

         ; Check for special cases

3$:      addq.l   #1,d3                ; length > 0?
         beq      ShowList             ; no ... display names

         cmpi.b   #'?',(a3)            ; a question mark?
         beq      ShowFmt              ; ... display format

         ; Allocate a message packet.

         lea      pr_MsgPort(a4),a0    ; replyport
         suba.l   a1,a1                ; (file extension)
         moveq    #0,d0                ; (host address)
         exg      a5,a6
         CALLSYS  CreateRexxMsg        ; return: D0=A0=packet
         exg      a5,a6
         beq      Error2               ; failure ...
         movea.l  d0,a2                ; save packet
         move.l   #RXADDCON,d2         ; default action code

         ; Parse the command string into name and value tokens.

ParseCommand
         move.l   a3,ARG0(a2)          ; name argument

1$:      subq.l   #1,d3                ; any left?
         bcs.s    RemoveClip           ; no ... remove clip
         move.b   (a3)+,d0             ; test character
         cmpi.b   #' ',d0              ; character <= blank?
         bls.s    2$                   ; yes
         cmpi.b   #'=',d0              ; equals sign?
         bne.s    1$                   ; no ... loop back

2$:      clr.b    -1(a3)               ; install a null

         ; Skip past any leading blanks ...

3$:      subq.l   #1,d3                ; any left?
         bcs.s    RemoveClip           ; no ... remove clip
         cmpi.b   #' ',(a3)+           ; character > blank?
         bls.s    3$                   ; no ... loop back

         subq.w   #1,a3                ; back up
         addq.l   #1,d3                ; restore length

         ; Check for a leading double-quote character.

         moveq    #'"',d0
         cmp.b    (a3),d0              ; quoted value?
         bne.s    5$                   ; no

         addq.w   #1,a3                ; advance pointer
         subq.l   #1,d3                ; decrement length
         beq.s    5$                   ; nothing left
         cmp.b    -1(a3,d3.l),d0       ; ending quote?
         bne.s    5$                   ; no
         subq.l   #1,d3                ; decrement length

5$:      move.l   a3,ARG1(a2)          ; first argument
         move.l   d3,ARG2(a2)          ; save length
         bra.s    SendMsg              ; send the message

         ; Second token was null ... remove name from ClipList.

RemoveClip
         move.l   #RXREMCON,d2         ; remove string

         ; Send the message to the RexxMaster

SendMsg  move.l   d2,ACTION(a2)        ; install command
         move.l   #4,RESULT1(a2)       ; lockout count
         movea.l  d4,a0                ; REXX port
         movea.l  a2,a1
         CALLSYS  PutMsg               ; send the message ...

         ; Wait for the reply.  Task switching is reenabled while Waiting.

         lea      pr_MsgPort(a4),a0
         CALLSYS  WaitPort
         lea      pr_MsgPort(a4),a0
         CALLSYS  GetMsg
         move.l   RESULT1(a2),d6       ; return code
         bra      CleanUp2

         ; Show the names currently defined.

ShowList exg      a5,a6
         moveq    #0,d0                ; all resources
         CALLSYS  LockRexxBase         ; lock structure
         move.l   rl_ClipList(a6),d2   ; first node

1$:      movea.l  d2,a2                ; current node
         move.l   (a2),d2              ; successor?
         beq.s    10$                  ; no

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

4$:      subq.l   #1,a1                ; back up
         lea      Arrow(pc),a0
5$:      move.b   (a0)+,(a1)+          ; copy
         bne.s    5$                   ; loop back
         subq.l   #1,a1                ; back up

         ; Copy the value string

         movea.l  CLVALUE(a2),a0       ; value string
         move.w   (ra_Length-ra_Buff)(a0),d0
         moveq    #MAXVALUE,d1
         cmp.w    d1,d0                ; > maximum?
         bls.s    9$                   ; no
         move.w   d1,d0                ; use maximum
         bra.s    9$

         ; Copy the value string, converting non-printing characters to '?'

6$:      move.b   (a0)+,d1             ; value character
         cmpi.b   #' ',d1              ; > blank?
         bcs.s    7$                   ; no
         cmpi.b   #$7F,d1              ; < $7F?
         bcs.s    8$                   ; yes
7$:      move.b   #'?',d1              ; use a '?'
8$:      move.b   d1,(a1)+
9$:      dbf      d0,6$                ; loop back
         move.b   #$0A,(a1)+           ; a newline
         clr.b    (a1)

         ; Display the string.

         movea.l  sp,a0                ; buffer
         bsr      WriteMsg
         bra.s    1$                   ; loop back

10$:     moveq    #0,d0                ; all resources
         CALLSYS  UnlockRexxBase       ; unlock structure
         exg      a5,a6
         bra.s    CleanUp1

         ; Display the command format

ShowFmt: lea      FmtMsg(pc),a0        ; format string
         bsr.s    WriteMsg
         bra.s    CleanUp1

         ; Error messages

Error1:  lea      ErrMsg1(pc),a0       ; REXX port not found
         bsr.s    WriteMsg
         moveq    #RETURN_ERROR,d6
         bra.s    CloseDOS

Error2:  lea      ErrMsg2(pc),a0       ; "couldn't open library"
         bsr.s    WriteMsg
         moveq    #RETURN_FAIL,d6
         bra.s    CleanUp1

Error3:  lea      ErrMsg3(pc),a0       ; "allocation failure"
         bsr.s    WriteMsg
         moveq    #RETURN_FAIL,d6

         ; Recycle the message packet.

CleanUp2 movea.l  a2,a0                ; message packet
         exg      a5,a6
         CALLSYS  DeleteRexxMsg        ; release it
         exg      a5,a6

         ; Close the REXX library

CleanUp1 movea.l  a5,a1
         CALLSYS  CloseLibrary

         ; Close the DOS library

CloseDOS movea.l  d5,a1
         CALLSYS  CloseLibrary

         ; Reenable task switching

         CALLSYS  Permit

         move.l   d6,d0                ; return code
         lea      STACKBF(sp),sp       ; restore stack
         rts

* ========================      WriteMsg     ========================
* Writes a message to the output stream.
* Registers:   A0 -- message
*              D0 -- length
WriteMsg:
         movem.l  d2/d3/a6,-(sp)
         movea.l  d5,a6

         move.l   a0,d2                ; message
1$:      tst.b    (a0)+
         bne.s    1$
         subq.l   #1,a0
         suba.l   d2,a0
         move.l   a0,d3                ; length

         CALLSYS  Output               ; D0=stream
         move.l   d0,d1
         CALLSYS  Write

         movem.l  (sp)+,d2/d3/a6
         rts

         ; String data

DOSLib   dc.b     'dos.library',0
RXSLib   RXSLIBNAME
RexxName dc.b     'REXX',0             ; public port
Ellipsis dc.b     '...',0              ; an ellipsis
Arrow    dc.b     '=',0                ; name/value separator

FmtMsg   dc.b     'Usage: RXSET [NAME [[=] VALUE]]',10,0
ErrMsg1  dc.b     'ARexx server not active',10,0
ErrMsg2  dc.b     'Couldn't open rexxsyslib.library',10,0
ErrMsg3  dc.b     'Memory allocation failed',10,0
         CNOP     0,2

         END
