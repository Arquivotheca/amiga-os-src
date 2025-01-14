/*  yclass.c */
/* Copyright (C) Agfa Compugraphic, 1989. All rights reserved. */

/*  16-May-89 awr  Cloned from if 2.11 yclass.c
    07-Jul-90 awr  added xy COORD_DATA parameter for quadrant rotation
                   removed skeletal_scale()
---------------------------------------------------------------------*/


#include  "debug.h"
#include  "profile.h"
#include  "port.h"
#include  "imath.h"
#include  "segments.h"
#include  "if_type.h"
#include  "adj_skel.h"


/*
#include        "skel_def.h"
#include        "if_def.h"
#define GLOBAL_EXTERN   EXTERN
#include        "char_i6.h"
#include        "param_id.h"
#include        "pixels.h"
*/


EXTERN VOID  first_loop();     /*  if_init.c  */
EXTERN LOOP* next_loop();

EXTERN pixel_align();
EXTERN struct {
                  WORD value;
                  WORD grid_line;
              } pixel_aligned;

EXTERN FACE     face;
EXTERN IF_CHAR  c;
EXTERN IF_DATA  d;

GLOBAL adjusted_skel_type      x_skel[MAX_SKEL];
GLOBAL adjusted_skel_type      y_skel[MAX_SKEL];


/*--------------------------------------------------------------------*/
/*                                                                    */
/*      skeletal_init ()                                              */
/*              sets the adjusted skeletal data arrays to their       */
/*              initial values                                        */
/*                                                                    */
/*--------------------------------------------------------------------*/


GLOBAL BOOLEAN
skeletal_init ()
{

    COUNTER i, j, ky, kx, total_y, total_x;
    LOOP  *lp;         /* pointer to current loop strcture */
    WORD  *x, *y;     /* pointers to coordinates */
    adjusted_skel_type  *xs, *ys;
    WORD  index;
    WORD  val;

    ky=0;
    kx=0;
    xs = x_skel;
    ys = y_skel;
    total_y = 0;
    total_x = 0;

   DBG ("skeletal_init()  orig  new\n");

   /* for each contour loop */
    first_loop();
    for (i=0; i<c.num_loops; i++)
    {
        DBG1("    loop %ld\n", i);
        DBG2("    c.yskel.num_skel_loop[%ld] = %ld\n",
                                        i, c.yskel.num_skel_loop[i]);
        DBG2("    c.xskel.num_skel_loop[%ld] = %ld\n",
                                        i, c.xskel.num_skel_loop[i]);

        lp = next_loop();

        /* if skel array exceeded, return error */
        total_y += c.yskel.num_skel_loop[i];
        total_x += c.xskel.num_skel_loop[i];
        if ((total_y > MAX_SKEL) || (total_x > MAX_SKEL))
            return FALSE;

        x = lp->x;
        y = lp->y;
        for (j=0; j < c.yskel.num_skel_loop[i]; j++)
        {
            index = c.yskel.skel_to_contr[ky];
	    val    = *(y+index) & 0x3fff;

            ys->original          = val;
            ys->intermed          = val;
            ys->adjusted          = val;
            ys->pixel_grid_line   = NOT_ALIGNED;
            ys->processing_status = NOT_PROC;
            ys->parent = ROOT;       /* dave, don't need this */
            DBG2("    yskel[%ld]   %ld\n", index, val);

            ys++;
            ky++;
        }
        for (j=0; j < c.xskel.num_skel_loop[i]; j++)
        {
            index = c.xskel.skel_to_contr[kx];
	    val    = *(x+index) & 0x3fff;

            xs->original          = val;
            xs->intermed          = val;
            xs->adjusted          = val;
            xs->pixel_grid_line   = NOT_ALIGNED;
            xs->processing_status = NOT_PROC;
            xs->parent = ROOT;       /* dave, don't need this */
            DBG2("    xskel[%ld]   %ld\n", index, val);

            xs++;
            kx++;
        }
    }


  /* check data integrity */
#if 0
    if ((ky != c.yskel.num_skel) || (kx != c.xskel.num_skel))
    {
        DBG("BAD SKELETAL DATA\n");
        DBG2("    total number of y,x skels = %ld, %ld\n",
                                     c.yskel.num_skel, c.xskel.num_skel);
        DBG2("    number processed = %ld, %ld\n", ky, kx);
        return FALSE;
    }
#endif

    return TRUE;
}



#if 0

/*-----------------------*/
/*  skeletal_scale       */
/*-----------------------*/
GLOBAL VOID
skeletal_scale()
{

    COUNTER i, j;
    adjusted_skel_type  *xs, *ys;

    xs = x_skel;
    ys = y_skel;

   DBG ("skeletal_scale()  orig  new\n");

   /* for each contour loop */
    for (i=0; i<c.num_loops; i++)
    {
        DBG1("\nLoop %ld    Y-skel\n", i);
        for (j=0; j < c.yskel.num_skel_loop[i]; j++)
        {
             DBG2("    %ld    %ld\n", ys->adjusted, ty(ys->adjusted));

#ifdef PROFILE
if(prof_val < PROF_txy_skel_init)
#endif
            ys->adjusted          = ty(ys->adjusted);

            ys++;
        }
        DBG("\n           X-skel\n");
        for (j=0; j < c.xskel.num_skel_loop[i]; j++)
        {
             DBG2("    %ld    %ld\n", xs->adjusted, tx(xs->adjusted));
#ifdef PROFILE
if(prof_val < PROF_txy_skel_init)
#endif
            xs->adjusted          = tx(xs->adjusted);
            xs++;
        }
    }
}

