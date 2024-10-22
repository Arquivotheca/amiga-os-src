/*
**	$Id: maker.c,v 8.0 91/03/24 12:17:15 kodiak Exp $
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
**
**	$Log:	maker.c,v $
 * Revision 8.0  91/03/24  12:17:15  kodiak
 * Phase 2 Beta 1 (38.2)
 * 
 * Revision 7.0  91/03/19  18:36:45  kodiak
 * after Amigaizing memory, lists, and adding UniCode.  Before cache work.
 * 
 * Revision 6.0  91/03/18  15:27:26  kodiak
 * folded in Bullet 1.30
 * 
 * Revision 5.1  91/03/18  09:01:27  kodiak
 * eliminate old style symbol table lookup
 * 
 * Revision 3.0  90/11/09  17:11:07  kodiak
 * Kodiak's Alpha 1
 * 
*/
/* maker */
/* Copyright (C) Agfa Compugraphic, 1989, 1990. All rights reserved. */
/*---------------------------------------------------------------------
     02-26-90   jfd   Changed "MAKfontSize()" to implement screen font
     05-02-90   jfd   Removed calls to routines
                         GETcharWidth()
                         GETprohibitFlags()
                         MAKEscreenFontBold()
                         MAKEscreenFontNonBold()
                         MAKEscreenFontItalic()
                         MAKEscreenFontNonItalic()
                         CHANGEfontID()
                      and replaced them with in-line code
     05-03-90   jfd   If explicitly requesting a screen font, variable
                      "face.glob_ital_tan" never gets set to "NO_TANGENT";
                      Added pointer "widthPtr" to shorten long expressions.
     05-04-90   jfd   Save/restore user's font context
     05-05-90   awr   Added landscape. New function rot90(), changed
                      des2wrkbm() to rotate coords. Changed bitmpa_size()
                      to swap bmwidth and bmdepth and compute d.max_x
                      and d.max.y.
     06-25-90   jfd   Added conditional compile for screen fonts
     07-01-90   awr   Reorganized screen font code. Won't try to sub screen
                      fonts is screen fonts not installed or if char is
                      not typeface sensitive.
     07-28-90   awr   Added check in set_kern() for existence of kerning
                      segments
     08-03-90   awr   Bold calculations now use ABS(round[0]).
     08-05-90   awr   Prohibit flags checked in MAKfontSize()
     08-05-90   awr   Doubled x_bold and y_bold in MAKfontSize(). Bold
                      characters do not grow in all directions but may
                      grow only to the right or only down.
     08-20-90   awr   In bitmap_size(), computation of increases due to
                      bold were not correct for rotate or shear because
                      we were using tx() and ty(). Added bold change to
                      design unit bound box which goes through des2wrkbm()
                      to correct.
     08-29-90   det   In "MAKfontSize()", when calling get_face_and_char(),
                      pass "local_fc.font_id" as first argument.
     09-01-90   awr   Corrected the recently added conditional in
                      bitmap_size() from just checking bold to also check
                      spotsize.
     09-03-90   awr   In bitmap_size() changed computation of bitmap width
                      and depth in pixels. There was a double round in the
                      old method that was masked by the bold value (no longer
                      there- see 08-20-90 above).
     09-05-90   awr   In make_char_part(), if calculation of "size" > 32K,
                      result is a negative number because "d.bmwidth" and
                      "d.bmdepth" are typed as WORDs, not LONGs. Therfore,
                      cast "d.bmwidth" and "d.bmdepth" as ULONG.
     09-07-90   jfd   In make_char_part(), moved calculation of "making_bold"
                      to before "if (char_is_if)"
     09-11-90   jfd   In bitmap_size(), change pad added to bitmap width and
                      depth from 4 pixels to 5 pixels because grid alignment
                      was pushing the character extremes outside of the 
                      bounding box.
     09-11-90   bjg   Added quadratic outline support.
     09-21-90   jfd   Changed BITMAP to CGBITMAP due to conflict in "windows.h"
     09-28-90   jfd   In make_char_part(), when calling bitmap_size(),
                      forgot to pass last argument "making_bold";
                      Moved assignment for "making_bold" to beginning of
                      make_char_part()
     26-Oct-90  jfd   Conditionally compiling declarations for routines
                      linear() and quadra().
     11-22-90   awr   Fixed bug in merge, rewrote bitblt portion based
                      on bg's if2.2 version.
     11-29-90   jfd   At start of MAKfontSize(), check if font id is 0
                      and return an error if it is.
                      Later, if comp_pix() returns an error, set current
                      local font_id to 0 so that when processing next
                      character, comp_pix() will be called again.
     12-05-90   dET   In MAKchar_width(), when testing for a compound
                      character, instead of testing for "num_parts > 1",
                      test for "is_compound". This bug resulted in
                      an incorrect width being returned for a compound
                      character consisting of only one part.
     12-05-90   bg    In MAKfontSize(), make_char_part() and MAKbitMap(),
                      check for format<2 for bitmap output and format==2
                      for outline output.
                      If !CGBITMAP, add GLOBAL declaration for "or_on".
     01-02-91   bg    In make_char_part() and MAKbitMap(), corrected
                      compound character bug for outline output.
     01-28-91   jfd   In MAKbitMap() and set_kern(), when testing for a
                      compound character, use "is_compound" instead of
                      "num_parts>1".

                tnc   In outline_metrics(), if compound character, store
                      "h_esc" as escapement instead of calculating it 
                      from bounding box.

-----------------------------------------------------------------------*/


#include <exec/types.h>
#include <exec/nodes.h>
#include "debug.h"
#include "profile.h"
#include "port.h"
#include "cgconfig.h"
#include "imath.h"
#include "cgif.h"
#include "if_type.h"
#include "segments.h"
#include "ix.h"
#include "fmath.h"

#define xpix       d.x.pixel_size
#define ypix       d.y.pixel_size
#define half_xpix  d.x.half_pixel
#define half_ypix  d.y.half_pixel


/* fc.c        */
EXTERN BOOLEAN FCis_equal();
EXTERN VOID    FCcopy_fc();
/* fm.c        */
EXTERN UWORD   FMrd_char();
/* if_init.c   */
EXTERN BOOLEAN if_init_glob();
EXTERN UWORD   if_init_char();
/*  if_init.c  */
EXTERN VOID  first_loop();
EXTERN LOOP* next_loop();
/* if.c        */
EXTERN UWORD   intel_char();
/* raster.c    */
EXTERN BOOLEAN raster();
#ifdef LINEAR
EXTERN BOOLEAN linear();
#endif
#if QUADRA
EXTERN BOOLEAN quadra();
#endif
/* fill.c      */
EXTERN VOID    fill();
/* margin.c    */
EXTERN VOID    margin();
/* metrics.c   */
EXTERN VOID    metrics();
/* comp_pix.c  */
EXTERN BOOLEAN comp_pix();
/* ix.c */
EXTERN UWORD   IXget_fnt_index();
EXTERN UWORD   IXsetup_chr_def();

