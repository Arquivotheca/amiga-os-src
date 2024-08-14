/* -----------------------------------------------------------------------
 * utilities.h   telnet
 *
 * $Locker:  $
 *
 * $Id: utilities.c,v 1.1 91/01/15 18:01:09 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inetx/src/telnet/RCS/utilities.c,v 1.1 91/01/15 18:01:09 bj Exp $
 *
 * $Log:	utilities.c,v $
 * Revision 1.1  91/01/15  18:01:09  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

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

#define	TELOPTS
#include <arpa/telnet.h>
#include <sys/types.h>

#include <ctype.h>

#include "general.h"

#include "ring.h"

#include "externs.h"

FILE	*NetTrace = 0;		/* Not in bss, since needs to stay */

/* -------------------------------------------
 * upcase()
 *
 *	Upcase (in place) the argument.
 * ---------------------------------------------
 */

void
upcase( register char *argument)
{
	register int c;

	while ((c = *argument) != 0) 
		{
		if (islower(c)) 
			{
			*argument = toupper(c);
			}
		argument++;
		}
}

/* ------------------------------------------------
 * SetSockOpt()
 *
 * Compensate for differences in 4.2 and 4.3 systems.
 * -------------------------------------------------
 */

int
SetSockOpt(int fd, int level, int option, int yesno)
{
#ifndef	NOT43
	return setsockopt(fd, level, option,(char *)&yesno, sizeof yesno);
#else	/* NOT43 */
	if (yesno == 0) 		/* Can't do that in 4.2! */
		{
		fprintf(stderr, "Error: attempt to turn off an option 0x%x.\n",option);
		return -1;
		}
	return setsockopt(fd, level, option, 0, 0);
#endif	/* NOT43 */
}

/* -------------------------------------------------------------------
 * The following are routines used to print out debugging information.
 * --------------------------------------------------------------------
 */

void
Dump(char direction, char *buffer, int length)
{
#define BYTES_PER_LINE	32

/* #define min(x,y)	((x<y)? x:y)  */

	char *pThis;
	int offset;

	offset = 0;

	while (length) 
		{
		/* print one line */
		fprintf(NetTrace, "%c 0x%x\t", direction, offset);
		pThis = buffer;
		buffer = buffer+min(length, BYTES_PER_LINE);
		while (pThis < buffer) 
			{
		fprintf(NetTrace, "%.2x", (*pThis)&0xff);
		pThis++;
		}
	fprintf(NetTrace, "\n");
	length -= BYTES_PER_LINE;
	offset += BYTES_PER_LINE;
	if (length < 0) 
		{
		return;
		}
	/* find next unique line */
	}
}


/* --------------------------------------- */
/*VARARGS*/
void
printoption(char *direction, char *fmt, int option, int what)
{
	if (!showoptions)
		return;
	fprintf(NetTrace, "%s ", direction+1);
	if (fmt == doopt)
		fmt = "do";
	else if (fmt == dont)
		fmt = "dont";
	else if (fmt == will)
		fmt = "will";
	else if (fmt == wont)
		fmt = "wont";
	else
		fmt = "???";
	if (option < (sizeof telopts/sizeof telopts[0]))
		fprintf(NetTrace, "%s %s", fmt, telopts[option]);
	else
		fprintf(NetTrace, "%s %d", fmt, option);
	if (*direction == '<') 
		{
		fprintf(NetTrace, "\r\n");
		return;
		}
	fprintf(NetTrace, " (%s)\r\n", what ? "reply" : "don't reply");
}

/* --------------------------------------- */
void
printsub(char *direction, char *pointer, int length)
		/* direction  "<" or ">"               */
		/* pointer;  where suboption data sits */
		/* length;   length of suboption data  */
{
    
	char tmpbuf[SUBBUFSIZE];
	int minlen = min(length, sizeof tmpbuf);

	if (showoptions) 
		{
		fprintf(NetTrace, "%s suboption ",
				(direction[0] == '<')? "Received":"Sent");
		switch (pointer[0]) 
			{
			case TELOPT_TTYPE:
				fprintf(NetTrace, "Terminal type ");
				switch (pointer[1]) 
					{
					case TELQUAL_IS:
						memcpy(tmpbuf, pointer+2, minlen);
						tmpbuf[minlen-1] = 0;
						fprintf(NetTrace, "is %s.\n", tmpbuf);
						break;
					case TELQUAL_SEND:
						fprintf(NetTrace, "- request to send.\n");
						break;
					default:
						fprintf(NetTrace,"- unknown qualifier %d (0x%x).\n",
							pointer[1], pointer[1]);
					}
				 break;
			default:
				fprintf(NetTrace, "Unknown option %d (0x%x)\n",
						pointer[0], pointer[0]);
			}
		}
}
