#ifndef  CLIB_NONVOLATILE_PROTOS_H
#define  CLIB_NONVOLATILE_PROTOS_H

/*
**	$Id: nonvolatile_protos.h,v 40.5 93/07/30 17:13:06 vertex Exp Locker: vertex $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  EXEC_LISTS_H
#include <exec/lists.h>
#endif
#ifndef  LIBRARIES_NONVOLATILE_H
#include <libraries/nonvolatile.h>
#endif
/*--- functions in V40 or higher (Release 3.1) ---*/
APTR GetCopyNV( STRPTR appName, STRPTR itemName, long killRequesters );
void FreeNVData( APTR data );
UWORD StoreNV( STRPTR appName, STRPTR itemName, APTR data,
	unsigned long length, long killRequesters );
BOOL DeleteNV( STRPTR appName, STRPTR itemName, long killRequesters );
struct NVInfo *GetNVInfo( long killRequesters );
struct MinList *GetNVList( STRPTR appName, long killRequesters );
BOOL SetNVProtection( STRPTR appName, STRPTR itemName, long mask,
	long killRequesters );
#endif   /* CLIB_NONVOLATILE_PROTOS_H */
