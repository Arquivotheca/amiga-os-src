#include <exec/types.h>
#include <graphics/view.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>

struct View myview;
struct ViewPort myvp;
struct RasInfo myri;
struct RasInfo myri2;
struct BitMap *bm;
struct BitMap *bm2;

struct RastPort myrp;
struct RastPort myrp2;
int GfxBase;

struct View *oldview;

main()
{
	int i,c ;
	GfxBase=OpenLibrary("graphics.library",0l);
	oldview=GfxBase->ActiView;
	InitView(&myview);
	InitVPort(&myvp);
	myview.ViewPort=&myvp;
	bm=(struct BitMap *) AllocBitMap(320,400,4,0,0);
	bm2=(struct BitMap *) AllocBitMap(640,400,4,0,0);
	InitRastPort(&myrp);
	InitRastPort(&myrp2);
	myri.BitMap=myrp.BitMap=bm;
	myri2.BitMap=myrp2.BitMap=bm2;
	myri.Next=&myri2;
	myvp.RasInfo=&myri;
	myvp.Modes=DUALPF /*| LACE*/ ;
//	myview.Modes |= LACE;
	myvp.DWidth=320;
	myvp.DHeight=200;
	myvp.ColorMap=GetColorMap(32);
	printf("makevp returned %d\n",MakeVPort(&myview,&myvp));
	printf("mrgcop returned %d\n",MrgCop(&myview));
	SetRast(&myrp,0);
	SetRast(&myrp2,0);
	LoadView(&myview);
	SetRGB4(&myvp,1,15,0,0);
	SetRGB4(&myvp,9,0,15,0);
	SetAPen(&myrp,1);
	for(i=0;i<400;i+=10)
	{
		Move(&myrp,0,i);
		Draw(&myrp,319,i);
	};
	SetAPen(&myrp2,1);
	for(i=0;i<640;i+=10)
	{
		Move(&myrp2,i,0);
		Draw(&myrp2,i,399);
	}
	for(c=0;c<100;c+=1)
	{
		myri.RyOffset=c % 10;
		myri2.RxOffset=c % 10;
		WaitBOVP(&myvp);
		ScrollVPort(&myvp);
	}
	LoadView(oldview);
	WaitTOF();
	FreeBitMap(bm);
	FreeBitMap(bm2);

}
