/* sprite test - convert pixels in upper left of wb screen to a sprite */

#include <stdio.h>
#include <exec/types.h>
#include <intuition/intuition.h>
#include "/sprite.h"
#include <graphics/videocontrol.h>

void *GfxBase, *IntuitionBase;

ULONG tags[]={VTAG_SPODD_BASE_SET,0,VTAG_SPEVEN_BASE_SET,0,0};



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
		mysp[i]=0;
	}
	mysp[0]=AllocSpriteData(&mybitmap,SPRITEA_Width,64,0);
	mysp[1]=AllocSpriteData(&mybitmap,SPRITEA_Width,64,SPRITEA_Attached,-1,0);
	sprites[2]=GetExtSprite(mysp[0],GSTAG_SPRITE_NUM,2,0);
	sprites[3]=GetExtSprite(mysp[1],GSTAG_SPRITE_NUM,3,0);
	
	MoveSprite(0,mysp[0],0,0);
	MoveSprite(0,mysp[1],0,0);
	SetTaskPri(FindTask(0l),22);
	for(j=0;j<60*10;j++)
	{
		WaitTOF();
		MoveSprite(&(iscreen->ViewPort),mysp[0],((j<<1) & 511),0);
//		MoveSprite(&(iscreen->ViewPort),mysp[1],((j<<1) & 511),0);
		if (getchar()==-1) goto done;
	}

done:
	SetTaskPri(FindTask(0l),0);
	for(i=0;i<8;i++)
	{
		if (sprites[i] != -1) FreeSprite(sprites[i]);
		FreeSpriteData(mysp[i]);
	}
	CloseLibrary(GfxBase);
	CloseLibrary(IntuitionBase);
}
