/*
**	$Id: nxt_pt.c,v 7.0 91/03/19 18:37:11 kodiak Exp $
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
**
**	$Log:	nxt_pt.c,v $
 * Revision 7.0  91/03/19  18:37:11  kodiak
 * after Amigaizing memory, lists, and adding UniCode.  Before cache work.
 * 
 * Revision 6.0  91/03/18  15:28:16  kodiak
 * folded in Bullet 1.30
 * 
 * Revision 3.0  90/11/09  17:12:03  kodiak
 * Kodiak's Alpha 1
 * 
*/
/* nxt_pt.c */
/* Copyright (C) Agfa Compugraphic, 1989. All rights reserved. */
/*------------------------------------------------------------------------
   05-May-90  awr  Added landscpae. nxy() calls rot90.
   04-Jun90   awr  Initialized td->pixel->size and td->orig_pixel to
                   allow nx() and ny() to handle zero denominator and
                   intepolate points outside interval.
   04-Jul-90  awr  Changed nxy() to call des2wrkbm()
   26-Jun-90  bjg  Added first vertex code and current vertex code for quadra.
                   Made px1, py1 and loop_not_done GLOBAL not EXTERN.
   04-Sep-90  awr  Eliminated EXTERN ttx, tty; - not referenced.
------------------------------------------------------------------------*/

#include "debug.h"
#include "profile.h"
#include "port.h"
#include "imath.h"
#include "fmath.h"
#include "if_type.h"
#include "adj_skel.h"
#include "tr_type.h"


MLOCAL BOOLEAN no_skel;


EXTERN  IF_CHAR c;
EXTERN  IF_DATA d;
EXTERN adjusted_skel_type      x_skel[];
EXTERN adjusted_skel_type      y_skel[];

MLOCAL WORD first_vertex_code,current_vertex_code;

EXTERN  WORD    nx();
EXTERN  WORD    ny();
EXTERN  VOID    des2wrkbm();

EXTERN  UWORD   q3();

EXTERN  BOOLEAN loop_not_done;		/* KODIAK */
EXTERN  VOID    nxt_pt();

/* contour data */

EXTERN  WORD    px1, py1;		/* KODIAK */
MLOCAL  WORD    px10, py10;

GLOBAL  UWORD   quality;

MLOCAL  WORD    num_coords;
MLOCAL  WORD    next_vec_num;
MLOCAL  WORD   *pnx, *pny;
MLOCAL  BYTE   *ncx, *ncy;
MLOCAL  WORD    next_x, next_y;    /* next coordinate                  */
MLOCAL  WORD    next_cx, next_cy;  /* next contact point               */
MLOCAL  BOOLEAN contact;           /* TRUE if next_x,_y has contact pt */

MLOCAL  UWORD   num_curve_pts;
GLOBAL  WORD    cpx[33], cpy[33];
MLOCAL  WORD   *pcpx, *pcpy;       /* point to next point in above array */

/* transforms */

GLOBAL TRAN x_tran, y_tran;
MLOCAL WORD end_scale_seg;  /* last contour (index) in scale segment */




/*-------------------------------------------------------------------*/
/*        nxy()                                                      */
/*-------------------------------------------------------------------*/
MLOCAL VOID
nxy(x, y, newx, newy)
  WORD  x, y;
  WORD *newx, *newy;
{
    WORD xx, yy;

#ifdef PROFILE
if(prof_val >= PROF_nxy)
{
    *newx = 1000;
    *newy = 1000;
    return;
}
#endif

  /* Manipulate in design space */

    if(no_skel)
    {
        xx = x;
        yy = y;
    }
    else 
    {
        xx = nx(x);
        yy = ny(y);
    }

  /* Transform from design space to working bitmap space */

    des2wrkbm(xx, yy, newx, newy);
}




/*-------------------*/
/*  set_transform    */
/*-------------------*/
/*   Sets a new x or y scaling transform that is used by nx() and ny()
 *   to scale countour coordinate data.
 *
 *    The trasforms are specified by:
 *        old0
 *        num
 *        den
 *        rnd
 *
 *    The transform rule is:
 *
 *        x -> rnd + ((x - old0) * num) / den;   if den != 0 
 *        x -> rnd + (x - old0);                 if den == 0
 *
 *    The four transform parameters are returned in the structure
 *
 *        TRAN *td;
 *
 *    provided by the caller. There are two TRAN structures, one for x
 *    coordinate scaling and one for y coordinate scaling:
 *
 *        TRAN x_tran, y_tran;
 *
 *    In addition to the 4 transform parameters, this function returns
 *    the contour coordinate index of the first point in the next scaling
 *    segment. This is returned in
 *
 *        td->end
 *
 *
 */

