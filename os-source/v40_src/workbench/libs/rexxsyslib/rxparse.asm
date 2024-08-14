* == rxparse.asm =======================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ==========================     rxParse     ===========================
* Parsing and execution control module for REXX interpreter.
* Registers:   A3 -- global pointer (preserved)
* Return:      D6 -- error code
* Internal register assignments:
*     A3 -- global pointer
*     A4 -- current environment
*     A5 -- clause
*     A6 -- REXX library base
*     A7 -- stack pointer
*     D4 -- internal machine status
*     D5 -- instruction word
*     D6 -- error code
*     D7 -- tracing option [0:15]
* All registers have been saved by the caller.

QUICKTST SET      EXF_EXTRACE!EXF_SIGNAL!EXF_PAUSE!EXF_FLUSH!EXF_MOVE
rxParse
         moveq    #0,d6                ; clear error
         bra      NextCycle            ; jump in

         ; Start a new "machine" cycle.

BeginCycle
         lea      ev_EStack(a4),a2     ; execution stack
         moveq    #TRACEMASK,d7        ; trace option mask
         and.b    d4,d7                ; trace option

         ; Check the external interface for various interrupt flags.

         movea.l  a3,a0                ; base environment
         CALLSYS  rxSuspend            ; D0=interrupt mask
         beq.s    2$                   ; ... no signals

         moveq    #INB_HALT,d1         ; signals to check
1$:      btst     d1,d0                ; flag set?
         dbne     d1,1$                ; loop back
         bne      CheckTrap            ; ... check if trapped

         ; Check for an untrapped error from the last instruction.

2$:      tst.l    d6                   ; error code?
         bne      CheckError           ; yes

         ; Test the status word and external flags ...

         move.l   d4,d0                ; status word
         move.b   rt_Flags(a3),d0      ; flag byte
         andi.l   #QUICKTST!1<<RTFB_TRACE,d0
         beq      CheckSource          ; ... no flags

         ; Check if the external trace bit is set.

         moveq    #EXB_EXTRACE,d1      ; load flag
         tst.b    d0                   ; external trace?
         beq.s    3$                   ; no ...
         bset     d1,d4                ; already set?
         bne.s    CheckSIGNAL          ; yes

         ; Entering interactive trace mode ... update tracing option.

         bset     #EXB_ACTIVE,d4       ; set 'interactive' bit
         move.w   #1<<TRACE_B!1<<TRACE_S!1<<TRACE_R!1<<TRACE_I,d0
         btst     d7,d0                ; change trace?
         bne.s    4$                   ; no

         moveq    #TRACE_R,d7          ; load 'RESULTS' mode
         andi.b   #~TRACEMASK,d4       ; clear current option
         or.b     d7,d4                ; install new option
         bra.s    4$

         ; Leaving interactive tracing?

3$:      bclr     d1,d4                ; internal trace shadow set?
         beq.s    CheckSIGNAL          ; no

         ; Leaving interactive tracing mode ... clear all trace flags.

         clr.b    d4                   ; clear trace
         clr.b    d7                   ; clear option

4$:      move.l   d4,STATUS(a4)        ; update status

         ; Processing for the 'SIGNAL' (interrupt) flag.

CheckSIGNAL
         bclr     #(EXB_SIGNAL-16),(STATUS+1)(a4)
         beq.s    CheckPAUSE           ; ... no interrupts

         ; Look for the signalled label.

         movea.l  a4,a0                ; environment
         movea.l  gn_SigLabel(a3),a1   ; signalled label
         bsr      rxLocate             ; D0=error D1=location
         move.l   d0,d6                ; label found?
         bne      CheckError           ; no
         move.l   d1,d2                ; save location

         ; Interrupt processing ... dismantle all control ranges.

1$:      movea.l  a4,a0                ; environment
         bsr      FreeRange            ; D0=boolean
         bne.s    1$                   ; success? loop back

         ; Clear selected flags, set 'MOVE', and update the PC.

         move.l   STATUS(a4),d4        ; load status
         andi.l   #~SIGMASK,d4         ; clear flags
         bset     #EXB_MOVE,d4         ; set 'MOVE'
         move.l   d4,STATUS(a4)        ; update status
         move.l   d2,ev_PC(a4)         ; new position

         ; Check if interactive (debugging) source to be entered.

CheckPAUSE
         bclr     #(EXB_PAUSE-24),(STATUS+0)(a4)
         beq.s    CheckMOVE            ; ... no pause

         bsr      rxDebug              ; D6=error code
         bne      CheckError           ; ... error
         move.l   STATUS(a4),d4        ; load status

         ; Processing for 'MOVE' flag ... flush the pipeline to prepare
         ; for a new source position.  The 'MOVE' flag is not cleared
         ; until after range processing.

