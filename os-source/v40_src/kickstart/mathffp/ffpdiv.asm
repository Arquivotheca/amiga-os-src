*        TTL       FAST FLOATING POINT DIVIDE (FFPDIV)
*****************************************
*  (C) COPYRIGHT 1980 BY MOTOROLA INC.  *
*****************************************

********************************************
*           FFPDIV SUBROUTINE              *
*                                          *
* INPUT:                                   *
*        D1 - FLOATING POINT DIVISOR       *
*        D0 - FLOATING POINT DIVIDEND      *
*                                          *
* OUTPUT:                                  *
*        D0 - FLOATING POINT QUOTIENT      *
*                                          *
* CONDITION CODES:                         *
*        N - SET IF RESULT NEGATIVE        *
*        Z - SET IF RESULT ZERO            *
*        V - SET IF RESULT OVERFLOWED      *
*        C - UNDEFINED                     *
*        X - UNDEFINED                     *
*                                          *
* REGISTERS D3 THRU D5 ARE CHANGED         *
*                                          *
* CODE: 150 BYTES     STACK WORK: 0 BYTES  *
*                                          *
* NOTES:                                   *
*   1) DIVISOR IS UNALTERED (D1).          *
*   2) UNDERFLOWS RETURN ZERO WITHOUT      *
*      ANY INDICATORS SET.                 *
*   3) OVERFLOWS RETURN THE HIGHEST VALUE  *
*      WITH THE PROPER SIGN AND THE 'V'    *
*      BIT SET IN THE CCR.                 *
*   4) IF A DIVIDE BY ZERO IS ATTEMPTED    *
*      THE DIVIDE BY ZERO EXCEPTION TRAP   *
*      IS FORCED BY THIS CODE WITH THE     *
*      ORIGINAL ARGUMENTS INTACT.  IF THE  *
*      EXCEPTION RETURNS WITH THE DENOM-   *
*      INATOR ALTERED THE DIVIDE OPERATION *
*      CONTINUES, OTHERWISE AN OVERFLOW    *
*      IS FORCED WITH THE PROPER SIGN.     *
*      THE FLOATING DIVIDE BY ZERO CAN BE  *
*      DISTINGUISHED FROM TRUE ZERO DIVIDE *
*      BY THE FACT THAT IT IS AN IMMEDIATE *
*      ZERO DIVIDING INTO REGISTER D0.     *
*                                          *
* TIME: (8 MHZ NO WAIT STATES ASSUMED)     *
* DIVIDEND ZERO         5.250 MICROSECONDS *
* MINIMUM TIME OTHERS  72.750 MICROSECONDS *
* MAXIMUM TIME OTHERS  85.000 MICROSECONDS *
* AVERAGE OTHERS       76.687 MICROSECONDS *
*                                          *
********************************************
******* mathffp.library/SPDiv ******
*
*NAME
*	SPDiv -- Divide two floating point numbers.
*
*SYNOPSIS
*	fnum3 = SPDiv(fnum1, fnum2)
*	D0	      D1     D0
*
*	float SPDiv(float fnum1, float fnum2);
*
*FUNCTION
*	Accepts two floating point numbers and returns the arithmetic
*	division of said numbers.
*
*INPUTS
*	fnum1 	- floating point number.
*	fnum2	- floating point number.
*
*RESULT
*
*	fnum3 	- floating point number.
*
*BUGS
*	None.
*
*SEE ALSO
*
******/
         PAGE
*FFPDIV   IDNT      1,1  FFP DIVIDE

*         OPT       PCS

         XDEF      FFPDIV    ENTRY POINT
		 XDEF	   MFSPDiv

*        XREF      FFPCPYRT  COPYRIGHT STUB


* DIVIDE BY ZERO EXIT
FPDDZR   DIVU.W    #0,D0     FORCE DIVIDE BY ZERO

* IF THE EXCEPTION RETURNS WITH ALTERED DENOMINATOR - CONTINUE DIVIDE
         TST.L     D1        ? EXCEPTION ALTER THE ZERO
         BNE.S     FFPDIV    BRANCH IF SO TO CONTINUE
