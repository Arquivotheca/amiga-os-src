;/* pubscreenlist.c - Execute me to compile me with SAS C 6.x
SC pubscreenlist.c data=near nominc strmer streq nostkchk saveds ign=73
Slink FROM LIB:c.o,pubscreenlist.o TO pubscreenlist LIBRARY LIB:SC.lib,LIB:Amiga.lib ND NOICONS
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <intuition/screens.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>

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

UBYTE *vers = "\0$VER: pubscreenlist 40.1";
UBYTE *Copyright = 
  "pubscreenlist v40.1\n(c) Copyright 1993 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: pubscreenlist";

void bye(UBYTE *s, int e);
void cleanup(void);

struct Library *IntuitionBase;
  
void main(int argc, char **argv)
    {
    struct List *list;
    struct PubScreenNode *pub;

    if(((argc)&&(argc<MINARGS))||((argc>1)&&(argv[argc-1][0]=='?')))
	{
	printf("%s\n%s\n",Copyright,usage);
	bye("",RETURN_OK);
	}

    if(!(IntuitionBase = OpenLibrary("intuition.library",37)))
	{
	bye("Can't open V37+ intuition library",RETURN_FAIL);
	}

    list = LockPubScreenList();
    for ( pub = (struct PubScreenNode *)list->lh_Head;
          pub->psn_Node.ln_Succ;
          pub = (struct PubScreenNode *)pub->psn_Node.ln_Succ )
	{
	printf("%s\n",pub->psn_Node.ln_Name);
	printf("\tpsn_Screen        = $%08lx\n",pub->psn_Screen);
	printf("\tpsn_Flags         = $%04lx\n",pub->psn_Flags);
	printf("\tpsn_VisitorCount  = %ld\n",pub->psn_VisitorCount);
	}
    UnlockPubScreenList();
    
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
    if(IntuitionBase)	CloseLibrary(IntuitionBase);
    }
