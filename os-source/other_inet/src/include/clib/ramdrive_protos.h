#ifndef  CLIB_RAMDRIVE_PROTOS_H
#define  CLIB_RAMDRIVE_PROTOS_H

/*
**	$Id: ramdrive_protos.h,v 36.3 90/11/07 15:53:27 darren Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
/*--- functions in V34 or higher (Release 1.3) ---*/
STRPTR KillRAD0( void );
/*--- functions in V36 or higher (Release 2.0) ---*/
STRPTR KillRAD( unsigned long unit );
#endif   /* CLIB_RAMDRIVE_PROTOS_H */
