*******************************************
* (C)  COPYRIGHT 1980 BY MOTOROLA INC.    *
*******************************************

* Only uncomment one of these...
FFP_MUL2	equ	1
*FFP_MUL		equ	1

 ifd	FFP_MUL2

********************************************
*          FFPMUL2 SUBROUTINE              *
*                                          *
*   THIS MODULE IS THE SECOND OF THE       *
*   MULTIPLY ROUTINES.  IT IS 18% SLOWER   *
*   BUT PROVIDES THE HIGHEST ACCURACY      *
*   POSSIBLE.  THE ERROR IS EXACTLY .5     *
*   LEAST SIGNIFICANT BIT VERSUS AN ERROR  *
*   IN THE HIGH-SPEED DEFAULT ROUTINE OF   *
*   .50390625 LEAST SIGNIFICANT BIT DUE    *
*   TO TRUNCATION.                         *
*                                          *
* INPUT:                                   *
*          D1 - FLOATING POINT MULTIPLIER  *
*          D0 - FLOATING POINT MULTIPLICAND*
*                                          *
* OUTPUT:                                  *
*          D0 - FLOATING POINT RESULT      *
*                                          *
* REGISTERS D3 THRU D5 ARE CHANGED         *
*                                          *
* CONDITION CODES:                         *
*          N - SET IF RESULT NEGATIVE      *
*          Z - SET IF RESULT IS ZERO       *
*          V - SET IF OVERFLOW OCCURRED    *
*          C - UNDEFINED                   *
*          X - UNDEFINED                   *
*                                          *
* CODE: 134 BYTES    STACK WORK: 0 BYTES   *
*                                          *
* NOTES:                                   *
*   1) MULTIPIER UNALTERED (D1).           *
*   2) UNDERFLOWS RETURN ZERO WITH NO      *
*      INDICATOR SET.                      *
*   3) OVERFLOWS WILL RETURN THE MAXIMUM   *
*      VALUE WITH THE PROPER SIGN AND THE  *
*      'V' BIT SET IN THE CCR.             *
*                                          *
*  TIMES: (8MHZ NO WAIT STATES ASSUMED)    *
* ARG1 ZERO            5.750 MICROSECONDS  *
* ARG2 ZERO            3.750 MICROSECONDS  *
* MINIMUM TIME OTHERS 45.750 MICROSECONDS  *
* MAXIMUM TIME OTHERS 61.500 MICROSECONDS  *
* AVERAGE OTHERS      52.875 MICROSECONDS  *
*                                          *
********************************************
******* mathffp.library/SPMul ******
*
*NAME
*	SPMul -- Multiply two floating point numbers.
*
*SYNOPSIS
*	fnum3 = SPMul(fnum1, fnum2)
*	D0	      D1     D0
*
*	float SPMul(float fnum1, float fnum2);
*
*FUNCTION
*	Accepts two floating point numbers and returns the arithmetic
*	multiplication of said numbers.
*
*INPUTS
*	fnum1 	- floating point number
*	fnum2 	- floating point number
*
*RESULT
*	fnum3 	- floating point number
*
*BUGS
*	None
*
*SEE ALSO
*
******/
       PAGE
*FFPMUL2  IDNT  1,1 FFP HIGH-PRECISION MULTIPLY

       XDEF     FFPMUL       ENTRY POINT
	   XDEF		MFSPMul

*      XREF     FFPCPYRT     COPYRIGHT STUB


