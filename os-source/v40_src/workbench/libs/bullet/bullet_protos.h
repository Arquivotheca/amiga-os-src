#ifndef  CLIB_BULLET_PROTOS_H
#define  CLIB_BULLET_PROTOS_H
/*
**	$Id: bullet_protos.h,v 7.0 91/03/19 18:35:29 kodiak Exp $
**
**	C prototypes
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
/* "bullet.library" */
#ifndef  UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif
#ifndef  LIBRARIES_GLYPH_H
#include <libraries/glyph.h>
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
ULONG GetGlyphMap( struct GlyphEngine *glyphEngine, unsigned long glyphCode,
	struct GlyphMap **glyphMap );
#endif   /* CLIB_BULLET_PROTOS_H */
