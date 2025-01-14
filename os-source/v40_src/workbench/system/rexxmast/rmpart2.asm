* === rmpart2.asm ======================================================
*
* Copyright � 1986, 1987-1990 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
         XDEF     AddClip
         XDEF     AddLib
         XDEF     DoMessage
         XDEF     GroomTasks
         XDEF     rxAsync
         XDEF     rxLaunch

* ==========================     DoMessage      ========================
* Processes message packets received by the RexxMaster.
* Registers:      D0 -- message packet
DoMessage
         movem.l  d2/a2,-(sp)
         movea.l  d0,a2                ; save message

         move.l   ACTION(a2),d2        ; command code
         move.l   d2,d1
         rol.l    #8,d1                ; code byte
         moveq    #DMNUM-1,d0          ; loop count
         lea      DMTab(pc),a0         ; start of table

1$:      cmp.b    (a0)+,d1             ; code matched?
         dbeq     d0,1$                ; loop back
         bne      DMErr10              ; ... unrecognized code

         ; Load registers and branch to process the message

         movea.l  ARG0(a2),a1          ; first argument
         move.b   (DMNUM-1)(a0),d0     ; load offset
         adda.w   d0,a0                ; branch address
         jmp      (a0)                 ; ... process it

         ; Table of valid action codes

DMTab    dc.b     RXCOMM>>24           ; search begins here
         dc.b     RXFUNC>>24
         dc.b     RXADDCON>>24
         dc.b     RXREMCON>>24
         dc.b     RXADDLIB>>24
         dc.b     RXREMLIB>>24
         dc.b     RXADDFH>>24
         dc.b     RXTCOPN>>24
         dc.b     RXTCCLS>>24
         dc.b     RXCLOSE>>24
         dc.b     RXQUERY>>24
DMNUM    EQU      *-DMTab              ; number of entries

         ; Table of jump offsets ...

         dc.b     DMcommf-DMTab-1      ; function packet
         dc.b     DMcommf-DMTab-2      ; command packet
         dc.b     DMaddcl-DMTab-3      ; add Clip Node
         dc.b     DMremcl-DMTab-4      ; remove Clip node
         dc.b     DMaddlb-DMTab-5      ; add library node
         dc.b     DMremlb-DMTab-6      ; remove library node
         dc.b     DMaddfh-DMTab-7      ; add function host
         dc.b     DMtcopn-DMTab-8      ; open console
         dc.b     DMtccls-DMTab-9      ; close console
         dc.b     DMclose-DMTab-10     ; close rexxmaster
         dc.b     DMquery-DMTab-11     ; query request

         ; Remove an existing function library.

DMremlb  tst.w    rl_ReadLock(a5)      ; list locked?
         bne      DMQueue              ; yes
         bsr      RemLib
         bra      DMReturn

         ; Add a node to the Library List.  The node may be either a
         ; "function library" (RRT_LIB) or a "function host" (RRT_HOST).

DMaddlb  moveq    #RRT_LIB,d0          ; function library
         bra.s    DM001

         ; Add a function host node.

DMaddfh  moveq    #RRT_HOST,d0         ; function host

DM001    tst.w    rl_ReadLock(a5)      ; list locked?
         bne      DMQueue              ; yes
         lea      ARG0(a2),a0          ; parameter block
         bsr      AddLib               ; D0=error
         bra      DMReturn

         ; Open a tracing console.  Currently active REXX programs will
         ; install it as the tracing stream when they see the 'TCOPN' flag.

DMtcopn  tst.l    d4                   ; already open?
         bne.s    2$                   ; yes ...

         ; Check whether a "GLOBALCON" value has been defined.

         lea      rl_ClipList(a5),a0   ; Clip List header
         lea      GlCName(pc),a1       ; 'GLOBALCON'
         moveq    #RRT_ANY,d0          ; (any node)
         exg      a5,a6
         CALLSYS  FindRsrcNode         ; D0=A0=node
         exg      a5,a6
         lea      CONDef(pc),a0        ; default console
         beq.s    1$                   ; not found ...

         ; "GLOBALCON" node found ... use definition.

         movea.l  d0,a1                ; resource node
         movea.l  CLVALUE(a1),a0       ; load value

         ; Open the console ...

