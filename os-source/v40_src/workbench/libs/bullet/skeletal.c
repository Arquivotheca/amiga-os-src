/*
**	$Id: skeletal.c,v 6.0 91/03/18 15:28:57 kodiak Exp $
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
**
**	$Log:	skeletal.c,v $
 * Revision 6.0  91/03/18  15:28:57  kodiak
 * folded in Bullet 1.30
 * 
 * Revision 3.0  90/11/09  17:14:08  kodiak
 * Kodiak's Alpha 1
 * 
*/
/* skeletal.c */
/* Copyright (C) Agfa Compugraphic, 1989. All rights reserved. */

/*  17-May-89  awr  Cloned from if 2.11 skeletal.c
    05-Oct-90  awr  added root adjust parameter to skel_proc() for
                    connecting characters
    29-Nov-90  tbh  Missing pixel fix in skel_proc()
----------------------------------------------------------------------*/

#include  "debug.h"
#include  "port.h"
#include  "if_type.h"
#include  "adj_skel.h"
#include  "segments.h"


#define ALIGN_MASK      0x08    /*  alignment attribute mask: bit 3   */
#define CON_RTZ_MASK    0x14    /*  const && non-RTZ dim mask for HQ3 */
#define R_TYPE_MASK     0x03    /*  value to mask for "R" types       */
#define DIM_MASK        0x04    /*  dimension attribute mask: bit 2   */
#define HQ3_CON_MASK    0x10    /*  constrained dimension mask (HQ3)  */
#define DIM_RTZ_MASK    0x01    /*  dimension RTZ mask                */
#define HQ2_CON_MASK    0x02    /*  constrained dimension mask (HQ2)  */
#define STAN_DIM_OFF    0x00    /* 11-29-90 tbh                       */
#define STAN_DIM_ON     0x01    /* 11-29-90 tbh                       */


#if 0
#define NO_STANDARD_DIM 0xffff  /*  id for none .standard_value       */
#define DIM_NOT_PROC    -2      /*  status: dimension not processed   */
#define HQ2             2       /*  HQ2 data format                   */
#define HQ3             3       /*  HQ3 data format                   */
#endif


EXTERN FACE face;

MLOCAL void (*pix_al_ptr)();   /* ptr to pixel_align() function      */
EXTERN void pixel_align();
GLOBAL struct {
                  WORD value;
                  WORD grid_line;
              } pixel_aligned;


/*----------------------*/
/*   sd_pixel_align     */
/*----------------------*/
MLOCAL VOID
sd_pixel_align(val, xy, round_i)
    WORD val;
    COORD_DATA *xy;
    WORD  round_i;
{
    WORD  value;
    WORD  abs_value;
    WORD  half_pix;


    abs_value = ABS(val);
    half_pix  = xy->p_half_pix >> xy->bin_places;


  /*  If val is none zero (i.e., significantly non-zero) */

    if (abs_value > half_pix)
    {

      /*  value is difference between input val and standard dimension */

        value = ABS(abs_value - xy->stand_value);

      /*  if value is within one half pixel of standard dimension */

        if (value <= half_pix)
        {
          /* aligned value is equal to aligned standard dimension */

            pixel_aligned.value     =  xy->st_value[round_i];
            pixel_aligned.grid_line =  xy->st_pixels[round_i];

          /* aligned data must have same sign as val */

            if(val < 0)
            {
                pixel_aligned.value     = -pixel_aligned.value;
                pixel_aligned.grid_line = -pixel_aligned.grid_line;
            }

  DBG4("sd_pixel_align: val = %ld  round_i = %ld  new val = %ld   grid = %ld\n",
               val, round_i, pixel_aligned.value, pixel_aligned.grid_line);
            return;
        }
    }

  /* otherwise, do normal grid alignment */

    pixel_align(val, xy, round_i);
}




/**********************************************************************/
/*                                                                    */
/*      hq3_assoc_proc ( assoc_i, skel_i, attribute )                 */
/*              adjusts associated point "assoc_i" by an amount equal */
/*              to that used to adjust its skeletal point "skel_i".   */
/*                                                                    */
/**********************************************************************/


