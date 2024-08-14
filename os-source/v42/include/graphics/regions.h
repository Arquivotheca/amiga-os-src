#ifndef	GRAPHICS_REGIONS_H
#define	GRAPHICS_REGIONS_H
/*
**	$Id: regions.h,v 42.0 93/06/16 11:10:51 chrisg Exp $
**
**	
**
**	(C) Copyright 1985,1986,1987,1988,1989 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef GRAPHICS_GFX_H
#include <graphics/gfx.h>
#endif

struct RegionRectangle
{
    struct RegionRectangle *Next,*Prev;
    struct Rectangle bounds;
};

struct Region
{
    struct Rectangle bounds;
    struct RegionRectangle *RegionRectangle;
};

#endif	/* GRAPHICS_REGIONS_H */
