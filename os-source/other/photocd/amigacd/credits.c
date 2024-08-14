/*** credits.c ***************************************************************
 *
 *  $Id: credits.c,v 1.1 94/03/08 13:52:05 jjszucs Exp $
 *
 *  Photo CD Player for Amiga CD
 *  Credits ("Easter Egg") Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:	credits.c,v $
 * Revision 1.1  94/03/08  13:52:05  jjszucs
 * Initial revision
 * 
*/

/****************************************************************************
 *                                                                          *
 *  Includes                                                                *
 *                                                                          *
 ****************************************************************************/

/* Amiga includes */
#include <exec/types.h>

#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>

#include <libraries/lowlevel.h>

#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/debug_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>

/* Local includes */
#include "global.h"
#include "display.h"
#include "interface.h"

/****************************************************************************
 *                                                                          *
 *  Local Definitions                                                       *
 *                                                                          *
 ****************************************************************************/

/* Dimensions of credits page */
#define CREDITS_WIDTH   381
#define CREDITS_HEIGHT  356

/****************************************************************************
 *                                                                          *
 *  showCredits()  -    show credits                                        *
 *                                                                          *
 ****************************************************************************/

void showCredits(struct appContext *appContext)
{

    struct RastPort *rastport;
    WORD x, y;

    /* Hide thumbnail cursor */
    thumbnailCursor(appContext, appContext->ac_Selection, FALSE);

    /* Fetch display rastport */
    rastport=appContext->ac_DisplayWindow->RPort;

    /* Erase display panel */
    EraseRect(rastport,
        0, 0,
        appContext->ac_DisplayWindow->Width-1,
        appContext->ac_DisplayWindow->Height-1);

    /* Output credits glyph */
    x=(appContext->ac_DisplayWindow->Width/2)-(CREDITS_WIDTH/2);
    y=0;
#ifdef DEBUG
    kprintf("showCredits(): Displaying credits at (%ld, %ld)\n",
        x, y);
#endif /* DEBUG */
    putGlyph(
        appContext,
        GLYPH_CREDITS,
        rastport,
        (appContext->ac_DisplayWindow->Width/2)-(CREDITS_WIDTH/2),
        (appContext->ac_DisplayWindow->Height/2)-(CREDITS_HEIGHT/2)
    );

    /* Unload credits glyph */
    unloadGlyph(appContext, GLYPH_CREDITS);

}
