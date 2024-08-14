/*
    Math Runtime library - Copyright 1983,1984,1985,1986 by Green Hills Software Inc.
    This program is the property of Green Hills Software, Inc,
    its contents are proprietary information and no part of it
    is to be disclosed to anyone except employees of Green Hills
    Software, Inc., or as agreed in writing signed by the President
    of Green Hills Software, Inc.
*/
/****** mathieeesingtrans.library/IEEESPLog ********************************
*
*   NAME
*	IEEESPLog -- compute the natural logarithm of a floating point number
*
*   SYNOPSIS
*	  x   = IEEESPLog(  y  );
*	d0	          d0
*
*	float	x,y;
*
*   FUNCTION
*	Compute ln(y) in IEEE single precision
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
*	IEEESPExp()
**************************************************************************/

#include "math.h"
#define LOG2	0.69314718055994530942
#define LOG10	2.30258509299404568402

/* takes the natural log of a number.  First we reduce the number to
	between sqrt(2) and 1/sqrt(2) (by removing any exponant)
	then we take the log of this, and add the exponent*LOG2
	the procedure for the log is HART 2705
*/


float rlog(x)
float x;
{
    int exp;
    float z, zsq, temp;

    if (x <= 0.0) {
	/*errno = EDOM;*/
	return(0.0);
    }
    temp = x;
    exp = EXTRACTFLOATEXPONENT(temp);
    SETFLOATEXPONENT(temp, 0);
    if (temp < 0.707106781186547524) {		/* 1/sqrt(2) */
	SETFLOATEXPONENT(temp, 1);
	exp--;
    }
    x = temp;
    z = x;
    z -= 1.0;
    x += 1.0;
    z /= x;
    zsq = z;
    zsq *= z;
    x = 0.89554061525e+0;
    x *= zsq;
    x += -0.331355617479e+1;
    x *= z;
    zsq += -0.165677797691e+1;
    x /= zsq;
    z = exp;
    z *= LOG2;
    x += z;
    return(x);
}




/****** mathieeesingtrans.library/IEEESPLog10 ********************************
*
*   NAME
*	IEEESPLog10 -- compute logarithm base 10 of a number
*
*   SYNOPSIS
*	  x   = IEEESPLog10(  y  );
*	d0	            d0
*
*	float	x,y;
*
*   FUNCTION
*	Compute the logarithm base 10 of y in IEEE single precision
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
*	IEEESPLog()
**************************************************************************/

single rlog10(x)
single x;
{
    return(rlog(x)*(1/LOG10));
}
