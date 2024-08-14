/*
**	$Id: q3.c,v 7.0 91/03/19 18:37:14 kodiak Exp $
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
**
**	$Log:	q3.c,v $
 * Revision 7.0  91/03/19  18:37:14  kodiak
 * after Amigaizing memory, lists, and adding UniCode.  Before cache work.
 * 
 * Revision 6.0  91/03/18  15:28:47  kodiak
 * folded in Bullet 1.30
 * 
 * Revision 3.0  90/11/09  17:12:40  kodiak
 * Kodiak's Alpha 1
 * 
*/
/* q3.c */
/* Copyright (C) Agfa Compugraphic, 1989. All rights reserved. */
/*---------------------------------------------------------------------
   6-26-90  bjg  Added arc_bitmap, arc_outline and arc_function for
                 outline processors.  Also added control0 and control1
                 for quadratic.
   9-21-90  jfd  Changed BITMAP to CGBITMAP due to conflict in "windows.h"

*/

/*?K?#include <stdio.h>*/
#include "debug.h"
#include "port.h"
#include "cgconfig.h"
#include "if_type.h"

#define dot(A,B) ( ((LONG)(A.x)*(LONG)(B.x)) + ((LONG)(A.y)*(LONG)(B.y)) )
#define crs(A,B) ( ((LONG)(A.x)*(LONG)(B.y)) - ((LONG)(A.y)*(LONG)(B.x)) )

EXTERN IF_DATA  d;
EXTERN WORD cpx[], cpy[];      /* curve points go here */

MLOCAL WORDVECTOR control0,control1;

EXTERN UWORD (*arc_function)();

#define nxpel  d.x.grid_shift


/*-----------------*/
/*   curve_pt      */
/*-----------------*/
MLOCAL VOID
curve_pt(ax, ay, n, P1x, P1y)
    UWORD *ax, *ay;
    UWORD n;
    UWORD  P1x, P1y;
{
    UWORD  P2x, P2y, P3x, P3y;
    UWORD  *cx, *cy;
    UWORD  n2;

    DBG5("curve_pt(%ld, %ld, %ld, %ld, %ld)\n", *ax, *ay, n, P1x, P1y);
    DBG2("         %ld %ld\n", *(ax+n), *(ay+n));

    n2 = n >> 1;

    P2x = (*ax + P1x) >> 1;
    P2y = (*ay + P1y) >> 1;

    P3x = (P1x + *(ax+n)) >> 1;
    P3y = (P1y + *(ay+n)) >> 1;

    cx = ax + n2;
    cy = ay + n2;

    *cx = (P2x + P3x) >> 1;
    *cy = (P2y + P3y) >> 1;
    DBG2("    %ld %ld\n", *cx, *cy);

    if(n2 > 1)
    {
        curve_pt(ax, ay, n2, P2x, P2y);
        curve_pt(cx, cy, n2, P3x, P3y);
    }
}


/*-----------------*/
/*   arc           */
/*-----------------*/
GLOBAL UWORD
arc(x0, y0, x1, y1, ax, ay, neg, pxs, pys)
    WORD x0, y0, x1, y1, ax, ay;
    BOOLEAN  neg;
    WORD *pxs, *pys;
{
    WORDVECTOR   B, D, E;
    LONG  div, fac;
    WORD  P0x, P0y, P1x, P1y, P3x, P3y;
    WORD  powr, indx;
    WORD  area;

    DBG6("arc(x0=%ld, y0=%ld, x1=%ld, y1=%ld, ax=%ld, ay=%ld,\n",
                                                  x0,y0,x1,y1,ax,ay);
    DBG1("        neg=%ld)\n", neg);

    B.x = x1 - x0;
    B.y = y1 - y0;
    fac = dot(B,B) >> 1;
    div =  ((LONG)(ax)*(LONG)(B.x)) + ((LONG)(ay)*(LONG)(B.y));
    DBG2("    B = (%ld,%ld)\n", B.x, B.y);
    DBG2("    fac = %ld     div = %ld\n", fac, div);

    if ( div == 0 )
    {
        E.x = 0;
        E.y = 0;
    }
    else
    {
        E.x = (WORD)((fac*(LONG)ax)/div) << 1;
        E.y = (WORD)((fac*(LONG)ay)/div) << 1;
    }
    DBG2("    E = (%ld,%ld)\n", E.x, E.y);

    P0x = x0 << 1;
    P0y = y0 << 1;

    P3x = x1 << 1;
    P3y = y1 << 1;

    if(neg)
    {
        P1x = P3x - E.x;
        P1y = P3y - E.y;
    }
    else
    {
        P1x = P0x + E.x;
        P1y = P0y + E.y;
    }
    return( (*arc_function)(neg,B,E,P0x,P0y,P1x,P1y,P3x,P3y,pxs,pys));
}

