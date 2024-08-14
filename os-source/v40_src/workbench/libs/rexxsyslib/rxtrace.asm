* == rxtrace.asm =======================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ===========================     rxTrace     ==========================
* Tracing module for REXX interpreter.  Displays a clause or result string
* on the (trace) console.
* Registers:   A0 -- environment
*              A1 -- trace object (clause or string)
*              D0 -- error code
*              D1 -- action code
TRBUFLEN SET      120                  ; maximum total length
TRMAXLEN SET      54                   ; maximum length of string
TRDOTLEN SET      4                    ; ellipsis length
STACKBF  SET      TRBUFLEN+TRMAXLEN+TRDOTLEN
rxTrace
         tst.l    ev_TraceCnt(a0)      ; trace inhibited?
         bpl.s    TRSave               ; no
         rts                           ; ... quick exit

TRSave   movem.l  d2-d6/a2-a5,-(sp)    ; delayed save
         movea.l  a0,a4                ; environment
         movea.l  sp,a5                ; initial stack
         lea      -STACKBF(sp),sp      ; stack buffer

         moveq    #0,d3                ; clear indentation
         moveq    #' ',d4              ; load space
         move.w   d1,d5                ; trace action code
         move.l   d0,d6                ; error code

         ; Determine the specific trace action.

         movea.l  sp,a2                ; buffer pointer
         subq.w   #ACT_SYNTAX,d1       ; syntax (execution) error?
         bne.s    1$                   ; no

         ; 'SYNTAX' action ... display error code, line number, and message.

         move.l   d6,d0                ; error code
         CALLSYS  ErrorMsg             ; A0=error message
         pea      ns_Buff(a0)          ; push message
         move.l   ev_SrcLine(a4),-(sp) ; push line number
         move.l   d6,-(sp)             ; push error code
         lea      ErrorFmt(pc),a0      ; format string
         bra      TRFormat

         ; A host command error?

1$:      subq.w   #ACT_ERROR-ACT_SYNTAX,d1
         bne.s    2$

         ; 'ERROR' action ... display command return code.

         move.l   d6,-(sp)             ; push error
         lea      CommFmt(pc),a0       ; command string
         bra      TRFormat

         ; 'LINE' or 'RESULT' action ... compute the total indentation
         ; based on the nesting level.

2$:      add.w    (ev_RStack+skNum)(a4),d3
         add.w    ev_Level(a4),d3      ; plus function level
         add.w    d3,d3                ; multiply by 2
         add.w    ev_Indent(a4),d3     ; ... line number indentation

         ; In 'DEBUG' mode? ... suppress the tracing action.

         btst     #(EXB_DEBUG-16),(STATUS+1)(a4)
         bne      TROut                ; ... debugging

         ; List the clause?

         subq.w   #ACT_LINE-ACT_ERROR,d1
         bne.s    3$

         ; 'LINE' action ... list the source clause.

         move.l   SRCPOS(a4),d0        ; source position
         move.l   ev_PC(a4),d2         ; next clause
         sub.l    d0,d2                ; clause length
         bsr      SrcSegment           ; D0=segment  D1=offset
         lsl.w    #2,d0                ; scale for 4 bytes
         movea.l  ev_Segment(a4),a1    ; segment array
         movea.l  sSeg(a1,d0.l),a1     ; source segment
         lea      ns_Buff(a1,d1.l),a1  ; ... start of clause

         lea      LineFmt(pc),a0       ; line format
         move.l   ev_SrcLine(a4),d4    ; line number
         bra.s    4$

         ; 'RESULT' or intermediate action ... display result string.

3$:      lea      ResFmt(pc),a0        ; results format
         moveq    #0,d2                ; default length
         move.l   a1,d1                ; a result string?
         beq.s    4$                   ; no ...
         move.w   ns_Length(a1),d2     ; string length
         addq.w   #ns_Buff,a1          ; result string

         ; Copy the clause or string to the top of the stack buffer,
         ; and append an ellipsis if the line is too long.

