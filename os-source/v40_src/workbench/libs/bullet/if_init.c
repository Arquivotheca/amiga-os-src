/*
**	$Id: if_init.c,v 7.0 91/03/19 18:36:13 kodiak Exp $
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
**
**	$Log:	if_init.c,v $
 * Revision 7.0  91/03/19  18:36:13  kodiak
 * after Amigaizing memory, lists, and adding UniCode.  Before cache work.
 * 
 * Revision 6.0  91/03/18  15:26:43  kodiak
 * folded in Bullet 1.30
 * 
 * Revision 3.0  90/11/09  17:09:41  kodiak
 * Kodiak's Alpha 1
 * 
*/
/* if_init.c */
/* Copyright (C) Agfa Compugraphic, 1989. All rights reserved. */
/*
 * HISTORY
 *
 * 28-Jan-91  jfd  Due to change in if_type.h ("baseline_offset" is now a
 *                 WORD), in routine if_data(), cast value stored in
 *                 "c.baseline_offset" as  *(WORD*) instead of *(UWORD*). 
 */

#include <exec/types.h>
#include <exec/nodes.h>
#include "debug.h"
#include "port.h"
#include "cgif.h"
#include "segments.h"
#include "ix.h"
#include "if_type.h"


typedef struct
{
    UWORD loop;
    UWORD coord_index;   /* coordinate index within loop */
    UWORD x;
    UWORD y;
    UBYTE xsk0, xsk1, ysk0, ysk1;
} DUP_POINTS;

#define MAX_DUP_POINTS 48
GLOBAL DUP_POINTS dup_points[MAX_DUP_POINTS];


/*-----------------------------*/

GLOBAL
FACE       face;


GLOBAL IF_CHAR   c;
GLOBAL IF_DATA   d;
GLOBAL BOOLEAN   char_is_if;




/*---------------*/
/*  next_loop    */
/*---------------*/
GLOBAL LOOP*
next_loop()
{
    BYTE *p;

    p = c.next_contour_loop;

    c.lp.npnt   = *(WORD*)p;   p += 2;
    c.lp.nauxpt = *(WORD*)p;   p += 2;
    c.lp.x      = (WORD*)p;    p += 2*c.lp.npnt;
    c.lp.y      = (WORD*)p;    p += 2*c.lp.npnt;
    c.lp.xa     = p;           p += c.lp.nauxpt;
    c.lp.ya     = p;           p += c.lp.nauxpt;

    c.next_contour_loop = p;

    return &c.lp;
}





/*---------------*/
/*  first_loop   */
/*---------------*/
GLOBAL VOID
first_loop()
{
    c.next_contour_loop = c.contour_data;
}






/*---------------*/
/*  glob_dims    */
/*---------------*/
MLOCAL BYTE*
glob_dims(d,p)
    dimension_type *d;
    BYTE *p;
{
    UWORD uw;

    uw = *(UWORD *)p;
    d->num_dim = (UBYTE)uw;
        p += 2;         /* step to array of global dimension values */
    d->value = (UWORD *)p;
        p += uw<<1;     /* step to array of global dimension flags  */
    d->attrib = (UBYTE *)p;
        p += uw;        /* step to Global dimension data            */
    if(((UWORD)p&1) != 0) p++;

#ifdef DEBUG
    DBG("\n\nGlobal Dimensions\n");
    DBG1("    number of dimensions  %ld\n",d->num_dim);
    for(uw = 0; uw < d->num_dim; uw++)
        DBG2("    %ld    %x\n", d->value[uw], d->attrib[uw]);

#endif

    return p;
}


