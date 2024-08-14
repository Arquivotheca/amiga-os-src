/* $$TABS=4 */

#include <exec/types.h>
#include <clib/exec_protos.h>
#include <clib/game_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/game_pragmas.h>
#include <graphics/view.h>
#include <graphics/gfxbase.h>
#include <graphics/videocontrol.h>
#include <graphics/displayinfo.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <libraries/dos.h>
#include <libraries/game.h>
#include <graphics/sprite.h>
#include <stdio.h>


/* This demo demonstrates the following Amiga CD 32 features:

	Wide Sprites
	Fast Scrolling
	Using bordersprites to make multiple viewports look like
	   a single viewport.

	Relatively "safe" stealing of sprite 0 from intuition.


*/



#define NSLICES 4
#define VPMODE (0x8000 | SPRITES)
#define SLICESPACING (200/NSLICES)
#define VPWIDTH 640
#define VPHEIGHT (SLICESPACING-2)
#define BMWIDTH (VPWIDTH*2)
#define BMHEIGHT VPHEIGHT
#define BMDEPTH 4

/*#define SETCOLOR(x) *((UWORD *) 0xdff180)=x */
#define SETCOLOR(x)


struct View myview;
struct ViewPort myvp[NSLICES];
struct ViewPortExtra *myvpe[NSLICES];
struct RasInfo myri[NSLICES];
struct ColorMap *mycm[NSLICES];
struct BitMap *slicebm;
struct RastPort myrp;

WORD rx[NSLICES],dx[NSLICES]={ -7*32, 6*32,-6*32,7*32 };

struct GfxBase *GfxBase;

extern UBYTE far Logo[];

int oldp=65536;


UBYTE got[8]={255,255,255,255,255,255,255,255};

struct ExtSprite *boing00[20],*boing01[20],*boing10[20],*boing11[20],*boing20[20],*boing21[20];
int spx[4],spy[4],spdx[4],spdy[4];
struct View *oldview;

struct Library *GameBase;

struct TagItem SysControlTags[] = {{SCON_StopInput, TRUE}, {TAG_DONE, TRUE}};

void Fail(char *msg)
{
	int i;
	if (oldp != 65536) SetTaskPri(FindTask(0),oldp);
	if (GfxBase!=NULL)
	{
		if (GfxBase->ActiView==&myview) { LoadView(oldview); WaitTOF(); }
		for(i=0;i<8;i++) if (got[i] != 255) FreeSprite(got[i]);
		for(i=0;i<20;i++)
		{
			if (boing00[i]) FreeSpriteData(boing00[i]);
			if (boing01[i]) FreeSpriteData(boing01[i]);
			if (boing10[i]) FreeSpriteData(boing10[i]);
			if (boing11[i]) FreeSpriteData(boing11[i]);
			if (boing20[i]) FreeSpriteData(boing20[i]);
			if (boing21[i]) FreeSpriteData(boing21[i]);
		}
		for(i=0;i<NSLICES;i++)
		{
			if (mycm[i]) FreeColorMap(mycm[i]);
			FreeVPortCopLists(myvp+i);
			if (myvpe[i]) GfxFree(myvpe[i]);
		}

		if (myview.LOFCprList) FreeCprList(myview.LOFCprList);
		if (myview.SHFCprList) FreeCprList(myview.SHFCprList);
		if (slicebm) FreeBitMap(slicebm);
		CloseLibrary((struct Library *) GfxBase);
	}
	if (GameBase!=NULL)
	{
		SysControlTags[0].ti_Data=FALSE;
		SystemControlA(SysControlTags);
		CloseLibrary(GameBase);
	}
	if (msg) {printf("%s\n",msg); exit(0);}
}


struct Library *openlib(char *name,ULONG version)
{
	struct Library *t1;
	t1=OpenLibrary(name,version);
	if (! t1)
	{
		printf("error- needs %s version %d\n",name,version);
		Fail(0l);
	}
	else return(t1);
}




ULONG colortab[3*16+2];

struct BitMap spbitmap;
UBYTE bmdata[16*100*2];

char fname[]="progdir:boing.0.00.lo.128x100x2";

