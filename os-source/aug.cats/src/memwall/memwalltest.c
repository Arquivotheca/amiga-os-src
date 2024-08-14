;/* memwalltest.c - Execute me to compile me with Lattice 5.04
LC -b1 -cfistq -v -y -j73 memwalltest.c
Blink FROM LIB:c.o,memwalltest.o TO memwalltest LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>

#ifdef LATTICE
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

char *vers = "\0$VER: memwalltest 36.10";
char *Copyright = 
  "memwalltest v36.10\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";

void main(int argc, char **argv)
    {
    UBYTE *alloc;

    if(alloc = AllocMem(100,MEMF_CLEAR))
	{
	alloc[100] = '\0';
	FreeMem(alloc,100);
	}
    exit(0L);
    }
