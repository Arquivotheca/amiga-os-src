#ifndef  CLIB_TRANSLATOR_PROTOS_H
#define  CLIB_TRANSLATOR_PROTOS_H
/*
**	$Id: translator_protos.h,v 36.1 90/11/07 15:31:59 eric Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
LONG Translate( STRPTR inputString, long inputLength, STRPTR outputBuffer,
	long bufferSize );
#endif   /* CLIB_TRANSLATOR_PROTOS_H */