/*---------------*/
/*  if_init_glob */
/*---------------*/
GLOBAL BOOLEAN
if_init_glob(bucket)
    BUCKET  *bucket;
{
  WORD   temp_glob_ital_ang;
  UWORD  uw;
  BYTE  *p;
#ifdef DEBUG
  WORD   i;
  DISPLAY  *display;
  FACE_ATT *attribute;
#endif

  /* raster parameter */

    if (bucket->prast)
    {
        p = bucket->prast;
        face.orThreshold = *(UWORD *)(p);
        temp_glob_ital_ang = *(WORD *)(p+2);
        DBG("Raster parameter\n");
        DBG1("  orThreshhold        %ld\n",face.orThreshold);
        DBG1("  temp_glob_ital_ang  %ld\n",temp_glob_ital_ang);
    }
    else
    {
        face.orThreshold = 0;
        temp_glob_ital_ang = 0;
    }




  /* display header */

    if (!bucket->pdisplay)
        return FALSE;
#ifdef DEBUG
    else {
        display = bucket->pdisplay;
        DBG("Display header segment\n");
        DBG1("  NCHAR             %ld\n",display->NCHAR);
        DBG1("  leftReference     %ld\n",display->leftReference);
        DBG1("  baselinePosition  %ld\n",display->baselinePosition);
        DBG1("  minimumPointSize  %ld\n",display->minimumPointSize);
        DBG1("  maximumPointSize  %ld\n",display->maximumPointSize);
        DBG1("  minimumSetSize    %ld\n",display->minimumSetSize);
        DBG1("  maximumSetSize    %ld\n",display->maximumSetSize);
        DBG1("  masterPointSize   %ld\n",display->masterPointSize);
        DBG1("  scanDirection     %ld\n",display->scanDirection);
        DBG1("  italicAngle       %ld\n",display->italicAngle);
        DBG1("  xHeight           %ld\n",display->xHeight);
        DBG1("  scanResolutionY   %ld\n",display->scanResolutionY);
        DBG1("  scanResolutionX   %ld\n",display->scanResolutionX);
        DBG1("  outputEnable      %ld\n",display->outputEnable);
    }
#endif



  /* attribute header */

    if (!bucket->pattribute)
	return FALSE;
#ifdef DEBUG
    else {
        attribute = bucket->pattribute;
        DBG("Face attribute segment\n");
        DBG1("  languageType     %ld\n", attribute->languageType);
        DBG1("  fontUsage        %ld\n", attribute->fontUsage);
        DBG1("  isFixedPitch     %ld\n", attribute->isFixedPitch);
        DBG1("  escapeUnit       %ld\n", attribute->escapeUnit);
        DBG1("  scaleFactor      %ld\n", attribute->scaleFactor);
        DBG1("  leftReference    %ld\n", attribute->leftReference);
        DBG1("  baselinePosition %ld\n", attribute->baselinePosition);
        DBG1("  windowTop        %ld\n", attribute->windowTop);
        DBG1("  windowBottom     %ld\n", attribute->windowBottom);
        DBG1("  ascender         %ld\n", attribute->ascender);
        DBG1("  descender        %ld\n", attribute->descender);
        DBG1("  capHeight        %ld\n", attribute->capHeight);
        DBG1("  xHeight          %ld\n", attribute->xHeight);
        DBG1("  lcAccenHeight    %ld\n", attribute->lcAccenHeight);
        DBG1("  ucAccentHeight   %ld\n", attribute->ucAccentHeight);
        DBG1("  charPica         %ld\n", attribute->charPica);
        DBG1("  leftAlign        %ld\n", attribute->leftAlign);
        DBG1("  rightAlign       %ld\n", attribute->rightAlign);
        DBG1("  uscoreDepth      %ld\n", attribute->uscoreDepth);
        DBG1("  uscoreThickness  %ld\n", attribute->uscoreThickness);
        DBG1("  windowLeft       %ld\n", attribute->windowLeft);
        DBG1("  windowRight      %ld\n", attribute->windowRight);
        DBG1("  spaceBand        %ld\n", attribute->spaceBand);
    }
#endif




/*
 *  Locate Global Intellifont Data
 */
  if (bucket->pgif && bucket->gifct)
  {
    face.if_flag = TRUE;
  }
  else
  {
    face.if_flag = FALSE;
    if ( face.orThreshold == 0 ) face.orThreshold = 210;
  }




    if (bucket->pgif && bucket->gifct)
    {
       DBG("Global Intellifont Data\n");
       p = bucket->pgif;

      /*  Y Class Data  */

        uw = face.num_ylines = *(UWORD *)p;
            p += 2;
        face.ylines = (UWORD *)p;
            p += uw<<1;      /* step to global Y-class defin data */
        uw = *(UWORD *)p;
        face.glob_yclass.num_yclass_def = (UBYTE)uw;
            p += 2;          /* step to high Y-line index array   */
        face.glob_yclass.yline_high_i = (UBYTE *)p;
            p += uw;         /* step to low Y-line index array    */
        face.glob_yclass.yline_low_i = (UBYTE *)p;
            p += uw;         /* step to Global Dimension Data     */

#ifdef DEBUG
        DBG1("  number of ylines %ld\n", face.num_ylines);
        for (i = 0; i < face.num_ylines; i++)
            DBG1("    %ld\n", face.ylines[i]);

        DBG1("    number of yclass definitions  %ld\n",
                                           face.glob_yclass.num_yclass_def);
        for (i = 0; i < face.glob_yclass.num_yclass_def; i++)
            DBG2("    %ld  %ld\n", face.glob_yclass.yline_high_i[i],
                                face.glob_yclass.yline_low_i[i]);
#endif


      /*  Global Y Dimension Data  */

        p = glob_dims(&face.glob_y_dim, p);

      /*  Global X Dimension Data  */

        p = glob_dims(&face.glob_x_dim, p);

      /*  orThreshold value  */

        if ( face.orThreshold == 0 )
        {
            if ( *(UWORD *)p == 0 )
                face.orThreshold = 210;
            else
                face.orThreshold = *(UWORD *)p;
        }
            p += 2;         /* step to std dim cut-in limit               */
        face.stan_dim_lim = *(UWORD *)p;
            p += 2;         /* step to screen face substitution width fact*/
        face.subst_wdth_fac = *(UWORD *)p;
            p += 2;         /* step to screen face substitution cut-in    */
        face.subst_cutin = *(UWORD *)p;
            /* p += 2;         step to detail face descriptor             */
        face.glob_ital_ang = temp_glob_ital_ang;
        if ( (face.stan_dim_lim == 0) || (face.subst_cutin == 0) )
        {
           face.stan_dim_lim = MAX_WORD;
           face.subst_cutin  = MAX_WORD;
        }
    }


    return TRUE;
}



