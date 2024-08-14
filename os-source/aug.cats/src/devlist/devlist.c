;/* devlist.c - Execute me to compile me with Lattice 5.04
LC -b1 -cfistq -v -y -j73 devlist.c
Blink FROM LIB:c.o,devlist.o TO devlist LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/libraries.h>
#ifdef LATTICE
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <stdlib.h>
#include <stdio.h>
int CXBRK(void) { return(0); }		/* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }	/* really */
#endif

char *vers = "\0$VER: devlist 37.1";
char *Copyright = 
  "devlist v37.1\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";

#define ARRAYSIZE 64L
extern struct ExecBase *SysBase;

void main(int argc,char **argv)
    {
    struct Library *lib, *libs[ARRAYSIZE];
    ULONG count = 0L, k;


    if(argc > 1) printf("%s\n",Copyright), exit(0L);

    Forbid();
    /* Note - printing within Forbid() would break the forbidden state */
    for ( lib = (struct Library *)SysBase->DeviceList.lh_Head;
          NULL != lib->lib_Node.ln_Succ;
          lib = (struct Library *)lib->lib_Node.ln_Succ)
        {
        if (count < ARRAYSIZE) libs[count++] = lib;
        }
    Permit();

    printf("Devices currently in system:\n");
    for (k=0; k<count; k++)
	{
	lib = libs[k];
	printf(" $%08lx  v%d.%d  \tOpenCnt=%d  \t%s\n",
	    lib, lib->lib_Version,lib->lib_Revision,
		 lib->lib_OpenCnt,lib->lib_Node.ln_Name);    
	}

    if (count == ARRAYSIZE) printf("Error: array overflow\n");
    }