* FFPMUL SUBROUTINE ENTRY POINT
FFPMUL:
MFSPMul:
	   MOVEM.L D3-D5,-(SP)
	   MOVE.B D0,D5     PREPARE SIGN/EXPONENT WORK      4
       BEQ.S  FFMRTN    RETURN IF RESULT ALREADY ZERO   8/10
       MOVE.B D1,D4     COPY ARG1 SIGN/EXPONENT         4
       BEQ.S  FFMRT0    RETURN ZERO IF ARG1=0           8/10
       ADD.W  D5,D5     SHIFT LEFT BY ONE               4
       ADD.W  D4,D4     SHIFT LEFT BY ONE               4
       MOVEQ  #-128,D3  PREPARE EXPONENT MODIFIER $80   4
       EOR.B  D3,D4     ADJUST ARG1 EXPONENT TO BINARY  4
       EOR.B  D3,D5     ADJUST ARG2 EXPONENT TO BINARY  4
       ADD.B  D4,D5     ADD EXPONENTS                   4
       BVS.S  FFMOUF    BRANCH IF OVERFLOW/UNDERFLOW    8/10
       MOVE.B D3,D4     OVERLAY $80 CONSTANT INTO D4    4
       EOR.W  D4,D5     D5 NOW HAS SIGN AND EXPONENT    4
       ROR.W  #1,D5     MOVE TO LOW 8 BITS              8
       SWAP.W D5        SAVE FINAL S+EXP IN HIGH WORD   4
       MOVE.W D1,D5     COPY ARG1 LOW BYTE              4
       CLR.B  D0        CLEAR S+EXP OUT OF ARG2         4
       CLR.B  D5        CLEAR S+EXP OUT OF ARG1LOWB     4
       MOVE.W D5,D4     PREPARE ARG1LOWB FOR MULTIPLY   4
       MULU.W D0,D4     D4 = ARG2LOWB X ARG1LOWB        38-54(46)
       SWAP.W D4        PLACE RESULT IN LOW WORD        4
       MOVE.L D0,D3     COPY ARG2                       4
       SWAP.W D3        TO ARG2HIGHW                    4
       MULU.W D5,D3     D3 = ARG1LOWB X ARG2HIGHW       38-54(46)
       ADD.L  D3,D4     D4 = PARTIAL PRODUCT (NO CARRY) 8
       SWAP.W D1        TO ARG1 HIGH TWO BYTES          4
       MOVE.L D1,D3     COPY ARG1HIGHW OVER             4
       MULU.W D0,D3     D3 = ARG2LOWB X ARG1HIGHW       38-54(46)
       ADD.L  D3,D4     D4 = PARTIAL PRODUCT            8
       CLR.W  D4        CLEAR LOW END RUNOFF            4
       ADDX.B D4,D4     SHIFT IN CARRY IF ANY           4
       SWAP.W D4        PUT CARRY INTO HIGH WORD        4
       SWAP.W D0        NOW TOP OF ARG2                 4
       MULU.W D1,D0     D0 = ARG1HIGHW X ARG2HIGHW      40-70(54)
       SWAP.W D1        RESTORE ARG1                    4
       SWAP.W D5        RESTORE S+EXP TO LOW WORD       4
       ADD.L  D4,D0     ADD PARTIAL PRODUCTS            8
       BPL.S  FFMNOR    BRANCH IF MUST NORMALIZE        8/10
       ADD.L  #$80,D0   ROUND UP (CANNOT OVERFLOW)      16
       MOVE.B D5,D0     INSERT SIGN AND EXPONENT        4
       BEQ.S  FFMRT0    RETURN ZERO IF ZERO EXPONEN     8/10
FFMRTN:
	   MOVEM.L (SP)+,D3-D5
	   RTS              RETURN TO CALLER                16

* MUST NORMALIZE RESULT
FFMNOR SUBQ.B  #1,D5    BUMP EXPONENT DOWN BY ONE       4
       BVS.S   FFMRT0   RETURN ZERO IF UNDERFLOW        8/10
       BCS.S   FFMRT0   RETURN ZERO IF SIGN INVERTED    8/10
       MOVEQ   #$40,D4  ROUNDING FACTOR                 4
       ADD.L   D4,D0    ADD IN ROUNDING FACTOR          8
       ADD.L   D0,D0    SHIFT TO NORMALIZE              8
       BCC.S   FFMCLN   RETURN NORMALIZED NUMBER        8/10
       ROXR.L  #1,D0    ROUNDING FORCED CARRY IN MS BIT 10
       ADDQ.B  #1,D5    UNDO NORMALIZE ATTEMPT          4
FFMCLN MOVE.B  D5,D0    INSERT SIGN AND EXPONENT        4
       BEQ.S   FFMRT0   RETURN ZERO IF EXPONENT ZERO    8/10
	   MOVEM.L (SP)+,D3-D5
       RTS              RETURN                          16

