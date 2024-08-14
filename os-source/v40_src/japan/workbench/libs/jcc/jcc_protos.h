#ifndef  CLIB_JCC_PROTOS_H
#define  CLIB_JCC_PROTOS_H

/*
**	$Id: jcc_protos.h,v 39.1 92/12/12 15:14:44 kaori Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif
#ifndef  JAPAN_JCC_H
#include <japan/jcc.h>
#endif

/*--- functions in V39 or higher (Release 3) ---*/
struct JConversionCodeSet *AllocConversionHandleA( struct TagItem *taglist );
struct JConversionCodeSet *AllocConversionHandle( Tag tag1, ... );
void FreeConversionHandle( struct JConversionCodeSet *jcc );
LONG GetJConversionAttrsA( struct JConversionCodeSet *jcc,
	struct TagItem *taglist );
LONG GetJConversionAttrs( struct JConversionCodeSet *jcc, Tag tag1, ... );
void SetJConversionAttrsA( struct JConversionCodeSet *jcc,
	struct TagItem *taglist );
void SetJConversionAttrs( struct JConversionCodeSet *jcc, Tag tag1, ... );
LONG IdentifyCodeSet( struct JConversionCodeSet *jcc, UBYTE *inbuf,
	long length );
LONG JStringConvertA( struct JConversionCodeSet *jcc, UBYTE *inbuf,
	UBYTE *outbuf, long incode, long outcode, long length,
	struct TagItem *taglist );
LONG JStringConvert( struct JConversionCodeSet *jcc, UBYTE *inbuf,
	UBYTE *outbuf, long incode, long outcode, long length, Tag tag1,
	... );
LONG HanToZen( struct JConversionCodeSet *jcc, UBYTE *inbuf, UBYTE *outbuf,
	long incode, long outcode, long length );
LONG ZenToHan( struct JConversionCodeSet *jcc, UBYTE *inbuf, UBYTE *outbuf,
	long incode, long outcode, long length );
void EightToSeven( LONG *p1, LONG *p2 );
void SevenToEight( LONG *p1, LONG *p2 );
#endif   /* CLIB_JCC_PROTOS_H */
