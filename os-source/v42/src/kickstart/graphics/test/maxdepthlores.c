/* lc -L -cfist -v -j73 maxdepthlores quit
 * maxdepthlores.c by Christian Ludwig (CATS)
 * (91.08.02)
 */

/* Hi Chris, this program works 100% of the time if I leave in the Delay()
 calls after the LoadRGB32() calls, but If I take out the Delay() calls,
 CRASH, GURU... I have no idea why, could it be that LoadRGB32() is
 returning too fast?

the problem starts at the line " -------- !!!!!!! ----------- "
*/

/* 
  By the way, I use to bang the hardware to code all my stuff, and I used
 the blitter to write directly to the copper list to change the colors for
 a plasma type effect.  This works great on pre AGA machines, but I need
 to know since  we are not suppose to hit the copper directly anymore,
 how can I acomplish the same task?  (since the Copper list format has
 either expanded or change, and no info is being given)  After all, we
 need some AGA demos!!  :')
*/

/* Lastly, you did a good job on that Tmapdemo, if you would of taken say
 4 weeks instead of 1, you could have made a very nice game!  :')
  But I was wondering if you (or anyone else) at C= has anymore AGA type
 code I can get?  (I am a Certified Developer, and got the source that
 came with the 3.0 disk pack.)  I need more "game" type source so I can
 finish converting my game to meet AGA regulations.  I hit the hardware
 directly, and need to change lots of stuff to make it AGA playable.
 Any ideas/suggestions?

 E-Mail :  22dussia@cs.wmich.edu

 Thanks for any help you can give me!     ->  Dimitri Dussias  <-

*/

#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
#include <graphics/displayinfo.h>
#include <intuition/screens.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>


#include <stdio.h>
#include <stdlib.h>

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
extern struct ExecBase *SysBase;


void Quit(char whytext[],UBYTE failcode)
{
	if (IntuitionBase) CloseLibrary((struct Library *) IntuitionBase);
	if (GfxBase) CloseLibrary((struct Library *) GfxBase);
	printf("%s\n",whytext);
	exit(failcode);
}

void main(void)
{
	ULONG modeID = LORES_KEY;
	DisplayInfoHandle displayhandle;
	struct DimensionInfo dimensioninfo;

	UWORD maxdepth, maxcolors;

	ULONG soerror = NULL,colornum;

	ULONG t1=256l<<16+0;			/*each table is 256 long*/
	int i;
	ULONG z=0;
	ULONG *rgb;
	ULONG *rgb2;
	ULONG *rgb3;
	ULONG *rgb4;

	/*,0xffffffff,0,0,*/
	struct Screen *screen;

	rgb=malloc(sizeof(ULONG)*260*3);
	rgb2=malloc(sizeof(ULONG)*260*3);
	rgb3=malloc(sizeof(ULONG)*260*3);
	rgb4=malloc(sizeof(ULONG)*260*3);
	printf("For a table of Ulongs, (size:%d) we need : %d\n",sizeof(ULONG),sizeof(ULONG)*260*3);
z=0;
	for(i=1;i<255*3;i+=3)
	{	rgb[i]=(((z++)<<24) | (0x00ffffff) );	/*red*/
		rgb[i+1]=0;			/*green*/
		rgb[i+2]=0;			/*blue*/
	}
	rgb[i]=0;
 z=0;
	for(i=1;i<255*3;i+=3)
	{	rgb2[i]=0;	/*red*/
		rgb2[i+1]=(((z++)<<24) | (0x00ffffff) );			/*green*/
		rgb2[i+2]=0;			/*blue*/
	}
	rgb2[i]=0;
z=0;
	for(i=1;i<255*3;i+=3)
	{	rgb3[i]=0;	/*red*/
		rgb3[i+1]=0;			/*green*/
		rgb3[i+2]=(((z++)<<24) | (0x00ffffff) );			/*blue*/
	}
	rgb3[i]=0;
z=0;
	for(i=1;i<255*3;i+=3)
	{	rgb4[i]=(((z++)<<24) | (0x00ffffff) );	/*red*/
		rgb4[i+1]=(((z)<<24) | (0x00ffffff) );			/*green*/
		rgb4[i+2]=(((z)<<24) | (0x00ffffff) );			/*blue*/
	}
	rgb4[i]=0;


	rgb[0]=t1;
	rgb2[0]=t1;
	rgb3[0]=t1;
	rgb4[0]=t1;

	if ((GfxBase=
	(struct GfxBase *) OpenLibrary("graphics.library",36))==NULL)
		Quit("graphics.library is too old <V36",25);

	if ((IntuitionBase=
	(struct IntuitionBase *) OpenLibrary("intuition.library",36))==NULL)
		Quit("intuition.library is too old <V36",25);

	if ((displayhandle=FindDisplayInfo(modeID))==NULL)
		Quit("modeID not found in display database",25);

	if (GetDisplayInfoData(displayhandle,(UBYTE *) &dimensioninfo,
	sizeof(struct DimensionInfo),DTAG_DIMS,NULL)==0)
		Quit("mode dimension info not available",25);

	maxdepth=dimensioninfo.MaxDepth;
	printf("dimensioninfo.MaxDepth=%d\n",(int) maxdepth);

	if (screen=OpenScreenTags(NULL,SA_DisplayID		,modeID,
   SA_Depth			,(UBYTE) maxdepth,
	SA_Title			,"MaxDepth LORES",
	SA_ErrorCode		,&soerror,
	SA_FullPalette	,TRUE,
	TAG_END))

		{
			/* Zowee! we actually got the screen open!
			 *
			 * now let's try drawing into it.
			 */
			maxcolors=1<<maxdepth;
			printf("maxcolors=%d\n",(int) maxcolors);
			for(colornum=0;colornum<maxcolors;++colornum)
			{
				SetAPen(&(screen->RastPort),colornum);
				Move(&(screen->RastPort),colornum,screen->BarHeight + 2);
				Draw(&(screen->RastPort),colornum,199);
			}


/*  Below is where I get the GURU if I remove the Delay() !!!!!!*/
/*     -------- !!!!!!! -----------      */
//			Delay(50*4);
			LoadRGB32(&(screen->ViewPort),rgb);
//			Delay(50*4);
			LoadRGB32(&(screen->ViewPort),rgb2);
/*			Delay(50*4);    remove these and crash!*/
			LoadRGB32(&(screen->ViewPort),rgb3);
/*			Delay(50*4);*/
			LoadRGB32(&(screen->ViewPort),rgb4);
//			Delay(50*1*6);
			CloseScreen(screen);
			free(rgb);
			free(rgb2);
			free(rgb3);
			free(rgb4);
		}

	else
		{
			/*
			 * Hmmm.  Couldn't open the screen.  maybe not
			 * enough CHIP RAM? Maybe not enough chips! ;-)
			 */

			switch(soerror)
			{
				case OSERR_NOCHIPS:
					Quit("Bummer! You need new chips dude!",25);
					break;
				case OSERR_UNKNOWNMODE:
					Quit("Bummer! Unknown screen mode.",25);
					break;
				case OSERR_NOCHIPMEM:
					Quit("Not enough CHIP memory.",25);
					break;
				case OSERR_NOMEM:
					Quit("Not enough FAST memory.",25);
					break;
				default:
					printf("soerror=%d\n",soerror);
					Quit("Screen opening error.",25);
					break;
			}

			Quit("Couldn't open screen.",25);
		}

	Quit("Done.",0);
}
