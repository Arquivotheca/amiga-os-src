/**************************************************************************
 *
 *	overscandisplay.h    -	Defines for Overscan Editor's Display
 *
 *	Copyright 1989, Commodore-Amiga, Inc.
 *
 *	Revision History:
 *
 *	15-Sep-89		Created this file.  Peter Cherna
 *
 **************************************************************************/

/*------------------------------------------------------------------------*/

#define GAD_SAVE	1
#define GAD_USE		2
#define GAD_CANCEL	3
#define GAD_EDITTXT	4
#define GAD_EDITSTD	5
#define GAD_MONITORS 	6

/*------------------------------------------------------------------------*/

/*  Window dimensions.  Note that MINWINWIDTH is chosen so that the
    sizing gadget can never land in the sketchpanel, which would
    overwrite its imagery: */

#define WINLEFT		0
#define WINTOP		8
#define WINWIDTH	308
#define WINHEIGHT	155		/*  InnerHeight */
#define MINWINWIDTH	1
#define MINWINHEIGHT	1
#define MAXWINWIDTH	0
#define MAXWINHEIGHT	0

#define ZOOMLEFT	WINLEFT
#define ZOOMTOP		WINTOP
#define ZOOMWIDTH	200
#define ZOOMHEIGHT	10

/*------------------------------------------------------------------------*/
