/*
 *  $Id$
 *
 *  label.image
 *  Bitmap Rendering Module
 *  Copyright (C) 1994 Commodore-Amiga, Inc.
 *  All Rights Reserved
 *  Unauthorized Duplication or Distribution Prohibited
 *
 */

/*
$Log$
*/

/*****************************************************************************
 *
 *  Includes
 *
 *****************************************************************************/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/imageclass.h>
#include <intuition/classusr.h>
#include <intuition/classes.h>
#include <intuition/cghooks.h>
#include <graphics/displayinfo.h>
#include <graphics/gfxmacros.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/macros.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "classbase.h"
#include "classdata.h"

/*****************************************************************************
 *
 *  renderBitmap()    -   render bitmap
 *
 *****************************************************************************/

BOOL renderBitmap(
    struct ClassLib *ClassBase,
    struct RastPort *rastPort,
    struct BitMap *pBitmap,
    UWORD offsetX, UWORD offsetY,
    UWORD width, UWORD height
)
{

#ifdef DEBUG
    kprintf("renderBitmap(): Entry\n");
#endif /* DEBUG */

    BltBitMapRastPort(
        pBitmap,
        0, 0,
        rastPort,
        offsetX, offsetY,
        width, height,
        MINTERM_COPY
    );

    return TRUE;

}
