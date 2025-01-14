* == library.asm =======================================================
*
* Copyright (c) 1986-1990 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* Library initialization routines.

* ===========================      LibInit     =========================
* This routine is called after the library has been loaded.  The library
* pointer is returned if no errors occur.
* Registers:      A0 -- DOS seglist
*                 A6 -- EXEC base
*                 D0 -- the library pointer
* Return:         D0 -- library base or 0
STACKBF  SET      64
LibInit
         move.l   a5,-(sp)
         movea.l  d0,a5                ; save library base

         move.l   a6,rl_SysBase(a5)    ; save EXEC base
         move.l   a0,rl_SegList(a5)    ; save library seglist

         ; Install various initialization constants.

         move.w   #RXSCHUNK,(rl_Chunk+2)(a5)
         move.w   #RXSNEST,(rl_MaxNest+2)(a5)

         ; Initialize string pointers ... avoid relocations.

         lea      rxsName(pc),a0      ; library name
         move.l   a0,LN_NAME(a5)
         lea      rxsID(pc),a0        ; ID string
         move.l   a0,LIB_IDSTRING(a5)
         lea      rxsTName(pc),a0      ; name for tasks
         move.l   a0,rl_TaskName(a5)
         lea      rxsPATH(pc),a0       ; REXX directory name
         move.l   a0,rl_RexxDir(a5)    ; ... install it
         lea      rxNotice(pc),a0      ; copyright notice
         move.l   a0,rl_Notice(a5)
         lea      CTable(pc),a0        ; attribute table
         move.l   a0,rl_CTABLE(a5)

         lea      rxSegList(pc),a0     ; startup code
         move.l   a0,d0
         lsr.l    #2,d0                ; convert to BPTR
         move.l   d0,rl_TaskSeg(a5)    ; install it
         move.w   #RXSSTACK,(rl_StackSize+2)(a5)

         ; Open the required libraries.

         lea      IEEELib(pc),a1       ; IEEE DP library
         moveq    #0,d0                ; any version
         CALLSYS  OpenLibrary
         move.l   d0,rl_IeeeDPBase(a5) ; opened?
         beq.s    3$                   ; no

         lea      DosLib(pc),a1        ; DOS library
         moveq    #0,d0                ; any version
         CALLSYS  OpenLibrary
         move.l   d0,rl_DOSBase(a5)    ; "guaranteed"

         ; Install pointers to the static strings.

         lea      gnNULL(pc),a0        ; start of array
         lea      rl_NULL(a5),a1       ; destination slot
         moveq    #NSMINSIZE,d0        ; structure size
         moveq    #(rl_STDERR-rl_NULL+4)/4-1,d1

1$:      move.l   a0,(a1)+             ; install string
         adda.w   d0,a0                ; advance pointer
         dbf      d1,1$                ; loop back

         ; Initialize the array of list headers ...

         lea      rl_TaskList(a5),a0   ; first list header
         moveq    #5-1,d1              ; loop count

2$:      bsr.s    InitList             ; A0=list header (D1 preserved)
         lea      (LH_SIZE+2)(a0),a0   ; advance pointer
         dbf      d1,2$                ; loop back

         ; Create a fake (NIL:) filehandle ... cleared to zero.

         exg      a5,a6                ; EXEC=>A5, REXX=>A6
         moveq    #fh_SIZEOF,d0        ; FileHandle size
         suba.l   a0,a0                ; no string
         CALLSYS  CreateArgstring      ; D0=A0=argstring
         lsr.l    #2,d0                ; make a BPTR
         move.l   d0,rl_NIL(a6)        ; install it

         ; Create the hardware configuration string.

         link     a2,#-STACKBF         ; stack buffer
         movea.l  sp,a0                ; buffer area
         bsr      SysConfig            ; D0=length A0=buffer
         CALLSYS  CreateArgstring      ; D0=A0=argstring
         unlk     a2                   ; restore stack
         move.l   d0,rl_Version(a6)    ; install version
         exg      a5,a6                ; REXX=>A5, EXEC=>A6

         ; Successful initialization ... return library base.

         move.l   a5,d0                ; load library base

3$:      movea.l  (sp)+,a5
         rts

* ==========================     InitList     ==========================
* Initializes an EXEC list header.  (All registers preserved.)
* Registers:      A0 -- pointer to list header
InitList
         move.l   a0,(a0)+
         clr.l    (a0)                 ; LH_TAIL
         addq.l   #LH_TAIL,-(a0)       ; pointer to tail
         move.l   a0,LH_TAILPRED(a0)
         rts

* ============================      Open     ===========================
* Increment the open count and return the library pointer.
* Registers:   A6 -- library base
* Return:      D0 -- library base
LibOpen
         addq.w   #1,LIB_OPENCNT(a6)   ; increment open count
         moveq    #LIBB_DELEXP,d1      ; 'delayed expunge' flag
         bclr     d1,LIB_FLAGS(a6)     ; clear flag
         move.l   a6,d0                ; return library base
         rts

