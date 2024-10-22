/* bold.c */
/* Copyright (C) Agfa Compugraphic, 1990. All rights reserved.
 *
 *  28-Jul-90 awr  created
 *  12-Oct-90 tbh  In bold(), computing point of intersection "dp->x",
 *                 "dp->y" differently in the event that both X and Y
 *                 skeletal data do not exist for a duplicate point.
 *  15-Oct-90 awr  In intersect(), check that intersection point does
 *                 not exceed bounding box
 */

#include "debug.h"
#include "profile.h"
#include "port.h"
#include "imath.h"
#include "if_type.h"
#include "adj_skel.h"
#include "tr_type.h"

#define TEST_INTERSECT 0    /* test against floating point math        */
#define DLONG_TEST 0        /* print debugging in DLONG math functions */


#ifdef  LOCAL_DLONG
/*-------------------------------------------------------------*/
/*                      l o n g    m a t h                     */

#define LMATH_DIV0   1
#define LMATH_DIV_OV 2

MLOCAL UWORD Lmath_code;



#define HIBIT 0x80000000

/* LT() and LE() only work for positive x and y */
#define LT(x, y) ((x.hi<y.hi) || ( x.hi == y.hi && x.lo <  y.lo))
#define LE(x, y) ((x.hi<y.hi) || ( x.hi == y.hi && x.lo <= y.lo))

#define LEFTSHIFT(x)  x.hi<<=1; if(x.lo&HIBIT) x.hi|=1; x.lo<<=1
#define RIGHTSHIFT(x) x.lo>>=1; if(x.hi&1) x.lo|=HIBIT; x.hi>>=1
#define FLO(x)  (x & 0xffff)
#define FHI(x)  (x >> 16)


typedef struct
        {
            ULONG lo;
            ULONG hi;
        } DLONG;



#define DLONG_2_LONG(x)   (x & 0xffffffff)
/*------------------*/
/*   LONG_2_DLONG   */
/*------------------*/
MLOCAL DLONG
LONG_2_DLONG(x)
    LONG x;
{
    DLONG r;

    r.lo = x;

    if(x<0)
        r.hi = -1;
    else
        r.hi = 0;

    return r;
}


/*------------------*/
/*       Lneg       */
/*------------------*/
/* return -x */
MLOCAL DLONG
Lneg(x)
    DLONG x;
{
    DLONG r;

    x.lo = ~x.lo;
    x.hi = ~x.hi;
    r.lo = x.lo + 1;
    r.hi = x.hi;
    if(!(r.lo & HIBIT) && (x.lo & HIBIT))   /* Check for carry into r.hi */
        r.hi++;

    return r;
}




#if DLONG_TEST

#define K64 65536.0

MLOCAL VOID
print_DLONG(x)
    DLONG x;
{
    DLONG r;

    if(x.hi & HIBIT)
    {
        r = Lneg(x);
        DBG("- ");
    }
    else
    {
        r.lo = x.lo;
        r.hi = x.hi;
        DBG("  ");
    }
    DBG1("%0.f   ", K64 * (K64 * (K64 * FHI(r.hi) + FLO(r.hi))
                                + FHI(r.lo)) + FLO(r.lo)    );

    DBG2("(%lx %lx)\n", x.hi, x.lo);
}
#endif




/*------------------*/
/*       Ladd       */
/*------------------*/
/* return x + y */
MLOCAL DLONG
Ladd(x, y)
    DLONG x, y;
{
    DLONG r;
    BOOLEAN xhi, yhi;

#if DLONG_TEST
    DBG("Ladd()\n");
    DBG("      ");
    print_DLONG(x);
    DBG("    + ");
    print_DLONG(y);
#endif

    r.lo = x.lo + y.lo;
    r.hi = x.hi + y.hi;

  /*  There is a carry into r.hi in two cases:
   *      - if hi bit of exactly one of x.lo or y.lo is on
   *        and the hi bit of r.lo is off
   *      - the hi bits of both x.lo and x.hi are on
   */

    xhi = (x.lo & HIBIT) != 0;
    yhi = (y.lo & HIBIT) != 0;
    if( (!(r.lo & HIBIT) && (xhi != yhi))  ||   (xhi && yhi))
        r.hi++;

#if DLONG_TEST
    DBG("---------------------------------\n");
    print_DLONG(r);
#endif
    return r;
}