1$:      bsr      OpenConsole          ; D0=filehandle

2$:      move.l   d4,rl_TraceFH(a5)    ; opened?
         beq.s    DMWarn               ; no -- error
         bclr     #CLOSETC,d6          ; clear 'close console' flag
         bra.s    DMAllOK

         ; Request to close the tracing console.  Console can't be closed 
         ; while programs are using it ... clear filehandle and wait.

DMtccls  clr.l    rl_TraceFH(a5)       ; clear filehandle
         bset     #CLOSETC,d6          ; set 'close console' flag
         bra.s    DMAllOK

         ; Set the global 'close' flag and wait for current tasks to finish.
         ; The public port is withdrawn, but messages are accepted until the
         ; last task finishes.

DMclose  bset     #RLFB_CLOSE,rl_Flags(a5)
         bra.s    DMAllOK

DMquery  bra.s    DMErr10              ; not implemented

         ; Add a value string to the Clip List.

DMaddcl  tst.w    rl_ReadLock(a5)      ; list locked?
         bne.s    DMQueue              ; yes
         lea      ARG0(a2),a0          ; parameter block
         bsr      AddClip              ; D0=error code
         bra.s    DMReturn

         ; Remove a value string from the Clip List.

DMremcl  tst.w    rl_ReadLock(a5)      ; list locked?
         bne.s    DMQueue              ; yes
         bsr      RemClip              ; D0=error
         bra.s    DMReturn

         ; Command/Function packet processing.  Creates and initializes
         ; a new DOS process, then forwards the message packet as the
         ; "wake-up."  The message carries the address of the RexxTask
         ; structure used to manage the process.

DMcommf  moveq    #ERR10_001,d0        ; "program not found"
         tst.w    d6                   ; RexxMaster closing?
         bmi.s    DMReturn             ; yes
         btst     #RLFB_STOP,rl_Flags(a5)
         bne.s    DMWarn

         movea.l  a2,a0                ; message
         bsr      rxLaunch             ; D0=error
         bne.s    DMReturn             ; ... error
         bra.s    DMOut

         ; Resource list locked ... queue message for later processing

DMQueue  tst.w    d6                   ; closing?
         bmi.s    DMWarn               ; yes
         tst.l    RESULT1(a2)          ; count > 0?
         ble.s    DMWarn               ; no

         ; Queue the message

         lea      rl_MsgList(a5),a0    ; pending messages
         movea.l  a2,a1                ; this message
         CALLSYS  AddTail              ; post to end
         addq.w   #1,rl_NumMsg(a5)
         bra.s    DMOut

         ; Error conditions:  load error code and return the packet

DMErr03: moveq    #ERR10_003,d0        ; insufficient memory
         bra.s    DMReturn

DMErr10: moveq    #ERR10_010,d0        ; bad message packet
         bra.s    DMReturn

DMWarn:  moveq    #RMWARN,d0           ; warning error
         bra.s    DMReturn

DMAllOK  moveq    #0,d0                ; no errors

         ; Send the message back with the error codes filled in.

DMReturn btst     #RXFB_NONRET,d2      ; a "no return" message?
         beq.s    1$                   ; no

         ; A "no return" message ... release it

         movea.l  a2,a0                ; message packet
         exg      a5,a6
         CALLSYS  DeleteRexxMsg        ; release it
         exg      a5,a6
         bra.s    DMOut

         ; Fill in the result codes and reply the message

1$:      move.l   d0,d2                ; an error?
         beq.s    3$                   ; no ...
         bmi.s    2$                   ; a warning

         ; Get the system return code

         exg      a5,a6
         CALLSYS  ErrorMsg             ; A0=string structure
         exg      a5,a6
         moveq    #0,d0
         move.w   (IVALUE+2)(a0),d0    ; return code
         bra.s    3$

