* == rxinstruct.asm ====================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ==========================     rxInstruct     =========================
* Instruction dispatcher for REXX interpreter.
* Registers:   A3 -- global pointer
*              A4 -- environment
*              A5 -- clause
*              D5 -- instruction word
*              D6 -- error code (cleared)
*              D7 -- tracing option
* Return:      D6 -- error code
* Scratch:     D2-D7
* Register assignments in the instruction modules are as follows:
*     A0 -- first token
*     A1 -- token string
*     A2 -- control stack
*     A3 -- global pointer
*     A4 -- environment
*     A5 -- clause
*     D0 -- cleared
*     D1 -- cleared
*     D4 -- machine status word
*     D5 -- instruction code
*     D6 -- error code (cleared)
*     D7 -- trace option
* Instruction routines may alter the scratch registers as required.
rxInstruct:
         movem.l  a2-a5,-(sp)

         movea.l  cTL_Head(a5),a0      ; first token
         move.l   STATUS(a4),d4        ; status word

         ; Check whether the machine is in 'SKIP' mode

         btst     #EXB_SKIP,d4         ; skip mode?
         beq.s    1$                   ; no
         btst     #IPB_SKIPSUPP,d5     ; suppressed in 'SKIP'?
         bne.s    4$                   ; yes

         ; Check for extraneous tokens if clause should be "empty"

1$:      btst     #IPB_EMPTY,d5        ; must be empty?
         beq.s    2$                   ; no
         bsr      rxCheckSTOP          ; D6=error or 0
         bne.s    4$                   ; ... error

         ; Check whether the machine is in 'SCAN' (tracing) mode

2$:      cmpi.b   #TRACE_S,d7          ; 'scan' mode?
         bne.s    3$                   ; no
         btst     #IPB_SCANSUPP,d5     ; suppressed?
         bne.s    4$                   ; yes

3$:      move.l   d5,d2                ; instruction word
         swap     d2                   ; quick shift
         andi.w   #KTF_CODEMASK>>16,d2 ; mask opcode
         lsr.w    #8-1,d2              ; opcode*2

         ; Final register setup for the call ...

         moveq    #0,d0                ; clear D0
         moveq    #0,d1                ; clear D1
         movea.l  TSTRING(a0),a1       ; token string
         lea      ev_RStack(a4),a2     ; control range stack
         move.w   rxIjump(pc,d2.w),d2  ; offset to function
         jsr      rxIjump(pc,d2.w)     ; D6=error

4$:      movem.l  (sp)+,a2-a5
         rts

         ; The jump table for the instructions

