;/* timetest.c - Execute me to compile me with SAS C 5.10
LC -b1 -cfistq -v -y -j73 timetest.c
Blink FROM LIB:c.o,timetest.o TO timetest LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <libraries/dos.h>
#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef LATTICE
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif


#define MINARGS 3

UBYTE *vers = "\0$VER: timetest 37.1";
UBYTE *Copyright = 
  "timetest v37.1\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: timetest allocsize iterations";

void bye(UBYTE *s, int e);
void cleanup(void);
int getval(char *s);

void main(int argc, char **argv)
    {
    extern struct ExecBase *SysBase;
    struct DateStamp DI, DE;
    ULONG size, iterate, mins, ticks, vfreq;
    int k;
    UBYTE *mem;

    if(((argc)&&(argc<MINARGS))||(argv[argc-1][0]=='?'))
	{
	printf("%s\n%s\n",Copyright,usage);
	bye("",RETURN_OK);
	}

    size = getval(argv[1]);
    if(!size)     size=512;

    iterate = getval(argv[2]);
    if(!iterate)  iterate=1;

    vfreq = SysBase->VBlankFrequency;

    printf("Will allocate %ld bytes %ld times\n",size,iterate);

    DateStamp(&DI);
    for(k=0; k<iterate; k++)
	{
	if(mem=AllocMem(size,MEMF_PUBLIC))	FreeMem(mem,size);
	else	bye("Size not available\n",RETURN_WARN);
	}
    DateStamp(&DE);
    
    while(DE.ds_Tick < DI.ds_Tick)
	{
	DE.ds_Minute--;
	DE.ds_Tick += vfreq;
	}

    mins  = DE.ds_Minute - DI.ds_Minute;
    ticks = DE.ds_Tick - DI.ds_Tick;

    printf("For %ld allocs of %ld bytes: ELAPSED Min:Tick = %ld:%ld\n",
		iterate,size,mins,ticks);
    bye("",RETURN_OK);
    }

void bye(UBYTE *s, int e)
    {
    cleanup();
    exit(e);
    }

void cleanup()
    {

    }


int getval(char *s)
   {
   int value, count;

   if((s[1]|0x20) == 'x')  count = stch_i(&s[2],&value);
   else count = stcd_i(&s[0],&value);
   return(value);
   }

