*      TTL     FAST FLOATING POINT ABS/NEG (FFPABS/FFPNEG)
***************************************
* (C) COPYRIGHT 1981 BY MOTOROLA INC. *
***************************************

************************************************************
*                     FFPABS                               *
*           FAST FLOATING POINT ABSOLUTE VALUE             *
*                                                          *
*  INPUT:  D0 - FAST FLOATING POINT ARGUMENT               *
*                                                          *
*  OUTPUT: D0 - FAST FLOATING POINT ABSOLUTE VALUE RESULT  *
*                                                          *
*      CONDITION CODES:                                    *
*              N - CLEARED                                 *
*              Z - SET IF RESULT IS ZERO                   *
*              V - CLEARED                                 *
*              C - UNDEFINED                               *
*              X - UNDEFINED                               *
*                                                          *
*               ALL REGISTERS TRANSPARENT                  *
*                                                          *
************************************************************
******* mathffp.library/SPAbs ******
*
*NAME
*	SPAbs -- Obtain the absolute value of the fast floating point number.
*
*SYNOPSIS
*	fnum2 = SPAbs(fnum1)
*	D0	      D0
*
*	float SPAbs(float fnum1);
*
*FUNCTION
*	Accepts a floating point number and returns the absolute value of
*	said number.
*
*INPUTS
*	fnum1 	- floating point number.
*
*RESULT
*	fnum2 	- floating point absolute value of fnum1.
*
*BUGS
*	None
*
*SEE ALSO
*
*
******/
         PAGE
*FFPABS IDNT    1,1  FFP ABS/NEG

         XDEF      FFPABS    FAST FLOATING POINT ABSLUTE VALUE
		 XDEF	   MFSPAbs


******************************
* ABSOLUTE VALUE ENTRY POINT *
******************************
FFPABS:
MFSPAbs:
         ANDI.B   #$7F,D0   CLEAR THE SIGN BIT
         RTS                 RETURN
         PAGE
************************************************************
*                     FFPNEG                               *
*           FAST FLOATING POINT NEGATE                     *
*                                                          *
*  INPUT:  D0 - FAST FLOATING POINT ARGUMENT               *
*                                                          *
*  OUTPUT: D0 - FAST FLOATING POINT NEGATED RESULT         *
*                                                          *
*      CONDITION CODES:                                    *
*              N - SET IF RESULT IS NEGATIVE               *
*              Z - SET IF RESULT IS ZERO                   *
*              V - CLEARED                                 *
*              C - UNDEFINED                               *
*              X - UNDEFINED                               *
*                                                          *
*               ALL REGISTERS TRANSPARENT                  *
*                                                          *
************************************************************
******* mathffp.library/SPNeg ******
*
*NAME
*	SPNeg -- Negate the supplied floating point number.
*
*SYNOPSIS
*	fnum2 = SPNeg(fnum1)
*	D0	      D0
*
*	float SPNeg(float fnum1);
*
*FUNCTION
*	Accepts a floating point number and returns the value
*	of said number after having been subtracted from 0.0.
*
*INPUTS
*	fnum1 	- floating point number.
*
*RESULT
*	fnum2 	- floating point negation of fnum1.
*
*BUGS
*	None
*
*SEE ALSO
*
*
******/

         PAGE
         XDEF      FFPNEG    FAST FLOATING POINT NEGATE
		 XDEF	   MFSPNeg

**********************
* NEGATE ENTRY POINT *
**********************
FFPNEG:
MFSPNeg:
	     TST.B     D0        ? IS ARGUMENT A ZERO
         BEQ.S     FFPRTN    RETURN IF SO
         EORI.B    #$80,D0   INVERT THE SIGN BIT
FFPRTN   RTS                 RETURN

         END