2$:      moveq    #RC_WARN,d0          ; warning level
         moveq    #0,d2                ; no reason

3$:      movem.l  d0/d2,RESULT1(a2)    ; install results
         movea.l  a2,a1
         CALLSYS  ReplyMsg

DMOut    movem.l  (sp)+,d2/a2
         rts

* ==========================     rxLaunch     ==========================
* Processes Command and Function packets.  Creates and initializes a new
* DOS process, then forwards the message packet as the "wake-up" message.
* The message carries the address of the RexxTask structure used to manage
* the process.
* Registers:      A0 -- message
* Return:         D0 -- error code
rxLaunch
         movem.l  d2-d4/d7/a2/a3,-(sp)
         movea.l  a0,a2                ; save message

         ; Allocate a RexxTask structure.

         move.l   #rt_SIZEOF,d0        ; structure size
         move.l   #MEMCLEAR!MEMQUICK,d1
         CALLSYS  AllocMem             ; D0=memory
         tst.l    d0                   ; success?
         beq      5$                   ; no ...
         movea.l  d0,a3                ; save structure
         movem.l  a3/a5,RESULT1(a2)    ; RexxTask pointer/library base

         ; Link the RexxTask structure into our task list, using the 
         ; message port as the linkage node.

         lea      rl_TaskList(a5),a0   ; task list header
         lea      rt_MsgPort(a3),a1    ; linkage node
         CALLSYS  AddTail              ; ... link it
         addq.w   #1,rl_NumTask(a5)    ; increment task count

         ; Create the new process, using the parameters stored in the
         ; REXX system library.  The message packet is forwarded to 
         ; the new process, which won't start until the RexxMaster Waits.

         movem.l  rl_TaskName(a5),d1-d4; load parameters
         exg      d5,a6                ; EXEC=>D5 , DOS=>A6
         CALLSYS  CreateProc           ; D0=processid
         exg      d5,a6                ; DOS=>D5 , EXEC=>A6
         move.l   d0,d7                ; success?
         beq      4$                   ; no ... set 'close' flag

         movea.l  d0,a0                ; new processid (message port)
         lea      -pr_MsgPort(a0),a0   ; the task ID
         exg      a0,a3                ; structure=>A0, new task=>A3

         ; Get the client's task ID from the message packet.

         move.l   MN_REPLYPORT(a2),d1  ; a replyport?
         beq.s    1$                   ; ... no replyport

         movea.l  d1,a1
         tst.b    MP_FLAGS(a1)         ; PA_SIGNAL?
         bne.s    1$                   ; no ...

         ; Check whether the client is a process.

         movea.l  MP_SIGTASK(a1),a1    ; client task
         cmpi.b   #NT_PROCESS,LN_TYPE(a1)
         bne.s    2$                   ; not a process ...
         move.l   pr_CLI(a1),d0        ; a CLI?
         bne.s    3$                   ; yes
         bra.s    2$                   ; no ... use default

         ; No client task ... clear the pointer.

1$:      suba.l   a1,a1                ; clear task ID

         ; Copy the RexxMaster's CLI structure.

2$:      move.l   pr_CLI(a4),d0        ; default CLI

         ; Fill in the communications fields in the RexxTask structure:
         ; ClientID/MsgPkt/TaskID/RexxMaster

3$:      movem.l  a1/a2/a3/a4,rt_ClientID(a0)