#ifdef DEBUG
EXTERN VOID    pr_tran();           /* pr_tran.c */
#endif
GLOBAL UWORD (*arc_function)();
#if CGBITMAP | LINEAR
EXTERN UWORD arc_bitmap();
#endif
#if QUADRA
EXTERN UWORD arc_outline();
#endif

#if SCREEN_FONTS
EXTERN UWORD   screen_fnt_ct;              /* 06-30-90   awr */
#endif


/*  if_init.c  */
EXTERN FACE    face;
EXTERN IF_CHAR c;
EXTERN IF_DATA d;
EXTERN BOOLEAN char_is_if;
/* raster.c */

#if CGBITMAP
EXTERN BOOLEAN or_on;
#else
GLOBAL BOOLEAN or_on;  /* 12-06-90 jfd */
#endif

/* comp_pix.c */
EXTERN MATRIX  tran;
/* ix.c */
EXTERN SS_ENTRY  *symbol_set;     /* current symbol set            */



MLOCAL WORD    ttx;
GLOBAL WORD    tty;

/* current font parameters */

MLOCAL FONTCONTEXT cur_loc_fc; /* The last font context used by comp_pix() */

GLOBAL LONG        if_init_face;  /* last typeface run through if_init.
                                   * The only reason it's GLOBAL is so
                                   * IXinit() can zero it.
                                   */

/* Compound Character */

GLOBAL UBYTE   *cc_buf;    /* compound character buffer */
GLOBAL UWORD    cc_size;   /* size of comp. char buffer */


/*............Data from compound character segment.........................*/

MLOCAL CHR_DEF chr_def[MAX_CC_PARTS];
MLOCAL OUTLINE_PARAMS outline_params;

/* gid rid of - make same for regular and compound */
GLOBAL WORD  h_esc;                  /* Horizontal escapement of character */
GLOBAL WORD  v_esc;                  /* Vertical       "      "     "      */
GLOBAL WORD  num_parts;              /* Number of parts of character       */
GLOBAL BOOLEAN is_compound;

/*  Compound character metrics */

MLOCAL XY   origin;       /* Origin of compound character in design space */
MLOCAL BOX  des_bound;    /* Bounding box in design units                 */
MLOCAL WORDPOINT omargin;




/*----------------------*/
/*    clear_mem         */
/*----------------------*/
GLOBAL VOID
clear_mem(p, size)
    UBYTE *p;
    ULONG  size;
{
    ULONG i;

    for(i=0; i<size; i++)
        *p++ = 0;
}




/*----------------------*/
/*   des2wrkbm          */
/*----------------------*/
GLOBAL VOID
des2wrkbm(x, y, newx, newy)
  WORD  x, y;
  WORD *newx, *newy;
{
    WORD xx, yy;

    if(!d.quadrant)    /* arbitrary rotation */
    {
        *newx = ifmul(x, tran[0]) + ifmul(y, tran[2]) + ttx;
        *newy = ifmul(x, tran[1]) + ifmul(y, tran[3]) + tty;
    }
    else
    {
        if(d.quadrant == ROT0)
        {
            xx = x;
            yy = y;
        }
        else if(d.quadrant == ROT90)
        {
            xx = d.x.max_des - y;
            yy = x;
        }
        else if(d.quadrant == ROT180)
        {
            xx = d.x.max_des - x;
            yy = d.y.max_des - y;
        }
        else if(d.quadrant == ROT270)
        {
            xx = y;
            yy = d.y.max_des - x;
        }
        *newx = tx(xx) + d.x.margin;
        *newy = ty(yy) + d.y.margin;
    }
}

#if CGBITMAP

/*----------------------*/
/*    bitmap_size       */
/*----------------------*/
/*  Compute character bitmap size from the bounding box in the
 *  work space. Allow for larger bitmap due to pseudo bold or
 *  pseudo italic.
 *
 *  Sets the following values in the IF_DATA structure:
 *
 *    d.tbound.ll.x        Bounding box in power of 2 space
 *    d.tbound.ll.y
 *    d.tbound.ur.x
 *    d.tbound.ur.y
 *
 *    d.x.margin          Translation from power of 2 space to
 *    d.y.margin          working bitmap space
 *
 *    d.bmwidth            Bitmap width in bytes
 *    d.bmdepth            Bitmap depth in rasters
 *    d.max_x              Working bitmap width
 *    d.max_y                "       "    depth
 *
 *    ttx                  Overall translation for matrix transform,
 *    tty                  to get to working bitmap space.
 *
 */
