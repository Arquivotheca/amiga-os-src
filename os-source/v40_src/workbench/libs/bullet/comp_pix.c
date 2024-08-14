/*
**	$Id: comp_pix.c,v 7.0 91/03/19 18:35:47 kodiak Exp $
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
**
**	$Log:	comp_pix.c,v $
 * Revision 7.0  91/03/19  18:35:47  kodiak
 * after Amigaizing memory, lists, and adding UniCode.  Before cache work.
 * 
 * Revision 6.0  91/03/18  15:25:57  kodiak
 * folded in Bullet 1.30
 * 
 * Revision 3.0  90/11/09  17:08:17  kodiak
 * Kodiak's Alpha 1
 * 
*/
/*  comp_pix.c  */
/* Copyright (C) Agfa Compugraphic, 1989. All rights reserved. */
/*-----------------------------------------------------------------
  20-Mar-90  jfd  Added check for tangent already computed
                  in routine COMP_PIX()
  05-May-90  awr  Added landscape. Set new variable d.quadrant to indicate
                  rotation type. Eliminated rotate_on. Swap x and y
                  resolution and spot size for 90 and 270 degree rotation.
  07-Jul-90  awr  removed above swapping.
  10-Sep-90  awr  changed computations for arbitrary rotation and shear to
                  produce a rotated pixel size and spot size.
  25-Sep-90  dET  made 'fpix' and 'fnew_pix' MLOCAL to prevent protection
                  violations under windows 3.0. This is an obscure bug!
  11-Nov-90  dET  Added check in comp_pix to limit  ( -45 < shear < 45 )
                  as does IF 2.2.

---------------------------------------------------------------------------*/

#include <exec/types.h>
#include <exec/nodes.h>
#include "debug.h"
#include "port.h"
#include "imath.h"
#include "cgif.h"
#include "if_type.h"
#include "segments.h"
#include "ix.h"
#include "fmath.h"

#define CON_SIZE 2

/* nxt_pt.c */
EXTERN UWORD     quality;
/* if_init.c */
EXTERN IF_DATA   d;
EXTERN FACE      face;

EXTERN pixel_align();
EXTERN struct {
                  WORD value;
                  WORD grid_line;
              } pixel_aligned;

GLOBAL  MATRIX   tran;

MLOCAL  FIXED fpix;
MLOCAL  FIXED fnew_pix;
 

/*--------------------*/
/*   shift_left       */
/*--------------------*/
/*  Shift the ULONG val left as far as possible; until bit 31, the
 *  "sign bit" is 1. Return the shifted val. Add the number of
 *  bits shifted to *shift.
 */
MLOCAL ULONG
shift_left(val, shift)
    ULONG  val;
    WORD *shift;
{
    while (val < 0x80000000)
    {
        val <<= 1;
        (*shift)++;
    }
    return val;
}


/*--------------------*/
/*   fill_in_data     */
/*--------------------*/
MLOCAL VOID
fill_in_data(xy, grid_shift, t, s, spot, bold, fixed_pix)
    COORD_DATA *xy;
    WORD        grid_shift;
    ULONG       t;          /* precise pixel size = t >> s */
    WORD        s;
    FIXED       spot;
    WORD        bold;
    FIXED       fixed_pix;
{
    WORD new_pix, mask;
    WORD i;
    LONG  bias;
    LONG  phpix;

    new_pix = 1;
    mask    = 0xffff;
    for(i=0; i < grid_shift; i++)
    {
        new_pix <<= 1;
        mask    <<= 1;
    }

    xy->pixel_size = new_pix;
    xy->grid_shift = grid_shift;
    xy->grid_align = mask;
    xy->half_pixel = new_pix >> 1;


    DBG2("    pixel_size = %ld  half_pixel = %ld\n",xy->pixel_size,
                                                  xy->half_pixel);
    DBG2("    grid_align = %x  grid_shift = %ld\n", xy->grid_align,
                                                   xy->grid_shift);

  /*  Constrained dimension limit = actual pixel size * CON_SIZE.
   *  (CON_SIZE = 2)
   */

    xy->con_size = (WORD)((t * (LONG)CON_SIZE) >> s);

  /* Fractional pixel values used in skeletal processing */

    xy->p_pixel    = (WORD)t;
    phpix          = t >> 1;      /* precise half pixel */
    xy->p_half_pix = (WORD)phpix;
    xy->bin_places = s;

    bias = (fmul(fixed_pix, spot - F_ONE) >> (16-s))  - ((LONG)bold << s);


    xy->round[0] = phpix - bias;
    xy->round[1] = phpix + bias;
    xy->round[2] = phpix;
    xy->round[3] = phpix;
    DBG4("    round  %ld  %ld  %ld  %ld\n", xy->round[0], xy->round[1],
                                            xy->round[2], xy->round[3]);
    DBG2("    p_pixel = %ld  bin_places = %ld\n", xy->p_pixel, xy->bin_places);
    DBG1("    con_size = %ld\n", xy->con_size);

  /* Standard dimension */

    xy->stand_value = 0;   /* to indicate that it must be recomputed */
}




