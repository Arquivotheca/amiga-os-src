/**********************************************************************
*
*   $Id: ldexp.c,v 37.1 91/01/21 15:20:24 mks Exp $
*
**********************************************************************/
/*
    C Runtime library - Copyright 1983,1984,1985,1986 by Green Hills Software Inc.
    This program is the property of Green Hills Software, Inc,
    its contents are proprietary information and no part of it
    is to be disclosed to anyone except employees of Green Hills
    Software, Inc., or as agreed in writing signed by the President
    of Green Hills Software, Inc.
*/
#include "math.h"

#ifdef KNOWN_FLOATING_POINT_FORMAT
double ldexp(value, exp)
double value;
{
    exp += EXTRACTEXPONENT(value);
    if (value != 0.0)
	if (exp > MAXIMUMEXPONENT || exp < MINIMUMEXPONENT)
	    /*errno = ERANGE*/;
	else
	    SETEXPONENT(value, exp);
    return(value);
}
#else

double ldexp(value, exp)
double value;
{
    double pwroftwo;

    if (value != 0.0) {
	if (exp > 0)
	    pwroftwo = 2.0;
	else {
	    exp = -exp;
	    pwroftwo = 0.5;
	}
	while (exp > 0) {
	    if (exp & 1)
		value *= pwroftwo;
	    pwroftwo *= pwroftwo;
	    exp >>= 1;
	}
    }
    return(value);
}
#endif
