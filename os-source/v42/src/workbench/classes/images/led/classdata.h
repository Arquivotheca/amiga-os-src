#ifndef CLASSDATA_H
#define CLASSDATA_H

/*****************************************************************************/

#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>
#include <intuition/screens.h>

#include "classbase.h"

/*****************************************************************************/

struct objectData
{
    struct DrawInfo	*od_DrawInfo;
    LONG		 od_BackgroundPen;
    LONG		 od_TextPen;
    ULONG		 od_Flags;		/* LED_Colon, LED_Negative, LED_Signed */
    WORD		 od_Pairs;		/* LED_Pairs */
    WORD		 od_Values[8];		/* LED_Values */

    /* Cached calculations */
    UBYTE		 od_LitSegments[16][7];
    struct Rectangle	 od_Segments[10];
    WORD		 od_HSW;		/* Horizontal segment width */
    WORD		 od_VSW;		/* Vertical segment width */
    WORD		 od_HCell;		/* Cell horizontal width */
    WORD		 od_VCell;		/* Cell vertical width */
};

/*****************************************************************************/

#define	ODF_COLON	0x0001
#define	ODF_NEGATIVE	0x0002
#define	ODF_SIGNED	0x0004

/*****************************************************************************/

LONG ASM ClassDispatcher (REG(a0) Class *cl, REG(a1) ULONG *msg, REG(a2) struct Image *im);

/*****************************************************************************/

#endif /* CLASSDATA_H */
