/* raster.c */
/* Copyright (C) Agfa Compugraphic, 1989. 1990. All rights reserved. */

#include "debug.h"
#include "profile.h"
#include "port.h"
#include "imath.h"
#include "if_type.h"



#define SETOR  *(ortran+(xx>>3)) |= bit[xx&7];

/*  Missing pixel recovery and the hermit pixel hunter  */

#define AAA	DBG1("    AAA %ld\n", ptran);\
		if(ptran == xx) {\
			DBG1("**%ld\n",xx);\
			*(ortran+(xx>>3)) ^= bit[xx&7];}

#define BBB 	DBG1("    BBB %ld\n", ptran);\
		for(zz = ptran+1; zz<xx; zz++) {\
			DBG1("**%ld\n",zz);\
			*(ortran-dytran+(zz>>3)) |= bit[zz&7];}

#define CCC	DBG1("    CCC %ld\n", ptran);\
		for(zz = ptran-1; zz>xx; zz--) {\
			DBG1("**%ld\n",zz);\
			*(ortran+(zz>>3)) |= bit[zz&7];}


#define DDD	DBG1("    DDD %ld\n", ptran);\
		for(zz=ptran; zz<xx; zz++) {\
			DBG1("**%ld\n",zz);\
			*(ortran-dytran+(zz>>3)) |= bit[zz&7];}


#define EEE	DBG1("    EEE %ld\n", ptran);\
		for(zz=xx; zz<ptran; zz++) {\
			DBG1("**%ld\n",zz);\
			*(ortran-dytran+(zz>>3)) |= bit[zz&7];}

EXTERN VOID  first_loop();     /*  if_init.c  */
EXTERN LOOP* next_loop();

EXTERN VOID np_init_char();    /*  nxt_pt.c  */
EXTERN VOID np_init_loop();
EXTERN VOID nxt_pt();

EXTERN WORD     tty;

EXTERN IF_CHAR  c;
EXTERN IF_DATA  d;


MLOCAL UBYTE bit[8]  = {  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};


/*-------next char--------*/
GLOBAL  BOOLEAN  or_on;
GLOBAL  BOOLEAN  loop_not_done;
GLOBAL  WORD     px1, py1;
/*-------------------------*/

#define xpix       d.x.pixel_size
#define ypix       d.y.pixel_size
#define half_xpix  d.x.half_pixel
#define half_ypix  d.y.half_pixel


/*-------------------*/
/*  set_bold         */
/*-------------------*/

#define SETBOLD  set_bold(xx, ytran, ortran, boldtran, beam);

MLOCAL VOID
set_bold(x, y0, y1, y2, beam)
    WORD x;
    UBYTE *y0, *y1, *y2;
    BOOLEAN beam;
{
    UBYTE bit_mask;
    UWORD byte_offset;
    WORD on_count;
    WORD new_on_count;

  /*  Get current on count value  */

    bit_mask = bit[x&7];
    byte_offset = x>>3;

    on_count = 0;
    if(*(y0 + byte_offset) & bit_mask)  on_count++;
    if(*(y1 + byte_offset) & bit_mask)  on_count += 2;
    if(*(y2 + byte_offset) & bit_mask)  on_count += 4;
    new_on_count = on_count;
  
  /*  Change on count value  */

    if(new_on_count>3) new_on_count |= 0xfff8;  /*  sign extend  */
    if(beam)
      new_on_count++;
    else
      new_on_count--;

    if((new_on_count >3) || (new_on_count <-3))
    {
        DBG("Pseudo Bold on_count overflow\n");
    }

  /*  Store on count value  */

    new_on_count ^= on_count;   /* Now, only one bits must change */
    if(new_on_count & 1) *(y0 + byte_offset) ^= bit_mask;
    if(new_on_count & 2) *(y1 + byte_offset) ^= bit_mask;
    if(new_on_count & 4) *(y2 + byte_offset) ^= bit_mask;
}


