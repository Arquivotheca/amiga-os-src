#ifndef  CLIB_POTGO_PROTOS_H
#define  CLIB_POTGO_PROTOS_H
/*
**	$Id: potgo_protos.h,v 36.3 90/11/07 15:58:11 darren Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
UWORD AllocPotBits( unsigned long bits );
void FreePotBits( unsigned long bits );
void WritePotgo( unsigned long word, unsigned long mask );
#endif   /* CLIB_POTGO_PROTOS_H */
