;/* mungwalltest.c - Execute me to compile me with Lattice 5.04
LC -b0 -cfistq -v -y -j73 mungwalltest.c
Blink FROM LIB:c.o,mungwalltest.o TO mungwalltest LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>

#ifdef LATTICE
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
extern struct Library *SysBase;
#include <pragmas/exec_pragmas.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int CXBRK(void)  { return(0); }  /* Disable Lattice CTRL/C handling */
void chkabort(void) { return; }  /* really */
#endif


char *vers = "\0$VER: mungwalltest 37.6";
char *Copyright =
  "mungwalltest v37.6\n(c) Copyright 1992-93 Commodore-Amiga, Inc.  All Rights Reserved";
char *usage = "mungwalltest [range] (default=32)";

VOID testtask(void);


#define ASM __asm
#define REG(x) register __## x

typedef VOID (*ASM PFV)();

VOID (*ASM ExtFunc)( REG(a1) UBYTE *address, REG(d0) ULONG size);

PFV  ExtFunc;

UBYTE *extsegname="PROGDIR:mungtestseg";


#define INITMAXTEST 32


void main(int argc, char **argv)
{
    UBYTE *lonelyheart, *alloc1, *alloc2;
    UBYTE *tooshort;
    UBYTE oddname[32], beefname[32];
    ULONG size, *hit=0L, maxtest = INITMAXTEST, utmp, segtmp;
    BPTR seg;

    if((argc>1)&&(argv[1][0]=='?'))
	{
	printf("%s\n%s\n",Copyright,usage);
	exit(0L);
	}
    if((argc>1)&&(utmp=atoi(argv[1])))	maxtest=utmp;
    if(maxtest>128)			maxtest=32;

    printf("Mungwalltest at : 0x%lx\n", FindTask(0));
	tooshort = AllocMem(0, MEMF_CLEAR);
	if (tooshort) FreeMem(tooshort, 0);

    if (lonelyheart = (UBYTE *)AllocMem(16, MEMF_CLEAR)) {
        printf("Allocated 16 bytes at 0x%lx\n", lonelyheart);
        printf("Attempting NULL pointer FreeMem\n");
        FreeMem(NULL, 16);
        printf("Attempting NULL size FreeMem\n");
        FreeMem(lonelyheart, NULL);
        tooshort = lonelyheart;
        tooshort += 4;
        FreeMem(tooshort, 16);
        printf("Trashing memory after allocation.\n");
        strcpy(lonelyheart, "Lonely Hearts Club Band");
        printf("Trashing memory before allocation\n");
        lonelyheart -= 19;
        strcpy(lonelyheart, "Sergeant's Pepper ");
        lonelyheart += 19;
        printf("Freeing memory with wrong size\n");
        FreeMem(lonelyheart, 14);
    }

    printf("\nAllocating different sizes, trashing 1 byte before/after with 0x11\n");
    for (size=1; size < maxtest; size ++)
	{
    	if (lonelyheart = (UBYTE *)AllocMem(size, MEMF_CLEAR))
	    {
            printf("Allocated %ld bytes at 0x%lx, trashing... ", size, lonelyheart);

	    *(lonelyheart -1)     = 0x11;
	    *(lonelyheart + size) = 0x11;
            printf("freeing\n");
            FreeMem(lonelyheart, size);
	    }
    	}

    printf("\nTesting pairs of allocations, trashing 1 byte after via copy\n");
    for (size=1; size < maxtest; size ++)
	{
    	if (alloc1 = (UBYTE *)AllocMem(size, MEMF_CLEAR))
	    {
    	    if (alloc2 = (UBYTE *)AllocMem(size, MEMF_CLEAR))
		{
            	printf("Allocated %ld bytes at 0x%lx and 0x%lx, copying 1 byte too much... ", 
			size, alloc1, alloc2);
		CopyMem(alloc1,alloc2,size+1);
            	printf("freeing\n");
            	FreeMem(alloc2, size);
		}
	    FreeMem(alloc1,size);
	    }
    	}

    printf("\nAllocating too large memory block (size 0x60000000)\n");
    if (lonelyheart = (UBYTE *)AllocMem(0x60000000, MEMF_PUBLIC))
	    {
            printf("Allocated %ld bytes at 0x%lx\n", lonelyheart);
            FreeMem(lonelyheart, size);
	    }

    printf("Causing Enforcer hit by reading contents of address 0: %ld...\n",*hit);

    if(seg=LoadSeg(extsegname))
	{
	segtmp = (ULONG)seg;
	ExtFunc = (PFV)((segtmp << 2) + 4);
    	if (alloc1 = (UBYTE *)AllocMem(32, MEMF_CLEAR))
	    {
	    printf("Causing loaded segment \"%s\" at $%lx to improperly free\n",
		extsegname,ExtFunc);
    	    (*ExtFunc)(alloc1,size);
	    }
	UnLoadSeg(seg);
	}

    printf("\nOdd address named task \"oddname\" to Alloc 0 bytes - test on 68000\n");
    strcpy(oddname," oddname");
    CreateTask(&oddname[1],0,testtask,4000);
    Delay(100);

    printf("\n0xDEADBEEF named task to Alloc 0 bytes- test on 68000\n");
    beefname[0] = 0xDE;
    beefname[1] = 0xAD;
    beefname[2] = 0xBE;
    beefname[3] = 0xEF;
    CreateTask(beefname,0,testtask,4000);

    printf("Will exit in 10 seconds - hope tasks have deleted selves\n");
    Delay(500);
}



VOID testtask(void)
   {
   UBYTE	*tooshort;
   
   tooshort = AllocMem(0, MEMF_CLEAR);
   if (tooshort) FreeMem(tooshort, 0);
   DeleteTask(FindTask(NULL));
   }
