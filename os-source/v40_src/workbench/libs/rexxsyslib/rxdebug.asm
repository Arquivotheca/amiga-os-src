* == rxdebug.asm =======================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ==========================      rxDebug     ==========================
* Prompts for interactive (debugging) input.  If an input line is entered,
* a new source segment is created and installed.  Private to 'rxParse'.
* Registers:   A3 -- global pointer
*              A4 -- environment
* Return:      D6 -- error code
* Scratch:     D2/D3/D5/A5
rxDebug
         tst.l    ev_TraceCnt(a4)      ; trace inhibited?
         beq.s    1$                   ; no
         bmi.s    3$                   ; ... suppress trace
         subq.l   #1,ev_TraceCnt(a4)   ; decrement count
         bra.s    3$

         ; Find the trace output stream.

1$:      movea.l  a3,a0                ; global pointer
         bsr      FindTrace            ; D0=A0=IoBuff
         beq.s    3$                   ; ... no stream

         ; Read a string (uses the global temporary buffer.)

         movea.l  LN_NAME(a0),a1       ; logical name string
         subq.w   #ns_Buff,a1          ; string structure
         lea      TRMsgp(pc),a0        ; prompt string
         move.l   a0,d0
         move.l   a3,a0                ; global pointer
         bsr      PullString           ; D0=length A1=buffer
         ble.s    3$                   ; ... nothing entered

         move.l   d0,d1                ; save length
         move.l   SRCPOS(a4),d2        ; starting position of clause
         subq.w   #1,d0                ; just one character?
         bne.s    2$                   ; no
         cmpi.b   #'=',(a1)            ; an '=' sign?
         bne.s    2$                   ; no

         ; Clause to be re-executed.  Update PC and set 'MOVE' flag.

         move.l   d2,ev_PC(a4)         ; new program counter
         bset     #(EXB_MOVE-8),(STATUS+3)(a4)
         bra.s    3$

         ; Interactive source entered.  Scan it and install the segment.

2$:      movea.l  a4,a0                ; environment
         moveq    #NSF_STRING,d0       ; string type
         bsr      AddString            ; D0=A0=string
         move.l   a0,d3                ; save string

         ; Call the "string file" parser.

         movea.l  a0,a1                ; source string
         movea.l  a4,a0                ; environment
         move.l   d2,d0                ; source position
         moveq    #-1,d1               ; set 'DEBUG' mode
         bsr.s    ParseString          ; D0=error code
         move.l   d0,d6                ; save error

         ; Release the source string.

         movea.l  a4,a0                ; environment
         movea.l  d3,a1                ; source string
         bsr      FreeString           ; release it

3$:      tst.l    d6                   ; set CCR
         rts

* ==========================     ParseString     =======================
* Scans a string to create a segment array, and installs the new segment.
* Registers:   A0 -- environment
*              A1 -- string structure
*              D0 -- source position
*              D1 -- boolean 'debug mode' flag
* Return:      D0 -- error code
ParseString
         movem.l  d2/a3-a5,-(sp)
         movea.l  a0,a4                ; environment
         movea.l  a1,a5                ; source string
         movea.l  ev_GlobalPtr(a4),a3  ; global pointer
         move.w   d1,d2                ; mode flag

         ; Allocate an 'INTERPRET' range.

         movea.l  a4,a0                ; environment
         moveq    #NRANGE_INTERP,d1    ; range type
         bsr      GetRange             ; D0=A0=range
         move.b   #NRFF_ACTIVE,r_Flags(a0)

         ; Save current segment and PC and clear the new PC.

         move.l   ev_Segment(a4),r_Segment(a0)  ; save old segment
         move.l   ev_PC(a4),r_NextPos(a0)       ; save current PC

         clr.l    ev_Segment(a4)       ; clear segment
         clr.l    ev_PC(a4)            ; clear PC
         bset     #(EXB_MOVE-8),(STATUS+3)(a4)
*         bsr      NewPosition          ; set 'MOVE' (A0 preserved)

         ; Check if we're entering 'DEBUG' mode.

         tst.w    d2                   ; 'DEBUG' mode?
         beq.s    1$                   ; no
         bset     #NRFB_DEBUG,r_Flags(a0)
         bset     #(EXB_DEBUG-16),(STATUS+1)(a4)

         ; Open a file buffer and get the IoBuff pointer

1$:      lea      FILELIST(a3),a0      ; global File List
         lea      ns_Buff(a5),a1       ; source string
         moveq    #RXIO_STRF,d0        ; "string file" mode
         moveq    #0,d1                ; no logical name
         CALLSYS  OpenF                ; D0=IoBuff
         beq.s    2$                   ; error ...

         ; Scan the input source and build the segment array.

         movea.l  a3,a0                ; (base) environment
         movea.l  d0,a1                ; IoBuff node
         moveq    #0,d0                ; no Lines/Labels Arrays
         bsr      ReadSource           ; D0=error A0=segment array
         bne.s    3$                   ; error ...
         move.l   a0,ev_Segment(a4)    ; install new segment
         bra.s    3$

2$:      moveq    #ERR10_003,d0        ; open failure??

3$:      movem.l  (sp)+,d2/a3-a5
         rts

         ; String constants

TRMsgp   dc.b     '>+> ',0             ; prompt string
         CNOP     0,2
