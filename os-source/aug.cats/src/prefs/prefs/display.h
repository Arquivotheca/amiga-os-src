/**************************************************************************
 *
 *	display.h    -	Defines for Workbench Screen Editor
 *
 *	Copyright 1989, Commodore-Amiga, Inc.
 *
 *	Revision History:
 *
 *	15-Sep-89		Created this file.  Peter Cherna
 *
 **************************************************************************/

/*------------------------------------------------------------------------*/

#define DE_DEFAULTW	1
#define DE_DEFAULTH	2
#define DE_WIDTH	3
#define DE_HEIGHT	4
#define DE_AUTOSCROLL	5
#define DE_SAVE		6
#define DE_USE		7
#define DE_CANCEL	8
#define DE_MODEVIEW	9
#define DE_PROPERTY	10
#define DE_COLORS	11

/*------------------------------------------------------------------------*/

#define OPEN_MENU	1
#define	SAVEAS_MENU	2
#define QUIT_MENU	3
#define RESETALL_MENU	4
#define LASTSAVED_MENU	5
#define RESTORE_MENU	6
#define SAVEICONS_MENU	7

/*------------------------------------------------------------------------*/

/*  Window dimensions.  Note that MINWINWIDTH is chosen so that the
    sizing gadget can never land in the sketchpanel, which would
    overwrite its imagery: */

#define WINLEFT		0
#define WINTOP		8
#define WINWIDTH	612
#define WINHEIGHT	158
#define MINWINWIDTH	1
#define MINWINHEIGHT	1
#define MAXWINWIDTH	0
#define MAXWINHEIGHT	0

#define ZOOMLEFT	WINLEFT
#define ZOOMTOP		WINTOP
#define ZOOMWIDTH	200
#define ZOOMHEIGHT	10

/*------------------------------------------------------------------------*/
