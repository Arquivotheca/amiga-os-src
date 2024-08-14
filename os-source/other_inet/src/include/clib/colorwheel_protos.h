#ifndef  CLIB_COLORWHEEL_PROTOS_H
#define  CLIB_COLORWHEEL_PROTOS_H

/*
**	$Id: colorwheel_protos.h,v 39.1 92/07/21 15:42:37 vertex Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  GADGETS_COLORWHEEL_H
#include <gadgets/colorwheel.h>
#endif
/*--- functions in V39 or higher (Release 3) ---*/

/* Public entries */

void ConvertHSBToRGB( struct ColorWheelHSB *hsb, struct ColorWheelRGB *rgb );
void ConvertRGBToHSB( struct ColorWheelRGB *rgb, struct ColorWheelHSB *hsb );
#endif   /* CLIB_COLORWHEEL_PROTOS_H */
