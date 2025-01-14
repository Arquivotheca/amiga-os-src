/*
    Math Runtime library - Copyright 1983,1984,1985,1986 by Green Hills Software Inc.
    This program is the property of Green Hills Software, Inc,
    its contents are proprietary information and no part of it
    is to be disclosed to anyone except employees of Green Hills
    Software, Inc., or as agreed in writing signed by the President
    of Green Hills Software, Inc.
*/
/****** mathieeesingtrans.library/IEEESPCos ********************************
*
*   NAME
*	IEEESPCos -- compute the cosine of a floating point number 
*
*   SYNOPSIS
*	  x   = IEEESPCos(  y  );
*	d0		   d0
*
*	float	x,y;
*
*   FUNCTION
*	Compute cosine of y in IEEE single precision
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
*	IEEESPAcos(), IEEESPSin(), IEEESPTan()
**************************************************************************/

#include "math.h"

float rcos(f0)
float f0;
{
    float f1, f2;
    int r0;

    f0 *= 6.3661977236758134e-01;
    if (f0 < 0)
	f0 = -f0;
    if (f0 > 1073741824)
	f0 = 0;
    f2 = f0;
    f2 += 0.5;
    r0 = f2;
    f1 = r0;
    f1 -= f0;
    f0 = f1;
    f0 *= f0;
    if (r0 & 1) {
	f2 = 1.6044118478735982e-04;
	f2 *= f0;
	f2 += -4.6817541353186881e-03;
	f2 *= f0;
	f2 += 7.9692626246167046e-02;
	f2 *= f0;
	f2 += -6.4596409750624625e-01;
	f0 *= f2;
	f0 += 1.5707963267948966e+00;
	f0 *= f1;
    } else {
	f2 = 9.1926027483942659e-04;
	f2 *= f0;
	f2 += -2.0863480763352961e-02;
	f2 *= f0;
	f2 += 2.5366950790104802e-01;
	f2 *= f0;
	f2 += -1.2337005501361698e+00;
	f0 *= f2;
	f0 += 1.0;
    }
    if (r0 & 2)
	f0 = -f0;
    return(f0);
}
