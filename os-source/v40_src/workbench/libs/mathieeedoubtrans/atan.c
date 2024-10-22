/**********************************************************************
*
*   Source Control
*   --------------
*   $Header: atan.c,v 1.2 89/09/08 19:49:04 dale Exp $
*
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
/****** mathieeedoubtrans.library/IEEEDPAtan ********************************
*
*   NAME
*	IEEEDPAtan -- compute the arc tangent of number
*
*   SYNOPSIS
*	  x   = IEEEDPAtan(  y  );
*	d0/d1		   d0/d1
*
*	double	x,y;
*
*   FUNCTION
*	Compute arctangent of y in IEEE double precision
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
**************************************************************************/
#define PI	3.14159265358979323846
#define PI2	1.57079632679489661923

#define DEBUG

#ifdef DEBUG
dumplong(a,b)
long a,b;
{
	kprintf("(%lx,%lx)\n",a,b);
}
#endif

double atan(x)
double x;
{
    int invrt=0;
    int neg = 0;
    double x_sq;
    register double f0;
    double sqrt();
    /* Hart 5077 */
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
#ifdef DEBUG
	kprintf("x=");dumplong(x);
	kprintf("x_sq=");dumplong(x_sq);
#endif
    if (x < (PI/8.)) {
	f0 = x_sq;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 += .5895697050844462222791e2;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 *= x_sq;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 += .536265374031215315104235e3;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 *= x_sq;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 += .16667838148816337184521798e4;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 *= x_sq;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 += .207933497444540981287275926e4;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 *= x_sq;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 += .89678597403663861962481162e3;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	x /= f0;
#ifdef DEBUG
	kprintf("x=");dumplong(x);
#endif
	f0 = .161536412982230228262e2;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 *= x_sq;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 += .26842548195503973794141e3;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 *= x_sq;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 += .11530293515404850115428136e4;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 *= x_sq;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 += .178040631643319697105464587e4;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 *= x_sq;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 += .89678597403663861959987488e3;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
    } else {
	x = (sqrt(x_sq+1)-1)/x;
#ifdef DEBUG
	kprintf("sqrt=");dumplong(x);
#endif
	x_sq = x*x;
#ifdef DEBUG
	kprintf("x_sq=");dumplong(x_sq);
#endif
	f0 = x_sq;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 += .5895697050844462222791e2;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 *= x_sq;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 += .536265374031215315104235e3;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 *= x_sq;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 += .16667838148816337184521798e4;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 *= x_sq;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 += .207933497444540981287275926e4;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 *= x_sq;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 += .89678597403663861962481162e3;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	x /= f0;
#ifdef DEBUG
	kprintf("x=");dumplong(x);
#endif
	f0 = 2.0*.161536412982230228262e2;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 *= x_sq;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 += 2.0*.26842548195503973794141e3;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 *= x_sq;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 += 2.0*.11530293515404850115428136e4;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 *= x_sq;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 += 2.0*.178040631643319697105464587e4;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 *= x_sq;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
	f0 += 2.0*.89678597403663861959987488e3;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
    }
    f0 *= x;
#ifdef DEBUG
	kprintf(" before invert check f0=");dumplong(f0);
#endif
    if (invrt)
	f0 = PI2-f0;
    if (neg)
	f0 = -f0;
#ifdef DEBUG
	kprintf("f0=");dumplong(f0);
#endif
    return(f0);
}