MLOCAL UWORD
bitmap_size(bound_box, max_bm, actual_bm, making_bold)
    BOX     *bound_box;
    UWORD    max_bm;      /*  Maximum size for the IFBITMAP structure =
                           *  header plus actual character bitmap.
                           */
    UWORD   *actual_bm;   /*  Actual bitmap memory needed */
    BOOLEAN  making_bold;
{
    WORD xmin, ymin, xmax, ymax;
    WORDPOINT *vp;
    WORDPOINT v[4];
    UWORD i;

    WORD  abs_x_bold, abs_y_bold;

    LONG  tan;
    WORD  dtop, dbot;

    LONG    num_bytes;


    DBG("bitmap_size()\n");

  /*  Transform bounding box from design space to working space
   *  d.tbound.
   */

    d.tbound.ll.x = bound_box->ll.x;
    d.tbound.ll.y = bound_box->ll.y;
    d.tbound.ur.x = bound_box->ur.x;
    d.tbound.ur.y = bound_box->ur.y;

    if(d.quadrant)     /* If special quadrant rotation.. */
    {
      /*  Account for Intellifont italic processing. Skew bounding
       *  box left by local angle and then skew right by global
       *  angle.
       */

        if(c.loc_ital_ang && face.glob_ital_ang && !c.italic_flag)
        {
            tan = face.glob_ital_ang - c.loc_ital_ang;
            dtop = (WORD)((tan * (LONG)d.tbound.ur.y) >> 15);
            dbot = (WORD)((tan * (LONG)d.tbound.ll.y) >> 15);
            DBG1("    tan %.4f\n", (double)tan/32768.0);
            DBG2("    dtop %ld  dbot %ld\n", dtop, dbot);

            if(tan>0)
            {
                d.tbound.ur.x += dtop;
                d.tbound.ll.x += dbot;
            }
            else
            {
                d.tbound.ur.x += dbot;
                d.tbound.ll.x += dtop;
            }
        }
    }

  /* If current FONTCONTEXT is using spot size control or psuedo bold... */

    if(making_bold || cur_loc_fc.xspot != F_ONE
                   || cur_loc_fc.yspot != F_ONE)
    {
      /*  Take care of Bold
       *
       *  Bold round values times 2. We do times 2 because we don't
       *  know in what direction the boldening may take place.
       */

        abs_x_bold = 2 * (WORD)(LABS(d.x.round[0]) >> d.x.bin_places);
        abs_y_bold = 2 * (WORD)(LABS(d.y.round[0]) >> d.y.bin_places);

        d.tbound.ur.x += abs_x_bold;
        d.tbound.ur.y += abs_y_bold;
        d.tbound.ll.x -= abs_x_bold;
        d.tbound.ll.y -= abs_y_bold;

        DBG2("    abs_x_bold, abs_y_bold   %ld  %ld\n", abs_x_bold, abs_y_bold);
    }

  /* Set margin translation to 0 */

    ttx = F2I(tran[4]);
    tty = F2I(tran[5]);
    d.x.margin = 0;
    d.y.margin = 0;

    vp = v;
    des2wrkbm(d.tbound.ll.x, d.tbound.ll.y, &vp->x, &vp->y);  vp++;
    des2wrkbm(d.tbound.ll.x, d.tbound.ur.y, &vp->x, &vp->y);  vp++;
    des2wrkbm(d.tbound.ur.x, d.tbound.ll.y, &vp->x, &vp->y);  vp++;
    des2wrkbm(d.tbound.ur.x, d.tbound.ur.y, &vp->x, &vp->y);

    xmin = ymin = MAX_DESIGN;
    xmax = ymax = 0;
    for(i=0, vp = v; i<4; i++, vp++)
    {
        if(vp->x < xmin) xmin = vp->x;
        if(vp->x > xmax) xmax = vp->x;
        if(vp->y < ymin) ymin = vp->y;
        if(vp->y > ymax) ymax = vp->y;
    }
    d.tbound.ll.x = xmin;
    d.tbound.ll.y = ymin;
    d.tbound.ur.x = xmax;
    d.tbound.ur.y = ymax;


    DBG("     Working Space\n");
    DBG4("    bounding box (%ld,%ld)   (%ld,%ld)\n", d.tbound.ll.x,
                                                 d.tbound.ll.y,
                                                 d.tbound.ur.x,
                                                 d.tbound.ur.y);


  /*---------------------------------------------------------------
   *  Compute translation values
   *
   *  These values get added to coordinates in the working space to
   *  shift the bounding box to the lower left corner of the working
   *  space.
   */

    d.x.margin = 4 * xpix - (d.tbound.ll.x & d.x.grid_align);
    d.y.margin = 4 * ypix - (d.tbound.ll.y & d.y.grid_align);

    ttx += d.x.margin;   /* tty = F2I(tran[4] + I2F(d.x.margin));  */
    tty += d.y.margin;   /* tty = F2I(tran[5] + I2F(d.y.margin));  */



    DBG2("    margin  %ld %ld\n", d.x.margin, d.y.margin);

  /*  Bitmap width & depth in pixels. We add 7 or 8 (conversion truncates)
   *  pixels to both the width and the depth because grid alignment
   *  may push the character extremes outside of the bounding box.
   */

#if 0
    d.bmwidth =  8 + ((d.tbound.ur.x - d.tbound.ll.x) >> d.x.grid_shift);
    d.bmdepth =  8 + ((d.tbound.ur.y - d.tbound.ll.y) >> d.y.grid_shift);
#endif
    d.bmwidth = 5 + ((d.x.margin + d.tbound.ur.x + xpix-1) >> d.x.grid_shift);
    d.bmdepth = 5 + ((d.y.margin + d.tbound.ur.y + ypix-1) >> d.y.grid_shift);

    DBG2("bm width(pixels) depth %ld  %ld\n", d.bmwidth, d.bmdepth);

  /*  Convert pixel width to width in bytes. Application has
   *  specified whether bit map should be a multiple of BYTEs
   *  or a multiple of WORDs or a multiple of LONGs by setting
   *  bit_map_width to 8, 16, or 32.
   */
  /*  bit_map_width made a constant "32" for Amiga */

    d.bmwidth = ((d.bmwidth + 31) & 0xffe0) >> 3;
    DBG1("bm byte width(BYTEs)  %ld\n", d.bmwidth);
    DBG("\n");



    num_bytes = (LONG)d.bmwidth * (LONG)d.bmdepth + sizeof(IFBITMAP) - 4;

    if(or_on)
    {
      /*  This is a little sloppy.  Since we've included the header already
       *  in num_bytes, we shouldn't multiply it by 2--- but it's only a
       *  a dozen BYTEs or so and if cc_size is that close to the limit,
       *  then the application is going to fail anyway.
       */

        if( (num_bytes<<1) > cc_size)
        {
            DBG("    or_on bitmap > cc_size\n");
            return ERR_bm_gt_oron;
        }
    }
    else
        if(num_bytes > max_bm)
        {
            DBG("    bitmap > bitmap size\n");
            return ERR_bm_too_big;
        }

    *actual_bm = (UWORD)num_bytes;

    return SUCCESS;
}

/*----------------------*/
/*   merge              */
/*----------------------*/
/*  Merge the bitmap in the source buffer into the destination buffer
 *  so that the two origins coincide.
 *  The origins are in working bitmap spaces.
 */