/*------------------*/
/*       Lsub       */
/*------------------*/
/* return x - y */
MLOCAL DLONG
Lsub(x, y)
    DLONG x, y;
{
    return Ladd(x, Lneg(y));
}




/*------------------*/
/*       Lmul       */
/*------------------*/
MLOCAL DLONG
Lmul(x, y)
    LONG x, y;
{
    WORD sign;
    ULONG a1, a2, a3, a4;   /* Partial products */
    DLONG p;                /* Product          */

  /*  We'll work with positive numbers so remember sign of product */

    sign = 1;
    if(x < 0)
    {
        x = -x;
        sign = -sign;
    }
    if(y < 0)
    {
        y = -y;
        sign = -sign;
    }

  /* compute partial products */
 
    a1 = FLO(x) * FLO(y);
    a2 = FHI(x) * FLO(y);
    a3 = FLO(x) * FHI(y);
    a4 = FHI(x) * FHI(y);

  /* Add partial products */

    a4 += FHI(a2) + FHI(a3);
    a2 =  FLO(a2) + FLO(a3) + FHI(a1);
    a4 += FHI(a2);
    a1 =  FLO(a1) + (a2<<16);

    p.hi = a4;
    p.lo = a1;

    if(sign < 0) p = Lneg(p);
    return p;
}




/*------------------*/
/*       Ldiv       */
/*------------------*/
/*  Divide a DLONG by a long and return a LONG
 *  Returns x / divisor
 *  Errors:
 *      Lmath_code = LMATH_DIV0     if divide by 0
 *                 = LMATH_DIV_OV   if result won't fit in a LONG
 */
MLOCAL LONG
Ldiv(x, divisor)
    DLONG x;
    LONG  divisor;
{
    WORD  sign;   /* sign of quotient             */
    UWORD qct;    /* significant bits in quotient */
    LONG  q;      /* quotient                     */
    DLONG d;      /* divisor                      */

  /* Check for divide by 0 */

    if(!divisor)
    {
        Lmath_code  = LMATH_DIV0;
        return 0L;
    }

    if(!x.lo && !x.hi) return 0L;

  /*  We'll work with positive numbers so remember sign of quotient */

    sign = 1;
    if(x.hi & HIBIT)
    {
        x = Lneg(x);
        sign = -sign;
    }
    if(divisor < 0)
    {
        divisor = -divisor;
        sign = -sign;
    }

  /* Round */

    x = Ladd(x, LONG_2_DLONG(divisor/2));

  /* if x < divisor the quotient is 0 */

    if(!x.hi && (x.lo < divisor)) return 0L;

  /* Load the divisor */

    d.hi = 0;
    d.lo = divisor;

  /*  x >= d so shift d to the left as far as possible so that x >= d.
   *  qct = number of significant bits in quotient
   */

    qct = 1;
    while(LE(d, x))
    {
        LEFTSHIFT(d);
        qct++;
    }
    RIGHTSHIFT(d);
    qct--;
 
    if(qct > 31)
    {
        Lmath_code  = LMATH_DIV_OV;
        return 0L;
    }

    q = 0;
    while(qct)
    {
        q <<=1;
        if(LE(d,x))
        {
            q |= 1;

          /* x -= d */

            if(d.lo > x.lo)    /* check for borrow */
                x.hi -= 1;
            x.lo -= d.lo;
            x.hi -= d.hi;
        }
        LEFTSHIFT(x);
        qct--;
    }
 
    if(sign < 0) q = -q;
 
    return q;
}

/*                      l o n g    m a t h                     */
/*-------------------------------------------------------------*/
#else

#include "math.h"
#define  DLONG	double

#endif

/*-------------------------------------------------------------*/
/*                  b o l d     i n t e r s e c t              */


typedef struct
{
    UWORD loop;
    UWORD coord_index;   /* coordinate index within loop */
    UWORD x;
    UWORD y;
    UBYTE xsk0, xsk1, ysk0, ysk1;
} DUP_POINTS;

#define MAX_DUP_POINTS 48
EXTERN DUP_POINTS dup_points[];


EXTERN  IF_CHAR c;
EXTERN adjusted_skel_type      x_skel[];
EXTERN adjusted_skel_type      y_skel[];