#endif


/*----------------------*/
/*  pix_align           */
/*----------------------*/
MLOCAL WORD
pix_align(val)
    WORD val;
{
    pixel_align(val, d.py, 2);
    return pixel_aligned.value;
}


/**********************************************************************/
/*                                                                    */
/*      yclass_proc ()                                                */
/*              completes the entire Y Class manipulation process by  */
/*              appropriately adjusting all Y skeletal root point     */
/*              values in y_skel.adjusted                             */
/*                                                                    */
/**********************************************************************/

GLOBAL VOID
yclass_proc ()
{
    WORD    baseline_adj;   /*  implement baseline offset concept */
    COUNTER i,j,k;

    WORD   class_i;
    WORD   ihigh, ilow;
    WORD   orig_low, orig_high;
    WORD   adj_low,  adj_high;
    WORD   root_i;
    WORD  *adj_skel_ptr;

    DBG ("\n\n-------------------yclass_proc()--------------------\n");

    if (c.baseline_offset != 0)
    {
        baseline_adj = pix_align(c.baseline_offset) - c.baseline_offset;
        DBG1("    baseline_adj based on baseline offset = %ld\n", baseline_adj);

    }
    else
        baseline_adj = 0;
    DBG1("    baseline_adj = %ld\n",baseline_adj);



    k = 0;
    DBG1("%ld yclass assignment(s) in character\n", c.num_yclass_ass);
    for (i=0; i < c.num_yclass_ass; i++)  /* for each yclass assignment */
    {
        class_i = c.yclass_i[i];

        DBG2("\nyclass assignment %ld  definition index %ld\n",i, class_i);

        if (c.num_yclass_def)   /* if character has local yclass defs */
        {
            ilow  = c.yline_low_i [class_i];
            ihigh = c.yline_high_i[class_i];
            DBG2("    local yclass indices low = %ld  high = %ld\n",
                                                         ilow, ihigh);

            /* Note: local yclass def can use either loc or glob ylines. */
            if (c.num_ylines)      /* i.e. local ylines available */
            {
                DBG("        local ylines\n");
                orig_low  = c.ylines[ilow];
                orig_high = c.ylines[ihigh];
            }
            else
            {
                DBG("        global ylines\n");
                orig_low  = face.ylines[ilow];
                orig_high = face.ylines[ihigh];
            }
        }
        else
        {
           /*  Note: global yclass defs must use global ylines  */

            orig_low  = face.ylines[face.glob_yclass.yline_low_i[class_i]];
            orig_high = face.ylines[face.glob_yclass.yline_high_i[class_i]];

            DBG2("    Global yclass indices low = %ld  high = %ld\n",
                                   face.glob_yclass.yline_low_i[class_i],
                                   face.glob_yclass.yline_high_i[class_i]);
        }

        adj_high = pix_align(orig_high - d.origBaseline) + d.aBaselnVal;
        adj_low  = pix_align(orig_low  - d.origBaseline) + d.aBaselnVal;

        DBG2 ("        original hi,lo %ld %ld\n", orig_high, orig_low);
        DBG2 ("        aligned  hi,lo %ld %ld\n", adj_high,  adj_low);




        DBG1("\n\n    %ld roots in this assignment:\n", c.num_root_p_ass[i]);
        for (j=0; j < c.num_root_p_ass[i]; j++)  /* each root in assnmt */
        {
            root_i = c.root_skel_i[k];
            adj_skel_ptr = &y_skel[root_i].adjusted;

            DBG2("        y_skel[%ld] before yclass adjustment    = %ld\n",
                           root_i, *adj_skel_ptr);

            /* if root skel is above yclass, then shift */
            if (*adj_skel_ptr - c.baseline_offset >= orig_high)
            {

                *adj_skel_ptr += adj_high - orig_high + baseline_adj;

        DBG2("            y_skel[%ld] is above yclass, adj y_skel = %ld\n",
                            root_i, *adj_skel_ptr);

            }

            /*  else if root skel's below yclass, then shift */
            else if (*adj_skel_ptr - c.baseline_offset <= orig_low)
            {
                *adj_skel_ptr += adj_low - orig_low + baseline_adj;

     DBG2("            y_skel[%ld] is below yclass, adj y_skel = %ld\n",
                            root_i, *adj_skel_ptr);

            }

            /* else, root skel is within, so scale */
            else
            {
                *adj_skel_ptr =
                    scale_iii(*adj_skel_ptr - c.baseline_offset - orig_low,
                                  adj_high - adj_low,
                                  orig_high - orig_low
                              )
                            + adj_low + baseline_adj + c.baseline_offset;

       DBG2("            y_skel[%ld] is within yclass, adj y_skel = %ld\n",
                          root_i, *adj_skel_ptr);

            }

            k++;    /* NO CHECK FOR OVERRUNNING ROOT ARRAY*/

        }  /* for ( j = ...  */
    }  /* for ( i = ... */
}

