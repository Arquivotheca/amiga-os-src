#ifndef CLIB_MACROS_H
#define CLIB_MACROS_H
/*
** 	$Id: macros.h,v 36.0 90/11/30 15:51:33 eric Exp $
**
**	C prototypes
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#define MAX(a,b)    ((a)>(b)?(a):(b))
#define MIN(a,b)    ((a)<(b)?(a):(b))
#define ABS(x)      ((x<0)?(-(x)):(x))

#endif	/* CLIB_MACROS_H */
