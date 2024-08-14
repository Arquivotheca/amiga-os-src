#ifndef  CLIB_MATHTRANS_PROTOS_H
#define  CLIB_MATHTRANS_PROTOS_H
/*
**	$Id: mathtrans_protos.h,v 1.2 90/11/07 15:55:43 mks Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
FLOAT SPAtan( FLOAT parm );
FLOAT SPSin( FLOAT parm );
FLOAT SPCos( FLOAT parm );
FLOAT SPTan( FLOAT parm );
FLOAT SPSincos( FLOAT *cosResult, FLOAT parm );
FLOAT SPSinh( FLOAT parm );
FLOAT SPCosh( FLOAT parm );
FLOAT SPTanh( FLOAT parm );
FLOAT SPExp( FLOAT parm );
FLOAT SPLog( FLOAT parm );
FLOAT SPPow( FLOAT power, FLOAT arg );
FLOAT SPSqrt( FLOAT parm );
FLOAT SPTieee( FLOAT parm );
FLOAT SPFieee( FLOAT parm );
/*--- functions in V31 or higher (Release 1.1) ---*/
FLOAT SPAsin( FLOAT parm );
FLOAT SPAcos( FLOAT parm );
FLOAT SPLog10( FLOAT parm );
#endif   /* CLIB_MATHTRANS_PROTOS_H */
