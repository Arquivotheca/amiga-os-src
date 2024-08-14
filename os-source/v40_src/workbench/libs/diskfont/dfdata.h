/*
**	$Header: V38:src/workbench/libs/diskfont/RCS/dfdata.h,v 38.2 91/03/24 17:12:43 kodiak Exp $
**
**      diskfont.library library data definition
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
*/

#include	<exec/types.h>
#include	<exec/nodes.h>
#include	<exec/lists.h>
#include	<exec/ports.h>
#include	<exec/libraries.h>
#include	<exec/semaphores.h>
#include	<graphics/gfxbase.h>

/*----- LIB_FLAGS --------------------------------------------------*/
#define	LIBF_EXPUNGED	0x80	/* an expunge is pending for the library */

struct	DiskfontLibrary {
    struct Library dfl_L;		/* Standard library node */
    APTR    dfl_Segment;		/* library segment */
    WORD    dfl_PrivOpenCnt;		/* private open count (public is 0) */
    struct Library *dfl_SysBase;	/* SysBase */
    struct Library *dfl_UtilityBase;	/* UtilityBase */
    struct GfxBase *dfl_GfxBase;	/* GfxBase */
    struct Library *dfl_DOSBase;	/* DOSBase */
    struct Library *dfl_DiskfontBase;	/* DiskfontBase */
    struct Library *dfl_BulletBase;	/* BulletBase */
    WORD    dfl_NumLFonts;		/* number of disk fonts loaded */
    struct List dfl_DiskFonts;		/* list of all open disk fonts */
    struct SignalSemaphore dfl_BSemaphore; /* Bullet semaphore */
    UWORD   dfl_XDPI;			/* default DPI for YSize conversion */
    UWORD   dfl_YDPI;
    UWORD   dfl_DotSizeX;		/* default DotSize */
    UWORD   dfl_DotSizeY;
};
