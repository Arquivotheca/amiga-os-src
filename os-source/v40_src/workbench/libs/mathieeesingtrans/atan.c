/*
    Math Runtime library - Copyright 1983,1984,1985,1986 by Green Hills Software Inc.
    This program is the property of Green Hills Software, Inc,
    its contents are proprietary information and no part of it
    is to be disclosed to anyone except employees of Green Hills
    Software, Inc., or as agreed in writing signed by the President
    of Green Hills Software, Inc.
*/
/****** mathieeesingtrans.library/IEEESPAtan ********************************
*
*   NAME
*	IEEESPAtan -- compute the arc tangent of number
*
*   SYNOPSIS
*	  x   = IEEESPAtan(  y  );
*	d0		   d0
*
*	single	x,y;
*
*   FUNCTION
*	Compute arctangent of y in IEEE single precision
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
**************************************************************************/
#define PI	3.14159265358979323846
#define PI2	1.57079632679489661923

float ratan(x)
float x;
{
    register int invrt=0;
    register int neg = 0;
    float x_sq;
    float f0;
    float rsqrt();
    /* Hart 5072 */
    /* returns (-PI/2,PI/2) */
    /* before I do the approximation, I use the following identity
	 atan(x) = 2*atan((sqrt(x^2+1)-1)/x)
	where the thing inside the atan on the right is between 0 and sqrt(2)-1
	    (=tan PI/8) instead of 0 and 1.
		x^2+1 <= (x^2+1)^2	for x in [0,1]
		sqrt(x^2+1) <= x^2+1	ignoring negative roots
		sqrt(x^2+1)-1 <= x^2
		(sqrt(x^2+1)-1)/x <= x
    */

    if (x < 0.0) {
	x = -x;
	neg = 1;
    }
    if (x > 1.0) {
	x = 1/x;
	invrt = 1;
    }
    x_sq = x*x;
    if (x < (PI/8.)) {
	f0 = x_sq;
	f0 += .48666399687883e1;
	f0 *= x_sq;
	f0 += .448250097927091e1;
	x /= f0;
	f0 = .2742666270116e0;
	f0 *= x_sq;
	f0 += .33724733791827e+1;
	f0 *= x_sq;
	f0 += .448250097798532e+1;
    } else {
	x = (rsqrt(x_sq+1)-1)/x;
	x_sq = x*x;
	f0 = x_sq;
	f0 += .48666399687883e1;
	f0 *= x_sq;
	f0 += .448250097927091e1;
	x /= f0;
	f0 = 2.0*.2742666270116e0;
	f0 *= x_sq;
	f0 += 2.0*.33724733791827e+1;
	f0 *= x_sq;
	f0 += 2.0*.448250097798532e+1;
    }
    f0 *= x;
    if (invrt)
	f0 = PI2-f0;
    if (neg)
	f0 = -f0;
    return(f0);
}
