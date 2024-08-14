/****************************************************************************
 **									   **
 **  pref.h								   **
 **									   **
 **  Confidential Information: Commodore-Amiga, Inc.			   **
 **  Copyright (c) 1989 Commodore-Amiga, Inc.				   **
 **									   **
 ****************************************************************************
 *
 * SOURCE CONTROL
 *
 * $Header:  $
 *
 * $Locker:  $
 *
 ****************************************************************************/

#ifndef PREF_H
#define PREF_H

#include <exec/types.h>
#include <graphics/text.h>

/* ----- Graphic Printer Preferences ------------------------------------- */

/* Print aspect */
#define ASPECT_HORIZ		0x00
#define ASPECT_VERT		0x01

/* Print shade */
#define SHADE_BW		0x00
#define SHADE_GREYSCALE		0x01
#define SHADE_COLOR		0x02
#define GREY_SCALE2		0x03	/* for use with hi-res monitor */

/* PrintiImage */
#define IMAGE_POSITIVE		0x00
#define IMAGE_NEGATIVE		0x01

#define CORRECT_0		0x00	/* no color correction */
#define CORRECT_RED		0x01	/* color correct red shades */
#define CORRECT_GREEN		0x02	/* color correct green shades */
#define CORRECT_BLUE		0x04	/* color correct blue shades */

#define IGNORE_DIMENSIONS	0x0000  /* ignore max width/height settings */
#define BOUNDED_DIMENSIONS	0x0001  /* use max w/h as boundaries */
#define ABSOLUTE_DIMENSIONS	0x0002  /* use max w/h as absolutes */
#define PIXEL_DIMENSIONS	0x0003  /* use max w/h as prt pixels */
#define MULTIPLY_DIMENSIONS	0x0004  /* use max w/h as multipliers */

#define ORDERED_DITHERING	0x0000  /* ordered dithering */
#define HALFTONE_DITHERING	0x0001  /* halftone dithering */
#define FLOYD_DITHERING		0x0002  /* Floyd-Steinberg dithering */

#define CENTER_IMAGE		0x0001	/* center image on paper */
#define INTEGER_SCALING		0x0002	/* force integer scaling */
#define ANTI_ALIAS		0x0004	/* anti-alias image */


typedef struct PGraphic {
    LONG  pp_Reserved[4];		/* System reserved */
    UWORD pp_Aspect;			/* Default: ASPECT_HORIZ */
    UWORD pp_Shade;			/* Default: SHADE_BW */
    UWORD pp_Image;			/* Default: IMAGE_POSITIVE */
    WORD pp_Threshold;			/* Default: 7 */
    UBYTE pp_ColorCorrect;		/* Default: CORRECT_0 */
    UBYTE pp_Dimensions;		/* Default: IGNORE_DIMENSIONS */
    UBYTE pp_Dithering;			/* Default: ORDERED_DITHERING */
    UWORD pp_GraphicFlags;		/* Misc. flags, Default 0 */
    UBYTE pp_PrintDensity;		/* Print density 1 - 7 */
    UWORD pp_PrintMaxWidth;
    UWORD pp_PrintMaxHeight;
    UBYTE pp_PrintXOffset;
    UBYTE pp_PrintYOffset;
} PREF;


#define ENV_NAME "ENV:Sys/printergfx.prefs"
#define ARC_NAME "ENVARC:Sys/printergfx.prefs"
#define PRE_DIR	 "Sys:Prefs/Presets"
#define PRE_NAME "PrinterGfx.pre"
#define REQ_LOAD "Load PrinterGfx Preferences"
#define REQ_SAVE "Save PrinterGfx Preferences"
#define F_LENGTH (sizeof(PREF))

#define ENV_TITLE	"PrinterGfx Preferences"
#define TASK_NAME	"pgfx_pref_task"
#define TOOL_NAME	"SYS:Prefs/PrinterGfx"

#define ENV_DEF		{0L, 0L, 0L, 0L, ASPECT_HORIZ, SHADE_BW, IMAGE_POSITIVE, 7, CORRECT_0, IGNORE_DIMENSIONS, ORDERED_DITHERING, 0, 1, 0, 0, 0, 0}

#endif



