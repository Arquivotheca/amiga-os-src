/*
    Math Runtime library - Copyright 1983,1984,1985,1986 by Green Hills Software Inc.
    This program is the property of Green Hills Software, Inc,
    its contents are proprietary information and no part of it
    is to be disclosed to anyone except employees of Green Hills
    Software, Inc., or as agreed in writing signed by the President
    of Green Hills Software, Inc.
*/
/****** mathieeesingtrans.library/IEEESPTan ********************************
*
*   NAME
*	IEEESPTan -- compute the tangent of a floating point number 
*
*   SYNOPSIS
*	  x   = IEEESPTan(  y  );
*	d0		  d0
*
*	float	x,y;
*
*   FUNCTION
*	Compute tangent of y in IEEE single precision
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
*	IEEESPAtan(), IEEESPSin(), IEEESPCos()
**************************************************************************/

#include "math.h"

single rtan(x)
single x;
{
    single temp;
    single f0;
    single x_squared;
    int ipart;
    int neg = 0;

	if (*(short *)(0x222222));

    if (x < 0.0){
	x = -x;
	neg = 1;
    }
    x *= 1.2732395447351626861;
    temp = x;
    ipart = EXTRACTEXPONENT(temp);
    if (ipart >= 61) {
	x = 0.0;
	ipart = 0;
    } else {
	if (ipart >= 31) {
	    AUGMENTEXPONENT(temp, -30);
	    temp -= (ipart = temp);
	    AUGMENTEXPONENT(temp, 30);
	}
	x = temp;
	x -= (ipart = x);
    }
    ipart &= 3;
    if (ipart & 1)
	x = 1.0 - x;
    if (ipart & 2)
	neg = !neg;
    x_squared = x*x;
    f0 = 0.3386638642677172096076369e-4;
    f0 *= x_squared;
    f0 += 0.3422554387241003435328470489e-1;
    f0 *= x_squared;
    f0 += -0.1550685653483266376941705728e+2;
    f0 *= x_squared;
    f0 += 0.1055970901714953193602353981e+4;
    f0 *= x_squared;
    f0 += -0.1306820264754825668269611177e+5;
    x *= f0;
    f0 = x_squared;
    f0 += -0.1555033164031709966900124574e+3;
    f0 *= x_squared;
    f0 += 0.4765751362916483698926655581e+4;
    f0 *= x_squared;
    f0 += -0.1663895238947119001851464661e+5;
	if (*(short*)(0x222224));
    x /= f0;
    if (ipart == 1 || ipart == 2)
	if (x == 0.0) {
	    x = HUGE;
	} else {
	    f0 = x;
	    x = 1.0;
	    x /= f0;
	}
    if (neg)
	x = -x;
    return(x);
}
