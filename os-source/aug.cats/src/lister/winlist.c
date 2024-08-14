;/* winlist.c - Execute me to compile me with Lattice 5.04
LC -b1 -cfistq -v -y -j73 winlist.c
Blink FROM LIB:c.o,winlist.o TO winlist LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/


#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>

#ifdef LATTICE
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

char *vers = "\0$VER: winlist 37.2";
char *Copyright = 
  "winlist v37.2\nCopyright (c) 1990-1991 Commodore-Amiga, Inc.  All Rights Reserved";

struct IntuitionBase *IntuitionBase;
struct Library *GfxBase;

void main(int argc, char **argv)
{
struct Window *w;
struct Screen *s;

if(argc > 1) printf("%s\n",Copyright), exit(0L);

if((IntuitionBase=(struct IntuitionBase *) OpenLibrary("intuition.library",0L))
 &&(GfxBase = OpenLibrary("graphics.library",0L)))
	{
	for(s=IntuitionBase->FirstScreen; s; s=s->NextScreen)
	    {
	    printf("\nSCREEN: $%lx  \"%s\"    Flags=$%04lx\n",s,s->Title,s->Flags);
	    printf("  at %ld,%ld, w=%ld, h=%ld, d=%ld, ViewPort.Modes=$%04lx",
			s->TopEdge, s->LeftEdge,
			s->Width,s->Height,
			s->RastPort.BitMap->Depth,
			s->ViewPort.Modes);
	    if(GfxBase->lib_Version >= 36)
		{
		printf(" VPModeID=$%08lx\n",GetVPModeID(&s->ViewPort));
		}
	    else printf("\n");

	    printf("  ColorMap type %ld, ColorMap.Count %ld\n",
			s->ViewPort.ColorMap->Type, s->ViewPort.ColorMap->Count);
 	    printf("  BitMap.Flags=$%02lx, BitMap.BytesPerRow=%ld\n",
			s->RastPort.BitMap->Flags,
			s->RastPort.BitMap->BytesPerRow);

	    for(w=s->FirstWindow; w; w=w->NextWindow)
		{
		printf("\n WINDOW: $%lx  \"%s\"\n",w,w->Title);
		printf("  at %ld,%ld, w=%ld, h=%ld, Flags=$%08lx\n",
		   w->TopEdge,w->LeftEdge,w->Width,w->Height,w->Flags);
		}
	    }
	CloseLibrary((struct Library *)IntuitionBase);
	CloseLibrary(GfxBase);

	printf("\nSome Viewport Modes:\n");
	printf("  HIRES     =$8000  LACE      =$0004  SUPERHIRES =$0020\n");
	printf("  HAM       =$0800  HALFBRITE =$0080  DUALPF     =$0400\n");
	printf("  GEN_AUDIO =$0100  GEN_VIDEO =$0002  PFBA       =$0040\n");
	printf("  VP_HIDE   =$2000  SPRITES   =$4000  EXTENDED   =$1000\n");
	printf("\nSome Window Flags:\n");
	printf("  REFRESHBITS   =$000000C0  SIMPLEREFRESH=$00000040\n");
	printf("  GIMMEZEROZERO =$00000400  SUPERBITMAP  =$00000080\n");
	printf("  BORDERLESS    =$00000800  BACKDROP     =$00000100\n");
	}	
	exit (0L);
}
