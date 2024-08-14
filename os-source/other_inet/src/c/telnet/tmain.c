/* -----------------------------------------------------------------------
 * tmain.c    telnet      bj
 *
 * this is the "real" main() function that is called from the 'main'
 * in sys_amiga.c
 *
 * $Locker:  $
 *
 * $Id: tmain.c,v 1.1 91/01/15 18:02:20 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inetx/src/telnet/RCS/tmain.c,v 1.1 91/01/15 18:02:20 bj Exp $
 *
 * $Log:	tmain.c,v $
 * Revision 1.1  91/01/15  18:02:20  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */
 
/* #define DEBUG 1  */

#ifdef DEBUG
#define DB(x,y)    printf("DEBUG: %s: %s\n",(x),(y)) ;
#else
#define DB(x,y)  ;
#endif

/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


#include <sys/types.h>

#include "ring.h"

#include "externs.h"
#include "defines.h"

/*
 * Initialize variables.
 */

void
tninit( void )
{

DB( "tninit", "into" ) ;

	init_sys();

	init_terminal();

	init_network();
	
	init_telnet();

	init_3270();
}


/* =============================================================
 * main.  Parse arguments, invoke the protocol or command parser
 * =============================================================
 */


int
tmain( int argc, char **argv)
{
	DB("tmain", "into" ) ;

	tninit();		/* Clear out things */

	DB("tmain", "TerminalSaveState()" ) ;

	TerminalSaveState();

	prompt = argv[0];
	
	DB( "tmain", "while(argv...)") ;
	
	while ((argc > 1) && (argv[1][0] == '-')) 
		{
		if (!strcmp(argv[1], "-d")) 
			{
			debug = 1;
			} 
		else if (!strcmp(argv[1], "-n")) 
			{
			if ((argc > 1) && (argv[2][0] != '-')) 	/* get file name */
				{
				NetTrace = fopen(argv[2], "w");
				argv++;
				argc--;
				if (NetTrace == NULL) 
					{
					NetTrace = stdout;
					}
				}
			}
		else 
			{
			if (argv[1][1] != '\0') 
				{
				fprintf(stderr, "Unknown option *%s*.\n", argv[1]);
				}
			}
		argc--;
		argv++;
		}
		

	if (argc != 1) 
		{
		DB("tmain", "if(argc != 1)") ;		
		DB("tmain", "1. setjmp(toplevel) attempt...") ;
		if (setjmp(toplevel) != 0)
			{
			DB("tmain", "1. setjmp(toplevel) failed") ;
			Exit(0);
			}
		DB("tmain", "1. setjmp(toplevel) success") ;
		if (tn(argc, argv) == 1) 
			{
			DB("tmain", "tn(argc,argv) == 1)") ;
			Exit(0);
			} 
		else 
			{
			Exit(1);
			}
		}


	DB("tmain", "2. setjmp(toplevel)") ;

	(void) setjmp(toplevel);

	DB("tmain", "for(;;)") ;

	for (;;) 
		{
#ifndef TN3270
		DB("tmain", "ifndef TN3270 - command(1)" ) ;
		command(1);
#else	/* !defined(TN3270) */
		if (!shell_active) 
			{
			DB("tmain", "shell_active - command(1)" ) ;
			command(1);
			} 
		else 
			{
#ifdef TN3270
			shell_continue();
#endif	/* defined(TN3270) */
			}
#endif	/* !defined(TN3270) */
		}
}
