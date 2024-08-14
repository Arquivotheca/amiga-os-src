#ifndef INTERNAL_CDUI_H
#define INTERNAL_CDUI_H
/*
**      $Id: cdui.h,v 40.1 93/04/15 19:00:49 vertex Exp $
**
**      Private definitions for cdui.library
**
**      (C) Copyright 1993 Commodore-Amiga, Inc.
**      All Rights Reserved
*/

/*****************************************************************************/


#define STARTUP_ANIM_PORT "Startup Animation"

/* Possible message types to send to the animation port */
#define ANIMMSG_STARTANIM   4  /* No disk to boot, do full blown animation   */
#define ANIMMSG_DOORCLOSED  2  /* Door closed, hold off anim                 */
#define ANIMMSG_RESTARTANIM 3  /* No disk to boot, animate again             */
#define ANIMMSG_BOOTING     1  /* Booting title, start boot anim             */
#define ANIMMSG_SHUTDOWN    0  /* Title booted, free animation               */
#define ANIMMSG_RED_BUTTON  5  /* Pretend red controller button was pressed  */
#define ANIMMSG_BLUE_BUTTON 6  /* Pretend blue controller button was pressed */
#define ANIMMSG_NOP         7  /* Do nothing, ignored                        */


/*****************************************************************************/


#endif  /* INTERNAL_CDUI_H */
