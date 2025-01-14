/**********************************************************************
*
*   $Id: asin.c,v 37.1 91/01/21 15:19:18 mks Exp $
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
/****** mathieeedoubtrans.library/IEEEDPAsin ********************************
*
*   NAME
*	IEEEDPAsin -- compute the arcsine of a number
*
*   SYNOPSIS
*	  x   = IEEEDPAsin(  y  );
*	d0/d1	           d0/d1
*
*	double	x,y;
*
*   FUNCTION
*	Compute the arc sine of y in IEEE double precision
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
*	IEEEDPSin(), IEEEDPAtan(), IEEEDPAcos()
**************************************************************************/

#include "math.h"
#define PI2	1.57079632679489661923

double asin(x)
double x ;
{
    double f0;
    /* I use the identity that asin(x)=atan(1/sqrt(1/x^2-1)) */

    /* assumes e-30 is not an underflow (at least 8 bit binary exponent) */
    if (x >= 0.0) {
	if (x < 1.0e-15)
	    return(x);		/* prevent underflow: arc sin(x)=x */
	x *= x;
	if (x >= 1.0) {
	    if (x == 1.0)
		return(PI2);
	    return(0.0);
	}
	x = sqrt(x / (1.0 - x));
    } else {
	if (x > -1.0e-15)
	    return(x);		/* prevent underflow: arc sin(x)=x */
	x *= x;
	if (x >= 1.0) {
	    if (x == 1.0)
		return(-PI2);
	    return(0.0);
	}
	x = -sqrt(x / (1.0 - x));
    }
    return(atan(x));
}