* ===========================      Close     ===========================
* Decrements the open count and check whether the library is still open.
* If not, and if the 'delayed expunge' flag is set, it is expunged.
* Registers:      A6 -- library base
* Return:         D0 -- seglist or 0
LibClose
         subq.w   #1,LIB_OPENCNT(a6)   ; decrement open count
         bne.s    Null                 ; still open ...

         ; Check for a delayed expunge.

         btst     #LIBB_DELEXP,LIB_FLAGS(a6)
         beq.s    Null                 ; ... no expunge

* ==========================     LibExpunge     ========================
* If the library is still open, the 'delayed expunge' flag is set, but
* the library is left in memory.  Otherwise, the library is unlinked and
* released, and the seglist is returned in D0.
* Registers:      A6 -- library base
* Return:         D0 -- seglist or 0
LibExpunge
         bset     #LIBB_DELEXP,LIB_FLAGS(a6)
         tst.w    LIB_OPENCNT(a6)      ; library still open?
         bne.s    Null                 ; yes

         ; Library not open ... proceed with expunge.

         movea.l  rl_Version(a6),a0    ; version string (may be null)
         CALLSYS  DeleteArgstring      ; release it

         ; Release the fake NIL: filehandle (allocated as an argstring).

         movea.l  rl_NIL(a6),a0        ; BPTR to (fake) FileHandle
         adda.l   a0,a0
         adda.l   a0,a0                ; real address
         CALLSYS  DeleteArgstring      ; release it

         move.l   rl_SegList(a6),d0    ; get the seglist
         movem.l  d0/a5/a6,-(sp)
         movea.l  a6,a5                ; REXX base => A5
         movea.l  rl_SysBase(a5),a6    ; EXEC base => A6
         movea.l  a5,a1                ; library node
         CALLSYS  Remove               ; unlink us

         ; Close the external libraries that were opened.

         movea.l  rl_DOSBase(a5),a1    ; DOS library
         CALLSYS  CloseLibrary         ; close it

         movea.l  rl_IeeeDPBase(a5),a1 ; IEEE library
         CALLSYS  CloseLibrary         ; close it
   
         ; Calculate the size of the library node and release it.

         movea.l  a5,a1                ; node
         movem.w  LIB_NEGSIZE(a5),d0/d1; load NEGSIZE/POSSIZE
         suba.w   d0,a1                ; low memory extent
         add.w    d1,d0                ; total size
         CALLSYS  FreeMem              ; release it

         movem.l  (sp)+,d0/a5/a6       ; seglist=>D0
         rts

Null
LibReserved                            ; reserved entry
         moveq    #0,d0                ; clear return
         rts

* =========================      InitPort     ==========================
* Initializes a previously allocated message port.  The calling task is
* assumed to be the one to be signalled.  The supplied name is installed
* in the structure, but port is not linked into the system Ports List.
* Registers:   A0 -- message port
*              A1 -- port name
* Return:      D0 -- signal bit
*              A1 -- message port
InitPort
         move.l   a6,-(sp)
         move.l   a0,-(sp)             ; push port

         addq.w   #LN_TYPE,a0          ; advance pointer
         move.w   #NT_MSGPORT<<8,(a0)+ ; LN_TYPE/LN_PRI
         move.l   a1,(a0)+             ; LN_NAME port name
         clr.b    (a0)                 ; MP_FLAGS (PA_SIGNAL)

         ; Initialize the message list header.

         addq.w   #(MP_MSGLIST-MP_FLAGS),a0
         CALLSYS  InitList             ; A0=header

         ; Allocate a signal bit for the port.

         moveq    #-1,d0               ; any bit
         movea.l  rl_SysBase(a6),a6    ; EXEC base
         CALLSYS  AllocSignal          ; D0=signal bit or -1

         movea.l  (sp)+,a1             ; pop port
         move.l   ThisTask(a6),MP_SIGTASK(a1)
         move.b   d0,MP_SIGBIT(a1)     ; install signal bit

         movea.l  (sp)+,a6
         rts

* ===========================     FreePort     =========================
* Releases the signal bit associated with a message port.  The memory for
* the port structure is not deallocated.
* Registers:      A0 -- port
FreePort
         move.l   a6,-(sp)
         movea.l  rl_SysBase(a6),a6

         ; Mark the port as "closed".

         moveq    #-1,d1
         move.b   d1,LN_TYPE(a0)
         move.l   d1,(MP_MSGLIST+LH_HEAD)(a0)

         moveq    #0,d0
         move.b   MP_SIGBIT(a0),d0     ; signal bit
         CALLSYS  FreeSignal           ; deallocate it

         movea.l  (sp)+,a6
         rts

