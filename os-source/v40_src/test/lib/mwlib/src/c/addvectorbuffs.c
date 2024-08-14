/*
    Copyright (c) 1989 Mitchell/Ware Systems, Inc.

    May be used by Commodore-Amiga, Inc. for in-house purposes only.
    No part of this program, or any parts or modifications thereof, either
    in source-code or object-code, or library form, may be used by
    Commodore, or any of its employees, either indepently, or in conjunction
    with Commordore, as, or as a part of , any public-domain or commercial
    product, program, or any type of software, teaching aid, etc. without
    the prior written permission of Mitchell/Ware Systems.

    This copyright notice must not be removed from the document of which
    it is attached.
*/
/****** MWLib.library/AddVectorBuffs ******************************************

    NAME
        AddVectorBuffs -- Allocate a vector table for RastPort

    VERSION
        1.06    22-Oct-1988
        1.07    11 Jan 1992 - Doc cleanup

    AUTHOR
        Copyright © Mitchell/Ware Systems

    SYNOPSIS
        BOOL AddVectorBuffs(struct Remember **key,
                            struct RastPort *rp, long num)

    FUNCTION
        Adds Vector Buffers to RastPort for AreaDraw and PolyDraw
        operations. Also inits AreaPtrn if NULL.

    INPUTS
        key - pointer to a pointer to a Remember structure. Memory
              can be librated via a FreeRemember() call.

        rp  - Pointer to the RastPort to add the vectors to.

        num - Number of vectors to add.


    RESULTS
        Returns TRUE if successful.

    NOTE
        Intuition.library must be available, as should graphics.library.

    SEE ALSO
        intuition.library/FreeRemember()

*****************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <graphics/rastport.h>
#include <graphics/gfxmacros.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

#define CHIP    (MEMF_CLEAR | MEMF_CHIP)
#define PUB     (MEMF_CLEAR | MEMF_PUBLIC)

BOOL AddVectorBuffs(struct Remember **key, struct RastPort *rp, long num)
{
    struct TmpRas *tmpras;
    struct AreaInfo *areainfo;
    WORD *areabuffer, *apt;

    tmpras = AllocRemember(key, sizeof(*tmpras), PUB);
    areainfo = AllocRemember(key, sizeof(*areainfo), PUB);
    areabuffer = AllocRemember(key, num * 5, PUB);
    if (!tmpras || !areainfo || !areabuffer)
        return FALSE;
    InitArea(areainfo, areabuffer, num);
    rp->AreaInfo = areainfo;
    rp->TmpRas = tmpras;
    tmpras->Size = rp->BitMap->BytesPerRow * rp->BitMap->Rows;
    if (!(tmpras->RasPtr = AllocRemember(key, tmpras->Size, CHIP)))
            return FALSE;

    if (!rp->AreaPtrn && (apt = AllocRemember(key, sizeof(WORD) * 2, CHIP)))
    {
        apt[0] = -1;
        apt[1] = -1;
        SetAfPt(rp, apt, 1);
    }

    return TRUE;
}
