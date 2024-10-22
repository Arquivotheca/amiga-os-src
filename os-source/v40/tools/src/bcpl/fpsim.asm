*
* FLOATING POINT EMULATION SUPPORT ROUTINE
*-----------------------------------------
* by C.G. Selwyn
*     Bath University (JULY -83)
*
* This routine is entered by a 1111 emulator trap
* with a command of the following format :-
*
*   -------------------------------------------
*   | 1 1 1 1 | x x x x x x x | F | L | O O O |
*   -------------------------------------------
*   15      12                 4    3  2     0
*
*   Where :-
*      Bits 15-12 specify an emulator trap to address 02CH
*
*      Bit 4 specifies a fixed or floating point operation
*            1   -   Fixed point op.
*            0   -   Floating point op.
*
*      Bit 3 specifies the length of the operation
*            1   -   Double precision
*            0   -   Single precision
*
*      Bits 0-2 specify the operation to be performed
*
*         The following table shows which operations will be supported
*         for which kinds of operands, and their opcodes.
*
*         Opcode     Operation        Fixed   Single   Double
*            0   -   ADD                        ~        ~
*            1   -   SUB                        ~        ~
*            2   -   MUL                ~       ~        ~
*            3   -   DIV                ~       ~        ~
*            4   -   NEG                        ~        ~
*            5   -   CMP                        ~        ~
*            6   -   TST                       (~)       ~
*            7   -   <unused>
*
*   Arguments will be passed in D2 & D1 and the result returned in D1
*   Double precision arguments will be passed by reference.
*
* Two BCPL callable routines are also provided :-
*
*   I  := FIX(F)
*   F  := FLOAT(I)
*
* Where I  is a Integer
*       F  is a single precision no.
*
	  GET  "LIBHDR-i"
          GET  "FPSIMHDR-i"
*
*
* Single precision equates
*
EXCESS   EQU      127                     Exponent excess
HIDDEN   EQU      $800000                 Hidden bit mask
HIDDENB  EQU      23                      Hidden bit no.
EXPT     EQU      $7F800000               Exponent mask
INFINITY EQU      $7F800000               Infinity
MANT     EQU      $007FFFFF               Mantissa mask
*
* Double Precision equates
*
DEXCESS  EQU      1023                    Excess
DHIDDEN  EQU      $100000                 Hidden bit mask
DHIDDENB EQU      20                      Hidden bit no.
DEXPT    EQU      $7FF00000               Exponent mask
DINFINITY EQU     $7FF00000               Infinity
DMANT    EQU      $000FFFFF               Mantissa mask
*
* General equates
*
SIGN     EQU      $80000000               Sign bit mask
SIGNB    EQU      31                      Sign bit no.
NOTSIGN  EQU      $7FFFFFFF               Everything but sign
*
*
* Relocatable section
*
         RORG     $0
*
FPSECT   DC.L     (FPEND-FPSECT)/4
         DC.L     SECWORD
         DC.B     19,'1111 emulator      '
*
* Status register bit masks
*
SRM_C    EQU      1                    Carry bit
SRM_V    EQU      2                    Overflow bit
SRM_Z    EQU      4                    Zero bit
SRM_N    EQU      8                    Negative bit
SRM_X    EQU      $10                  Extend bit
*
* Entry point to floating point emulator
*
         DC.L     LIBWORD
         DC.B     7,'FPSIM  '
*
FPSIM:   MOVEM.L  A0/D0/D3/D5,-(SP)    Save some space
NREGS    EQU      4                    No. of registers I have saved
         MOVEA.L  NREGS*4+2(SP),A0     Get address just before TRAP
*
          IFD         TRIPOS
          MOVE.W   (A0),D0              Get the offending instruction
          ENDC
*
          IFND     TRIPOS
          MOVE.W   8(A0),D0             Get the offending instruction
          ENDC
*
         ANDI.L   #$1F,D0              Mask all but the FP opcode
         ASL.L    #2,D0                And convert to Jump table offset
         JMP      FPOPTAB(D0)          Jump to processor
*
*  At dispatch there are NREGS long words on the stack
*  D1          First argument
*  D2          Second argument if required
*
FPOPTAB  BRA      FADD
         BRA      FSUB
         BRA      FMUL
         BRA      FDIV
         BRA      FNEG
         BRA      FCMP
         BRA      FTST
         BRA      BADTRAP
         BRA      DFADD
         BRA      DFSUB
         BRA      DFMUL
         BRA      DFDIV
         BRA      DFNEG
         BRA      DFCMP
         BRA      DFTST
         BRA      BADTRAP
*
* Single precision arithmetics
*
* FSUB D2,D1
*
FSUB     MOVEM.L  D2/D4/D6,-(SP)
         BCHG     #SIGNB,D2               Negate and add
         BRA.S    FADD1
*
* FADD D2,D1
*
FADD     MOVEM.L  D2/D4/D6,-(SP)
FADD1    MOVE.L   D1,D5                   Copy arg1
         MOVE.L   D2,D6                   and arg2
         MOVE.L   #EXPT,D4                Exponent mask
         AND.L    D4,D6                   In each register
         BEQ      FAROK                   If EQ D2 = 0
         AND.L    D4,D5                   Leave exponent
         BNE.S    FA11                    If EQ D1 = 0
         MOVE.L   D2,D1
         BRA      FAROK                   If EQ ans = D2
