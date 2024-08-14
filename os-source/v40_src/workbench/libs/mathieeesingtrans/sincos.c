

/* compute sin/cos if single at same time */
/* currently just hack it */
/****** mathieeesingtrans.library/IEEESPSincos ********************************
*
*   NAME
*	IEEESPSincos -- compute the arc tangent of a floating point number 
*
*   SYNOPSIS
*	  x   = IEEESPSincos( z ,  y  );
*	d0		     a0  d0
*
*	float	x,y,*z;
*
*   FUNCTION
*	Compute sin and cosine of y in IEEE single precision.
*	Store the cosine in *z. Return the sine of y.
*
*   INPUTS
*	y - IEEE single precision floating point value
*	z - pointer to IEEE single precision floating point number
*
*   RESULT
*	x - IEEE single precision floating point value
*
*   BUGS
*
*   SEE ALSO
*	IEEESPSin(), IEEESPCos()
**************************************************************************/

#include "math.h"

single rsin(),rcos();

single rsincos(f0,pcos)
single f0;
single *pcos;
{
	*pcos = rcos(f0);
	return(rsin(f0));
}
