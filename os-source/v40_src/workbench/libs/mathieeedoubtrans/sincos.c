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
*   $Id: sincos.c,v 37.1 91/01/21 15:20:27 mks Exp $
*
**********************************************************************/

/* compute sin/cos if double at same time */
/* currently just hack it */
/****** mathieeedoubtrans.library/IEEEDPSincos ********************************
*
*   NAME
*	IEEEDPSincos -- compute the arc tangent of a floating point number
*
*   SYNOPSIS
*	  x   = IEEEDPSincos( z ,  y  );
*	d0/d1		     a0  d0/d1
*
*	double	x,y,*z;
*
*   FUNCTION
*	Compute sin and cosine of y in IEEE double precision.
*	Store the cosine in *z. Return the sine of y.
*
*   INPUTS
*	y - IEEE double precision floating point value
*	z - pointer to IEEE double precision floating point number
*
*   RESULT
*	x - IEEE double precision floating point value
*
*   BUGS
*
*   SEE ALSO
*	IEEEDPSin(), IEEEDPCos()
**************************************************************************/

double sin(),cos();

double sincos(f0,pcos)
double f0;
double *pcos;
{
	*pcos = cos(f0);
	return(sin(f0));
}