/*--------------------*/
/* fill_in_rot_data   */
/*--------------------*/
MLOCAL VOID
fill_in_rot_data(xy, fpixel, bold, overhang)
    COORD_DATA *xy;
    FIXED fpixel;
    WORD  bold;
    FIXED overhang;
{
    ULONG t;
    WORD  s;
    LONG  bias;
    LONG  phpix;

    t = fpixel;
    s = 16;
    while ( t >= 0x8000 )  /* shift till < 32K */
    {
        t >>= 1;
        s--;
    }

  /*  Constrained dimension limit = actual pixel size * CON_SIZE.
   *  (CON_SIZE = 2)
   */

    xy->con_size = (WORD)((t * (LONG)CON_SIZE) >> s);

  /* Fractional pixel values used in skeletal processing */

    xy->p_pixel    = (WORD)t;
    phpix          = t >> 1;      /* precise half pixel */
    xy->p_half_pix = (WORD)phpix;
    xy->bin_places = s;

    bias = (overhang >> (16-s))  - ((LONG)bold << s);


    xy->round[0] = phpix - bias;
    xy->round[1] = phpix + bias;
    xy->round[2] = phpix;
    xy->round[3] = phpix;
    DBG4("    round  %ld  %ld  %ld  %ld\n", xy->round[0], xy->round[1],
                                            xy->round[2], xy->round[3]);
    DBG2("    p_pixel = %ld  bin_places = %ld\n", xy->p_pixel, xy->bin_places);
    DBG1("    con_size = %ld\n", xy->con_size);

  /* Standard dimension */

    xy->stand_value = 0;   /* to indicate that it must be recomputed */
}



/*--------------------*/
/*   rotate_pixel     */
/*--------------------*/
MLOCAL BOOLEAN
rotate_pixel(set, pnt, abscos, abssin, p, q, ho, vo, xbold, ybold)
    FIXED set, pnt;
    FIXED abscos, abssin;  /* absolute values of cos and sin */
    FIXED p, q;            /* pixel size just after rotate- actual size    */
    FIXED ho, vo;
    WORD  xbold, ybold;
{
    FIXED ver, hor;        /* pixel size just before rotate                */
    FIXED xpixel, ypixel;  /* pixel size in original design space          */
    FIXED dho, dvo;

  /* Compute pixel size just before rotation */

    ver = fmul(q, abscos) + fmul(p, abssin);
    hor = fdiv(fmul(p, q), ver);

  /* Compute pixel size in original design space */

    xpixel = fdiv(hor, set);
    ypixel = fdiv(ver, pnt);

  /* Compute spot overhang in original design space */

    dho = fdiv(fmul(ho, abscos) + fmul(vo, abssin), set);
    dvo = fdiv(fmul(vo, abscos) + fmul(ho, abssin), pnt);

    DBG4("    hor ver %ld.%04ld %ld.%04ld\n", DBINT(hor), DBFRAC(hor),
	    DBINT(ver), DBFRAC(ver));
    DBG4("    xpixel ypixel %ld.%04ld %ld.%04ld\n", DBINT(xpixel),
	    DBFRAC(xpixel), DBINT(ypixel), DBFRAC(ypixel));
    DBG4("    dho dvo %ld.%04ld %ld.%04ld\n", DBINT(dho), DBFRAC(dho),
	    DBINT(dvo), DBFRAC(dvo));

    if(fmath_error)
    {
        DBG("***** fmath_error ***** \n");
        return FALSE;
    }

    fill_in_rot_data(&d.x, xpixel, xbold, dho);
    fill_in_rot_data(&d.y, ypixel, ybold, dvo);

    return TRUE;
}





/*--------------------*/
/*   rpix_calc        */
/*--------------------*/
/*  This function performs three functions:
 *
 *    1. Fills in the skeletal processing pixel data to handle psuedo
 *       bold for rotated/sheared characters.
 *    2. Fills in the power of two pixel data for rasterization.
 *    3. Computes the final scaling transform that must be
 *       appended to the transform matrix to scale to the the power of
 *       two working space. These scaling factors are refered to
 *       as sx and sy in the design notes. These values are returned.
 */