CheckMOVE
         bclr     #EXB_FLUSH,d4        ; 'FLUSH' flag set?
         bne.s    1$                   ; yes
         btst     #EXB_MOVE,d4         ; 'MOVE' flag set?
         beq.s    CheckSource          ; ... not set

         ; Flush the execution pipeline ...

1$:      movea.l  a4,a0                ; environment
         bsr      FlushClause
         move.l   d4,STATUS(a4)        ; update status

         ; Check if execution pipeline is empty ... if so, get a new clause.

CheckSource
         tst.w    skNum(a2)            ; clause stacked?
         bne.s    CheckTrace           ; yes

         ; Fetch a new clause ... registers D2/D3/D5/A5 are scratch.

         bsr      ReadClause           ; fetch a clause
         tst.w    skNum(a2)            ; clause stacked?
         bne.s    CheckTrace           ; yes

         ; End of source ... check for errors.

         bsr      rxEnd                ; 'end-of-program' processing
         bra      CheckError

         ; Array of trace flag bits, indexed by trace option.

TraceTst dc.b     0,1<<C_COMMAND,0,$3E,1<<C_LABEL,$3E,0,$3E,$3E
         CNOP     0,2

         ; Update global data and check whether to trace the clause.

CheckTrace
         movea.l  STACKBOT(a2),a5      ; clause
         move.l   c_Keys(a5),d5        ; instruction word
         move.l   d5,d3
         and.l    PRIMask(pc),d3       ; primary opcode
         move.l   c_SrcPos(a5),d1
         move.l   d1,SRCPOS(a4)        ; source position
         add.w    c_Len(a5),d1         ; add length
         move.l   d1,ev_PC(a4)         ; ... update 'PC'

         ; Update the source line if the clause is from the global source.

         move.l   gn_SrcSeg(a3),d0
         sub.l    ev_Segment(a4),d0    ; global source segment?
         bne.s    1$                   ; no
         move.w   c_SrcLine(a5),d0     ; source line number
         move.l   d0,ev_SrcLine(a4)    ; ... install it

         ; Check whether the clause is to be traced.

1$:      move.b   c_Type(a5),d2        ; clause type
         bclr     #(EXB_TRACE-24),(STATUS+0)(a4)
         btst     d2,TraceTst(pc,d7.w) ; trace clause?
         beq.s    CheckParse           ; no
         bset     #(EXB_TRACE-24),(STATUS+0)(a4)

         ; Check for second-stage parsing of INSTRUCTION clauses.  If an
         ; error occurs, the clause is reclassifed as an 'ERROR'.

CheckParse
         subq.b   #C_INSTRUCT,d2       ; an instruction?
         bne.s    1$                   ; no
         btst     #CFB_PARSED,c_Flags(a5)
         bne.s    1$                   ; ... already parsed
         bsr      ScanClause           ; parse instruction

         ; Check whether to cache the clause ...

1$:      btst     #(EXB_CACHE-16),(STATUS+1)(a4)
         beq.s    CheckRange           ; ... cache not on
         btst     #CFB_CACHED,c_Flags(a5)
         bne.s    CheckRange           ; ... already cached

         movea.l  a4,a0                ; environment
         movea.l  a5,a1                ; clause
         bsr      CacheClause

         ; Control range processing.  Finished ranges are removed from the
         ; stack and recycled.  If an error is detected, the clause is
         ; reclassifed as an error.

CheckRange
         bsr      rxRange              ; process ranges

         ; SKIP flag processing ... flag is cleared if the 'CLEAR' flag
         ; is set and there are either no ranges or the range is active.

CheckSkip
         moveq    #EXB_SKIP,d2         ; load 'SKIP' flag
         move.l   STATUS(a4),d4        ; load status

         ; Make a quick check for any flags ...

         move.w   #EXF_MOVE!EXF_CLEAR!EXF_SKIP,d0                    8
         and.w    d4,d0                ; any flags set?              4
         beq.s    CheckFinal           ; no                         10

         bclr     #EXB_MOVE,d4         ; clear 'MOVE'               10
         tst.l    d5                   ; a macro instruction?        4
         bpl.s    2$                   ; no                          8
         bclr     #EXB_CLEAR,d4        ; clear 'SKIP'?              10
         beq.s    2$                   ; no                         10

         ; Ready to clear ... check whether the topmost range is active.

         tst.w    (ev_RStack+skNum)(a4); any ranges?
         beq.s    1$                   ; no -- clear 'SKIP'
         movea.l  ev_RStack(a4),a0     ; topmost range
         btst     #NRFB_ACTIVE,r_Flags(a0)
         beq.s    2$                   ; ... not active

