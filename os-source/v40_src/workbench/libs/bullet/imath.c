/*
**	$Id: imath.c,v 3.0 90/11/09 17:10:08 kodiak Exp $
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
**
**	$Log:	imath.c,v $
 * Revision 3.0  90/11/09  17:10:08  kodiak
 * Kodiak's Alpha 1
 * 
*/
/************************************************************************
**    imath.c                                                          **
**    -------------------------------------------------------------    **
**    Copyright 1986-90 by Agfa Compugraphic. All rights reserved.     **
*************************************************************************/
/*
 *  04-Jun90  awr  changed nx() and ny() to scale "move as" if v is
 *                 outside original interval.
 *
 */

#include "debug.h"
#include "port.h"
#include "adj_skel.h"
#include "tr_type.h"
#include "fmath.h"
#include "if_type.h"

EXTERN IF_DATA  d;
EXTERN TRAN    x_tran,y_tran;

/*  tx() and ty() tranform from the design space to the working
 *  power of two space.
 */

/*-------------------*/
/*    tx             */
/*-------------------*/
GLOBAL WORD
tx(v)
    WORD v;
{
    union {
              LONG l;
#if BYTEORDER == LOHI
              struct {WORD lo, hi;} w;
#else
              struct {WORD hi, lo;} w;
#endif
           } val;

    DBG1("tx(%ld)",v);

    val.w.lo = 0;
    val.w.hi = v;
    val.l = val.l >> 4;
    DBG1(" = %ld\n", (WORD)((val.l + d.x.p_half_pix) / d.x.p_pixel));
    return (WORD)((val.l + d.x.p_half_pix) / d.x.p_pixel);
}

/*-------------------*/
/*    ty             */
/*-------------------*/
GLOBAL WORD
ty(v)
    WORD v;
{
    union {
              LONG l;
#if BYTEORDER == LOHI
              struct {WORD lo, hi;} w;
#else
              struct {WORD hi, lo;} w;
#endif
           } val;

    DBG1("ty(%ld)",v);

    val.w.lo = 0;
    val.w.hi = v;
    val.l = val.l >> 4;
    DBG1(" = %ld\n", (WORD)((val.l + d.y.p_half_pix) / d.y.p_pixel));
    return (WORD)((val.l + d.y.p_half_pix) / d.y.p_pixel);

}




/*------------------*/
/*    nx            */
/*------------------*/
GLOBAL WORD
nx(v)
  WORD v;
{
return
    (WORD)( ( (  (LONG)(v-x_tran.old0) * (LONG)x_tran.num + x_tran.half_den )
                                 /x_tran.den
          )   + x_tran.new0);
}


/*------------------*/
/*    ny            */
/*------------------*/
GLOBAL WORD
ny(v)
  WORD v;
{
return
    (WORD)( ( (  (LONG)(v-y_tran.old0) * (LONG)y_tran.num + y_tran.half_den )
                                 /y_tran.den
          )   + y_tran.new0);
}


/*-------------------------------------------------------------------*/
/*        multiply_i_i()                                             */
/*-------------------------------------------------------------------*/

LONG
multiply_i_i (x, y)

        WORD x;
        WORD y;
{
        return ((LONG)x * (LONG)y);
}



/*-------------------------------------------------------------------*/
/*        scale_iii()                                                */
/*-------------------------------------------------------------------*/

WORD
scale_iii (x, y, z)

        WORD x,y,z;
{
        return (
                  (WORD)(
                           ( (LONG)x  * (LONG)y ) / (LONG)z
                        )
               );
}




/*-------------------*/
/*  scale_rem        */
/*-------------------*/
GLOBAL WORD
scale_rem(a, p, q, ptr_rem, ptr_deltax1)
    WORD a,p, q;
    WORD *ptr_rem;
    WORD *ptr_deltax1;
{
    WORD x, rem, deltax1;

    DBG3("scale_rem(%ld, %ld, %ld)\n", a, p, q);

    x   = scale_iii(a, p, q);
    rem = (WORD)((LONG)a * (LONG)p  -  (LONG)x * (LONG)q);
    DBG2("    x %ld   rem  %ld\n", x, rem);

    rem <<= 1;
    if(rem <0) rem = - rem;
    *ptr_rem = rem;

  /*  deltax1 */

    if(x>0)              deltax1 = x + 1;
    else if(x<0)         deltax1 = x -1;
    else /* x == 0 */
    {
        if(p>0) deltax1 = x + 1;
        else    deltax1 = x -1;
    }

    *ptr_deltax1 = deltax1;
    return x;
}

