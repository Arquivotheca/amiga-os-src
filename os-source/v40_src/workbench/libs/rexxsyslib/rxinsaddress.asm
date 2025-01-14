* == rxinsaddress.asm ==================================================
*
* Copyright (c) 1986-1990 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ==========================     rxInsADDR     =========================
* Implements the 'ADDRESS' instruction.
* [instruction module environment]
rxInsADDR
         btst     #ISB_SKIP1ST,d5      ; symbol or string?
         beq.s    1$                   ; no ...
         move.l   a1,d1                ; yes -- get host address
         movea.l  (a0),a0              ; advance token

1$:      btst     #CFB_NULLEXPR,c_Flags(a5)
         bne.s    2$                   ; ... no expression result
         move.l   d1,d0
         move.l   TSTRING(a0),d1       ; command string (or new address)

         ; Check that the clause ends properly.

2$:      cmpi.b   #T_END,t_Type(a0)    ; a STOPTOKEN?
         bne      rxErr35              ; no -- error

         ; Check if an external host specified ...

         tst.l    d0                   ; external host given?
         bne.s    5$                   ; yes

         ; Update the current host address?

         lea      ev_COMMAND(a4),a2
         move.l   ev_LastComm(a4),d2   ; load prior address
         move.l   (a2),ev_LastComm(a4) ; push current address
         tst.l    d1                   ; new host address?
         beq.s    3$                   ; no

         ; Mark the new string as 'private' and release the old string.

         movea.l  a4,a0                ; environment
         movea.l  d1,a1                ; new string
         bsr      KeepString           ; D0=A1=private string
         exg      d2,a1                ; new=>D2, old=>A1
         movea.l  a4,a0                ; environment
         bsr      FreeKeepStr          ; release previous address

         ; Install the new current host address.

3$:      move.l   d2,(a2)              ; previous ==> current
         rts

         ; Issue the command to an external host

5$:      movea.l  d0,a0                ; host address
         movea.l  d1,a1                ; command string
         bra.s    HostCommand          ; D6=error

* =========================     rxCOMMAND     ==========================
* Executes a "Command" statement.  The result string is issued to the
* current host address.
* [instruction module environment]
rxCOMMAND
         btst     #CFB_NULLEXPR,c_Flags(a5)
         beq.s    1$                   ; ... result available
         movea.l  rl_NULL(a6),a1       ; use null string

1$:      movea.l  ev_COMMAND(a4),a0    ; current host address

* ========================     HostCommand     =========================
* Issues a command to an external host environment.
* [instruction module environment]
* Registers:      A0 -- host address
*                 A1 -- command 
* Return:         D6 -- error code
HostCommand
         movea.l  a0,a2                ; host address
         lea      ns_Buff(a1),a5       ; command string
         moveq    #0,d3                ; clear 'results'

         ; Check whether to trace the command string.

         subq.b   #TRACE_C,d7          ; trace commands?
         bne.s    1$                   ; no
         movea.l  a4,a0                ; environment
         moveq    #ACT_RESULT,d1       ; trace result string
         bsr      rxTrace

         ; Check if in 'DEBUG' mode.  If so, commands are always issued.

1$:      btst     #EXB_DEBUG,d4        ; 'DEBUG' mode?
         bne.s    2$                   ; yes

         ; Check if in 'command inhibition' mode ... suppress command.

         moveq    #0,d1                ; default return code
         btst     #EXB_NOCOMM,d4       ; inhibit commands?
         bne.s    HCCheck              ; yes

         ; Prepare the message packet and send it to the external host.

2$:      movea.l  a3,a0                ; global pointer
         lea      ns_Buff(a2),a1       ; host address string
         movea.l  gn_MsgPkt(a3),a2     ; global message packet

         ; Load the action code and check whether to request a result string.

         move.l   #RXCOMM,d0           ; command code
         btst     #EFB_REQ,EVFLAGS(a4) ; request results?
         beq.s    3$                   ; no
         bset     #RXFB_RESULT,d0      ; ... set 'RESULT' bit
         moveq    #-1,d3               ; set 'results'

         ; Send the message to the host.

3$:      move.l   d0,ACTION(a2)        ; install action
         move.l   a5,ARG0(a2)          ; command string
         move.l   a2,d0                ; message packet
         bsr      SendRexxMsg          ; D0=error D1=return code
         move.l   d0,d6                ; error in issuing command?
         bne.s    HCOut                ; yes ...

         ; Set the 'RC' variable and check for interrupts.  Check first
         ; for command FAILUREs (RC >= FAILAT) and then ERRORs.

HCCheck  move.l   d1,d0                ; return code
         moveq    #2-1,d2              ; loop count
         cmp.l    ev_TraceLim(a4),d0   ; failure?
         blt.s    2$                   ; no ... jump in
         moveq    #INB_FAILURE,d1      ; 'FAILURE' interrupt

         ; Check for an enabled signal.

1$:      movea.l  a4,a0                ; environment
         bsr      rxSignal             ; D0=error D1=boolean
         tst.w    d1                   ; trapped?
         bne.s    HCOut                ; yes