1$:      bclr     d2,d4                ; clear the 'SKIP'

2$:      move.w   d4,(STATUS+2)(a4)    ; update (half) status         8?

         ; Final parsing and expression processing (if not in 'SKIP' mode).

CheckFinal
         btst     d2,d4                ; 'SKIP' mode?
         bne.s    DoClause             ; yes

         bset     #CFB_FINAL,c_Flags(a5)
         bne.s    CheckExpr            ; ... already processed

         ; Check if the clause is to be traced.  

         btst     #EXB_TRACE,d4        ; trace it?
         beq.s    2$                   ; no
         tst.l    ev_TraceCnt(a4)      ; inhibited?
         bpl.s    1$                   ; no
         addq.l   #1,ev_TraceCnt(a4)   ; advance counter

1$:      movea.l  a4,a0                ; environment
         moveq    #ACT_LINE,d1         ; list clause
         bsr      rxTrace

         ; Check for a 'DO' instruction.  If found, branch to construct
         ; the increment expression (if required).

2$:      cmp.l    DOCode(pc),d3        ; a 'DO' instruction?
         bne.s    CheckExpr            ; no
         tst.w    (ev_RStack+skNum)(a4); any ranges?
         beq.s    CheckExpr            ; no

         movea.l  ev_RStack(a4),a0     ; first range
         cmpi.b   #NRANGE_DO,r_Type(a0); a 'DO' range?
         bne.s    CheckExpr            ; no ...

         move.l   r_SrcPos(a0),d0      ; source position of range
         cmp.l    c_SrcPos(a5),d0      ; matches clause?
         bne.s    CheckExpr            ; no ...
         bsr      ScanDO               ; process 'DO'

         ; Process any expressions ... 'FUNCtion' clauses are created to
         ; evaluate function calls.  Expression analysis continues until
         ; the current (bottom) clause is irreducible.

CheckExpr
         movea.l  STACKBOT(a2),a5      ; current clause
         btst     #CFB_SIMPLE,c_Flags(a5)
         bne.s    DoClause             ; ... already processed

         move.l   #IPF_EVAL!ISF_EVAL!ISF_NOEVAL,d0
         and.l    c_Keys(a5),d0        ; an expression?
         beq.s    DoClause             ; no
         btst     #ISB_NOEVAL,d0       ; suppress evaluation?
         bne.s    DoClause             ; yes

         ; Analyze the expression ... may create a 'FUNCtion' clause.

         bsr      ScanExpr             ; process expression

         ; Remove the current clause from the pipeline and execute it.

DoClause movea.l  a2,a0                ; pipeline header
         bsr      PopTail              ; D0=clause
         movea.l  d0,a5                ; current clause
         move.l   c_Keys(a5),d5        ; instruction word

         ; Pre-execution test:  clear 'SKIP' mode?

         btst     #EXB_SKIP,d4         ; in 'SKIP' mode?
         beq.s    1$                   ; no
         tst.l    d5                   ; a macroinstruction?
         bpl.s    1$                   ; no
         bset     #(EXB_CLEAR-8),(STATUS+2)(a4)

         ; Execute the clause ... registers D2-D7 are scratch.

1$:      CALLSYS  rxInstruct           ; D6=error

         move.l   STATUS(a4),d4        ; load status
         move.l   c_Keys(a5),d5        ; a macroinstruction?
         bpl.s    4$                   ; no ... release clause

         bclr     #EFB_TIMER,EVFLAGS(a4)

         ; Check whether any status flags need checking ...

         move.w   #1<<EXB_NEXTC!1<<EXB_COND!1<<EXB_ACTIVE,d1
         and.w    d4,d1                ; any flags?
         beq.s    4$                   ; no ... release clause

         bclr     #EXB_COND,d4         ; leave 'CONDitional'

         ; Check whether we're entering 'CONDitional' mode.

         bclr     #EXB_NEXTC,d4        ; next clause conditional?
         beq.s    2$                   ; no
         bset     #EXB_COND,d4         ; set 'CONDitional' flag

         ; Check whether to pause for interactive input after this clause.

