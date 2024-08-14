/**********************************************************************
*
*   $Id: acos.c,v 37.1 91/01/21 15:19:14 mks Exp $
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
/****** mathieeedoubtrans.library/IEEEDPAcos ********************************
*
*   NAME
*	IEEEDPAcos -- compute the arc cosine of a number
*
*   SYNOPSIS
*	  x   = IEEEDPAcos(  y  );
*	d0/d1	           d0/d1
*
*	double	x,y;
*
*   FUNCTION
*	Compute arc cosine of y in IEEE double precision
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
*	IEEEDPCos(), IEEEDPAtan(), IEEEDPAsin()
**************************************************************************/

#include "math.h"
#define PI	3.14159265358979323846
#define PI2	1.57079632679489661923

double acos(x)
double x;
{
    double sqrt(), atan();
    /* I use the identity that acos(x)=atan(sqrt(1/x^2-1)) */

    if (x >= 0) {
	x *= x;
	if (x == 0.0)
	    return(PI2);			/* arc cos(0)=PI/2 */
	if (x >= 1.0) {
	    if (x == 1.0)
		return(0);
	    return(0.0);
	}
	x = atan(sqrt(1.0/x - 1.0));
    } else {
	x *= x ;
	if (x == 0.0)
	    return(PI2);			/* arc cos(0)=PI/2 */
	if (x >= 1.0) {
	    if (x == 1.0)
		return(PI);
	    return(0.0);
	}
	x = PI - atan(sqrt(1.0/x - 1.0));
    }
    return(x);
}
