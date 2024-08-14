;/* testtstat.c - Execute me to compile me with Lattice 5.04
LC -b1 -cfistq -v -y -j73 testtstat.c
Blink FROM LIB:c.o,testtstat.o TO testtstat LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <intuition/intuition.h>

#ifdef LATTICE
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif


#define MINARGS 1

UBYTE *vers = "\0$VER: testtstat 36.10";
UBYTE *Copyright = 
  "testtstat v36.10\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: testtstat";

void bye(UBYTE *s, int e);
void cleanup(void);
void changetaskname(UBYTE *s);
UBYTE *taskname(void);

#define TDELAY 100

UBYTE *name;

void main(int argc, char **argv)
    {
    ULONG k;
    struct Library *libbase;

    if(((argc)&&(argc<MINARGS))||(argv[argc-1][0]=='?'))
	{
	printf("%s\n%s\n",Copyright,usage);
	bye("",RETURN_OK);
	}

    name = taskname();

    changetaskname("f0d0W");    
    printf("(from ~$%lx) Next 2 second wait: 0 forbids, 0 disables\n",main);
    Delay(TDELAY);
    changetaskname("f0d0B"); 
    printf("(from ~$%lx) Next 2 second busy: 0 forbids, 0 disables\n",main);
    for(k=0; k<0x00F0; k++) printf(".");
    printf("\n");

    Forbid();
    changetaskname("f1d0W");
    printf("(from ~$%lx) Next 2 second wait: 1 forbid, 0 disables\n",main);
    Delay(TDELAY);
    changetaskname("f1d0B");
    printf("(from ~$%lx) Next 2 second busy: 1 forbid, 0 disables\n",main);
    for(k=0; k<0x00F0; k++) printf(".");
    printf("\n");

    Disable();
    changetaskname("f1d1W");
    printf("(from ~$%lx) Next 2 second wait: 1 forbid, 1 disables\n",main);
    Delay(TDELAY);
    changetaskname("f1d1B");
    printf("(from ~$%lx) Next 2 second busy: 1 forbid, 1 disables\n",main);
    for(k=0; k<0x00F0; k++) printf(".");
    printf("\n");

    changetaskname("xxxxx");
    Enable();
    Permit();


    if(libbase=OpenLibrary("mathieeedoubbas.library",0))
	{
    	changetaskname("ieee_in_use");
    	printf("Next 2 second busy: IEEE in use\n");
    	for(k=0; k<0x00F0; k++) printf(".");
    	printf("\n");
	CloseLibrary(libbase);
	}
    
    changetaskname(name);
    bye("",0L);
    }


UBYTE *taskname()
    {
    struct Task *ta = FindTask(NULL);
    return(ta->tc_Node.ln_Name);
    }

void changetaskname(UBYTE *s)
    {
    struct Task *ta = FindTask(NULL);
    strcpy(ta->tc_Node.ln_Name,s);
    }

void bye(UBYTE *s, int e)
    {
    cleanup();
    exit(e);
    }

void cleanup()
    {

    }