2$:      tst.l    d6                   ; an execution error?
         bne.s    3$                   ; yes
         btst     #EXB_ACTIVE,d4       ; interactive tracing?
         beq.s    3$                   ; no
         btst     #EXB_TRACE,d4        ; clause traced?
         beq.s    3$                   ; no

         move.l   #(1<<EXB_SKIP!1<<EXB_DEBUG),d1
         and.l    d4,d1                ; 'SKIP' or 'DEBUG' mode?
         bne.s    3$                   ; yes
         btst     #IPB_PAUSE,d5        ; a "pause clause"?
         beq.s    3$                   ; no
         bset     #EXB_PAUSE,d4        ; set flag

3$:      move.l   d4,STATUS(a4)        ; update status

         ; Release the clause.

4$:      movea.l  a4,a0                ; environment
         movea.l  a5,a1                ; clause
         bsr      FreeClause           ; release it

         ; Check for errors during this machine cycle.

CheckError
         tst.l    d6                   ; execution error?
         beq.s    CheckRETURN          ; no ...

         moveq    #INB_SYNTAX,d1       ; 'SYNTAX' interrupt

         ; Check if the interrupt condition is enabled.

CheckTrap
         move.l   d6,d0                ; error code
         movea.l  a4,a0                ; environment
         bsr      rxSignal             ; D0=error D1=boolean
         move.l   d0,d6                ; an error?
         beq.s    CheckRETURN          ; no

         ; An error occurred ... report it.

         movea.l  a4,a0                ; environment
         moveq    #ACT_SYNTAX,d1       ; syntax error
         bsr      rxTrace              ; report it

         ; Check whether to clear the error.  In 'DEBUG' mode, errors are
         ; cleared after being reported.

         btst     #EXB_DEBUG,d4        ; 'DEBUG' mode?
         beq.s    ForceRETURN          ; no ... force return
         moveq    #0,d6                ; clear error

         ; Clear the 'DEBUG' (topmost) range.

         movea.l  a4,a0                ; environment
         bsr      FreeRange            ; release range

         ; Check whether the 'RETURN' flag is set, and exit if 'RETURN'
         ; is set in base environment.

CheckRETURN
         btst     #EXB_RETURN,d4       ; 'RETURN' set?
         beq.s    NextCycle            ; no

         ; Check whether we're returning from an (internal) function ...

ForceRETURN
         cmpa.l   a3,a4                ; base environment?
         beq.s    Exit                 ; no ... exit
         movea.l  a3,a0                ; base environment
         bsr      FreeEnv              ; release environment

         ; Update the current environment and machine status and loop back.

NextCycle
         movea.l  gn_CurrEnv(a3),a4    ; current environment
         move.l   STATUS(a4),d4        ; current status

         btst     #EXB_EXIT,d4         ; 'EXIT' set?
         beq      BeginCycle           ; no ... loop back

Exit     move.l   d6,d0                ; set CCR
         rts

* ============================     rxEnd     ===========================
* Processes the 'end-of-source' condition.  Private to 'rxParse'.
* Registers:   A4 -- environment
* Return:      D6 -- error code
rxEnd
         moveq    #0,d2                ; clear flag
         bra.s    6$                   ; jump in

         ; Check the topmost control range ...

1$:      movea.l  (ev_RStack+skHead)(a4),a0
         move.b   r_Type(a0),d0        ; range type
         move.b   r_Flags(a0),d1       ; range flags

         subq.b   #NRANGE_IF,d0        ; an 'IF' range?
         bne.s    3$                   ; no
         btst     #(EXB_COND-8),(STATUS+2)(a4)
         beq.s    2$                   ; ... not conditional
         moveq    #ERR10_029,d6        ; error ... incomplete 'IF'
         bra.s    5$

2$:      btst     #NRFB_THEN,d1        ; a 'THEN' clause?
         bne.s    5$                   ; yes
         moveq    #ERR10_024,d6        ; error ... missing 'THEN'
         bra.s    5$

         ; Check for an 'INTERPRET' range ...

3$:      subq.b   #NRANGE_INTERP-NRANGE_IF,d0
         bne.s    4$                   ; ... not 'INTERPRET'
         moveq    #-1,d2               ; set 'resolved'
         bra.s    5$                   ; remove range

         ; Must be a 'DO' or 'SELECT' range ...

4$:      btst     #NRFB_FINISH,d1      ; range finished?
         bne.s    5$                   ; yes
         moveq    #ERR10_026,d6        ; error ... missing 'END'

         ; Release the topmost control range ...

5$:      movea.l  a4,a0                ; environment
         bsr      FreeRange            ; remove range

