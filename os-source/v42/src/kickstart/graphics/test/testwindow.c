#include <exec/types.h>
#include <clib/graphics_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/expansion_protos.h>
// #include <pragmas/graphics_pragmas.h>

#include "/bitmap_internal.h"
#include "/gfx.h"
#include "/rastport.h"

#include "stdio.h"
#include "stdlib.h"

#include "igc_gtk.h"
#include "igc_macros.h"

#include ".protos"

struct BitMap mybitmap;
struct RastPort myrp;

struct Library *GfxBase,*IntuitionBase;

#include <sys/types.h>
#include <stdlib.h>

#include <libraries/expansion.h>
#include <libraries/configvars.h>

struct TextAttr myta={"helvetica.font",24,0,0};

#define AMERISTAR	1053	/* mfg number for Ameristar */
#define PROD_A1600GX	20	/* product num of 1600GX */

#define VIDREG	(0x100000)	/* video register base */
#define FB_BASE	(0x200000)	/* frame buffer base */

#define D4_HRZBR	(0x40+4)	/* horizontal blank start register */
#define D4_HRZBF	(0x40+5)	/* horizontal blank end register */
#define D4_VRTBR	(0x40+10)	/* vertical start register */
#define D4_VRTBF	(0x40+11)	/* vertical end register */

#define BT468_LOADDR	(0x80+0)	/* BT468 low order address bits */
#define BT468_REG	(0x80+2)
#define BT468_HIADDR	(0x80+1)	/* BT468 high order address bits */
#define BT468_DATA	(0x80+3)	/* BT468 data register */

UBYTE *fbp;	/* pointer to frame buffer */
volatile ULONG *vp;	/* pointer to video control registers */


int RangeRand(int);

struct Library *ExpansionBase;

/*
 * return pointer to first pixel (upper left corner) in frame buffer
 */
UBYTE *A1600GX_getfbp(void)
{
	char *explib = EXPANSIONNAME;
	struct ConfigDev *cd;
	u_char *base;

	ExpansionBase = (struct Library *)OpenLibrary(explib, 0);
	if(!ExpansionBase){
		return (u_char *)0;
	}

	cd = FindConfigDev((struct ConfigDev *)0, AMERISTAR, PROD_A1600GX);
	if(cd){
		base = cd->cd_BoardAddr;
		fbp = base +  FB_BASE;
		vp = (u_long *)(base + VIDREG);
	}
	CloseLibrary(ExpansionBase);
	ExpansionBase = 0;

	return fbp;
}

/*
 * return number of pixels per line
 */
int A1600GX_getH(void)
{
	int H = -1;
	if(vp){
		H = 8*(vp[D4_HRZBF] - vp[D4_HRZBR]);
	}
	return H;
}

/*
 * return number of lines per frame
 */
int A1600GX_getV(void)
{
	int V = -1;
	if(vp){
		V = (vp[D4_VRTBF] - vp[D4_VRTBR]);
	}
	return V;
}

/*
 * load colormap: values (R[], G[], B[]) -> [start..(start+count-1)]
 */
void SetColor(int index, ULONG r, ULONG g, ULONG b)
{
	vp[BT468_LOADDR] = index;
	vp[BT468_HIADDR] = 0;
	vp[BT468_DATA] = (r>>24);
	vp[BT468_DATA] = (g>>24);
	vp[BT468_DATA] = (b>>24);
}


void InitSprite(void)
{
	unsigned int i;
	for(i=0x400;i<0x7ff;i++)
	{
		vp[BT468_HIADDR]=(i>>8);
		vp[BT468_LOADDR]=(i & 0xff);
		vp[BT468_REG]=(i & 16)?0xcc:0x33;
	}
	vp[BT468_HIADDR]=3;
	vp[BT468_LOADDR]=0;
	vp[BT468_REG]=0xc0;
}

void __asm (*OldMoveSprite) (register __a0 struct ViewPort *vp,register __a1 struct SimpleSprite *ss,
					register __d0 int x, register __d1 int y, register __a6 struct GfxBase *gb);