MLOCAL VOID
merge(src, sx, sy, dst, dx, dy)
    IFBITMAP *src;  /* Source      buffer                                */
    WORD      sx;   /*             origin in source working bitmap space */
    WORD      sy;
    IFBITMAP *dst;  /* Destination buffer                                */
    WORD      dx;   /*             origin in dest working bitmap space   */
    WORD      dy;
{
    WORD   vx, vy;    /* vector from origin to upper left of black box   */
    WORD   ddx, ddy;  /* pixel address of blackbox in destination bitmap */
    UBYTE *ps, *pd;   /* pointers used to merge the bitmaps.             */
    WORD   sstart;    /* BYTE offset to starting BYTE of src blackbox    */
    WORD   dstart;    /* BYTE offset to starting BYTE of dst blackbox    */
    WORD   sw;        /* BYTE width of src blackbox                      */
    WORD   dw;        /* BYTE width of dst blackbox                      */
    UWORD  i,j;
    UWORD  mid_src_ct;
    UWORD  sbit0, dbit0;
    UWORD  sbit1, dbit1;
    WORD   n, nb;

    DBG4("\nmerge(sx,sy = %d, %d    dx, dy = %d, %d)\n", sx, sy, dx, dy);

  /*  Compute vector from source origin to upper left corner of
   *  black box.
   */
  
    vx = (src->left_indent << d.x.grid_shift) - sx;
    vy = ((src->depth - src->top_indent) << d.y.grid_shift) - sy;
    DBG2("    vx, vy  %d, %d\n", vx, vy);

  /*  Compute pixel address of blackbox in destination space. Address
   *  is upper left corner of pixel with origin at upper left of bitmap.
   */

    ddx = (dx + vx + half_xpix) >> d.x.grid_shift;
    ddy = dst->depth - ((dy + vy + half_ypix) >> d.y.grid_shift);
    DBG2("    ddx, ddy  %d, %d\n", ddx, ddy);

  /*  Compute overhangs. Sbit0 and dbit0 are the number of white bytes
   *  to the left of the black box in the left most bytes that intersect
   *  the black box.
   *  Sbit1 and dbit1 are the number of black bits in the right most
   *  bytes that intersect the black box.
   */
    sbit0 = src->left_indent & 7;
    dbit0 = ddx & 7;
    sbit1 = (sbit0 + src->black_width) & 7;  if(!sbit1) sbit1 = 8;
    dbit1 = (dbit0 + src->black_width) & 7;  if(!dbit1) dbit1 = 8;
    DBG2("    sbit0 = %d    dbit0 = %d\n", sbit0, dbit0);
    DBG2("    sbit1 = %d    dbit1 = %d\n", sbit1, dbit1);

  /*  Compute starting byte offset along a raster and the number
   *  of bytes that covers the black width in a raster. Note that
   *  sw and dw can vary from each other by as much as 1 depending
   *  on overhangs.
   */

    sstart = src->left_indent >> 3;
    dstart = ddx >> 3;
    sw    = ((src->left_indent + src->black_width + 7) >> 3) - sstart;
    dw    = ((ddx + src->black_width + 7) >> 3) - dstart;

  /*  Set pointers */

    ps = src->bm + (src->top_indent * src->width) + sstart;
    pd = dst->bm + (ddy * dst->width) + dstart;

  /*  Special case if no shifting is required */

    if(sbit0 == dbit0)
    {
        for(i=0; i<src->black_depth; i++)
        {
            for(j=0; j<sw; j++)
                *pd++ |= *ps++;

            ps += src->width - sw;
            pd += dst->width - dw;
        }
        return;
    }

  /*  Shifting case. Transfer each raster in (up to) three steps:
   *    1. If src overhangs on left, then shift src and or into dst.
   *    2. Transfer all "middle" src bytes that overlap two dst
   *       bytes (if any).
   *    3. If src overhangs on right, then shift src and or into dst.
   */

  /* number of source bytes to transfer in the middle loop, step 2 */

    mid_src_ct = sw;  /* Assume no overhangs, all src bytes are in step 2 */
    if(sbit0 > dbit0) /* Source overhangs on left  */
        mid_src_ct--;
    if(sbit1 < dbit1) /* Source overhangs on right */
        mid_src_ct--;

    DBG1("    mid_src_ct = %u\n", mid_src_ct);

  /*  Shifting values. Each source overlaps two dst bytes.
   *  Shift src byte >> n  before oring into left  dst byte
   *  Shift src byte << nb before oring into right dst byte
   */

    n  = dbit0 - sbit0;   /* calculate n. Assume no left overhang        */
    if(n<0)               /* If n < 0, then left overhnag and we really  */
        n = 8 + n;        /* calculated -nb, so convert to n.            */

    nb = 8 - n;
    DBG2("    n = %u   nb = %u\n", n, nb);

  /*  For each raster in the black box shift and transfer the data from
   *  the source to the destination
   */

    for(i=0; i<src->black_depth; i++)
    {

      /*  Step 1. Take care of the case that the source overhangs the
       *  destination on the left. In this case, this first source byte only
       *  contributes to one destination byte.
       */
        if(sbit0 > dbit0)
            *pd |= *ps++ << nb;

      /*  Step 2. Transfer the middle data. For each source byte, put
       *  a piece in each of the two destination bytes that it overlaps.
       */
        for(j=0; j<mid_src_ct; j++)
        {
            *pd++ |= *ps   >> n;
            *pd   |= *ps++ << nb;
        }

              /*  Rule for pointers after each of these three steps:
               *      ps now points to the next src byte to be processed.
               *      pd now points to the last dest byte processed.
               */

      /*  Step 3. Take care of the case that the source overhangs the
       *  destination on the right. In this case, this last source byte only
       *  contributes to one destination byte.
       */

        if(sbit1 < dbit1)
            *pd |= *ps++ >> n;

      /* On to the next raster */

        ps += src->width - sw;
        pd += 1 + dst->width - dw;
    }
}
#endif

#if OUTLINE
/*----------------------*/
/*    outline_metrics   */
/*----------------------*/
MLOCAL VOID
outline_metrics(outlineptr)
  IFOUTLINE *outlineptr;
{
  outlineptr->size = d.bmwidth;
  outlineptr->depth = 1;
  outlineptr->xscale = d.x.pixel_size;
  outlineptr->yscale = d.y.pixel_size;
  outlineptr->top = outline_params.bound.ur.y;
  outlineptr->right = outline_params.bound.ur.x;
  outlineptr->bottom = outline_params.bound.ll.y;
  outlineptr->left = outline_params.bound.ll.x;
  if (is_compound)  /* 01-28-91 tnc */
     outlineptr->escapement = h_esc;
  else
     outlineptr->escapement = c.metric->escape_box.ur.x - c.metric->escape_box.ll.x;
}

/*----------------------*/
/*    outline_size           */
/*----------------------*/
/*  Compute character outline size 
 *
 *  Sets the following values in the IF_DATA structure:
 *
 *    d.x.margin          Translation from power of 2 space to
 *    d.y.margin          working bitmap space
 *
 *    d.bmwidth            Bitmap width in bytes
 *    d.bmdepth            Bitmap depth in rasters
 *
 *    ttx                  Overall translation for matrix transform,
 *    tty                  to get to working bitmap space.
 *
 */