6$:      or.l     d6,d2                ; error or resolved?
         bne.s    7$                   ; yes
         tst.w    (ev_RStack+skNum)(a4); any ranges?
         bne.s    1$                   ; yes

         ; End not resolved ... simulate a 'RETURN', but with no value.

         bset     #(EXB_RETURN-8),(STATUS+2)(a4)

7$:      rts

* ===========================     rxRange     ==========================
* Processes control ranges, and checks that instructions are valid in the
* current scope.  If an error is detected, the clause is reclassified.
* This routine is private to 'rxParse'.
* Registers:   A4 -- environment
*              A5 -- clause
*              D3 -- instruction opcode
*              D5 -- instruction word
rxRange 
         tst.w    (ev_RStack+skNum)(a4); any ranges?
         beq.s    rROut                ; no

         ; Get the top range and check the 'FINISH' flag ... remove if set.

         movea.l  (ev_RStack+skHead)(a4),a0
         move.b   r_Flags(a0),d1       ; range flags
         btst     #NRFB_FINISH,d1      ; finished?
         bne.s    1$                   ; yes ... remove it

         move.w   (STATUS+2)(a4),d4    ; load (half) status

         ; Check first for an 'IF' range ,,,

         move.b   r_Type(a0),d0        ; range type
         subq.b   #NRANGE_IF,d0        ; an IF range?
         bne.s    rRSelect             ; no

         btst     #EXB_COND,d4         ; still conditional?
         bne.s    rRActive             ; yes

         ; Check whether the 'THEN' has been executed.

         btst     #NRFB_THEN,d1        ; 'THEN' clause yet?
         beq.s    2$                   ; no

         ; 'IF' range implicit removal processing.

         btst     #NRFB_ELSE,d1        ; an 'ELSE' yet?
         bne.s    1$                   ; yes -- remove it
         btst     #EXB_MOVE,d4         ; moving position?
         bne.s    1$                   ; yes -- remove it

         ; Check for the 'ELSE' clause hold-off:  range is not removed
         ; if an 'ELSE' clause is pending, or if the instruction is not
         ; a macroinstruction (one that clears the 'SKIP' flag).

         tst.l    d5                   ; a macroinstruction?
         bpl.s    rRActive             ; no -- holdoff
         cmpi.l   #KC_ELSE,d3          ; an 'ELSE' clause?
         beq.s    rRActive             ; yes -- holdoff

         ; No 'ELSE' clause ... attempt to cache the current position.

         movea.l  a4,a0                ; environment
         movea.l  a5,a1                ; clause
         bsr      SetCachePosition     ; install position

         ; Remove the (topmost) control range, then check for more.

1$:      movea.l  a4,a0                ; storage environment
         bsr      FreeRange            ; remove topmost range
         bra.s    rxRange              ; ... loop back

         ; No 'THEN' yet ... check for a valid 'IF' instruction.

2$:      tst.l    d5                   ; a macroinstruction?
         bpl.s    rRActive             ; no
         cmpi.l   #KC_THEN,d3          ; 'THEN' clause?
         beq.s    rRActive             ; yes
         moveq    #ERR10_024,d0        ; error ... missing 'THEN'

         ; An error occurred ... reclassify as an error clause.

rRError  movea.l  a5,a0                ; this clause
         bsr      ErrClause            ; tag as 'error'

rROut    rts

         ; Check whether the range is 'ACTIVE'.  If not, set the 'SKIP' flag.

rRActive btst     #NRFB_ACTIVE,d1      ; still active?
         beq.s    rRSkip               ; no
         rts

         ; Check for a 'SELECT' range.

rRSelect subq.b   #NRANGE_SELECT-NRANGE_IF,d0
         bne.s    rRActive             ; ... not 'SELECT'

         btst     #EXB_COND,d4         ; still conditional?
         bne.s    rRActive             ; yes ... nothing to check

         btst     #NRFB_ELSE,d1        ; 'OTHERWISE' reached?
         bne.s    rRActive             ; yes

         ; Check for statements valid within a 'SELECT' range.

         tst.l    d5                   ; a macroinstruction?
         bpl.s    rRActive             ; no
         moveq    #ERR10_023,d0        ; "invalid SELECT statement"
         btst     #IPB_SELECT,d5       ; valid in SELECT?
         beq.s    rRError              ; no

         ; Check whether a branch has been taken.  If so, clear 'ACTIVE'.

         btst     #NRFB_BRANCH,d1      ; a branch taken?
         beq.s    rRActive             ; no
         bclr     #NRFB_ACTIVE,r_Flags(a0)

         ; Range inactive ... set the 'SKIP' flag.

rRSkip   bset     #(EXB_SKIP-8),(STATUS+2)(a4)
         rts
