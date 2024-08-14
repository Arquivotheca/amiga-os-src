/* fill.c */
/* Copyright (C) Agfa Compugraphic, 1989. All rights reserved. */
/*    27-Nov-90  bg   Changed type of "or_on" in EXTERN statement from
 *                    WORD to BOOLEAN to resolve some missing pixel
 *                    problems.
 */

#include "debug.h"
#include "port.h"
#include "filltab.h"     /* definition of fill tables */

EXTERN  BOOLEAN  or_on;  /* 11-27-90 bg */





/*--------------------*/
/*    fill_bold       */
/*--------------------*/
MLOCAL VOID
fill_bold (y0, y1, y2, dstptr, width, depth)
    UBYTE  *y0;
    UBYTE  *y1;
    UBYTE  *y2;
    UBYTE  *dstptr;
    WORD    width;   /* width of bitmap in bytes   */
    WORD    depth;   /* depth of bitmap in rasters */
{
    UBYTE  b;            /*  Destination BYTE   */
    WORD   on_count;
    WORD   on_change;
    WORD   w,d;          /*  width and depth counters  */
    WORD   bit;          /*  bit counter */
    UBYTE  bit_mask, b0, b1, b2;


    for (d=0; d<depth; d++)           /* for each horizontal raster        */
    {
        on_count = 0;                 /* beam off at start of each raster  */
DBG1("\nraster %ld\n", d);

        for(w=0; w<width; w++)        /* loop for each byte of the raster  */
        {
            bit_mask = 0x80;
            b0 = *y0++;
            b1 = *y1++;
            b2 = *y2++;
            b = 0;
            for(bit=0; bit<8; bit++)
            {
                b <<= 1;              /*  shift to next destination bit    */

                  /*  Get next on count change */

                    on_change = 0;
                    if(b0 & bit_mask) on_change++;
                    if(b1 & bit_mask) on_change += 2;
                    if(b2 & bit_mask) on_change += 4;
                    if(on_change>3)
                        on_change |= 0xfff8;  /*  sign extend  */
                    on_count += on_change;
DBG2("    on_change %ld      on_count %ld\n", on_change, on_count);

                if(on_count > 0)      /*  If we're on                      */
                    b |= 1;           /*  write another bit to destination */

                bit_mask >>= 1;
            }
            *dstptr++ = b;            /* Write UBYTE to destination        */
        }
    }
}



/*--------------------*/
/*    fill            */
/*--------------------*/
GLOBAL VOID
fill (srcptr, dstptr, width, depth, or_buffer, making_bold, bold_buf2)
    UBYTE  *srcptr;
    UBYTE  *dstptr;
    WORD    width;   /* width of bitmap in bytes   */
    WORD    depth;   /* depth of bitmap in rasters */
    UBYTE  *or_buffer;
    BOOLEAN making_bold;
    UBYTE  *bold_buf2;
{
    UBYTE  p;

    BYTE   mode;          /* writing beam mode.  Off = 0,  on = 1. */
    WORD   w,d;
    UBYTE *dptr;
    WORD   tran_size;

#ifdef  DEBUG
    DBG3("fill w %ld d %ld, %ld\n", width, depth, or_on);
    dptr = srcptr;
    for (d = 0; d < depth; d++) {
        for(w=0; w<width; w++) {
	    mode = *dptr++;
	    for (tran_size = 0; tran_size < 8; tran_size++)
		if ((mode << tran_size) & 0x80) {
		    DBG("*");
		}
		else {
		    DBG("-");
		}
	}
	DBG("\n");
    }
#endif

    if(making_bold)
    {
        DBG("fill bold\n");
        fill_bold (srcptr, or_buffer, bold_buf2, dstptr, width, depth);
        return;
    }
    DBG("fill normal\n");
    dptr = dstptr;    /* remember start of destination */

    for (d=0; d<depth; d++)          /* for each horizontal raster      */
    {
        mode = 0;                    /* beam off at start of each raster */

        for(w=0; w<width; w++)       /* loop for each byte of the raster */
        {
            p = *srcptr++;
            if(mode ==0)
            {
                mode      = off_nxt[p];
                *dstptr++ = off_tab[p];
            }
            else
            {
                mode      = (BYTE)1 - off_nxt[p];
                *dstptr++ = ~off_tab[p];
            }
        }
    }

#ifdef  DEBUG
    dstptr = dptr;
    DBG1("fill result @ $%lx\n", dstptr);
    for (d = 0; d < depth; d++) {
        for(w=0; w<width; w++) {
	    mode = *dstptr++;
	    for (tran_size = 0; tran_size < 8; tran_size++)
		if ((mode << tran_size) & 0x80) {
		    DBG("*");
		}
		else {
		    DBG("-");
		}
	}
	DBG("\n");
    }
    if (or_on) {
	DBG("orBuffer\n");
	dstptr = or_buffer;
	for (d = 0; d < depth; d++) {
	    for(w=0; w<width; w++) {
		mode = *dstptr++;
		for (tran_size = 0; tran_size < 8; tran_size++)
		    if ((mode << tran_size) & 0x80) {
			DBG("*");
		    }
		    else {
			DBG("-");
		    }
	    }
	    DBG("\n");
	}
    }
#endif

    if(or_on)
    {
	DBG("or_on\n");
        tran_size = width * depth;
        for(w=0; w < tran_size; w++)  *dptr++ |= *or_buffer++;
    }
}
