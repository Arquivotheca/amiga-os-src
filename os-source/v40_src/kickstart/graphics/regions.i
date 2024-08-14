	IFND	GRAPHICS_REGIONS_I
GRAPHICS_REGIONS_I	SET	1
**
**	$Id: regions.i,v 39.0 91/08/21 17:12:28 chrisg Exp $
**
**	
**
**	(C) Copyright 1985,1986,1987,1988,1989 Commodore-Amiga, Inc.
**	    All Rights Reserved
**

    IFND    EXEC_TYPES_I
    include 'exec/types.i'
    ENDC

   IFND  GRAPHICS_GFX_I
   include  "graphics/gfx.i"
   ENDC

    STRUCTURE   Region,0
      STRUCT   rg_bounds,ra_SIZEOF
      APTR  rg_RegionRectangle
   LABEL    rg_SIZEOF

   STRUCTURE   RegionRectangle,0
      APTR  rr_Next
      APTR  rr_Prev
      STRUCT   rr_bounds,ra_SIZEOF
   LABEL    rr_SIZEOF

	ENDC	; GRAPHICS_REGIONS_I
