;/* warnifpressed.c - Execute me to compile me with SAS C 5.10
LC -b1 -cfistq -v -y -j73 warnifpressed.c
Blink FROM LIB:c.o,warnifpressed.o TO warnifpressed LIBRARY LIB:LC.lib,LIB:Amiga.lib,LIB:debug.lib ND
quit
*/

#include <exec/types.h>
#include <libraries/lowlevel.h>
#include <dos/dos.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/lowlevel_protos.h>

extern struct Library *SysBase;
extern struct Library *DOSBase;

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/lowlevel_pragmas.h>


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef LATTICE
int CXBRK(void)  { return(0); }  /* Disable Lattice CTRL/C handling */
void chkabort(void) { return; }  /* really */
#endif

#define MINARGS 2

UBYTE *vers = "\0$VER: warnifpressed 40.2";
UBYTE *Copyright = 
  "warnifpressed v40.2\n(c) Copyright 1993 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage =
"Usage: warnifpressed button [button button...] [PORT0]\n"
"RED(LMB) BLUE(RMB) GREEN YELLOW FORWARD(R_EAR) REVERSE(L_EAR) PLAY\n";


UBYTE *names[] = {
"RED", "LMB", "BLUE", "RMB",
"GREEN", "YELLOW",
"FORWARD", "R_EAR", "REVERSE", "L_EAR",
"PLAY", NULL};

ULONG bits[] = {
JPF_BUTTON_RED,JPF_BUTTON_RED,JPF_BUTTON_BLUE,JPF_BUTTON_BLUE,
JPF_BUTTON_GREEN,JPF_BUTTON_YELLOW,
JPF_BUTTON_FORWARD,JPF_BUTTON_FORWARD,JPF_BUTTON_REVERSE,JPF_BUTTON_REVERSE,
JPF_BUTTON_PLAY, 0};

/**********    debug macros     ***********/
#define MYDEBUG  0
void kprintf(UBYTE *fmt,...);
void dprintf(UBYTE *fmt,...);
#define DEBTIME 0
#define bug kprintf
#if MYDEBUG
#define D(x) (x); if(DEBTIME>0) Delay(DEBTIME);
#define D2(x) ;
#else
#define D(x) ;
#define D2(x) ;
#endif /* MYDEBUG */
/********** end of debug macros **********/

struct Library 	*LowLevelBase = NULL;

void cleanup( void )
    {
    if(LowLevelBase)	CloseLibrary(LowLevelBase);
    }

void bye(UBYTE *s, int e)
    {
    if(*s)	printf("%s\n",s);
    cleanup();
    exit(e);	
    }


void main(int argc, char **argv)
    {
    ULONG askstate, state, port;
    int k,i,retval;

    if(((argc)&&(argc<MINARGS))||((argc>1)&&(argv[argc-1][0]=='?')))
	{
	printf("%s\n%s\n",Copyright,usage);
	exit(RETURN_OK);
	}

    port=1;
    askstate = 0L;
    for(k=1; k<argc; k++)
	{
	if(!(stricmp(argv[k],"PORT0")))	port=0;
	else for(i=0; names[i]; i++)
	    {
	    if(!(stricmp(argv[k],names[i])))
		{
		askstate |= bits[i];
		break;
		}
	    }
	}

    if(!(LowLevelBase = OpenLibrary("lowlevel.library",40)))
	{
	bye("Can't open lowlevel.library v40+",RETURN_FAIL);
	}	

    /* Note - in startup-sequnece, single ReadJoyPort() did not
     * return special game controller buttons
     */

    state = ReadJoyPort(port);
    Delay(10);
    state = ReadJoyPort(port);
    Delay(10);
    state = ReadJoyPort(port);


    retval = 0L;
    if((state&askstate)==askstate)	retval = RETURN_WARN;

    bye("",retval);
    }
