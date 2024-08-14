*      TTL     FAST FLOATING POINT ADD/SUBTRACT (FFPADD/FFPSUB)
***************************************
* (C) COPYRIGHT 1980 BY MOTOROLA INC. *
***************************************

******************************************************
*                  FFPADD/FFPSUB                     *
*             FAST FLOATING POINT ADD/SUBTRACT       *
*                                                    *
*  INPUT:                                            *
*      FFPADD                                        *
*          D1 - FLOATING POINT ADDEND                *
*          D0 - FLOATING POINT ADDER                 *
*      FFPSUB                                        *
*          D1 - FLOATING POINT SUBTRAHEND            *
*          D0 - FLOATING POINT MINUEND               *
*                                                    *
*  OUTPUT:                                           *
*          D0 - FLOATING POINT ADD RESULT            *
*                                                    *
*  CONDITION CODES:                                  *
*          N - RESULT IS NEGATIVE                    *
*          Z - RESULT IS ZERO                        *
*          V - OVERFLOW HAS OCCURRED                 *
*          C - UNDEFINED                             *
*          X - UNDEFINED                             *
*                                                    *
*           REGISTERS D3 THRU D5 ARE VOLATILE        *
*                                                    *
*  CODE SIZE: 228 BYTES    STACK WORK AREA: 0 BYTES  *
*                                                    *
*  NOTES:                                            *
*    1) ADDEND/SUBTRAHEND UNALTERED (D1).            *
*    2) UNDERFLOW RETURNS ZERO AND IS UNFLAGGED.     *
*    3) OVERFLOW RETURNS THE HIGHEST VALUE WITH THE  *
*       CORRECT SIGN AND THE 'V' BIT SET IN THE CCR  *
*                                                    *
*  TIME: (8 MHZ NO WAIT STATES ASSUMED)              *
*  ALL TIMES ARE UNITS OF MICROSECONDS               *
*                                                    *
*           COMPOSITE AVERAGE  20.625                *
*                                                    *
*  ADD:         ARG1=0          7.75                 *
*               ARG2=0          5.25                 *
*                                                    *
*          LIKE SIGNS  14.50 - 26.00                 *
*                    AVERAGE   18.00                 *
*         UNLIKE SIGNS 20.13 - 54.38                 *
*                    AVERAGE   22.00                 *
*                                                    *
*  SUBTRACT:    ARG1=0          4.25                 *
*               ARG2=0          9.88                 *
*                                                    *
*          LIKE SIGNS  15.75 - 27.25                 *
*                    AVERAGE   19.25                 *
*         UNLIKE SIGNS 21.38 - 55.63                 *
*                    AVERAGE   23.25                 *
*                                                    *
******************************************************
******* mathffp.library/SPAdd ******
*
*NAME
*	SPAdd -- Add two floating point numbers.
*
*SYNOPSIS
*	fnum3 = SPAdd(fnum1, fnum2)
*	D0	      D1     D0
*
*	float SPAdd(float fnum1, float fnum2);
*
*FUNCTION
*	Accepts two floating point numbers and returns the arithmetic
*	sum of said numbers.
*
*INPUTS
*	fnum1 	- floating point number to add.
*	fnum2 	- other floating point number to add.
*
*RESULT
*	fnum3 	- floating point number, sum of fnum1 and fnum2.
*
*BUGS
*	None.
*
*SEE ALSO
*
******/
         PAGE
*FFPADD IDNT    1,1  FFP ADD/SUBTRACT

*       OPT     PCS

       XDEF    FFPADD,MFSPAdd	ENTRY POINTS
	   XDEF	   FFPSUB,MFSPSub

*      XREF    FFPCPYRT         COPYRIGHT STUB

******* mathffp.library/SPSub ******
*
*NAME
*	SPSub -- Subtract two floating point numbers.
*
*SYNOPSIS
*	fnum3 = SPSub(fnum1, fnum2)
*	D0	      D1     D0
*
*	float SPSub(float fnum1, float fnum2);
*
*FUNCTION
*	Accepts two floating point numbers and returns the arithmetic
*	subtraction of said numbers.
*
*INPUTS
*	fnum1 	- floating point number.
*	fnum2 	- floating point number.
*
*RESULT
*	fnum3 	- floating point number.
*
*BUGS
*	None.
*
*SEE ALSO
*
*******/