/* if_init.c */
EXTERN VOID     first_loop();
EXTERN LOOP    *next_loop();

/* nxt_pt.c */
EXTERN VOID     np_init_char();
EXTERN TRAN x_tran, y_tran;




#if TEST_INTERSECT

MLOCAL BOOLEAN
test_intersect(xi0, yi0, xi1, yi1, xo0, yo0, xo1, yo1, xx, yy)
    WORD xi0, yi0, xi1, yi1, xo0, yo0, xo1, yo1;
    WORD *xx, *yy;
{
    double a0, b0, a1, b1;
    double c0, c1;
    double det, half_det;
    double x_det, y_det;

    DBG("test_intersect()\n");
    DBG4("    (%ld, %ld)  (%ld, %ld)\n", xi0, yi0, xi1, yi1);
    DBG4("    (%ld, %ld)  (%ld, %ld)\n", xo0, yo0, xo1, yo1);

  /*  For the two lines, determine equations of the form:
   *
   *        a0*x + b0*y = c0
   *        a1*x + b1*y = c1
   */

    a0 = (double)(yi1 - yi0);
    b0 = (double)(xi0 - xi1);
    c0 = (double)((LONG)xi0*(LONG)yi1 - (LONG)yi0*(LONG)xi1);

    a1 = (double)(yo1 - yo0);
    b1 = (double)(xo0 - xo1);
    c1 = (double)((LONG)xo0*(LONG)yo1 - (LONG)yo0*(LONG)xo1);

#ifdef  FLOAT_DEBUG
    DBG3("    a0 b0 c0    %f  %f  %f\n", a0, b0, c0);
    DBG3("    a1 b1 c1    %f  %f  %f\n", a1, b1, c1);
#endif

    /*  Solve equations using determinants */

    det = a0*b1 - a1*b0;
    half_det = (det+1)/2;

#ifdef  FLOAT_DEBUG
    DBG1("    det  %f\n", det);
#endif

    if(!det)
    {
        DBG("    lines do not intersect\n");
        return FALSE;    /* lines don't intersect */
    }


    x_det = c0*b1 - c1*b0;
    y_det = a0*c1 - a1*c0;

    DBG1("    x_det %f\n", x_det);
    DBG1("    y_det %f\n", y_det);

    *xx = (WORD)((x_det + half_det) / det);
    *yy = (WORD)((y_det + half_det) / det);

    DBG2("    intersection (%ld, %ld)\n", *xx, *yy);

    return TRUE;
}
#endif




MLOCAL BOOLEAN
no_intersect(dp)
    DUP_POINTS  *dp;
{
    DBG("    lines do not intersect\n");

    dp->xsk0 = 255;
    dp->xsk1 = 255;
    dp->ysk0 = 255;
    dp->ysk1 = 255;

    return FALSE;    /* lines don't intersect */
}