/*===============================================================*/
/*            I n i t i a l i z e    C h a r a c t e r           */


#ifdef DEBUG
/*----------------*/
/* print_contour  */
/*----------------*/
MLOCAL BOOLEAN
print_contour()
{
    CTREE *tnode;
    WORD i;

    DBG("Contour Tree\n");
    DBG2("    num loops, flag  %ld  %ld\n",c.num_loops, c.contour_flag);

    tnode = c.ctree;
    for(i=0; i< c.num_loops; i++, tnode++)
    {
        DBG4("    offset/pol/child/sib   %ld  %ld  %ld  %ld\n",
                  tnode->xyoff, tnode->polarity,
                  tnode->child, tnode->sibling);
    }
    return TRUE;
}


/*----------------*/
/* print_metric   */
/*----------------*/
MLOCAL VOID
print_metric()
{
    DBG("    metric data\n");
    DBG4("    bounding box %ld,%ld   %ld,%ld\n", c.metric->bound_box.ll.x,
                                             c.metric->bound_box.ll.y,
                                             c.metric->bound_box.ur.x,
                                             c.metric->bound_box.ur.y);
    DBG4("    escape box   %ld,%ld   %ld,%ld\n", c.metric->escape_box.ll.x,
                                             c.metric->escape_box.ll.y,
                                             c.metric->escape_box.ur.x,
                                             c.metric->escape_box.ur.y);
    DBG2("    half/center line     %ld,%ld\n", c.metric->halfline,
                                             c.metric->centerline);
}