/*-------------------*/
/*  raster           */
/*-------------------*/
GLOBAL VOID
raster(tran_buffer, or_buffer, making_bold, bold_buf2)
    UBYTE   *tran_buffer;
    UBYTE   *or_buffer;
    BOOLEAN  making_bold;
    UBYTE   *bold_buf2;
{

#ifdef DEBUG
    UWORD elem_num;
#endif

    WORD    px0, py0;
    WORD    dx, dy;
    WORD    hiyrnd;
    WORD    num_strokes;
    WORD    end_yraster;
    UWORD   tranct;
    WORD    direction;
    LONG    tran_size;
    WORD    dytran;
    WORD    yraster;
    BYTE   *ytran;
    BYTE   *ortran;
    BYTE   *boldtran;

    LOOP   *lp;        /* current loop */


    UWORD   loopct;
    UWORD   i;

  /* Transition production */

    WORD    x, y;             /* Intersection of current y rastor and
                               * current current segment.
                               */
    WORD    deltax;
    WORD    deltax1;          /* if(deltax>0) deltax+1 else deltax-1 */

    WORD    rem;
    WORD    r;
    WORD    absdy;
    WORD    dq;
    WORD    t;
    WORD    xx;               /* Bit offset into raster of transition */

  /* or_on tweaks */

    WORD    zz;
    BOOLEAN beam;             /*  TRUE if current segment makes on trans */
    BOOLEAN first_tran;       /*  TRUE until 1st transition is produced  */

    WORD    ptran;            /*  previous transition: last xx produced  */
                              /*  by previous line segment.              */
    BOOLEAN seton;            /*  Current line segment is producing on   */
                              /*  transitions.                           */
    BOOLEAN wason;            /*  Previous line segment was producing on */
                              /*  transitions.                           */

        /*  Values when first transition is produced. These values are
         *  used to close the loop by linking the last transition in
         *  the loop to the first transition.
         */

    BOOLEAN seton1;
    WORD    xx1;
    WORD    dx1;
    WORD    dy1;
    WORD    dytran1;
    UBYTE  *ortran1;


    DBG("\n\n\n\nR A S T E R ( )\n\n");

    DBG1("number of loops = %ld\n", c.num_loops);
    DBG2("d.bmwidth = %ld    d.bmdepth = %ld\n",d.bmwidth,d.bmdepth);

    d.y.margin -= half_ypix;
    tty -=  half_ypix;

    hiyrnd   = ypix - 1;
    DBG2("xpix = %ld    ypix = %ld\n", xpix, ypix);


  /* clear transition array */

    tran_size = (LONG)d.bmwidth*(LONG)d.bmdepth;

  /* Initialize vector generator */

    np_init_char();

    first_loop();
    for(loopct = 0; loopct < c.num_loops; loopct++)
    {
#ifdef DEBUG
  elem_num = 0;
#endif

        DBG("\n\n\nN e x t    L o o p\n");
        lp = next_loop();
        np_init_loop(lp);     /* initialize next point producer */
                              /* 1st point is in px1, py1       */

        DBG2("----first element  %ld %ld\n", px1, py1);

      /* Set direction  and initialize transition_producer */

        direction  = -1;          /* assume going down */
        beam       = ((c.ctree + loopct) -> polarity == 1);
        dytran     = d.bmwidth;
        yraster    = py1 >> d.y.grid_shift;

        first_tran = TRUE;

      /* Set pointer to transition array to correspond to yraster */

        ytran    = tran_buffer + tran_size - ((yraster+1) * d.bmwidth);
        ortran   = or_buffer   + tran_size - ((yraster+1) * d.bmwidth);
        boldtran = bold_buf2   + tran_size - ((yraster+1) * d.bmwidth);


        tranct  = 0;    /* no transitions produced yet */

        DBG2("direction %ld  dytran %ld\n",direction, dytran);
        DBG1("  yraster %ld\n",yraster);

#ifdef PROFILE
if(prof_val >= PROF_nxt_pt)
    continue;
#endif
        while(loop_not_done)
        {
          /* get next vector point */

            px0 = px1;
            py0 = py1;
            nxt_pt();

#ifdef PROFILE
if(prof_val >= PROF_rast_loop)
    continue;
#endif

            dx = px1 - px0;
            dy = py1 - py0;

#ifdef DEBUG
	    {
		short qqqd, qqqw, qqqb;
		for (qqqd = 0; qqqd < d.bmdepth; qqqd++) {
		    for(qqqw = 0; qqqw < d.bmwidth; qqqw++) {
			for (qqqb = 0; qqqb < 8; qqqb++) {
			    if (tran_buffer[qqqd*d.bmwidth+qqqw]&bit[qqqb]) {
				DBG("*");
			    }
			    else {
				DBG("-");
			    }
			}
		    }
		    DBG("\n");
		}
	    }
#endif

            DBG5("--next element  %ld %ld    %ld %ld---------------------%ld\n",
                    px0,py0,px1,py1,elem_num++);
            DBG4("                %ld%% %ld%%   %ld%% %ld%%\n",
                    px0*100/xpix, py0*100/ypix, px1*100/xpix, py1*100/ypix);

            DBG2("           dx dy  %ld %ld\n", dx, dy);

            if(!dy) continue;  /* skip horizontal elements */

          /* see if we changed direction- up or down */

            if ((dy ^ direction) < 0)
            { /* we did */
              direction = - direction;
              beam      =  !beam;
   
              yraster  +=  direction;

              dytran    = - dytran;
              ytran    +=   dytran;
              ortran   +=   dytran;
              boldtran +=   dytran;
            }


            /* Compute last stroke. Continue on to next vector  */
            /* if this segment isn't stroked.                   */

            DBG1("  yraster %ld\n",yraster);

            if (direction > 0)
            {
                DBG1("    direction = %ld  UP\n",direction);
                end_yraster = py1 >> d.y.grid_shift;
                num_strokes = end_yraster - yraster + 1;
            }
            else
            {
                DBG1("    direction = %ld  DOWN\n",direction);
                end_yraster = (py1 + hiyrnd) >> d.y.grid_shift;
                num_strokes = yraster - end_yraster + 1;
            }

            DBG2("    end_yraster = %ld   num_strokes = %ld\n\n\n",
                                              end_yraster,num_strokes);

            if(!num_strokes) continue;
#ifdef PROFILE
if(prof_val >= PROF_rast_tran) continue;
#endif


            /*  P R O D U C E    T R A N S I T I O N S */

            seton = or_on && beam;

            if (dx == 0)
            {
              /*  Character countour is vertical; worth special casing. */

                DBG("     vertical stroke\n");

                xx = (px0 + half_xpix) >> d.x.grid_shift;

                if(first_tran)
                {
                    first_tran = FALSE;
                    wason   = seton;
                    ptran   = xx;

                    dx1     = dx;
                    dy1     = dy;
                    xx1     = xx;
                    seton1  = seton;
                    ortran1 = ortran;
                    dytran1 = dytran;
                }

              /* S E T    T R A N S I T I O N    B I T */
                DBG1("                        xx %ld\n",xx);

                if(making_bold)
                    SETBOLD
                else
                {
                    *(ytran + (xx>>3)) ^= bit[xx&7];
                    if(seton)
                    {
                        SETOR
                        if(wason)
                        {
                            CCC
                            BBB
                        }
                        else
                        {
                            AAA
                            if(dy>0)
                            {
                                DDD
                            }
                        }
                    }
                    else
                    {
                        if(wason)
                        {
                            AAA
                            EEE
                        }
                    }
                }
                yraster += direction;
                ytran   += dytran;
                ortran  += dytran;
                boldtran +=   dytran;

                for (i=1; i<num_strokes; i++)
                {
                  /* S E T    T R A N S I T I O N    B I T */
                    DBG1("                        xx %ld\n",xx);

                    if(making_bold)
                        SETBOLD
                    else
                    {
                        *(ytran + (xx>>3)) ^= bit[xx&7];
                        if(seton)
                            SETOR
                    }

                    yraster += direction;
                    ytran   += dytran;
                    ortran  += dytran;
                    boldtran +=   dytran;
                }

                tranct  += num_strokes;

            }
            else  /* character contour is diagonal */
            {
              /* Compute first transition point */

                DBG("first transition\n");
              
                y = yraster * ypix;
                DBG1("    y %ld\n", y);

                dq  = dy<<1;
                if(dq<0)  dq = - dq;
                DBG1("    dq %ld\n", dq);

                deltax = scale_rem(y-py0, dx, dy, &rem, &deltax1);
                DBG3("    deltax %ld  rem %ld  deltax1 %ld\n",
                                                 deltax, rem, deltax1);

                r = rem - (dq>>1);    /* Initialize r for new x */
                DBG1("    init r %ld\n", r);

                x = px0 + half_xpix;
                if (r>0)
                {
                    x += deltax1;
                    r -= dq;            /* adjust r for current x */
                }
                else
                    x += deltax;

                xx = x >> d.x.grid_shift;
                DBG3("    x %ld    xx %ld   next r %ld", x, xx, r);

                if(first_tran)
                {
                    first_tran = FALSE;
                    wason   = seton;
                    ptran   = xx;

                    dx1     = dx;
                    dy1     = dy;
                    xx1     = xx;
                    seton1  = seton;
                    ortran1 = ortran;
                    dytran1 = dytran;
                }

              /* S E T    T R A N S I T I O N    B I T */
                DBG1("                xx %ld\n",xx);

                if(making_bold)
                    SETBOLD
                else
                {
                    *(ytran + (xx>>3)) ^= bit[xx&7];
                    if(seton)
                    {
                        SETOR
                        if(wason)
                        {
                            if(dx>0)
                            {
                                BBB
                            }
                            else
                            {
                                CCC
                            }
                        }
                        else
                        {
                            AAA
                            if(dy>0)
                            {
                                DDD
                            }
                        }
                    }
                    else
                    {
                        if(wason)
                        {
                            AAA
                            EEE
                        }
                    }
                }

                yraster += direction;
                ytran   += dytran;
                ortran  += dytran;
                boldtran +=   dytran;
                tranct++;

                if (num_strokes > 1)
                {
                    DBG("\n\n        Bresnham\n");

                    if(dy>0)  absdy =  dy;
                       else   absdy = -dy;

                    deltax = scale_rem(ypix, dx, absdy, &rem, &deltax1);
                    DBG3("    deltax %ld  rem %ld  deltax1 %ld\n",
                                                     deltax, rem, deltax1);

                    t = rem - dq;  /*  To be used when r>0. First subtract
                                    *  dq to account for the extra 1 we
                                    *  moved x, then add rem to set r for
                                    *  the next y raster.
                                    */

                    r += rem;      /* set r for next y raster */
                    DBG2("    t %ld   next r %ld\n", t, r);

                    ptran = xx;
                    for(i=1; i<num_strokes; i++)
                    {
                      /*    N E X T    X X  */

                        if(r>0)
                        {
                            x += deltax1;
                            r += t;             /* set r for next x */
                        }
                        else
                        {
                            x += deltax;
                            r += rem;           /* set r for next x */
                        }

                        xx = x >> d.x.grid_shift;
                        DBG3("    x %ld    xx %ld   next r %ld", x, xx, r);

                      /* S E T    T R A N S I T I O N    B I T */
                        DBG1("                        xx %ld\n",xx);

                        if(making_bold)
                            SETBOLD
                        else
                        {
                            *(ytran + (xx>>3)) ^= bit[xx&7];
                            if(seton)
                            {
                                SETOR
                                if(dx>0)     
                                {
                                    BBB
                                }
                                else
                                {
                                    CCC
                                }

                                ptran = xx;
                            }
                        }

                        yraster += direction;
                        ytran   += dytran;
                        ortran  += dytran;
                        boldtran +=   dytran;

                    }   /* for loop */
                    tranct  += num_strokes - 1;

                } /* if num_strokes > 1 */
            }  /*  else character contour is diagonal */

            wason = seton;
            ptran = xx;       /* last x transition */

        } /*  while(loop_not_done) */


      /*    O D D    T R A N S I T I O N    C H E C K      */
      /* If an odd number of transitions were produced     */
      /* during this loop, then undo the last  transition. */
      /* This can happen if we start and stop a loop on    */
      /* the same transition, inadvertantly turning it off.*/
      /* This code turns it back on.                       */

                  if (tranct & 1)
                  {
                      DBG("Odd transition count fix up");
                      ytran    -= dytran;
                      if(making_bold)
                          set_bold(xx, ytran, ortran-dytran,
                                                boldtran-dytran, !beam);
                      else
                          *(ytran + (xx>>3)) ^= bit[xx&7];
                  }

      /*  Connect last line segment to first line segment in the
       *  or_on buffer.
       */

        if(tranct)
        {
            seton  = seton1;
            xx     = xx1;
            dx     = dx1;
            dy     = dy1;
            ortran = ortran1;
            dytran = dytran1;

            if(dx == 0)
            {
                        if(seton)
                        {
                            SETOR
                            if(wason)
                            {
                                CCC
                                BBB
                            }
                            else
                            {
                                AAA
                                if(dy>0)
                                {
                                    DDD
                                }
                            }
                        }
                        else
                        {
                            if(wason)
                            {
                                AAA
                                EEE
                            }
                        }
            }
            else
            {
                    if(seton)
                    {
                        SETOR
                        if(wason)
                        {
                            CCC
                            BBB
                        }
                        else
                        {
                            AAA
                            if(dy>0)
                            {
                                DDD
                            }
                        }
                    }
                    else
                    {
                        if(wason)
                        {
                            AAA
                            EEE
                        }
                    }
            }
        } /* if(tranct) */			 	
    } /* end of character */

    d.y.margin += half_ypix;   /* restore y translation */
    tty +=  half_ypix;
    DBG("raster done.\n");
}

