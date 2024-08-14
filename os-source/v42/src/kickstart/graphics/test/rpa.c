/* test readpixelline8 */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <graphics/rastport.h>
#include <graphics/gfx.h>

struct IntuitionBase *IntuitionBase;
ULONG GfxBase;

UBYTE pixarray[15];

main()
{
	int i;
	struct BitMap *mybm;
	struct BitMap *tempbm;
	struct RastPort temprp;
	struct RastPort *wrp;
	IntuitionBase=OpenLibrary("intuition.library",0);
	GfxBase=OpenLibrary("graphics.library",0);
	mybm=&(IntuitionBase->FirstScreen->BitMap);
	printf("bm_pad=%04lx\n",mybm->pad);
	tempbm=AllocBitMap(320,1,2,0,mybm);
	InitRastPort(&temprp);
	temprp.BitMap=tempbm;
	wrp=IntuitionBase->ActiveWindow->RPort;
	ReadPixelLine8(wrp,0,0,15,pixarray,&temprp);
	for(i=0;i<15;i++) printf("%02lx ",pixarray[i]);
	printf("\n");

}