MLOCAL UWORD
outline_size(bound_box, max_bm, actual_bm)
    BOX   *bound_box;
    UWORD  max_bm;    /*  Maximum size for the IFBITMAP structure =
                       *  header plus actual character bitmap.
                       */
    UWORD *actual_bm; /*  Actual bitmap memory needed */
{
    UWORD i,j;

    WORD  status;

    LONG  num_bytes;
    LOOP *lp;
    CHR_DEF *cd;
    WORD ncoords;


    DBG("outline_size()\n");

    /*  translate leftreference baseline to 0,0 */
    ttx = F2I(tran[4]);
    tty = F2I(tran[5]);
    d.x.margin = 0;
    d.y.margin = 0;

    des2wrkbm(c.metric->escape_box.ll.x, c.metric->escape_box.ll.y, 
       &omargin.x, &omargin.y);

    /*   need vector from left reference/baseline to origin */
    d.x.margin = -omargin.x;
    d.y.margin = -omargin.y;

    ttx += d.x.margin;   /* tty = F2I(tran[4] + I2F(d.x.margin));  */
    tty += d.y.margin;   /* tty = F2I(tran[5] + I2F(d.y.margin));  */


  /*  KLUDGE:  set margins to 0 because Intellifont is on during rotate
   *  (so we get psuedo bold) and the interpolation of non-skels in
   *  nxt_pt() includes the margins in x_tran.rnd and y_tran.rnd.
   */

    if(!d.quadrant)
    {
        d.x.margin = 0;
        d.y.margin = 0;
    }

    /*  compute size of quadratic outline */
    num_bytes = HEADERSIZE + 4;  /* header size plus size and num_loops */
    outline_params.tot_loops = 0;
    outline_params.current_loop = 0;

    cd = chr_def;
    for(i=0; i<num_parts; i++)
    {

      if(status = FMrd_char(cd)) return status;

      first_loop();
      num_bytes += 10 * c.num_loops;
      outline_params.tot_loops += c.num_loops;
     
      /*  curve points require 2 segments and 4 points.  Since 1 segment and
          one point is already accounted for, need to add one segment and
          3 points or 13 bytes/curve point */
      ncoords = 13;
      if((cur_loc_fc.format&0xf) == 3)   /*  linear */
        ncoords = 130;


      for (j=0; j< c.num_loops; j++)
      {
          lp = next_loop();
          /*  for each point allow 1 byte for segment and 4 bytes for points
              for each curve point allow an additional ncoords bytes and
              add one byte extra per loop for bytealignment of segments */
          num_bytes += 5 * (lp->npnt+1) + ncoords * lp->nauxpt + 1;
      }
      cd++;

    }


    *actual_bm = (UWORD)num_bytes;
    d.bmwidth = (WORD)num_bytes - HEADERSIZE;
    d.bmdepth = 1;

    return SUCCESS;
}

#endif







/*----------------------*/
/*    des_bound_box     */
/*----------------------*/
/*  KLUDGE
 *  Compute the compound character bounding box in design units.
 *  Results go in des_bound structure.
 */

MLOCAL UWORD
des_bound_box()
{
    WORD  offx, offy;
    UWORD i;
    UWORD status;
    CHR_DEF *cd;

    DBG("\ndes_bound_box()----------------------------------\n");

  /* Read character data for first part */

    DBG("character part 0:\n");
    cd = chr_def;
    if(status = FMrd_char(cd)) return status;

  /*  Declare origin of first character part translated by first part's
   *  offset to be origin of compound character. First part stays fixed
   *  in design space.
   */
    offx = cd->offset.x;
    offy = cd->offset.y;

    origin.x = c.metric->escape_box.ll.x - offx;
    origin.y = c.metric->escape_box.ll.y - offy;
    DBG2("    origin.x, origin.y: (%ld,%ld)\n", origin.x, origin.y);

    DBG4("    bound box: (%ld,%ld) (%ld,%ld)\n", c.metric->bound_box.ll.x,
                                             c.metric->bound_box.ll.y,
                                             c.metric->bound_box.ur.x,
                                             c.metric->bound_box.ur.y);

    des_bound.ll.x = c.metric->bound_box.ll.x;
    des_bound.ll.y = c.metric->bound_box.ll.y;
    des_bound.ur.x = c.metric->bound_box.ur.x;
    des_bound.ur.y = c.metric->bound_box.ur.y;

    for(i=1; i<num_parts; i++)
    {
      /* Read character data for next part */

        DBG1("character part %ld:\n", i);
        cd++;
        if(status = FMrd_char(cd)) return status;

      /* Merge character part bounding box with total box */

        DBG4("    bound box: (%ld,%ld) (%ld,%ld)\n", c.metric->bound_box.ll.x,
                                                 c.metric->bound_box.ll.y,
                                                 c.metric->bound_box.ur.x,
                                                 c.metric->bound_box.ur.y);

        des_bound.ll.x = MIN(des_bound.ll.x,
                             cd->offset.x - offx + c.metric->bound_box.ll.x);
        des_bound.ll.y = MIN(des_bound.ll.y,
                             cd->offset.y - offy + c.metric->bound_box.ll.y);
        des_bound.ur.x = MAX(des_bound.ur.x,
                             cd->offset.x - offx + c.metric->bound_box.ur.x);
        des_bound.ur.y = MAX(des_bound.ur.y,
                             cd->offset.y - offy + c.metric->bound_box.ur.y);
    }
    DBG("des_bound_box()----------------------------------\n\n");
    return SUCCESS;
}


/*----------------------*/
/*  get_face_and_char   */
/*----------------------*/
/*  Set BUCKETs and chr_def  */
MLOCAL UWORD
get_face_and_char(font_id, chId, pp_bucket)
    LONG      font_id;
    UWORD     chId;
    BUCKET  **pp_bucket;
{
    UWORD     status;
    BUCKET   *bucket;

    if(status = IXget_fnt_index(font_id))                  return status;
    if(status = IXsetup_chr_def(chId, chr_def, &bucket))  return status;

  /*  If the library file has changed,
   *  initialize the global Intellifont data.
   */

    if(if_init_face != bucket->tfnum)
    {
        if(!if_init_glob(bucket))
        {
            if_init_face = 0;
            return ERR_if_init_glob;
        }
        if_init_face = bucket->tfnum;
    }

    *pp_bucket = bucket;
    return SUCCESS;
}

/*----------------------*/
/*    MAKfontSize       */
/*----------------------*/
/*  Returns number of bytes needed for cached bitmap and sets up the
 *  the character generation subsytem (the "maker") to produce a
 *  character bitmap.
 *
 *  In addition, checks if screen font substitution is necessary.
 *  If so, sets up BUCKETs for the appropriate screen font (serif
 *  or sans serif) using the character widths of the requested font.
 *  (03-01-90 JFD)
 *
 *  In addition, check if screen font must be emboldened or italicized.
 *  (03-13-90 jfd )
 *
 *  We Assume that most of the requests for character bitmaps have been
 *  handled successfully by the cache subsystem. Thus, when we get called
 *  the FONTCONTEXT has (we  hope) changed many times. We cannot assume
 *  that any of our private parameters are still valid.
 */
