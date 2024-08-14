*
* $Id: slf200.asm,v 36.1 90/08/28 11:34:12 mks Exp $
*
* $Log:	slf200.asm,v $
* Revision 36.1  90/08/28  11:34:12  mks
* Added RCS header information.  ($Id and $Log)
* 
* ===========================     FORBID     ===========================
* Implements the FORBID() function.
* Usage: FORBID()
SFforbid
         exg      d4,a6
         CALLSYS  Forbid
         bra.s    SF2000

* ===========================     PERMIT     ===========================
* Implements the PERMIT() function.
* Usage: PERMIT()
SFpermit
         exg      d4,a6
         CALLSYS  Permit

SF2000   move.b   TDNestCnt(a6),d3     ; nesting count
         ext.w    d3
         ext.l    d3                   ; signed count
         exg      d4,a6
         bra      SFNumber

* ==========================     NULL     ==============================
* Returns a NULL address ('00000000'x).
* Usage: NULL()
SFnull
         clr.l    (a3)
         bra.s    SF2001

* =============================     BADDR     ==========================
* Converts a BPTR to a machine address.
* Usage: BADDR(BPTR-address)
SFbaddr
         subq.w   #4,d0                ; 4-byte address?
         bne.s    SF2002               ; no ... error
         move.l   (a0),d0              ; load BPTR
         lsl.l    #2,d0                ; real address
         move.l   d0,(a3)              ; install it
         bra.s    SF2001

* ============================     NEXT     ============================
* Returns the next node in a list.
* Usage: NEXT(address,[offset])
SFnext
         subq.w   #4,d0                ; 4-byte address?
         bne.s    SF2002               ; no
         add.l    (a0),d3              ; (address+displacement)
         btst     #0,d3                ; even address?
         bne.s    SF2002               ; no ... error
         movea.l  d3,a0
         move.l   (a0),(a3)            ; next node
         bra.s    SF2001

* ==========================     OFFSET     ============================
* Computes a signed displacement to an address.
* Usage: OFFSET(address,displacement)
SFoffset
         subq.w   #4,d0                ; 4-byte address?
         bne.s    SF2002               ; no
         movea.l  (a0),a2              ; load address

         ; Convert the displacement argument ...

         lea      -ns_Buff(a1),a0      ; argument string
         exg      a4,a6
         CALLSYS  CVs2i                ; D0=error D1=value
         exg      a4,a6
         bne.s    SF2002               ; ... error

         adda.l   d1,a2                ; new address
         move.l   a2,(a3)              ; install it
         bra.s    SF2001

* =========================     ALLOCMEM     ===========================
* Allocates a block of memory from the system and returns it as a 4-byte
* address.  The default attribute is for PUBLIC memory.
* Usage: ALLOCMEM(size,[attribute])
SFallocmem
         move.l   d3,d0                ; length given?
         beq.s    SF2002               ; no -- error

         move.l   #MEMQUICK,d2         ; default attribute
         cmpi.b   #2,d7                ; attribute argument?
         bne.s    1$                   ; no

         subq.w   #4,d1                ; 4-byte string?
         bne.s    SF2002               ; no -- error
         move.l   (a1),d2              ; load attribute

1$:      move.l   d2,d1                ; memory attributes
         exg      d4,a6
         CALLSYS  AllocMem             ; D0=block
         exg      d4,a6

         move.l   d0,(a3)              ; memory allocated?
         beq      SFErr03              ; no --error

SF2001   moveq    #4,d3                ; length
         bra      SFNewstr

SF2002   bra      SFErr18              ; "invalid operand"