* SETUP MAXIMUM NUMBER FOR DIVIDE OVERFLOW
FPDOVF   OR.L  #$FFFFFF7F,D0 MAXIMIZE WITH PROPER SIGN
         TST.B     D0        SET CONDITION CODE FOR SIGN
*        ORI       #$02,CCR  SET OVERFLOW BIT
         DC.L      $003C0002 *****SICK ASSEMBLER*****
FPDRTN:
		 MOVEM.L   (SP)+,D3-D5
	     RTS                 RETURN

* OVER OR UNDERFLOW DETECTED
FPDOV2   SWAP.W    D1        RESTORE ARG1
         SWAP.W    D0        RESTORE ARG2 FOR SIGN
FPDOVFS  EOR.B     D1,D0     SETUP CORRECT SIGN
         BRA.S     FPDOVF    AND ENTER OVERFLOW HANDLING
FPDOUF   BMI.S     FPDOVFS   BRANCH IF OVERFLOW
FPDUND   MOVEQ     #0,D0     UNDERFLOW TO ZERO
		 MOVEM.L   (SP)+,D3-D5
         RTS                 RETURN

***************
* ENTRY POINT *
***************

* FIRST SUBTRACT EXPONENTS
FFPDIV:
MFSPDiv:
		 MOVEM.L   D3-D5,-(SP)
	     MOVE.B    D1,D5     COPY ARG1 (DIVISOR)
         BEQ.S     FPDDZR    BRANCH IF DIVIDE BY ZERO
         MOVE.L    D0,D4     COPY ARG2 (DIVIDEND)
         BEQ.S     FPDRTN    RETURN ZERO IF DIVIDEND ZERO
         MOVEQ     #-128,D3  SETUP SIGN MASK
         ADD.W     D5,D5     ISOLATE ARG1 SIGN FROM EXPONENT
         ADD.W     D4,D4     ISOLATE ARG2 SIGN FROM EXPONENT
         EOR.B     D3,D5     ADJUST ARG1 EXPONENT TO BINARY
         EOR.B     D3,D4     ADJUST ARG2 EXPONENT TO BINARY
         SUB.B     D5,D4     SUBTRACT EXPONENTS
         BVS.S     FPDOUF    BRANCH IF OVERFLOW/UNDERFLOW
         CLR.B     D0        CLEAR ARG2 S+EXP
         SWAP.W    D0        PREPARE HIGH 16 BIT COMPARE
         SWAP.W    D1        AGAINST ARG1 AND ARG2
         CMP.W     D1,D0     ? CHECK IF OVERFLOW WILL OCCUR
         BMI.S     FPDNOV    BRANCH IF NOT
* ADJUST FOR FIXED POINT DIVIDE OVERFLOW
         ADDQ.B    #2,D4     ADJUST EXPONENT UP ONE
         BVS.S     FPDOV2    BRANCH OVERFLOW HERE
         ROR.L     #1,D0     SHIFT DOWN BY POWER OF TWO
FPDNOV   SWAP.W    D0        CORRECT ARG2
         MOVE.B    D3,D5     MOVE $80 INTO D5.B
         EOR.W     D5,D4     CREATE SIGN AND ABSOLUTIZE EXPONENT
         LSR.W     #1,D4     D4.B NOW HAS SIGN+EXPONENT OF RESULT

* NOW DIVIDE JUST USING 16 BITS INTO 24
         MOVE.L    D0,D3     COPY ARG1 FOR INITIAL DIVIDE
         DIVU.W    D1,D3     OBTAIN TEST QUOTIENT
         MOVE.W    D3,D5     SAVE TEST QUOTIENT

