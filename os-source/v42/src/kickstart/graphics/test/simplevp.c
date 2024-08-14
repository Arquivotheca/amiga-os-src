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

main()
{
	int i,c ;
	GfxBase=OpenLibrary("graphics.library",0l);
	InitView(&myview);
	InitVPort(&myvp);
	myview.ViewPort=&myvp;
	bm=(struct BitMap *) AllocBitMap(320,200,5,0,0);
	InitRastPort(&myrp);
	myri.BitMap=myrp.BitMap=bm;
	myvp.RasInfo=&myri;
	myvp.DWidth=320;
	myvp.DHeight=200;
	printf("makevp returned %d\n",MakeVPort(&myview,&myvp));
	printf("mrgcop returned %d\n",MrgCop(&myview));
	SetRast(&myrp,0);
	LoadView(&myview);
	for(c=0;c<100;c++)
		for(i=0;i<200;i++)
		{
			SetAPen(&myrp,c);
			Move(&myrp,0,i);
			Draw(&myrp,319,199-i);
		}
	FreeBitMap(bm);

}