************************
* SUBTRACT ENTRY POINT *
************************
FFPSUB:
MFSPSub:
		 MOVEM.L D3-D5,-(SP)
	     MOVE.B  D1,D4    TEST ARG1
         BEQ.S   FPART2   RETURN ARG2 IF ARG1 ZERO
         EORI.B  #$80,D4  INVERT COPIED SIGN OF ARG1
         BMI     FPAMI1   BRANCH ARG1 MINUS
* + ARG1
         MOVE.B  D0,D5    COPY AND TEST ARG2
         BMI     FPAMS    BRANCH ARG2 MINUS
         BNE.S   FPALS    BRANCH POSITIVE NOT ZERO
         BRA.S   FPART1   RETURN ARG1 SINCE ARG2 IS ZERO

*******************
* ADD ENTRY POINT *
*******************
FFPADD:
MFSPAdd:
		 MOVEM.L D3-D5,-(SP)
	     MOVE.B  D1,D4    TEST ARG1
         BMI.S   FPAMI1   BRANCH IF ARG1 MINUS
         BEQ.S   FPART2   RETURN ARG2 IF ZERO

* + ARG1
         MOVE.B  D0,D5    TEST ARG2
         BMI.S   FPAMS    BRANCH IF MIXED SIGNS
         BEQ.S   FPART1   ZERO SO RETURN ARG1

* +ARG1 +ARG2
* -ARG1 -ARG2
FPALS    SUB.B   D4,D5    TEST EXPONENT MAGNITUDES
         BMI.S   FPA2LT   BRANCH ARG1 GREATER
         MOVE.B  D0,D4    SETUP STRONGER S+EXP IN D4

* ARG1EXP <= ARG2EXP
         CMP.B   #24,D5   OVERBEARING SIZE
         BCC.S   FPART2   BRANCH YES, RETURN ARG2
         MOVE.L  D1,D3    COPY ARG1
         CLR.B   D3       CLEAN OFF SIGN+EXPONENT
         LSR.L   D5,D3    SHIFT TO SAME MAGNITUDE
         MOVE.B  #$80,D0  FORCE CARRY IF LSB-1 ON
         ADD.L   D3,D0    ADD ARGUMENTS
         BCS.S   FPA2GC   BRANCH IF CARRY PRODUCED
FPARSR   MOVE.B  D4,D0    RESTORE SIGN/EXPONENT
		 MOVEM.L (SP)+,D3-D5
         RTS              RETURN

* ADD SAME SIGN OVERFLOW NORMALIZATION
FPA2GC   ROXR.L  #1,D0    SHIFT CARRY BACK INTO RESULT
         ADDQ.B  #1,D4    ADD ONE TO EXPONENT
         BVS.S   FPA2OS   BRANCH OVERFLOW
         BCC.S   FPARSR   BRANCH IF NO EXPONENT OVERFLOW
FPA2OS   MOVEQ   #-1,D0   CREATE ALL ONES
         SUBQ.B  #1,D4    BACK TO HIGHEST EXPONENT SIGN
         MOVE.B  D4,D0    REPLACE IN RESULT
*        ORI     #$02,CCR SHOW OVERFLOW OCCURRED
         DC.L    $003C0002 ****ASSEMBLER ERROR****
		 MOVEM.L (SP)+,D3-D5
         RTS              RETURN

* RETURN ARGUMENT1
FPART1   MOVE.L  D1,D0    MOVE IN AS RESULT
         MOVE.B  D4,D0    MOVE IN PREPARED SIGN+EXPNENT
		 MOVEM.L (SP)+,D3-D5
         RTS              RETURN

* RETURN ARGUMENT2
FPART2   TST.B   D0       TEST FOR RETURNED VALUE
		 MOVEM.L (SP)+,D3-D5
         RTS              RETURN

* -ARG1EXP > -ARG2EXP
* +ARG1EXP > +ARG2EXP
FPA2LT   CMP.B   #-24,D5  ? ARGUMENTS WITHIN RANGE
         BLE.S   FPART1   NOPE, RETURN LARGER
         NEG.B   D5       CHANGE DIFFERENCE TO POSIIVE
         MOVE.L  D1,D3    SETUP LARGER VALUE
         CLR.B   D0       CLEAN OFF SIGN+EXPONENT
         LSR.L   D5,D0    SHIFT TO SAME MAGNITUDE
         MOVE.B  #$80,D3  FORCE CARRY IF LSB-1 ON
         ADD.L   D3,D0    ADD ARGUMENTS
         BCS.S   FPA2GC   BRANCH IF CARRY PRODUCED
         MOVE.B  D4,D0    RESTORE SIGN/EXPONENT
		 MOVEM.L (SP)+,D3-D5
         RTS              RETURN

