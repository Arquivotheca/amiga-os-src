#ifndef CLASSDATA_H
#define CLASSDATA_H

/*****************************************************************************/

#include <exec/types.h>
#include <intuition/screens.h>
#include <graphics/rastport.h>

#ifndef	GADGETS_TABS_H
#include <gadgets/tabs.h>
#endif

#include "classbase.h"

/*****************************************************************************/

typedef struct tagWTabLabel
{
    STRPTR		 tl_Label;		/* Label */
    WORD		 tl_Pens[MAX_TL_PENS];	/* Pens */
    struct IBox		 tl_Domain;		/* Domain of the label */
    WORD		 tl_LabelLength;	/* Label length */
    WORD		 tl_Offset;		/* Label offset */

} WTabLabel, *WTabLabelP;

/*****************************************************************************/

struct objectData
{
    struct IBox		 od_Domain;		/* The domain */
    struct TextFont	*od_TF;			/* Font to use */
    WTabLabelP		 od_TLP;		/* TabLabel array */
    WORD		 od_Maximum;		/* Size of array */
    WORD		 od_Current;		/* Current image */
    WORD		 od_Previous;		/* Previous image */
    UWORD		 od_Flags;		/* Control freak flags */
    WORD		 od_Hit;		/* Current hit */
};

/*****************************************************************************/

#define	ODF_LAYOUT	0x0001
#define	ODF_MAXWIDTH	0x0002

/*****************************************************************************/

LONG ASM ClassDispatcher (REG(a0) Class *cl, REG(a1) ULONG *msg, REG(a2) struct Gadget *g);

/*****************************************************************************/

#endif /* CLASSDATA_H */
