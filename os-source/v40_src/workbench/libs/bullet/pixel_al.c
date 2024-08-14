/* pixel_al.c */
/* Copyright (C) Agfa Compugraphic, 1990. All rights reserved. */
/*   11-29-90  tbh   Bug fix for missing pixels in pixel_align() */

#include  "debug.h"
#include  "port.h"
#include  "if_type.h"
#if 0
#include  "adj_skel.h"
#include  "segments.h"


#define ALIGN_MASK      0x08    /*  alignment attribute mask: bit 3   */
#define CON_RTZ_MASK    0x14    /*  const && non-RTZ dim mask for HQ3 */
#define R_TYPE_MASK     0x03    /*  value to mask for "R" types       */
#define DIM_MASK        0x04    /*  dimension attribute mask: bit 2   */
#define HQ3_CON_MASK    0x10    /*  constrained dimension mask (HQ3)  */
#define DIM_RTZ_MASK    0x01    /*  dimension RTZ mask                */
#define HQ2_CON_MASK    0x02    /*  constrained dimension mask (HQ2)  */


#if 0
#define NO_STANDARD_DIM 0xffff  /*  id for none .standard_value       */
#define DIM_NOT_PROC    -2      /*  status: dimension not processed   */
#define HQ2             2       /*  HQ2 data format                   */
#define HQ3             3       /*  HQ3 data format                   */
#endif


EXTERN FACE face;

MLOCAL WORD (*pix_al_ptr)();   /* ptr to pixel_align() function      */
GLOBAL struct {
                  WORD value;
                  WORD grid_line;
              } pixel_aligned;

#endif

EXTERN struct {
                  WORD value;
                  WORD grid_line;
              } pixel_aligned;

/*----------------------*/
/*   pixel_align        */
/*----------------------*/
GLOBAL VOID
pixel_align(val, xy, round_i)
    WORD val;
    COORD_DATA *xy;
    WORD  round_i;
{
    WORD value;
    WORD grid;

    value = ABS(val);

    grid = (WORD)((((LONG)value <<xy->bin_places) + xy->round[round_i])
                          / xy->p_pixel);

    if (val < 0)        /* 11-29-90 tbh */
       grid = - grid;   /* 11-29-90 tbh */

    pixel_aligned.value =
                (WORD)(((LONG)grid * (LONG)xy->p_pixel) >> xy->bin_places);
    pixel_aligned.grid_line = grid;

    DBG4("pixel_align: val = %ld  round_i = %ld  new val = %ld   grid = %ld\n",
               val, round_i, pixel_aligned.value, pixel_aligned.grid_line);
}
