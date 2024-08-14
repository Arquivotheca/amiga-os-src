/*
    Math Runtime library - Copyright 1983,1984,1985,1986 by Green Hills Software Inc.
    This program is the property of Green Hills Software, Inc,
    its contents are proprietary information and no part of it
    is to be disclosed to anyone except employees of Green Hills
    Software, Inc., or as agreed in writing signed by the President
    of Green Hills Software, Inc.
*/
/****** mathieeesingtrans.library/IEEESPPow ********************************
*
*   NAME
*	IEEESPPow -- raise a number to another number power
*
*   SYNOPSIS
*	  z   = IEEESPPow(  x  ,  y  );
*	d0	          d1 d0
*
*	float	x,y,z;
*
*   FUNCTION
*	Compute y^x in IEEE single precision
*
*   INPUTS
*	x - IEEE single precision floating point value
*	y - IEEE single precision floating point value
*
*   RESULT
*	z - IEEE single precision floating point value
*
*   BUGS
*
*   SEE ALSO
**************************************************************************/

#include "math.h"

extern single hardpow();

static single explog(x,y)
single x,y;
{
	single rexp(),rlog();
	return( rexp( y*rlog( x )));
}
single rpow( x ,y )
single x,y;
{
	single result;
	result = hardpow(x,y,explog);
	return (result);
}

