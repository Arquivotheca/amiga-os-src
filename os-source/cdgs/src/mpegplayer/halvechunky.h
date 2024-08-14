#ifndef HALVECHUNKY_H
#define HALVECHUNKY_H


/*****************************************************************************/


#include <exec/types.h>
#include "mpegplayerbase.h"


/*****************************************************************************/


void ASM HalveChunky(REG(a0) STRPTR src,
                     REG(a1) STRPTR dst,
                     REG(d0) LONG width,
                     REG(d1) LONG height);

void ASM WCP(REG(a6) struct Library *gfxBase,
             REG(a0) struct RastPort *rp,
             REG(d0) LONG xstart,
             REG(d1) LONG ystart,
             REG(d2) LONG xstop,
             REG(d3) LONG ystop,
             REG(a2) UBYTE *array,
             REG(d4) LONG bpr);


/*****************************************************************************/


#endif /* HALVECHUNKY_H */
