/* sprite test - convert pixels in upper left of wb screen to a sprite */

#include <exec/types.h>
#include <intuition/intuition.h>
#include "/sprite.h"
#include <graphics/videocontrol.h>

void *GfxBase, *IntuitionBase;

struct SimpleSprite *AllocSpriteData(bm,tags)
{
	return(AllocSpriteDataA(bm,tags));
}
	

ULONG tags[]={VTAG_SPODD_BASE_SET,0,VTAG_SPEVEN_BASE_SET,0,0};
ULONG mytags[]={SPRITEA_Width,64,0};
ULONG gstags[]={0};



int GetExtSprite(ss,tag)
{
	return(GetExtSpriteA(ss,tag));
}

main()
{
	struct Screen *iscreen;
	struct SimpleSprite *mysp[8];
	struct BitMap mybitmap;
	int sprites[8];
	int i,j;
	GfxBase=OpenLibrary("graphics.library",39);
	IntuitionBase=OpenLibrary("intuition.library",39);
	iscreen=LockPubScreen("Workbench");
	UnlockPubScreen(0,iscreen);

	VideoControl(iscreen->ViewPort.ColorMap,tags);
	RemakeDisplay();

	mybitmap=*(iscreen->RastPort.BitMap);
	for(i=0;i<8;i++)
	{
		sprites[i]=-1;
		mysp[i]=AllocSpriteData(&mybitmap,mytags);
		printf("i=%ld got ss=%08lx\n",i,mysp[i]);
		if (mysp[i])
		{
			sprites[i]=GetExtSprite(mysp[i],gstags);
			printf("gots=%ld\n",sprites[i]);
			if (sprites[i] != -1)
			{
				mybitmap.Planes[0]+=8;
				mybitmap.Planes[1]+=8;
			}
		}
	}

	SetTaskPri(FindTask(0l),22);
	for(j=0;j<60*10;j++)
	{
		WaitTOF();
		for(i=0;i<8;i++)
			if (sprites[i] != -1)
				MoveSprite(&(iscreen->ViewPort),mysp[i],((j<<1) & 511)+i*64,0);
	}

	SetTaskPri(FindTask(0l),0);
	for(i=0;i<8;i++)
	{
		if (sprites[i] != -1) FreeSprite(sprites[i]);
		FreeSpriteData(mysp[i]);
	}
	CloseLibrary(GfxBase);
	CloseLibrary(IntuitionBase);
}
