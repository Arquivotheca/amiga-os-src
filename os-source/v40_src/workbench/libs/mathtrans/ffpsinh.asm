
*        TTL       FAST FLOATING POINT HYPERBOLICS (FFPSINH)
***************************************
* (C) COPYRIGHT 1981 BY MOTOROLA INC. *
***************************************

*************************************************
*            FFPSINH/FFPCOSH/FFPTANH            *
*       FAST FLOATING POINT HYPERBOLICS         *
*                                               *
*  INPUT:   D7 - FLOATING POINT ARGUMENT        *
*                                               *
*  OUTPUT:  D7 - HYPERBOLIC RESULT              *
*                                               *
*     ALL OTHER REGISTERS ARE TRANSPARENT       *
*                                               *
*  CODE SIZE:  36 BYTES   STACK WORK: 50 BYTES  *
*                                               *
*  CALLS: FFPEXP, FFPDIV, FFPADD AND FFPSUB     *
*                                               *
*  CONDITION CODES:                             *
*        Z - SET IF THE RESULT IS ZERO          *
*        N - SET IF THE RESULT IS NEGATIVE      *
*        V - SET IF OVERFLOW OCCURRED           *
*        C - UNDEFINED                          *
*        X - UNDEFINED                          *
*                                               *
*  NOTES:                                       *
*    1) AN OVERFLOW WILL PRODUCE THE MAXIMUM    *
*       SIGNED VALUE WITH THE "V" BIT SET.      *
*    2) SPOT CHECKS SHOW AT LEAST SEVEN DIGIT   *
*       PRECISION.                              *
*                                               *
*  TIME: (8MHZ NO WAIT STATES ASSUMED)          *
*                                               *
*        SINH  623 MICROSECONDS                 *
*        COSH  601 MICROSECONDS                 *
*        TANH  623 MICROSECONDS                 *
*                                               *
*************************************************
         PAGE
*FFPSINH  IDNT      1,2                 FFP SINH COSH TANH

*         OPT       PCS

         XREF      FFPEXP,FFPDIV,FFPADD,FFPSUB FUNCTIONS CALLED
*        XREF      FFPCPYRT            COPYRIGHT STUB

         XDEF      FFPSINH,FFPCOSH,FFPTANH       ENTRY POINTS

FPONE    EQU       $80000041           FLOATING ONE

**********************************
*            FFPCOSH             *
*  THIS FUNCTION IS DEFINED AS   *
*            X    -X             *
*           E  + E               *
*           --------             *
*              2                 *
*                                *
**********************************
******* mathtrans.library/SPCosh ************************
*
*NAME	
* 
*	SPCosh - obtain the hyperbolic cosine of the floating point number
*
*SYNOPSIS
* 
*	fnum2 = SPCosh(fnum1);
*                       d0.l
*	float fnum2;
*	float fnum1;
*
*FUNCTION
* 
*	Accepts a floating point number representing an angle
*	in radians and returns the hyperbolic cosine of said angle.
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
*	SPSinh
******

FFPCOSH  MOVE.L    D6,-(SP)  SAVE WORK REGISTER
         AND.B     #$7F,D7   FORCE POSITIVE (RESULTS SAME BUT EXP FASTER)
         JSR       FFPEXP    EVALUATE E TO THE X
         BVS.S     FHCRTN    RETURN IF OVERFLOW (RESULT IS HIGHEST NUMBER)
         MOVE.L    D7,-(SP)  SAVE RESULT
         MOVE.L    D7,D6     SETUP FOR DIVIDE INTO ONE
         MOVE.L    #FPONE,D7 LOAD ONE
         JSR       FFPDIV    COMPUTE E TO -X AS THE INVERSE
         MOVE.L    (SP)+,D6  PREPARE TO ADD TOGETHER
         JSR       FFPADD    CREATE THE NUMERATOR
         BEQ.S     FHCRTN    RETURN IF ZERO RESULT
         SUBQ.B    #1,D7     DIVIDE BY TWO
         BVC.S     FHCRTN    RETURN IF NO UNDERFLOW
         MOVEQ     #0,D7     RETURN ZERO IF UNDERFLOW
FHCRTN   MOVEM.L   (SP)+,D6  RESTORE WORK REGISTER
         RTS                 RETURN
         PAGE
