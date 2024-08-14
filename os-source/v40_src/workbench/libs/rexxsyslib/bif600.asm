* == bif600.asm ========================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ==========================     PRAGMA     ============================
* Sets options relating to the external system.
* Usage: oldvalue = PRAGMA(option,[value])
BIFpragma
         movea.l  ev_GlobalPtr(a4),a2  ; global pointer
         move.l   (rt_MsgPort+MP_SIGTASK)(a2),d2

         move.b   (a0),d0              ; option
         CALLSYS  ToUpper              ; D0=UPPERcase
         subi.b   #'*',d0              ; 'Star'?
         beq.s    BF6_Star             ; yes
         subi.b   #'D'-'*',d0          ; 'Directory'?
         beq.s    BF6_Dir              ; yes
         subq.b   #'I'-'D',d0          ; 'Id'?
         beq      BF6_Id               ; yes
         subq.b   #'P'-'I',d0          ; 'Priority'?
         beq      BF6_Priority         ; yes
         subq.b   #'S'-'P',d0          ; 'Stack'?
         beq      BF6_Stack            ; yes
         subq.b   #'W'-'S',d0          ; 'Window'?
         bne      BFErr18              ; no

         ; PRAGMA('W',option) ... set the WindowPointer.

         tst.l    d1                   ; an argument?
         beq.s    1$                   ; no
         move.b   (a1),d0              ; test character
         CALLSYS  ToUpper              ; D0=uppercase

         move.l   rt_ClientID(a2),d1   ; a client?
         beq.s    1$                   ; no
         movea.l  d1,a1                ; client task
         move.l   pr_WindowPtr(a1),d1  ; client's value
1$:      subi.b   #'C',d0              ; 'Client'?
         beq.s    2$                   ; yes
         moveq    #-1,d1               ; 'No requestors'
         subi.b   #'N'-'C',d0          ; 'None'?
         beq.s    2$                   ; yes
         moveq    #0,d1                ; ... use default

2$:      movea.l  d2,a1                ; task ID
         move.l   d1,pr_WindowPtr(a1)  ; install pointer
         moveq    #-1,d3               ; success

BF6001   bra      BFBool               ; boolean return

         ; PRAGMA("*",name) ... install current console handler.

BF6_Star tst.w    d1                   ; name specified?
         beq.s    1$                   ; no

         ; Look for the specified name ...

         movea.l  a2,a0                ; global pointer
         subq.w   #ns_Buff,a1          ; logical name
         bsr      FindConsoleHandler   ; D0=processid
         bra.s    2$

         ; No name specified ... get the client task's console handler.

1$:      move.l   rt_ClientID(a2),d0   ; a client?
         beq.s    2$                   ; no
         movea.l  d0,a0                ; client task
         move.l   pr_ConsoleTask(a0),d0; client's console handler

2$:      movea.l  d2,a1                ; task ID
         move.l   d0,pr_ConsoleTask(a1); ... install it
         sne      D3                   ; set flag
         bra.s    BF6001               ; boolean return

         ; PRAGMA('D',[directory]) ... sets the current directory and 
         ; returns the old directory name.

BF6_Dir  movea.l  a1,a0                ; directory name
         tst.w    d1                   ; name specified?
         bne.s    1$                   ; yes
         movea.l  a3,a0                ; buffer area
         clr.b    (a3)                 ; ... null string

1$:      moveq    #RXIO_READ,d0        ; shared lock
         bsr      LockF                ; D0=lock
         beq.s    2$                   ; ... error

         IFLT     TEMPBUFF-fib_SIZEOF
         FAIL     "Temporary buffer smaller than FileInfoBlock!"
         ENDC

         ; Make sure it's a directory ...

         move.l   d0,d2                ; save lock
         movea.l  a3,a1                ; FileInfoBlock
         bsr      DOSExamine           ; D0=boolean
         exg      d0,d2                ; lock=>D0
         beq.s    2$                   ; ... failure
         tst.l    fib_DirEntryType(a3) ; a directory?
         bmi.s    2$                   ; no ... error

         ; Update the current directory.

         bsr      SetDirectory         ; D0=old lock

         ; Get the pathname of the old directory ...

         move.l   d0,d3                ; save lock
         movea.l  a3,a0                ; buffer
         moveq    #(MAXFNLEN*2)-1,d0   ; maximum length
         move.l   d3,d1                ; lock
         bsr      PathName             ; D0=length
         exg      d0,d3                ; lock=>D0, length=>D3

         ; ... and release the old lock.

2$:      bsr      UnlockF              ; release it
         bra.s    BIF601               ; new string

         ; PRAGMA('Id') ... return this task ID

BF6_Id   cmpi.b   #1,d7                ; one argument?
         bne      BFErr18              ; no ... error
         movea.l  a3,a0                ; buffer
         lea      (rt_MsgPort+MP_SIGTASK)(a2),a1
         moveq    #-1,d0               ; hex conversion
         moveq    #4,d1                ; length
         CALLSYS  CVc2x                ; D0=error D1=length
         move.w   d1,d3                ; save length
         bra.s    BIF601               ; new string

         ; PRAGMA('P',priority) ... set the task priority.