MLOCAL VOID
set_transform(td)
      TRAN *td;     /* ptr to x or y transform data  */
{
    adjusted_skel_type  *skel;

    DBG("\n\n====set_transform=======================================\n");
    DBG1("     td->sct %ld\n", td->sct);

    skel = td->skel;   /* adjusted skel data */

    /* have we processed all sk pts ?  */
    if (td->sct == td->nsk)
    {
        /* last part of loop, scaling parameters should */
        /* be same as in first scaling segment.         */

        td->num      = td->num0;
        td->den      = td->den0;
        td->half_den = td->half_den0;
        td->end      = td->end0;
        td->old0     = td->old00;
        td->new0     = td->new00;
    }
    else
    {
 	   
      /* Vector index at which to start the next scaling segment */

        td->end   = *(td->skel_to_contr++);

      /*  Step to next pair of new pts and compute rnd & num  */

        td->new0 = td->new1;
        td->new1 = skel->adjusted;
        td->num  = td->new1 - td->new0;

      /* Step to next pair of original pts and compute den */

        td->old0 = td->old1;
        td->old1 = skel->original;
        td->den  =  td->old1 - td->old0;
        if(!td->den) td->num = td->den = 1;
        td->half_den = td->den / 2;

      /* set for next scaling segment  */

        td->skel++;
        td->sct += 1;

        DBG1("end %ld\n", td->end);
        DBG2("new0  %ld   new1  %ld\n", td->new0, td->new1);
        DBG2("old0  %ld   old1  %ld\n", td->old0, td->old1);
        DBG2("num %ld  den %ld\n\n\n",td->num,td->den);
    } 

}




/*---------------------------*/
/* init_trasnsform()        */
/*--------------------------*/
/*  Initialize transform data for the loop */

MLOCAL VOID
init_transform(td)
    TRAN *td;  
{
    td->nsk = *(td->num_skel_loop++);
    if (td->nsk < 2)
    {
        /*  According to Type Division, 0 skeletal points can
         *  occur in un-Intellifonted characters. 1 skel
         *  point can never happen- we ignore if it happens.
         */

        td->end      = num_coords;

        td->old0     = 0;
        td->new0     = 0;
        td->num      = 1;
        td->den      = 1;
        td->half_den = 0;
no_skel = TRUE;  
    }
    else
    {
no_skel = FALSE;

      td->sct = 0;         /* Number of scale segments processed           */

      td->old1 = (td->skel + td->nsk - 1)->original;
      td->new1 = (td->skel + td->nsk - 1)->adjusted;


      set_transform(td);

      /*  remember data from first scaling segment */
      /*  because we'll need it again when we come */
      /*  around the loop.                         */

            td->end0     = num_coords;  /*  just bigger than the last
                                         *  vector number so we'll never
                                         *  make a new scaling segment
                                         */

            td->num0      = td->num;
            td->den0      = td->den;
            td->half_den0 = td->half_den;
            td->old00     = td->old0;
            td->new00     = td->new0;
      }

}




/*-------------------*/
/*  np_init_char     */
/*-------------------*/
GLOBAL VOID
np_init_char()
{
    DBG("np_init_char()\n");

  /* Set ptrs to arrays of adjusted skel data */

    x_tran.skel = x_skel;
    y_tran.skel = y_skel;

  /* set ptrs to arrays of number of skel pts per loop */

    x_tran.num_skel_loop = c.xskel.num_skel_loop;
    y_tran.num_skel_loop = c.yskel.num_skel_loop;

  /* set pointers to loop order lists of skel pts (contour indices)  */

    x_tran.skel_to_contr = c.xskel.skel_to_contr;
    y_tran.skel_to_contr = c.yskel.skel_to_contr;

  /*  These values get added to the scaled points in the working
   *  space to shift the image to the lower left corner of the working
   *  space.
   */

    x_tran.margin = d.x.margin;
    y_tran.margin = d.y.margin;

    DBG2("x & y margin   %ld %ld\n", x_tran.margin, y_tran.margin);
}




