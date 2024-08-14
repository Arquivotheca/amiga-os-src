*      TTL    FAST FLOATING POINT SQUARE ROOT (FFPSQRT)

*******************************************
* (C)  COPYRIGHT 1981 BY MOTOROLA INC.    *
*******************************************

********************************************
*           FFPSQRT SUBROUTINE             *
*                                          *
* INPUT:                                   *
*          D0 - FLOATING POINT ARGUMENT    *
*                                          *
* OUTPUT:                                  *
*          D0 - FLOATING POINT SQUARE ROOT *
*                                          *
* CONDITION CODES:                         *
*                                          *
*          N - CLEARED                     *
*          Z - SET IF RESULT IS ZERO       *
*          V - SET IF ARGUMENT WAS NEGATIVE*
*          C - CLEARED                     *
*          X - UNDEFINED                   *
*                                          *
* CODE: 194 BYTES    STACK WORK: 4 BYTES   *
*                                          *
* NOTES:                                   *
*   1) NO OVERFLOWS OR UNDERFLOWS CAN      *
*      OCCUR.                              *
*   2) A NEGATIVE ARGUMENT CAUSES THE      *
*      ABSOLUTE VALUE TO BE USED AND THE   *
*      "V" BIT SET TO INDICATE THAT A      *
*      NEGATIVE SQUARE ROOT WAS ATTEMPTED. *
*                                          *
* TIMES:                                   *
* ARGUMENT ZERO         3.50 MICROSECONDS  *
* MINIMUM TIME > 0    187.50 MICROSECONDS  *
* AVERAGE TIME > 0    193.75 MICROSECONDS  *
* MAXIMUM TIME > 0    200.00 MICROSECONDS  *
********************************************
******* mathtrans.library/SPSqrt ************************
*
*NAME	
* 
*	SPSqrt - obtain the square root of the floating point number
*
*SYNOPSIS
* 
*	fnum2 = SPSqrt(fnum1);
*                      d0.l
*	float fnum2;
*	float fnum1;
*
*FUNCTION
* 
*	Accepts a floating point number and returns the square toot
*	of said number
*
*INPUTS
* 
*	fnum1 - Motorola fast floating point number
*
*RESULT
* 
*	fnum2 - Motorola fast floating point number
* 
*BUGS
* 
*	None
*
*SEE ALSO
* 
*	SPPow, SPMul
******

	section mathtrans

         PAGE
*FFPSQRT  IDNT   1,1  FFP SQUARE ROOT

*         OPT    PCS

*        XREF   FFPCPYRT  COPYRIGHT STUB

*8-MAY-84 MJS SQUARE ROOT END PATTERN NOW IN REG. D6.
*8-MAY-84 MJS WAS FETCHED FROM TABLE.

* NEGATIVE ARGUMENT HANDLER
FPSINV   AND.B     #$7F,D0   TAKE ABSOLUTE VALUE
         BSR.S     MFSPSqrt   FIND SQRT(ABS(X))
*        OR.B      $02,CCR   SET "V" BIT
         DC.L      $003C0002 **ASSEMBLER ERROR**
         RTS

*********************
* SQUARE ROOT ENTRY *
*********************
	xdef	MFSPSqrt
MFSPSqrt:
*	called with argument in d0
*	D1 is a scratch register
         MOVE.B    D0,D1     COPY S+EXPONENT OVER
         BEQ.S     FPSRTN    RETURN ZERO IF ZERO ARGUMENT
         BMI.S     FPSINV    NEGATIVE, REJECT WITH OVERFLOW
	movem.l	d4-d6,-(sp)	save used non-scratch registers
         LSR.B     #1,D1     DIVIDE EXPONENT BY TWO
         BCC.S     FPSEVEN   BRANCH EXPONENT WAS EVEN
         ADDQ.B    #1,D1     ADJUST ODD VALUES UP BY ONE
         LSR.L     #1,D0     OFFSET ODD EXPONENT'S MANTISSA ONE BIT
