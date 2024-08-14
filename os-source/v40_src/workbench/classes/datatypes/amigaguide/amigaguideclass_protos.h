#ifndef  CLIB_AMIGAGUIDECLASS_PROTOS_H
#define  CLIB_AMIGAGUIDECLASS_PROTOS_H
/*
**	$Id: amigaguideclass_protos.h,v 39.1 92/06/19 04:10:35 davidj Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  DOS_DOS_H
#include <dos/dos.h>
#endif
#ifndef  DOS_DOSEXTENS_H
#include <dos/dosextens.h>
#endif
#ifndef  INTUITION_CLASSES_H
#include <intuition/classes.h>
#endif
#ifndef  INTUITION_CLASSUSR_H
#include <intuition/classusr.h>
#endif
/*--- functions in V39 or higher (distributed as Release 3.0) ---*/

Class *ObtainEngine( void );
LONG LoadXRef( BPTR lock, STRPTR name );
void ExpungeXRef( void );
BPTR AddPathEntries( BPTR path, STRPTR *argptr );
BPTR CopyPathList( BPTR p );
void FreePathList( BPTR p );
ULONG ParsePathString( STRPTR line, STRPTR *argv, unsigned long max );
BPTR LockE( BPTR p, STRPTR name, long mode );
BPTR OpenE( BPTR p, STRPTR name, long mode );
BPTR SetCurrentDirE( BPTR p, STRPTR name );
STRPTR GetAGString( long id );
ULONG AddAGHostA( struct Hook *h, STRPTR name, struct TagItem *attrs );
LONG RemoveAGHostA( APTR handle, struct TagItem *attrs );
#endif   /* CLIB_AMIGAGUIDECLASS_PROTOS_H */
