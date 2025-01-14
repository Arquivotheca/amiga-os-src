
/* *** main.c ***************************************************************
 * 
 * Main Routine.
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY    Name           Description
 * -------    ------         --------------------------------------------
 * 21 jul 89   -BK         Added code to open intuition library BEFORE
 *                         the first possible call to MyAlert since
 *                         MyAlert/AutoRequest tends to blow the fuck up
 *                         otherwise. If only RJ had a brain!
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 27 Nov 86  Dieter Preiss  Added Startup code, CLI/WB parameter check
 * 23 Feb 86  =RJ Mical=     Created this file
 *
 * *************************************************************************/

#include "zaphod.h"
#include <exec/memory.h>
#include <workbench/startup.h>
#include <proto/exec.h>
#include <stdlib.h>

#define  DBG_MAIN_ENTER             1
#define  DBG_MAIN_DISPLAY_PRIORITY  1
#define  DBG_MAIN_RETURN            1
#define  DBG_FNDSTR_ENTER           0
#define  DBG_TOLOWER_ENTER          0

extern void main();

void main(argc, argv)
int argc;
char **argv;
{
   char     *arg;
   struct   WBArg    *WBArg;  
   struct   Display  *display;

#if (DEBUG1 & DBG_MAIN_ENTER)
   kprintf("Main.c:        Main(argc=%ld,argv=0x%lx)\n",argc,argv);
#endif

   /******************/
   /* Open Intuition */
   /******************/
   if((IntuitionBase=(struct IntuitionBase *)
       OpenLibrary((UBYTE *)"intuition.library",0))==NULL)
   {
      return;
   }

   /******************************/
   /* Allocate Display structure */
   /******************************/
   if ((display = (struct Display *)AllocMem(sizeof(struct Display),
         MEMF_CHIP | MEMF_CLEAR)) == NULL)
   {
      MyAlert(ALERT_NO_DISPLAY_STRUC_MEMORY, NULL);
      return;
   }

   DisplayPriority = ((struct Task *)FindTask(0))->tc_Node.ln_Pri;

#if (DEBUG3 & DBG_MAIN_DISPLAY_PRIORITY)
   kprintf("Main.c:       Display priority = %ld\n",DisplayPriority);
#endif

   /**********************/
   /* Get CLI or WB args */
   /**********************/
   if (argc)
   {
      /* Called from CLI, look for arguments */
      arg = (argc > 1) ? argv[1] : argv[0];
   } else {
      /* Called from Workbench, find the name of last icon passed
         since this is the one that called us. */
      WBArg = ((struct WBStartup *)argv)->sm_ArgList;
      WBArg += ((((struct WBStartup *)argv)->sm_NumArgs)-1);
      arg   =  WBArg->wa_Name;
   }

   /* Convert arg string to lower case */
   tolower(arg);

   /* Look for "color" in arg string */
   while (!fndstr(arg,"color") && *arg) arg++;

   /* If arg not exhausted, we found the magic word.
      So we do a color show now. */
   if (*arg) display->Modes |= (OPEN_SCREEN | COLOR_DISPLAY);

   /*****************************/
   /* Start this shooting match */
   /*****************************/
   if (InitZaphod(display))
   {
      UnmakeWaitWindow();
      DisplayTask(display);
   } else {
      UnmakeWaitWindow();
      CloseEverything();
      FreeMem(display, sizeof(struct Display));
   }

#if (DEBUG2 & DBG_MAIN_RETURN)
   kprintf("Main.c:       Main() Returns!\n");
#endif

   /* Actually, we should never return from DisplayTask() */
   exit(0);
}


/*
 * Some Aux. routines
 */

/*
 * Check for String S2 to occur in String S1
 */
 
fndstr (s1,s2)
UBYTE *s1,*s2;
/* by Dieter ... */
{
#if (DEBUG1 & DBG_FNDSTR_ENTER)
   kprintf("Main.c:       fndstr(s1=0x%lx,s2=0x%lx)\n",s1,s2);
#endif

   while (*s1 && *s2 && ( *s1 == *s2))
   {
      s1++;
      s2++;
   }
   /* return TRUE only if at end of s2 */
   return((*s2)==0);
}      

void tolower(s)
UBYTE *s;
/* by Dieter ... */
{
#if (DEBUG1 & DBG_TOLOWER_ENTER)
   kprintf("Main.c:       tolower(s=0x%lx)\n",s);
#endif

   do
      if ((*s >= 'A') && (*s <= 'Z')) 
	     *s += 0x20;
   while (*s++);
}