main()
{
	int i,j,tempc[3],fhandle,newsp,lastsp;

	GfxBase=(struct GfxBase *) openlib("graphics.library",40);
	GameBase=openlib("game.library",40);

	InitBitMap(&spbitmap,2,128,100);
	for(i=0;i<20;i++)
	{
		fname[6+8]=((i+i)/10)+'0';
		fname[8+8]=((i+i) % 10)+'0';
		fhandle=Open(fname,MODE_OLDFILE);
		if (! fhandle) Fail("can't open file");
		Read(fhandle,bmdata,16*100*2);
		Close(fhandle);
		spbitmap.Planes[0]=bmdata;
		spbitmap.Planes[1]=bmdata+16*100;
		boing00[i]=(struct ExtSprite *) AllocSpriteData(&spbitmap,SPRITEA_Width,64,0);
		boing10[i]=(struct ExtSprite *) AllocSpriteData(&spbitmap,SPRITEA_Width,64,0);
		boing20[i]=(struct ExtSprite *) AllocSpriteData(&spbitmap,SPRITEA_Width,64,0);
		if (! boing00[i]) Fail("can't get sprite ram");
		if (! boing10[i]) Fail("can't get sprite ram");
		if (! boing20[i]) Fail("can't get sprite ram");
		spbitmap.Planes[0]+=8; spbitmap.Planes[1]+=8;
		boing01[i]=(struct ExtSprite *) AllocSpriteData(&spbitmap,SPRITEA_Width,64,0);
		boing11[i]=(struct ExtSprite *) AllocSpriteData(&spbitmap,SPRITEA_Width,64,0);
		boing21[i]=(struct ExtSprite *) AllocSpriteData(&spbitmap,SPRITEA_Width,64,0);
		if (! boing01[i]) Fail("can't get sprite ram");
		if (! boing11[i]) Fail("can't get sprite ram");
		if (! boing21[i]) Fail("can't get sprite ram");
	}

	/* the GetExtSprite calls below will fail if this system does not support
	   4x sprites */

	got[0]=GetExtSprite(boing00[0],GSTAG_SPRITE_NUM,7,0);
	if (got[0]==255)	Fail("can't get extsprite");
	got[1]=GetExtSprite(boing01[0],GSTAG_SPRITE_NUM,1,0);
	if (got[1]==255) Fail("can't get extsprite");
	got[2]=GetExtSprite(boing10[0],GSTAG_SPRITE_NUM,2,0);
	if (got[2]==255)	Fail("can't get extsprite");
	got[3]=GetExtSprite(boing11[0],GSTAG_SPRITE_NUM,3,0);
	if (got[3]==255) Fail("can't get extsprite");
	got[4]=GetExtSprite(boing20[0],GSTAG_SPRITE_NUM,4,0);
	if (got[4]==255)	Fail("can't get extsprite");
	got[5]=GetExtSprite(boing21[0],GSTAG_SPRITE_NUM,5,0);
	if (got[5]==255) Fail("can't get extsprite");
	boing00[0]->es_SimpleSprite.num=0; /* relatively safe way to use sprite 0 */


	slicebm=(struct BitMap *) AllocBitMap(BMWIDTH,BMHEIGHT,BMDEPTH,BMF_CLEAR|BMF_DISPLAYABLE,0);
	if (!slicebm) Fail("can't get bitmap");
	for(i=0;i<BMDEPTH;i++)
	{
		for(j=0;j<BMHEIGHT;j++)
		{
			CopyMem(Logo+i*48*80+j*80,slicebm->Planes[i]+j*slicebm->BytesPerRow,80);
			CopyMem(Logo+i*48*80+j*80,slicebm->Planes[i]+80+j*slicebm->BytesPerRow,80);
		}
	}


	colortab[0]=(16<<16);
	for(i=1;i<16;i++)
	{
		colortab[i*3+1]=i*0x11111111;
		colortab[i*3+2]=((i+2) & 15)*0x11111111;
		colortab[i*3+3]=((i+5) & 15)*0x11111111;
	}

	InitView(&myview);
	for(i=0;i<NSLICES;i++)
	{
		int dark=0;
		mycm[i]=(struct ColorMap *) GetColorMap(32);
		if (! mycm[i]) Fail("can't get colormap");
		InitVPort(myvp+i);
		myvp[i].ColorMap=mycm[i];
		myvp[i].DHeight=VPHEIGHT;
		myvp[i].DWidth=VPWIDTH;
		myvp[i].Modes=VPMODE;
		myvp[i].RasInfo=myri+i;
		myri[i].BitMap=slicebm;
		myvp[i].Next=myview.ViewPort;
		myview.ViewPort=myvp+i;
		myvp[i].DyOffset=i*SLICESPACING;
		myvpe[i]=(struct ViewPortExtra *) GfxNew(VIEWPORT_EXTRA_TYPE);
		if (! myvpe[i]) Fail("can't get vpextra");

		VideoControlTags(mycm[i],VTAG_VIEWPORTEXTRA_SET,myvpe[i],
						VTAG_BORDERSPRITE_SET,-1,0);

		VideoControlTags(mycm[i],VTAG_ATTACH_CM_SET,myvp+i,0);

	/* the tags below cause in (graphics 39.102 and up) ScrollVPort() to not update
	   the intermediate copper lists. This only works safely in custom viewports, as
	   intuition may re-use your intermediate copper lists if someone calls RethinkDisplay().
	   In Kickstart 39.116 and up, intuition handles this properly */

		VideoControlTags(mycm[i],VC_IntermediateCLUpdate,0,0);

		for(j=17;j<32;j+=4)
		{
			SetRGB4CM(mycm[i],j,15-dark,15-dark,15-dark);
			SetRGB4CM(mycm[i],j+1,15-dark,7-(dark/2),7-(dark/2));
			SetRGB4CM(mycm[i],j+2,15-dark,0,0);
			dark+=2;
		}
	/* set sprite to playfield priorities so that one sprite pair is in front, one in the middle,
	   and one behind. See HW manual */

		VideoControlTags(mycm[i],VTAG_PF2_TO_SPRITEPRI_SET, (i&1)?1:2, 0 );
		LoadRGB32(myvp+i,colortab);
		MakeVPort(&myview,myvp+i);
	}
	MrgCop(&myview);
	oldview=GfxBase->ActiView;
	LoadView(&myview);

	spdx[0]=64; spx[0]=0; spdy[0]=32;
	spdx[1]=-64; spx[1]=32*200; spdy[1]=-64;
	spdx[2]=-128; spx[2]=32*100; spdy[2]=32*5;

	lastsp=0;

	/* set task priority to 30 so that beam-synchronized stuff will happen
	reliably. It is NOT safe to call intuition with this high task priority */

	oldp=SetTaskPri(FindTask(0),30);

	if (SystemControlA(SysControlTags)!=0) Fail("can't get system");

	for(i=0;(GfxBase->ActiView==&myview) && ((ReadJoyPort(1) & JPF_BTN2)==0) && ((ReadJoyPort(0) & JPF_BTN2)==0);i++)
	{
		WaitTOF();
		SETCOLOR(0xf00);
		if (i & 1)	/* color cycle */
		{
			tempc[0]=colortab[4]; tempc[1]=colortab[5];	tempc[2]=colortab[6];
			CopyMem(colortab+1+6,colortab+1+3,3*14*4);
			colortab[15*3+1]=tempc[0]; colortab[15*3+2]=tempc[1]; colortab[15*3+3]=tempc[2];
		}
		for(j=0;j<NSLICES;j++)
		{
			SETCOLOR(0xfff);
			myri[j].RxOffset=rx[j]>>5;
			rx[j]+=dx[j];
			rx[j]=(rx[j] % (VPWIDTH*32));
			if (rx[j]<0) rx[j]=VPWIDTH*32+rx[j];
			ScrollVPort(myvp+j);
			SETCOLOR(0x0f0);
			if ((i ^j ) & 1) LoadRGB32(myvp+j,colortab);
		}

		SETCOLOR(0xf);
		MoveSprite(0,(struct SimpleSprite *)boing00[lastsp],spx[0]>>5,(spy[0]>>5));
		MoveSprite(0,(struct SimpleSprite *)boing01[lastsp],(spx[0]>>5)+64,(spy[0]>>5));
		MoveSprite(0,(struct SimpleSprite *)boing10[lastsp],spx[1]>>5,(spy[1]>>5));
		MoveSprite(0,(struct SimpleSprite *)boing11[lastsp],(spx[1]>>5)+64,(spy[1]>>5));
		MoveSprite(0,(struct SimpleSprite *)boing20[lastsp],spx[2]>>5,(spy[2]>>5));
		MoveSprite(0,(struct SimpleSprite *)boing21[lastsp],(spx[2]>>5)+64,(spy[2]>>5));
		newsp=(lastsp+2) % 20;
		SETCOLOR(0xff);
		ChangeExtSprite(0,boing00[lastsp],boing00[newsp],0);
		ChangeExtSprite(0,boing01[lastsp],boing01[newsp],0);
		ChangeExtSprite(0,boing10[lastsp],boing10[newsp],0);
		ChangeExtSprite(0,boing11[lastsp],boing11[newsp],0);
		ChangeExtSprite(0,boing20[lastsp],boing20[newsp],0);
		ChangeExtSprite(0,boing21[lastsp],boing21[newsp],0);
		SETCOLOR(0xf0f);
		lastsp=newsp;
		for(j=0;j<3;j++)
		{
			spx[j]+=spdx[j];
			spy[j]+=spdy[j];
			if ((spx[j] <0) || (spx[j]>((320-128)<<5)))
			{
				spx[j]-=spdx[j];
				spdx[j]=-spdx[j];
			}
			if ((spy[j]<0) || (spy[j]>3200))
			{ spy[j]-=spdy[j]; spdy[j]=-spdy[j]; }
			spdy[j]+=10;
		}
		SETCOLOR(0xf00);
	}
//	for(i=0;!(*((BYTE *) 0xbfe001) & 192); i++) WaitTOF();
	Fail(0);
}
