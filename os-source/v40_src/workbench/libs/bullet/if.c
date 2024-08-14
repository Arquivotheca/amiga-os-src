/*
**	$Id: if.c,v 7.0 91/03/19 18:36:12 kodiak Exp $
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
**
**	$Log:	if.c,v $
 * Revision 7.0  91/03/19  18:36:12  kodiak
 * after Amigaizing memory, lists, and adding UniCode.  Before cache work.
 * 
 * Revision 6.0  91/03/18  15:26:34  kodiak
 * folded in Bullet 1.30
 * 
 * Revision 3.0  90/11/09  17:09:34  kodiak
 * Kodiak's Alpha 1
 * 
*/
/* if.c */
/* Copyright (C) Agfa Compugraphic, 1989, 1990. All rights reserved. */

/*  07-Jul-90  awr  switch pixel size data for quadrant rotation
    25-Sep-90  jfd  Uncommented call to bold() at end of routine
    05-Oct-90  awr  added root adjust parameter to skel_proc() for
                    connecting characters
   28-Jan-91  jfd  Replaced "//" comments.
 */

#include <exec/types.h>
#include <exec/nodes.h>
#include "debug.h"
#include "profile.h"
#include "port.h"
#include "cgif.h"
#include "if_type.h"
#include "adj_skel.h"
#include "segments.h"

EXTERN BOOLEAN skeletal_init();
EXTERN BOOLEAN yclass_proc();
EXTERN BOOLEAN skel_proc();
EXTERN VOID    italic();

EXTERN VOID    bold();    /* 09-25-90 jfd */

EXTERN FACE     face;
EXTERN IF_CHAR  c;
EXTERN IF_DATA  d;
EXTERN adjusted_skel_type  x_skel[];
EXTERN adjusted_skel_type  y_skel[];

EXTERN pixel_align();
EXTERN struct {
                  WORD value;
                  WORD grid_line;
              } pixel_aligned;

/*-------------------*/
/*  intel_char       */
/*-------------------*/
GLOBAL UWORD
intel_char(making_bold)
    BOOLEAN making_bold;
{
    DBG("\n\nintel_char()\n");


#ifdef PROFILE
if(prof_val < PROF_skeletal_init)
#endif
    if(!skeletal_init ()) return ERR_skeletal_init;

  /* process y vectors */

#ifdef PROFILE
if(prof_val < PROF_yclass)
#endif
    yclass_proc ();
#ifdef PROFILE
if(prof_val < PROF_yskel)
#endif
    if(!skel_proc (&c.yskel, &face.glob_y_dim, d.py, &y_skel[0], (WORD)0))
        return ERR_y_skel_proc;

  /* process x vectors */

    DBG1("    c.loc_ital_ang = %ld\n", c.loc_ital_ang);
    if(c.loc_ital_ang)                        /* process italic  */
        italic();

    pixel_align (c.metric->escape_box.ll.x, d.px, R_TWO_I);

#ifdef PROFILE
if(prof_val < PROF_xskel)
#endif
    if(!skel_proc (&c.xskel, &face.glob_x_dim, d.px, &x_skel[0],
          pixel_aligned.value - c.metric->escape_box.ll.x)) /* root_adjust */
        return ERR_x_skel_proc;

/*?K?*/    if(making_bold)
/*?K?*/        bold();

    return SUCCESS;
}
