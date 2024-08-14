/* apsh_deftext.c
 * default text for the message handlers
 * written by David N. Junod
 *
 * created this file May 19, 1990
 *
 * $Id: apsh_deftext.c,v 1.4 1992/09/07 17:50:30 johnw Exp johnw $
 *
 * $Log: apsh_deftext.c,v $
 * Revision 1.4  1992/09/07  17:50:30  johnw
 * Minor changes.
 *
 * Revision 1.1  91/12/12  14:50:02  davidj
 * Initial revision
 * 
 * Revision 1.1  90/07/02  11:18:28  davidj
 * Initial revision
 *
 *
 */

#include <exec/types.h>

STRPTR apsh_text[] =
{
  "",						/*  0 */
  "%s is not an icon.",				/*  1 */
  "%s not available.",				/*  2 */
  "%s port already active.",			/*  3 */
  "port, %s, already active.",			/*  4 */
  "%s is not an IFF file",			/*  5 */
  "%1$s is not an IFF %2$s file",		/*  6 */
  "Close all windows.",				/*  7 */
  "Cmd>",					/*  8 */
  "Could not create %s.",			/*  9 */
  "Could not create port, %s.",			/* 10 */
  "Could not create object.",			/* 11 */
  "Could not create object, %s.",		/* 12 */
  "Could not create file.",			/* 13 */
  "Could not create file, %s.",			/* 14 */
  "Could not initialize %s.",			/* 15 */
  "Could not initialize %s message handler.",	/* 16 */
  "Could not lock %s.",				/* 17 */
  "Could not lock directory.",			/* 18 */
  "Could not lock directory, %s.",		/* 19 */
  "Could not lock public screen.",		/* 20 */
  "Could not lock public screen, %s.",		/* 21 */
  "Could not obtain %s.",			/* 22 */
  "Could not open %s.",				/* 23 */
  "Could not open file.",			/* 24 */
  "Could not open file, %s.",			/* 25 */
  "Could not open font, %s.",			/* 26 */
  "Could not open macro file.",			/* 27 */
  "Could not open preference file, %s.",	/* 28 */
  "Could not open screen.",			/* 29 */
  "Could not open window.",			/* 30 */
  "Could not set up timer event.",		/* 31 */
  "Could not set up HotKeys.",			/* 32 */
  "Could not start process.",			/* 33 */
  "Could not start tool.",			/* 34 */
  "Could not start tool, %s.",			/* 35 */
  "Could not write to file.",			/* 36 */
  "Could not write to file, %s.",		/* 37 */
  "Could not write to macro file.",		/* 38 */
  "CON:0/150/600/50/Command Shell/CLOSE",	/* 39 */
  "No name given for window.",			/* 40 */
  "No port name specified.",			/* 41 */
  "Not enough memory.",				/* 42 */
  "Waiting for macro return.",			/* 43 */
  "%s is disabled.",				/* 44 */
  "IoErr #%ld.",				/* 45 */
  "Invalid name tag.",				/* 46 */
  "Okay",					/* 47 */
  "Cancel",					/* 48 */
  "Continue",					/* 49 */
  "Done",					/* 50 */
  "Abort",					/* 51 */
  "Quit",					/* 52 */
  "Unnamed",					/* 53 */
  "Syntax Error: %s",				/* 54 */
  "Unknown command",				/* 55 */
  "%s Command Shell",				/* 56 */
  "User request aborted",			/* 57 */
  NULL
};
