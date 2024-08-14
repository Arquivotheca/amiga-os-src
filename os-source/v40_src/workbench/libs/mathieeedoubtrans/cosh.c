/**********************************************************************
*
*   $Id: cosh.c,v 37.1 91/01/21 15:19:54 mks Exp $
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
/****** mathieeedoubtrans.library/IEEEDPCosh ********************************
*
*   NAME
*	IEEEDPCosh -- compute the hyperbolic cosine of a floating point number
*
*   SYNOPSIS
*	  x   = IEEEDPCosh(  y  );
*	d0/d1		   d0/d1
*
*	double	x,y;
*
*   FUNCTION
*	Compute hyperbolic cosine of y in IEEE double precision
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
*	IEEEDPSinh(), IEEEDPTanh()
**************************************************************************/

#include "math.h"
#define MAX	 ((double)0x10000000)	  /* for x>MAX x+1/x == x */
			  	/* Assumes no more than 56 bit mantissa */
			  	/* MAX = sqrt(pow(2.0,56.0))) */
double cosh(x)
double x;
{
    if (x < 0.0)
	x = -x;
    x = exp(x);
    if (x <= MAX) x = (x+1.0/x);
    HALVE(x);
    return(x);
}
