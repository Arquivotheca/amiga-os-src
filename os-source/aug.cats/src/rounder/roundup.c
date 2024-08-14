;/* roundup.c - Execute me to compile me with SAS C 5.10
if not exists roundupa.o
  HX68 -a roundupa.asm -i H:include -o roundupa.o
endif
LC -b0 -cfistq -v -j73 roundup.c
Blink FROM LIB:awstartup.obj,roundup.o roundupa.o TO roundup LIBRARY LIB:amiga.lib,LIB:LCnb.lib
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <workbench/startup.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_stdio_protos.h>

#include <stdlib.h>
#include <string.h>

#define MINARGS 2

UBYTE *vers = "\0$VER: roundup 39.1";
UBYTE *Copyright = 
  "roundup v39.1\n(c) Copyright 1992 Commodore-Amiga, Inc.  All Rights Reserved";

UBYTE *usage[] = {
"Usage: roundup ON|OFF [value] (value must be 16,32,64,128; default is 64)",
"From Workbench: Double-click to toggle on and off\n",
"Emulates the V39+AA rounding up of the BitMap width (BytesPerRow) of Screen",
"bitmaps and displayable AllocBitMap'd bitmaps of the higher bandwidth modes.",
"Use on non-AA or non-promoted V39 machines to test for many BytesPerRow",
"compatiblity problems.  Test with bitmap/screen widths not divisible by 64.",
NULL };


void bye(UBYTE *s, int e);
void cleanup(void);
BOOL strEqu(UBYTE *p, UBYTE *q);

extern UBYTE  far NeedWStartup;
UBYTE  *HaveWStartup = &NeedWStartup;

char AppWindow[] = "CON:0000/0040/0640/0100/Roundup - V39 AA BytesPerRow Emulation Wedge/auto/close/wait";

#define TOUPPER(c)      ((c)>='a'&&(c)<='z'?(c)-'a'+'A':(c)) 
#define HIGHER(x,y)     ((x)>(y)?(x):(y))

typedef LONG far (*PFL)();

#define LVOAllocBitMap (-918L)
extern  ULONG far startFix, endFix, useCount, testcnt, rounder;


struct Library *GfxBase;
PFL OldFunc;

BOOL  FromWb;
ULONG wSize;
UBYTE *toggle;

struct WEDGEY {
   struct MsgPort WedgePort;
   ULONG          *Wedge;
   ULONG          WedgeSize;
   PFL            RealFunc;
   PFL            MyFunc;
   ULONG          *ptrToUseCount;
   char           Name[48];
   };

struct WEDGEY *wedgey;
char *wedgeyName ="CAS_Roundup_Port";


void main(int argc, char **argv)
    {
    extern struct Library *SysBase;
    struct WBStartup *WBenchMsg;
    ULONG  fixsize, *reloc;
    int k;

    FromWb = (argc==0) ? TRUE : FALSE;
    if(FromWb)  WBenchMsg = (struct WBStartup *)argv;

    if(((argc)&&(argc<MINARGS))||((argc>1)&&(argv[argc-1][0]=='?')))
	{
	printf("%s\n",Copyright);
	for(k=0; usage[k]; k++)	printf("%s\n",usage[k]);
	bye("",RETURN_OK);
	}

    if(!(GfxBase = OpenLibrary("graphics.library",39)))
	{
	bye("Roundup requires V39",RETURN_WARN);
	}

    rounder = 64;
    if(argc == 3)
	{
	rounder = atoi(argv[2]);
	if((rounder != 16)&&(rounder != 32)&&(rounder != 64)&&(rounder != 128))
	    {
	    printf("Roundup value must be 16, 32, 64, or 128.  Using 64\n");
	    rounder = 64;
	    }
	}
    
    if(FromWb) toggle = "ON";
    else toggle = argv[1];

    Forbid();
    /* If present already, remove, whether resetting or installing */
    if(wedgey = (struct WEDGEY *)FindPort(wedgeyName))
	{
      	if(*(wedgey->ptrToUseCount))
	    {
            Permit();
            bye("Roundup may not be removed or changed while in use",
                     RETURN_WARN);
            }
      	/* Try to unwedge */
      	OldFunc = SetFunction(GfxBase,LVOAllocBitMap,wedgey->RealFunc);

      	if(OldFunc != wedgey->MyFunc)
            {
            /* Someone else has changed the vector */
            /* We put theirs back - can't exit yet  */
            SetFunction(GfxBase, LVOAllocBitMap, OldFunc);
            Permit();
            bye("Can't remove Roundup - another SetFunction present",
                     RETURN_WARN);
            }
      	else
            {
            RemPort(wedgey);
            while(*(wedgey->ptrToUseCount))  Delay(20);
            FreeMem(wedgey->Wedge, wedgey->WedgeSize);
	    if(FromWb) toggle = "OFF";
	    printf("Roundup - existing SetFunction removed\n");
            }
      	}
    else
	{
        if(strEqu(toggle,"OFF"))
	    {
	    Permit();
	    bye("Roundup was not on",RETURN_WARN);
	    }
	}
    Permit();


    /* If not just resetting, install new wedge */
    if(strEqu(toggle,"ON"))
      	{
      	fixsize = (ULONG)&endFix - (ULONG)&startFix;
      	wSize = fixsize+sizeof(struct WEDGEY)+16;

      	if(!(reloc = (ULONG *)AllocMem(wSize,MEMF_PUBLIC|MEMF_CLEAR)))
            {
            bye("Not enough memory",RETURN_FAIL);
            }
      	else
            {
            /* Copy relocatable assembler code to alloc'd memory */
            CopyMem(&startFix,reloc,fixsize);
	    if(SysBase->lib_Version >= 37)	CacheClearU();

            /* Then WedgePort... Set up port */
            wedgey = (struct WEDGEY *)((ULONG)reloc + (ULONG)fixsize);

            wedgey->ptrToUseCount =
               (ULONG *)((ULONG)reloc+((ULONG)&useCount-(ULONG)&startFix));

            wedgey->Wedge = (ULONG *)reloc;
            wedgey->WedgeSize = wSize;
            strcpy(wedgey->Name,wedgeyName);
            wedgey->WedgePort.mp_Node.ln_Name = wedgey->Name;
            wedgey->WedgePort.mp_Node.ln_Type = NT_MSGPORT;
            Forbid();

            /* If we find another WedgePort at this point, Free ours and abort */
            if(FindPort(wedgeyName))
            	{
            	Permit();
            	FreeMem(wedgey->Wedge,wedgey->WedgeSize);
            	bye("Aborted... Another Roundup installation occurred",
                         RETURN_WARN);
            	}
            else
            	{
            	/* else Add our port and SetFunction to our code */
            	AddPort(wedgey);

            	/* Store old vector in first long of reloc   */
            	/* New function entry is just past that long */
            	*reloc =
             	(ULONG)SetFunction(GfxBase, LVOAllocBitMap,(PFL)(((ULONG)reloc)+4));
            	wedgey->RealFunc = (PFL)(*reloc);
            	wedgey->MyFunc   = (PFL)(((ULONG)reloc)+4);

	    	printf("Roundup SetFunction installed\n");
	    	printf("Test with bitmap widths not divisible by %ld\n",rounder);
            	}
            Permit();
            }
      	}
    bye("",RETURN_OK);
    }



void bye(UBYTE *s, int err)
    {
    if(*s)
      	{
      	printf("%s\n",s);
      	}
    cleanup();
    exit(err);
    }

void cleanup()
    {
    if(GfxBase)	CloseLibrary(GfxBase);
    return;
    }


/* String functions */

BOOL strEqu(UBYTE *p, UBYTE *q) 
    { 
    while(TOUPPER(*p) == TOUPPER(*q))
    	{
      	if (*(p++) == 0)  return(TRUE);
      	++q; 
      	}
    return(FALSE);
    } 
