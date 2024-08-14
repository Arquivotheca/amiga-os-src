#ifndef SP_MAGNIFY_H
#define SP_MAGNIFY_H


/*****************************************************************************/


#include <exec/types.h>
#include <graphics/gfx.h>

#include "ieutils.h"


/*****************************************************************************/


VOID ASM MagnifyBitMap(REG(a0) struct BitMap *,
                       REG(a1) struct BitMap *,
                       REG(d0) UBYTE,
                       REG(d1) UBYTE,
                       REG(d2) SHORT,
                       REG(d3) SHORT);


/*****************************************************************************/


#endif /* SP_MAGNIFY_H */
