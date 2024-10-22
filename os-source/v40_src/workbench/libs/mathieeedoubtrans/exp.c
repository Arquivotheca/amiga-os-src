/**********************************************************************
*
*   $Id: exp.c,v 37.1 91/01/21 15:20:01 mks Exp $
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

#include "math.h"
/****** mathieeedoubtrans.library/IEEEDPExp ********************************
*
*   NAME
*	IEEEDPExp -- compute the exponential of e
*
*   SYNOPSIS
*	  x   = IEEEDPExp(  y  );
*	d0/d1	          d0/d1
*
*	double	x,y;
*
*   FUNCTION
*	Compute e^y in IEEE double precision
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
*	IEEEDPLog()
**************************************************************************/

/* this is e^x, and is implemented by converting it to 2^(x/log2)
	we split this into integer and fraction parts, apply the
	transformation from HART 1069 to get 2 raised to the
	fractional part, and the multiply by 2^integer part
*/

double exp(x)
double x;
{
    double temp;
    double f0, f1, f2;
    int i;

    x *= 1.4426950408889634073599247;
    if (x == 0)
	return(1.0);
    if (x < MINIMUMEXPONENT-1)
	return(0.0);
    if (x >= MAXIMUMEXPONENT) {
	return(HUGE);
    }
    i = x;
    if (x < 0 && x != i)
	i--;
    x -= i;
    x -= 0.5;
    f1 = x*x;
    f0 = 0.6061485330061080841615584556e2;
    f0 *= f1;
    f0 += 0.3028697169744036299076048876e5;
    f0 *= f1;
    f0 += 0.2080384346694663001443843411e7;
    x *= f0;
    f2 = f1;
    f2 += 0.1749287689093076403844945335e4;
    f2 *= f1;
    f2 += 0.3277251518082914423057964442e6;
    f2 *= f1;
    f2 += 0.6002720360238832528230907598e7;
    f1 = f2;
    f2 -= x;
    x += f1;
    x /= f2;
    x *= 1.41421356237309504880167887;
    if (x == 0.0)
	return(x);
    else {
#ifdef KNOWN_FLOATING_POINT_FORMAT
	temp = x;
	i += EXTRACTEXPONENT(temp);
	if (i < MINIMUMEXPONENT)
	    return(0.0);
	else if (i > MAXIMUMEXPONENT)
	    return(HUGE);
	else {
	    SETEXPONENT(temp, i);
	    return(temp);
	}
#else
	return(ldexp(x, i));
#endif
    }
}
