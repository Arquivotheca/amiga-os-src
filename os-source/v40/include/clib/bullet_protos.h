#ifndef  CLIB_BULLET_PROTOS_H
#define  CLIB_BULLET_PROTOS_H

/*
**	$Id: bullet_protos.h,v 38.0 92/06/19 11:20:38 darren Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef  UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif
#ifndef  DISKFONT_GLYPH_H
#include <diskfont/glyph.h>
#endif
struct GlyphEngine *OpenEngine( void );
void CloseEngine( struct GlyphEngine *glyphEngine );
ULONG SetInfoA( struct GlyphEngine *glyphEngine, struct TagItem *tagList );
ULONG SetInfo( struct GlyphEngine *glyphEngine, Tag tag1, ... );
ULONG ObtainInfoA( struct GlyphEngine *glyphEngine, struct TagItem *tagList );
ULONG ObtainInfo( struct GlyphEngine *glyphEngine, Tag tag1, ... );
ULONG ReleaseInfoA( struct GlyphEngine *glyphEngine,
	struct TagItem *tagList );
ULONG ReleaseInfo( struct GlyphEngine *glyphEngine, Tag tag1, ... );
#endif   /* CLIB_BULLET_PROTOS_H */