4$:      movea.l  a5,a3                ; top of buffer
         clr.l    -(a3)                ; null bytes
         moveq    #TRMAXLEN,d1         ; maximum length
         cmp.l    d1,d2                ; line too long?
         bls.s    5$                   ; no
         move.l   d1,d2                ; truncate length
         move.l   #'...'<<8,(a3)       ; install ellipsis

         ; Copy the string, converting troublesome characters to blanks.

5$:      adda.w   d2,a1                ; end of string
         movea.l  rl_CTABLE(a6),a2     ; attribute table
         bra.s    8$                   ; jump in

6$:      move.b   -(a1),d1             ; load character
         cmpi.b   #' ',d1              ; a space?
         beq.s    7$                   ; yes
         btst     #CTB_SPACE,0(a2,d1.w); white space?
         beq.s    7$                   ; no
         moveq    #'?',d1              ; substitute a '?'
7$:      move.b   d1,-(a3)             ; copy string
8$:      dbf      d2,6$                ; loop back

         movea.l  sp,a2                ; buffer area
         move.l   a3,-(sp)             ; push string

         ; Push the trace code and line number (or space character).

TRFormat lsl.w    #2,d5                ; scale for four bytes
         pea      (TRCode-4)(pc,d5.w)  ; push code sequence
         move.l   d4,-(sp)             ; line number or space

         ; Format the data stream ... format string is in A0.

         movea.l  sp,a1                ; data stream
         move.l   a2,d0                ; buffer area
         bsr      FormatString         ; D0=D1=length A0=buffer
         move.l   d0,d2                ; save length

         ; Compute the final indentation count and adjust the line length.

         moveq    #MAXSHIFT,d0         ; maximum indent
         sub.w    d3,d0                ; too deep?
         bcc.s    1$                   ; no
         moveq    #0,d0                ; ... maximum indentation

1$:      sub.w    d0,d2                ; decrement length (always > 0)
         adda.w   d0,a2                ; advance start

         ; Find the trace output stream ...

         movea.l  ev_GlobalPtr(a4),a0  ; global pointer
         bsr      FindTrace            ; D0=A0=IoBuff
         beq.s    TROut                ; ... no stream

         ; ... and write the string to the trace stream.

         movea.l  a2,a1                ; start of string
         move.l   d2,d0                ; length
         CALLSYS  WriteF               ; write it

TROut    movea.l  a5,sp                ; restore stack
         movem.l  (sp)+,d2-d6/a2-a5
         rts

         ; String constants

* Trace code string array (4 byte entries)
TRCode
TRCode1  dc.b     '+++',0              ; 1 syntax error
TRCode2  dc.b     '+++',0              ; 2 host command error
TRCode3  dc.b     '*-*',0              ; 3 line number
TRCode4  dc.b     '>F>',0              ; 4 function result
TRCode5  dc.b     '>L>',0              ; 5 literal value
TRCode6  dc.b     '>O>',0              ; 6 dyadic operation
TRCode7  dc.b     '>P>',0              ; 7 prefix operation
TRCode8  dc.b     '>V>',0              ; 8 symbol value
TRCode9  dc.b     '>C>',0              ; 9 expanded compound variable
TRCodeA  dc.b     '>>>',0              ; 10 result
TRCodeB  dc.b     '>.>',0              ; 11 placeholder
TRCodeC  dc.b     '>>>',0              ; 12 assignment

MAXSHIFT SET      19                   ; maximum total indentation
ErrorFmt dc.b     '%19lc%s Error %ld in line %ld: %s',10,0     (max ??)
CommFmt  dc.b     '%19lc%s Command returned %ld',10,0
LineFmt  dc.b     '%19ld %s %s',10,0                           (max 80)
ResFmt   dc.b     '%22lc%s "%s"',10,0                          (max 88)
         CNOP     0,2
