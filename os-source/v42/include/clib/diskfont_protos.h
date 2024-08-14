#ifndef  CLIB_DISKFONT_PROTOS_H
#define  CLIB_DISKFONT_PROTOS_H
/*
**	$Id: diskfont_protos.h,v 38.0 92/06/18 11:23:09 darren Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
#ifndef  DOS_DOS_H
#include <dos/dos.h>
#endif
#ifndef  DISKFONT_DISKFONT_H
#include <diskfont/diskfont.h>
#endif
struct TextFont *OpenDiskFont( struct TextAttr *textAttr );
LONG AvailFonts( STRPTR buffer, long bufBytes, long flags );
/*--- functions in V34 or higher (Release 1.3) ---*/
struct FontContentsHeader *NewFontContents( BPTR fontsLock, STRPTR fontName );
void DisposeFontContents( struct FontContentsHeader *fontContentsHeader );
/*--- functions in V36 or higher (Release 2.0) ---*/
struct DiskFont *NewScaledDiskFont( struct TextFont *sourceFont,
	struct TextAttr *destTextAttr );
#endif   /* CLIB_DISKFONT_PROTOS_H */