MLOCAL BOOLEAN
intersect(xi0, yi0, xi1, yi1, xo0, yo0, xo1, yo1, dp)
    WORD xi0, yi0, xi1, yi1, xo0, yo0, xo1, yo1;
    DUP_POINTS  *dp;
{
    LONG a0, b0, c0, a1, b1, c1;
    LONG det;

    DLONG x_det, y_det;
    DLONG xl, yl;
    LONG  x, y;
#if TEST_INTERSECT
    WORD xx, yy;

    test_intersect(xi0, yi0, xi1, yi1, xo0, yo0, xo1, yo1, &xx, &yy);
#endif

    DBG("intersect()\n");
    DBG4("    (%ld, %ld)  (%ld, %ld)\n", xi0, yi0, xi1, yi1);
    DBG4("    (%ld, %ld)  (%ld, %ld)\n", xo0, yo0, xo1, yo1);

  /*  For the two lines, determine equations of the form:
   *
   *        a0*x + b0*y = c0
   *        a1*x + b1*y = c1
   */

    a0 = (LONG)(yi1 - yi0);
    b0 = (LONG)(xi0 - xi1);
    c0 = (LONG)xi0*(LONG)yi1 - (LONG)yi0*(LONG)xi1;

    a1 = (LONG)(yo1 - yo0);
    b1 = (LONG)(xo0 - xo1);
    c1 = (LONG)xo0*(LONG)yo1 - (LONG)yo0*(LONG)xo1;

    DBG3("    a0 b0 c0    %ld  %ld  %ld\n", a0, b0, c0);
    DBG3("    a1 b1 c1    %ld  %ld  %ld\n", a1, b1, c1);

    /*  Solve equations using determinants */

    det = a0*b1 - a1*b0;

    DBG1("    det  %ld\n", det);

    if(!det)
        return no_intersect(dp);

    x_det = DSub(LMul(c0, b1), LMul(c1, b0));    /* c0*b1 - c1*b0 */
    y_det = DSub(LMul(a0, c1), LMul(a1, c0));    /* a0*c1 - a1*c0 */

#if DLONG_TEST
    DBG("    x_det ");
    print_DLONG(x_det);
    DBG("    y_det ");
    print_DLONG(y_det);
#endif

#ifdef LOCAL_DLONG
    Lmath_code = 0;
#endif
    xl = DDiv(x_det, LToD(det));
    x = DLoToL(xl);
    yl = DDiv(y_det, LToD(det));
    y = DLoToL(yl);
    if(     /* check for overflow or negative */
	    DHiToL(xl) || DHiToL(yl) ||
	    /* Check that we do not exceed bounding box   (10-15-90 awr) */
	    (x < (LONG)c.metric->bound_box.ll.x) ||
            ((LONG)c.metric->bound_box.ur.x < x) ||
            (y < (LONG)c.metric->bound_box.ll.y) ||
            ((LONG)c.metric->bound_box.ur.y < y))
        return no_intersect(dp);

    dp->x = (WORD)x;
    dp->y = (WORD)y;

    DBG2("    intersection (%ld, %ld)\n", dp->x, dp->y);
#if TEST_INTERSECT
    if((dp->x != xx) || (dp->y != yy))
    {
        DBG("***difference in intersections:\n");
        DBG2("    float: (xx, yy) = (%ld, %ld)\n", xx, yy);
        DBG2("    int:   (x, y)   = (%ld, %ld)\n", dp->x, dp->y);

    }
#endif

    return TRUE;
}




MLOCAL VOID
set_tran(td, first_skel_ind, last_skel_ind)
    TRAN *td;
    UWORD  first_skel_ind, last_skel_ind;
{
    adjusted_skel_type *skel0, *skel1;

    skel0 = td->skel + td->sct + first_skel_ind;
    skel1 = td->skel + td->sct + last_skel_ind;

    td->new0 = skel0->adjusted;
    td->new1 = skel1->adjusted;
    td->num  = td->new1 - td->new0;

    td->old0 = skel0->original;
    td->old1 = skel1->original;
    td->den  =  td->old1 - td->old0;
    if(!td->den) td->num = td->den = 1;
    td->half_den = td->den / 2;

    DBG2("new0  %ld   new1  %ld\n", td->new0, td->new1);
    DBG2("old0  %ld   old1  %ld\n", td->old0, td->old1);
    DBG2("num %ld  den %ld\n\n\n",td->num,td->den);
}

/* 10-12-90 tbh  Added argument "pskelMinus1" to find_in_scale_seg() */

MLOCAL VOID
find_in_scale_seg(coord_index, s2c_base, td, pskel, pskelMinus1)
    UWORD   coord_index;
    UWORD  *s2c_base;
    TRAN   *td;                       /* ptr to x or y transform data  */
    UBYTE  *pskel, *pskelMinus1;      /* 10-12-90 tbh */
{
    UWORD i;
    UWORD *p;
    UWORD  first_skel_ind, last_skel_ind;

    DBG1("find_in_scale_seg(%ld)\n", coord_index);
    DBG1("    nsk = %ld\n", td->nsk);

    for(i=0, p=s2c_base + td->sct; i<td->nsk; i++, p++)
    {
        DBG1("    %ld\n", *p);
        if(*p >= coord_index)
        {
            DBG("      found it\n");
            break;
        }
    }
    if(i==0 || i==td->nsk)  /* scale seg = last skel to first skel */
    {
        first_skel_ind = td->nsk - 1;
        last_skel_ind  = 0;
    }
    else
    {
        first_skel_ind = i - 1;
        last_skel_ind = i;
    }
    DBG2("    first_skel_ind %ld    last_skel_ind %ld\n",
                                 first_skel_ind, last_skel_ind);

    *pskel = -1;    /* 255 */
    *pskelMinus1 = (UBYTE)(td->sct + first_skel_ind);  /* 10-12-90 tbh */
    if(i<td->nsk)
        if(*p == coord_index)
            *pskel = (UBYTE)(td->sct + last_skel_ind);

    set_tran(td, first_skel_ind, last_skel_ind);
}

