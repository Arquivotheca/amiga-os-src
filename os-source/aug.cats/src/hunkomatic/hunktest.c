;/* hunktest.c - Execute me to compile me with SAS C 5.10
LC -b1 -cfistq -v -y -j73 hunktest.c
Blink FROM LIB:c.o,hunktest.o,hunkomatic.o TO hunktest LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <dos/dos.h>
#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef LATTICE
int CXBRK(void)  { return(0); }  /* Disable Lattice CTRL/C handling */
void chkabort(void) { return; }  /* really */
#endif


#define MINARGS 1

UBYTE *vers = "\0$VER: hunktest 39.1";
UBYTE *Copyright = 
  "hunktest v39.1\n(c) Copyright 1993 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: hunktest";

void bye(UBYTE *s, int e);
void cleanup(void);


#define ASM __asm
#define REG(x) register __## x
extern BOOL ASM InThisSegList(REG(a0) BPTR seglist, REG(a1) VOID *address, REG(a2) ULONG *hunkp, REG(a3) ULONG *offsp, REG(a6) struct Library *SysBase);
extern BOOL ASM InProcSegList(REG(a0) struct Task *task, REG(a1) VOID *address, REG(a2) ULONG *hunkp, REG(a3) ULONG *iffsp, REG(a6) struct Library *SysBase);

ULONG globalvariable;
ULONG hunk,offs;

extern struct Library *SysBase;

void main(int argc, char **argv)
    {
    BPTR seg;
    ULONG *hit = 0L;
    UBYTE *caddress;

    if(((argc)&&(argc<MINARGS))||((argc>1)&&(argv[argc-1][0]=='?')))
	{
	printf("%s\n%s\n",Copyright,usage);
	bye("",RETURN_OK);
	}

    if(InProcSegList(FindTask(NULL),&globalvariable,&hunk,&offs,SysBase))
	{
	printf("globalvariable at $%lx in process seglist:  Hunk: $%lx  Offset: $%lx\n",
			&globalvariable, hunk, offs);
	}
    else
	{
	printf("globalvariable not in process seglist, error=%ld\n",hunk);
	}


    if(InProcSegList(FindTask(NULL),(VOID *)42,&hunk,&offs,SysBase))
	{
	printf("Address $%lx in process seglist:  Hunk: $%lx  Offset: $%lx\n",
			42 , hunk, offs);
	}
    else
	{
	printf("address 42 not in process seglist, error=%ld\n",hunk);
	}

    printf("Causing Enforcer hit by reading contents of address 0...\n",*hit);
    
    if(!(seg = LoadSeg("c:copy")))	exit(20L);

    caddress = BADDR(seg);
    caddress += 4;
    if(InThisSegList(seg,caddress,&hunk,&offs,SysBase))
	{
	printf("Address $%lx in loaded seglist:  Hunk: $%lx  Offset: $%lx\n",
			caddress , hunk, offs);
	}
    else
	{
	printf("address $%lx not in loaded seglist, error=%ld\n",caddress,hunk);
	}
    UnLoadSeg(seg);    
    }


void bye(UBYTE *s, int e)
    {
    cleanup();
    exit(e);
    }

void cleanup()
    {

    }

