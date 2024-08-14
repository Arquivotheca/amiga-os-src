
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
#include <exec/memory.h>

#define  DBG_MAKE_WAIT_WINDOW_ENTER       1
#define  DBG_UNMAKE_WAIT_WINDOW_ENTER     1
#define  DBG_SET_WAIT_POINTER_ENTER       1


struct NewWindow NewWaitWindow =
   {
   0, 0,      /* LeftEdge, TopEdge */
   1, 1,      /* Width, Height */
   -1, -1,      /* Detail/BlockPens */
   NULL,      /* IDCMP Flags filled in later */
   SMART_REFRESH | ACTIVATE,
   NULL, /* FirstGadget */
   NULL,      /* Checkmark */
   NULL,      /* WindowTitle */
   NULL,      /* Screen */
   NULL,      /* SuperBitMap */
   0, 0,      /* MinWidth, MinHeight */
   0, 0,      /* MaxWidth, MaxHeight */
   WBENCHSCREEN,
   };
struct Window *WaitWindow;


void MakeWaitWindow()
{
#if (DEBUG1 & DBG_MAKE_WAIT_WINDOW_ENTER)
   kprintf("wait.c:       MakeWaitWindow(VOID)\n");
#endif

   if (WaitWindow = OpenWindow(&NewWaitWindow))
         {
         SetWaitPointer(0, WaitWindow);
         SetWindowTitles(WaitWindow,(UBYTE *) -1, (UBYTE *)"Please wait ...");
         }
}


void UnmakeWaitWindow()
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



/* **************************************************************************
 *
 * Intuition Pointers Routines
 *    from Book 1 of the Amiga Programmers' Suite by RJ Mical
 *
 * Copyright (C) 1986, =Robert J. Mical=
 * All Rights Reserved.
 *
 * Created for Amiga developers.
 * Any or all of this code can be used in any program as long as this
 * entire notice is retained, ok?  Thanks.
 *
 * HISTORY     NAME         DESCRIPTION
 * -----------  --------------  --------------------------------------------
 * 12 Aug 86   RJ >:-{)*      Prepare (clean house) for release
 * 3 May 86    =RJ Mical=     Fix prop gadget for both 1.1 and 1.2
 * 1 Feb 86    =RJ Mical=     Created this file.
 *
 * *********************************************************************** */


#include "pointers.h"


USHORT ElecArtsWaitPointer[(ELECARTSPOINT_HEIGHT * 2) + 4] =
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


USHORT *WaitPointer = NULL;

void SetWaitPointer(pointnumber, window)
SHORT pointnumber;
struct Window *window;
{
/*???      switch (pointnumber)*/
/*???            {*/
/*???            case GCRAFT_POINTER:*/
/*???                  SetPointer(window, &GCraftWaitPointer[0], GCRAFTPOINT_HEIGHT,*/
/*???                              16, GCRAFTPOINT_XOFF, GCRAFTPOINT_YOFF);*/
/*???                  break;*/
/*???            case WBENCH_POINTER:*/
/*???                  SetPointer(window, &WBenchWaitPointer[0], WBENCHPOINT_HEIGHT,*/
/*???                              16, WBENCHPOINT_XOFF, WBENCHPOINT_YOFF);*/
/*???                  break;*/
/*???            case ELECARTS_POINTER:*/

/* I do only because my **cking ATOM program won't atomize this file.
 * It's the aargh factor, don'cha know.
 */
{
   SHORT i, i2;
   USHORT *tptr;

#if (DEBUG1 & DBG_SET_WAIT_POINTER_ENTER)
   kprintf("wait.c:       SetWaitPointer(pointnumber=0x%lx,window=0x%lx)\n",pointnumber,window);
#endif

   if (WaitPointer == NULL)
      {
      i = ((ELECARTSPOINT_HEIGHT * 2) + 4) * 2;
      WaitPointer = (USHORT *)AllocRemember(&GlobalKey, i, MEMF_CHIP);
      i = i / 2;
      if (tptr = WaitPointer)
         for (i2 = 0; i2 < i; i2++)
            *tptr++ = ElecArtsWaitPointer[i2];
      }

   SetPointer(window, WaitPointer, ELECARTSPOINT_HEIGHT,
         16, ELECARTSPOINT_XOFF, ELECARTSPOINT_YOFF);
}

/*???                  break;*/
/*???            }*/
}