2$:      moveq    #INB_ERROR,d1        ; 'ERROR' interrupt
         dbf      d2,1$                ; loop back

         ; Check whether the command return code indicated an error.

         move.l   d0,d2                ; command error?
         beq.s    5$                   ; no

         ; Error occurred ... check for tracing.

         tst.b    d7                   ; trace 'COMMANDS'?
         beq.s    4$                   ; yes
         btst     #EXB_DEBUG,d4        ; 'DEBUG' mode?
         bne.s    3$                   ; yes
         subq.b   #TRACE_E-TRACE_C,d7  ; trace 'ERRORS'?
         beq.s    3$                   ; yes
         subq.b   #TRACE_N-TRACE_E,d7  ; trace 'NORMAL'?
         bne.s    HCOut                ; no

         ; 'Normal' tracing ... report only command failures.

         cmp.l    ev_TraceLim(a4),d2   ; RC >= failat?
         blt.s    HCOut                ; no

         ; First trace the clause ...

3$:      movea.l  a4,a0                ; environment
         moveq    #ACT_LINE,d1         ; list the clause
         bsr      rxTrace

         ; ... and then the error.

4$:      movea.l  a4,a0                ; environment
         move.l   d2,d0                ; return code
         moveq    #ACT_ERROR,d1        ; trace action
         bsr      rxTrace              ; report it
         bra.s    HCOut                ; ... no result allowed

         ; All OK ... check whether a result string was requested.

5$:      tst.w    d3                   ; results requested?
         beq.s    HCOut                ; no ...
         move.l   RESULT2(a2),d0       ; result available?
         beq.s    HCAssign             ; no ... use NULL

         ; Copy the result to internal memory.

         movea.l  a4,a0                ; environment
         movea.l  d0,a1                ; argstring
         bsr      CVarg2s              ; D0=A0=string

         ; Assign the result to the special variable 'RESULT'.

HCAssign movea.l  a4,a0                ; environment
         moveq    #STRESULT,d1         ; variable index
         bsr      AssignSpecial        ; D0=error

HCOut    tst.l    d6                   ; set CCR
         rts

* ========================      rxInsCALL     ==========================
* Implements the 'CALL' instruction.
* [instruction module routine]
rxInsCALL
         move.l   a1,d0                ; result string (possibly NULL)
         bra.s    HCAssign             ; install in 'RESULT'

* ========================      rxFUNCTION      ========================
* Evaluates a function call.  The search order for function calls is:
*  (1) An internal function defined by a label.  This step is skipped if
*      the function name is not a symbol token.
*  (2) The built-in function library.
*  (3) External function libraries/function hosts list.
* [instruction module routine]
rxFUNCTION
         btst     #TTB_SYMBOL,t_Type(a0)
         beq.s    1$                   ; ... skip internal search

         ; Check for an internal function defined by a label.

         move.w   TOFFSET(a0),d2       ; save token offset
         movea.l  a4,a0                ; environment
         bsr      rxLocate             ; D0=error D1=position
         exg      d1,d2                ; offset=>D1, position=>D2
         beq.s    4$                   ; ... label found

         ; Label not found ... change the cached token to type T_STRING
         ; in order to prevent further label searches.

         movea.l  a4,a0                ; environment
         move.l   c_SrcPos(a5),d0      ; source position
         bsr      FindCachedToken      ; D0=A0=cached token
         beq.s    1$                   ; ... not cached

         ; Check for implicit concatenation (operator has same offset).

         cmpi.b   #T_OPERATOR,(a0)     ; implicit concatenation?
         bne.s    0$                   ; no
         addq.w   #(t_SIZEOF-t_Type),a0; next token
0$:      move.b   #T_STRING,(a0)       ; new token type

         ; Check for a "Built-In" function.

1$:      movea.l  a4,a0                ; environment
         movea.l  a5,a1                ; clause
         bsr      BIFunc               ; D0=error A0=result
         move.l   d0,d6                ; function found?
         beq.s    2$                   ; yes -- post result

         moveq    #ERR10_015,d0        ; "function not found"
         cmp.l    d0,d6                ; not found?
         bne.s    3$                   ; no -- execution error

         ; Check for an external function or program (REXX or otherwise).

         bsr      ExtFunction          ; D6=error  A0=result
         bne.s    3$                   ; error ...

2$:      move.l   a0,EVNAME(a3)        ; post the result

3$:      rts

         ; An internal function ... allocate a new environment.

4$:      movea.l  a3,a0                ; global pointer
         movea.l  a4,a1                ; this environment
         bsr      GetEnv               ; D0=A0=environment
         movea.l  a0,a2                ; new environment
         move.l   d2,ev_PC(a2)         ; install new position

         ; Make a copy of the clause and install it as the arglist.

         movea.l  a4,a0                ; environment
         movea.l  a5,a1                ; clause
         move.l   cTL_Head(a5),d0      ; first token
         bsr      CopyClause           ; D0=A1=new clause
         move.l   d0,ev_ArgList(a2)    ; ... install arglist

         rts