FA11     NOT.L    D4                      Make mantissa mask
         BCLR     #SIGNB,D1               Remove sign bit
         SNE.B    D3                      D3 = sign of D1
         BCLR     #SIGNB,D2               Remove sign bit
         SNE.B    D0                      D0 = sign of D2
         AND.L    D4,D1                   Leave Mantissa
         AND.L    D4,D2                   In each register
         BSET     #HIDDENB,D1             Replace hidden bits
         BSET     #HIDDENB,D2
         TST.B    D3                      Make 2's comp no. of D1?
         BEQ.S    FA1
         NEG.L    D1
FA1      TST.B    D0                      And D2?
         BEQ.S    FA2
         NEG.L    D2
FA2      MOVE.L   D5,D0                   Copy D1 exponent
         SUB.L    D6,D0                   Difference of exponents
         BEQ.S    FA3                     If EQ no shift required
         MOVEQ.L  #HIDDENB,D3
         ASR.L    D3,D0                   Move to low byte
         BGT.S    FA4                     If GT |D1| > |D2|
         EXG.L    D1,D2                   Otherwise do it the other way round
         NEG.L    D0                      Make difference +ve
         MOVE.L   D6,D5                   D5 = non-normalised exponent of result
FA4      CMPI.L   #HIDDENB+1,D0           Is it worth doing?
         BLT.S    FA12                    If LT yes
         TST.L    D1                      Is answer -ve
         BLT.S    FA13                    If LT yes
         MOVEQ.L  #0,D0                   Set +ve ans
         BRA.S    FA7
FA13     MOVE.L   #SIGN,D0                Set -ve ans
         NEG.L    D1
         BRA.S    FA7
FA12     ASR.L    D0,D2                   Shift to make exp's equal
         MOVEQ.L  #0,D0
         ADDX.L   D0,D2                   Round it
FA3      ADD.L    D2,D1                   = Integer result
         BEQ.S    FA9                     0 result
         BLT.S    FA5                     Make sign of result
         MOVEQ.L  #0,D0                   Set +ve
         BRA.S    FA6
FA5      MOVE.L   #SIGN,D0                Set -ve
         NEG.L    D1                      And negate integer result
*
* Normalise-Round-Normalise
*
FA6      MOVEQ.L  #0,D2                   For the ADDX's
         MOVE.L   #HIDDEN*2,D3
         MOVE.L   #HIDDEN,D4
         CMP.L    D3,D1                   Normalised?
         BLT.S    FA8                     If LT possibly
         ADD.L    D4,D5                   Add 1 to exponent
         LSR.L    #1,D1                   Only 1 shift necessary
         ADDX.L   D2,D1                   Round
         CMP.L    D3,D1                   Still normalised?
         BLT.S    FA7                     If LT yes
         LSR.L    #1,D1                   Otherwise 1 more shift
         ADD.L    D4,D5                   Increment exponent
         BRA.S    FA7
*
FA8      CMP.L    D4,D1                   Normalise upwards
         BGE.S    FA7
         SUB.L    D4,D5
         LSL.L    #1,D1
         BRA.S    FA8
*
FA7      TST.L    D5                      Bad exponent ?
         BEQ.S    FAUO
         BMI.S    FAUO                    If MI or EQ yes
         BCLR     #HIDDENB,D1             Remove hidden result bit
         OR.L     D5,D1                   Insert exponent
FA14     OR.L     D0,D1                   And sign
         BRA.S    FAROK
*
FAUO     CMPI.L   #$C0800000,D1           Underflow?
         BGE.S    FAUV
         MOVE.L   #$7F7FFFFF,D1           Max value for overflow
         MOVE.W   #SRM_V!SRM_C,D3
         BRA.S    FAUOR
FAUV     MOVE.L   #$00800000,D1           Min value for underflow
         MOVE.W   #SRM_V,D3
FAUOR    OR.L     D0,D1
         BRA.S    FAR
*
FA9      MOVEQ.L  #0,D1                   Set zero result
FAROK    TST.L    D1                      Set flags appropriately
         GETCC
FAR      MOVEM.L  (SP)+,D2/D4/D6
         BRA      FPRET
*
*     FMUL D2,D1
*
FMUL     MOVEM.L  D2/D4/D7,-(SP)
         MOVEQ.L  #0,D0                   Ready for result
         MOVE.L   D1,D3                   Copy arg1
         BCLR     #SIGNB,D1
         SNE.B    D0                      D0 = Incoming sign of D1
         MOVE.L   D2,D7                   Copy arg2
         BCLR     #SIGNB,D2
         SNE.B    D5                      D5 = Incoming sign of D2
         MOVE.L   #EXPT,D4
         AND.L    D4,D3                   Leave the exponent
         BEQ      FM4                     If EQ res = 0
         AND.L    D4,D7                   In each arg
         BEQ      FM4                     If EQ res = 0
         NOT.L    D4                      Make mantissa mask
         AND.L    D4,D1                   Leave the mantissa
         AND.L    D4,D2                   In each arg
         EOR.B    D5,D0                   D0 = sign of result
         BEQ.S    FM1
         MOVE.L   #SIGN,D0                Set negative result
