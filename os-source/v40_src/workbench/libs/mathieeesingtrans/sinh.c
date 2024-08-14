/*
    Math Runtime library - Copyright 1983,1984,1985,1986 by Green Hills Software Inc.
    This program is the property of Green Hills Software, Inc,
    its contents are proprietary information and no part of it
    is to be disclosed to anyone except employees of Green Hills
    Software, Inc., or as agreed in writing signed by the President
    of Green Hills Software, Inc.
*/
/****** mathieeesingtrans.library/IEEESPSinh ********************************
*
*   NAME
*	IEEESPSinh -- compute the hyperbolic sine of a floating point number 
*
*   SYNOPSIS
*	  x   = IEEESPSinh(  y  );
*	d0		   d0
*
*	float	x,y;
*
*   FUNCTION
*	Compute hyperbolic sine of y in IEEE single precision
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
*	IEEESPCosh, IEEESPTanh
**************************************************************************/

#include "math.h"

# define MAXE	 9.0	  /* for x>MAXE exp(x)+exp(-x) == exp(x) */
			  /* Assumes no more than 24 bit exponent */
			  /* MAXE = log(sqrt(pow(2.0,24.0))) */

float rsinh(x)
float x;
{
    float f0, xsq;
    float rexp();
    /* for abs(x) > MAXE then the exp(-x) term is negligible and need not
	be evaluated, for x<.5 subtracting 2 numbers causes some loss
	of accuracy so use the series in HART 1983,
	otherwise use (exp(x)-exp(-x))/2
    */

    if (x < 0) {
	if (x < -MAXE)
	    return(-0.5*rexp(x));
	else if (x < -0.5) {
	    x = rexp(x);
	    return(0.5*(x-1.0/x));
	}
    } else {
	if (x > MAXE)
	    return(0.5*rexp(x));
	else if (x > 0.5) {
	    x = rexp(x);
	    return(0.5*(x-1.0/x));
	}
    }
    xsq = x*x;
    f0 = 0.19979150018e-3;
    f0 *= xsq;
    f0 += 0.833311842443e-2;
    f0 *= xsq;
    f0 += 0.166666677361144e0;
    f0 *= xsq;
    f0 += 0.999999999917156e0;
    f0 *= x;
    return(f0);
}
