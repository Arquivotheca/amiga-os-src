/************************************************************************
 *									*
 *	Copyright (c) 1989 Enhanced Software Technologies, Inc.		*
 *			   All Rights Reserved				*
 *									*
 *	This software and/or documentation is protected by U.S.		*
 *	Copyright Law (Title 17 United States Code).  Unauthorized	*
 *	reproduction and/or sales may result in imprisonment of up	*
 *	to 1 year and fines of up to $10,000 (17 USC 506).		*
 *	Copyright infringers may also be subject to civil liability.	*
 *									*
 ************************************************************************
 */

/*		Gadget image data (for all gadgets)		*/

/* These images are always 2 bitplanes of data (even though
 * they both are not neccessarily used).
 */

#include "standard.h"

#if !LATTICE
#define chip	/* define away the chip declaration */
#endif

/*------------------------------------------------------------------*/
/*			Line Box  64 x 12									    */
/*------------------------------------------------------------------*/

static USHORT chip box064_12data[] = {	/* 2 planes */
    /* Plane 0 */
    0x0000, 0x0000, 0x0000, 0x0000,		/* Line across */
    0x0000, 0x0000, 0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0003,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,		/* Line across */
    /* Plane 1 */
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,		/* Line across */
    0xC000, 0x0000, 0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000		/* Line across */
};

static USHORT chip hbox064_12data[] = {/* 1 plane */
    /* Plane 0 */
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,		/* Line across */
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
    0x0000, 0x0000, 0x0000, 0x0000,		/* Line Across */

	/* Plane 1 */
    0x0000, 0x0000, 0x0000, 0x0000,		/* Line Across */
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF		/* Line across */
};



/*------------------------------------------------------------------*/
/*			Line Box  96 x 12									    */
/*------------------------------------------------------------------*/

static USHORT chip box096_12data[] = {		/* 2 Planes */
    /* Plane 0 */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		/* Line across */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0003,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,		/* Line across */
    /* Plane 1 */
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,		/* Line across */
    0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000		/* Line Across */
};

static USHORT chip hbox096_12data[] = {	/* 1 Plane */
    /* Plane 0 */
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,		/* Line Across */
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		/* Line Across */
	/* Plane 1 */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		/* Line Across */
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF		/* Line Across */
};



/*------------------------------------------------------------------*/
/*			Line Box  160 x 12									    */
/*------------------------------------------------------------------*/

static USHORT chip box160_12_data[] = {		/* 2 Planes */
    /* Plane 0 */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0001,		/* Line across */

    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0003,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0003,

    0x7FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF,			/* Line across */

    /* Plane 1 */
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFE,		/* Line across */

    0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000,
    0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000,

    0x8000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000		/* Line across */
};


static USHORT chip hbox160_12_data[] = {
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF,			/* Line across */

    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFC,

    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000,			/* Line across */

	/* Plane 2 */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000,			/* Line across */

    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF,
    0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF,

    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF			/* Line across */
};



/********************************************************************/
/********************************************************************/
/********************************************************************/

struct Image box_064_12_image_1 = {
    0, 0,			/* LeftEdge, TopEdge */
    64, 12,			/* Width, Height */
    2,				/* Depth */
    box064_12data,		/* ImageData */
    0x03, 0x00,			/* PlanePick, PlaneOnOff */
    NULL			/* NextImage */
};

struct Image hbox_064_12_image_1 = {
    0, 0,
    64, 12,
    2,
    hbox064_12data,
    0x03, 0x00,
    NULL
};


struct Image box_096_12_image_1 = {
    0, 0,			/* LeftEdge, TopEdge */
    96, 12,			/* Width, Height */
    2,				/* Depth */
    box096_12data,		/* ImageData */
    0x03, 0x00,			/* PlanePick, PlaneOnOff */
    NULL			/* NextImage */
};

struct Image hbox_096_12_image_1 = {
    0, 0,
    96, 12,
    2,
    hbox096_12data,
    0x03, 0x00,
    NULL
};

struct Image box_096_12_image_2 = {
    -4, -2,			/* LeftEdge, TopEdge */
    96, 12,			/* Width, Height */
    2,				/* Depth */
    box096_12data,		/* ImageData */
    0x03, 0x00,			/* PlanePick, PlaneOnOff */
    NULL			/* NextImage */
};

struct Image hbox_096_12_image_2 = {
    -4, -2,
    96, 12,
    2,
    hbox096_12data,
    0x03, 0x00,
    NULL
};



struct Image box_160_12_image_1 = {
    0, 0,			/* LeftEdge, TopEdge */
    160, 12,			/* Width, Height */
    2,				/* Depth */
    box160_12_data,		/* ImageData */
    0x03, 0x00,			/* PlanePick, PlaneOnOff */
    NULL			/* NextImage */
};

struct Image hbox_160_12_image_1 = {
    0, 0,
    160, 12,
    2,
    hbox160_12_data,
    0x03, 0x00,
    NULL
};

