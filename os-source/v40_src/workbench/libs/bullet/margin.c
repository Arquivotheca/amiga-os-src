/* margin.c */
/* Copyright (C) Agfa Compugraphic, 1989, 1990. All rights reserved. */

#include <exec/types.h>
#include <exec/nodes.h>
#include "port.h"
#include "debug.h"
#include "cgif.h"

/*----------------------*/
/*    margin            */
/*----------------------*/
GLOBAL VOID
margin(bm)
    IFBITMAP *bm;
{

    UWORD    wid;     /* width of bit map array in BYTEs   */
    UWORD    dep;     /* depth  "  "   "    "    " rasters */
    UWORD    i;
    WORD     col;
    WORD     left, right;
    UBYTE   *p;
    UBYTE    bits;
    UWORD    size;

    DBG("margin()\n");

    wid = bm->width;
    dep = bm->depth;

   /* COMPUTE LEFT INDENT */

    bits = 0;
 
    for(col = 0; col < wid; col++)
    {
        p = bm->bm + col;    /* point p to column */
 
        for ( i = 0; i < dep; i++)
        {
          bits |= *p;
          p += wid; 
        }
        if(bits) break;
    }


  /* Portect against empty bitmap */

    if(!bits)    /* empty bit map */
    {
        DBG("    empty bit map\n");

        bm->left_indent = 1;
        bm->top_indent  = 1;
        bm->black_width = 1;
        bm->black_depth = 1;
        return;
    }

    for(left=0; left<8; left++)
    {
      if(bits & 0x80) break;
      bits <<= 1;
    }
    
    left = left + 8*col;
    bm->left_indent = left;

  /* COMPUTE RIGHT INDENT.  */

    bits = 0;

    for (col = wid - 1;  col >= 0;  col--)
    {
        p = bm->bm + col;

        for(i=0; i<dep; i++)
        {
          bits |= *p;
          p += wid; 
        }
        if (bits) break;
    }

    for(right=0; right<8; right++)
    {
      if(bits & 0x01) break;
      bits >>= 1;
    }


    right = 8*(col+1) - right;

    bm->black_width   = right - left;    /* in pixels */


  /* COMPUTE TOP INDENT */

    size = (UWORD)wid * (UWORD)dep;

    p = bm->bm;
    for (i=0; i<size; i++) if(*p++) break;
    bm->top_indent = i/wid;

 /* compute vertical black depth of character */

    p = (bm->bm) + size;
    for (i=0; i<size; i++) if(*(--p)) break;

    bm->black_depth = dep - bm->top_indent - (i/wid);

    DBG2("    left_indent, top_indent  %ld, %ld\n",
                                bm->left_indent, bm->top_indent);
    DBG2("    black_width, black_depth  %ld, %ld\n",
                               bm->black_width, bm->black_depth);

}