GLOBAL UWORD
MAKfontSize(fc, chId, size, making_bold)
    FONTCONTEXT  *fc;
    WORD          chId;
    UWORD        *size;
    BOOLEAN       making_bold;
{
    UWORD   status;
    UWORD   i;
    BUCKET *bucket;
    CHR_DEF *cd;
    CHARWIDTH   *widthPtr;                   /* 05-03-90   jfd */
    WORD         buck_bits;                  /* 06-30-90   awr */
    FONTCONTEXT  local_fc;

#define PROHIBIT_BOLD     0x0040
#define PROHIBIT_ITALIC   0x0080

    DBG1("MAKfontSize(%ld)\n", chId);

#ifdef PROFILE
if(prof_val >= PROF_MAKfontSize)
{
    *size = 100;
    return SUCCESS;
}
#endif

  /*  Map the symbol set code into a cg number */

    if((chId == 0x20) || (chId == 0xa0)) return ERR_fixed_space;

  /*  If font id is 0, return an error */
    
    if (!fc->font_id) return ERR_no_font; /* 11-29-90 jfd */

  /*  Load the font and set up to access the character */
 
    if(status = get_face_and_char(fc->font_id, chId, &bucket))
        return status;

    FCcopy_fc(fc, &local_fc);   /* Local copy of FONTCONTEXT */

  /* Bits indicate if base font is bold, serifed, and italic */

    buck_bits = bucket->bucket_num;

  /* Point to character width and prohibit flags */

    widthPtr = bucket->pwidth_seg + chr_def->index;

  /*  Don't psuedo-embolden this character if not allowed  */

    if (widthPtr->flags & PROHIBIT_BOLD)
    {
        local_fc.xbold = 0;
        local_fc.ybold = 0;
    }

  /*  Don't psuedo italicize this character if not allowed  */
 
    face.glob_ital_tan = NO_TANGENT;
    if(widthPtr->flags & PROHIBIT_ITALIC)
    {
        local_fc.shear.x      = 0;
        local_fc.shear.y      = 0x10000;
    }

/*================================================================*/
/*             G e n e r i c    S c r e e n    F o n t s          */
#if SCREEN_FONTS

      /* Check for screen font substitution
       *
       *  If pixel size exceeds screen face substitution cut-in limit,
       *  we want to substitute the appropriate screen font.
       *  Screen fonts only contain type face sensitive characters, so
       *  don't bother trying to substitute if we're building a LS or U.
       *  Also, don't substitue if we're already a screen font.
       *  First check that the screen fonts are installed. Init_font_index()
       *  in ix.c sets screen_fnt_ct to 2 if they are.
       */

  /*  If the pt/set size or output resolution changed, then
   *  recompute the pixel size.
   */

    if((local_fc.point_size != cur_loc_fc.point_size) ||
       (local_fc.set_size   != cur_loc_fc.set_size  ) ||
       (local_fc.xres       != cur_loc_fc.xres      ) ||  
       (local_fc.yres       != cur_loc_fc.yres      ))
    {
        if(!comp_pix(&local_fc, bucket)) /* 11-29-90 jfd */
        {
            cur_loc_fc.font_id = 0;  /* make invalid FONTCONTEXT */
            return ERR_comp_pix;
        }
        FCcopy_fc(&local_fc, &cur_loc_fc);
    }

    DBG2("face.subst_wdth_fac = %ld    face.subst_cutin = %ld\n",
                                   face.subst_wdth_fac, face.subst_cutin);

    if((screen_fnt_ct == 2)                 /* Screen fonts are installed  */
           &&
       (chr_def->bucknum == TF_SENSITIVE)   /* Only tfs have screen fonts  */
           &&
       (fc->font_id != FACE_S_SCR)          /* don't sub screen for screen */
           &&
       (fc->font_id != FACE_SS_SCR)
           &&
       (        d.x.orig_pixel > (WORD)face.subst_cutin
                                 ||
                d.y.orig_pixel > (WORD)face.subst_cutin     )

           &&
       face.subst_wdth_fac
           &&
       face.subst_cutin)
    {
      /* Select serif or sans-serif generic screen font */

        if (buck_bits & B_SANS_SERIF)
            local_fc.font_id = FACE_SS_SCR;
        else
            local_fc.font_id = FACE_S_SCR;

      /*  Psuedo-embolden this character if base font is bold
       *  and character is allowed to be emboldened.
       */

        if ((buck_bits & B_BOLD)
                           && !(BOOLEAN)(widthPtr->flags & PROHIBIT_BOLD))
        {
          /*  If application is using psuedo bold, then go with
           *  application's bold values.
           */
            if(!making_bold)
            {
                local_fc.xbold = 496;
                local_fc.ybold = 330;
            }
        }

      /*  Psuedo italicize this character if base font is italic
       *  and character is allowed to be italicized.
       */
 
        if((buck_bits & B_ITALIC)
                            && !(BOOLEAN)(widthPtr->flags & PROHIBIT_ITALIC))
        {
          /*  Calculate tangent of italic angle for later use
           *  and sign-extend if the value is negative.
           */

            face.glob_ital_tan = (face.glob_ital_ang && !c.italic_flag)
               ?    (FIXED)face.glob_ital_ang << 1
               :    (FIXED)c.loc_ital_ang << 1;
            if (face.glob_ital_tan >= 0x10000)
                face.glob_ital_tan |= 0xFFFF0000;
        }

      /*  Override screen character width with requested character's
       *  width unless application has already given an alternate width.
       */

        if (c.alt_width == 0)
            c.alt_width = (WORD)widthPtr->width;

      /*  Load the generic screen font and set up to access the character */

        if(status = get_face_and_char(local_fc.font_id, chId, &bucket))
            return status;
    }
#endif
/*             G e n e r i c    S c r e e n    F o n t s          */
/*================================================================*/

    if (!FCis_equal(&local_fc, &cur_loc_fc))
    {
        if(!comp_pix(&local_fc, bucket))  /* 11-29-90 jfd */
        {
            cur_loc_fc.font_id = 0;  /* make invalid FONTCONTEXT */
            return ERR_comp_pix;
        }
        FCcopy_fc(&local_fc, &cur_loc_fc);
    }


  /*------------------------------------------------------------------
   *  Compute (compound) character bounding box in design space.
   */
 
    if(status = des_bound_box()) return status;

    DBG("     Design space\n");
    DBG4("    bounding box (%ld,%ld)   (%ld,%ld)\n", des_bound.ll.x,
                                                 des_bound.ll.y,
                                                 des_bound.ur.x,
                                                 des_bound.ur.y);

  /*---------------------------------------------------------------
   *  Or on transitions flag
   */

    if(    (d.x.orig_pixel > face.orThreshold)
        || (d.y.orig_pixel > face.orThreshold))
        or_on = TRUE;
    else
        or_on = FALSE;

    DBG4("orThreshold %ld  orig pixel  %ld,%ld    or_on = %ld\n",
            face.orThreshold, d.x.orig_pixel, d.y.orig_pixel, or_on);

  /*-------------------------------------------------------
   *  Compute the character bitmap size and the translation
   *  from working space to working bitmap space.
   */

   if((local_fc.format&0xf) < 2 ) {  /* 12-05-90 bg */
#if CGBITMAP
       if(status = bitmap_size(&des_bound, 0xffff, size, making_bold))
        return status;              /* 0xffff = max IFBITMAP size */
#else
        return ERR_bitmap_not_installed;
#endif
   }
   else {
#if OUTLINE
      or_on = FALSE;   /*  do not or on transitions for outlines */
       if(status = outline_size(&des_bound, 0xffff, size))
        return status;
#else
      return ERR_outline_not_installed;
#endif
   }


  /*--------------------------------------------------------------
   *  Transform origin of each part to working bitmap space.
   */

    DBG("\nWorking bitmap space origins-------------------------------\n");

    cd = chr_def;
    for(i=0; i<num_parts; i++)
    {
        des2wrkbm(origin.x + cd->offset.x, origin.y + cd->offset.y,
                   &cd->bmorigin.x, &cd->bmorigin.y);
        DBG3("    part %ld: (%ld,%ld)\n\n\n", i, cd->bmorigin.x, cd->bmorigin.y);
        cd++;
    }

    return SUCCESS;
}


