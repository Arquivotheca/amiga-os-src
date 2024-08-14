* == rxinsnumeric.asm ==================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* =======================     rxInsNUMERIC     =========================
* Implements the 'NUMERIC' instruction.
OPTDIGIT EQU      KT12_OPTMASK&KT12_DIGITS
OPTENG   EQU      KT12_OPTMASK&KT12_ENG
rxInsNUMERIC
         btst     #ISB_EVAL,d5         ; an expression?
         beq.s    RINForm              ; no

         ; First form: NUMERIC {DIGITS/FUZZ} [expression]

         movea.l  a1,a0                ; result string
         moveq    #ENV_MAX_DIGITS,d2   ; maximum numeric digits

         ; Check which option was selected.

         andi.w   #OPTDIGIT,d5         ; 'DIGITS' option?
         beq.s    2$                   ; no

         ; 'DIGITS' option ...

         moveq    #9,d1                ; default digits
         btst     #CFB_NULLEXPR,c_Flags(a5)
         bne.s    1$                   ; ... no expression

         ; Convert and validate the expression result.

         bsr      CVs2i                ; D0=error D1=value
         bne.s    4$                   ; ... error

         ; Validate the result ... must have 0 < digits <= max.

         tst.l    d1                   ; digits > 0?
         ble.s    4$                   ; no ... error
         cmp.l    d2,d1                ; result <= max?
         bgt.s    4$                   ; no ... error

         ; Make sure digits > fuzz.

1$:      cmp.w    ev_NumFuzz(a4),d1    ; digits > fuzz?
         ble.s    4$                   ; no ... error
         move.w   d1,ev_NumDigits(a4)  ; ... install digits
         rts

         ; 'FUZZ' option ...

2$:      moveq    #0,d1                ; default fuzz
         btst     #CFB_NULLEXPR,c_Flags(a5)
         bne.s    3$                   ; ... no expression

         ; Convert and validate the expression result.

         bsr      CVs2i                ; D0=error D1=value
         bne.s    4$                   ; ... error

         ; Validate the result ... must have 0 <= fuzz < max.

         tst.l    d1                   ; fuzz >= 0?
         bmi.s    4$                   ; no ... error
         cmp.l    d2,d1                ; fuzz < max?
         bge.s    4$                   ; no ... error

         ; Make sure fuzz < digits.

3$:      cmp.w    ev_NumDigits(a4),d1  ; fuzz < digits?
         bge.s    4$                   ; no ... error
         move.w   d1,ev_NumFuzz(a4)    ; ... install fuzz
         rts

4$:      bra      rxErr44              ; "invalid operand"

         ; Second form: NUMERIC FORM {SCIENTIFIC/ENGINERING}

RINForm  moveq    #OPTENG,d0           ; 'ENGINEERING' specified?
         moveq    #EFB_SCI,d1          ; flag bit

* ==========================     SetEVFlags     ========================
* Tests the option and sets the environment flags.  ** Private **
* Registers:      D0 -- option mask
*                 D1 -- flag bit
SetEVFlags
         bset     d1,EVFLAGS(a4)       ; set flag
         and.w    d5,d0                ; option selected?
         beq.s    1$                   ; no
         bclr     d1,EVFLAGS(a4)       ; yes -- clear flag
1$:      rts

* =======================     rxInsOPTIONS     =========================
* Implements the 'OPTIONS' instruction.
rxInsOPTIONS
         moveq    #KT28_OPTMASK,d0     ; option mask
         and.w    d5,d0                ; any options?
         bne.s    1$                   ; yes

         ; No option specified ... reset everything to default state.

         bclr     #EFB_REQ,EVFLAGS(a4)
         bset     #EFB_CACHE,EVFLAGS(a4)
         moveq    #10,d1               ; default fail level ** use CLI! **
         move.l   d1,ev_TraceLim(a4)   ; reset limit
         bra.s    6$                   ; reset prompt

         ; Check for 'CACHE' option

1$:      moveq    #KT28_CACHE&KT28_OPTMASK,d0
         and.w    d5,d0                ; 'CACHE' keyword?
         beq.s    2$                   ; no
         moveq    #KT28_NO&KTF_SECMASK,d0
         moveq    #EFB_CACHE,d1        ; 'CACHE' bit
         bsr.s    SetEVFlags           ; set flag

         ; Check for the 'RESIDENT' option

2$:      moveq    #KT28_RESIDENT,d1    ; (not implemented)

         ; Check for the 'RESULTS' option.

3$:      moveq    #KT28_RESULTS&KT28_OPTMASK,d0
         and.w    d5,d0                ; 'RESULTS' keyword?
         beq.s    4$                   ; no
         moveq    #KT28_NO&KTF_SECMASK,d0
         moveq    #EFB_REQ,d1          ; 'REQUEST' bit
         bsr.s    SetEVFlags           ; set flag

         ; Check for 'OPTIONS FAILAT n' ... set failure level.

4$:      moveq    #KT28_FAILAT&KT28_OPTMASK,d0
         and.w    d5,d0                ; 'FAILAT' keyword?
         beq.s    5$                   ; no ...

         btst     #CFB_NULLEXPR,c_Flags(a5)
         bne      rxErr45              ; ... error
         movea.l  a1,a0                ; result string
         bsr      CVs2i                ; D0=error D1=value
         move.l   d0,d6                ; error?
         bne.s    8$                   ; ... yes
         move.l   d1,ev_TraceLim(a4)   ; failure level
         bra.s    8$

         ; Check for 'OPTIONS PROMPT string' ... set prompt.

5$:      moveq    #KT28_PROMPT&KT28_OPTMASK,d0
         and.w    d5,d0                ; 'PROMPT' keyword?
         beq.s    8$                   ; no ...

         ; 'PROMPT' option ... check for an expression result.

6$:      move.l   rl_NULL(a6),d0       ; default prompt
         movea.l  a4,a0                ; environment
         move.l   a1,d1                ; a result?
         beq.s    7$                   ; no
         bsr      KeepString           ; D0=A1=string

         ; Install the new prompt and release the old string.

7$:      move.l   ev_Prompt(a4),a1     ; load old prompt
         move.l   d0,ev_Prompt(a4)     ; install new prompt
         movea.l  a4,a0                ; environment
         bsr      FreeKeepStr          ; release old

8$:      rts
