/*
    Math Runtime library - Copyright 1983,1984,1985,1986 by Green Hills Software Inc.
    This program is the property of Green Hills Software, Inc,
    its contents are proprietary information and no part of it
    is to be disclosed to anyone except employees of Green Hills
    Software, Inc., or as agreed in writing signed by the President
    of Green Hills Software, Inc.
*/
/****** mathieeesingtrans.library/IEEESPCosh ********************************
*
*   NAME
*	IEEESPCosh -- compute the hyperbolic cosine of a floating point number 
*
*   SYNOPSIS
*	  x   = IEEESPCosh(  y  );
*	d0		   d0
*
*	float	x,y;
*
*   FUNCTION
*	Compute hyperbolic cosine of y in IEEE single precision
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
*	IEEESPSinh(), IEEESPTanh()
**************************************************************************/

#include "math.h"
#define MAX	 ((single)4096.0)	  /* for x>MAX x+1/x == x */
			  	/* Assumes no more than 56 bit mantissa */
			  	/* MAX = sqrt(pow(2.0,24.0))) */
single rcosh(x)
single x;
{
    single rexp();
    if (x < 0.0)
	x = -x;
    x = rexp(x);
    if (x <= MAX) x = (x+1.0/x);
    HALVE(x);
    return(x);
}