FM1      ADD.L    D3,D7                   Intermediate exponent result
         BSET     #HIDDENB,D1
         BSET     #HIDDENB,D2             Replace hidden bits
         MOVE.W   D1,D3                   Low word into D3
         MOVE.W   D2,D4                   Low word into D4
         SWAP     D1                      High word into D1
         SWAP     D2                      High word into D2
         MOVE.W   D2,D5                   And into D5
         MULU     D1,D5                   D5 = D1H * D2H
         MULU     D3,D2                   D2 = D2H * D1L
         MULU     D4,D1                   D1 = D1H * D2L
         MULU     D4,D3                   D3 = D1L * D2L
         CLR.W    D3
         SWAP     D3
         ADD.L    D2,D1                   D1 = D2H*D1L + D1H*D2L
         SWAP     D1                      Get into high word
         MOVEQ.L  #0,D2                   Ensure high part =0
         MOVE.W   D1,D2                   D2 = High part of D1
         CLR.W    D1                      Clear bottom word
         ADDX.L   D2,D5                   D1H*D2H +high(D2H*D1L+D1H*D2L)+carry
         MOVE.W   D5,D1                   16 bit shift coming up
         SWAP     D1                      Not bad turn of speed there
         ADD.L    D3,D1                   Add in D1L*D2L
*
* Normalise-Round-Normalise
*
         MOVE.L   #-EXCESS,D2             Amount to adjust the exponent by
         LSR.L    #7,D1                   Shift upper part
         CMP.L    #HIDDEN*2,D1            Normalised?
         BLT.S    FM5                     If LT yes
         ADDQ.L   #1,D2                   Increment amount to add to exp
         LSR.L    #1,D1                   Again
FM5      MOVEQ.L  #0,D3                   Set 0 so that we just add extend
         ADDX.L   D3,D1                   Round
         CMP.L    #HIDDEN*2,D1            Still normalised?
         BLT.S    FM6                     If LT yes
         ADDQ.L   #1,D2                   Increment amount to add to exp
         LSR.L    #1,D1                   Again
*
FM6      MOVEQ.L  #HIDDENB,D3
         BCLR     D3,D1                   Remove hidden bit
         LSL.L    D3,D2                   Shift up the mantissa change
         ADD.L    D2,D7                   Adjust mantissa
         BMI.S    FMUO
         BEQ.S    FMUO                    If MI or EQ -> underflow or overflow
         OR.L     D7,D1                   = Result
FM7      OR.L     D0,D1                   Insert sign
         BRA.S    FMROK                   Return to the outside world
*
FMUO     CMPI.L   #$C0800000,D7           Underflow?
         BGE.S    FMUV                    If GE yes
         MOVE.L   #$7F7FFFFF,D1           Set max value on overflow
         MOVE.W   #SRM_V!SRM_C,D3
         BRA.S    FMUOR
FMUV     MOVE.L   #$00800000,D1           Set min value on underflow
         MOVE.W   #SRM_V,D3
FMUOR    OR.L     D0,D1
         BRA.S    FMR

FM4      MOVEQ.L  #0,D1                   Zero result
FMROK    TST.L    D1
         GETCC
FMR      MOVEM.L  (SP)+,D2/D4/D7
         BRA      FPRET
*
* FDIV D2,D1
*  By the non-restorative shift & subtract method
*
FDIV     MOVEM.L  D0/D2/D4/D6/D7,-(SP)
         MOVE.L   D1,D3
         MOVE.L   D2,D4
         MOVE.L   #EXPT,D7
         AND.L    D7,D3                   Get exponent of arg1
         BEQ      FD3                     If EQ ans = 0
         AND.L    D7,D4                   And arg2
         BEQ      FD4                     If EQ ans = infinity
         SUB.L    D4,D3                   Exponent of result
         ADD.L    #EXCESS*HIDDEN,D3       Replace excess
         MOVE.L   D1,D5
         EOR.L    D2,D5                   Find sign of result
         ANDI.L   #SIGN,D5                Mask all but sign bit
         NOT.L    D7                      Make mantissa mask
         BCLR     #SIGNB,D7
         AND.L    D7,D1                   Mask all but mantissa
         AND.L    D7,D2
         BSET     #HIDDENB,D1             And replace hidden bits
         BSET     #HIDDENB,D2
         MOVEQ.L  #0,D4                   Initialise answer
*
         MOVEQ.L  #25,D7                  26 loops
FD1      SUB.L    D2,D1                   Subtract bottom from top
         BLT.S    FD9                     If LT -> can't do it this time
FD11     ADD.L    D4,D4                   Otherwise double answer
         ADD.L    D1,D1                   And numerator
         BSET     #0,D4                   Set new bit
         DBRA     D7,FD1                  Next bit
         BRA.S    FD12
*
FD10     ADD.L    D2,D1                   Add bottom to new top
         BGT.S    FD11                    If GT ok
FD9      ADD.L    D4,D4                   Otherwise double answer
         ADD.L    D1,D1                   And numerator
         DBRA     D7,FD10                 Next bit
*
* Normalise-Round-Normalise
*
FD12     CMPI.L   #HIDDEN*4,D4            How many shifts
         BLT.S    FD2                     If LT only 1
         LSR.L    #2,D4                   Otherwise shift 2 places
         BRA.S    FD5
FD2      SUBI.L   #HIDDEN,D3              Adjust exponent
         LSR.L    #1,D4
FD5      MOVEQ.L  #0,D7                   For the ADDX
         ADDX.L   D7,D4                   Round
         CMPI.L   #HIDDEN*2,D4            Still normalised?
         BLT.S    FD13                    If LT yes
         LSR.L    #1,D4                   Shift one more
         ADDI.L   #HIDDEN,D3              And adjust exponent
FD13     TST.L    D3                      Bad answer?
         BEQ.S    FDUO
         BMI.S    FDUO
         BCLR     #HIDDENB,D4             Remove hidden bit
         OR.L     D4,D3                   Insert mantissa into answer
         MOVE.L   D3,D1
FD14     OR.L     D5,D1                   And the sign
         BRA.S    FDROK
