/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (C) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                             BBS: (919) 382-8265    */
/* | o  | ||   Dave Baker      Alan Beale         Jim Cooper                 */
/* |  . |//    Jay Denebeim    Bruce Drake        Gordon Keener              */
/* ======      John Mainwaring Andy Mercier       Jack Rouse                 */
/*             John Toebes     Mary Ellen Toebes  Doug Walker   Mike Witcher */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**	(C) Copyright 1989 Commodore-Amiga, Inc.
 **	    All Rights Reserved
**/

/*----------------------------------------------------------------------*/
/* Command: AddBuffers                                                  */
/* Author:  Jay Denebeim                                                */
/* Change History:                                                      */
/*  Date    Person        Action                                        */
/* -------  ------------- -----------------                             */
/* 19MAR89  John Toebes   Initial Creation                              */
/* 24MAR89  Jay Denebeim  Initial Coding                                */
/* 27OCT89  Jay Denebeim  Re-write for new DOS                          */
/* 05DEC89  John Toebes   Added ability to print out result             */
/* 19JAN91  John Toebes   Fixed - B11170 - disallow addbuffers foo      */
/* Notes:                                                               */
/*----------------------------------------------------------------------*/

#include "internal/commands.h"
#include "addbuffers_rev.h"

#define MSG_CURRENT  "%s has %ld buffers\n"
#define MSG_INVALID_DEVICE "Invalid device or volume name '%s'\n"

#define TEMPLATE    "DRIVE/A,BUFFERS/N" CMDREV
#define OPT_DRIVE    0
#define OPT_BUFS     1
#define OPT_COUNT    2

long cmd_addbuffers(void)
{
   struct Library *SysBase = (*((struct Library **) 4));
   struct DosLibrary *DOSBase;
   char              *dev;
   long              opts[OPT_COUNT], bufs, rc;
   struct RDargs     *rdargs;

   rc = RETURN_FAIL;
   if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER)))
   {
      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);

      if (rdargs == NULL)
      {
         PrintFault(IoErr(), NULL);
      }
      else
      {
         rc = RETURN_OK;
         dev   = (char *)opts[OPT_DRIVE];

         /*-------------------------------------------------------------*/
         /* They must specify either a device that ends in a colon or   */
         /* Just a plain empty string (indicating the current volume)   */
         /* This catches cases like 'addbuffers foo'                    */
         /*-------------------------------------------------------------*/
         if (*dev && (dev[strlen(dev)-1] != ':'))
         {
            rc = RETURN_FAIL;
            VPrintf(MSG_INVALID_DEVICE, opts);
         }
         /*-------------------------------------------------------------*/
         /* If they specified any buffers, add them to the device       */
         /*-------------------------------------------------------------*/
         else if (opts[OPT_BUFS] &&
            (AddBuffers( dev, *(long *)opts[OPT_BUFS] ) == 0))
         {
               PrintFault( IoErr(), NULL );
         }
         else {
            /*----------------------------------------------------------*/
            /* Now display the number of buffers that the drive now     */
            /* has If we can.                                           */
            /*----------------------------------------------------------*/
            if ((bufs = AddBuffers(dev, 0)) <= 0) {
               /*-------------------------------------------------------*/
               /* If we failed to add the buffers, check to see if we   */
               /* are dealing with an old style handler so that we      */
               /* supress the (misleading) message                      */
               /*-------------------------------------------------------*/
               if (bufs == 0)
                  PrintFault( IoErr(), NULL );
            }
            else {
               /*-------------------------------------------------------*/
               /* Well, it worked tell them how many buffers there are  */
               /*-------------------------------------------------------*/
               opts[OPT_BUFS] = bufs;
               VPrintf(MSG_CURRENT, opts);
            }
         }

         FreeArgs(rdargs);
      }
      CloseLibrary((struct Library *)DOSBase);
   }
   else
   {
      OPENFAIL;
   }
   return(rc);
}
