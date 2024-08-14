/*
    Copyright (c) 1989 Mitchell/Ware Systems, Inc.
    
    May be used by Commodore-Amiga, Inc. for in-house purposes only.
    No part of this program, or any parts or modifications thereof, either
    in source-code or object-code, or library form, may be used by 
    Commodore, or any of its employees, either indepently, or in conjunction
    with Commordore, as, or as a part of , any public-domain or commercial
    product, program, or any type of software, teaching aid, etc. without
    the prior written permission of Mitchell/Ware Systems.

    This copyright notice must not be removed from the document of which
    it is attached.    
*/
/*****************************************************************************
    Ifunctions.c - Integer functions of Sin, Cos, and Sqrt

    Mitchell/Ware Systems          Version 1.12                8/9/87

______________________________________________________________________________  
 
    TRIG FUNCTIONS:
    
        ISetScale(number) EXECUTE before Icircle(). Changes scaling factor
                          (default is 65536) - refered to as 'scale'.

        Icircle(div)    how many parts to divide circle (360 for
                        degrees, 400 for gradians, or any
                        arbitrary value. Calls Iinit().
                        
        Iinit()     inits things for Icos & Isin
        
        Dinit()     deallocates resources used with Icos & Isin
        
        Isin(angle) the given angle is in degrees (or units 
                    specified by Icircle), result is scaled by 'scale'.

        Icos(angle) The given angle is in degrees (or units 
                    specified by Icircle), result is scaled by 'scale'.
______________________________________________________________________________  
    
    EXPONENTIAL FUNCTIONS:
 
        Isqrt(n)    Integral Square Root
       
        Ipow(x, n)  Raise integer x to power n

        Ilog2(n)    Find the integral log to the power of 2 of n (floored)
        
        I2floor(n)  Floor n to the nearest power of 2
        
        I2rnd(n)    Round off n to the nearest power of 2
*****************************************************************************/

#include "exec/types.h"
#include "math.h"

static double scale = 65536.0;

static long    *sin_table = NULL;
static long    *cos_table = NULL;
static double  slices = 360.0;     /* number of slices to divide circle */
static int iquart, ihalf, ithree, iwhole;

void ISetScale(o)  /* change scaling - EXECUTE before Icircle() */
long o;
{
    scale = o;
}

Icircle(div)   /* split circle into div parts - returns TRUE if successful */
int div;
{
   slices = div;
   return Iinit();
}

Iinit()    /* initializes system - returns TRUE if successful  */
{
   double  d, r, quarter;
   register long   *s, *c, i;

   if (!(sin_table = (long *) calloc((int) slices + 1, sizeof(long))))
       return FALSE;

   if (!(cos_table = (long *) calloc((int) slices + 1, sizeof(long))))
   {
       free(sin_table);
       return FALSE;
   }

   r = PI / (slices / 2.0);
   iquart = quarter = slices / 4.0;
   ihalf = 2 * iquart;
   ithree = 3 * iquart;
   iwhole = 4 * iquart;
   if (iwhole != slices)
   {
       printf("Ifunctions: Quantization Error");
       return FALSE;
   }

   for (d = 0.0, s = sin_table, c = cos_table; d <= quarter; d++)
       *s++ = sin(d * r) * scale;

   for (i = 0, s = sin_table, c = cos_table; i <= iquart; ++i)
   {
       s[ihalf-i] =               c[iquart-i] = c[ithree+i] = s[i];
       s[ihalf+i] = s[iwhole-i] = c[iquart+i] = c[ithree-i] = -s[i];
   }
   return TRUE;
}  /* uses 2888 bytes if slices == 360.0   */

void Dinit()        /* free resources   */
{
    if (sin_table)
        free(sin_table);
        
    if (cos_table)
        free(cos_table);
        
    cos_table = sin_table = NULL;
}

long   Isin(degrees)
register int   degrees;
{
   if ((degrees %= iwhole) < 0)
       degrees += iwhole;

   return sin_table[degrees];
}

long   Icos(degrees)
register int   degrees;
{
   if ((degrees %= iwhole) < 0)
       degrees += iwhole;

   return cos_table[degrees];
}

ULONG  Isqrt(n)
ULONG  n;
{
   register i, r, rb, b;

   for (i = 15, r = 0; i >= 0; --i)
       if ( (rb = r+(b=1<<i)) * rb <= n)
           r += b;
   return (ULONG) r;
}

long Ipow(x, n)
long x, n;
{
    register y = 1;
    
    while (n-- > 0)
        y *= x;
    
    return y;
}

ULONG I2floor(n)
ULONG n;
{
    register ULONG r;
    
    for (r = 1 << 31; r; r >>= 1)
        if (r & n)
            return r;
    
    return -1;  /* error */    
}

ULONG Ilog2(n)
ULONG n;
{
    register ULONG i, u, r;

    for (i = 0, u = 1, r = I2floor(n); u; u <<= 1, ++i)
        if (u & r)
            return i;
    
    return -1; /* error */
}

ULONG I2rnd(n)
ULONG n;
{
    register r;
    
    r = I2floor(n);
    if (n - r < (r<<1) - n)
        return r;
    else
        return r<<1; 
}
