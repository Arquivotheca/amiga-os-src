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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

char *vers = "\0$VER: winlist 36.12";
char *Copyright = 
  "winlist v36.12\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";

struct IntuitionBase *IntuitionBase;

void main(int argc, char **argv)
{
struct Window *w;
struct Screen *s;

if(argc > 1) printf("%s\n",Copyright), exit(0L);

if(IntuitionBase=(struct IntuitionBase *)
	OpenLibrary("intuition.library",0L))
	{
	for(s=IntuitionBase->FirstScreen; s; s=s->NextScreen)
	    {
	    printf("\nSCREEN: $%lx  \"%s\"\n",s,s->Title);
	    printf("        w=%ld, h=%ld, d=%ld, te/le=%ld/%ld ViewPort Modes=$%04lx\n",
			s->Width,s->Height,
			s->RastPort.BitMap->Depth,
			s->TopEdge, s->LeftEdge, s->ViewPort.Modes);
	    for(w=s->FirstWindow; w; w=w->NextWindow)
		{
		printf("\n WINDOW: $%lx  \"%s\"\n",w,w->Title);
		printf("  at %ld,%ld, w=%ld, h=%ld, Flags=0x%08lx\n",
		   w->TopEdge,w->LeftEdge,w->Width,w->Height,w->Flags);
		}
	    }
	CloseLibrary((struct Library *)IntuitionBase);

	printf("\nViewport Modes:\n");
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
