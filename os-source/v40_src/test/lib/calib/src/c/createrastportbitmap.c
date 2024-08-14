/****** CALib.library/CreateRastPortBitMap ************************************

    NAME
        CreateRastPortBitMap -- Create a fully useable RastPort/BitMap

    SYNOPSIS
        struct RastPort *CreateRastPortBitMap(long width,
                                              long height,
                                              long depth,
                                              long vectors,
                                              BOOL clip)

    FUNCTION
        Allocates a RastPort structure, Initializes it and fills in
        BitMap information and possibly Layers, for clipping. The
        structure returned is intended to be fully ready for use
        by all graphics calls that require a RastPort as a parameter.

        This function will be primarily used for double-buffering.

    INPUTS
        width   - Width of the bitmap, in pixels
        height  - Height of the bitmap, in pixels

        depth   - Depth of the BitMap, number of bitplanes
                  to allocate (1 thru 8)

        vectors - Number of vectors to allocate for AreaDraw type
                  of operations. If zero, no tmpras will be allocated.

        clip    - Setup proper clipping (layers) for RastPort (NIY)

    RESULTS
        A Fully useable RastPort will be created, the pointer to which
        will be returned, or NULL if operation failed for some reason.

    SEE ALSO
        FreeRastPortBitMap()

******************************************************************************/

// Includes
#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <graphics/rastport.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include "protos.h"

struct RastPort *CreateRastPortBitMap(long width,
                                      long height,
                                      long depth,
                                      long vectors,
                                      BOOL clip)
{
    WORD *areabuffer = NULL, *apt = NULL;
    UBYTE *ras = NULL;
    struct RastPort *rp;

    if (rp = AllocVec(sizeof(*rp), MEMF_CLEAR|MEMF_ANY))
    {
        InitRastPort(rp);
        if (rp->BitMap = AllocVec((depth > 8) ? sizeof(*rp->BitMap) + (depth - 8) * sizeof(char *)
                                              : sizeof(*rp->BitMap),
                                                                    MEMF_CLEAR|MEMF_ANY))
        {
            long i;

            InitBitMap(rp->BitMap, depth, width, height);

            // allocate depth bitplanes
            for (i = 0; i < depth; ++i)
                if (!(rp->BitMap->Planes[i] = AllocVec(RASSIZE(width, height), MEMF_CHIP|MEMF_CLEAR)))
                {   // failed if it gets here
                    FreeRastPortBitMap(rp), rp = NULL;
                    break;
                }

            if (rp) // still good?
            {
                if (vectors)
                {
                    rp->TmpRas = AllocVec(sizeof(*rp->TmpRas), MEMF_CLEAR|MEMF_PUBLIC);
                    rp->AreaInfo = AllocVec(sizeof(*rp->AreaInfo), MEMF_CLEAR|MEMF_PUBLIC);
                    rp->RP_User = areabuffer = AllocVec(vectors * 5, MEMF_CLEAR|MEMF_PUBLIC);
                    if (rp->TmpRas && rp->AreaInfo && areabuffer)
                    {
                        InitArea(rp->AreaInfo, areabuffer, vectors);
                        if (ras = AllocVec(RASSIZE(width, height), MEMF_CHIP|MEMF_CLEAR))
                        {
                            InitTmpRas(rp->TmpRas, ras, RASSIZE(width, height));
                            if (apt = AllocVec(sizeof(UWORD) * 2, MEMF_CHIP|MEMF_CLEAR))
                            {
                                apt[0] = -1;
                                apt[1] = -1;
                                SetAfPt(rp, apt, 1);
                            }
                            else
                                FreeRastPortBitMap(rp), rp = NULL;
                        }
                        else
                            FreeRastPortBitMap(rp), rp = NULL;
                    }
                    else // memory failure
                        FreeRastPortBitMap(rp), rp = NULL;
                }
            }
        }
        else
            FreeVec(rp), rp = NULL;
    }
    return rp;
}

/****** CALib.library/FreeRastPortBitMap **************************************

    NAME
        FreeRastPortBitMap -- Free a allocated RastPortBitMap

    VERSION
        1.00    16 Mar 1992 - Inception

    SYNOPSIS
        FreeRastPortBitMap(struct RastPort *rp)

    FUNCTION
        Frees the resources associated with the allocated RastPortBitMap.

    INPUTS
        rp  - RastPort allocated by CreateRastPortBitMap(). NULL is allowed.

    RESULTS
        None.

    SEE ALSO
        CreateRastPortBitMap().

******************************************************************************/

void FreeRastPortBitMap(struct RastPort *rp)
{
    if (rp)
    {
        if (rp->BitMap)
        {
            long i;

            for (i = 0; i < rp->BitMap->Depth; ++i)
                if (rp->BitMap->Planes[i])
                    FreeVec(rp->BitMap->Planes[i]);
            FreeVec(rp->BitMap);
        }

        if (rp->TmpRas)
        {
            if (rp->TmpRas->RasPtr) FreeVec(rp->TmpRas->RasPtr);
            FreeVec(rp->TmpRas);
        }

        if (rp->AreaInfo) FreeVec(rp->AreaInfo);
        if (rp->RP_User) FreeVec(rp->RP_User);  // areabuffer
        if (rp->AreaPtrn) FreeVec(rp->AreaPtrn);
        FreeVec(rp);
    }
}