FD3      MOVEQ.L  #0,D1                   Set zero result
         BRA.S    FDROK
*
FDUO     CMPI.L   #$C0800000,D3           Underflow?
         BGE.S    FDUV                    If GE yes
         MOVE.L   #$7F7FFFFF,D1           Set max value on overflow
         MOVE.W   #SRM_V!SRM_C,D3
         BRA.S    FDUOR
FDUV     MOVE.L   #$00800000,D1           Set min value on underflow
         MOVE.W   #SRM_V,D3
FDUOR    OR.L     D5,D1
         BRA.S    FD14
*
FD4      MOVE.L   #INFINITY,D1            Set infinite result
         MOVE.W   #SRM_V!SRM_C,D3
         BRA.S    FDR
FDROK    TST.L    D1
         GETCC
FDR      MOVEM.L  (SP)+,D0/D2/D4/D6/D7
         BRA      FPRET                   And exit
*
* Single precision Negate
*
FNEG     TST.L    D1                   Is it zero
         BEQ.S    FS1R1                If EQ yes
         BCHG     #SIGNB,D1            Change sign
         SEQ.B    D3                   Get previous sign status
         ANDI.B   #SRM_N,D3            Remove all but N bit
         BRA      FPRET                Set system status and exit
FS1R1    MOVEQ.L  #SRM_Z,D3            Set zero status
         BRA      FPRET
*
* Single precision compare  FCMP D2,D1
*
FCMP     BTST     #SIGNB,D1            Is DST negative
         BNE.S    FS31                 If NE yes
         CMP.L    D2,D1                Compare with at least D1 positive
         GETCC
         BRA      FPRET
FS31     BTST     #SIGNB,D2            Is the SRC negative as well
         BNE.S    FS32                 If NE yes
         MOVEQ.L  #SRM_N,D3            Otherwise DST<SRC so set for LT
         BRA      FPRET
FS32     CMP.L    D1,D2                Do negative to negative compare
         GETCC
         BRA      FPRET
*
* Single precision test FTST D1
*
FTST     TST.L    D1                   Get status
         GETCC
         BRA      FPRET
*
* Double length operations
*
* DFSUB (D2),(D1)
*
DFSUB    MOVEM.L  A1/D1/D2/D4/D6,-(SP)
         MOVEA.L  D1,A0
         MOVEA.L  D2,A1
         MOVE.L   (A1),D3
         BCHG     #SIGNB,D3               Negate and add
         BRA      DFADD1
*
* DFADD (D2),(D1)
*
DFADD    MOVEM.L  A1/D1/D2/D4/D6,-(SP)
         MOVEA.L  D1,A0
         MOVEA.L  D2,A1
         MOVE.L   (A1),D3
DFADD1   MOVE.L   (A0),D1
         MOVE.L   D1,D2                   Copy arg1
         MOVE.L   D3,D4                   and arg2
         MOVE.L   #DEXPT,D7               Exponent mask
         AND.L    D7,D4                   In each register
         BEQ      DFAROK                  If EQ D2 = 0
         AND.L    D7,D2                   Leave exponent
         BNE.S    DFA11                   If EQ D1 = 0
         MOVE.L   D3,(A0)
         MOVE.L   4(A1),4(A0)
         BRA      DFAROK                  If EQ ans = Arg2
DFA11    MOVEM.L  D2/D4,-(SP)
         NOT.L    D7                      Make mantissa mask
         BCLR     #SIGNB,D1               Remove sign bit
         SNE.B    D5                      D5 = sign of Arg1
         BCLR     #SIGNB,D3               Remove sign bit
         SNE.B    D6                      D6 = sign of Arg2
         AND.L    D7,D1                   Leave Mantissa
         AND.L    D7,D3                   In each register
         BSET     #DHIDDENB,D1            Replace hidden bits
         BSET     #DHIDDENB,D3
         MOVE.L   4(A0),D2
         MOVE.L   4(A1),D4
         MOVEQ.L  #0,D7
         TST.B    D5                      Make 2's comp no. of Arg1
         BEQ.S    DFA1                    If EQ arg1 +ve
         NOT.L    D2
         NOT.L    D1
         ADDQ.L   #1,D2
         ADDX.L   D7,D1
DFA1     TST.B    D6                      And Arg2
         BEQ.S    DFA2                    If EQ arg2 +ve
         NOT.L    D4
         NOT.L    D3
         ADDQ.L   #1,D4
         ADDX.L   D7,D3
DFA2     MOVEM.L  (SP)+,D5/D6             Restore exponents exp1 in D5
         MOVE.L   D5,D0                   Copy D1 exponent
         SUB.L    D6,D0                   Difference of exponents
         BEQ.S    DFA3                    If EQ no shift required
         MOVEQ.L  #DHIDDENB,D7
         ASR.L    D7,D0                   Move diff to low byte
         BGT.S    DFA4                    If GT |D1| > |D2|
         EXG.L    D1,D3                   Do it the other way round
         EXG.L    D2,D4
         NEG.L    D0                      Make difference +ve
         MOVE.L   D6,D5                   D5 = non-normalised exponent of result
DFA4     CMPI.L   #DHIDDENB+32+1,D0       Is it worth doing?
         BLT.S    DFA13                   If LT yes
*
* Simple answer case -> set answer to D3,D4
*
         TST.L    D1                      Is answer -ve
         BLT.S    DFA14                   If LT yes
         MOVEQ.L  #0,D0                   Set +ve ans
         BRA.S    DFA7
