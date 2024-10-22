*        TTL       FAST FLOATING POINT ARCTANGENT (FFPATAN)
***************************************
* (C) COPYRIGHT 1981 BY MOTOROLA INC. *
***************************************

*************************************************
*                  FFPATAN                      *
*       FAST FLOATING POINT ARCTANGENT          *
*                                               *
*  INPUT:   D7 - INPUT ARGUMENT                 *
*                                               *
*  OUTPUT:  D7 - ARCTANGENT RADIAN RESULT       *
*                                               *
*     ALL OTHER REGISTERS TOTALLY TRANSPARENT   *
*                                               *
*  CODE SIZE: 132 BYTES   STACK WORK: 32 BYTES  *
*                                               *
*  CONDITION CODES:                             *
*        Z - SET IF THE RESULT IS ZERO          *
*        N - CLEARED                            *
*        V - CLEARED                            *
*        C - UNDEFINED                          *
*        X - UNDEFINED                          *
*                                               *
*                                               *
*  NOTES:                                       *
*    1) SPOT CHECKS SHOW AT LEAST SIX DIGIT     *
*       PRECISION ON ALL SAMPLED CASES.         *
*                                               *
*  TIME: (8MHZ NO WAIT STATES ASSUMED)          *
*                                               *
*        THE TIME IS VERY DATA SENSITIVE WITH   *
*        SAMPLE VALUES RANGING FROM 238 TO      *
*        465 MICROSECONDS                       *
*                                               *
*************************************************
******* mathtrans.library/SPAtan ************************
*
*NAME	
* 
*	SPAtan - obtain the arctangent of the floating point number
*
*SYNOPSIS
* 
*	fnum2 = SPAtan(fnum1);
*                       d0.l
*	float fnum2;
*	float fnum1;
*
*FUNCTION
* 
*	Accepts a floating point number representing the tangent 
*	of an angle and returns the value of said angle in
*	radians
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
*	SPTan
******
         PAGE
*FFPATAN  IDNT      1,2                 FFP ARCTANGENT

*         OPT       PCS

         XREF      FFPTHETA            ARCTANGENT TABLE

         XREF      FFPDIV,FFPSUB       ARITHMETIC PRIMITIVES
         XREF      FFPTNORM            TRANSCENDENTAL NORMALIZE ROUTINE
*        XREF      FFPCPYRT            COPYRIGHT STUB

         XDEF      FFPATAN             ENTRY POINT

PIOV2    EQU       $C90FDB41           FLOAT PI/2
FPONE    EQU       $80000041           FLOAT 1

********************
* ARCTANGENT ENTRY *
********************

* SAVE REGISTERS AND PERFORM ARGUMENT REDUCTION
FFPATAN  MOVEM.L   D1-D6/A0,-(SP)      SAVE REGISTERS
         MOVE.B    D7,-(SP)            SAVE ORIGINAL SIGN ON STACK
         AND.B     #$7F,D7             TAKE ABSOLUTE VALUE
* INSURE LESS THAN ONE FOR CORDIC LOOP
         MOVE.L    #FPONE,D6           LOAD ONE
         CLR.B     -(SP)               DEFAULT NO INVERSE REQUIRED
         CMP.B     D6,D7               ? LESS THAN ONE
         BCS.S     FPAINRG             BRANCH IN RANGE
         BHI.S     FPARDC              HIGHER - MUST REDUCE
         CMP.L     D6,D7               ? LESS OR EQUAL TO ONE
         BLS.S     FPAINRG             BRANCH YES, IS IN RANGE
* ARGUMENT > 1:  ATAN(1/X) =  PI/2 - ATAN(X)
FPARDC   NOT.B     (SP)                FLAG INVERSE TAKEN
         EXG.L     D6,D7               TAKE INVERSE OF ARGUMENT
         JSR       FFPDIV              PERFORM DIVIDE

* PERFORM CORDIC FUNCTION
* CONVERT TO BIN(31,29) PRECISION
FPAINRG  SUB.B     #64+3,D7            ADJUST EXPONENT
         NEG.B     D7                  FOR SHIFT NECESSARY
         CMP.B     #31,D7              ? TOO SMALL TO WORRY ABOUT
         BLS.S     FPANOTZ             BRANCH IF NOT TOO SMALL
         MOVEQ     #0,D6               CONVERT TO ZERO
         BRA.S     FPAZRO              BRANCH IF ZERO
FPANOTZ  LSR.L     D7,D7               SHIFT TO BIN(31,29) PRECISION

*****************************************
* CORDIC CALCULATION REGISTERS:         *
* D1 - LOOP COUNT   A0 - TABLE POINTER  *
* D2 - SHIFT COUNT                      *
* D3 - Y'   D5 - Y                      *
* D4 - X'   D6 - Z                      *
* D7 - X                                *
*****************************************
 
         MOVEQ     #0,D6               Z=0
         MOVE.L    #1<<29,D5           Y=1
         LEA       FFPTHETA+4,A0       FIND ARCTANGENT TABLE
         MOVEQ     #24,D1              LOOP 25 TIMES
         MOVEQ     #1,D2               PRIME SHIFT COUNTER
         BRA.S     CORDIC              ENTER CORDIC LOOP

* CORDIC LOOP
FPLPLS   ASR.L     D2,D4               SHIFT(X')
         ADD.L     D4,D5               Y = Y + X'
         ADD.L     (A0),D6             Z = Z + ARCTAN(I)
CORDIC   MOVE.L    D7,D4               X' = X
         MOVE.L    D5,D3               Y' = Y
         ASR.L     D2,D3               SHIFT(Y')
FPLNLP   SUB.L     D3,D7               X = X - Y'
         BPL.S     FPLPLS              BRANCH NEGATIVE
         MOVE.L    D4,D7               RESTORE X
         ADDQ.L    #4,A0               TO NEXT TABLE ENTRY
         ADDQ.B    #1,D2               INCREMENT SHIFT COUNT
         LSR.L     #1,D3               SHIFT(Y')
         DBF       D1,FPLNLP           AND LOOP UNTIL DONE

* NOW CONVERT TO FLOAT AND RECONSTRUCT THE RESULT
         JSR       FFPTNORM            FLOAT Z
FPAZRO   MOVE.L    D6,D7               COPY ANSWER TO D7
         TST.B     (SP)+               ? WAS INVERSE TAKEN
         BEQ.S     FPANINV             BRANCH IF NOT
         MOVE.L    #PIOV2,D7           LOAD PI/2
         JSR       FFPSUB              SUBTRACT FROM PI/2
FPANINV  MOVE.B    (SP)+,D6            LOAD ORIGINAL SIGN
         TST.B     D7                  ? RESULT ZERO
         BEQ.S     FPARTN              RETURN IF SO
         AND.B     #$80,D6             CLEAR EXPONENT PORTION
         OR.B      D6,D7               IF MINUS, GIVE MINUS RESYLT
FPARTN   MOVEM.L   (SP)+,D1-D6/A0      RESTORE REGISTERS
         RTS                           RETURN

         END