MLOCAL VOID
hq3_assoc_proc ( assoc_i, skel_i, attribute, skel,xy)
    UBYTE   assoc_i;
    UBYTE   skel_i;
    UBYTE   attribute;
    adjusted_skel_type *skel;
    COORD_DATA         *xy;
{

    DBG2("       hq3_assoc_proc: start assoc skel [%ld] = %ld\n",
                        assoc_i,
                        skel[assoc_i].adjusted);

    /* Assume r types enabled- awr */

    skel[assoc_i].round_i = attribute & R_TYPE_MASK;

    /* identify this associate's parent in case of need when pixel aligned */
    skel[assoc_i].parent = skel_i;


    /* &&&  temporary to alleviate problem from unresolved convergency !  */
    if (skel[assoc_i].processing_status == PROCESSED) return;

    /* if this association is allowed to RTZ and is not constrained  */
    if ( (attribute & CON_RTZ_MASK) == DIM_MASK )
    {

        skel [assoc_i].adjusted +=
                        skel [skel_i].adjusted - skel [skel_i].intermed;
        DBG ("       hq3_assoc_proc: this association allowed to RTZ !\n");
    }
    /* else, this association must be considered for non-RTZ and constraint. */
    else
    {
        skel[assoc_i].processing_status = PROCESSED;
        (*pix_al_ptr)(skel[assoc_i].intermed - skel[skel_i].intermed,
                     xy, skel[assoc_i].round_i);

        /* if this association is "small" and "constrained"  */
        if ((ABS (pixel_aligned.value) < xy->con_size)
                && (attribute & HQ3_CON_MASK))
        {
            /* collapse it to zero */
            skel[assoc_i].adjusted = skel[skel_i].adjusted;
        }

        /*  else if the association rounds to zero  */
        else if (pixel_aligned.value == 0)
        {
            /* set the association to of one pixel */
            /* if assoc is right of skel, add      */
            if (skel[assoc_i].intermed >= skel[skel_i].intermed)
            {
                skel[assoc_i].adjusted =
                            skel[skel_i].adjusted
                                      + (xy->p_pixel>>xy->bin_places);

                skel[assoc_i].pixel_grid_line =
                            skel[skel_i].pixel_grid_line + 1;
            }
            /* else, is left of skel, so subtract */
            else
            {
                skel[assoc_i].adjusted =
                            skel[skel_i].adjusted
                                      - (xy->p_pixel>>xy->bin_places);

                skel[assoc_i].pixel_grid_line =
                            skel[skel_i].pixel_grid_line - 1;
            }
        }
        /*  else use value returned from pixel_align () */
        else
        {
                skel[assoc_i].adjusted =
                    skel[skel_i].adjusted + pixel_aligned.value;
                skel[assoc_i].pixel_grid_line =
                    skel[skel_i].pixel_grid_line + pixel_aligned.grid_line;
        }
    }

    DBG2("       hq3_assoc_proc: end   assoc skel [%ld] = %ld\n",
                            assoc_i,
                            skel[assoc_i].adjusted);
}





/**********************************************************************/
/*                                                                    */
/*      interp_assoc_proc ()                                          */
/*              makes interpolated adjustments to specified skeletal  */
/*              points based on parent skeletals (previously pos-     */
/*              itioned by the tree process).                         */
/*                                                                    */
/**********************************************************************/


MLOCAL BOOLEAN
interp_assoc_proc (skel_ptr, skel)
    SKEL               *skel_ptr;
    adjusted_skel_type *skel;
{
    COUNTER i,j,k;
    LONG    numerator, denominator;

    k = 0;

    /* for each set of interpolated associations  */
    for (i = 0; i < skel_ptr->num_intrp; i++)
    {

        /*  calculate a scale factor, and  */

        numerator =   (LONG)(skel[skel_ptr->intrp2_skel_i[i]].adjusted -
                             skel[skel_ptr->intrp1_skel_i[i]].adjusted);
        denominator = (LONG)(skel[skel_ptr->intrp2_skel_i[i]].intermed -
                             skel[skel_ptr->intrp1_skel_i[i]].intermed);
        /* if denominator is not zero  */
        if (denominator)
        {

            /*  for each interp assoc in this set, scale it */
            for (j = 0; j < skel_ptr->num_intrp_assoc[i]; j++)
            {
                if (k >= skel_ptr->tot_num_intrp_assoc) return FALSE;
                skel[skel_ptr->intrp_assoc_skel_i[k]].adjusted =
                    (WORD)
                    (((LONG)(skel[skel_ptr->intrp_assoc_skel_i[k]].intermed -
                              skel[skel_ptr->intrp1_skel_i[i]].intermed)
                    * numerator) / denominator)
                    + skel[skel_ptr->intrp1_skel_i[i]].adjusted;
                k++;
            }
        }
        /*  else we can not divide by zero  */
        else
        {

            /*  for each interp assoc in this set, set equal to base.  */
            for (j = 0; j < skel_ptr->num_intrp_assoc[i]; j++)
            {
                if (k >= skel_ptr->tot_num_intrp_assoc) return FALSE;

                skel[skel_ptr->intrp_assoc_skel_i[k]].adjusted =
                              skel[skel_ptr->intrp1_skel_i[i]].adjusted;
                k++;
            }
        }
    }
    return TRUE;
}


