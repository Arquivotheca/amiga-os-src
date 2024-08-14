/**********************************************************************
*
*   $Id: pow.c,v 37.1 91/01/21 15:20:34 mks Exp $
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
/****** mathieeedoubtrans.library/IEEEDPPow ********************************
*
*   NAME
*	IEEEDPPow -- raise a number to another number power
*
*   SYNOPSIS
*	  z   = IEEEDPPow(  x  ,  y  );
*	d0/d1	          d2/d3 d0/d1
*
*	double	x,y,z;
*
*   FUNCTION
*	Compute y^x in IEEE double precision
*
*   INPUTS
*	x - IEEE double precision floating point value
*	y - IEEE double precision floating point value
*
*   RESULT
*	z - IEEE double precision floating point value
*
*   BUGS
*
*   SEE ALSO
**************************************************************************/

#include "math.h"

#define NEWCODE

#ifdef NEWCODE

extern double hardpow();

static double explog(x,y)
double x,y;
{
	return( exp( y*log( x )));
}
double pow( x ,y )
double x,y;
{
	double result;
	result = hardpow(x,y,explog);
	return (result);
}




#else
double pow( x, y ) double x, y ; {
	/* calculate x^y */
    register double result ;
	int iy;

	if (y == 0.0)	return (1.0);

    if (floor(y) == y && ((-1000 < y && y < 1000) || x <= 0.0)) {
	if (x == 0.0)
	    if (y > 0)
		return(0.0);
	    else {
		return(1.0);
	    }
	result = 1.0;
	/* we know that -1000 < y < 1000 and it has no fraction */
	iy = y;		/* convert to an integer to be quicker */
	if (iy < 0) {
	    iy = -iy;
	    x = 1/x;
	}
	if (iy > 10)
	{	/* lets magnitude reduce */
		int a,b;
		a = iy>>1;
		b = iy-a;
		result = pow(x,(double)a);
		result *= result;
		if (a != b)	result *= x;	/* one more */
	}
	else
	{
		while (iy-- > 0)
    		result *= x ;
	}
	return( result );
    } else if ( x <= 0.0 ) {
	return( 0.0 );
    } else
	return( exp( y*log( x )));
}
#endif