/*-------------------*/
/*  np_init_loop     */
/*-------------------*/
GLOBAL VOID
np_init_loop(lp)
    LOOP *lp;
{
    WORD    num_contact;

  /* Initialize transform scaling data */

    DBG("np_init_loop()--------\n");


    loop_not_done = TRUE;

  /* pointers to coordinates and contact deltas */

    num_coords = lp->npnt;       /* number of coordinate points */
    pnx  = lp->x;                /* coordinate points */
    pny  = lp->y;
    num_contact = lp->nauxpt;    /* number of contact pts */
    ncx = lp->xa;                /* contact point offsets */
    ncy = lp->ya;

    DBG1("num_coords  = %ld\n", num_coords);
    DBG1("num_contact = %ld\n", num_contact);


    next_vec_num = -1;

    num_curve_pts = 0;

    next_x = *(pnx + num_coords - 1);
    first_vertex_code = contact = !(BOOLEAN)(next_x & 0x4000);
    current_vertex_code = !(BOOLEAN)(*pnx & 0x8000);
    next_x  = next_x & 0x3fff;

    next_y = *(pny + num_coords - 1) & 0x3fff;

    DBG2("    next_x, next_y  %ld, %ld\n", next_x, next_y);

    if(contact)
    {
        DBG("    contact\n");
        next_cx = (WORD)*(ncx + num_contact - 1);
        next_cy = (WORD)*(ncy + num_contact - 1);
    }
    else
    {
        DBG("    no contact\n");
        next_cx = (WORD)*ncx++;
        next_cy = (WORD)*ncy++;
    }
    DBG2("    next_cx, next_cy  %ld, %ld\n", next_cx, next_cy);

  /*  Initialize skeletal transforms */

    DBG("x init_transform()\n");
    init_transform(&x_tran);

    DBG("y init_transform()\n");
    init_transform(&y_tran);

    if (x_tran.end < y_tran.end)
        end_scale_seg = x_tran.end;
    else
        end_scale_seg = y_tran.end;



  /*  Get 1st point which is the last point of the loop
   *  and save, we'll return this point again at the end.
   */

    nxt_pt();
    px10 = px1;
    py10 = py1;

    DBG2("    px1, py1  %ld, %ld\n", px1, py1);
    DBG("End np_init_loop()--------\n");
}





/*-------------------*/
/*  nxt_pt()         */
/*-------------------*/
GLOBAL VOID
nxt_pt()
{
    WORD nx1, ny1;
    WORD cx, cy;
    WORD xc, yc, xe, ye;


    if(num_curve_pts)
    {
        DBG("                                           contact\n");
        px1 = *pcpx++ >> 1;
        py1 = *pcpy++ >> 1;
        num_curve_pts--;
    }
    else
    {
        /*  Set up next scaling segment  */

        if (next_vec_num == end_scale_seg)
        {
            if (next_vec_num == x_tran.end)   set_transform(&x_tran);
            if (next_vec_num == y_tran.end)   set_transform(&y_tran);

            if (x_tran.end < y_tran.end) end_scale_seg = x_tran.end;
                                    else end_scale_seg = y_tran.end;

            DBG1("end_scale_seg %ld\n",end_scale_seg);
        }




        if(num_coords)
        {
            nxy(next_x, next_y, &px1, &py1);

            if(quality > 1)
            {
                /* compute contact point for next time */

                if(contact)
                {
                    DBG("making contact\n");
                    nx1 = 0x3fff & *pnx;
                    ny1 = 0x3fff & *pny;

                    cx = ((next_x + nx1) >> 1) + next_cx;
                    cy = ((next_y + ny1) >> 1) + next_cy;
                    next_cx = (WORD)*ncx++;
                    next_cy = (WORD)*ncy++;
                    DBG2("cx xy  %ld %ld\n", cx, cy);

                    nxy(cx, cy, &xc, &yc);

                    pcpx = &cpx[1];  /* skip cp?[0]; it's the coordinate */
                    pcpy = &cpy[1];  /* point for q3; already in p?1.    */

                    if(quality == 2)
                    {
                        DBG("    Quality 2\n");
                        *pcpx = xc << 1;
                        *pcpy = yc << 1;
                        num_curve_pts = 1;
                    }
                    else /* quality level must be 3 */
                    {
                        DBG("    Quality 3\n");
                        nxy(nx1, ny1, &xe, &ye);
                        num_curve_pts = q3(px1, py1, xc, yc, xe, ye);
                    }
                }
                contact = !(BOOLEAN)(*pnx & 0x4000);
                current_vertex_code = !(BOOLEAN)(*pnx & 0x8000);
            }


            next_vec_num++;
            num_coords--;

            next_x = *pnx++ & 0x3fff;
            next_y = *pny++ & 0x3fff;
            DBG2("    next_x, next_y  %ld, %ld\n", next_x, next_y);
        }
        else
        {
          /* Return last point of loop */

            px1 = px10;
            py1 = py10;
            loop_not_done = FALSE;
        }
    }
}