/* 10-12-90 tbh  Added argument "pskelPlus1" to find_out_scale_seg() */

MLOCAL VOID
find_out_scale_seg(coord_index, s2c_base, td, pskel, pskelPlus1)
    UWORD   coord_index;
    UWORD  *s2c_base;
    TRAN   *td;                       /* ptr to x or y transform data  */
    UBYTE  *pskel, *pskelPlus1;       /* 10-12-90 tbh */
{
    UWORD i;
    UWORD *p;
    UWORD  first_skel_ind, last_skel_ind;

    DBG1("find_out_scale_seg(%ld)\n", coord_index);
    DBG1("    nsk = %ld\n", td->nsk);

    i=td->nsk-1;
    for(p=s2c_base + td->sct + i; i>0; i--, p--)
    {
        DBG1("    %ld\n", *p);
        if(*p <= coord_index)
        {
            DBG("      found it\n");
            break;
        }
    }
    if(i==-1 || i==td->nsk-1)  /* scale seg = last skel to first skel */
    {
        first_skel_ind = td->nsk - 1;
        last_skel_ind  = 0;
    }
    else
    {
        first_skel_ind = i;
        last_skel_ind = i + 1;
    }
    DBG2("    first_skel_ind %ld    last_skel_ind %ld\n",
                                 first_skel_ind, last_skel_ind);

    *pskel = -1;   /* 255 */
    *pskelPlus1 = (UBYTE)(td->sct + last_skel_ind);  /* 10-12-90 tbh */
    if(i>=0)
        if(*p == coord_index)
            *pskel = (UBYTE)(td->sct + first_skel_ind);

    set_tran(td, first_skel_ind, last_skel_ind);
}




