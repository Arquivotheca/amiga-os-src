/*
**	$Id: metrics.c,v 9.0 91/04/09 20:05:48 kodiak Exp $
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
**
**	$Log:	metrics.c,v $
 * Revision 9.0  91/04/09  20:05:48  kodiak
 * implement IFBITMAP x0,y0 x1,y1
 * note special case of empty bitmap
 * 
 * Revision 7.0  91/03/19  18:37:08  kodiak
 * after Amigaizing memory, lists, and adding UniCode.  Before cache work.
 * 
 * Revision 6.0  91/03/18  15:28:05  kodiak
 * folded in Bullet 1.30
 * 
 * Revision 3.0  90/11/09  17:11:53  kodiak
 * Kodiak's Alpha 1
 * 
*/
/* metrics.c */
/* Copyright (C) Agfa Compugraphic, 1989, 1990. All rights reserved. */
/*    03-27-90   jfd     In "metrics()", instead of splitting the difference
 *                       between the alternate width and the real width
 *                       and applying that value to both "x0" and "x1",
 *                       apply it only to "x0" and set "x1" equal to "x0"
 *                       plus the alternate width. This will ensure the
 *                       proper escapement. It was off by 1 design unit
 *                       in some cases.
 *    05-22-90   awr     Corrected yorigin calculation to use the grid
 *                       alligned baseline (d.aBaselnVal) instead of the
 *                       the unaligned version.
 *    05-22-90   awr     Added metric calculations for quadrant rotations
 *    09-04-90   awr     Declared des2wrkbm() to fix compiler warning,
 *                       removed rot90() declaration.
 *    05-Oct-90  awr     Added connecting character side bearing calculation
 */

#include <exec/types.h>
#include <exec/nodes.h>
#include "port.h"
#include "debug.h"
#include "imath.h"
#include "if_type.h"
#include "cgif.h"

EXTERN IF_CHAR  c;
EXTERN IF_DATA  d;
/* maker.c */
EXTERN VOID des2wrkbm();
/* pixel_al.c */
EXTERN pixel_align();
EXTERN struct {
                  WORD value;
                  WORD grid_line;
              } pixel_aligned;

/*----------------------*/
/*    inv_des2wrkbm     */
/*----------------------*/
MLOCAL VOID
inv_des2wrkbm(x, y, newx, newy)
  WORD  x, y;
  WORD *newx, *newy;
{
    WORD xx, yy;

    xx = (WORD)(   ( (LONG)(x - d.x.margin) * (LONG)d.x.p_pixel + 2048L)
                         / 4096
               );
    yy = (WORD)(   ( (LONG)(y - d.y.margin) * (LONG)d.y.p_pixel + 2048L)
                         / 4096
               );

    if(d.quadrant == ROT0)
    {
        *newx = xx;
        *newy = yy;
    }
    else if(d.quadrant == ROT270)
    {
        *newx = d.y.max_des - yy;
        *newy = xx;
    }
    else if(d.quadrant == ROT180)
    {
        *newx = d.x.max_des - xx;
        *newy = d.y.max_des - yy;
    }
    else if(d.quadrant == ROT90)
    {
        *newx = yy;
        *newy = d.x.max_des - xx;
    }
}




/*----------------------*/
/*    fp_to_16()        */
/*----------------------*/
/*  Convert working bitmap space fractional pixels to 16ths of a pixel */
MLOCAL WORD
fp_to_16(v, xy)
    WORD v;
    COORD_DATA *xy;
{
    WORD val;

    val = ABS(v);
    val = (WORD)(  ((LONG)val * 16 + xy->half_pixel) >> xy->grid_shift);

    if(v<0)
        return (WORD) -val;
    else
        return val;
}