MLOCAL FIXED
r_pix_calc(xy, scan_res, out_res, rs, bold, spot, pq, hvo)
    COORD_DATA  *xy;
    ULONG scan_res, out_res;
    FIXED rs;
    WORD  bold;
    FIXED spot;
    FIXED *pq;                 /* returned pixel size in actual size space,
                                * just after rotation.
                                */
    FIXED *hvo;                /* returned spot size overlap in actual size
                                * space
                                */
{
    UWORD shft;
    WORD  grid_shift;

    DBG4("\nr_pix_calc(scan_res = %ld, out_res = %ld, rs = %ld.%04ld,\n",
	    scan_res, out_res, DBINT(rs), DBFRAC(rs));
    DBG3("                   bold = %ld, spot = %ld.04ld)\n",
            bold, DBINT(spot), DBFRAC(spot));


  /* Pixel size */

    shft = 0;
    fpix = (FIXED)(shift_left(scan_res, &shft) / out_res);
    if(shft<16) fpix <<= (16-shft);    /* convert to FIXED format */
           else fpix >>= (shft-16);

    *pq = fpix;                       /* Pixel size in actual size space,
                                       * just after rotation.
                                       */
    *hvo = fmul(fpix, spot - F_ONE);  /* overhang in above space */

    fpix = fdiv(fpix,rs);             /* Actual pixel size in design space
                                       * after all transforms except final
                                       * scaling to working space.
                                       */

    xy->orig_pixel = F2I(fpix);

  /* power of two pixel size in working space */

    fnew_pix = F_ONE;
    grid_shift = 0;
    while(fnew_pix <= fpix)
    {
        fnew_pix <<= 1;
        grid_shift ++;
    }
    fnew_pix >>= 1;
    grid_shift--;

  /* Divide power of two pixel size by 4 */

    fnew_pix  >>= 2;
    grid_shift -= 2;

  /*  Fractional pixel values used in skeletal processing. Hard wire
   *  to some very small value so we don't distort the character by
   *  aligning much. Ignore spot size- assume it is set to 1.
   */

    fill_in_data(xy, grid_shift, 32000L, 10, F_ONE, bold, F_ONE);

    return fdiv(fnew_pix, fpix);   /* scaing factor sx or sy */
}




/*--------------------*/
/*   pix_calc         */
/*--------------------*/
/*  Compute pixel data for x or y.
 *
 *  Actual pixel size in design units = (masterpt * scanres) /(size * outres)
 *  This pixel size is rounded up to the nearest power of two. All input
 *  data is eventually scaled to conform to this new pixel size.
 *
 *  This function computes
 *
 *      p_size        original pixel size in binary fraction
 *      pixel_size    new pixel size, power of 2
 *      half_pixel    pixel_size / 2
 *      grid_shift    ammount to shift x to obtain x/pixel_size
 *      grid_align    x & grid_align is a whole number of pixels
 *
 *      p_size is the original pixal size shifted left to obtain
 *             as much precision as possible.
 *
 *             original pixel size = p_size/(2**s)
 *             where s is determined so that  16K <= p_size < 32K
 */

MLOCAL VOID
pix_calc (xy, masterpt, scanres, outres, size, spot, bold)
        COORD_DATA  *xy;
        ULONG         masterpt;
        ULONG         scanres;
        ULONG         outres;
        ULONG         size;
        FIXED         spot;
        WORD          bold;
{
    ULONG t;
    WORD  s;
    FIXED fixed_pix;

    s = 0;

    t = shift_left((ULONG)(masterpt * scanres), &s);
    t = shift_left((ULONG)(t/outres), &s);
    t = shift_left(t / size, &s);

    fixed_pix = t >> (s - 16);   /* convert to FIXED for later */

    while ( t >= 0x8000 )  /* shift till < 32K */
    {
        t >>= 1;
        s--;
    }

  /*  Original pixel size with no fractional precision. Used to
   *  compare with type face or on threshold.
   */

    xy->orig_pixel = (WORD)(t >> s);

  /*  Fill in power of two pixel values and fractional values used
   *  in skeletal processing.
   */

    fill_in_data(xy, 12-s, t, s, spot, bold, fixed_pix);
                  /* 12-s = grid_shift */

  /* Grid aligned max design coordiante */

    pixel_align(MAX_DESIGN, xy, 2);
    xy->max_des = pixel_aligned.value;
    DBG1("    grid aligned max_des %ld\n", xy->max_des);
}

