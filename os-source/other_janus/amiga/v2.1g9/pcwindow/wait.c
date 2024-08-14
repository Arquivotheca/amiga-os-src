
/* *** wait.c ***************************************************************
 * 
 * Little Wait Window Routines
 * This file has to go into chip memory, or else the initialized data of
 * this file has to go there.
 *
 * Copyright (C) 1986, =Robert J. Mical=
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY    Name         Description
 * ---------  -----------  --------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 16 Dec 86  =RJ Mical=   Created this file
 *
 * *************************************************************************/

#include "zaphod.h"
#include <proto/intuition.h>

#define  DBG_MAKE_WAIT_WINDOW_ENTER       1
#define  DBG_UNMAKE_WAIT_WINDOW_ENTER     1
#define  DBG_SET_WAIT_POINTER_ENTER       1

#define ELECARTSPOINT_HEIGHT		22
#define ELECARTSPOINT_XOFF			-9
#define ELECARTSPOINT_YOFF			-13

static USHORT chip ElecArtsWaitPointer[(ELECARTSPOINT_HEIGHT * 2) + 4] =
      {
      0x0000, 0x0000,

      0x0700, 0x0000,
      0x0FA0, 0x0700,
      0x3FF0, 0x0FA0,
      0x70F8, 0x3FF0,
      0x7DFC, 0x3FF8,
      0xFBFC, 0x7FF8,
      0x70FC, 0x3FF8,
      0x7FFE, 0x3FFC,
      0x7F0E, 0x3FFC,
      0x3FDF, 0x1FFE,
      0x7FBE, 0x3FFC,
      0x3F0E, 0x1FFC,
      0x1FFC, 0x07F8,
      0x07F8, 0x01E0,
      0x01E0, 0x0080,
      0x07C0, 0x0340,
      0x0FE0, 0x07C0,
      0x0740, 0x0200,
      0x0000, 0x0000,
      0x0070, 0x0020,
      0x0078, 0x0038,
      0x0038, 0x0010,

      0x0000, 0x0000,
      };

static struct NewWindow NewWaitWindow =
   {
   0, 0,      /* LeftEdge, TopEdge */
   1, 1,      /* Width, Height */
   -1, -1,    /* Detail/BlockPens */
   NULL,      /* IDCMP Flags filled in later */
   SMART_REFRESH | ACTIVATE,
   NULL,      /* FirstGadget */
   NULL,      /* Checkmark */
   NULL,      /* WindowTitle */
   NULL,      /* Screen */
   NULL,      /* SuperBitMap */
   0, 0,      /* MinWidth, MinHeight */
   0, 0,      /* MaxWidth, MaxHeight */
   WBENCHSCREEN,
   };

static struct Window *WaitWindow;

/****i* PCWindow/MakeWaitWindow ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID MakeWaitWindow()
{
#if (DEBUG1 & DBG_MAKE_WAIT_WINDOW_ENTER)
   kprintf("wait.c:       MakeWaitWindow(VOID)\n");
#endif

   if (WaitWindow = OpenWindow(&NewWaitWindow))
   {
      SetPointer(WaitWindow, ElecArtsWaitPointer, ELECARTSPOINT_HEIGHT,
                 16, ELECARTSPOINT_XOFF, ELECARTSPOINT_YOFF);
      SetWindowTitles(WaitWindow,(UBYTE *) -1, (UBYTE *)"Please wait ...");
   }
}

/****i* PCWindow/UnmakeWaitWindow ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID UnmakeWaitWindow()
{
#if (DEBUG1 & DBG_UNMAKE_WAIT_WINDOW_ENTER)
   kprintf("wait.c:       UnmakeWaitWindow(VOID)\n");
#endif

   if (WaitWindow)
   {
      CloseWindow(WaitWindow);
      WaitWindow = NULL;
   }
}
