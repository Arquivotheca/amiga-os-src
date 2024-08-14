#include <exec/types.h>
#include <graphics/view.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>

struct View myview;
struct ViewPort myvp;
struct RasInfo myri;
struct BitMap *bm;

struct RastPort myrp;
int GfxBase;

struct BitMap *abm(int width, int height,int depth)
{
	struct BitMap *bm;
	int i;
	bm=AllocMem(sizeof(*bm),0l);
	InitBitMap(bm,depth,width,height);
	for(i=0;i<depth;i++)
		bm->Planes[i]=AllocRaster(width,height);
	return(bm);
}


main()
{
	int i,c ;
	GfxBase=OpenLibrary("graphics.library",0l);
	InitView(&myview);
	InitVPort(&myvp);
	myview.ViewPort=&myvp;
	bm=(struct BitMap *) abm(1280,400,2);
	InitRastPort(&myrp);
	myri.BitMap=myrp.BitMap=bm;
	myvp.RasInfo=&myri;
	myvp.DWidth=1280;
	myvp.DHeight=400;
	myvp.Modes=0x8020 | LACE;
	myview.Modes |= LACE;
	myvp.ColorMap=GetColorMap(32);
	printf("makevp returned %d\n",MakeVPort(&myview,&myvp));
	printf("mrgcop returned %d\n",MrgCop(&myview));
	SetRast(&myrp,0);
	WritePixel(&myrp,319,100);
	WritePixel(&myrp,639,100);
	LoadView(&myview);
	SetAPen(&myrp,1);
	Move(&myrp,myri.RxOffset,myri.RyOffset);
	Draw(&myrp,myri.RxOffset+myvp.DWidth-1,myri.RyOffset);
	Draw(&myrp,myri.RxOffset+myvp.DWidth-1,myri.RyOffset+myvp.DHeight-1);
	Draw(&myrp,myri.RxOffset,myri.RyOffset+myvp.DHeight-1);
	Draw(&myrp,myri.RxOffset,myri.RyOffset);
	Delay(50*4);


}
