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
/*****************************************************************************
    ImageTools.c
    
    Mitchell/Ware Systems           Version 1.00            08/29/89

    Tools for Creating Image Structures.

_____________________________________________________________________________

    struct Image *BitMapToImage (key, bitmap, left, top) 
        
        key     - memory will be allocated using AllocRemember
        bitmap  - bitmap to be converted to an Image
        left    \__ location (where it will be 
        top     /   placed in the destination)

        BitMapToImage will 'optimize' the image- that is, it will make use of
        PlanePick and PlaneOnOff.

        BitMapToImage will return the Image pointer, or NULL if failed.
*****************************************************************************/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include "Tools.h"
#include "Handy.h"

#define PUB     (MEMF_PUBLIC | MEMF_CLEAR)
#define CHIP    (MEMF_CHIP | MEMF_CLEAR)

BOOL _AllZeroBits(p, s) /* return TRUE if bits are all off */
UBYTE *p;
int s;
{
    while (s--)
        if (*p++)
            return FALSE;

    return TRUE;
}

BOOL _AllOneBits(p, s) /* return TRUE if bits are all on */
UBYTE *p;
int s;
{
    while (s--)
        if (*p++ != 0xFF)
            return FALSE;

    return TRUE;
}

struct Image *BitMapToImage(key, bm, left, top)
struct Remember **key;
struct BitMap *bm;
int left, top;
{
    struct Image *im;
    USHORT i;
    int size = bm->BytesPerRow * bm->Rows;
    BOOL z, zeros[8], o, ones[8];
    UBYTE *data;
    
    ifnot(im = AllocRemember(key, sizeof(*im), PUB))
        return NULL;
    
    for (im->Depth = i = 0; i < bm->Depth; ++i)
    {
        zeros[i] = z = _AllZeroBits(bm->Planes[i], size);
        ones[i] = o = _AllOneBits(bm->Planes[i], size);
        if (!z && !o)
        {
            im->PlanePick |= 1 << i;
            ++im->Depth;
        }
        else if (o)
            im->PlaneOnOff |= 1 << i;
    }
    
    if (im->Depth && !(im->ImageData = AllocRemember(key, size * im->Depth, CHIP)))
        return NULL;
    
    im->LeftEdge = left;
    im->TopEdge = top;
    im->Width = bm->BytesPerRow * 8;
    im->Height = bm->Rows;

    for (data = im->ImageData, i = 0; i < bm->Depth; ++i)
        if (!zeros[i] && !ones[i])
        {
            memcpy(data, bm->Planes[i], size);
            data += size;
        }

    return im;
}
