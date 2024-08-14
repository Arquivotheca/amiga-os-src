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
void Fail(char *msg)
{
	if (msg) printf("%s\n",msg);
	exit(0);
}


void main(void)
{
	int x,y,w,h,i;
	struct TextFont *myfont;
	ULONG startsecs,endsecs,micros;
	ULONG numpixels=0;
	struct ColorMap *mycolormap;
	ULONG rgb[3];
	A1600GX_getfbp();
	IntuitionBase=OpenLibrary("intuition.library",0);
	GfxBase=OpenLibrary("graphics.library",42);
	if (! GfxBase) Fail("can't open graphics v42");
	myfont=OpenDiskFont(&myta);
	mycolormap=GetColorMap(256);
	for(i=0;i<256;i++)
	{
		GetRGB32(mycolormap,i,1,rgb);
		SetColor(i,rgb[0],rgb[1],rgb[2]);
	}
	FreeColorMap(mycolormap);
	InitRastPort(&myrp);
	myrp.BitMap=&mybitmap;
	mybitmap.BytesPerRow=A1600GX_getH();
	mybitmap.Rows=A1600GX_getV();
	mybitmap.Flags=IBMF_CHUNKY | IBMF_FAST;
	mybitmap.Depth=8;
	mybitmap.pad=0x805c;
	mybitmap.Planes[0]=	A1600GX_getfbp();
	CurrentTime(&startsecs,&micros);
	SetFont(&myrp,myfont);
	SetAPen(&myrp,0);
	RectFill(&myrp,0,0,mybitmap.BytesPerRow-1,mybitmap.Rows-1);
	for(i=0;i<1000;i++)
	{
		int x1,y1,color;
		SetDrMd(&myrp,JAM2);
		SetAPen(&myrp,RangeRand(256));
		SetBPen(&myrp,RangeRand(256));
		Move(&myrp,RangeRand(mybitmap.BytesPerRow-200),RangeRand(mybitmap.Rows-100));
		Text(&myrp,"Hello There!",12);
#if 0
		myrp.Mask=0xff;
		SetAPen(&myrp,0);
		SetDrMd(&myrp,JAM1);
		RectFill(&myrp,0,0,mybitmap.BytesPerRow-1,mybitmap.Rows-1);

		SetAPen(&myrp,i);
		x=RangeRand(mybitmap.BytesPerRow-701);
		y=RangeRand(mybitmap.Rows-701);
		w=400+RangeRand(300);
		h=400+RangeRand(300);
		RectFill(&myrp,x,y,x+w,y+h);
		numpixels+=(w+1)*(h+1);
#endif
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
	CloseFont(myfont);
	CurrentTime(&endsecs,&micros);
//	printf("pixel rate=%ld / second\n",numpixels/(endsecs-startsecs));
}
