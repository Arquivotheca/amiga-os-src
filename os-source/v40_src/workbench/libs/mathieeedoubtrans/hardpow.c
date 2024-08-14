/**********************************************************************
*								     *
*   Copyright 1985, Commodore-Amiga Inc.   All rights reserved.	     *
*   No part of this program may be reproduced, transmitted,	     *
*   transcribed, stored in retrieval system, or translated into	     *
*   any language or computer language, in any form or by any	     *
*   means, electronic, mechanical, magnetic, optical, chemical,	     *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030				     *
*								     *
**********************************************************************
*
*   $Id: hardpow.c,v 37.1 91/01/21 15:20:31 mks Exp $
*
**********************************************************************/

#include "math.h"


double hardpow(x,y,f)
double x,y;
double (*f)();
	/* compute x^y */
{
	int iy;

	if (y == 0.0)	return (1.0);

	/* are we raising X to an integer power? */
	if ( (floor(y) == y) && ((x <= 0.0) || (fabs(y) < 2000.0) ) )
	{
		int iy;
		double result;
		if (x == 0.0)
		{	/* raising 0 to an integer power */
			if (iy > 0)	return(0.0);
			else		return(1.0);
		}
		if (y > 0x7fffffff)
		{
			double t;
			/* the only way to get here is with x<0 */
			/* just use exp function */
			result = (*f)(-x,y);

			t = y;
			/* was y even or odd now */
			HALVE(t);
			if (y != (2 * IEEEDPFloor(t)))	result = -result;
			return (result);
		}
		iy = y;
		if (iy < 0)
		{
			iy = -iy;
			x = 1/x;
		}
		if (iy <= 3)
		{
			result = x;
			while (--iy)	result *= x;
			return (result);
		}
		else
		{	/* reduce and remultiply */
			int a,b;
			a = iy>>1;
			b = iy-a;
			result = hardpow(x,(double)a);
			result *= result;
			if (a != b)	result *= x; /* once more */
			return (result);
		}
	}
	else
	{
		if (x <= 0.0)	return (0.0);
		return( (*f)(x,y) );	/* call hardware routine */
		/* we call local hardware routine */
		/* because it can work in extended precision */
		/* for partial results */
	}
}