*         move.l   MN_REPLYPORT(a2),d1  ; a replyport?
*         movea.l  d1,a1
*         beq.s    1$                   ; ... no replyport
*         tst.b    MP_FLAGS(a1)         ; PA_SIGNAL?
*         movea.l  MP_SIGTASK(a1),a1    ; (client task)
*         beq.s    1$                   ; yes
*         suba.l   a1,a1                ; clear task ID
*
*         ; Fill in the communications fields in the RexxTask structure:
*         ; ClientID/MsgPkt/TaskID/RexxMaster
*
*1$:      movem.l  a1/a2/a3/a4,rt_ClientID(a0)
*
*         ; Create a CLI structure for the new process ...
*
*         cmpi.b   #NT_PROCESS,LN_TYPE(a1)
*         bne.s    2$                   ; not a process
*         move.l   pr_CLI(a1),d0        ; a CLI?
*         bne.s    3$                   ; yes
*2$:      move.l   pr_CLI(a4),d0        ; ... use default CLI
*
         ; Copy the CLI structure and install it in the new process.

         exg      a5,a6
         CALLSYS  CreateCLI            ; D0=BPTR to CLI
         exg      a5,a6
         move.l   d0,pr_CLI(a3)        ; ... install it

         ; Send the invocation message packet as the "wake-up".

         movea.l  d7,a0                ; new processid
         movea.l  a2,a1                ; message packet
         CALLSYS  PutMsg               ; send it off

         moveq    #0,d0                ; all ok
         bra.s    6$

         ; Error ... process creation failed.  Mark the RexxTask structure
         ; to be removed ...

4$:      bset     #RTFB_CLOSE,rt_Flags(a3)

         ; Error ... allocation failure.

5$:      moveq    #ERR10_003,d0        ; allocation failure

6$:      movem.l  (sp)+,d2-d4/d7/a2/a3
         rts

* ===========================     AddLib     ===========================
* Allocates and links a function library or function host node.
* Registers:      A0 -- parameter block
*                 A1 -- node name
*                 D0 -- node type
AddLib
         movem.l  d2/a2,-(sp)
         movea.l  a0,a2                ; save parameters
         move.b   d0,d2                ; save type

         ; Check for an existing node

         lea      rl_LibList(a5),a0
         bsr.s    FindAnyNode          ; D0=A0=node
         bne.s    ALWarn               ; already exists ...

         ; Allocate a new resource node.

         lea      rl_LibList(a5),a0    ; library list header
         movea.l  (a2)+,a1             ; name string
         moveq    #RRSIZEOF,d0         ; structure size
         exg      a5,a6
         CALLSYS  AddRsrcNode          ; D0=A0=node
         exg      a5,a6
         beq.s    ALErr03              ; failure?

         ; Install fields in the node

         move.b   d2,RRTYPE(a0)        ; node type
         movem.l  (a2),d0/d1/d2        ; load values
         movem.l  d1/d2,LLOFFSET(a0)   ; install offset/version
         move.b   d0,LN_PRI(a0)        ; set priority
         movea.l  a0,a2                ; save node

         ; Requeue the node at the correct priority.

         movea.l  a2,a1                ; node
         CALLSYS  Remove               ; unlink
         lea      rl_LibList(a5),a0    ; header
         movea.l  a2,a1                ; node
         CALLSYS  Enqueue              ; enqueue it

         addq.w   #1,rl_NumLib(a5)
         moveq    #0,d0
         bra.s    ALOut

ALWarn   moveq    #RMWARN,d0
         bra.s    ALOut

ALErr03  moveq    #ERR10_003,d0        ; allocation failure

ALOut    movem.l  (sp)+,d2/a2
         rts

* =========================     FindAnyNode     ========================
* Searches for a resource node of "any" type.
* Registers:      A0 -- list header
*                 A1 -- node name
* Return:         D0 -- node or 0
*                 A0 -- same
FindAnyNode:
         moveq    #RRT_ANY,d0          ; any node
         exg      a5,a6
         CALLSYS  FindRsrcNode         ; D0=A0=node
         exg      a5,a6
         rts

* =========================     RemAnyNode     =========================
* Removes a resource node.
* Registers:      A0 -- node
RemAnyNode:
         exg      a5,a6
         CALLSYS  RemRsrcNode
         exg      a5,a6
         rts

* ===========================     RemClip     ==========================
* Removes a Clip List node.
* Registers:      A1 -- node name
* Return:         D0 -- error code
RemClip:
         lea      rl_ClipList(a5),a0   ; Clip List header
         bsr.s    FindAnyNode          ; D0=A0=node
         beq.s    RL002                ; not found ...
         subq.w   #1,rl_NumClip(a5)    ; decrement count
         bra.s    RL001

