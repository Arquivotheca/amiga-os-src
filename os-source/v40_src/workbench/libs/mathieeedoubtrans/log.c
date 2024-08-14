/**********************************************************************
*
*   $Id: log.c,v 37.1 91/01/21 15:20:06 mks Exp $
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
/****** mathieeedoubtrans.library/IEEEDPLog ********************************
*
*   NAME
*	IEEEDPLog -- compute the natural logarithm of a floating point number
*
*   SYNOPSIS
*	  x   = IEEEDPLog(  y  );
*	d0/d1	          d0/d1
*
*	double	x,y;
*
*   FUNCTION
*	Compute ln(y) in IEEE double precision
*
*   INPUTS
*	y - IEEE double precision floating point value
*
*   RESULT
*	x - IEEE double precision floating point value
*
*   BUGS
*
*   SEE ALSO
*	IEEEDPExp()
**************************************************************************/

#include "math.h"
#define LOG2	0.69314718055994530942
#define LOG10	2.30258509299404568402

/* takes the natural log of a number.  First we reduce the number to
	between sqrt(2) and 1/sqrt(2) (by removing any exponant)
	then we take the log of this, and add the exponent*LOG2
	the procedure for the log is HART 2705
*/

/*#define DEBUG*/
double log(x)
double x;
{
    int exp;
    double z, zsq, temp;

#ifdef DEBUG
	kprintf("log(%lx.%lx)\n",x);
#endif

    if (x <= 0.) {
	return(0.0);
    }
    temp = x;
    exp = EXTRACTEXPONENT(temp);
    SETEXPONENT(temp, 0);
    if (temp < 0.707106781186547524) {		/* 1/sqrt(2) */
	SETEXPONENT(temp, 1);
	exp--;
    }
    x = temp;
    z = x - 1.0;
    x += 1.0;
    z /= x;
    zsq = z*z;
#ifdef DEBUG
	kprintf("zsq = %lx.%lx    ",zsq);
#endif
    x = 0.4210873712179797145;
    x *= zsq;
#ifdef DEBUG
	kprintf("x *=zsq = %lx.%lx\n",x);
#endif
    x += -0.96376909336868659324e1;
#ifdef DEBUG
	kprintf(" check 1 x=%lx.%lx z=%lx.%lx &",x,z);
#endif
    x *= zsq;
#ifdef DEBUG
	kprintf(" check 2 x=%lx.%lx z=%lx.%lx &",x,z);
#endif
    x += 0.30957292821537650062264e2;
    x *= zsq;
    x += -0.240139179559210509868484e2;
    x *= z;
#ifdef DEBUG
	kprintf(" in middle x=%lx.%lx z=%lx.%lx &",x,z);
#endif
    z = zsq;
    z += -0.89111090279378312337e1;
    z *= zsq;
    z += 0.19480966070088973051623e2;
    z *= zsq;
    z += -0.120069589779605254717525e2;
#ifdef DEBUG
	kprintf("x=%lx.%lx z=%lx.%lx &",x,z);
#endif
    x /= z;
#ifdef DEBUG
	kprintf("x=%lx.%lx\n",x);
#endif
    z = exp;
#ifdef DEBUG
	kprintf("z=%lx.%lx\n",z);
#endif
    z *= LOG2;
#ifdef DEBUG
	kprintf("z=%lx.%lx\n",z);
#endif
    x += z;
#ifdef DEBUG
	kprintf("(%lx.%lx)\n",x);
#endif
    return(x);
}
/****** mathieeedoubtrans.library/IEEEDPLog10 ********************************
*
*   NAME
*	IEEEDPLog10 -- compute logarithm base 10 of a number
*
*   SYNOPSIS
*	  x   = IEEEDPLog10(  y  );
*	d0/d1	            d0/d1
*
*	double	x,y;
*
*   FUNCTION
*	Compute the logarithm base 10 of y in IEEE double precision
*
*   INPUTS
*	y - IEEE double precision floating point value
*
*   RESULT
*	x - IEEE double precision floating point value
*
*   BUGS
*
*   SEE ALSO
*	IEEEDPLog()
**************************************************************************/

double log10(x)
double x;
{
    return(log(x)*(1/LOG10));
}
