#ifndef CLASSDATA_H
#define CLASSDATA_H

/*****************************************************************************/

#include <exec/types.h>
#include <intuition/screens.h>
#include <graphics/rastport.h>

#include "classbase.h"

/*****************************************************************************/

#define	MAX_BUT		5

/*****************************************************************************/

struct Glyph
{
    WORD	 w, h;
    UWORD	*data;
};

/*****************************************************************************/

struct objectData
{
    struct Glyph	*od_Glyphs;		/* The glyphs to use */
    struct IBox		 od_Buttons[MAX_BUT];	/* The coordinates of each of the buttons */
    ULONG		 od_ID[MAX_BUT];	/* The ID */
    UWORD		 od_Mode;		/* Current mode */
    UWORD		 od_OMode;		/* Previous mode */
    UWORD		 od_Flags;		/* Are we paused? */
    UWORD		 od_NumButtons;		/* Number of buttons */

    /* Used with animation controls */
    struct Glyph	*od_PlayPause;		/* The play/pause glyph */
    struct Gadget	*od_Active;		/* Active gadget */
    struct Gadget	*od_ScrollG;		/* Scroller gadget */
    LONG		 od_CurrentFrame;	/* Current frame */
    LONG		 od_Frames;		/* Number of frames */
    LONG		 od_OldFrame;		/* Frame at activation frame */
    UWORD		 od_OldMode;		/* Mode at activation time */
    UWORD		 od_AMode;		/* Mode activated by */
    WORD		 od_Width;		/* Width of the object */
};

#define	ODF_PAUSED	0x0001
#define	ODF_PCHANGE	0x0002
#define	ODF_TAPE	0x0004

/*****************************************************************************/

LONG ASM ClassDispatcher (REG(a0) Class *cl, REG(a1) ULONG *msg, REG(a2) struct Gadget *g);

/*****************************************************************************/

#endif /* CLASSDATA_H */
