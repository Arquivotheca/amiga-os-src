/* fmath.c */
/* Copyright (C) Agfa Compugraphic, 1989. All rights reserved. */

#include "debug.h"
#include "port.h"
#include "fmath.h"
#include "math.h"

/* WARNING: these macros assume x is a ULONG, not a LONG */

#define FLO(x)  (x & 0xffff)
#define FHI(x)  (x >> 16)

GLOBAL BOOLEAN  fmath_error;

/*------------------*/
/*     ifmul        */
/*------------------*/
/*  Multiply a WORD by a FIXED, return a rounded WORD  */
GLOBAL WORD
ifmul(x, y)
    WORD  x;
    FIXED y;
{
    WORD sign;
    UWORD xx;
    ULONG yy;
    ULONG p1, p2;
    WORD m;

    DBG3("ifmul(%x, %x.%04lx)", x, (WORD)FHI(y), FLO(y));

    sign = 1;
    if(x < 0)
    {
        sign = -sign;
        x = -x;
    }
    if(y < 0)
    {
        sign = -sign;
        y = -y;
    }

    xx = (UWORD)x;
    yy = (ULONG)y;

    p1 = xx * FLO(yy) + 0x8000;   /* 0x8000 rounds  */
    p2 = xx * FHI(yy);

    m = (WORD)(FHI(p1) + FLO(p2));

    if((m < 0) || FHI(p2))
    {
        DBG(" (overflow) ");
        fmath_error = TRUE;		/* FMATH_MUL_OV */
    }

    if(sign < 0) m = -m;

    DBG1(" = %x\n", m);
    return m;
}




/*------------------*/
/*      fmul        */
/*------------------*/
/*  Multiply a FIXED by a FIXED, return a FIXED  */
GLOBAL FIXED
fmul(x, y)
    FIXED x,y;
{
    double res1;
    FIXED res2;

    DBG4("fmul(%ld.%04ld, %ld.%04ld): ",
	    DBINT(x), DBFRAC(x), DBINT(y), DBFRAC(y));
    res1 = DShift(LMul(x, y), -16);
    res2 = DLoToL(res1);
    if (DHiToL(res1) != ((res2<0)?0x0000ffff:0)) {
	fmath_error = TRUE;		/* FMATH_MUL_OV */
	return(0);
    }
    DBG2("%ld.%04ld\n", DBINT(res2), DBFRAC(res2));
    return(res2);
}


/*------------------*/
/*      fdiv        */
/*------------------*/
/*  Divide a FIXED by a FIXED, return a FIXED  */
GLOBAL FIXED
fdiv(x,y)
    FIXED x,y;
{
    double res1;
    FIXED res2;

    DBG4("fdiv(%ld.%04ld, %ld.%04ld): ",
	    DBINT(x), DBFRAC(x), DBINT(y), DBFRAC(y));
    if (y == 0) {
	DBG("divide by zero\n");
	fmath_error = TRUE;		/* FMATH_DIV0 */
	return(0);
    }
    res1 = DDiv(DShift(LToD(x),16),LToD(y));
    DBG2("  $%08lx:%08lx  ", DHiToL(res1), DLoToL(res1));
    res2 = DLoToL(res1);
    if (DHiToL(res1) != ((res2<0)?-1:0)) {
	DBG("overflow\n");
	fmath_error = TRUE;		/* FMATH_DIV_OV */
	return(0);
    }
    DBG2("%ld.%04ld\n", DBINT(res2), DBFRAC(res2));
    return(res2);
}
