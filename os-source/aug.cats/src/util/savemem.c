;/* savemem.c - Execute me to compile me with SAS C 5.10
LC -b1 -cfistq -v -y -j73 savemem.c
Blink FROM LIB:c.o,savemem.o TO savemem LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef LATTICE
int CXBRK(void)  { return(0); }  /* Disable Lattice CTRL/C handling */
void chkabort(void) { return; }  /* really */
#endif


#define MINARGS 4

UBYTE *vers = "\0$VER: savemem 37.1";
UBYTE *Copyright = 
  "savemem v37.1\n(c) Copyright 1993 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: savemem filename [0x]startaddress [0x]lastaddress";

int getval(char *s);
void usageExit(void);

void main(int argc, char **argv)
    {
    char  *start, *end, *fname;
    LONG  file, wLen;
    LONG  len, tmp;

    if(((argc)&&(argc<MINARGS))||((argc>1)&&(argv[argc-1][0]=='?'))) usageExit();

    fname = argv[1];
    start = (char *)getval(argv[2]);
    end   = (char *)getval(argv[3]);

    tmp = getval(fname);	/* check for error in arg order */

    if((tmp)||(!end)) usageExit();

    len = (LONG)end - (LONG)start + 1;
    if(len < 0) usageExit(); 

    if(!(file = Open(fname,MODE_NEWFILE)))
    	{
      	printf("Can't open file \"%s\"\n",fname);
      	exit(0);
      	}

    wLen = Write(file,start,len);
    if(wLen < 0) printf("Write error\n");
    Close(file);
    }

void usageExit()
    {
    printf("%s\n%s\n",Copyright,usage);
    exit(RETURN_OK);
    }

int getval(char *s)
    {
    int value, count;

    if((s[1]|0x20) == 'x')  count = stch_i(&s[2],&value);
    else count = stcd_i(&s[0],&value);
    return(value);
    }


