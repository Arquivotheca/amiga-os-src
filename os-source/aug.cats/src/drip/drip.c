;/* drip.c - Execute me to compile me with SAS C 5.10
LC -b0 -cfistq -v -y -j73 drip.c
Blink FROM LIB:astartup.obj,drip.o TO drip LIBRARY LIB:amiga.lib,LIB:LC.lib ND
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>

#include <dos/dos.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#include <clib/alib_stdio_protos.h>

#include <stdlib.h>

#define MINARGS 1

UBYTE *vers = "\0$VER: drip 39.1";
UBYTE *Copyright = 
  "drip v39.1\n(c) Copyright 1993 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: drip [threshold]";

ULONG StrToInt(char *s);

char *program;
struct MsgPort *FindPort();

ULONG StrToInt(char *s)
{
    ULONG   num = 0;
    char    ch;

    while ((ch = *s++) != 0) {
	switch (ch) {
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9': 
		num = num * 10 + (ch - '0');
		break;
	    default: 
		printf ("usage: %s <decimal-number>\n", program);
		exit (10);
	}
    }

    return num;
}


void main(int argc, char **argv)
    {
    struct MsgPort *port;
    long    threshold,
            temp,
           *past,
            curr;
    char   *name;
    char   *s,
           *d;
    UBYTE  *mem;

    if(((argc)&&(argc<MINARGS))||((argc>1)&&(argv[argc-1][0]=='?')))
	{
	printf("%s\n%s\n",Copyright,usage);
	exit(RETURN_OK);
	}


    if(!argc)exit(0);

    program = *argv;
    name = "drip.port";		/* no larger than 11 chars */

    /* cause an expunge sweep */
    if(mem= AllocMem (0x07FFFFFF,0))  FreeMem(mem,0x07FFFFFF);

    if (argc < 2)
	threshold = 0;
    else
	threshold = StrToInt (argv[1]);

    port = FindPort (name);
    if (port == 0) {
	port = (struct MsgPort *) AllocMem (sizeof (*port), MEMF_CLEAR);
	if (port == 0) {
	    printf ("%s: not enough memory\n");
	    exit (10);
	}
	else {
	    AddPort (port);
	    s = &name[0];
	    port -> mp_Node.ln_Name = d = (char *) (&port -> mp_MsgList);
	    while (*s != 0) {
		*d++ = *s++;
	    }
	    *d = 0;
	    port -> mp_Node.ln_Type = NT_MSGPORT;
	    port -> mp_SigTask = (struct Task  *) AvailMem (0);
	}
    }
    past = (long *) (&port -> mp_SigTask);

    curr = AvailMem (0);
    temp = *past - curr;
    *past = curr;

    if (temp > threshold) {
	printf ("==========>> %s: lost %ld bytes\n", program, temp);
	exit (5);
    }

    if (temp < -threshold) {
	printf ("==========>> %s: gained %ld bytes\n", program, -temp);
	exit (5);
    }
}
    
