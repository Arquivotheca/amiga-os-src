/**********************************************************************
*
*   $Id: tanh.c,v 37.1 91/01/21 15:20:21 mks Exp $
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
/****** mathieeedoubtrans.library/IEEEDPTanh ********************************
*
*   NAME
*	IEEEDPTanh -- compute the hyperbolic tangent of a floating point number
*
*   SYNOPSIS
*	  x   = IEEEDPTanh(  y  );
*	d0/d1		   d0/d1
*
*	double	x,y;
*
*   FUNCTION
*	Compute hyperbolic tangent of y in IEEE double precision
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
*	IEEEDPSinh(), IEEEDPCosh()
**************************************************************************/

#include "math.h"
# define MAXE	 22.	  /* for x>MAXE exp(x)+exp(-x) == exp(x) */
			  /* Assumes 64 bit double */

double tanh(x)
double x;
{
    /* The only trick here is that x>MAXE sinh==cosh */
    if (x < -MAXE)
	return(-1.0);
    else if (x > MAXE)
	return(1.0);
    else
	return(sinh(x)/cosh(x));
}
