;/* myrequest.c - Execute me to compile me with Lattice 5.04
LC -b1 -cfistq -v -y -j73 myrequest.c
Blink FROM LIB:c.o,myrequest.o TO myrequest LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

/*
#define MYDEBUG
*/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>

#ifdef LATTICE
#include <proto/all.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

LONG myrequest(struct Window *win,
		 UBYTE *t1,UBYTE *t2,UBYTE *t3,UBYTE *tp,UBYTE *tn,
		 LONG pflags, LONG nflags);

struct Library *IntuitionBase;


void main (int argc, char **argv)
{
struct Screen *scr;
struct Window *win;
LONG response;
if(IntuitionBase = OpenLibrary("intuition.library",0l))
	{
	if(scr = (struct Screen *)OpenWorkBench())
		{
		win = scr->FirstWindow;

		response = myrequest(win, "Line 1", "Line 2", "Line 3",
					  "Yes", "No",
					  0L, 0L);
		printf("response = %ld\n",response);
		}
	CloseLibrary(IntuitionBase);
	}
exit(response);
}

#define WIDTH 320
#define HEIGHT 80

#define FSIZE 8
#define TOPEDGE1 0
#define TOPEDGE2 (TOPEDGE1 + FSIZE)
#define TOPEDGE3 (TOPEDGE2 + FSIZE)
#define TOPEDGEPN (TOPEDGE3 + FSIZE);
SHORT topedges[5] = {TOPEDGE1,TOPEDGE2,TOPEDGE3,0L,0L};

struct TextAttr topaz8 = {"topaz.font",FSIZE,FS_NORMAL,FPF_ROMFONT};


LONG myrequest(struct Window *win,
			UBYTE *t1,UBYTE *t2,UBYTE *t3,UBYTE *tp,UBYTE *tn,
			LONG pflags,LONG nflags)
{
struct IntuiText itexts[5];
UBYTE **tarray;
LONG response;
int k;

tarray = (UBYTE **)(&t1);

for(k=0; k<5; k++)
	{
/*
	itexts[k].FrontPen = 0;
	itexts[k].BackPen =  -1;
	itexts[k].DrawMode = JAM1;
	itexts[k].TopEdge = topedges[k];
	itexts[k].LeftEdge = 0;
	itexts[k].ITextFont = &topaz8;
	itexts[k].IText = tarray[k];
	itexts[k].NextText = NULL;
*/
	itexts[k].FrontPen = AUTOFRONTPEN;
	itexts[k].BackPen =  AUTOBACKPEN;
	itexts[k].DrawMode = AUTODRAWMODE;
	itexts[k].TopEdge = topedges[k];
	itexts[k].LeftEdge = AUTOLEFTEDGE;
	itexts[k].ITextFont = &topaz8;
	itexts[k].IText = tarray[k];
	itexts[k].NextText = NULL;

#ifdef MYDEBUG
	printf("tarray[k] = $%lx (%s)\n",tarray[k],tarray[k]);
#endif
	}

if(t2) itexts[0].NextText = &itexts[1];
if(t3) itexts[1].NextText = &itexts[2];

#ifdef MYDEBUG
printf("Press <RET> to open requester: ");
getchar();
#endif
response = AutoRequest(win,&itexts[0],&itexts[3],&itexts[4],
			pflags,nflags,WIDTH,HEIGHT);

#ifdef MYDEBUG
printf("Back from requester... press <RET> to exit: ");
getchar();
#endif

return(response);
}
 
