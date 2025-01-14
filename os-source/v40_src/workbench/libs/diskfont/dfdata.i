**
**	$Header: V38:src/workbench/libs/diskfont/RCS/dfdata.i,v 38.2 91/03/24 17:12:55 kodiak Exp $
**
**      diskfont.library library data definition
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/semaphores.i"

*------ LIB_FLAGS ----------------------------------------------------
	BITDEF	LIB,EXPUNGED,7	; an expunge is pending for the library

 STRUCTURE  DFL,LIB_SIZE	; Standard library node
	APTR	dfl_Segment	; library segment
	WORD	dfl_PrivOpenCnt	; private open count (public always 0)
	APTR	dfl_SysBase	; SysBase
	APTR	dfl_UtilityBase	; UtilityBase
	APTR	dfl_GfxBase	; GfxBase
	APTR	dfl_DOSBase	; DOSBase
	APTR	dfl_DiskfontBase ; DiskfontBase
    APTR    dfl_BulletBase		; BulletBase
	WORD	dfl_NumLFonts	; number of disk fonts loaded
	STRUCT	dfl_DiskFonts,LH_SIZE ; list of all open disk fonts
    STRUCT  dfl_BSemaphore,SS_SIZE	; Bullet semaphore
	UWORD   dfl_XDPI	; default DPI for YSize conversion
	UWORD   dfl_YDPI
	UWORD   dfl_DotSizeX	; default DotSize
	UWORD   dfl_DotSizeY
	LABEL	dfl_SIZEOF
