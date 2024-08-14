#ifndef SPLINES_H
#define SPLINES_H


/*****************************************************************************/


#include <exec/types.h>
#include "mpegplayerbase.h"


/*****************************************************************************/


struct Context
{
    APTR CGfxBase;
    APTR RastPort;
    LONG OldX;
    LONG OldY;
};


VOID __asm DrawSpline(register __a4 struct Context *con,
		      register __a0 LONG x1, register __a1 LONG y1,
                      register __a2 LONG x2, register __a3 LONG y2,
                      register __a5 LONG x3, register __a6 LONG y3,
                      register __d6 LONG x4, register __d7 LONG y4);


/*****************************************************************************/


#endif /* SPLINES_H */
