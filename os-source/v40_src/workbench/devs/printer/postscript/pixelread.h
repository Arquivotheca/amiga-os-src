#ifndef PIXELREAD_H
#define PIXELREAD_H


/*****************************************************************************/


#include <exec/types.h>
#include <graphics/rastport.h>
#include <graphics/view.h>


/*****************************************************************************/


struct RGB
{
    UBYTE Red;
    UBYTE Green;
    UBYTE Blue;
};


/*****************************************************************************/


BOOL OpenReader(struct RastPort *rp, struct ColorMap *cm, ULONG displayMode,
                ULONG x, ULONG y, ULONG w, ULONG h);

VOID CloseReader(VOID);

VOID ReadNextLine(UBYTE *redPlane, UBYTE *greenPlane, UBYTE *bluePlane);


/*****************************************************************************/


#endif /* PIXELREAD_H */
