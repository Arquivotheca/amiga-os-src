#ifndef  CLIB_PHOTOCD_PROTOS_H
#define  CLIB_PHOTOCD_PROTOS_H

/*
**	$Id: PhotoCD_protos.h,v 40.1 93/11/18 17:39:42 jjszucs Exp $
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
#ifndef  LIBRARIES_PHOTOCD_H
#include <libraries/photocd.h>
#endif
/*--- functions in V40 or higher (Release 3.1) ---*/

/*   Public entries */

UBYTE *AllocPCDImageBufferA( unsigned long resolution, struct TagItem *tags );
UBYTE *AllocPCDImageBuffer( unsigned long resolution, Tag firstTag, ... );
void ClosePhotoCD( APTR pcdHandle );
void FreePCDImageBuffer( UBYTE *imageBuffer );
BOOL GetPCDImageDataA( APTR pcdHandle, UBYTE *imageBuffer,
	struct TagItem *tags );
BOOL GetPCDImageData( APTR pcdHandle, UBYTE *imageBuffer, Tag firstTag, ... );
BOOL GetPCDResolution( unsigned long resolution, UWORD *width,
	UWORD *height );
BOOL IsPhotoCD( void );
struct TagItem *ObtainPhotoCDInfoA( APTR pcdHandle, struct TagItem *tags );
struct TagItem *ObtainPhotoCDInfo( APTR pcdHandle, Tag firstTag, ... );
APTR OpenPhotoCDA( struct TagItem *tags );
APTR OpenPhotoCD( Tag firstTag, ... );
void ReleasePhotoCDInfo( struct TagItem *pcdInfo );
#endif   /* CLIB_PHOTOCD_PROTOS_H */
