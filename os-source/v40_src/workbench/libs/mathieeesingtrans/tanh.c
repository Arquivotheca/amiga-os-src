/*
    Math Runtime library - Copyright 1983,1984,1985,1986 by Green Hills Software Inc.
    This program is the property of Green Hills Software, Inc,
    its contents are proprietary information and no part of it
    is to be disclosed to anyone except employees of Green Hills
    Software, Inc., or as agreed in writing signed by the President
    of Green Hills Software, Inc.
*/
/****** mathieeesingtrans.library/IEEESPTanh ********************************
*
*   NAME
*	IEEESPTanh -- compute the hyperbolic tangent of a floating point number 
*
*   SYNOPSIS
*	  x   = IEEESPTanh(  y  );
*	d0		   d0
*
*	float	x,y;
*
*   FUNCTION
*	Compute hyperbolic tangent of y in IEEE single precision
*
*   INPUTS
*	y - IEEE single precision floating point value
*
*   RESULT
*	x - IEEE single precision floating point value
*
*   BUGS
*
*   SEE ALSO
*	IEEESPSinh(), IEEESPCosh()
**************************************************************************/

#include "math.h"
# define MAXE	 9.0	  /* for x>MAXE exp(x)+exp(-x) == exp(x) */
			  /* Assumes no more than 24 bit exponent */
			  /* MAXE = log(sqrt(pow(2.0,24.0))) */

float rtanh(x)
float x;
{
    float f0, xsq, rexp();
    /* for abs(x) > MAXE then the exp(-x) term is negligible and need not
	be evaluated, for x<.5 subtracting 2 numbers causes some loss
	of accuracy so use the series in HART 1983,
	otherwise use (exp(x)-exp(-x))/(exp(x)+exp(-x))
    */

    if (x < 0) {
	if (x < -MAXE)
	    return(-1.0);
	else if (x < -0.5) {
	    x = rexp(x);
	    f0 = 1.0/x;
	    return((x-f0)/(x+f0));
	}
    } else {
	if (x > MAXE)
	    return(1.0);
	else if (x > 0.5) {
	    x = rexp(x);
	    f0 = 1.0/x;
	    return((x-f0)/(x+f0));
	}
    }
    xsq = x*x;
    f0 = 2.0*0.19979150018e-3;
    f0 *= xsq;
    f0 += 2.0*0.833311842443e-2;
    f0 *= xsq;
    f0 += 2.0*0.166666677361144e0;
    f0 *= xsq;
    f0 += 2.0*0.999999999917156e0;
    f0 *= x;
    x = rexp(x);
    f0 /= (x+1.0/x);
    return(f0);
}
