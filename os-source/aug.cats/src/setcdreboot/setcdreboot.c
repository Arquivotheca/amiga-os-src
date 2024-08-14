;/* setcdreboot.c - Execute me to compile me with SAS C 6.x
SC setcdreboot.c data=near nominc strmer streq nostkchk saveds ign=73
Slink FROM LIB:c.o,setcdreboot.o TO setcdreboot LIBRARY LIB:SC.lib,LIB:Amiga.lib ND
quit

*/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <libraries/lowlevel.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/lowlevel_protos.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#ifdef __SASC
void __regargs __chkabort(void) {}      /* Disable SAS CTRL-C checking. */
#else
#ifdef LATTICE
void chkabort(void) {}                  /* Disable LATTICE CTRL-C checking */
#endif
#endif


#define MINARGS 2

UBYTE *vers = "\0$VER: setcdreboot 40.1";
UBYTE *Copyright = 
  "setcdreboot v40.1\n(c) Copyright 1993 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: setcdreboot ON | OFF | DEFAULT";

void bye(UBYTE *s, int e);
void cleanup(void);

struct Library *LowLevelBase = NULL;

void main(int argc, char **argv)
    {
    BOOL state = CDReboot_On;

    if(((argc)&&(argc<MINARGS))||((argc>1)&&(argv[argc-1][0]=='?')))
	{
	printf("%s\n%s\n",Copyright,usage);
	bye("",RETURN_OK);
	}

    if(!(stricmp(argv[1],"ON")))		state = CDReboot_On;
    else if(!(stricmp(argv[1],"OFF")))		state = CDReboot_Off;
    else if(!(stricmp(argv[1],"DEFAULT")))	state = CDReboot_Default;
    else
	{
	printf("Unknown arg: %s\n",argv[1]);
	bye("",RETURN_WARN);
	}

    if(!(LowLevelBase = OpenLibrary("lowlevel.library",0)))
	{
	bye("Can't open lowlevel.library", RETURN_FAIL);
	}

    SystemControl(SCON_CDReboot,state,TAG_DONE);

    bye("",RETURN_OK);
    }



void bye(UBYTE *s, int e)
    {
    if(*s) printf("%s\n",s);
    cleanup();
    exit(e);
    }

void cleanup(void)
    {
    if(LowLevelBase)	CloseLibrary(LowLevelBase);
    }