rxIjump  dc.w     rxInsADDR-rxIjump    ; $00: 'ADDRESS'
         dc.w     rxInsBREAK-rxIjump   ; $01: 'BREAK'
         dc.w     rxInsCALL-rxIjump    ; $02: 'CALL'
         dc.w     rxInsDO-rxIjump      ; $03: 'DO'
         dc.w     rxInsDROP-rxIjump    ; $04: 'DROP'
         dc.w     rxInsELSE-rxIjump    ; $05: 'ELSE'
         dc.w     rxInsEND-rxIjump     ; $06: 'END'
         dc.w     rxERROR-rxIjump      ; $07: 'ERROR' clause
         dc.w     rxInsIF-rxIjump      ; $08: 'IF'
         dc.w     rxInsINTERP-rxIjump  ; $09: 'INTERPRET'
         dc.w     rxInsITERATE-rxIjump ; $0A: 'ITERATE'
         dc.w     rxInsITERATE-rxIjump ; $0B: 'LEAVE'
         dc.w     rxInsNOP-rxIjump     ; $0C: 'NOP'
         dc.w     rxInsNUMERIC-rxIjump ; $0D: 'NUMERIC'
         dc.w     rxInsOTHRW-rxIjump   ; $0E: 'OTHERWISE'
         dc.w     rxInsPARSE-rxIjump   ; $0F: 'PARSE'
         dc.w     rxInsPROC-rxIjump    ; $10: 'PROCEDURE'
         dc.w     rxInsPUSH-rxIjump    ; $11: 'PUSH'
         dc.w     rxInsQUEUE-rxIjump   ; $12: 'QUEUE'
         dc.w     rxInsRETURN-rxIjump  ; $13: 'RETURN'
         dc.w     rxInsSAY-rxIjump     ; $14: 'SAY'
         dc.w     rxInsSELECT-rxIjump  ; $15: 'SELECT'
         dc.w     rxInsSIGNAL-rxIjump  ; $16: 'SIGNAL'
         dc.w     rxInsTHEN-rxIjump    ; $17: 'THEN'
         dc.w     rxInsTRACE-rxIjump   ; $18: 'TRACE'
         dc.w     rxInsUNTIL-rxIjump   ; $19: 'UNTIL'
         dc.w     rxInsUPPER-rxIjump   ; $1A: 'UPPER'
         dc.w     rxInsWHEN-rxIjump    ; $1B: 'WHEN'
         dc.w     rxInsOPTIONS-rxIjump ; $1C: 'OPTIONS'
         dc.w     rxIII-rxIjump        ; $1D: unassigned
         dc.w     rxIII-rxIjump        ; $1E: unassigned
         dc.w     rxIII-rxIjump        ; $1F: unassigned
         dc.w     rxASSIGN-rxIjump     ; $20: assignment statement
         dc.w     rxCOMMAND-rxIjump    ; $21: command statement
         dc.w     rxIII-rxIjump        ; $22: unassigned
         dc.w     rxFUNCTION-rxIjump   ; $23: 'FUNCTION'
         dc.w     rxLABEL-rxIjump      ; $24: 'LABEL'
         dc.w     rxPASS-rxIjump       ; $25: 'PASS'

         ; Unimplemented opcodes

rxIII                                  ; invalid code
         moveq    #-1,d6
         rts

         ; ERROR instruction ... error code is in lower instruction word

rxERROR  move.w   d5,d6                ; error code

         ; NOP instructions

rxLABEL                                ; LABEL statement
rxASSIGN                               ; assignment statement
rxInsNOP                               ; NOP instruction
rxPASS                                 ; PASS instruction
         rts

* =======================     rxCheckSTOP     ==========================
* Checks whether the last token is the STOPTOKEN.
* Registers:   A0 -- last token
rxCheckSTOP
         cmpi.b   #T_END,t_Type(a0)    ; the STOPTOKEN?
         bne.s    rxErr35              ; no ... extraneous token
rxAllOK  moveq    #0,d6                ; clear error
         rts

         ; Shared error returns

rxErr19  moveq    #ERR10_019,d6        ; invalid 'PROCEDURE' instruction
         rts
rxErr20  moveq    #ERR10_020,d6        ; unexpected 'THEN' or 'ELSE'
         rts
rxErr21  moveq    #ERR10_021,d6
         rts
rxErr22  moveq    #ERR10_022,d6
         rts
rxErr24  moveq    #ERR10_024,d6        ; missing or multiple 'THEN'
         rts
rxErr25  moveq    #ERR10_025,d6
         rts
rxErr26  moveq    #ERR10_026,d6
         rts
rxErr27  moveq    #ERR10_027,d6
         rts
rxErr28  moveq    #ERR10_028,d6        ; invalid 'DO' syntax
         rts
rxErr29  moveq    #ERR10_029,d6        ; incomplete 'IF' or 'SELECT'
         rts
rxErr31  moveq    #ERR10_031,d6        ; symbol expected
         rts
rxErr34  moveq    #ERR10_034,d6        ; required keyword missing
         rts
rxErr35  moveq    #ERR10_035,d6        ; extraneous characters
         rts
rxErr44  moveq    #ERR10_044,d6        ; invalid expression result
         rts
rxErr45  moveq    #ERR10_045,d6        ; expression required
         rts