* ==========================     GETPKT     ============================
* Checks the specified message port for a packet.  If found, the packet
* is removed and queued to be replied.
* USAGE: GETPKT(name)
SFgetpkt
         movea.l  a0,a1                ; port name
         lea      PORTLIST(a2),a0      ; list header
         moveq    #RRT_PORT,d0         ; node type
         exg      a4,a6                ; SUPP=>A4, REXX=>A6
         CALLSYS  FindRsrcNode         ; D0=A0=node
         exg      a4,a6
         beq.s    SF2002               ; error ... not found

         ; Check for a message packet (but don't wait).

         movea.l  a0,a2                ; structure
         lea      rmp_Port(a2),a0      ; message port
         exg      d4,a6                ; SUPP=>D4, EXEC=>A6
         CALLSYS  GetMsg               ; D0=message

         ; Return the address of the message packet (or '00000000'x).

         lea      rmp_ReplyList(a2),a0 ; reply list
         lea      LH_TAIL(a0),a3       ; default return (NULL)
         tst.l    d0                   ; a message?
         beq.s    1$                   ; no
         addq.w   #(LH_TAILPRED-LH_TAIL),a3

         ; Queue the packet on the ReplyList ...

         movea.l  d0,a1                ; message
         CALLSYS  AddTail              ; link it

         ; Create the return string ...

1$:      exg      d4,a6                ; EXEC=>D4, SUPP=>A6
         bra.s    SF2001               ; 4-byte return

* ===========================     TYPEPKT     ==========================
* Returns the packet action code as a 4-byte string, or checks for a
* specific packet type.
* Usage: TYPEPKT(packet,['Arguments' | 'Command' | 'Function'])
SFtypepkt
         subq.w   #4,d0                ; 4-byte string?
         bne.s    SF2002               ; no ... error

         movea.l  (a0),a2              ; message packet
         lea      ACTION(a2),a3        ; pointer to command word
         move.l   (a3),d2              ; action code
         move.b   d2,d3                ; ... argument count

         ; Check whether an option was specified.

         cmpi.b   #2,d7                ; option argument?
         blt.s    SF2001               ; no ... return command code

         move.b   (a1),d1              ; option character
         bclr     #LOWERBIT,d1
         subi.b   #'A',d1              ; 'Arguments'?
         beq      SFNumber             ; yes ... return count
         move.l   #RXCOMM,d0
         subq.b   #'C'-'A',d1          ; 'Command'?
         beq.s    1$                   ; yes
         move.l   #RXFUNC,d0
         subq.b   #'F'-'C',d1          ; 'Function'?
         bne.s    SF2002               ; no -- error

         ; Check whether the action code matches the selected value.

1$:      andi.l   #RXCODEMASK,d2       ; command code
         cmp.l    d0,d2                ; selected code?
         seq      d3                   ; ... set flag
         bra.s    SF2003               ; boolean return

* ===========================     FREEMEM     ==========================
* Releases a block of memory to the system.
* Usage: FREEMEM(address,size)
SFfreemem
         subq.w   #4,d0                ; 4-byte address?
         bne.s    SF2004               ; no ...
         move.l   d3,d0                ; length to released
         beq.s    SF2004               ; zero length??
         move.l   (a0),d1              ; a block?
         beq.s    SF2004               ; no
         movea.l  d1,a1                ; yes ...

         exg      d4,a6
         CALLSYS  FreeMem
         exg      d4,a6
         moveq    #-1,d3               ; TRUE return
         bra.s    SF2003               ; boolean return

* ===========================     OPENPORT     =========================
* Opens a public message port for commands or function messages.
* USAGE: OPENPORT(name)
SFopenport
         movea.l  a0,a3                ; save name

         ; Exclude multitasking, then check whether a public port with
         ; this name already exists ...

         exg      d4,a6                ; SUPP=>D4, EXEC=>A6
         CALLSYS  Forbid
         movea.l  a3,a1                ; port name
         CALLSYS  FindPort             ; D0=node
         tst.l    d0                   ; port found?
         bne.s    1$                   ; yes ... don't allocate

         ; Allocate the message port as a resource node.

         lea      PORTLIST(a2),a0      ; list header
         movea.l  a3,a1                ; port name
         exg      a4,a6                ; SUPP=>A4, REXX=>A6
         CALLSYS  OpenPublicPort       ; D0=A0=node
         exg      a4,a6
         sne      d3                   ; set flag

         ; Turn multitasking back on ...

1$:      CALLSYS  Permit
         exg      d4,a6
         bra.s    SF2003               ; boolean return

* =========================     CLOSEPORT     ==========================
* Closes a named message port and releases the node.
* USAGE: CLOSEPORT(name)
SFcloseport
         exg      a4,a6
         movea.l  a0,a1                ; name
         lea      PORTLIST(a2),a0      ; list header
         moveq    #RRT_PORT,d0         ; node type
         CALLSYS  FindRsrcNode         ; D0=A0=node
         move.l   d0,d3                ; found?
         beq.s    1$                   ; no

         ; Remove the port from the system list and deallocate it.

         CALLSYS  ClosePublicPort      ; recycle the node
1$:      exg      a4,a6

SF2003   bra      SFBool               ; boolean return

SF2004   bra      SFErr18              ; invalid operand

* ===========================     FORWARD     ==========================
* Forwards a message packet to the named port.
* USAGE: FORWARD(packet,name)
SFforward
         subq.w   #4,d0                ; 4-byte address?
         bne.s    SF2004               ; no
         movea.l  (a0),a2              ; get packet

         ; Forbid task switching, then look for the port.

         exg      d4,a6
         CALLSYS  Forbid               ; (registers preserved)

         CALLSYS  FindPort             ; D0=port
         move.l   d0,d3                ; port found?
         beq.s    1$                   ; no

         movea.l  a2,a1                ; message node
         CALLSYS  Remove               ; unlink it
         movea.l  d2,a0                ; forwarding port
         movea.l  a2,a1                ; message node
         CALLSYS  PutMsg               ; ... send it on

1$:      CALLSYS  Permit               ; reenable switching
         exg      d4,a6
         bra.s    SF2003               ; boolean return

* ============================     REPLY     ===========================
* Returns a message packet and sets the return code.
* USAGE: REPLY(packet,code,[secondary])
SFreply
         subq.w   #4,d0                ; 4-byte address?
         bne.s    SF2004               ; no
         move.l   (a0),d0              ; load pointer
         beq.s    SF2004               ; ... invalid
         movea.l  d0,a2                ; message packet

         ; Fill in primary and secondary return codes.

         clr.l    RESULT2(a2)          ; clear secondary
         move.l   d3,RESULT1(a2)       ; primary return code
         bne.s    1$                   ; ... error

         ; Check whether a result was requested (and available).

         btst     #(RXFB_RESULT-16),(rm_Action+1)(a2)
         beq.s    1$                   ; ... not requested
         cmpi.b   #3,d7                ; an argument?
         blt.s    1$                   ; no
         move.l   (ARG2+4)(a5),d1      ; argument supplied?
         beq.s    1$                   ; no

         ; Result string requested ... allocate an argstring.

         movea.l  d1,a0                ; secondary result
         move.w   (ns_Length-ns_Buff)(a0),d0
         exg      a4,a6
         CALLSYS  CreateArgstring      ; D0=A0=result
         exg      a4,a6
         move.l   d0,RESULT2(a2)       ; install it

         ; Remove the message from the replylist and reply ...

1$:      exg      d4,a6
         movea.l  a2,a1                ; message node
         CALLSYS  Remove               ; unlink it
         movea.l  a2,a1                ; message node
         CALLSYS  ReplyMsg             ; reply it
         exg      d4,a6

         moveq    #-1,d3               ; set flag
         bra.s    SF2003               ; boolean return

* ==========================     WAITPKT     ===========================
* Waits for a message at the specified port.  The boolean return indicates
* whether a message was found.
* USAGE: WAITPKT(name)
SFwaitpkt
         movea.l  a0,a1                ; port name
         lea      PORTLIST(a2),a0      ; PortList header
         moveq    #RRT_PORT,d0         ; node type
         exg      a4,a6
         CALLSYS  FindRsrcNode         ; D0=A0=node
         exg      a4,a6
         beq      SFErr18              ; error -- not found

         ; Check whether a message is already here ...

         movea.l  (rmp_Port+MP_MSGLIST)(a0),a1
         move.l   (a1),d3              ; a successor?
         bne.s    1$                   ; yes

         ; Build the wait mask ... two signals are ORed together.

         move.b   rt_SigBit(a2),d1     ; global signal bit
         move.b   (rmp_Port+MP_SIGBIT)(a0),d2
         moveq    #0,d0                ; clear mask
         bset     d1,d0                ; set signal
         bset     d2,d0                ; set signal

         ; Wait for a message packet or signal event.

         exg      d4,a6
         CALLSYS  Wait                 ; D0=signal mask
         exg      d4,a6
         btst     d2,d0                ; port signal?
         sne      d3                   ; ... set flag

1$:      bra      SFBool               ; boolean return

* ============================     GETARG     ==========================
* Returns the argument string from a message packet.
* USAGE: GETARG(packet,[argument])
SFgetarg
         subq.w   #4,d0                ; 4-byte string?
         bne      SFErr18              ; no -- error

         movea.l  (a0),a2              ; message packet
         move.l   ACTION(a2),d1        ; command word
         move.b   d1,d2                ; argument count
         andi.l   #RXCODEMASK,d1       ; command code
         cmpi.l   #RXCOMM,d1           ; a command?
         beq.s    1$                   ; yes
         cmpi.l   #RXFUNC,d1           ; a function?
         bne      SFErr18              ; no -- error

         ; Check for a valid argument count.

1$:      cmp.l    d2,d3                ; argument <= count?
         bgt.s    2$                   ; no ... null return

         ; Extract the command string and copy it ...

         lsl.l    #2,d3                ; scale for 4 bytes
         move.l   ARG0(a2,d3.l),d0     ; argument pointer
         beq.s    2$                   ; ... not supplied

         movea.l  d0,a3                ; argstring
         move.w   (ra_Length-ra_Buff)(a3),d3
         bra      SFNewstr             ; string return

2$:      bra      SFNull

* ===========================     SHOWLIST     =========================
* Builds a string of the names in a list, or checks for a particular name.
* The list can be either an option keyword or the 4-byte machine address
* of a valid EXEC-style list header.
* Usage: SHOWLIST(list,[name],[pad],['Address'])
STACKBF  SET      LH_SIZE
SFshowlist
         move.l   a6,-(sp)
         lea      -STACKBF(sp),sp
         movea.l  a0,a2                ; save argument

         ; Initialize the stack list header

         movea.l  sp,a0                ; list header
         move.l   a4,a6                ; REXX base
         CALLSYS  InitList

         ; Check for a list keyword

         moveq    #0,d2                ; clear 'disable' flag
         move.b   (a2),d0              ; list option
         bclr     #LOWERBIT,d0         ; convert to uppercase
         subi.b   #'@',d0              ; absolute?
         beq.s    1$                   ; yes

         movea.l  d4,a2                ; EXEC base
         moveq    #DLT_DIRECTORY,d1
         subq.b   #'A'-'@',d0          ; 'Assigns'?
         beq.s    2$                   ; yes
         lea      DeviceList(a2),a0
         subq.b   #'D'-'A',d0          ; 'Devices'?
         beq.s    3$                   ; yes
         moveq    #DLT_DEVICE,d1
         subq.b   #'H'-'D',d0          ; 'Handlers'?
         beq.s    2$                   ; yes
         lea      IntrList(a2),a0
         subq.b   #'I'-'H',d0          ; 'Interrupts'?
         beq.s    3$                   ; yes
         lea      LibList(a2),a0
         subq.b   #'L'-'I',d0          ; 'Libraries'?
         beq.s    3$                   ; yes
         lea      MemList(a2),a0
         subq.b   #'M'-'L',d0          ; 'Memory'?
         beq.s    3$                   ; yes
         lea      PortList(a2),a0
         subq.b   #'P'-'M',d0          ; 'Ports'?
         beq.s    3$                   ; yes
         lea      ResourceList(a2),a0
         subq.b   #'R'-'P',d0          ; 'Resources'?
         beq.s    3$                   ; yes
         lea      SemaphoreList(a2),a0
         subq.b   #'S'-'R',d0          ; 'Semaphores'?
         beq.s    3$                   ; yes
         moveq    #DLT_VOLUME,d1
         subq.b   #'V'-'S',d0          ; 'Volumes'?
         beq.s    2$                   ; yes

         ; The following options require disabling interrupts ...

         moveq    #-1,d2               ; disable flag
         lea      TaskReady(a2),a0     ; ready tasks
         addq.b   #'V'-'T',d0          ; 'Tasks'?
         beq.s    3$                   ; yes
         lea      TaskWait(a2),a0      ; waiting tasks
         subq.b   #'W'-'T',d0          ; 'Waiting'?
         beq.s    3$                   ; yes

0$:      bra      SF2005               ; error ... invalid option

         ; Check for an absolute list address in the form @<address>

1$:      subq.w   #5,d0                ; @ADDR format?
         bne.s    0$                   ; no ... error

         move.l   (a0)+,d0             ; load upper address
         lsl.l    #8,d0                ; shift over
         move.b   (a0),d0              ; low-order byte
         btst     #0,d0                ; even address?
         bne.s    0$                   ; no ... error

         ; Check for a valid list header.

         movea.l  d0,a0                ; list head
         tst.l    4(a0)                ; tail NULL?
         bne.s    0$                   ; no ... invalid header
         bra.s    3$

         ; DOS list selected ... build a list of the names and nodes.

2$:      movea.l  sp,a0                ; list header
         bsr.s    WalkDOSList          ; A0=header (A1 preserved)

         ; List selected ... look for a particular node name?

3$:      cmpi.b   #2,d7                ; name argument?
         blt.s    6$                   ; no
         move.l   a1,d1                ; supplied?
         beq.s    6$                   ; no

         ; Name specified ... look for it in the list.

         movea.l  d4,a6                ; SUPP=>A2, EXEC=>A6
         move.l   a0,d3                ; save list header
         CALLSYS  FindName             ; D0=node
         exg      d0,d3                ; header=>D0, node=>D3

         cmpi.b   #4,d7                ; 4th argument?
         blt.s    5$                   ; no
         move.l   (ARG2+8)(a5),d1      ; 'Address' argument
         beq.s    5$                   ; no
         movea.l  d1,a1
         move.b   (a1),d1              ; test character
         bclr     #LOWERBIT,d1         ; uppercase
         cmpi.b   #'A',d1              ; 'Address'?
         bne.s    SF2005               ; no ... error

         ; Return the node address ...

         tst.l    d3                   ; node found?
         beq.s    4$                   ; no ...
         cmp.l    sp,d0                ; DOS (temporary) list?
         bne.s    4$                   ; no ...
         move.l   d3,a0                ; temporary node
         move.l   rr_Arg1(a0),d3       ; ... DOS node address

4$:      move.l   d3,(a3)              ; node address
         moveq    #4,d3                ; 4-byte return
         bset     #NEWSTR,d7
         bra.s    SF2006

         ; Return a boolean value.

5$:      bset     #BOOLEAN,d7          ; set flag
         bra.s    SF2006

         ; Copy the list names to a string, separated by a space or by
         ; the specified pad character.

6$:      moveq    #BLANK,d0            ; default pad
         moveq    #3,d1                ; third argument
         movea.l  (ARG2+4)(a5),a1      ; pad argument
         bsr      LoadPad              ; D0=pad character

         movea.l  a4,a6                ; SUPP=>A4, REXX=>A6
         move.l   d2,d1                ; forbid/disable flag
         CALLSYS  ListNames            ; D0=A0=argstring
         bne.s    SF2006               ; all OK

         moveq    #ERR10_003,d6        ; ... allocation failure
         bra.s    SF2006

SF2005   moveq    #ERR10_018,d6        ; "invalid operand"

         ; Release any temporary nodes ...

SF2006   movea.l  a0,a2                ; save return
         movea.l  sp,a0                ; list header
         movea.l  a4,a6                ; REXX=>A6
         CALLSYS  RemRsrcList          ; release nodes

         movea.l  a2,a0                ; return string
         lea      STACKBF(sp),sp       ; restore stack
         movea.l  (sp)+,a6
         rts

* =========================     WalkDOSList     ========================
* Walks the DOS DeviceList and builds a list of node names.
* Registers:   A0 -- list header
*              D1 -- node type
STACKBF  SET      256
WalkDOSList
         movem.l  d2/d3/d5/a0-a2/a6,-(sp)
         link     a5,#-STACKBF         ; stack buffer
         movea.l  a0,a2                ; list header
         move.l   d1,d2                ; node type

         movea.l  d4,a6                ; EXEC Base
         CALLSYS  Forbid               ; (registers preserved)

         ; Find the DOS Device List.

         movea.l  a4,a6                ; REXX Base
         movea.l  rl_DOSBase(a6),a1    ; DOS Base
         movea.l  dl_Root(a1),a1       ; Root Node
         movea.l  rn_Info(a1),a1       ; BPTR to Info
         adda.l   a1,a1
         adda.l   a1,a1                ; real address
         move.l   di_DevInfo(a1),d3    ; first node
         bra.s    4$                   ; jump in

         ; Look for nodes of the specified type.

1$:      movea.l  d3,a0                ; current node
         move.l   (a0),d3              ; next node
         cmp.l    dl_Type(a0),d2       ; correct type?
         bne.s    4$                   ; no
         move.l   a0,d5                ; save node

         ; Copy the device name (in uppercase) and null-terminate it.

         move.l   dl_Name(a0),d0       ; BSTR to name
         lsl.l    #2,d0                ; real address
         movea.l  d0,a0
         moveq    #0,d1                ; clear
         move.b   (a0)+,d1             ; load length
         movea.l  sp,a1                ; stack buffer
         bra.s    3$                   ; jump in

2$       move.b   (a0)+,d0             ; load character
         CALLSYS  ToUpper              ; D0=uppercase
         move.b   d0,(a1)+             ; install it
3$:      dbf      d1,2$                ; loop back
         clr.b    (a1)

         ; Create a resource node for the device name and node.

         moveq    #rr_SIZEOF,d0        ; node size
         movea.l  a2,a0                ; header
         movea.l  sp,a1                ; node name
         CALLSYS  AddRsrcNode          ; D0=A0=node
         beq.s    4$                   ; failure??
         move.l   d5,rr_Arg1(a0)       ; save node address

4$:      lsl.l    #2,d3                ; next node?
         bne.s    1$                   ; yes

         movea.l  d4,a6                ; EXEC Base
         CALLSYS  Permit               ; reenable switching
         unlk     a5                   ; restore stack
         movem.l  (sp)+,d2/d3/d5/a0-a2/a6
         rts
