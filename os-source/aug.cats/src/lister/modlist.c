;/* modlist.c - Execute me to compile me with SAS C 6.x
SC modlist.c data=near nominc strmer streq nostkchk saveds ign=73
Slink FROM LIB:c.o,modlist.o TO modlist LIBRARY LIB:SC.lib,LIB:Amiga.lib ND NOICONS
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/resident.h>
#include <dos/dos.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

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


#define MINARGS 1

UBYTE *vers = "\0$VER: modlist 40.2";
UBYTE *Copyright = 
  "modlist v40.2\n(c) Copyright 1993 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: modlist";

#define ARRAYSIZE 80L
extern struct ExecBase *SysBase;

void main(int argc,char **argv)
    {
    struct Resident **modp, *mod, *mods[ARRAYSIZE];
    ULONG count = 0L, umod, k;
    BOOL Done = FALSE;

    if(((argc)&&(argc<MINARGS))||((argc>1)&&(argv[argc-1][0]=='?')))
	{
	printf("%s\n%s\n",Copyright,usage);
	exit(RETURN_OK);
	}

    if(argc > 1) printf("%s\n",Copyright), exit(0L);

    Forbid();
    /* Note - printing within Forbid() would break the forbidden state */
    modp = (struct Resident **)SysBase->ResModules;
    while(!Done)
        {
	mod = *modp;
	if(!mod) Done = TRUE;
	else
	    {
	    umod = (ULONG)mod;
	    if(umod & 0x80000000)
		{
		printf("extension\n");
		modp = (struct Resident **)(umod & 0x7FFFFFFF);
		}
            else
		{
		if(count < ARRAYSIZE)   mods[count++] = mod, modp++;
		}
	    }
        }
    Permit();

    printf("Resident modules currently in system:\n");
    for (k=0; k<count; k++)
	{
	if(SetSignal(0,0) & SIGBREAKF_CTRL_C)	break;
	mod = mods[k];
	printf("% 8lx %+4d %d %24s V%d ID=%s",
	    mod, mod->rt_Pri, mod->rt_Type,
		mod->rt_Name,mod->rt_Version,mod->rt_IdString);
	if(mod->rt_IdString[strlen(mod->rt_IdString)-1] > 0x13) printf("\n");    
	}

    if (count == ARRAYSIZE) printf("Error: array overflow\n");
    
    }