/*----------------------*/
/*   make_char_part     */
/*----------------------*/
MLOCAL UWORD
make_char_part(pn, bm, bold_buf1, bold_buf2)
    UWORD   pn;
    IFBITMAP *bm;
    UBYTE  *bold_buf1;
    UBYTE  *bold_buf2;
{
    ULONG  size;          /*  Size of bitmap in BYTEs  */
    UWORD  status;
    UBYTE *or_buffer;
    UWORD  junk;
    BOOLEAN making_bold;
#if OUTLINE
    CHR_DEF *cd;
#endif

    DBG1("make_char_part(%ld)\n", pn);

    making_bold = (bold_buf1 != (UBYTE*)0);

    if(num_parts > 1)  /* else character & bitmap size is already set */
    {
      /*  Load and initialize proper bucket, read in character data,
       *  and compute the bounding box in the work space.
       */
        if(status = FMrd_char(&chr_def[pn]))  return status;

        if(pn > 0  && ((cur_loc_fc.format&0xf) < 2 )) /* else use comp ch bm size */ /* 12-05-90 bg */
        {
#if CGBITMAP
          if(status = bitmap_size(&c.metric->bound_box, cc_size, &junk,
               making_bold))
              return status;
          bm->width = d.bmwidth;
          bm->depth = d.bmdepth;
#endif
        }
        else if(pn > 0 && ((cur_loc_fc.format&0xf) >= 2 )) { /* 12-05-90 bg */
#if OUTLINE
          cd = chr_def;
          ttx = F2I(tran[4]);
          tty = F2I(tran[5]);
          d.x.margin = 0;
          d.y.margin = 0;

          des2wrkbm(c.metric->escape_box.ll.x - cd[pn].offset.x, 
                    c.metric->escape_box.ll.y - cd[pn].offset.y, 
                    &omargin.x, &omargin.y);
          d.x.margin = -omargin.x;  /* 01-02-91 bg */
          d.y.margin = -omargin.y;  /* 01-02-91 bg */
          ttx += d.x.margin;   /* tty = F2I(tran[4] + I2F(d.x.margin));  */
          tty += d.y.margin;   /* tty = F2I(tran[5] + I2F(d.y.margin));  */
#endif
        }
    }

  /*------------------------------------------------------------
   *  Apply Intellifont rules to skeletal points
   */

    if(char_is_if)
        if(status = intel_char(making_bold))
            return status;

  /*------------------------------------------------------------
   *  Sort out which buffers we're using and clear them
   */

  if(pn == 0  || ((cur_loc_fc.format&0xf) < 2 )) /*do not clear buffer for
                                                    outline unless first part */
                                                 /* 01-02-91 bg */

      {


    size = (ULONG)d.bmwidth * (ULONG)d.bmdepth;
    or_buffer = (UBYTE*)0;

    if(making_bold)
    {
        DBG("    making bold\n");
        or_buffer = bold_buf1;
        or_on =FALSE;
    }
    else if(or_on)
    {
      /*  Use compound character buffer as the or_buffer for the first
       *  part of the character. For all other parts, use the end of the
       *  compound character buffer.
       */

        if(pn == 0)
            or_buffer = cc_buf;
        else
            or_buffer =  bm->bm + size;
    }

    clear_mem(bm->bm, size);
    if(or_buffer) clear_mem(or_buffer, size);
    if(bold_buf2) clear_mem(bold_buf2, size);

 }
  /*------------------------------------------------------------
   *  Make the trasition array
   */

    if((cur_loc_fc.format&0xf) < 2) {  /*  bitmap */  /* 12-05-90 bg */
#if CGBITMAP

    /*------------------------------------------------------------
     *  Make the trasition array
     */

#ifdef PROFILE
if(prof_val < PROF_raster)
#endif
      arc_function = arc_bitmap;
      raster(bm->bm, or_buffer, making_bold, bold_buf2);
#if 0
    if(trace_sw)
    {
        pr_tran(bm->bm, d.bmwidth, d.bmdepth);
        if(or_on || making_bold)
        {
          pr_tran(or_buffer, d.bmwidth, d.bmdepth);
        }
        if(making_bold)
        {
          pr_tran(bold_buf2, d.bmwidth, d.bmdepth);
        }
    }
#endif


    /*------------------------------------------------------------
     *  Fill the transition array to make the final bitmap
     */

#ifdef PROFILE
if(prof_val < PROF_fill)
#endif
      fill(bm->bm, bm->bm, d.bmwidth, d.bmdepth, or_buffer,
                                    making_bold, bold_buf2);
#ifdef PROFILE
if(prof_val < PROF_margin)
#endif
      margin(bm);

#if 0
    if(trace_sw)
    {
        pr_tran(bm->bm, d.bmwidth, d.bmdepth);
    }
#endif
#endif
  }  /*  end of if bitmap */
  else {  /* outline */
#if OUTLINE
      outline_params.current_part = pn;
      if((cur_loc_fc.format&0xf) == 3) {  /*  linear */
#if LINEAR
        arc_function = arc_bitmap;
        linear(bm->bm, or_buffer, making_bold, bold_buf2);
#else
        return ERR_linear_not_installed;
#endif
      }
      else {  /*  quadratic */
       if ((cur_loc_fc.format&0xf) == 2) {  /* quadratic */ /* 12-06-90 jfd */
#if QUADRA
        arc_function = arc_outline;
        quadra(bm->bm, or_buffer, making_bold, bold_buf2);
#else
        return ERR_quadratic_not_installed;
#endif
       }
      }
#endif
  }

    return SUCCESS;
}

/*----------------------*/
/*    MAKbitMap         */
/*----------------------*/
/*  This function, MAKbitMap(), is called from font.c's make_char().
 *  We'll make the required bitmap in bm.
 * 
 */

