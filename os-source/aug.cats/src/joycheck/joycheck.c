;/* joycheck.c - Execute me to compile me with SAS C 5.10
LC -b1 -cfistq -v -y -j73 joycheck.c
Blink FROM LIB:c.o,joycheck.o TO joycheck LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/


#include <exec/types.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include "libraries/lowlevel.h"

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/lowlevel_protos.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pragmas/lowlevel_pragmas.h"

#ifdef LATTICE
int CXBRK(void)  { return(0); }  /* Disable Lattice CTRL/C handling */
void chkabort(void) { return; }  /* really */
#endif

UBYTE version[] = "$VER: 39.3 joycheck";



/* Mask #define with result of ReadJoyPort(). */
#define MSK(MASK) ( MASK & state )

struct Library *IntuitionBase = NULL;
struct Library *GfxBase = NULL;
struct Library *LowLevelBase = NULL;

struct Screen *scr = NULL;
struct Window *win = NULL;
struct RastPort *rp;
struct BitMap *bitmap;
UBYTE *plane;
UWORD  bpr;

/* in bytes */
#define PX0	(0)
#define PX1	(0)

/* in scan lines / pixels */
#define HELPX   (16)
#define HELPY	(40)
#define TEXTX   (PX0<<3)
#define TEXTY	(64)
#define PY0	(52)
#define PY1	(116)

UWORD xpos[2] = { PX0, PX1 };
UWORD ypos[2] = { PY0, PY1 };


UBYTE *help = 
"To Exit - click close gadget OR hold PORT1 A&B (BLUE&RED) down a long time";

UBYTE *labels[] = {
"0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 PORT 0 ABOVE",
"h h h h h h h h v v v v v v v v <- Mouse    CType->none 0 0 0 0",
"J J J J                           P R F G Y R B    game 1 0 0 0",
"O O O O                           L V O R E E L   mouse 0 1 0 0",
"Y Y Y Y                           A S R E L D U   joyst 1 1 0 0",
"R L D U                           Y   W E L   E     ??? 0 0 1 0",
"0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 PORT 1 BELOW",
"",
"",
"",
"        _____          _____               _____          _____ ",
"       |     |        |     |             |     |        |     |",
"       |     |________|     |             |     |________|     |",
"       |                    |             |                    |",
"       |                    |             |                    |",
"       |____________________|             |____________________|",
/*
         --X--          --X--
         -----          -----
         --X-------------X-X-
         X---X-------X---X-X-
         --X-----------------
*/
NULL };

/* Button offsets -  X in bytes, Y in lines                                */
#define BXSTART0  (8)
#define BXSTART1  (43)
#define BYSTART   (TEXTY+(11*8)-6)


/*                 R    L    D    U    P    V    F    G    Y    R    B     */
UWORD bxoffs[] = { 4,   0,   2,   2,  12,   2,  17,  16,  18,  16,  18 }; 
UWORD byoffs[] = { 3*8, 3*8, 4*8, 2*8, 3*8, 0*8, 0*8, 2*8, 2*8, 3*8, 3*8 };

BOOL cleared[2] = { TRUE, TRUE };

struct TextAttr topaz80 =
   {"topaz.font",TOPAZ_EIGHTY,FS_NORMAL,FPF_ROMFONT };

VOID bye(UBYTE *msg, LONG err )
{
    if(*msg)		printf("%s\n",msg);
    if(win)		CloseWindow(win);
    if(scr)		CloseScreen(scr);
    if(GfxBase)		CloseLibrary(GfxBase);
    if(IntuitionBase)	CloseLibrary(IntuitionBase);
    if(LowLevelBase)	CloseLibrary(LowLevelBase);
    exit(err);
}