* ============================    RemLib     ===========================
* Removes a node from the function library list.
* Registers:      A1 -- node name
* Return:         D0 -- error code
RemLib
         lea      rl_LibList(a5),a0    ; Library List header
         bsr.s    FindAnyNode          ; D0=A0=node
         beq.s    RL002                ; not found
         subq.w   #1,rl_NumLib(a5)     ; decrement count

RL001:   bsr.s    RemAnyNode           ; remove it
         moveq    #0,d0
         bra.s    RL003

RL002:   moveq    #RMWARN,d0           ; warning error

RL003:   rts

* =========================     AddClip     ============================
* Adds a value string to the Clip List.
* Registers:      A0 -- parameter block
*                 A1 -- node name
* Return:         D0 -- error code
AddClip
         move.l   a2,-(sp)
         movea.l  a0,a2                ; save parameter

         ; Remove an existing node of the same name ...

         bsr.s    RemClip

         ; Allocate a new node, including space for the constant string.

         lea      rl_ClipList(a5),a0
         movea.l  (a2)+,a1             ; name string
         move.l   (a2)+,d1             ; value string
         move.l   (a2),d0              ; value length
         cmpi.l   #MAXLEN,d0           ; too long?
         bgt.s    ACErr09              ; yes

         exg      a5,a6
         CALLSYS  AddClipNode          ; D0=A0=node
         exg      a5,a6
         beq.s    ACErr03              ; failure

         addq.w   #1,rl_NumClip(a5)
         moveq    #0,d0
         bra.s    ACOut

ACErr03: moveq    #ERR10_003,d0        ; allocation failure
         bra.s    ACOut

ACErr09: moveq    #ERR10_009,d0        ; string too long

ACOut:   movea.l  (sp)+,a2
         rts

* =========================     GroomTasks     =========================
* Scans the task list looking for completed tasks, and passes signals from
* the RexxMaster to the REXX tasks.
GroomTasks
         movem.l  d2/d3/a2/a3,-(sp)

         ; Load the selected new flags.

         moveq    #RLFMASK,d3          ; flags mask
         and.b    rl_Flags(a5),d3      ; new flags

         clr.w    rl_TraceCnt(a5)      ; clear count
         move.l   rl_TaskList(a5),d2   ; first node
         bra.s    4$                   ; jump in

         ; Check if any tasks are using the global tracing console.

1$:      move.b   rt_Flags(a3),d1      ; task flags
         btst     #RTFB_TCUSE,d1       ; console in use?
         beq.s    2$                   ; no ...
         addq.w   #1,rl_TraceCnt(a5)   ; bump count

         ; Check whether the task has been completed.  If so, recycle it.

2$:      btst     #RTFB_CLOSE,d1       ; task complete?
         beq.s    3$                   ; no ...

         movea.l  a2,a1                ; linkage node
         CALLSYS  Remove               ; ... unlink it
         movea.l  a3,a1                ; RexxTask structure
         move.l   #rt_SIZEOF,d0        ; load size
         CALLSYS  FreeMem              ; release it
         subq.w   #1,rl_NumTask(a5)    ; decrement count
         bra.s    4$

         ; Compare the current control flags to the shadow flags, and
         ; pass any differences to the individual ARexx tasks.

3$:      move.b   d3,d0                ; new flags
         or.b     rl_Shadow(a5),d0     ; OR in shadow
         not.b    d0                   ; invert mask
         and.b    d1,d0                ; select old flags
         or.b     d3,d0                ; OR in new flags
         cmp.b    d1,d0                ; anything changed?
         beq.s    4$                   ; no

         ; Flags changed ... update the task flags and signal the task.

         move.b   d0,rt_Flags(a3)      ; new flags
         move.b   rt_SigBit(a3),d1     ; signal bit
         moveq    #0,d0
         bset     d1,d0                ; signal mask
         movea.l  rt_TaskID(a3),a1     ; task ID
         CALLSYS  Signal               ; signal it

         ; Get the next linkage node in the Tasks List.