**********************************
*            FFPSINH             *
*  THIS FUNCTION IS DEFINED AS   *
*            X    -X             *
*           E  - E               *
*           --------             *
*              2                 *
* HOWEVER, WE EVALUATE IT VIA    *
* THE COSH FORMULA SINCE ITS     *
* ADDITION IN THE NUMERATOR      *
* IS SAFER THAN OUR SUBTRACTION  *
*                                *
* THUS THE FUNCTION BECOMES:     *
*            X                   *
*    SINH = E  - COSH            *
*                                *
**********************************
******* mathtrans.library/SPSinh ************************
*
*NAME	
* 
*	SPSinh - obtain the hyperbolic sine of the floating point number
*
*SYNOPSIS
* 
*	fnum2 = SPSinh(fnum1);
*                       d0.l
*	float fnum2;
*	float fnum1;
*
*FUNCTION
* 
*	Accepts a floating point number representing an angle
*	in radians and returns the hyperbolic sine of said angle.
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
*	SPCosh
******
 
FFPSINH  MOVE.L    D6,-(SP)  SAVE WORK REGISTER
         JSR       FFPEXP    EVALUATE E TO THE X
         BVS.S     FHSRTN    RETURN IF OVERFLOW FOR MAXIMUM VALUE
         MOVE.L    D7,-(SP)  SAVE RESULT
         MOVE.L    D7,D6     SETUP FOR DIVIDE INTO ONE
         MOVE.L    #FPONE,D7 LOAD ONE
         JSR       FFPDIV    COMPUTE E TO -X AS THE INVERSE
         MOVE.L    (SP),D6   PREPARE TO ADD TOGETHER
         JSR       FFPADD    CREATE THE NUMERATOR
         BEQ.S     FHSZRO    BRANCH IF ZERO RESULT
         SUBQ.B    #1,D7     DIVIDE BY TWO
         BVC.S     FHSZRO    BRANCH IF NO UNDERFLOW
         MOVEQ     #0,D7     ZERO IF UNDERFLOW
FHSZRO   MOVE.L    D7,D6     MOVE FOR FINAL SUBTRACT
         MOVE.L    (SP)+,D7  RELOAD E TO X AGAIN AND FREE
         JSR       FFPSUB    RESULT IS E TO X MINUS COSH
FHSRTN   MOVEM.L   (SP)+,D6  RESTORE WORK REGISTER
         RTS                 RETURN
         PAGE
**********************************
*            FFPTANH             *
*  THIS FUNCTION IS DEFINED AS   *
*  SINH/COSH WHICH REDUCES TO:   *
*            2X                  *
*           E  - 1               *
*           ------               *
*            2X                  *
*           E  + 1               *
*                                *
**********************************
******* mathtrans.library/SPTanh ************************
*
*NAME	
* 
*	SPTanh - obtain the hyperbolic tangent of the floating point number
*
*SYNOPSIS
* 
*	fnum2 = SPTanh(fnum1);
*                       d0.l
*	float fnum2;
*	float fnum1;
*
*FUNCTION
* 
*	Accepts a floating point number representing an angle
*	in radians and returns the hyperbolic tangent of said angle.
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
*	SPSinh, SPCosh
******

FFPTANH  MOVE.L    D6,-(SP)  SAVE WORK REGISTER
         TST.B     D7        ? ZERO
         BEQ.S     FFPTRTN   RETURN TRUE ZERO IF SO
         ADDQ.B    #1,D7     X TIMES TWO
         BVS.S     FFPTOVF   BRANCH IF OVERFLOW/UNDERFLOW
         JSR       FFPEXP    EVALUATE E TO THE 2X
         BVS.S     FFPTOVF2  BRANCH IF TOO LARGE
         MOVE.L    D7,-(SP)  SAVE RESULT
         MOVE.L    #FPONE,D6 LOAD ONE
         JSR       FFPADD    ADD 1 TO E**2X
         MOVE.L    D7,-(SP)  SAVE DENOMINATOR
         MOVE.L    4(SP),D7  NOW PREPARE TO SUBTRACT
         JSR       FFPSUB    CREATE NUMERATOR
         MOVE.L    (SP)+,D6  RESTORE DENOMINATOR
         JSR       FFPDIV    CREATE RESULT
         ADDQ.L    #4,SP     FREE E**2X OFF OF STACK
FFPTRTN  MOVE.L    (SP)+,D6  RESTORE REGISTER
         RTS                 RETURN

FFPTOVF  MOVE.L    #$80000082,D7 FLOAT ONE WITH EXPONENT OVER TO LEFT
         ROXR.B    #1,D7     SHIFT IN CORRECT SIGN
         BRA.S     FFPTRTN   AND RETURN

FFPTOVF2 MOVE.L    #FPONE,D7 RETURN +1 AS RESULT
         BRA.S     FFPTRTN

         END
