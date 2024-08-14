	IFND	GRAPHICS_RAWDISPLAYINFO_I
GRAPHICS_RAWDISPLAYINFO_I	SET	1
**
**	$Filename: graphics/displayinfo_internal.i $
**	$Release: $
**	$Revision: 39.0 $
**	$Date: 91/08/21 17:09:32 $
**
**	
**
**	(C) Copyright 1985,1986,1987,1988,1989 Commodore-Amiga, Inc.
**	    All Rights Reserved
**

        IFND    EXEC_TYPES_I
        include "exec/types.i"
        ENDC

        IFND    GRAPHICS_GFX_I
        include "graphics/gfx.i"
        ENDC
	
        IFND    UTILITY_TAGITEM_I
        include "utility/tagitem.i"
        ENDC

	STRUCTURE RecordNode,0
	APTR rcn_Succ
	APTR rcn_Pred
	APTR rcn_Child
	APTR rcn_Parent
	LABEL rcn_SIZEOF

	STRUCTURE DisplayInfoRecord,rcn_SIZEOF
	UWORD	  rec_MajorKey
	UWORD	  rec_MinorKey
	STRUCT	  rec_Tag,ti_SIZEOF
	ULONG     rec_Control
	APTR	  rec_get_data
	APTR	  rec_set_data
	STRUCT	  rec_ClipOScan,ra_SIZEOF
	STRUCT	  rec_reserved,8
	LABEL	  rec_SIZEOF


	ENDC	; GRAPHICS_RAWDISPLAYINFO_I
