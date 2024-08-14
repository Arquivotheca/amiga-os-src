/* benchmark and test of wpa8 */

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>

struct Library *GfxBase,*LayersBase;
struct Library *IntuitionBase;
struct Screen *myscreen;
struct Window *mywindow;
struct IntuiMessage *msg;
struct BitMap *tempbm;
struct RastPort myrp;

#pragma libcall GfxBase WriteChunkyPixels 420 4A3210807

void Fail(char *msg)
{
	if (tempbm) FreeBitMap(tempbm);
	if (mywindow) CloseWindow(mywindow);
	if (msg) printf("%s\n",msg);
	if (LayersBase) CloseLibrary(LayersBase);
	if (GfxBase) CloseLibrary(GfxBase);
	if (IntuitionBase) CloseLibrary(IntuitionBase);
	exit(0);
}


struct Library *openlib(char *name,ULONG version)
{
	struct Library *t1;
	t1=(struct Library *) OpenLibrary(name,version);
	if (! t1)
	{
		printf("error- needs %s version %d\n",name,version);
		Fail(0l);
	}
	else return(t1);
}





void redraw(trap)
{
	int width,height,xl,yl,ix,iy,rwidth;
	UBYTE *bytebuff;
	width=mywindow->Width; xl=0;
	yl=mywindow->BorderTop;
	height=mywindow->Height-yl;
	printf("w=%d h=%d x=%d y=%d\n",width,height,xl,yl);
	rwidth=((width+15)>>4)<<4;
	bytebuff=(UBYTE *) AllocMem(rwidth*height,0);
	if (! bytebuff) Fail("no mem for bytebuf");
	for(ix=0;ix<width;ix++)
		for(iy=0;iy<height; iy++)
			bytebuff[ix+iy*rwidth]=(ix+iy)>>2 ; /*((ix ^ iy)>>2) & 1; */
	if(trap) bkpt();
	mywindow->RPort->Mask=-1;
//	WritePixelArray8(mywindow->RPort,xl,yl,xl+width-1,yl+height-1,bytebuff,&myrp);
	WriteChunkyPixels(mywindow->RPort,xl,yl,xl+width-1,yl+height-1,bytebuff,rwidth);
	FreeMem(bytebuff,rwidth*height);
}
	

main(argc,argv)
int argc;
char **argv;
{
	int quit;


	GfxBase=openlib("graphics.library",39l);
	LayersBase=openlib("layers.library",39l);
	IntuitionBase=openlib("intuition.library",39l);
	tempbm=(struct BitMap *) AllocBitMap(1024,1,8,BMF_CLEAR,0);
	if (! tempbm) Fail("can't get tmprast");
	InitRastPort(&myrp);
	myrp.BitMap=tempbm;
	mywindow=OpenWindowTags(0,WA_Left,0,WA_Top,100,WA_Width,160,WA_Height,50,
			WA_Title,"wpa8 test",WA_SizeGadget,-1,WA_DragBar,-1,WA_DepthGadget,-1,
			WA_CloseGadget,-1,WA_NoCareRefresh,-1,WA_SmartRefresh,-1,
			WA_IDCMP,CLOSEWINDOW|VANILLAKEY,WA_MinWidth,1,WA_MinHeight,11,
			WA_MaxWidth,800,WA_MaxHeight,1000,0);
	redraw(0);
	for(quit=0;! quit;)
	{
		Wait(1l<<(mywindow->UserPort->mp_SigBit));
		printf("got wait\n");
		while(msg=(struct IntuiMessage *) GetMsg(mywindow->UserPort))
		{
			printf("cl=%08lx\n",msg->Class);
			switch(msg->Class)
			{
				case CLOSEWINDOW:
					quit=-1;
					break;
				case VANILLAKEY:
					switch(msg->Code)
					{
						case 'b':
						case 'B':
							SetRast(mywindow->RPort,0);
							redraw(1);
							break;
						default:
							redraw(0);
							break;
					}
			}
			ReplyMsg(msg);
		}
	}
	Fail(0);
}