DFA14    MOVE.L   #SIGN,D0                Set -ve answer
         MOVEQ.L  #0,D7                   And renegate
         NOT.L    D1
         NOT.L    D2
         ADDI.L   #1,D2
         ADDX.L   D7,D1
         BRA.S    DFA7
*
DFA13    SUBQ.L   #1,D0
DFA10    ASR.L    #1,D3                   Shift to make exp's equal
         ROXR.L   #1,D4
         DBRA     D0,DFA10
*
         MOVEQ.L  #0,D0
         ADDX.L   D0,D4                   Round before add
         ADDX.L   D0,D3
DFA3     MOVEQ.L  #0,D7
         ADD.L    D4,D2                   Lower part
         ADDX.L   D3,D1                   And upper
         BEQ      DFA9                    0 result
         BLT.S    DFA5                    Make sign of result
         MOVEQ.L  #0,D0                   Set +ve
         BRA.S    DFA6
DFA5     MOVE.L   #SIGN,D0                Set -ve
         NOT.L    D2                      And negate integer result
         NOT.L    D1
         ADDQ.L   #1,D2
         ADDX.L   D7,D1
*
* Normalise-Round-Normalise
*
DFA6     MOVE.L   #DHIDDEN*2,D3
         MOVE.L   #DHIDDEN,D4
         CMP.L    D3,D1                   Normalised?
         BLT.S    DFA8                    If LT possibly
         ADD.L    D4,D5                   Increment exp
         LSR.L    #1,D1                   Otherwise only 1 shift necessary
         ROXR.L   #1,D2
         ADDX.L   D7,D2                   Round
         ADDX.L   D7,D1
         CMP.L    D3,D1                   Still normalised?
         BLT.S    DFA7                    If LT yes
         ADD.L    D4,D5                   Increment exp
         LSR.L    #1,D1                   And shift once more
         ROXR.L   #1,D2
         BRA.S    DFA7
*
DFA8     CMP.L    D4,D1                   Normalise upwards
         BGE.S    DFA7
         SUB.L    D4,D5
         LSL.L    #1,D2
         ROXL.L   #1,D1
         BRA.S    DFA8
*
DFA7     BCLR     #DHIDDENB,D1            Remove hidden result bit
         TST.L    D5                      Bad exponent?
         BMI.S    DFAUO                   If LT yes
         BEQ.S    DFAUO
         OR.L     D5,D1                   Insert exponent
DFA15    OR.L     D0,D1                   And sign
         MOVE.L   D1,(A0)
         MOVE.L   D2,4(A0)
         BRA      DFAROK
*
DFAUO    CMPI.L   #$C0100000,D5           Underflow?
         BGE.S    DFAUV                   If GE yes
         MOVE.L   #$7FEFFFFF,D1           Set max value on overflow
         MOVEQ.L  #-1,D2
         MOVE.W   #SRM_V!SRM_C,D3
         BRA.S    DFAUOR
DFAUV    MOVE.L   #$00100000,D1           Set min value on underflow
         MOVEQ.L  #0,D2
         MOVE.W   #SRM_V,D3
DFAUOR   OR.L     D0,D1                   And sign
         MOVE.L   D1,(A0)
         MOVE.L   D2,4(A0)
         BRA      DFAR
*
DFA9     CLR.L    (A0)
         CLR.L    4(A0)
DFAROK   TST.L    (A0)
         GETCC
DFAR     MOVEM.L  (SP)+,A1/D1/D2/D4/D6
         BRA      FPRET
*
* DFMUL (D2),(D1)
*
DFMUL    MOVEM.L  A1/D1/D2/D4/D6,-(SP)
         MOVEA.L  D1,A0
         MOVEA.L  D2,A1
         MOVE.L   (A0),D1                 Get first word of arg1
         MOVE.L   D1,D2                   And copy
         BCLR     #SIGNB,D1
         SNE.B    D0                      D0 = Incoming sign of Arg1
         MOVE.L   (A1),D3                 First word of arg2
         MOVE.L   D3,D4                   And copy
         BCLR     #SIGNB,D3
         SNE.B    D5                      D5 = Incoming sign of D2
         MOVE.L   #DEXPT,D6
         AND.L    D6,D2                   Leave the exponent
         BEQ      DFM4                    If EQ res = 0
         AND.L    D6,D4                   In each arg
         BEQ      DFM4                    If EQ res = 0
         NOT.L    D6                      Make mantissa mask
         AND.L    D6,D1                   Leave the mantissa
         AND.L    D6,D3                   In each arg
         ADD.L    D2,D4                   Intermediate exponent result
         MOVE.B   D0,D4
         EOR.B    D5,D4                   D0 = sign of result
         MOVE.L   D4,-(SP)                Save it all for later
         BSET     #DHIDDENB,D1
         BSET     #DHIDDENB,D3            Replace hidden bits
         MOVE.L   4(A0),D2                Fetch the rest of each number
         MOVE.L   4(A1),D4
*
         MOVE.L   #10,D5
DFM1     LSL.L    #1,D2                   Shift everybody left 11 places
         ROXL.L   #1,D1
         LSL.L    #1,D4
         ROXL.L   #1,D3
         DBRA     D5,DFM1
*
         CLR.L    (A0)
         CLR.L    4(A0)
         MOVEQ.L  #0,D7                   For the ADDX'S
         MOVEQ.L  #0,D5
*
* Right here we go
* (A4 + A3 + A2 + A1) * (B4 + B3 + B2 + B1)
* Except for the bottom six multiplications
*
         SWAP     D1                      A4
         MOVE.W   D4,D6
         MULU     D1,D6                   A4 * B1
         CLR.W    D6
         SWAP     D6