/*--------------------*/
/*     comp_pix       */
/*--------------------*/
/*  Computes the scaling paramters */

GLOBAL BOOLEAN
comp_pix(fc, bucket)
    FONTCONTEXT  *fc;
    BUCKET       *bucket;
{
  /* Rotate and Shear */

    FIXED fmasterpt;
    FIXED set, pnt;
    FIXED cos, sin, tan;
    FIXED scos, ssin;
    FIXED m0, m1, m2, m3;
    FIXED r, s;            /* scale facter from rotated actual size to 16K */
    FIXED sx, sy;          /* scale facter to go to fractioanl pixel space */
    FIXED p, q;            /* pixel size just after rotate- actual size    */
    FIXED ho, vo;

  /* Scan resolution */

    ULONG mastpt;
    ULONG x_scan_meter;
    ULONG y_scan_meter;

    WORD  min_pix;

    DISPLAY  *display;

    DBG("\ncomp_pix()\n");

    display = bucket->pdisplay;  /* recover DISPLAY ptr */

  /*  Convert FAIS scans per inch into scans per meter.
   *
   *       scans_per_meter = (10,000 * scans_per_inch)/254
   */

    x_scan_meter = (10000L * (ULONG)display->scanResolutionX)/254L;
    y_scan_meter = (10000L * (ULONG)display->scanResolutionY)/254L;


  /*  Rotation and shear  */

    cos  = fc->rotate.x;        /*  rotation */
    sin  = fc->rotate.y;
    scos = fc->shear.y;         /*  shear    */
    ssin = fc->shear.x;
    tan = face.glob_ital_tan;


  /* Check for special 90 degree rotations */

    if(ssin != 0 || scos != F_ONE ||     /* shear is like arb rotation */
           tan != NO_TANGENT)     /* 20-Mar-90 jfd */

        d.quadrant = 0;
    else if(cos == F_ONE && sin == 0)    /* no rotation        */
        d.quadrant = ROT0;
    else if(cos == 0 && sin ==F_ONE)     /* rotate 90 degrees  */
        d.quadrant = ROT90;
    else if(cos == -F_ONE && sin == 0)   /* rotate 180 degrees */
        d.quadrant = ROT180;
    else if(cos == 0 && sin == -F_ONE)    /* rotate 270 degrees */
        d.quadrant = ROT270;
    else
        d.quadrant = 0;                   /* arbitrary rotation */

    DBG1("    d.quadrant = %ld\n", d.quadrant);

    if(!d.quadrant)
    {
      /* Arbitrary rotation */

        fmath_error = FALSE;          /* fmath error flag */

        fmasterpt = I2F((WORD)display->masterPointSize);

        if (tan == NO_TANGENT)   /* 20-Mar-90 jfd */
           tan = fdiv(ssin, scos);

        if(tan < -F_ONE || tan > F_ONE)  /* 11-19-90 dET */
           return FALSE;

        set = fdiv( I2F(fc->set_size),    fmasterpt);
        pnt = fdiv( I2F(fc->point_size),  fmasterpt);

        DBG2("    fmasterpt %ld.%04ld\n", DBINT(fmasterpt), DBFRAC(fmasterpt));
        DBG4("    cos  %ld.%04ld   sin  %ld.%04ld\n", DBINT(cos), DBFRAC(cos),
                                        DBINT(sin), DBFRAC(sin));
        DBG2("    tan %ld.%04ld\n", DBINT(tan), DBFRAC(tan));
        DBG4("    set  %ld.%04ld   pnt  %ld.%04ld\n", DBINT(set), DBFRAC(set),
                                        DBINT(pnt), DBFRAC(pnt));
        if(fmath_error)
        {
            DBG("***** fmath_error ***** tan, set, pnt compute\n");
            return FALSE;
        }

        m0 = fmul(cos, set);
        m1 = fmul(sin, set);
        m2 = fmul(pnt, fmul(cos, tan) - sin);
        m3 = fmul(pnt, fmul(sin, tan) + cos);

        r = MAX( LABS(m0+m2), LABS(m0-m2));
        s = MAX( LABS(m1+m3), LABS(m1-m3));

        DBG4("    m0 m1  %ld.%04ld  %ld.%04ld\n", DBINT(m0), DBFRAC(m0), DBINT(m1), DBFRAC(m1));
        DBG4("    m2 m3  %ld.%04ld  %ld.%04ld\n", DBINT(m2), DBFRAC(m2), DBINT(m3), DBFRAC(m3));
        DBG4("    r  s   %ld.%04ld  %ld.%04ld\n", DBINT(r ), DBFRAC(r ), DBINT(s ), DBFRAC(s ));

        if(fmath_error)
        {
            DBG("***** fmath_error ***** mi compute\n");
            return FALSE;
        }

        sx = r_pix_calc(&d.x, x_scan_meter, fc->xres, r, fc->xbold,
                                                         fc->xspot, &p, &ho);
        sy = r_pix_calc(&d.y, y_scan_meter, fc->yres, s, fc->ybold,
                                                         fc->yspot, &q, &vo);
        DBG4("    sx sy  %ld.%04ld  %ld.%04ld\n", DBINT(sx), DBFRAC(sx), DBINT(sy), DBFRAC(sy));
        DBG4("    p  q   %ld.%04ld  %ld.%04ld\n", DBINT(p ), DBFRAC(p ), DBINT(q ), DBFRAC(q ));
        DBG4("    ho vo  %ld.%04ld  %ld.%04ld\n", DBINT(ho), DBFRAC(ho), DBINT(vo), DBFRAC(vo));
        if(fmath_error)
        {
            DBG("***** fmath_error ***** r_pix_calc()\n");
            return FALSE;
        }

        tran[0] = fmul(sx, fdiv(m0, r));     tran[1] = fmul(sy, fdiv(m1, s));
        tran[2] = fmul(sx, fdiv(m2, r));     tran[3] = fmul(sy, fdiv(m3, s));
        tran[4] = fmul(sx, fmul(0x20000000, F_ONE - fdiv(m0+m2, r)));
        tran[5] = fmul(sy, fmul(0x20000000, F_ONE - fdiv(m1+m3, s)));
        DBG("    tran[]\n");
        DBG4("        %ld.%04ld  %ld.%04ld\n", DBINT(tran[0]), DBFRAC(tran[0]),
                                 DBINT(tran[1]), DBFRAC(tran[1]));
        DBG4("        %ld.%04ld  %ld.%04ld\n", DBINT(tran[2]), DBFRAC(tran[2]),
                                 DBINT(tran[3]), DBFRAC(tran[3]));
        DBG4("        %ld.%04ld  %ld.%04ld\n", DBINT(tran[4]), DBFRAC(tran[4]),
                                 DBINT(tran[5]), DBFRAC(tran[5]));


        if(fmath_error)
        {
            DBG("***** fmath_error ***** tran[i] compute\n");
            return FALSE;
        }

        if(!rotate_pixel(set, pnt, LABS(cos), LABS(sin), p, q,
                                     ho, vo, fc->xbold, fc->ybold))
            return FALSE;
    }
    else
    {
      /* set up pixel size data */

        mastpt = (ULONG)display->masterPointSize;
        DBG1("master point size %ld\n", mastpt);

        DBG2("xres = %ld    set_size = %ld\n", fc->xres, fc->set_size);
        pix_calc (&d.x, mastpt, x_scan_meter, fc->xres,
                   (ULONG)fc->set_size, fc->xspot, fc->xbold);

        DBG2("yres = %ld    point_size = %ld\n", fc->yres, fc->point_size);
        pix_calc (&d.y, mastpt, y_scan_meter, fc->yres,
                   (ULONG)fc->point_size, fc->yspot, fc->ybold);
    }

    if(d.quadrant == ROT0 || d.quadrant == ROT180)
    {
        d.px = &d.x;
        d.py = &d.y;
    }
    else
    {
        d.px = &d.y;
        d.py = &d.x;
    }

      /* compute baseline */
        d.origBaseline = display->baselinePosition;

      /* Grid aligned baseline in design space */
        pixel_align(d.origBaseline, d.py, 2);
        d.aBaselnVal  = pixel_aligned.value;

        DBG1("    orig baseline = %ld\n", d.origBaseline);
        DBG1("    grid aligned in design space: %ld \n", d.aBaselnVal);

  /* Set quality level */

    if(d.x.orig_pixel < d.y.orig_pixel)
        min_pix = d.x.orig_pixel;
    else
        min_pix = d.y.orig_pixel;

    quality = fc->format >> 14;
    if(!quality)
    {
        if(min_pix < 66)        quality = 3;     /*   > 30 pt at 300dpi  */
        else if(min_pix < 211)  quality = 2;     /* between 10 and 30 pt */
        else                    quality = 1;     /*      < 10 pt         */
    }

    DBG2("    quality = %ld (min_pix %ld)\n", quality, min_pix);
    return TRUE;
}