BF6_Priority
         lea      SetPriority(pc),a2   ; function address
         bra.s    BF6002

         ; PRAGMA('Stack',stacksize) ... set the CLI stack.

BF6_Stack
         lea      SetStack(pc),a2      ; function address

BF6002   lea      -ns_Buff(a1),a0      ; argument
         bsr      CVs2i                ; D0=error D1=value
         bne.s    BF6003               ; ... error

         move.l   d1,d0                ; new value
         jsr      (a2)                 ; D0=old value
         bra.s    BIF602               ; numeric return

BF6003   move.l   d0,d6                ; error code
         rts

* =========================     GETSPACE     ===========================
* Internal space allocator.  Allocates a block and returns its address.
* Usage: GETSPACE([size])
BIFgetspace
         move.l   gn_TotAlloc(a2),d0   ; total allocated
         tst.b    d7                   ; any arguments?
         beq.s    BIF602               ; no ...

         ; Get a block of memory from the internal allocator.

         movea.l  a4,a0                ; environment
         move.l   d3,d0                ; length
         beq.s    BIF603               ; ... error
         CALLSYS  GetSpace             ; D0=A0=block
         beq      BFErr03              ; error ...

         move.l   d0,(a3)              ; memory block
         moveq    #4,d3                ; 4-byte address

BIF601   bra      BFNewstr             ; 'new string' return

* =========================     FREESPACE     ==========================
* Releases a block of internal memory and returns the total free space.
* Usage: FREESPACE([address,size])
BIFfreespce
         tst.b    d7                   ; any arguments?
         beq.s    1$                   ; no ... return space

         subq.w   #4,d0                ; 4-byte address?
         bne.s    BIF603               ; ... invalid length
         move.l   d3,d0                ; block length
         beq.s    BIF603               ; zero??

         movea.l  (a0),a1              ; load address
         movea.l  a4,a0
         CALLSYS  FreeSpace            ; D0=boolean

1$:      move.l   gn_TotFree(a2),d0    ; internal free space

BIF602   move.l   d0,d3                ; integer
         bra      BFNumber             ; numeric return

BIF603   bra      BFErr18              ; invalid operand

* ===========================     IMPORT     ===========================
* Imports a string from the specified address.
* Usage: length = IMPORT(address,[length])
BIFimport
         subq.w   #4,d0                ; 4-byte address?
         bne.s    BIF603               ; no -- error

         movea.l  (a0),a3              ; get source address
         tst.w    d1                   ; length given?
         bne.s    BIF601               ; yes

         ; No length specified ... look for a null byte.

         movea.l  a3,a0                ; string pointer
         CALLSYS  Strlen               ; D0=length
         move.l   d0,d3                ; string length
         bra.s    BIF601               ; ... new string

* ==========================     STORAGE     ===========================
* Usage: STORAGE([address],[string],[length],[pad])
BIFstorage
         moveq    #-1,d4               ; save flag
         tst.b    d7                   ; any arguments?
         bne.s    BIF604               ; yes

         ; No arguments given -- compute available memory

         move.l   rl_SysBase(a6),d5    ; EXEC base
         moveq    #MEMQUICK,d1         ; public memory
         exg      d5,a6
         CALLSYS  AvailMem             ; D0=available memory
         exg      d5,a6
         bra.s    BIF602               ; numeric return

* ===========================     EXPORT     ===========================
* Copies a string or pad data to an external address.
* Usage: EXPORT(address,[string],[length],[pad])
BIFexport
         moveq    #0,d4                ; no save
         bset     #NUMBER,d7           ; numeric return

BIF604   exg      a1,a2                ; save string
         exg      d1,d2                ; ... and length
         cmpi.b   #4,d7                ; pad argument?
         blt.s    1$                   ; no ...
         tst.l    12(a5)               ; argument given?
         bne.s    2$                   ; yes

1$:      moveq    #0,d5                ; default pad

2$:      tst.w    d1                   ; length argument?
         bne.s    3$                   ; yes ...
         move.w   d2,d3                ; ... use string length

3$:      cmp.l    MaxLen(pc),d3        ; too long?
         bgt      BFErr09              ; yes

         subq.w   #4,d0                ; 4-byte address?
         bne.s    BIF603               ; no ... error
         movea.l  (a0),a3              ; destination address

         tst.w    d4                   ; save current?
         beq.s    BIF605               ; no

         ; 'STORAGE' function ... save the current value before copying.

         movea.l  a4,a0                ; environment
         movea.l  a3,a1                ; buffer area
         moveq    #NSF_STRING,d0       ; string flags
         move.l   d3,d1                ; string length
         bsr      AddString            ; D0=A0=string

         ; Disable task switching ...

BIF605   bsr      Forbid               ; (registers preserved)
         move.l   d3,d1                ; total length
         bra.s    3$                   ; jump in

1$:      move.b   d5,d0                ; pad character
         subq.l   #1,d2                ; used up?
         bmi.s    2$                   ; yes
         move.b   (a2)+,d0             ; load character
2$:      move.b   d0,(a3)+             ; install it
3$:      subq.l   #1,d1                ; done?
         bcc.s    1$                   ; ... loop back

         ; Reenable task switching 

         bra      Permit               ; (registers preserved)