* NOW MULTIPLY 16-BIT DIVIDE RESULT TIMES FULL 24 BIT DIVISOR AND COMPARE
* WITH THE DIVIDEND.  MULTIPLYING BACK OUT WITH THE FULL 24-BIT ALLOWS
* US TO SEE IF THE RESULT WAS TOO LARGE DUE TO THE 8 MISSING DIVISOR BITS
* USED IN THE HARDWARE DIVIDE.  THE RESULT CAN ONLY BE TOO LARGE BY 1 UNIT.
         MULU.W    D1,D3     HIGH DIVISOR X QUOTIENT
         SUB.L     D3,D0     D0=PARTIAL SUBTRACTION
         SWAP.W    D0        TO LOW DIVISOR
         SWAP.W    D1        REBUILD ARG1 TO NORMAL
         MOVE.W    D1,D3     SETUP ARG1 FOR PRODUCT
         CLR.B     D3        ZERO LOW BYTE
         MULU.W    D5,D3     FIND REMAINING PRODUCT
         SUB.L     D3,D0     NOW HAVE FULL SUBTRACTION
         BCC.S     FPDQOK    BRANCH FIRST 16 BITS CORRECT

************************************************************************************
* ESTIMATE TOO HIGH, DECREMENT QUOTIENT TILL CORRECT							V1.4
FPDCRT:
		 SUB.W	   #1,D5	 DOWN ANOTHER DIVISOR								V1.4
		 ADD.L	   D1,D0	 ADJUST UP BY ANOTHER DIVISOR						V1.4
		 BCC.S	   FPDCRT	 ADJUST MORE IF NOT BACK TO POSITIVE				V1.4
************************************************************************************
*         MOVE.L    D1,D3     REBUILD DIVISOR
*         CLR.B     D3        REVERSE HALVES
*         ADD.L     D3,D0     ADD ANOTHER DIVISOR
*         SUBQ.W    #1,D5     DECREMENT QUOTIENT
************************************************************************************

* COMPUTE LAST 8 BITS WITH ANOTHER DIVIDE.  THE EXACT REMAINDER FROM THE
* MULTIPLY AND COMPARE ABOVE IS DIVIDED AGAIN BY A 16-BIT ONLY DIVISOR.
* HOWEVER, THIS TIME WE REQUIRE ONLY 9 BITS OF ACCURACY IN THE RESULT
* (8 TO MAKE 24 BITS TOTAL AND 1 EXTRA BIT FOR ROUNDING PURPOSES) AND THIS
* DIVIDE ALWAYS RETURNS A PRECISION OF AT LEAST 9 BITS.
FPDQOK   MOVE.L    D1,D3     COPY ARG1 AGAIN
         SWAP.W    D3        FIRST 16 BITS DIVISOR IN D3 WORD
         CLR.W     D0        INTO FIRST 16 BITS OF DIVIDEND
         DIVU.W    D3,D0     OBTAIN FINAL 16 BIT RESULT
         SWAP.W    D5        FIRST 16 QUOTIENT TO HIGH HALF
         BMI.S     FPDISN    BRANCH IF NORMALIZED
* RARE OCCURRANCE - UNNORMALIZED
* HAPPENDS WHEN MANTISSA ARG1 < ARG2 AND THEY DIFFER ONLY IN LAST 8 BITS
         MOVE.W    D0,D5     INSERT LOW WORD OF QUOTIENT
         ADD.L     D5,D5     SHIFT MANTISSA LEFT ONE
         SUBQ.B    #1,D4     ADJUST EXPONENT DOWN (CANNOT ZERO)
         MOVE.W    D5,D0     CANCEL NEXT INSTRUCTION

* REBUILD OUR FINAL RESULT AND RETURN
FPDISN   MOVE.W    D0,D5     APPEND NEXT 16 BITS
         ADD.L     #$80,D5   ROUND TO 24 BITS (CANNOT OVERFLOW)
         MOVE.L    D5,D0     RETURN IN D0
         MOVE.B    D4,D0     FINISH RESULT WITH SIGN+EXPONENT
         BEQ.S     FPDUND    UNDERFLOW IF ZERO EXPONENT
		 MOVEM.L   (SP)+,D3-D5
         RTS                 RETURN


       END