VOID showbits(ULONG state, ULONG port)
    {
    ULONG k, j, bset;
    UWORD x, y, sbx, sby, bx, by;
    UBYTE *spot;
    int bi;

    x = xpos[port];	/* in bytes  */
    y = ypos[port];	/* in pixels */

    sbx = port ? BXSTART0 : BXSTART1;
    sby = BYSTART;

    for(k=0; k<32; k++, x+=2)
	{
	bset = state & (1L << k);
    	spot = plane + (y * bpr) + x;
	if(bset)	for(j=0; j<3; j++, spot+=bpr)	*spot = 0xFF;
	else		for(j=0; j<3; j++, spot+=bpr)   *spot = 0x00;

	if((state & JP_TYPE_MASK) == JP_TYPE_GAMECTLR )
	    {
	    cleared[port]=FALSE;
	    if( ((k>=0)&&(k<=3)) || ((k>=17)&&(k<=23)) )
	    	{
		bi = (k<=3) ? k : k - 13;
		by = sby + byoffs[bi];		/* in lines */
		bx = sbx + bxoffs[bi];		/* in bytes */
    		spot = plane + (by * bpr) + bx;
		if(bset)	for(j=0; j<3; j++, spot+=bpr)	*spot = 0xFF;
		else		for(j=0; j<3; j++, spot+=bpr)   *spot = 0x00;
	    	}
	    }
	else if((state & JP_TYPE_MASK) == JP_TYPE_JOYSTK )
	    {
	    cleared[port]=FALSE;
	    if( ((k>=0)&&(k<=3)) || ((k>=22)&&(k<=23)) )
		{
		bi = (k<=3) ? k : k - 13;
		by = sby + byoffs[bi];		/* in lines */
		bx = sbx + bxoffs[bi];		/* in bytes */
    		spot = plane + (by * bpr) + bx;
		if(bset)	for(j=0; j<3; j++, spot+=bpr)	*spot = 0xFF;
		else		for(j=0; j<3; j++, spot+=bpr)   *spot = 0x00;
		}
	    }
	else if(!(cleared[port]))
	    {
	    cleared[port]=TRUE;
	    if( ((k>=0)&&(k<=3)) || ((k>=17)&&(k<=23)) )
		{
		bi = (k<=3) ? k : k - 13;
		by = sby + byoffs[bi];		/* in lines */
		bx = sbx + bxoffs[bi];		/* in bytes */
    		spot = plane + (by * bpr) + bx;
		for(j=0; j<3; j++, spot+=bpr)   *spot = 0x00;
		}
	    }
	}
    }

