/**********************************************************************
*
*   $Id: sinh.c,v 37.1 91/01/21 15:20:14 mks Exp $
*
**********************************************************************/
/*
    Math Runtime library - Copyright 1983,1984,1985,1986 by Green Hills Software Inc.
    This program is the property of Green Hills Software, Inc,
    its contents are proprietary information and no part of it
    is to be disclosed to anyone except employees of Green Hills
    Software, Inc., or as agreed in writing signed by the President
    of Green Hills Software, Inc.
*/
/****** mathieeedoubtrans.library/IEEEDPSinh ********************************
*
*   NAME
*	IEEEDPSinh -- compute the hyperbolic sine of a floating point number
*
*   SYNOPSIS
*	  x   = IEEEDPSinh(  y  );
*	d0/d1		   d0/d1
*
*	double	x,y;
*
*   FUNCTION
*	Compute hyperbolic sine of y in IEEE double precision
*
*   INPUTS
*	y - IEEE double precision floating point value
*
*   RESULT
*	x - IEEE double precision floating point value
*
*   BUGS
*
*   SEE ALSO
*	IEEEDPCosh, IEEEDPTanh
**************************************************************************/

#include "math.h"
# define MAXE	 20.0	  /* for x>MAXE exp(x)+exp(-x) == exp(x) */
			  /* Assumes no more than 56 bit exponent */
			  /* MAXE = log(sqrt(pow(2.0,56.0))) */

double sinh(x)
double x;
{
    double f0, xsq ;
    /* for abs(x) > MAXE then the exp(-x) term is negligable and need not
	be evaluated, for x<.5 subtracting 2 numbers causes some loss
	of accuracy so use the series in HART 1986,
	otherwise use (exp(x)-exp(-x))/2
    */

    if (x < 0) {
	if (x < -MAXE) {
	    return(-0.5*exp(-x));	/* was exp(x), Dale */
	} else if (x < -0.5) {
	    x = exp(x);
	    return(0.5*(x-1.0/x));
	}
    } else {
	if (x > MAXE)
	    return(0.5*exp(x));
	else if (x > 0.5) {
	    x = exp(x);
	    return(0.5*(x-1.0/x));
	}
    }
    xsq = x*x;
    f0 = 0.1612599051957e-9;
    f0 *= xsq;
    f0 += 0.2505187850238657e-7;
    f0 *= xsq;
    f0 += 0.2755731961514919477e-5;
    f0 *= xsq;
    f0 += 0.198412698409283722458e-3;
    f0 *= xsq;
    f0 += 0.83333333333334751806098e-2;
    f0 *= xsq;
    f0 += 0.1666666666666666644591109e0;
    f0 *= xsq;
    f0 += 0.10000000000000000000056013e1;
    f0 *= x;
    return(f0);
}
