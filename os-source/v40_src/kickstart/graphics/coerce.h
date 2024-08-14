#ifndef GRAPHICS_COERCE_H
#define GRAPHICS_COERCE_H
/*
**	$Id: coerce.h,v 39.3 93/02/15 11:19:40 spence Exp $
**
**	mode coercion definitions
**
**	(C) Copyright 1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

/* These flags are passed (in combination) to CoerceMode() to determine the
 * type of coercion required.
 */

/* Ensure that the mode coerced to can display just as many colours as the
 * ViewPort being coerced.
 */
#define PRESERVE_COLORS 1

/* Ensure that the mode coerced to is not interlaced. */
#define AVOID_FLICKER 2

/* Coercion should ignore monitor compatibility issues. */
#define IGNORE_MCOMPAT 4


#define BIDTAG_COERCE 1	/* Private */

#endif