FPSEVEN  ADD.B     #$20,D1   RENORMALIZE EXPONENT
         SWAP.W    D1        SAVE RESULT S+EXP FOR FINAL MOVE
         MOVE.W    #23,D1    SETUP LOOP FOR 24 BIT GENERATION
         LSR.L     #7,D0     PREPARE FIRST TEST VALUE
         MOVE.L    D0,D4     D4 - PREVIOUS VALUE DURING LOOP
         MOVE.L    D0,D5     D5 - NEW TEST VALUE DURING LOOP
         MOVE.L    #1<<21,D6 D6 - PREVIOUS ENDING PATTERN
*8-MAY-84MOVE.L    A0,D6     SAVE ADDRESS REGISTER
*8-MAY-84LEA       FPSTBL(PC),A0 LOAD TABLE ADDRESS
         MOVE.L    #1<<23,D0 D0 - INITIAL RESULT
         SUB.L     D0,D4     PRESET OLD VALUE IN CASE ZERO BIT NEXT
         SUB.L     #$01200000,D5 COMBINE FIRST LOOP CALCULATION
         BRA.S     FPSENT    GO ENTER LOOP CALCULATIONS

*                   SQUARE ROOT CALCULATION
* THIS IS AN OPTIMIZED SCHEME FOR THE RECURSIVE SQUARE ROOT ALGORITHM:
*
*  STEP N+1:
*     TEST VALUE <= .0  0  0  R  R  R  0 1  THEN GENERATE A ONE IN RESULT R
*                     N  2  1  N  2  1        ELSE  ZERO IN RESULT R     N+1
*                                                                   N+1
* PRECALCULATIONS ARE DONE SUCH THAT THE ENTRY IS MIDWAY INTO STEP 2

FPSONE   BSET      D1,D0     INSERT A ONE INTO THIS POSITION
         MOVE.L    D5,D4     UPDATE NEW TEST VALUE
FPSZERO  ADD.L     D4,D4     MULTIPLY TEST RESULT BY TWO
         MOVE.L    D4,D5     COPY IN CASE NEXT BIT ZERO
*8-MAY-84SUB.L     (A0)+,D5  SUBTRACT THE '01' ENDING PATTERN
         LSR.L     #1,D6     SHIFT ENDING PATTERN AROUND
         SUB.L     D6,D5     SUBTRACT THE ENDING PATTERN
         SUB.L     D0,D5     SUBTRACT RESULT BITS COLLECTED SO FAR
FPSENT   DBMI      D1,FPSONE BRANCH IF A ONE GENERATED IN THE RESULT
         DBPL      D1,FPSZERO BRANCH IF A ZERO GENERATED

* ALL 24 BITS CALCULATED. NOW TEST RESULT OF 25TH BIT
         BLS.S     FPSFIN    BRANCH NEXT BIT ZERO, NO ROUNDING
*******************************************************************************************
*        ADDQ.L    #1,D0     ROUND UP (CANNOT OVERFLOW)
*******************************************************************************************
		 CMP.L	   #$00FFFFFF,D0	INSURE NO OVERFLOW								V1.4
		 BEQ.S	   FPSFIN			BRANCH MANTISSA ALL ONES					 	V1.4
         ADDQ.L    #1,D0     ROUND UP (CANNOT OVERFLOW)
*******************************************************************************************
FPSFIN   LSL.L     #8,D0     NORMALIZE RESULT
*8-MAY-84MOVE.L    D6,A0     RESTORE ADDRESS REGISTER
         SWAP.W    D1        RESTORE S+EXP SAVE
         MOVE.B    D1,D0     MOVE IN FINAL SIGN+EXPOENT
	movem.l	(sp)+,d4-d6
FPSRTN   RTS                 RETURN

* TABLE TO FURNISH '01' SHIFTS DURING THE ALGORITHMLOOP
*8-MAY-84FPSTBL   DC.L      1<<20,1<<19,1<<18,1<<17,1<<16,1<<15
*8-MAY-84DC.L      1<<14,1<<13,1<<12,1<<11,1<<10,1<<9,1<<8
*8-MAY-84DC.L      1<<7,1<<6,1<<5,1<<4,1<<3,1<<2,1<<1,1<<0
*8-MAY-84DC.L      0,0

         END