*
         SWAP     D3                      B4
         MOVE.W   D2,D0
         MULU     D3,D0                   A1 *  B4
         CLR.W    D0
         SWAP     D0
         ADD.L    D0,D6
*
         SWAP     D1                      A3
         SWAP     D4                      B2
         MOVE.W   D4,D0
         MULU     D1,D0                   A3 * B2
         CLR.W    D0
         SWAP     D0
         ADD.L    D0,D6
*
         SWAP     D2                      A2
         SWAP     D3                      B3
         MOVE.W   D3,D0
         MULU     D2,D0                   A2 * B3
         CLR.W    D0
         SWAP     D0
         ADD.L    D0,D6
*
         MOVE.W   D3,D0
         MULU     D1,D0                   A3 * B3
         ADD.L    D0,D6
         ADDX.W   D7,D5
*
         SWAP     D1                      A4
         MOVE.W   D4,D0
         MULU     D1,D0                   A4 * B2
         ADD.L    D0,D6
         ADDX.W   D7,D5
*
         SWAP     D3                      B4
         MOVE.W   D2,D0
         MULU     D3,D0                   A2 * B4
         ADD.L    D0,D6
         ADDX.W   D7,D5
*
         MOVE.W   D6,6(A0)
         MOVE.W   D5,D6
         SWAP     D6
         CLR.W    D5
*
         SWAP     D3                      B3
         MOVE.W   D3,D0
         MULU     D1,D0                   A4 * B3
         ADD.L    D0,D6
         ADDX.W   D7,D5
*
         SWAP     D3                      B4
         SWAP     D1                      A3
         MOVE.W   D3,D0
         MULU     D1,D0                   A3 * B4
         ADD.L    D0,D6
         ADDX.W   D7,D5
*
         MOVE.W   D6,4(A0)
         MOVE.W   D5,D6
         SWAP     D6
*
         SWAP     D1                      A4
         MULU     D3,D1                   A4 * B4
         ADD.L    D1,D6
*
         MOVE.L   4(A0),D5                Unnormalised result in D6/D5
         MOVE.L   (SP)+,D0                Fetch exp + sign again
         MOVE.B   D0,D4
         CLR.B    D0
*
* Normalise-Round-Normalise
*
         MOVE.L   #-DEXCESS,D2            Amount to adjust the exponent by
         MOVE.L   #9,D1
DFM2     LSR.L    #1,D6                   Shift right 10 places
         ROXR.L   #1,D5
         DBRA     D1,DFM2
         CMP.L    #DHIDDEN*2,D6           Normalised
         BLT.S    DFM5                    If LT yes
         ADDQ.L   #1,D2                   Increment amount to add to exp
         LSR.L    #1,D6                   Shift again
         ROXR.L   #1,D5
DFM5     MOVEQ.L  #0,D1                   For the ADDX
         ADDX.L   D1,D5                   Round
         ADDX.L   D1,D6
         CMP.L    #DHIDDEN*2,D6           Still normalised?
         BLT.S    DFM7                    If LT yes
         ADDQ.L   #1,D2                   Increment amount to add to exp
         LSR.L    #1,D6                   Shift again
         ROXR.L   #1,D5
DFM7     BCLR     #DHIDDENB,D6            Remove hidden bit
         MOVE.L   #DHIDDENB,D3
         LSL.L    D3,D2                   Shift up the exponent change
         ADD.L    D2,D0                   Adjust exponent
         BMI.S    DFMUO                   If LT -> exception
         BEQ.S    DFMUO
         OR.L     D0,D6                   = Result
DFM8     TST.B    D4
         BGE.S    DFM6
         BSET     #SIGNB,D6
DFM6     MOVE.L   D6,(A0)
         MOVE.L   D5,4(A0)
         BRA      DFMROK                  Return to the outside world
*
DFMUO    CMPI.L   #$C0100000,D0           Underflow?
         BGE.S    DFMUV                   If GE yes
         MOVE.L   #$7FEFFFFF,D6           Set max value on overflow
         MOVEQ.L  #-1,D5
         MOVE.W   #SRM_V!SRM_C,D3
         BRA.S    DFMUOR
DFMUV    MOVE.L   #$00100000,D6
         MOVEQ.L  #0,D5
DFMUOR   TST.B    D4
         BGE.S    DFMU1
         BSET     #SIGNB,D6
DFMU1    MOVE.L   D6,(A0)
         MOVE.L   D5,4(A0)
         BRA      DFMR                    Return to the outside world
*
DFM4     CLR.L    (A0)
         CLR.L    4(A0)
DFMROK   TST.L    (A0)
         GETCC
DFMR     MOVEM.L  (SP)+,A1/D1/D2/D4/D6
         BRA      FPRET
*
* DFDIV (D2),(D1)
*  By the non-restorative shift & subtract method
*
DFDIV    MOVEM.L  A1/D1/D2/D4/D6/D7,-(SP)
         MOVEA.L  D1,A0
         MOVEA.L  D2,A1
         MOVEM.L  (A0),D1/D2
         MOVEM.L  (A1),D3/D4
         MOVE.L   D1,D7                   Copy arg1
         MOVE.L   D3,D6                   And arg2
         MOVE.L   #DEXPT,D5               Exponent mask
         AND.L    D5,D7                   Remove all but exponent on arg1
         BNE.S    DFD7                    If Arg1 = 0 THEN RES = 0
         MOVEQ    #0,D5
         MOVEQ    #0,D6                   Set result of zero
         BRA      DFDROK
