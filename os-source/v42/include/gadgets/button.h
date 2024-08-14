#ifndef	GADGETS_BUTTON_H
#define	GADGETS_BUTTON_H

/*
**	$VER: button.h 42.1 (10.1.94)
**	Includes Release 42.1
**
**	Definitions for the button BOOPSI class
**
**	(C) Copyright 1994 Commodore-Amiga Inc.
**	All Rights Reserved
*/

/*****************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

#ifndef INTUITION_GADGETCLASS_H
#include <intuition/gadgetclass.h>
#endif

/*****************************************************************************/

/* Additional attributes defined by the button.gadget class */
#define BUTTON_Dummy		(TAG_USER+0x04000000)

#define	BUTTON_PushButton	(BUTTON_Dummy+1)
    /* (BOOL) Indicate whether button stays depressed when clicked */

#define	BUTTON_Glyph		(BUTTON_Dummy+2)
    /* (struct Image *) Indicate that image is to be drawn using BltTemplate */

#define	BUTTON_Array		(BUTTON_Dummy+3)
    /* (LONG) Indicate that text or image pointer is an array */

#define	BUTTON_TextPen		(BUTTON_Dummy+5)
    /* (LONG) Pen to use for text (-1 uses TEXTPEN) */

#define	BUTTON_FillPen		(BUTTON_Dummy+6)
    /* (LONG) Pen to use for fill (-1 uses FILLPEN) */

#define	BUTTON_FillTextPen	(BUTTON_Dummy+7)
    /* (LONG) Pen to use for fill (-1 uses FILLTEXTPEN) */

#define	BUTTON_BackgroundPen	(BUTTON_Dummy+8)
    /* (LONG) Pen to use for fill (-1 uses BACKGROUNDPEN) */

#define	BUTTON_Current		(BUTTON_Dummy+9)
    /* (LONG) Indicate which item in the array is current */

/*****************************************************************************/

#endif /* GADGETS_BUTTON_H */