/*----------------*/
/* print_xy       */
/*----------------*/
MLOCAL VOID
print_xy()
{
    WORD *xptr, *yptr;
    BYTE *xauxptr, *yauxptr;
    WORD i,j;
    LOOP *lp;

    first_loop();

    for (i=0; i< c.num_loops; i++)
    {
        DBG("    --------next loop-------\n");
        lp = next_loop();

        DBG2("    npt = %ld   nauxpt = %ld\n",
                   lp->npnt, lp->nauxpt);
        xptr = lp->x;
        yptr = lp->y;
        for(j=0; j < lp->npnt; j++)
        {
            DBG4("        %x  %ld      %x  %ld\n",
                 ((UWORD)*xptr) >> 14, *xptr&0x3fff,
                 ((UWORD)*yptr) >> 14, *yptr&0x3fff);

            xptr++;
            yptr++;
        }
        DBG("\n    auxilary points\n");
        xauxptr = lp->xa;
        yauxptr = lp->ya;
        for(j=0; j < lp->nauxpt; j++)
            DBG2("        %ld  %ld\n",*xauxptr++, *yauxptr++);


    }
}



/*----------------*/
/* print_skel     */
/*----------------*/
MLOCAL VOID
print_skel(sp)
    SKEL *sp;
{
    WORD i,j;
    UBYTE *skloop;
    UWORD *skcntr;

    /* tree_attrib bits */
    WORD   attrib;
    WORD   num_assoc;
    WORD   constr_dim;
    WORD   grid_align;
    WORD   rtz;
    WORD   rtype;


    DBG3("    num_skel / num_trees / num_nodes %ld  %ld  %ld\n",
                         sp->num_skel, sp->num_trees, sp->num_nodes);


  /* build loop order list of skeletal points */

    skloop = sp->num_skel_loop;
    skcntr = sp->skel_to_contr;

    for ( i=0; i<c.num_loops; i++, skloop++)
    {
        DBG2("        loop %ld has %ld skel points\n\n\n",i,*skloop);
        DBG("                      skel_to_contr\n");
        for ( j=0; j<*skloop; j++)
            DBG2("            %ld            %ld\n", j, *skcntr++);
    }

    DBG("\n        tree_attrib        tree_skel_i\n");
    DBG("        N C G Z R\n");
    for(i=0; i < sp->num_nodes ; i++)
    {
        attrib = *(sp->tree_attrib+i);
        num_assoc = attrib >> 5;
        constr_dim = (attrib>>4) & 1;
        grid_align = (attrib>>3) & 1;
        rtz        = (attrib>>2) & 1;
        rtype      = attrib & 3;

        DBG6("        %ld %ld %ld %ld %ld            %ld\n",
           num_assoc,constr_dim,grid_align,rtz,rtype,*(sp->tree_skel_i+i));
    }

    DBG1("    num_intrp %ld\n", sp->num_intrp);
    for(i=0; i < sp->num_intrp ; i++)
        DBG3("        %ld %ld %ld\n", *(sp->intrp1_skel_i+i),
                                   *(sp->intrp2_skel_i+i),
                                   *(sp->num_intrp_assoc+i));
    DBG1("    tot_num_intrp_assoc %ld\n", sp->tot_num_intrp_assoc);
    for(i=0; i < sp->tot_num_intrp_assoc ; i++)
        DBG1("        %ld\n",*(sp->intrp_assoc_skel_i+i));
}



/*----------------*/
/* print_if       */
/*----------------*/
MLOCAL VOID
print_if()
{
    WORD i;

  /* y skel data */

    DBG("y skeletal data\n");
    print_skel(&c.yskel);

  /* baseline */

    DBG1("baseline_offset %ld\n",c.baseline_offset);

  /* y classes */

    DBG("y classes\n");
    DBG1("    num_yclass_ass %ld\n",c.num_yclass_ass);
    DBG("        num_root_p_ass\n");
    for(i=0; i< c.num_yclass_ass; i++)
        DBG1("        %ld\n", *(c.num_root_p_ass+i));
    DBG("        root_skel_i\n");
    for(i=0; i< c.yskel.num_trees; i++)
        DBG1("        %ld\n", *(c.root_skel_i+i));
    DBG("        yclass_i\n");
    for(i=0; i< c.num_yclass_ass; i++)
        DBG1("        %ld\n", *(c.yclass_i+i));

    DBG1("    num_yclass_def  (high/low) %ld\n",c.num_yclass_def);
    for(i=0; i<c.num_yclass_def; i++)
        DBG2("        %ld %ld\n",*(c.yline_high_i+i), *(c.yline_low_i+i));

    DBG1("    num_ylines %ld\n",c.num_ylines);
    for(i=0; i< c.num_ylines; i++)
        DBG1("        %ld\n",*(c.ylines + i));

  /* x data */

    DBG("x skeletal data\n");
    print_skel(&c.xskel);

  /* local italic angle */

    DBG2("italic angle/flag  %ld %ld\n",c.loc_ital_ang, c.italic_flag);

  /* standard dimension data */

    DBG2("standard dims %ld %ld\n",c.xskel.stan_dim_i,c.yskel.stan_dim_i);
}
#endif




