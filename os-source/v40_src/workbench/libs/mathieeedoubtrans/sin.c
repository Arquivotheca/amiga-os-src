/**********************************************************************
*
*   $Id: sin.c,v 37.1 91/01/21 15:20:10 mks Exp $
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
/****** mathieeedoubtrans.library/IEEEDPSin ********************************
*
*   NAME
*	IEEEDPSin -- compute the sine of a floating point number
*
*   SYNOPSIS
*	  x   = IEEEDPSin(  y  );
*	d0/d1		  d0/d1
*
*	double	x,y;
*
*   FUNCTION
*	Compute sine of y in IEEE double precision
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
*	IEEEDPAsin(), IEEEDPTan(), IEEEDPCos()
**************************************************************************/

#include "math.h"

double sin(f0)
double f0;
{
    double f2, f4;
    int r0;

    f0 *= 6.3661977236758134e-01;
    if (f0 < 0) {
	if (f0 < -1073741824.0) {
	    if (f0 < -1.0e+18)
		f0 = 0;
	    f0 /= 1073741824.0;
	    r0 = f0;
	    f2 = r0;
	    f0 -= f2;
	    f0 *= 1073741824.0;
	}
	f2 = f0;
	f2 -= 0.5;
    } else {
	if (f0 >= 1073741824.0) {
	    if (f0 > 1.0e+18)
		f0 = 0;
	    f0 /= 1073741824.0;
	    r0 = f0;
	    f2 = r0;
	    f0 -= f2;
	    f0 *= 1073741824.0;
	}
	f2 = f0;
	f2 += 0.5;
    }
    r0 = f2;
    f2 = r0;
    f0 -= f2;
    if (r0 & 1) {
	f0 *= f0;
	f2 = 6.5659631149794723e-11;
	f2 *= f0;
	f2 += -6.3866030837918522e-09;
	f2 *= f0;
	f2 += 4.7108747788181715e-07;
	f2 *= f0;
	f2 += -2.5202042373060605e-05;
	f2 *= f0;
	f2 += 9.1926027483942659e-04;
	f2 *= f0;
	f2 += -2.0863480763352961e-02;
	f2 *= f0;
	f2 += 2.5366950790104802e-01;
	f2 *= f0;
	f2 += -1.2337005501361698e+00;
	f0 *= f2;
	f0 += 1.0;
    } else {
	f4 = f0;
	f0 *= f0;
	f2 = -6.6880351098114673e-10;
	f2 *= f0;
	f2 += 5.6921729219679268e-08;
	f2 *= f0;
	f2 += -3.5988432352120853e-06;
	f2 *= f0;
	f2 += 1.6044118478735982e-04;
	f2 *= f0;
	f2 += -4.6817541353186881e-03;
	f2 *= f0;
	f2 += 7.9692626246167046e-02;
	f2 *= f0;
	f2 += -6.4596409750624625e-01;
	f0 *= f2;
	f0 += 1.5707963267948966e+00;
	f0 *= f4;
    }
    if (r0 & 2)
	f0 = -f0;
    return(f0);
}
