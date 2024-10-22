#ifndef  CLIB_CIA_PROTOS_H
#define  CLIB_CIA_PROTOS_H

/*
**	$Id: cia_protos.h,v 1.7 90/07/19 13:14:03 rsbx Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  EXEC_INTERRUPTS_H
#include <exec/interrupts.h>
#endif
#ifndef  EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif
struct Interrupt *AddICRVector( struct Library *resource, long iCRBit,
	struct Interrupt *interrupt );
void RemICRVector( struct Library *resource, long iCRBit,
	struct Interrupt *interrupt );
WORD AbleICR( struct Library *resource, long mask );
WORD SetICR( struct Library *resource, long mask );
#endif   /* CLIB_CIA_PROTOS_H */
