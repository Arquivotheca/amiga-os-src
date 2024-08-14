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
float *x;
{
	int e;
	e = EXTRACTFLOATEXPONENT(*x);
	e--;	/* divide by two */
	if (e < MINIMUMFLOATEXPONENT)	*x = 0.0;
	else			 SETFLOATEXPONENT(*x,e);
}