void MoveSP(int x, int y)
{
	vp[BT468_HIADDR]=3;
	vp[BT468_LOADDR]=1;
	vp[BT468_REG]= x & 255;
	vp[BT468_LOADDR]=2;
	vp[BT468_REG]=(x>>8);
	vp[BT468_LOADDR]=3;
	vp[BT468_REG]=y & 255;
	vp[BT468_LOADDR]=4;
	vp[BT468_REG]=(y>>8);
}

void __asm MyMoveSprite(register __a0 struct ViewPort *vp,register __a1 struct SimpleSprite *ss,
					register __d0 int x, register __d1 int y, register __a6 struct GfxBase *gb)
{
	if (vp!=myvp)
	{
		(*OldMoveSprite)(vp,ss,x,y,gb);
	}
	else 	
		MoveSP(x,y);
}


void Fail(char *msg)
{
	if (msg) printf("%s\n",msg);
	exit(0);
}


UWORD screenpens[]={ ~0 };

void main(void)
{
	int x,y,w,h,i;
	ULONG status,raster;
	struct TextFont *myfont;
	struct Screen *myscreen,*wbscreen;
	struct Window *mywindow;
	ULONG startsecs,endsecs,micros;
	ULONG numpixels=0;
	ULONG rgb[3];
	A1600GX_getfbp();
	IntuitionBase=OpenLibrary("intuition.library",0);
	GfxBase=OpenLibrary("graphics.library",42);
	if (! GfxBase) Fail("can't open graphics v42");
	myfont=OpenDiskFont(&myta);
	InitRastPort(&myrp);
	myrp.BitMap=&mybitmap;
	mybitmap.BytesPerRow=A1600GX_getH();
	mybitmap.Rows=A1600GX_getV();
	mybitmap.Flags=IBMF_CHUNKY | IBMF_FAST;
	mybitmap.Depth=8;
	mybitmap.pad=0x805c;
	mybitmap.Planes[0]=	A1600GX_getfbp();

	wbscreen=LockPubScreen("WorkBench");
	SetColor(0,0,0,0);
	SetColor(1,0xffffffff,0,0);
	SetColor(2,0xffffffff,0xffffffff,0);
	SetColor(3,0xffffffff,0xffffffff,0xffffffff);

	InitSprite();
	for(x=0;x<1000;x+=50)
		for (y=0;y<1000;y+=50)
			MoveSP(x,y);

	myscreen=OpenScreenTags(0,SA_LikeWorkbench,-1,SA_BitMap,&mybitmap,SA_Width,mybitmap.BytesPerRow,SA_Height,
				mybitmap.Rows,SA_DisplayID,0x8000,SA_ShowTitle,0,SA_Quiet,-1,SA_Behind,-1,
				SA_Pens,screenpens,SA_PubName,"pubscreen",SA_Exclusive,-1,0);

	PubScreenStatus(myscreen,0);

	for(i=0;i<256;i++)
	{
		GetRGB32(myscreen->ViewPort.ColorMap,i,1,rgb);
		SetColor(i,rgb[0],rgb[1],rgb[2]);
	}

//	Chunkify(&(wbscreen->BitMap),mybitmap.Planes[0],0,0,800,600,-1,mybitmap.BytesPerRow);
	CurrentTime(&startsecs,&micros);
	SetFont(&myrp,myfont);
	SetAPen(&myrp,0);
//	RectFill(&myrp,0,0,mybitmap.BytesPerRow-1,mybitmap.Rows-1);
	mywindow=OpenWindowTags(0,WA_InnerWidth,800,WA_InnerHeight,600,WA_Title,"MyWindow",
							  WA_CustomScreen,myscreen,
							  WA_SizeGadget,-1, WA_DragBar,-1,WA_DepthGadget,-1,
							  WA_SimpleRefresh,-1,WA_Left,100,WA_Top,100,
							  WA_Activate,-1,
							0);

	

	IGC_WRITE_REG(vp,IGM_W_MIN_XY,IGM_PACK(0,0));
	IGC_WRITE_REG(vp,IGM_W_MAX_XY,IGM_PACK(1000,1000));

	raster=IGC_READ_REG(vp,IGM_RASTER);
		raster=(raster & 0x30000) | 0x1ff00; // | IGC_F_MASK | OVER_SIZED;
	IGC_WRITE_REG(vp,IGM_RASTER,raster);
	IGC_WRITE_REG(vp,IGM_PLANE_MASK,0x55);
	printf("pmask=%08lx\n",IGC_READ_REG(vp,IGM_PLANE_MASK));

	for(y=0; y<400;y+=2)
	for(x=0; x<400;x+=2)
	{
		IGC_WRITE_REG(vp,IGM_FGROUND,x+y);
		IGC_WRITE_2D_META_COORD(vp,IGM_ABS,IGM_XY,IGM_POINT,IGM_PACK(x,y));
		do status=IGC_DRAW_QUAD(vp);
		while (status & IGM_STATUS_QB_BUSY_MASK);
	}


	IGC_WRITE_2D_META_COORD(vp,IGM_ABS,IGM_XY,IGM_QUAD,IGM_PACK(400,130));
	IGC_WRITE_2D_META_COORD(vp,IGM_ABS,IGM_XY,IGM_QUAD,IGM_PACK(470,130));
	IGC_WRITE_2D_META_COORD(vp,IGM_ABS,IGM_XY,IGM_QUAD,IGM_PACK(400,240));
	IGC_WRITE_2D_META_COORD(vp,IGM_ABS,IGM_XY,IGM_QUAD,IGM_PACK(470,240));
	getchar();
	do status=IGC_DRAW_QUAD(vp);
	while (status & IGM_STATUS_QB_BUSY_MASK);
	getchar();

	for(i=0;i<100;i++)
	{
		int x1,y1,color;
#if 0
		SetDrMd(&myrp,JAM2);
		SetAPen(&myrp,RangeRand(256));
		SetBPen(&myrp,RangeRand(256));
		Move(&myrp,RangeRand(mybitmap.BytesPerRow-200),RangeRand(mybitmap.Rows-100));
		Text(&myrp,"Hello There!",12);
#endif
#if 0
		myrp.Mask=0xff;
		SetAPen(&myrp,0);
		SetDrMd(&myrp,JAM1);
		RectFill(&myrp,0,0,mybitmap.BytesPerRow-1,mybitmap.Rows-1);

#endif
		SetAPen(mywindow->RPort,i);
		x=RangeRand(800-200)+mywindow->BorderLeft;
		y=RangeRand(600-200)+mywindow->BorderTop;
		w=100+RangeRand(100);
		h=100+RangeRand(100);
		RectFill(mywindow->RPort,x,y,x+w,y+h);
		numpixels+=(w+1)*(h+1);
#if 0
		for(x1=x-1;x1<=x+w+1;x1++)
			for(y1=y-1;y1<=y+h+1;y1++)
			{
				color=ReadPixel(&myrp,x1,y1);
				if ((x1>=x) && (x1<=x+w) && (y1>=y) && (y1<=y+h))
					color ^= 1;
				if (color) {
					printf("pixel %d %d is bad!\n",x1,y1);
				}
			}

		SetAPen(&myrp,2);
		SetDrMd(&myrp,JAM1);
		WritePixel(&myrp,x,y);
		WritePixel(&myrp,x+w,y);
		WritePixel(&myrp,x+w,y+h);
		WritePixel(&myrp,x,y+h);
#endif
	}
	CurrentTime(&endsecs,&micros);
//	printf("pixel rate=%ld / second\n",numpixels/(endsecs-startsecs));
	for(;;)
	{
		Delay(25);
		if (SetSignal(0,0) & SIGBREAKF_CTRL_C) break;
		for(i=0;i<256;i++)
		{
			GetRGB32(myscreen->ViewPort.ColorMap,i,1,rgb);
			SetColor(i,rgb[0],rgb[1],rgb[2]);
		}
	}
	CloseWindow(mywindow);
	CloseScreen(myscreen);
	CloseFont(myfont);
}