* ARG1 ZERO
FFMRT0 MOVEQ  #0,D0     LOAD ZERO                       4
	   MOVEM.L (SP)+,D3-D5
       RTS              RETURN                          16

* OVERFLOW OR UNDERFLOW EXPONENT
FFMOUF BPL.S  FFMRT0    BRANCH IF UNDERFLOW             8/10
       EOR.B  D1,D0     CALCULATE PROPER SIGN           4
       OR.L   #$FFFFFF7F,D0 SET MAX VALUE               16
       TST.B  D0        SET SIGN BIT                    4
*      ORI     #$02,CCR SET OVERFLOW BIT                20
       DC.L   $003C0002 ** ASSEMBLER ERROR **           20
	   MOVEM.L (SP)+,D3-D5
       RTS              RETURN                          16

 endc	; End of slower but better multiply...

 ifd	FFP_MUL

********************************************
*          FFPMUL  SUBROUTINE              *
*                                          *
* INPUT:                                   *
*          D1 - FLOATING POINT MULTIPLIER  *
*          D0 - FLOATING POINT MULTIPLICAND*
*                                          *
* OUTPUT:                                  *
*          D0 - FLOATING POINT RESULT      *
*                                          *
*                                          *
* CONDITION CODES:                         *
*          N - SET IF RESULT NEGATIVE      *
*          Z - SET IF RESULT IS ZERO       *
*          V - SET IF OVERFLOW OCCURRED    *
*          C - UNDEFINED                   *
*          X - UNDEFINED                   *
*                                          *
* REGISTERS D3 THRU D5 ARE CHANGED         *
*                                          *
* SIZE: 122 BYTES    STACK WORK: 0 BYTES   *
*                                          *
* NOTES:                                   *
*   1) MULTIPIER UNALTERED (D1).           *
*   2) UNDERFLOWS RETURN ZERO WITH NO      *
*      INDICATOR SET.                      *
*   3) OVERFLOWS WILL RETURN THE MAXIMUM   *
*      VALUE WITH THE PROPER SIGN AND THE  *
*      'V' BIT SET IN THE CCR.             *
*   4) THIS VERSION OF THE MULTIPLY HAS A  *
*      SLIGHT ERROR DUE TO TRUNCATION      *
*      OF .00390625 IN THE LEAST SIGNIFIC- *
*      ANT BIT.  THIS AMOUNTS TO AN AVERAGE*
*      OF 1 INCORRECT LEAST  SIGNIFICANT   *
*      BIT RESULT FOR EVERY 512 MULTIPLIES.*
*                                          *
*  TIMES: (8MHZ NO WAIT STATES ASSUMED)    *
* ARG1 ZERO            5.750 MICROSECONDS  *
* ARG2 ZERO            3.750 MICROSECONDS  *
* MINIMUM TIME OTHERS 38.000 MICROSECONDS  *
* MAXIMUM TIME OTHERS 51.750 MICROSECONDS  *
* AVERAGE OTHERS      44.125 MICROSECONDS  *
*                                          *
********************************************
       PAGE

*FFPMUL IDNT  1,1  FFP MULTIPLY

       XDEF   FFPMUL     ENTRY POINT
	   XDEF	  MFSPMul

*      XREF   FFPCPYRT   COPYRIGHT STUB