* -ARG1
FPAMI1   MOVE.B  D0,D5    TEST ARG2'S SIGN
         BMI.S   FPALS    BRANCH FOR LIKE SIGNS
         BEQ.S   FPART1   IF ZERO RETURN ARGUMENT1

* -ARG1 +ARG2
* +ARG1 -ARG2
FPAMS    MOVEQ   #-128,D3  CREATE A CARRY MASK ($80)
         EOR.B   D3,D5    STRIP SIGN OFF ARG2 S+EXP COPY
         SUB.B   D4,D5    COMPARE MAGNITUDES
         BEQ.S   FPAEQ    BRANCH EQUAL MAGNITUDES
         BMI.S   FPATLT   BRANCH IF ARG1 LARGER
* ARG1 <= ARG2
         CMP.B   #24,D5   COMPARE MAGNITUDE DIFFERENCE
         BCC.S   FPART2   BRANCH ARG2 MUCH BIGGER
         MOVE.B  D0,D4    ARG2 S+EXP DOMINATES
         MOVE.B  D3,D0    SETUP CARRY ON ARG2
         MOVE.L  D1,D3    COPY ARG1
FPAMSS   CLR.B   D3       CLEAR EXTRANEOUS BITS
         LSR.L   D5,D3    ADJUST FOR MAGNITUDE
         SUB.L   D3,D0    SUBTRACT SMALLER FROM LARGER
         BMI.S   FPARSR   RETURN FINAL RESULT IF NO OVERFLOW

* MIXED SIGNS NORMALIZE
FPANOR   MOVE.B  D4,D5    SAVE CORRECT SIGN
FPANRM   CLR.B   D0       CLEAR SUBTRACT RESIDUE
         SUBQ.B  #1,D4    MAKE UP FOR FIRST SHIFT
         CMP.L   #$00007FFF,D0 ? SMALL ENOUGH FOR SWAP
         BHI.S   FPAXQN   BRANCH IF NOT
         SWAP.W  D0       SHIFT LEFT 16 BITS FAST
         SUB.B   #16,D4   COUNT 16 BIT SHIFT
FPAXQN   ADD.L   D0,D0    SHIFT UP ONE BIT
         DBMI    D4,FPAXQN DECREMENT AND BRANCH IF POSITIVE
         EOR.B   D4,D5    ? SAME SIGN
         BMI.S   FPAZRO   BRANCH UNDERFLOW TO ZERO
         MOVE.B  D4,D0    RESTORE SIGN/EXPONENT
         BEQ.S   FPAZRO   RETURN ZERO IF EXPONENT UNDERFLOWED
		 MOVEM.L (SP)+,D3-D5
         RTS              RETURN

* EXPONENT UNDERFLOWED - RETURN ZERO
FPAZRO   MOVEQ   #0,D0    CREATE A TRUE ZERO
		 MOVEM.L (SP)+,D3-D5
         RTS              RETURN
* ARG1 > ARG2
FPATLT   CMP.B   #-24,D5  ? ARG1 >> ARG2
         BLE     FPART1   RETURN IT IF SO
         NEG.B   D5       ABSOLUTIZE DIFFERENCE
         MOVE.L  D0,D3    MOVE ARG2 AS LOWER VALUE
         MOVE.L  D1,D0    SETUP ARG1 AS HIGH
         MOVE.B  #$80,D0  SETUP ROUNDING BIT
         BRA.S   FPAMSS   PERFORM THE ADDITION

* EQUAL MAGNITUDES
FPAEQ    MOVE.B  D0,D5    SAVE ARG1 SIGN
         EXG.L   D5,D4    SWAP ARG2 WITH ARG1 S+EXP
         MOVE.B  D1,D0    INSURE SAME LOW BYTE
         SUB.L   D1,D0    OBTAIN DIFFERENCE
         BEQ.S   FPAZRO   RETURN ZERO IF IDENTICAL
         BPL.S   FPANOR   BRANCH IF ARG2 BIGGER
         NEG.L   D0       CORRECT DIFFERENCE TO POSITIVE
         MOVE.B  D5,D4    USE ARG2'S SIGN+EXPONENT
         BRA.S   FPANRM   AND GO NORMALIZE


         END