#if QUADRA
/*-----------------*/
/*   arc_outline   */
/*-----------------*/
GLOBAL UWORD
arc_outline(neg,B,E,P0x,P0y,P1x,P1y,P3x,P3y,pxs,pys)
    BOOLEAN neg;
    WORDVECTOR   B, E;
    WORD  P0x, P0y, P1x, P1y, P3x, P3y;
    WORD *pxs, *pys;
{
    WORD powr;

    if(neg)
    {
        control0.x = P1x >> 1;
        control0.y = P1y >> 1;
    }
    else
    {
        control1.x = P1x >> 1;
        control1.y = P1y >> 1;
    }

    powr = 1;

    *pxs = P0x;
    *pys = P0y;

    *(pxs + powr) = P3x;
    *(pys + powr) = P3y;

    return powr;
}
#endif

#if CGBITMAP | LINEAR
/*-----------------*/
/*   arc_bitmap    */
/*-----------------*/
GLOBAL UWORD
arc_bitmap(neg,B,E,P0x,P0y,P1x,P1y,P3x,P3y,pxs,pys)
    BOOLEAN neg;
    WORDVECTOR   B, E;
    WORD  P0x, P0y, P1x, P1y, P3x, P3y;
    WORD *pxs, *pys;
{
    WORDVECTOR   D;
    WORD  powr, indx;
    WORD  area;

    DBG1("        neg=%u)\n", neg);

  /*  Compute number of points to generate */

    D.x = B.x << 1;
    D.y = B.y << 1;
    area = (WORD)( crs(D,E) >> ( 2 + nxpel + 5 ) );
    if(area<0) area = -area;

    powr = 1;
    for ( indx = 0; indx < 4; indx++ ) {
        if ( area <= powr ) break; 
        powr = powr << 1;
    }
    DBG3( " area %ld   indx %ld  powr %ld\n", area, indx, powr);

    *pxs = P0x;
    *pys = P0y;

    *(pxs + powr) = P3x;
    *(pys + powr) = P3y;

    if(powr > 1)
        curve_pt(pxs, pys, powr, P1x, P1y);

    return (UWORD) powr;
}
#endif



/*-----------------*/
/*   q3            */
/*-----------------*/
GLOBAL UWORD
q3(xs, ys, xc, yc, xe, ye)
    WORD  xs, ys, xc, yc, xe, ye;
{
    WORDVECTOR   A;
    UWORD ct0, ct1, num_curve_pts;
    WORD  *pxs, *pys;
#ifdef DEBUG
    UWORD i;
#endif

    DBG6("q3(%ld, %ld, %ld, %ld, %ld, %ld)\n", xs, ys, xc, yc, xe, ye);

    A.x = xe - xs;
    A.y = ye - ys;

    pxs = &cpx[0];
    pys = &cpy[0];
    ct0 = arc(xs, ys, xc, yc, A.x, A.y, TRUE, pxs, pys);
    ct1 = arc(xc, yc, xe, ye, A.x, A.y, FALSE, pxs+ct0, pys+ct0);

    num_curve_pts = ct0 + ct1 - 1;

#ifdef DEBUG
    for(i=0; i<num_curve_pts+2; i++)
        DBG2("    %ld  %ld\n", cpx[i], cpy[i]);
#endif

    return num_curve_pts;
}