/*----------------*/
/* skel_data      */
/*----------------*/
MLOCAL BYTE*
skel_data(pp, sp)
    BYTE *pp;
    SKEL *sp;
{
    BYTE *p;

    p = pp;

    sp->num_skel    = *(p++);
    sp->num_trees   = *(p++);
    sp->num_nodes   = *(p++);
    sp->tree_skel_i = p;    p += sp->num_nodes;
    sp->tree_attrib = p;    p += sp->num_nodes;


  /* Interpolated association data */

    sp->num_intrp           = *(p++);
    sp->intrp1_skel_i       = p;   p += sp->num_intrp;
    sp->intrp2_skel_i       = p;   p += sp->num_intrp;
    sp->num_intrp_assoc     = p;   p += sp->num_intrp;
    sp->tot_num_intrp_assoc = *(p++);
    sp->intrp_assoc_skel_i  = p;   p += sp->tot_num_intrp_assoc;

    if((p - pp) & 1) p++;
    return p;
}


/*----------------*/
/* if_data        */
/*----------------*/
MLOCAL UWORD
if_data(len, ifoff)
    UWORD len;
    UWORD ifoff;
{
    BYTE    *ifp;
    BYTE    *p;
    WORD     i,j;
    BYTE    *char_end;
    UBYTE   *xskloop, *yskloop;
    UWORD   *xskcntr, *yskcntr;
    LOOP    *lp;         /* points to character loop structures */
    WORD    *yptr;       /* points to y coordinate data */
/*B*/    WORD    *xptr;       /* points to x coordinate data */
/*B*/    DUP_POINTS *dp;
/*B*/    UWORD    dup_ct;
/*B*/    WORD     last_x, last_y;


    if(ifoff == MAX_UWORD)
    {
        char_is_if       = FALSE;
        c.yskel.num_skel = 0;
        c.xskel.num_skel = 0;
        c.loc_ital_ang   = 0;
    }
    else
    {
        char_is_if = TRUE;

        ifp = p = c.char_buf + ifoff;

      /* y skel data */

        p = skel_data(p, &c.yskel);

      /* baseline */

        c.baseline_offset = *(WORD*)p;  p += 2;

      /* y classes */

        c.num_yclass_ass = *(p++);
        c.num_root_p_ass = p;   p += c.num_yclass_ass;
        c.root_skel_i    = p;   p += c.yskel.num_trees;
        c.yclass_i       = p;   p += c.num_yclass_ass;

        /* local y class definitions */

        c.num_yclass_def = *(p++);
        c.yline_high_i   = p;   p += c.num_yclass_def;
        c.yline_low_i    = p;   p += c.num_yclass_def;

        if((p - ifp) & 1) p++;      /* pad to even byte */


        c.num_ylines = *(UWORD*)p;    p += 2;
        c.ylines     =  (UWORD*)p;    p += 2*c.num_ylines;

      /* x data */

        p = skel_data(p, &c.xskel);

      /* local italic angle */

        c.loc_ital_ang = *(WORD*)p;   p += 2;
        c.italic_flag  = *(UWORD*)p;  p += 2;;

      /* standard dimension data */

        c.xskel.stan_dim_i = *(UBYTE*)p++;
        c.yskel.stan_dim_i = *(UBYTE*)p;
    }


  /*  Build loop order list of skeletal points. This is done
   *  once here instead of twice in skel_data to save time- we
   *  only go through the coordinate data once.
   */

    char_end = c.char_buf + len;  /* unused char buffer after data */

    c.yskel.num_skel_loop = yskloop = (UBYTE*)char_end;
        char_end += c.num_loops;
    c.xskel.num_skel_loop = xskloop = (UBYTE*)char_end;
        char_end += c.num_loops;
    c.yskel.skel_to_contr = yskcntr = (UWORD *)char_end;
        char_end += 2*c.yskel.num_skel;
    c.xskel.skel_to_contr = xskcntr = (UWORD *)char_end;
        char_end += 2*c.xskel.num_skel;
    if ( ( char_end - c.char_buf ) > CHARBUFSIZE)
        return ERR_ov_char_buf;

/*B*/    dp = dup_points;
/*B*/    dup_ct = 0;

    first_loop();
    for ( i=0; i<c.num_loops; i++, yskloop++, xskloop++)
    {
        lp = next_loop();

/*B*/        xptr = lp->x;
        yptr = lp->y;

/*B*/        last_x = *(xptr + lp->npnt - 1) & 0x3fff;
/*B*/        last_y = *(yptr + lp->npnt - 1) & 0x3fff;

        *yskloop = 0;
        *xskloop = 0;
        for ( j=0; j<lp->npnt; j++, yptr++ )
        {
/*B*/            if((last_x == (*xptr&0x3fff)) && (last_y == (*yptr&0x3fff)))
/*B*/            {
/*B*/                DBG4("    dup pt: (%ld, %ld)  loop = %ld   index = %ld\n",
/*B*/                               last_x, last_y, i, j);
/*B*/                dp->loop  = i;
/*B*/                dp->coord_index = j;
/*B*/                dup_ct++;
/*B*/                if(dup_ct == MAX_DUP_POINTS) return ERRtoo_many_dup_skel;
/*B*/                dp++;
/*B*/            }
/*B*/            last_x = *xptr & 0x3fff;
/*B*/            last_y = *yptr & 0x3fff;
/*B*/            xptr++;

            if (*yptr & 0x4000)  /* test for x skel pt */
            {
                (*xskloop)++;
                *xskcntr++ = j;
            }
            if (*yptr & 0x8000)  /* test for y skel pt */
            {
                (*yskloop)++;
                *yskcntr++ = j;
            }
        }      
    }
/*B*/    dp->loop = -1;


#ifdef DEBUG
    print_if();
#endif


  return SUCCESS;
}