GLOBAL VOID
bold()
{
    DUP_POINTS *dp;
    LOOP  *lp;           /* current loop */
    UWORD  loopct;
    UWORD  id;           /* coordinate index of duplicate point in loop */
    UWORD  *x_s2c_base;
    UWORD  *y_s2c_base;
    UWORD   i0;
    UBYTE   xskPlus1, yskPlus1, xskMinus1, yskMinus1;  /* 10-12-90 tbh */

    WORD xi0, yi0, xi1, yi1, xo0, yo0, xo1, yo1;


    DBG("bold()\n");

  /* Initialize */

    dp = dup_points;
    np_init_char();
    x_tran.sct = 0;
    y_tran.sct = 0;

    x_s2c_base = c.xskel.skel_to_contr;
    y_s2c_base = c.yskel.skel_to_contr;

    DBG1("    number of loops = %ld\n", c.num_loops);

    first_loop();
    for(loopct = 0; loopct < c.num_loops; loopct++)
    {
        if(dp->loop == -1) break;

        DBG("\n\n\nN e x t    L o o p (bold)\n");

        x_tran.nsk = *(x_tran.num_skel_loop++);
        y_tran.nsk = *(y_tran.num_skel_loop++);

        lp = next_loop();

        while(dp->loop == loopct)
        {
          /* set id = index of first duplicate point */

            id = dp->coord_index;      /* index of second duplicate point */
            if(id == 0)
                id = lp->npnt - 1;
            else
                id--;

          /* find incoming scaling segment */

            find_in_scale_seg(id, x_s2c_base, &x_tran, &dp->xsk0, &xskMinus1);  /* 10-12-90 tbh */
            find_in_scale_seg(id, y_s2c_base, &y_tran, &dp->ysk0, &yskMinus1);  /* 10-12-90 tbh */

          /* compute incoming line segment */

            if(id == 0)
                i0 = lp->npnt - 1;
            else
                i0 = id - 1;

            xi0  = *(lp->x + i0) & 0x3fff;         /* coordinate points */
            yi0  = *(lp->y + i0) & 0x3fff;
            xi1  = *(lp->x + id) & 0x3fff;
            yi1  = *(lp->y + id) & 0x3fff;

            DBG4("    in  old: (%ld, %ld) (%ld, %ld)\n", xi0, yi0, xi1, yi1);

            xi0  = nx(xi0);
            yi0  = ny(yi0);
            xi1  = nx(xi1);
            yi1  = ny(yi1);

            DBG4("    in  new: (%ld, %ld) (%ld, %ld)\n", xi0, yi0, xi1, yi1);

          /* find outgoing scaling segment */

            id = dp->coord_index;     /* index of second duplicate point */

            find_out_scale_seg(id, x_s2c_base, &x_tran, &dp->xsk1, &xskPlus1); /* 10-12-90 tbh */
            find_out_scale_seg(id, y_s2c_base, &y_tran, &dp->ysk1, &yskPlus1); /* 10-12-90 tbh */

          /* 10-12-90 tbh  The following code was added */
        
            if ( ( (dp->xsk0 == 255)  /* if no x-skel at this duplicate . .   */
                        && 
                   (dp->xsk1 == 255) 
                 )                    /* and contour is not horizontal . .    */
                 &&                
                 ( ((y_skel+dp->ysk1)->original != (y_skel+yskPlus1)->original) 
                        || 
                   ((y_skel+dp->ysk1)->original != (y_skel+yskMinus1)->original) 
                 )
               )
            {
                if      (dp->ysk0 == 255) dp->y = (y_skel+dp->ysk1)->adjusted;
                else if	(dp->ysk1 == 255) dp->y = (y_skel+dp->ysk0)->adjusted;
                else    dp->y = ((y_skel+dp->ysk1)->adjusted + (y_skel+dp->ysk0)->adjusted + 1) >> 1;
            }

            else if
               ( ( (dp->ysk0 == 255)  /* if no y-skel at this duplicate . .   */
                        && 
                   (dp->ysk1 == 255) 
                 )                    /* and contour is not vertical . .      */
                 && 
                 ( ((x_skel+dp->xsk1)->original != (x_skel+xskPlus1)->original) 
                        || 
                   ((x_skel+dp->xsk1)->original != (x_skel+xskMinus1)->original) 
                 )
               )
            {
                if      (dp->xsk0 == 255) dp->x = (x_skel+dp->xsk1)->adjusted;
                else if	(dp->xsk1 == 255) dp->x = (x_skel+dp->xsk0)->adjusted;
                else    dp->x = ((x_skel+dp->xsk1)->adjusted + (x_skel+dp->xsk0)->adjusted + 1) >> 1;
            }

            else
            {
                /* compute outgoing line segment */
     
                i0 = id + 1;
                if(i0 == lp->npnt )
                    i0 = 0;
     
                xo0  = *(lp->x + i0) & 0x3fff;         /* coordinate points */
                yo0  = *(lp->y + i0) & 0x3fff;
                xo1  = *(lp->x + id) & 0x3fff;
                yo1  = *(lp->y + id) & 0x3fff;
     
                DBG4("    out old: (%ld, %ld) (%ld, %ld)\n",
		    xo0, yo0, xo1, yo1);
     
                xo0  = nx(xo0);
                yo0  = ny(yo0);
                xo1  = nx(xo1);
                yo1  = ny(yo1);
     
                DBG4("    out new: (%ld, %ld) (%ld, %ld)\n",
		    xo0, yo0, xo1, yo1);
      
              /* compute intersection of incomming and outgoing line segments */
     
                intersect(xi0, yi0, xi1, yi1, xo0, yo0, xo1, yo1, dp);
            }

          /* move to next duplicate point */

            dp++;

        } /*  while(dp->loop == loopct) */

        x_tran.sct += x_tran.nsk;
        y_tran.sct += y_tran.nsk;

    } /* end of character */

  /*  Change skeletal points  */

    for(dp = dup_points; dp->loop != -1; dp++)
    {
        if(dp->xsk0 != 255) (x_skel+dp->xsk0)->adjusted = dp->x;
        if(dp->xsk1 != 255) (x_skel+dp->xsk1)->adjusted = dp->x;
        if(dp->ysk0 != 255) (y_skel+dp->ysk0)->adjusted = dp->y;
        if(dp->ysk1 != 255) (y_skel+dp->ysk1)->adjusted = dp->y;
    }
}