4$:      movea.l  d2,a2                ; current linkage node
         lea      -rt_MsgPort(a2),a3   ; RexxTask structure
         move.l   (a2),d2              ; a successor?
         bne.s    1$                   ; yes

         ; Clear the HALT flag before updating shadow ... one time only.

         moveq    #RLFB_HALT,d1        ; load flag
         bclr     d1,d3
         bclr     d1,rl_Flags(a5)      ; clear HALT
         move.b   d3,rl_Shadow(a5)     ; update shadow

         movem.l  (sp)+,d2/d3/a2/a3
         rts

* ===========================     AgeList     ==========================
* Ages the pending actions list.
AgeList
         move.l   rl_MsgList(a5),d0    ; first node
         bra.s    2$                   ; jump in

1$:      subq.l   #1,RESULT1(a0)       ; decrement lockout count

2$:      movea.l  d0,a0                ; current node
         move.l   (a0),d0              ; a successor?
         bne.s    1$                   ; yes
         rts

* ===========================     rxAsync     ==========================
* Launches a command asynchronously.
* Registers:      D0 -- message
rxAsync
         movem.l  d2/d3/a2/a5/a6,-(sp)
         movea.l  d0,a2
         exg      a5,a6                ; EXEC=>A5, REXX=>A6

         moveq    #10,d2               ; default return
         moveq    #0,d3                ; default secondary

         ; An ASYNC message ... check whether it's a reply.

         cmpi.b   #NT_REPLYMSG,LN_TYPE(a2)
         bne.s    1$

         ; A reply message ... release it.

         subq.w   #1,sf_MsgCount(a3)   ; decrement count
         movea.l  rm_Args(a2),a0       ; command string
         CALLSYS  DeleteArgstring
         movea.l  a2,a0                ; message
         CALLSYS  DeleteRexxMsg        ; release it
         bra.s    5$

         ; An incoming ASYNC message ... clone it.

1$:      tst.w    d6                   ; closing?
         bmi.s    4$                   ; yes ... decline invocation

         ; Create a message packet.

         movea.l  a3,a0                ; replyport
         movea.l  rm_FileExt(a2),a1    ; file extension
         move.l   (rl_RexxPort+LN_NAME)(a6),d0
         CALLSYS  CreateRexxMsg        ; D0=A0=message
         beq.s    3$                   ; ... failure

         ; Clone the command argstring ...

         move.l   a0,-(sp)             ; push new message
         movea.l  rm_Args(a2),a0       ; command argument
         moveq    #0,d0                ; clear length
         move.w   (ra_Length-ra_Buff)(a0),d0
         CALLSYS  CreateArgstring      ; D0=A0=argstring
         movea.l  (sp)+,a1             ; pop new message
         beq.s    2$                   ; ... failure

         ; ... and fill in the new message packet.

         move.l   d0,rm_Args(a1)       ; install command
         move.l   #RXCOMM,rm_Action(a1)

         ; Send the new invocation to REXX itself.

         lea      rl_RexxPort(a6),a0   ; REXX port
         movea.l  a5,a6                ; EXEC base
         CALLSYS  PutMsg               ; send it
         addq.w   #1,sf_MsgCount(a3)   ; increment count

         moveq    #0,d2                ; clear return
         bra.s    4$

         ; Argstring allocation failed ... release the message.

2$:      movea.l  a1,a0                ; RexxMsg
         CALLSYS  DeleteRexxMsg        ; release it

         ; Error ... allocation failed.

3$:      moveq    #ERR10_003,d3        ; "insufficient memory"

         ; Fill in the results fields and reply the original message.

4$:      movem.l  d2/d3,rm_Result1(a2) ; install returns
         movea.l  a2,a1                ; old message
         movea.l  a5,a6                ; EXEC base
         CALLSYS  ReplyMsg

5$:      movem.l  (sp)+,d2/d3/a2/a5/a6
         rts