/*---------------*/
/*  if_init_char */
/*---------------*/
GLOBAL UWORD
if_init_char(char_buf)
    BYTE *char_buf;
{
    UWORD len, metoff, ifoff, conoff, xyoff;
    UWORD *wp;
    BYTE  *p;
    WORD   i;

    c.char_buf = char_buf;     /* character data buffer pointer */

  /*  Offsets to sections of character */

    wp     = (UWORD*)char_buf;
    len    = *wp++;   /* total length     */
    metoff = *wp++;   /* metric data      */
    ifoff  = *wp++;   /* Intellifont data */
    conoff = *wp++;   /* Contour tree     */
    xyoff  = *wp;   /* XY data          */


  /*-------------Contour Data---------------------------------*/

    if ( conoff == MAX_UWORD )
    {
        DBG("No contour tree\n");
        return ERR_no_contour;
    }

    p = (BYTE*)(char_buf+conoff);
    c.num_loops        = *(WORD*)p;   p += 2;
    i = c.contour_flag = *(WORD*)p;   p += 2;
        c.PreBrokenContrs = i & 0x0001;
        c.HqFormat = ((i & 0x0006) >> 1) + 1;
        c.ConnectingChar = (i & 0x0100) >> 8;
    if (c.HqFormat != 3)
    {
        DBG("----------not HQ3\n");

        return ERR_not_hq3;
    }
    c.ctree        = (CTREE*)p;

#ifdef DEBUG
    print_contour();
#endif


  /*-------------Metric Data----------------------------------*/

    c.metric = (METRIC*)(char_buf+metoff);

#ifdef DEBUG
    print_metric();
#endif

  /*-------------XY Data--------------------------------------*/

    c.contour_data = (BYTE*)(char_buf + xyoff);

#ifdef DEBUG
    print_xy();
#endif

  /*-------------Intellifont Data-----------------------------*/

     return if_data(len, ifoff);
}

