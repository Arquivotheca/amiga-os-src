/****** MWLib/SpriteTools *****************************************************
    NAME
        SpriteTools -- Tools that aid in the creation of simple sprites

    COPYRIGHT
        Mitchell/Ware Systems, Inc.    Version 1.00                24 Nov 1990

    SYNOPSIS
        struct SimpleSprite *BitMapToSprite(key, bitmap, left, top);

******* MWLib/SpriteTools/BitMapToSprite **************************************
    NAME
        BitMapToSprite -- Convert a bitmap to a SimpleSprite.

    SYNOPSIS
        struct SimpleSprite
            *BitMapToSprite(struct Remember **key,
                            struct BitMap *bitmap, short left, short top);

    FUNCTION
        Converts the given bitmap into a SimpleSprite. The given BitMap must
        be the exact intended size of the target sprite. Extraneous bitplanes
        are ignored.

    INPUTS
        key     - The key used for all memory allocations for this sprite
        bitmap  - The bitmap to be converted
        left    - Initial X position of sprite
        top     - Initial Y position of sprite

    RESULT
        If successful, returns a pointer to a SimpleSprite structure, else
        NULL if there were a problem (like incorrect BitMap dimensions or
        out-of-memory conditions).

    NOTES

    SEE ALSO
        MWLib/ImageTools/BitMapToImage

******************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>
#include <graphics/sprite.h>

struct SimpleSprite *
    BitMapToSprite(struct Remember **key, struct BitMap *bm, short left, short top)
{
    struct SimpleSprite *ss = NULL;
    UWORD *data = NULL, *p1 = NULL, *p2 = NULL;
    int i, j;

    // Check for proper BitMap conditions
    if (bm->BytesPerRow == 2)   // This may change for AA or AAA machines...
    {
        // Allocate data structures
        if (ss = (struct SimpleSprite *) AllocRemember(key, sizeof(*ss), MEMF_CLEAR | MEMF_PUBLIC))
        {
            if (data = (UWORD *) AllocRemember(key, bm->BytesPerRow * bm->Rows * 2 + sizeof(UWORD) * 4, MEMF_CHIP | MEMF_CLEAR))
            {
                // We now have all the data structures. Set them up.
                if (bm->Depth >= 1) p1 = (UWORD *) bm->Planes[0];
                if (bm->Depth >= 2) p2 = (UWORD *) bm->Planes[1];
                data[0] = left; data[1] = top;
                for (i = 0, j = 2; i < bm->Rows; ++i, j += 2)
                {
                    if (p1) data[j] = p1[i];
                    if (p2) data[j+1] = p2[i];
                }
                ss->posctldata = data;
                ss->height = bm->Rows;
                ss->x = left; ss->y = top;
            }
            else // data allocation failed
                ss = NULL;  // We don't dare deallocate - other important data may
                            //  be hanging off the key.
        }
    }
    return ss;
}
