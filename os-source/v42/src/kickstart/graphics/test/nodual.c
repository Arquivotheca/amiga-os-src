#include <exec/types.h>
#include "/videocontrol.h"
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


main()
{
	int i,c,myvar ;
	GfxBase=OpenLibrary("graphics.library",0l);
	InitView(&myview);
	InitVPort(&myvp);
	myview.ViewPort=&myvp;
	bm=(struct BitMap *) AllocBitMap(320,400,1,0,0);
	bm2=(struct BitMap *) AllocBitMap(640,400,1,0,0);
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
	myvp.ColorMap=GetColorMap(256);
	VideoControlTags(myvp.ColorMap,VC_DUALPF_Disable,-1,0);
	VideoControlTags(myvp.ColorMap,VC_DUALPF_Disable_Query,&myvar,0);
	printf("nodual_query=%ld\n",myvar);

	printf("makevp returned %d\n",MakeVPort(&myview,&myvp));
	printf("mrgcop returned %d\n",MrgCop(&myview));
	SetRast(&myrp,0);
	SetRast(&myrp2,0);
	LoadView(&myview);
	SetRGB4(&myvp,0,0,0,0);
	SetRGB4(&myvp,1,15,0,0);
	SetRGB4(&myvp,2,0,0,15);
	SetRGB4(&myvp,3,15,0,15);
	SetAPen(&myrp,1);
	for(i=0;i<400-5;i+=10)
	{
		RectFill(&myrp,0,i,319,i+5);
	};
	SetAPen(&myrp2,1);
	for(i=0;i<640-5;i+=10)
	{
		RectFill(&myrp2,i,0,i+5,399);
	}
	for(c=0;c<1000;c+=1)
	{
		myri.RyOffset=c % 10;
		myri2.RxOffset=c % 10;
		WaitBOVP(&myvp);
		ScrollVPort(&myvp);
	}
	FreeBitMap(bm);
	FreeBitMap(bm2);

}