DFD7     AND.L    D5,D6                   And on arg2
         BNE.S    DFD8
         MOVE.B   (A0),D0                 Set sign from arg1
         MOVE.L   #DINFINITY,D5           Set infinity (D6 already Zero)
         MOVE.W   #SRM_V!SRM_C,D3
         BRA      DFDUOR
DFD8     SUB.L    D6,D7                   D7 = Initial exponent of result
         ADDI.L   #DEXCESS*DHIDDEN,D7     Replace excess
         NOT.L    D5                      Make mantissa mask
         AND.L    D5,D1                   Leave sign and mantissa
         AND.L    D5,D3
         BSET     #DHIDDENB,D1            Replace hidden bits
         BSET     #DHIDDENB,D3
         BCLR     #SIGNB,D1               Remove sign bit
         SNE.B    D0                      And keep it
         BCLR     #SIGNB,D3
         SNE.B    D6
         EOR.B    D6,D0                   D0.B = sign of result
         OR.L     D7,D0                   Insert the exponent
*
         MOVEQ.L  #0,D5                   D5 = start of result
         MOVEQ.L  #0,D6
*
         MOVEQ.L  #54,D7                  55 loops
DFD1     SUB.L    D4,D2                   Subtract bottom from top
         SUBX.L   D3,D1
         BLT.S    DFD9                    If LT -> can't do it this time
DFD11    ADD.L    D6,D6                   Otherwise double answer
         ADDX.L   D5,D5
         ADD.L    D2,D2                   And numerator
         ADDX.L   D1,D1
         BSET     #0,D6                   Set new bit
         DBRA     D7,DFD1                 Next bit
         BRA.S    DFD12
*
DFD10    ADD.L    D4,D2                   Add bottom to new top
         ADDX.L   D3,D1
         BGT.S    DFD11                   If GT ok
DFD9     ADD.L    D6,D6                   Otherwise double answer
         ADDX.L   D5,D5
         ADD.L    D2,D2                   And numerator
         ADDX.L   D1,D1
         DBRA     D7,DFD10                Next bit
*
DFD12    CMPI.L   #DHIDDEN*4,D5           How much shift?
         BLT.S    DFD4                    If LT only one
         LSR.L    #1,D5                   Otherwise 2 shifts required
         ROXR.L   #1,D6                   Copy carry down
         BRA.S    DFD14
DFD4     SUBI.L   #DHIDDEN,D0             Adjust exponent
DFD14    LSR.L    #1,D5                   Shift 1 place
         ROXR.L   #1,D6
         MOVEQ.L  #0,D7
         ADDX.L   D7,D6                   Round
         ADDX.L   D7,D5
         CMPI.L   #DHIDDEN*2,D5           Still normalised?
         BLT.S    DFD13                   If LT yes
         ADDI.L   #DHIDDEN,D0             Increment exponent
         LSR.L    #1,D5                   1 more shift required
         ROXR.L   #1,D6
DFD13    TST.B    D0                      Check sign (held in D0.B)
         BEQ.S    DFD15
         CLR.B    D0                      Clear it if set
         BSET     #SIGNB,D5               And set sign bit in ans
DFD15    TST.L    D0                      Check for bad exponent
         BMI.S    DFDUO                   If LT yes
         BEQ.S    DFDUO
         BCLR     #DHIDDENB,D5            Remove hidden bit
         OR.L     D0,D5                   Insert exponent
         BRA.S    DFDROK
*
DFDUO    CMPI.L   #$C0100000,D0           Underflow?
         BGE.S    DFDUV                   If GE yes
         MOVE.L   #$7FEFFFFF,D5           Set max value on overflow
         MOVEQ.L  #-1,D6
         MOVE.W   #SRM_V!SRM_C,D3
         BRA.S    DFDUOR
DFDUV    MOVE.L   #$00100000,D5           Set min value on underflow
         MOVEQ.L  #0,D6
DFDUOR   TST.B    D0                      Negative result
         BEQ.S    DFDU1                   If EQ no
         BSET     #SIGNB,D5
DFDU1    MOVEM.L  D5/D6,(A0)
         BRA.S    DFDR
*
DFDROK   MOVEM.L  D5/D6,(A0)              Set result
         TST.L    D5
         GETCC
DFDR     MOVEM.L  (SP)+,A1/D1/D2/D4/D6/D7
         BRA      FPRET                   And exit
*
* Double length negate function DFNEG (D1)
*
DFNEG    MOVEA.L  D1,A0
         TST.W    (A0)                 Is it zero
         BEQ.S    FD1R1                If EQ yes
         BCHG     #7,(A0)
         SEQ.B    D3                   Get previous sign status
         ANDI.B   #SRM_N,D3            Remove all but N bit
         BRA      FPRET                Set system status and exit
FD1R1    MOVEQ.L  #SRM_Z,D3            Set zero status
         BRA      FPRET
*
* Double precision compare DFCMP (D2),(D1)
*
DFCMP    MOVEM.L  A1/D1,-(SP)
         MOVEA.L  D1,A0                Set addresses
         MOVEA.L  D2,A1
         BTST     #7,(A0)              Is DST negative
         BNE.S    DF31                 If NE yes
         BTST     #7,(A1)              Is SRC negative i.e. DST>SRC
         BNE.S    DF32                 If NE yes
         MOVE.L   (A0),D1              Fetch first lword of DST
         CMP.L    (A1),D1              Compare two positives
         BEQ.S    DF33                 Are these equal
         GETCC
         BRA.S    DFR