void main( int argc, char **argv)
{
ULONG port = 0L;
ULONG state = 0L, oldstate = (ULONG) JP_TYPE_NOTAVAIL;
ULONG state0, state1, heldtwo, htcnt;
BYTE x1,y1,x2,y2,diffx,diffy;
int k;

    if(!argc)	exit(RETURN_FAIL);

    IntuitionBase 	= OpenLibrary("intuition.library",39);
    GfxBase 		= OpenLibrary("graphics.library",39);
    LowLevelBase 	= OpenLibrary("lowlevel.library",39);

    if((!IntuitionBase)||(!GfxBase)||(!LowLevelBase))
	{
	bye("Error opening Intuition, Graphics, or LowLevel library V39+",
		RETURN_FAIL);
	}


    if(!(scr = OpenScreenTags(NULL,
				SA_DisplayID, HIRES,
				SA_Width,	640,
				SA_Height,	200,
				SA_Depth,	  1,
				SA_Font,	&topaz80,
				SA_Title, "     JoyCheck",
				TAG_DONE)))
	{
	bye("Can't open screen",RETURN_FAIL);
	}

    if(!(win = OpenWindowTags(NULL,
				WA_Width,	 40,
				WA_Height,	 20,
				WA_Flags,	 WFLG_CLOSEGADGET,
				WA_IDCMP,	 IDCMP_CLOSEWINDOW,
				WA_CustomScreen, scr,
				TAG_DONE)))
	{
	bye("Can't open window",RETURN_FAIL);
	}

    rp 	   = &scr->RastPort;
    bitmap = scr->RastPort.BitMap;
    bpr	   = bitmap->BytesPerRow;
    plane  = bitmap->Planes[0];

    SetAPen(rp,1);

    Move(rp,HELPX,HELPY);
    Text(rp,help,strlen(help));

    for(k=0; labels[k]; k++)
	{
    	Move(rp,TEXTX,TEXTY + (k<<3));
	Text(rp,labels[k],strlen(labels[k]));
	}

    htcnt=0;

    /* While the CLOSE gadget hasn't been hit... */
    while ( 0L == CheckSignal( 1L<<win->UserPort->mp_SigBit ) )
	{
	state0 = ReadJoyPort(0);
	state1 = ReadJoyPort(1);
	showbits(state0,0);
	showbits(state1,1);

	/* If Left and Right buttons held long, exit */
	if((state1&(JPF_BUTTON_BLUE|JPF_BUTTON_RED))==
			(JPF_BUTTON_BLUE|JPF_BUTTON_RED))	htcnt++;
	else htcnt=0;
	if(htcnt > 3000)	break;
	
#ifdef OLDSTUFF
	oldstate = state;
	printf("\nPort %1ld:  ", port);
	state = ReadJoyPort( port );

	if (oldstate == state)
			puts("No change.");
	else
	    {
	    switch ( MSK(JP_TYPE_MASK) )
		{
		case (JP_TYPE_NOTAVAIL):
			puts("Port data unavailable.");
		break;

		case (JP_TYPE_GAMECTLR):
			puts("Game controller:");
			if (MSK(JPF_BUTTON_BLUE))	puts("	Stop/BLUE");
			if (MSK(JPF_BUTTON_RED)) 	puts("	Select/RED");
			if (MSK(JPF_BUTTON_YELLOW))	puts("	Repeat/YELLOW");
			if (MSK(JPF_BUTTON_GREEN))	puts("	Shuffle/GREEN");
			if (MSK(JPF_BUTTON_FORWARD))	puts("	Right ear");
			if (MSK(JPF_BUTTON_REVERSE))	puts("	Left ear");
			if (MSK(JPF_BUTTON_PLAY))	puts("	Pause/Middle");
			if (MSK(JPF_JOY_UP))		puts("	Up direction");
			if (MSK(JPF_JOY_DOWN))		puts("	Down direction");
			if (MSK(JPF_JOY_LEFT))		puts("	Left direction");
			if (MSK(JPF_JOY_RIGHT))		puts("	Right direction");
		break;

		case (JP_TYPE_JOYSTK):
			puts("Joystick:");
			if (MSK(JPF_BUTTON_BLUE))	puts("	Right");
			if (MSK(JPF_BUTTON_RED))	puts("	Fire");
			if (MSK(JPF_JOY_UP))		puts("	Up direction");
			if (MSK(JPF_JOY_DOWN))		puts("	Down direction");
			if (MSK(JPF_JOY_LEFT))		puts("	Left direction");
			if (MSK(JPF_JOY_RIGHT))		puts("	Right direction");
		break;

		case (JP_TYPE_MOUSE):
			puts("Mouse:");

			x1 = MSK(JP_MHORZ_MASK);
			y1 = (MSK(JP_MVERT_MASK) >> 8);
			state = ReadJoyPort( port );
			x2 = MSK(JP_MHORZ_MASK);
			y2 = (MSK(JP_MVERT_MASK) >> 8);
			diffx = x2-x1;
			diffy = y2-y1;
			if (MSK(JPF_BUTTON_BLUE))	puts("	Right mouse button");
			if (MSK(JPF_BUTTON_RED))	puts("	Left mouse button");
			if (MSK(JPF_BUTTON_PLAY))	puts("	Middle mouse button");
			printf("\tx: %d\ty: %d\n",diffx,diffy); 
		break;

		case (JP_TYPE_UNKNOWN):
			puts("Unknown device.");
		break;

		default:                    /* default */
			puts("WARNING: Unexpected JP_TYPE!");
		break;
		}
	    }
#endif

	}
    bye("",RETURN_OK );
}
