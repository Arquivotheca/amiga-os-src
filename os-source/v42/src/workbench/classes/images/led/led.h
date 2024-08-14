#ifndef	IMAGES_LED_H
#define	IMAGES_LED_H

/*
**	$VER: led.h 42.1 (10.1.94)
**	Includes Release 42.1
**
**	Definitions for the LED BOOPSI image class
**
**	(C) Copyright 1994 Commodore-Amiga Inc.
**	All Rights Reserved
*/

/*****************************************************************************/

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

/*****************************************************************************/

#define LED_Dummy		(TAG_USER+0x04000000)

#define	LED_Pairs		(LED_Dummy+1)
    /* (WORD) Number of digit pairs (1-8) */

#define	LED_Values		(LED_Dummy+2)
    /* (WORD *) Array of digit pairs.  Must be LED_Pairs in size. */

#define	LED_Colon		(LED_Dummy+3)
    /* (BOOL) Colon on or off */

#define	LED_Negative		(LED_Dummy+4)
    /* (BOOL) Negative sign on or off */

#define	LED_Signed		(LED_Dummy+5)
    /* (BOOL) Leave space for negative sign */

/*****************************************************************************/

#endif /* IMAGES_LED_H */
