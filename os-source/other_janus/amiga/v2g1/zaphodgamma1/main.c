
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
 * 27 Nov 86  Dieter Preiss  Added Startup code, CLI/WB parameter check
 * 23 Feb 86  =RJ Mical=     Created this file
 *
 * *************************************************************************/

#include "zaphod.h"
#include <workbench/startup.h>


main(argc, argv)
int argc;
char **argv;
{
	char  *arg;
	SHORT i;
	struct WBArg *WBArg;  
	struct Display *display;

/*???kprintf("Zaphod prerelease 10:27p 27 July 1988\n");*/

	if ((display = (struct Display *)AllocMem(sizeof(struct Display),
			MEMF_CHIP | MEMF_CLEAR)) == NULL)
		{
		Alert(ALERT_NO_MEMORY, NULL);
		exit(-1);
		}

	DisplayPriority = (FindTask(0))->tc_Node.ln_Pri;


	if (argc)
		{
		/*
		 * Called from CLI, look for arguments
		 */
		arg = (argc > 1) ? argv[1] : argv[0];
		}
	else
		{
		/*
		 * Called from Workbench, find the name of last icon passed,
		 * since this is the one that called us.
		 */
		WBArg = ((struct WBStartup *)argv) -> sm_ArgList;
		WBArg += ((((struct WBStartup *)argv) -> sm_NumArgs) - 1 );
		arg =  WBArg -> wa_Name;
		}


	/*
	 * Convert arg string to lower case
	 */
	tolower(arg);

	/*
	 * Look for "color" in arg string
	 */
	while (!fndstr(arg,"color") && *arg) arg++;

	/*
	 * If arg not exhausted, we found the magic word.
	 * So we do a color show now.
	 */
	if (*arg) display->Modes |= (OPEN_SCREEN | COLOR_DISPLAY);


	if (InitZaphod(display))
		{
		UnmakeWaitWindow();
		DisplayTask(display);
		}
	else
		{
		UnmakeWaitWindow();
		CloseEverything();
		FreeMem(display, sizeof(struct Display));
		}

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
	while (*s1 && *s2 && ( *s1 == *s2))
		{
		s1++;
		s2++;
		}
	/* return TRUE only if at end of s2 */
	return((*s2)==0);
}		



tolower(s)
UBYTE *s;
/* by Dieter ... */
{
	do
		if ((*s >= 'A') && (*s <= 'Z')) *s += 0x20;
	while (*s++);
}