/**********************************************************************/
/*                                                                    */
/*      skel_proc (option)                                            */
/*              sets up the required control parameters for the tree  */
/*              and interp assoc processes based on option (X orY).   */
/*                                                                    */
/**********************************************************************/


GLOBAL BOOLEAN
skel_proc (skel_ptr, glob_dim_ptr, xy, skel, root_adjust)
    SKEL               *skel_ptr;
    dimension_type     *glob_dim_ptr;
    COORD_DATA         *xy;
    adjusted_skel_type *skel;
    WORD                root_adjust;  /* adjustment to x-values to
                                       * align left ref
                                       */
{
    UWORD   sk_indx;           /*  skeletal point index      */
    UWORD   as_indx;           /*  associated point index    */
    COUNTER i, j;
    WORD num_assoc;
    WORD adjustment;   /* 11-29-90 tbh */

    /* efficiency */
    UBYTE skel_i;
    WORD  *adj_skel;
    UBYTE *ps_skel;    /* processing status */
    WORD *grid;       /* grid line */
    UBYTE  par_i;      /* parent index */
    WORD  adj_par;     /* adjusted parent value */

  /* standard dimensions */

    WORD  stan_dim_i;     /*  x or y standard dim index */
    WORD  curr_stan_value;
    WORD  curr_stan_attrib;
    WORD  pix;              /* pixel size in design units */
    UBYTE stan_dim_option; /* 11-29-90 tbh */

    DBG("\n\n\n-----------------------skel_proc()------------------\n");

    stan_dim_i = (UWORD)skel_ptr->stan_dim_i;

/*---------------tree_proc ()-----------------*/


  /* set pointer to pixel alignment function NOT using standard dims  */

    pix_al_ptr = pixel_align;
    stan_dim_option = STAN_DIM_OFF;  /* 11-29-90 tbh */

  /*  If we need to process with standard dimensions because pixel size
   *  size is greater than font wide cutin limit
   */

    pix = (xy->p_pixel >> xy->bin_places);
    if (pix >= face.stan_dim_lim)
    {

        curr_stan_value  = glob_dim_ptr->value[stan_dim_i];
        curr_stan_attrib = glob_dim_ptr->attrib[stan_dim_i];
 
       DBG1("Using standard dimenstions,  stan_dim_i = %ld\n", stan_dim_i);
        DBG1("    curr_stan_value  = %ld\n", curr_stan_value);
        DBG1("    curr_stan_attrib = %x\n", curr_stan_attrib);

      /*  if we need to calculate the aligned standard dimension */

        if ( xy->stand_value != curr_stan_value)
        {
            xy->stand_value = curr_stan_value; /* so we don't recalculate
                                                * for next character.
                                                */
            DBG1("    Calculating st dims.   curr_stan_value = %ld\n",
                                                curr_stan_value);
            for (i=0; i < 4; i++)
            {
                pixel_align(curr_stan_value, xy, i);

              /* if this dimension is "small" and "constrained"  */
                if ((pixel_aligned.value < xy->con_size)
                        && (curr_stan_attrib & HQ2_CON_MASK))
                {
                  /* collapse it to zero  */
                    xy->st_value[i]  = 0;
                    xy->st_pixels[i] = 0;
                }
                /* else if dimension rounds to zero and is NOT supposed to */
                else if ( (pixel_aligned.value == 0)
                        && !(curr_stan_attrib & DIM_RTZ_MASK) )
                {
                  /* set the aligned value to that of one pixel */
                    xy->st_value[i]  = pix;
                    xy->st_pixels[i] = 1;
                }
                /* else use value returned from pixel_align ()  */
                else
                {
                    xy->st_value[i]  = pixel_aligned.value;
                    xy->st_pixels[i] = pixel_aligned.grid_line;
                }


                DBG3("    %ld:  value %ld    grid %ld\n", i, xy->st_value[i],
                                                          xy->st_pixels[i]);

           /* vvvvvv START OF NEW CODE FROM TBH vvvvvvvv */

                xy->st_variation[i] = xy->st_value[i] - (curr_stan_value
                   + (WORD)(xy->round[i] >> xy->bin_places) - (pix >> 1));

            }

            xy->st_variation[2] = 0;
            xy->st_variation[3] = 0;

           /* ^^^^^^ END OF NEW CODE FROM TBH ^^^^^^^ */

        }

        /*  set pointer to pixel alignment function
                                           which uses standard dims  */
        pix_al_ptr = sd_pixel_align;
        stan_dim_option = STAN_DIM_ON;  /* 11-29-90 tbh */

    }






    as_indx = 0;

    /* for each skeletal point in the tree   */
    for (sk_indx = 0; sk_indx < skel_ptr->num_nodes; sk_indx++)
    {
        skel_i = skel_ptr->tree_skel_i[sk_indx];
        adj_skel = &skel [skel_i].adjusted;
        ps_skel  = &skel [skel_i].processing_status;
        grid = &skel [skel_i].pixel_grid_line;

        DBG1("\n\n\n\nskel_i =%ld\n", skel_i);

        num_assoc = skel_ptr->tree_attrib[sk_indx] >> 5;
        DBG1("number of associates = %ld\n", num_assoc);
        DBG2("\ntree: processing skel [%ld] = %ld\n",
                         skel_i, skel [ skel_i ].adjusted);
 
        /* if assoc equals skel, we're at a root point; therefore, */
        if (as_indx == sk_indx)
        {
            DBG("      as a root skel\n");
            as_indx++;                /* increment assoc index */

            /* pixel align the root skeletal point */
            pixel_align (*adj_skel + root_adjust, xy, R_TWO_I);
            *adj_skel = pixel_aligned.value;
            *grid     = pixel_aligned.grid_line;
            *ps_skel  = PROCESSED;
        }
        else
        {
            par_i = skel [skel_i].parent;
            adj_par = skel [par_i].adjusted;

            if   /* skel has yet to be processed and should be aligned  */
            ( 
                (     (*ps_skel == NOT_PROC)
                             &&
                      (skel_ptr->tree_attrib[sk_indx] & ALIGN_MASK)
                )
                    ||
            /* or skel is not yet aligned, not terminal, but processed  */
                (  (*grid == NOT_ALIGNED) && (*ps_skel == PROCESSED)
                              &&
                     (num_assoc != 0 ))
            )
            {
                DBG("      as an aligned assoc skel\n");
                /* then, first, pixel align the skel point  */
                (*pix_al_ptr)(*adj_skel - adj_par, xy, skel [skel_i].round_i);

                *adj_skel = adj_par + pixel_aligned.value;
                *grid = skel [par_i].pixel_grid_line
                                                 + pixel_aligned.grid_line;
                *ps_skel = PROCESSED;
            }
            else if  /*  skel is non_aligned, incorporate rounding */
            (*grid == NOT_ALIGNED)
            {
                DBG ("      as a non-aligned assoc skel\n");

            /* vvvvv  START OF NEW CODE vvvvvv */

                adjustment = (WORD)((xy->round[ skel[skel_i].round_i ]
                            - (LONG)xy->p_half_pix) >> xy->bin_places);
 
                /* if this assoc is within half pix of standard dim,
                   bias it to help diags */

                 if (stan_dim_option &&
                 (ABS(skel[skel_i].original - skel[par_i].original)
                                     - curr_stan_value) < (pix>>1))
                 {
                    adjustment += xy->st_variation[ skel[skel_i].round_i ];
                 }

            /* ^^^^^^  END OF NEW CODE ^^^^^^ */

                /* if non_aligned is to right, then add rounding bias  */
                if ( skel [skel_i ].intermed >= skel [par_i].intermed )
                {
                    *adj_skel += adjustment;  /* 11-19-90 tbh */
                }
                /*  else, non-aligned to left, so subtract rounding bias */
                else
                {
                    *adj_skel -= adjustment;  /* 11-19-90 tbh */
                }
            }
        }

        DBG1("      after grid alignment:    .adjusted = %ld\n", *adj_skel);
        DBG1("                               .pixel_grid_line = %ld\n",*grid);
        DBG1("                               .proc_status = %ld\n", *ps_skel);
        DBG1("                               .parent = %ld\n", par_i);
        DBG1("                               .round_i = %ld\n",
                                                 skel [skel_i].round_i);

        /*      and, finally . . . .                                    */
        /*      for each of this skel's assocs, pass on the adjustment  */
        for (j = 0; j < num_assoc; j++)
        {
            DBG1("       passing adjustment to assoc [%ld]\n",j);
            hq3_assoc_proc
                         (
                             skel_ptr->tree_skel_i[as_indx],
                             skel_i,
                             skel_ptr->tree_attrib[as_indx],
                             skel, xy
                         );
            as_indx++;
        }


    }   /* for each skeletal point in tree */

    if (skel_ptr->num_intrp)
        if(!interp_assoc_proc (skel_ptr, skel)) return FALSE;

    return TRUE;
}
