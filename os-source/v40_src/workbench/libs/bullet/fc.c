/* fc.c */
/* Copyright (C) Agfa Compugraphic, 1989. All rights reserved. */

#include <exec/types.h>
#include <exec/nodes.h>
#include "port.h"
#include "cgif.h"


/*----------------------*/
/*     FCis_equal       */
/*----------------------*/

/*  Return truth value of "FONTCONTEXT f == FONTCONTEXT g". */

GLOBAL BOOLEAN
FCis_equal(f, g)
    FONTCONTEXT *f, *g;
{
    UBYTE *p, *q;
    UWORD i;

    p = (UBYTE*)f;
    q = (UBYTE*)g;
    for(i=0; i<sizeof(FONTCONTEXT); i++)
    {
        if(*p++ != *q++) return FALSE;
    }

#if 0
    if (f->font_id      != g->font_id)       return FALSE;
    if (f->point_size   != g->point_size)    return FALSE;
    if (f->set_size     != g->set_size)      return FALSE;
    if (f->shear.x      != g->shear.x)       return FALSE;
    if (f->shear.y      != g->shear.y)       return FALSE;
    if (f->rotate.x     != g->rotate.x)      return FALSE;
    if (f->rotate.y     != g->rotate.y)      return FALSE;
    if (f->xres         != g->xres)          return FALSE;
    if (f->yres         != g->yres)          return FALSE;
    if (f->xspot        != g->xspot)         return FALSE;
    if (f->yspot        != g->yspot)         return FALSE;
    if (f->xbold        != g->xbold)         return FALSE;
    if (f->ybold        != g->ybold)         return FALSE;
    if (f->ssnum        != g->ssnum)         return FALSE;
    if (f->format       != g->format)        return FALSE;
#endif
    return TRUE;
}

/*-------------------*/
/*     FCcopy_fc     */
/*-------------------*/

/*  Copy FONTCONTEXT f to FONTCONTEXT g. */

GLOBAL VOID
FCcopy_fc(f, g)
    FONTCONTEXT *f, *g;
{
    *g = *f;
#if 0
    g->font_id      = f->font_id;
    g->point_size   = f->point_size;
    g->set_size     = f->set_size;
    g->shear.x      = f->shear.x;
    g->shear.y      = f->shear.y;
    g->rotate.x     = f->rotate.x;
    g->rotate.y     = f->rotate.y;
    g->xres         = f->xres;
    g->yres         = f->yres;
    g->xspot        = f->xspot;
    g->yspot        = f->yspot;
    g->xbold        = f->xbold;
    g->ybold        = f->ybold;
    g->ssnum        = f->ssnum;
    g->format       = f->format;
#endif
}

