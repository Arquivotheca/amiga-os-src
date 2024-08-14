

#include "math.h"

extern single IEEESPFloor();
extern single IEEESPAbs();

single hardpow(x,y,f)
single x,y;
single (*f)();
	/* compute x^y */
{
	int iy;

	if (y == 0.0)	return (1.0);

	/* are we raising X to an integer power? */
	if ( (IEEESPFloor(y) == y) && ((x <= 0.0) || (IEEESPAbs(y) < 2000.0) ) )
	{
		int iy;
		single result;
		if (x == 0.0)
		{	/* raising 0 to an integer power */
			if (iy > 0)	return(0.0);
			else		return(1.0);
		}
		if (y > 0xffffff)
		{
			single t;
			/* the only way to get here is with x<0 */
			/* just use exp function */
			result = (*f)(-x,y);

			t = y;
			/* was y even or odd now */
			HALVE(t);
			if (y != (2 * IEEESPFloor(t)))	result = -result;
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
			result = hardpow(x,(single)a);
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