* FFPMUL SUBROUTINE ENTRY POINT
FFPMUL:
MFSPMul:
	   MOVEM.L D3-D5,-(SP)
	   MOVE.B D0,D5     PREPARE SIGN/EXPONENT WORK      4
       BEQ.S  FFMRTN    RETURN IF RESULT ALREADY ZEO    8/10
       MOVE.B D1,D4     COPY ARG1 SIGN/EXPONENT         4
       BEQ.S  FFMRT0    RETURN ZERO IF ARG1=0           8/10
       ADD.W  D5,D5     SHIFT LEFT BY ONE               4
       ADD.W  D4,D4     SHIFT LEFT BY ONE               4
       MOVEQ  #-128,D3  PREPARE EXPONENT MODIFIER $80   4
       EOR.B  D3,D4     ADJUST ARG1 EXPONENT TO BINARY  4
       EOR.B  D3,D5     ADJUST ARG2 EXPONENT TO BINARY  4
       ADD.B  D4,D5     ADD EXPONENTS                   4
       BVS.S  FFMOUF    BRANCH IF OVERFLOW/UNDERFLOW    8/10
       MOVE.B D3,D4     OVERLAY $80 CONSTANT INTO D4    4
       EOR.W  D4,D5     D5 NOW HAS SIGN AND EXPONENT    4
       ROR.W  #1,D5     MOVE TO LOW 8 BITS              8
       CLR.B  D0        CLEAR S+EXP OUT OF ARG2         4
       MOVE.L D0,D3     PREPARE ARG2 FOR MULTIPLY       4
       SWAP.W D3        USE TOP TWO SIGNIFICANT BYTES   4
       MOVE.L D1,D4     COPY ARG1                       4
       CLR.B  D4        CLEAR LOW BYTE (S+EXP)          4
       MULU.W D4,D3     A3 X B1B2                       38-54 46)
       SWAP.W D4        TO ARG1 HIGH TWO BYTES          4
       MULU.W D0,D4     B3 X A1A2                       38-54(46)
       ADD.L  D3,D4     ADD PARTIAL PRODUCTS R3R4R5     8
       CLR.W  D4        CLEAR LOW END RUNOFF            4
       ADDX.B D4,D4     SHIFT IN CARRY IF ANY           4
       SWAP.W D4        PUT CARRY INTO HIGH WORD        4
       SWAP.W D0        NOW TOP OF ARG2                 4
       SWAP.W D1        AND TOP OF ARG1                 4
       MULU.W D1,D0     A1A2 X B1B2                     40-70(54)
       SWAP.W D1        RESTORE ARG1                    4
       ADD.L  D4,D0     ADD PARTIAL PRODUCTS            8
       BPL.S  FFMNOR    BRANCH IF MUST NORMALIZE        8/10
FFMCON ADD.L  #$80,D0   ROUND UP (CANNOT OVERFLOW)      16
       MOVE.B D5,D0     INSERT SIGN AND EXPONENT        4
       BEQ.S  FFMRT0    RETURN ZERO IF ZERO EXPONENT    8/10
FFMRTN:
	   MOVEM.L (SP)+,D3-D5
	   RTS              RETURN                          16

* MUST NORMALIZE RESULT
FFMNOR SUBQ.B  #1,D5    BUMP EXPONENT DOWN BY ONE       4
       BVS.S   FFMRT0   RETURN ZERO IF UNDERFLOW        8/10
       BCS.S   FFMRT0   RETURN ZERO IF SIGN INVERTED    8/10
       MOVEQ   #$40,D4  ROUNDING FACTOR                 4
       ADD.L   D4,D0    ADD IN ROUNDING FACTOR          8
       ADD.L   D0,D0    SHIFT TO NORMALIZE              8
       BCC.S   FFMCLN   RETURN NORMALIZED NUMBER        8/10
       ROXR.L  #1,D0    ROUNDING FORCED CARRY IN TOPBIT 10
       ADDQ.B  #1,D5    UNDO NORMALIZE ATTEMPT          4
FFMCLN MOVE.B  D5,D0    INSERT SIGN AND EXPONENT        4
       BEQ.S   FFMRT0   RETURN ZERO IF EXPONENT ZER     8/10
	   MOVEM.L (SP)+,D3-D5
       RTS              RETURN                          16

* ARG1 ZERO
FFMRT0 MOVEQ  #0,D0     LOAD ZERO                       4
	   MOVEM.L (SP)+,D3-D5
       RTS              RETURN                          16

* OVERFLOW OR UNDERFLOW EXPONENT
FFMOUF BPL.S  FFMRT0    BRANCH IF UNDERFLOW             8/10
       EOR.B  D1,D0     CALCULATE PROPER SIGN           4
       OR.L   #$FFFFFF7F,D0 SET MAX VALUE               16
       TST.B  D0        SET SIGN BIT                    4
*      ORI    #$02,CCR  SET OVERFLOW BIT                20
       DC.L   $003C0002 ****SICK ASSEMBLER****          20
	   MOVEM.L (SP)+,D3-D5
       RTS              RETURN                          16

 endc	; End of faster multiply...

       END