DF33     MOVE.L   4(A0),D1             Fetch second lword of DST
         CMP.L    4(A1),D1             Second half of two positives
         BEQ.S    DF36                 If they are equal ok
         GETCC
         ASL.W    #3,D3                Otherwise move C bit to N bit
         BRA.S    DFR
DF32     CLR.B    D3                   Set for GT i.e. DST>SRC
         BRA.S    DFR
DF31     BTST     #7,(A1)              DST -ve , is SRC -ve as well
         BNE.S    DF34                 If NE yes
         MOVEQ.L  #SRM_N,D3            Otherwise DST<SRC
         BRA.S    DFR
DF34     MOVE.L   (A1),D1              Fetch first lword of SRC
         CMP.L    (A0),D1              Compare two negatives
         BEQ.S    DF35                 Are these equal
         GETCC
         BRA.S    DFR
DF35     MOVE.L   4(A1),D1             Fetch second lword of SRC
         CMP.L    4(A0),D1             Second half of two negatives
         BEQ.S    DF36                 If they are equal ok
         GETCC
         ASL.W    #3,D3                Otherwise move C bit to N bit
         BRA.S    DFR
DF36     MOVEQ.L  #SRM_Z,D3            Set zero status
DFR      MOVEM.L  (SP)+,A1/D1
         BRA.S    FPRET
*
* Double precision test DFTST (D1)
*
DFTST    MOVEA.L  D1,A0                Set address of operand
         TST.W    (A0)                 Get status
         GETCC
*
* Set up status from D3
*
FPRET    MOVE.B   D3,NREGS*4+1(SP)     Set in stack for return
         MOVEM.L  (SP)+,A0/D0/D3/D5    Restore registers
*
          IFND    TRIPOS
          ADDI.L   #10,2(SP)            Increment past naughty instruction
          RTR                           That was a long instruction
          ENDC
*
          IFD     TRIPOS
          ADDI.L  #2,2(SP)              Increment past naughty instruction
          RTE
          ENDC
*
*
          IFD     TRIPOS
BADTRAP   MOVE.L   #85,D0
          TRAP     #0
          BRA.S    FPRET
          ENDC
*
          IFND     TRIPOS
BADTRAP   DC.W     $FFFF        
          ENDC
*
*  The next six routines are callable from BCPL
*
* Convert Integer to IEEE single precision floating format
*
* F := FLOAT(I)
*
         CNOP     0,4
         DC.L     LIBWORD
         DC.B     7,'Float  '
FLOAT    TST.L    D1                      Is it negative
         BEQ.S    FL4                     If zero leave alone
         SLT.B    D2                      Save negative status of arg
         BGE.S    FL1                     Negate if negative
         NEG.L    D1                      So we have now a positive arg
FL1      MOVEQ.L  #31,D3                  Max No. of bits to shift
FL2      LSL.L    #1,D1
         DBCS     D3,FL2                  Shift until we get a carry
*
* D3 = exponent now
*
FL3      LSR.L    #8,D1                   Put the mantissa in its right place
         LSR.L    #1,D1
         ADDI.B   #EXCESS,D3              Make biassed exponent
         MOVEQ.L  #HIDDENB,D4
         ASL.L    D4,D3                   Put exponent in its right place
         OR.L     D3,D1                   And put it in
         TST.B    D2                      Was it negative to start with
         BEQ.S    FL4                     If EQ no
         BSET     #SIGNB,D1               Otherwise set N
FL4      JMP      (R)                     All done
*
*  Convert IEEE single precision floating point no.
*   to Integer
*  I := FIX(F)
*
         CNOP     0,4
         DC.L     LIBWORD
         DC.B     7,'Fix    '
FIX      BCLR     #SIGNB,D1               Ensure positive no.
         SNE.B    D2                      Save negative status
         MOVE.L   D1,D3
         ANDI.L   #EXPT,D3                Remove all but exponent
         BEQ.S    FI1                     If EQ return 0
         ANDI.L   #MANT,D1                Remove all but mantissa
         MOVEQ.L  #HIDDENB,D4
         ASR.L    D4,D3                   Get exponent in its correct place
         SUBI.L   #EXCESS+23,D3           And value
         BSET     #HIDDENB,D1             Replace hidden bit
         TST.L    D3                      Negative shifted required ?
         BLT.S    FI2                     If LT yes
         BEQ.S    FI3                     If EQ no shift required
         CMPI.L   #7,D3                   Overflow?
         BGT.S    FI4                     If GT yes
         ASL.L    D3,D1                   Do the left shift
         BRA.S    FI3
FI2      NEG.L    D3                      Convert to positive right shift
         CMPI.L   #HIDDENB+1,D3           Will it underflow ?
         BGE.S    FI5
         ASR.L    D3,D1                   Shift
FI3      TST.B    D2                      Was it negative?
         BEQ.S    FI1                     If EQ no
         NEG.L    D1
FI1      JMP      (R)                     And exit
FI5      CLR.L    D1
         JMP      (R)
*
FI4      
*        TRAP     #0                      Enter debugger
         MOVE.L   #$7FFFFFFF,D1           Set max no.
         BRA.S    FI3                     And sign
*
* Global definitions table
*
         CNOP     0,4
         DC.L     0
         DC.L     (G_FIX/4),(FIX-FPSECT)
         DC.L     (G_FLOAT/4),(FLOAT-FPSECT)
         DC.L     (G_FPSIM/4),(FPSIM-FPSECT)
         DC.L     G_FPSIM/4
FPEND    END