* =========================     SetPriority     ========================
* Changes this task's priority and returns the old priority.
* Registers:      D0 -- new priority [0:7]
* Return:         D0 -- old priority
SetPriority
         move.l   a6,-(sp)
         movea.l  rl_SysBase(a6),a6
         movea.l  ThisTask(a6),a1      ; our task
         CALLSYS  SetTaskPri           ; D0=old priority
         ext.w    d0                   ; extend to word
         ext.l    d0                   ; extend long
         movea.l  (sp)+,a6
         rts

* ==========================     SysConfig     =========================
* Builds the system hardware configuration string.
* Registers:   A0 -- buffer area
* Return:      D0 -- length of string
*              A0 -- buffer
SysConfig
         movem.l  a2/a5,-(sp)
         movea.l  a0,a2                ; save buffer
         movea.l  rl_SysBase(a6),a5    ; EXECbase

         ; Find the graphics library ...

         exg      a5,a6                ; REXX=>A5 , EXEC=>A6
         lea      LibList(a6),a0       ; library list
         lea      GfxLib(pc),a1        ; graphics library
         CALLSYS  FindName             ; D0=library node
         exg      a5,a6

         ; Determine the vertical blanking frequency

         moveq    #0,d1
         move.b   VBlankFrequency(a5),d1
         move.w   d1,-(sp)             ; push frequency

         ; Determine the video display mode ...

         pea      dispNTSC(pc)         ; default "NTSC"
         tst.l    d0                   ; library found?
         beq.s    1$                   ; no
         movea.l  d0,a0                ; Gfxbase
         moveq    #1<<2,d0             ; PAL bit mask
         and.w    gb_DisplayFlags(a0),d0
         beq.s    1$                   ; ... not PAL
         addq.l   #ADVDISP,(sp)        ; advance pointer

         ; Check whether a math chip is installed ...

1$:      move.w   AttnFlags(a5),d0     ; load AttnFlags
         pea      mpuNONE(pc)          ; push 'NONE'
         btst     #AFB_68881,d0        ; math chip?
         beq.s    2$                   ; no
         addq.l   #ADVMPU,(sp)         ; advance pointer

         ; Determine the CPU type ... 68000-68040

2$:      moveq    #AFB_68010-1,d1      ; flag bit

3$:      addq.w   #1,d1                ; bump flag
         btst     d1,d0                ; bit set?
         bne.s    3$                   ; yes
         move.w   d1,-(sp)             ; push count

         lea      VersFmt(pc),a0       ; format string
         movea.l  sp,a1                ; data stream
         move.l   a2,d0                ; buffer
         bsr.s    FormatString         ; D0=length A0=buffer
         lea      (2+4+4+2)(sp),sp     ; restore stack

         movem.l  (sp)+,a2/a5
         rts

* =========================     FormatString     =======================
* Formats a data stream according to the supplied format string.
* Registers:   A0 -- format
*              A1 -- data stream
*              D0 -- buffer area
* Return:         D0 -- length
*                 D1 -- length
*                 A0 -- buffer
FormatString
         movem.l  d0/a2/a3/a6,-(sp)
         lea      InsertChar(pc),a2    ; character function
         movea.l  d0,a3                ; buffer area
         movea.l  rl_SysBase(a6),a6    ; EXEC base
         CALLSYS  RawDoFmt
         movem.l  (sp)+,a0/a2/a3/a6

         CALLSYS  Strlen               ; D0=D1=length (A0 preserved)
         rts

InsertChar
         move.b   d0,(a3)+             ; install character
         clr.b    (a3)                 ; null byte
         rts

         ; Constant strings

DosLib   dc.b     'dos.library',0
IEEELib  dc.b     'mathieeedoubbas.library',0
GfxLib   dc.b     'graphics.library',0
rxsName  RXSLIBNAME                    ; the REXX system library
rxsID    RXSLIBID                      ; the ID string
rxsTName RXSTNAME                      ; task names
rxsPATH  dc.b     'REXX:',0            ; default path

         ; The copyright notice string

rxNotice dc.b     'ARexx Version '
         VERSIONNUM
         VERSIONBETA
         dc.b     10,'Copyright ',$A9,' 1987 by William S. Hawes',10
         dc.b     'All Rights Reserved',10,0

VersFmt  dc.b     'ARexx V'
         VERSIONNUM
         dc.b     ' 680%d0 %s %s %dHZ',0

mpuNONE  dc.b     'NONE',0
mpu68881 dc.b     '68881',0
ADVMPU   EQU      mpu68881-mpuNONE

dispNTSC dc.b     'NTSC',0
dispPAL  dc.b     'PAL',0
ADVDISP  EQU      dispPAL-dispNTSC

QMark    dc.b     '?',0                ; question mark
         CNOP     0,2
