/*** memory.c ***************************************************************
 *
 *  $Id$
 *
 *  Photo CD Player for Amiga CD
 *  Low-Memory Handler Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log$
*/

/****************************************************************************
 *                                                                          *
 *  Includes                                                                *
 *                                                                          *
 ****************************************************************************/

/* Amiga includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/interrupts.h>

#include <clib/exec_protos.h>
#include <clib/debug_protos.h>

#include <pragmas/exec_pragmas.h>

/* Local includes */
#include "global.h"
#include "image.h"

/****************************************************************************
 *                                                                          *
 *  memHandlerCode()    -   memory handler                                  *
 *                                                                          *
 ****************************************************************************/

UBYTE __asm __interrupt __saveds memHandlerCode(
    register __a0 struct MemHandlerData *pMemHandlerData,
    register __a1 struct appContext *appContext
)
{

    struct imageNode *imageNode;
    int nGlyph;

#ifdef DEBUG
    kprintf("memHandlerCode(): Entry\n");
#endif /* DEBUG */

    /* Loop through images */
    imageNode=(struct imageNode *) appContext->ac_ImageList.mlh_Head;
    while (imageNode->in_Node.mln_Succ) {

        /* If thumbnail loaded and not locked for this image ... */
        if (imageNode->in_Thumbnail && !imageNode->in_ThumbnailLock) {

#ifdef DEBUG
            kprintf("   Unloading thumbnail %ld\n", imageNode->in_Number);
#endif /* DEBUG */

            /* Unload thumbnail */
            unloadThumbnail(appContext, imageNode);

            /* Return, with status indicating that exec.library
               should re-try allocation */
            return MEM_TRY_AGAIN;

        }

        imageNode=(struct imageNode *) imageNode->in_Node.mln_Succ;

    }

    /* Loop through glyphs */
    for (nGlyph=0; nGlyph<GLYPH_COUNT; nGlyph++) {

        /* If glyph is loaded ... */
        if (appContext->ac_Glyphs[nGlyph]) {

#ifdef DEBUG
            kprintf("   Unloading glyph %ld\n", nGlyph);
#endif /* DEBUG */

            /* Unload glyph */
            unloadGlyph(appContext, nGlyph);

            /* Return, with status indicating that exec.library
               should re-try allocation */
            return MEM_TRY_AGAIN;

        }

    }

    /* Return, indicating that all available memory has been released;
       allocation is likely to fail at this point */
    return MEM_DID_NOTHING;

}