/*----------------------*/
/*    metrics           */
/*----------------------*/
GLOBAL VOID
metrics(bm, x0, y0, x1, y1, des_bound)
    IFBITMAP  *bm;
    WORD     x0, y0;    /* Origin of current character (des units) */
    WORD     x1, y1;    /*    "    "  next      "        "    "    */
    BOX     *des_bound; /*  bounding box (des units)               */
{
    WORD orig_black_width;
    WORD new_black_width;
    WORD orig_lsb, orig_rsb;
    WORD new_lsb, new_rsb;
    WORD adj_width;

    WORDPOINT lb, rb;     /* left & right bound (fractional pixels) */
    WORDPOINT dlb, drb;   /* left & right bound (design units)      */
    WORDPOINT new_org;    /* character origin (design units)        */
    WORD px0, py0;        /* character origin (fractional pixels)   */
    WORD px1, py1;        /* next character origin (fractional pixels)   */

    WORD tmp;

    DBG("metrics()\n");
    DBG4("    original escape box: (%ld, %ld)  (%ld, %ld)\n", x0, y0, x1, y1);
  /*  Adjust escape box for alternate width (if any) */


    if(c.alt_width > 0)  /* not supposed to be negative, ignore if < 0 */
    {
      /*  Distribute the alternate width change equally (not proportionately)
       *  over each side bearing.
       */

        DBG1("    alternate width = %ld\n", c.alt_width);
        adj_width = (c.alt_width - (x1-x0)) / 2;
        x0 -= adj_width;
        x1  = x0 + c.alt_width;
        DBG4("    alt width escape box: (%ld, %ld)  (%ld, %ld)\n",
                                                  x0, y0, x1, y1);
    }

  /* Compute Escapement */

    bm->escapement   = x1 - x0;


  /*  Compute vector (xorigin, yorign) in 16ths of an output pixel.
   *      step 1. Compute (px0, py0) the character origin in working
   *              bitmap space.
   *      step 2. Compute (xorigin, yorigin).
   */

/*---------step 1-------------------------*/

  /*  If arbitray rotation, simply transfer the design orign to
   *   working bitmap space. If special 90 degree rotation, then we
   *   take into account the change in the character's black width
   *   and distribute this change equally over both side bearings.
   */

    if((!d.quadrant) ||		/* arbitrary rotation */
	(!bm->black_depth))	/* or empty bitmap */
    {
        DBG("    Arbitrary rotate or empty bitmap\n");

      /* Transform character origin to working bitmap space */

        des2wrkbm(x0, y0, &px0, &py0);
        des2wrkbm(x1, y1, &px1, &py1);
    }
    else                /* special 0, 90, 180, or 270 degree rotation */
    {
      /*  Intellifont is on.
       */

        if(d.quadrant==ROT0 || d.quadrant==ROT180)
        {
            lb.y = rb.y = 0;

          /* assume ROT0, swap if ROT180 */

            lb.x = bm->left_indent << d.x.grid_shift;
            rb.x = lb.x + (bm->black_width << d.x.grid_shift);
            if(d.quadrant == ROT180)
            {
                tmp = lb.x;
                lb.x = rb.x;
                rb.x = tmp;
            }
        }
        else
        {
            lb.x = rb.x = 0;

          /* Assume ROT270, swap if ROT90 */

            lb.y = (d.bmdepth - bm->top_indent) << d.y.grid_shift;
            rb.y = lb.y - (bm->black_depth << d.y.grid_shift);
            if(d.quadrant==ROT90)
            {
                tmp = lb.y;
                lb.y = rb.y;
                rb.y = tmp;
            }
        }

        inv_des2wrkbm(lb.x, lb.y, &dlb.x, &dlb.y);
        inv_des2wrkbm(rb.x, rb.y, &drb.x, &drb.y);

        DBG2("lb = (%ld, %ld)\n", lb.x, lb.y);
        DBG2("rb = (%ld, %ld)\n", rb.x, rb.y);
        DBG2("dlb = (%ld, %ld)\n", dlb.x, dlb.y);
        DBG2("drb = (%ld, %ld)\n", drb.x, drb.y);

        orig_lsb = des_bound->ll.x - x0;
        orig_rsb = x1 - des_bound->ur.x;

        if(c.ConnectingChar)
        {
            pixel_align (orig_lsb, d.px, R_TWO_I);
            new_lsb = pixel_aligned.value;
            pixel_align (orig_rsb, d.px, R_TWO_I);
            new_rsb = pixel_aligned.value;
        }
        else
        {
            orig_black_width = des_bound->ur.x - des_bound->ll.x;
            new_black_width  = drb.x - dlb.x;

            adj_width = (orig_black_width - new_black_width)/2;
	    DBG3("    orig_black_ %ld  new_black_ %ld  adj_width %ld\n",
		    orig_black_width, new_black_width, adj_width);
            new_lsb = orig_lsb + adj_width;
            new_rsb = orig_rsb + adj_width;
	    DBG2("    orig_lsb %ld    new_lsb %ld\n", orig_lsb, new_lsb);
	    DBG2("    orig_rsb %ld    new_rsb %ld\n", orig_rsb, new_rsb);
        }

        new_org.x = dlb.x - new_lsb;
        new_org.y = d.aBaselnVal;
        DBG2("new_org = (%ld, %ld)\n", new_org.x, new_org.y);
        des2wrkbm(new_org.x, new_org.y, &px0, &py0);

        new_org.x = drb.x + new_rsb;
        DBG2("new_org = (%ld, %ld)\n", new_org.x, new_org.y);
        des2wrkbm(new_org.x, new_org.y, &px1, &py1);
    }    

    DBG2("    (px0, py0) = (%ld, %ld)\n", px0,py0);
    DBG2("    (px1, py1) = (%ld, %ld)\n", px1,py1);

/*---------step 2-------------------------*/

    bm->xorigin = fp_to_16(px0, &d.x);
    bm->yorigin = fp_to_16((bm->depth << d.y.grid_shift)  - py0, &d.y);
    bm->x0 = (bm->xorigin + 8) >> 4;
    bm->y0 = (bm->yorigin + 8) >> 4;
    bm->x1 = (fp_to_16(px1, &d.x) + 8) >> 4;
    bm->y1 = (fp_to_16((bm->depth << d.y.grid_shift)  - py1, &d.y) + 8) >> 4;

    DBG2("    width, depth             %ld  %ld\n", bm->width, bm->depth);
    DBG2("    left_indent, top_indent  %ld  %ld\n",
                             bm->left_indent, bm->top_indent);
    DBG2("    black_width, black_depth %ld  %ld\n",
                             bm->black_width, bm->black_depth);
    DBG2("    xorigin, yorigin         %ld  %ld\n", bm->xorigin, bm->yorigin);
    DBG4("    x0, y0     %ld %ld     x1, y1     %ld %ld\n",
	    bm->x0, bm->y0, bm->x1, bm->y1);
    DBG1("    escapement              %ld\n", bm->escapement);
}