GLOBAL UWORD ENTRY
MAKbitMap(bm, bold_buf1, bold_buf2)
    IFBITMAP  *bm;            /* Final result goes here */
    UBYTE   *bold_buf1;      /* If this is not nul, we're doing bold */
    UBYTE   *bold_buf2;
{
    UWORD status;
    UWORD i;

  /*  Saved translation portions of compound character transform */

    IFBITMAP *temp_buf; /* 01-02-91 bg */
    WORD xmargin, ymargin;
    WORD ttx0, tty0;

  /* Origin of part in part's working bitmap space */

    WORD  xorg, yorg;
    CHR_DEF  *cd;

    DBG("\n\n\n\nMAKbitMap()\n");

#ifdef PROFILE
if(prof_val >= PROF_MAKbitMap) return SUCCESS;
#endif

  /*  Make first part of (possible compound) character in bm. */

    xmargin = d.x.margin;
    ymargin = d.y.margin;
    ttx0    = ttx;
    tty0    = tty;

    bm->width = d.bmwidth;
    bm->depth = d.bmdepth;

    if(status = make_char_part(0, bm, bold_buf1, bold_buf2))
        return status;

  /*  Make remaining parts of compound character in compound
   *  character buffer and or them into bm.
   */

    if((cur_loc_fc.format&0xf) < 2)   /*  bitmap */ /* 01-02-91 bg */
      temp_buf = (IFBITMAP *)cc_buf;
    else
      temp_buf = bm;
    cd = chr_def;

    for(i=1; i<num_parts; i++)
    {
      /*  Make compound character part i in compound character buffer */

        cd++;
        if(status = make_char_part(i, temp_buf, bold_buf1, bold_buf2)) /* 01-02-91 bg */
            return status;

      /*  Compute origin in bitmap */

        DBG("     Design unit\n");
        DBG4("    escape box (%ld,%ld)  (%ld,%ld)\n", c.metric->escape_box.ll.x,
                                                 c.metric->escape_box.ll.y,
                                                 c.metric->escape_box.ur.x,
                                                 c.metric->escape_box.ur.y);

        des2wrkbm(c.metric->escape_box.ll.x, c.metric->escape_box.ll.y,
                                &xorg, &yorg);
      /*  Or compound character buffer into application buffer bm->bm */

#if CGBITMAP
        if((cur_loc_fc.format&0xf) < 2)   /*  bitmap */  /* 12-05-90 bg */
           merge((IFBITMAP*)cc_buf, xorg, yorg,
                  bm, cd->bmorigin.x, cd->bmorigin.y);
#endif

    }

  /*  Compute the metrics */

    if((cur_loc_fc.format&0xf) < 2)   /*  bitmap */ /* 12-05-90 bg */
    {
#if CGBITMAP
    if(is_compound)  /* 01-28-81 jfd */
    {
#ifdef PROFILE
if(prof_val < PROF_margin)
#endif
         margin(bm);

      /*  Restore compound character transform */

        d.x.margin = xmargin;
        d.y.margin = ymargin;
        ttx        = ttx0;
        tty        = tty0;

#ifdef PROFILE
if(prof_val < PROF_metrics)
#endif
        metrics(bm, origin.x,
                    origin.y,
                    origin.x + h_esc,
                    origin.y + v_esc,     &des_bound);
    }
    else
#ifdef PROFILE
if(prof_val < PROF_metrics)
#endif
        metrics(bm, origin.x,
                    origin.y,
                    c.metric->escape_box.ur.x,
                    c.metric->escape_box.ur.y,  &des_bound);
#endif
   }
   else {
#if OUTLINE
      outline_metrics((IFOUTLINE *)bm);
#endif
   }

    return SUCCESS;
}




/*-------------------*/
/*  MAKchar_width    */
/*-------------------*/
GLOBAL ULONG
MAKchar_width(chId)
    UWORD chId;
{
    BUCKET   *bucket;

    DBG1("MAKchar_width(%ld)\n", chId);

    if(IXsetup_chr_def(chId, chr_def, &bucket))
	return 0xffffffff;

  /* Check for fixed space first */

    if((chId == 0x20) || (chId == 0xa0))
    {
        DBG1("    space band %ld\n", bucket->pattribute->spaceBand);
        return(bucket->pattribute->spaceBand);
    }
    if(is_compound)
    {
        DBG1("    compound character %ld\n", h_esc);
        return (UWORD) h_esc;
    }
    DBG1("    %ld\n", (bucket->pwidth_seg + chr_def[0].index)->width);
    return((bucket->pwidth_seg + chr_def[0].index)->width);
}








/*-------------------*/
/*  set_kern         */
/*-------------------*/
MLOCAL BOOLEAN
set_kern(code, chId, left, right)
    UWORD code;
    UWORD chId;
    WORD  *left, *right;
{
    BUCKET     *bucket;
    WORD        index;
    UBYTE      *kern;
    UWORD       i;

    DBG2("set_kern(code=%ld, chId=%ld)\n", code, chId);

    if(IXsetup_chr_def(chId, chr_def, &bucket))  return FALSE;
    if(is_compound) return FALSE;  /* 01-28-91 jfd */

    index = chr_def[0].index;

  /* Point kern to start of sector info; skip 6 byte header */


    if(code == TEXT_KERN)
        kern = bucket->ptext_kern_seg;
    else /* must be DESIGN_KERN */
        kern = bucket->pdesign_kern_seg;

    if(!kern)
    {
        DBG("    missing kerning segment\n");
        return FALSE;
    }

    DBG3("    sign %ld   unit %ld    num sectors %ld\n",
                     *(WORD*)kern, *(WORD*)(kern+2), *(WORD*)(kern+4));

    kern += 6 + 4*index;

    for(i=0; i<4; i++)
    {
        *left++ = *kern >> 4;
        *right++ = *kern++ & 0xf;
    }

    return TRUE;
}

/*-------------------*/
/*  MAKkern          */
/*-------------------*/
GLOBAL WORD
MAKkern(code, chId0, chId1)
    UWORD code;
    UWORD chId0, chId1;
{
    WORD  left0[4], right0[4],
          left1[4], right1[4];
    WORD  *r, *l;
    WORD  min, sum;
    UWORD i;

    DBG("MAKkern()\n");

    if(!set_kern(code, chId0, &left0[0], &right0[0])) return 0;
    if(!set_kern(code, chId1, &left1[0], &right1[0])) return 0;

    min = 256;
    r = right0;
    l = left1;
    for(i=0; i<4; i++)
    {
        DBG2("    %ld %ld\n", *r, *l);
        sum = *r++ + *l++;
        if(sum < min) min = sum;
    }
    DBG1("    min = %ld\n", min);

  /* Convert from 54 units per em to 8782 units per em */

    return (WORD)(((LONG)min * 8782L + 27L) / 54L);
}

