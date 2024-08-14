
/*#define MAINC_MONO*/

/* ***************************************************************************
 * 
 * Dummy Main Routine.	All of this needs to go somewhere else.
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name		Description
 * -------	------		--------------------------------------------
 * 23 Feb 86	=RJ Mical=	Created this file
 *
 * **************************************************************************/

#include "zaphod.h"

main () {
    UBYTE inkey;
    UBYTE * bufptr;
    SHORT i;
    struct Display  display;

    bufptr = (UBYTE *) & display;
    for (i = 0; i < sizeof (struct Display); i++)
	*bufptr++ = 0;
    DisplayPriority = (FindTask (0)) -> tc_Node.ln_Pri;

#ifdef MAINC_MONO
    i = 0;
#else
    i = OPEN_SCREEN | COLOR_DISPLAY;
#endif

#ifdef DIAG

    kprintf ("\n\n\n*>>>  Zaphod Alpha Release  <<<*\n\n");
    kprintf ("What flavor of display is your pleasure (M or C) : ");
    inkey = ZGetChar ();
    switch (inkey) {
	case 'M': 
	    kprintf ("onochrome it shall be!");

	    i = NULL;

	    break;
	default: 
	    kprintf ("??? C");
	case 'C': 
	    kprintf ("olor display coming up!");
	    i = OPEN_SCREEN | COLOR_DISPLAY;
	    break;
    }

    kprintf ("\nWant VERBOSE (Y or N) : ");
    inkey = ZGetChar ();
    switch (inkey) {
	case 'Y': 
	    kprintf ("es, I want maximum verbosity.");
	    i |= VERBOSE;
	    break;
	default: 
	    kprintf ("??? Huh?  You must mean N");
	case 'N': 
	    kprintf ("ah, keep a lid on it.");
	    break;
    }

    kprintf ("\n\n");
    kprintf ("Display structure at %lx\n",&display);
#endif

    display.Modes = i;
    InitZaphod (&display);
    DisplayTask (&display);

 /* Should never return from DisplayTask() */
    exit (0);
}




