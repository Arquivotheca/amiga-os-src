/**********************************************************************
*
*   $Id: halve.c,v 37.1 91/01/21 15:20:38 mks Exp $
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

#include "math.h"

halve(x)
double *x;
{
	int e;
	e = EXTRACTEXPONENT(*x);
	e--;	/* divide by two */
	if (e < MINIMUMEXPONENT)	*x = 0.0;
	else			 SETEXPONENT(*x,e);
}
