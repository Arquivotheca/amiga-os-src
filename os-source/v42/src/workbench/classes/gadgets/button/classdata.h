#ifndef CLASSDATA_H
#define CLASSDATA_H

/*****************************************************************************/

#include <exec/types.h>
#include <intuition/screens.h>
#include <graphics/rastport.h>

#include "classbase.h"

#ifndef GADGETS_BUTTON_H
#include <gadgets/button.h>
#endif

/*****************************************************************************/

struct objectData
{
    struct IBox		 od_Domain;		/* The domain */
    struct IBox		 od_BDomain;		/* Border domain */
    struct IBox		 od_IDomain;		/* Image domain */
    struct TextFont	*od_TF;			/* Font to use */
    void		*od_Image;		/* Current image */
    void		**od_Array;		/* Image array */
    ULONG		 od_ISize;		/* Image size */
    UWORD		 od_Flags;		/* Control flags */
    WORD		 od_Maximum;		/* Size of array */
    WORD		 od_Previous;		/* Previous image */
    WORD		 od_Current;		/* Current image */
    LONG		 od_Pens[NUMDRIPENS];
};

/*****************************************************************************/

#define	ODF_TEXT	0x0001
#define	ODF_IMAGE	0x0002
#define	ODF_GLYPH	0x0004

#define	ODF_PUSH	0x0010

#define	ODF_LAYOUT	0x0020

#define	ODF_READONLY	0x0040

/*****************************************************************************/

LONG ASM ClassDispatcher (REG(a0) Class *cl, REG(a1) ULONG *msg, REG(a2) struct Gadget *g);

/*****************************************************************************/

#endif /* CLASSDATA_H */
