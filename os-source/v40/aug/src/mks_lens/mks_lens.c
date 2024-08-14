/*
 * Lens - By Michael Sinz
 */

#include	<exec/types.h>
#include	<exec/memory.h>
#include	<exec/lists.h>
#include	<exec/nodes.h>
#include	<intuition/intuition.h>
#include	<intuition/screens.h>
#include	<intuition/intuitionbase.h>
#include	<graphics/gfx.h>
#include	<graphics/regions.h>
#include	<devices/inputevent.h>

#include	<clib/exec_protos.h>
#include	<pragmas/exec_pragmas.h>

#include	<clib/intuition_protos.h>
#include	<pragmas/intuition_pragmas.h>

#include	<clib/graphics_protos.h>
#include	<pragmas/graphics_pragmas.h>

#include	<clib/layers_protos.h>
#include	<pragmas/layers_pragmas.h>

#include	<clib/dos_protos.h>
#include	<pragmas/dos_pragmas.h>

#include	<stdio.h>

#include	"Magnify.h"

struct MyLens
{
struct	Window		*window;
struct	Region		*region;
struct	BitMap		Normal;
struct	BitMap		Big;
struct	Rectangle	Rect;

	SHORT		NormalX;
	SHORT		NormalY;
	SHORT		BigX;
	SHORT		BigY;

	LONG		Delay;
	SHORT		Crosshair;
	UBYTE		MagX;
	UBYTE		MagY;
	SHORT		Grid;
};

static	char	FontName[]="topaz.font";

static	struct	TextAttr TOPAZ60 = {(STRPTR)FontName,TOPAZ_SIXTY,0,0};
static	struct	TextAttr TOPAZ80 = {(STRPTR)FontName,TOPAZ_EIGHTY,0,0};

extern	struct	SysBase		*SysBase;
	struct	IntuitionBase	*IntuitionBase;
	struct	GfxBase		*GfxBase;
	struct	DOSBase		*DOSBase;
	struct	LayersBase	*LayersBase;


#define	X_PLUS	1
#define	X_MINUS	2
#define	Y_PLUS	3
#define	Y_MINUS	4
#define	GRID_T	5
#define	DELAY_G	6
#define	CROSS_H	7
#define	JUMP_W	8

#define	DELAY_VALUE	3

SHORT SetUp_BitMaps(struct MyLens *MyData)
{
register	SHORT	Flag=TRUE;
register	SHORT	x;
register	SHORT	y;
register	SHORT	z;
register	SHORT	loop;

	for (loop=0;loop<7;loop++)
	{
		MyData->Big.Planes[loop]=NULL;
		MyData->Normal.Planes[loop]=NULL;
	}
	MyData->region=NULL;

	y=MyData->window->Height
		- (MyData->Rect.MinY=MyData->window->BorderTop)
		- (MyData->Rect.MaxY=MyData->window->BorderBottom)
		+ MyData->MagY - 1;

	x=MyData->window->Width
		- (MyData->Rect.MinX=MyData->window->BorderLeft)
		- (MyData->Rect.MaxX=MyData->window->BorderRight)
		+ MyData->MagX - 1;

	MyData->Rect.MaxX=MyData->window->Width-MyData->Rect.MaxX-1;
	MyData->Rect.MaxY=MyData->window->Height-MyData->Rect.MaxY-1;

	z=MyData->window->WScreen->BitMap.Depth;

	if ((x<MyData->MagX)||(y<MyData->MagY)) Flag=FALSE;
	else
	{
		if (!(MyData->region=NewRegion())) Flag=FALSE;
		else
		{
			OrRectRegion(MyData->region,&(MyData->Rect));
			InstallClipRegion(MyData->window->RPort->Layer,MyData->region);

			InitBitMap(&(MyData->Big),z,MyData->BigX=x,MyData->BigY=y);
			for (loop=0;loop<z;loop++)
			{
				if (!(MyData->Big.Planes[loop]=AllocRaster(x,y))) Flag=FALSE;
			}

			x=x / MyData->MagX;
			y=y / MyData->MagY;
			InitBitMap(&(MyData->Normal),z,MyData->NormalX=x,MyData->NormalY=y);
			for (loop=0;loop<z;loop++)
			{
				if (!(MyData->Normal.Planes[loop]=AllocRaster(x,y))) Flag=FALSE;
			}
		}
	}
	return(Flag);
}

VOID Clear_BitMaps(struct MyLens *MyData)
{
register	SHORT	loop;

	for (loop=0;loop<7;loop++)
	{
		if (MyData->Big.Planes[loop])
		{
			FreeRaster(MyData->Big.Planes[loop],MyData->BigX,MyData->BigY);
			MyData->Big.Planes[loop]=NULL;
		}
		if (MyData->Normal.Planes[loop])
		{
			FreeRaster(MyData->Normal.Planes[loop],MyData->NormalX,MyData->NormalY);
			MyData->Normal.Planes[loop]=NULL;
		}
	}
	if (MyData->region)
	{
		InstallClipRegion(MyData->window->RPort->Layer,NULL);
		DisposeRegion(MyData->region);
		MyData->region=NULL;
	}
}

short MainLoop(struct MyLens *MyData)
{
register		SHORT		ExitFlag=FALSE;
register	struct	IntuiMessage	*msg;
register	struct	Gadget		*gad;
register	struct	Screen		*sc;
register	struct	RastPort	*rp;
register		SHORT		x;
register		SHORT		y;
register		SHORT		x1;
register		SHORT		y1;
register		SHORT		x_zero=0;
register		SHORT		y_zero=0;
register		SHORT		xlock;
register		SHORT		ylock;
register		SHORT		JumpFlag=FALSE;
register		SHORT		lock=FALSE;
		struct	Region		*rtmp;
			char		myText[14];
		struct	IntuiText	MyText = {1,0,JAM2,62,-9,&TOPAZ80,NULL,NULL};

	MyText.IText=myText;
	while (!ExitFlag)
	{
		/* Do Magnify */
		sc=MyData->window->WScreen;

		if (!lock)
		{
			xlock=sc->MouseX;
			ylock=sc->MouseY;
		}

		x=(x1=xlock) - ((MyData->NormalX) >> 1);
		if (x<0) x=0;
		else if ((x+MyData->NormalX) > sc->Width) x=sc->Width-MyData->NormalX;

		y=(y1=ylock) - ((MyData->NormalY) >> 1);
		if (y<0) y=0;
		else if ((y+MyData->NormalY) > sc->Height) y=sc->Height-MyData->NormalY;

		sprintf(myText,"%4ld,%-4ld",(LONG)(xlock-x_zero),(LONG)(ylock-y_zero));
		MyText.TopEdge=MyData->window->Height-9;
		MyText.FrontPen=1;
		MyText.BackPen=0;
		if (MyData->window->Flags & WFLG_WINDOWACTIVE)
		{
			MyText.FrontPen=2;	/* Kludge!!! */
			MyText.BackPen=3;
		}

		/* Render the coordinates */
		rtmp=InstallClipRegion(MyData->window->RPort->Layer,NULL);
		PrintIText(MyData->window->RPort,&MyText,0,0);
		InstallClipRegion(MyData->window->RPort->Layer,rtmp);

		if ((MyData->MagX==1) && (MyData->MagY==1))
		{
			BltBitMapRastPort(&(sc->BitMap),x,y,
					MyData->window->RPort,
					MyData->window->BorderLeft,
					MyData->window->BorderTop,
					MyData->BigX,
					MyData->BigY,
					0xC0);
		}
		else
		{
			BltBitMap(&(sc->BitMap),x,y,
					&(MyData->Normal),0,0,
					MyData->NormalX,MyData->NormalY,
					0xC0,0xFF,NULL);
			WaitBlit();

			MagnifyBitMap(&(MyData->Normal),&(MyData->Big),
					MyData->MagX,MyData->MagY,
					MyData->NormalX,
					MyData->Grid);

			BltBitMapRastPort(&(MyData->Big),0,0,
					MyData->window->RPort,
					MyData->window->BorderLeft,
					MyData->window->BorderTop,
					MyData->BigX,
					MyData->BigY,
					0xC0);
		}

		if (MyData->Crosshair)
		{	/* Draw the cross hair... */
			SetDrMd(rp=MyData->window->RPort,COMPLEMENT);
			x1=((x1-x)*(MyData->MagX))+((MyData->MagX)>>1)+(MyData->window->BorderLeft)-1;
			y1=((y1-y)*(MyData->MagY))+((MyData->MagY)>>1)+(MyData->window->BorderTop)-1;
			Move(rp,x1,MyData->window->BorderTop);
			Draw(rp,x1,MyData->BigY+MyData->window->BorderTop);
			Move(rp,MyData->window->BorderLeft,y1);
			Draw(rp,MyData->BigX+MyData->window->BorderLeft,y1);
		}

		if (MyData->Delay) Delay(MyData->Delay);

		if (msg=(struct IntuiMessage *)GetMsg(MyData->window->UserPort))
		{
			if (msg->Class==NEWSIZE)
			{
				Clear_BitMaps(MyData);
				if (!SetUp_BitMaps(MyData)) ExitFlag=TRUE;
				RefreshWindowFrame(MyData->window);
			}
			else if (msg->Class==RAWKEY)
			{
				/*
				 * Check for scoll lock  (F10 for A1000 people)
				 */
				if ((msg->Code==0x5B) || (msg->Code==0x59)) lock=!lock;
				else if (msg->Code==0x50) /* Clear 0,0 */
				{
					x_zero=0;
					y_zero=0;
				}
				else if (msg->Code==0x51) /* Set 0,0 */
				{
					x_zero=xlock;
					y_zero=ylock;
				}

			}
			else if (msg->Class==CLOSEWINDOW) ExitFlag=TRUE;
			else if ((msg->Class==GADGETUP)||(msg->Class==GADGETDOWN))
			{
				gad=(struct Gadget *)(msg->IAddress);
				switch (gad->GadgetID)
				{
				case JUMP_W:	JumpFlag=TRUE;
						ExitFlag=TRUE;
						break;
				case X_PLUS:	if (MyData->MagX<16)
						{
							Clear_BitMaps(MyData);
							MyData->MagX++;
							if ((msg->Qualifier & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) &&
								(MyData->MagY<16)) MyData->MagY++;
							if (!SetUp_BitMaps(MyData)) ExitFlag=TRUE;
						}
						break;
				case X_MINUS:	if (MyData->MagX>1)
						{
							Clear_BitMaps(MyData);
							MyData->MagX--;
							if ((msg->Qualifier & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) &&
								(MyData->MagY>1)) MyData->MagY--;
							if (!SetUp_BitMaps(MyData)) ExitFlag=TRUE;
						}
						break;
				case Y_PLUS:	if (MyData->MagY<16)
						{
							Clear_BitMaps(MyData);
							MyData->MagY++;
							if ((msg->Qualifier & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) &&
								(MyData->MagX<16)) MyData->MagX++;
							if (!SetUp_BitMaps(MyData)) ExitFlag=TRUE;
						}
						break;
				case Y_MINUS:	if (MyData->MagY>1)
						{
							Clear_BitMaps(MyData);
							MyData->MagY--;
							if ((msg->Qualifier & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) &&
								(MyData->MagX>1)) MyData->MagX--;
							if (!SetUp_BitMaps(MyData)) ExitFlag=TRUE;
						}
						break;
				case GRID_T:	MyData->Grid=((gad->Flags & SELECTED)!=0);
						break;
				case CROSS_H:	MyData->Crosshair=((gad->Flags & SELECTED)!=0);
						break;
				case DELAY_G:	MyData->Delay=((gad->Flags & SELECTED) ? 0 : DELAY_VALUE );
						break;
				}
			}
			ReplyMsg((struct Message *)msg);
		}
	}

	return(JumpFlag);
}

/**********************************************************************/

/*
 * The data for the window and gadgets...
 */

static	char	Grid[2]="#";
static	char	Plus[2]="+";
static	char	Minus[2]="-";
static	char	Speed[2]="F";
static	char	Jump[2]="J";
static	char	Title[]="Lens";

static	SHORT	BorderVectors1[10] = {0,0,13,0,13,8,0,8,0,1};
static	SHORT	BorderVectors2[10] = {0,0,11,0,11,8,0,8,0,1};

static	struct	Border	Border1 = {0,0,1,0,COMPLEMENT,5,BorderVectors1,NULL};
static	struct	Border	Border2 = {0,0,1,0,COMPLEMENT,5,BorderVectors2,NULL};

static	struct	IntuiText	IText1 = {1,0,COMPLEMENT,2,1,&TOPAZ60,Minus,NULL};
static	struct	IntuiText	IText2 = {1,0,COMPLEMENT,1,1,&TOPAZ60,Minus,NULL};
static	struct	IntuiText	IText3 = {1,0,COMPLEMENT,1,1,&TOPAZ60,Plus,NULL};
static	struct	IntuiText	IText4 = {1,0,COMPLEMENT,2,1,&TOPAZ60,Plus,NULL};
static	struct	IntuiText	IText5 = {1,0,COMPLEMENT,1,1,&TOPAZ60,Grid,NULL};
static	struct	IntuiText	IText6 = {1,0,COMPLEMENT,2,1,&TOPAZ80,Speed,NULL};
static	struct	IntuiText	IText7 = {1,0,COMPLEMENT,1,1,&TOPAZ80,Jump,NULL};
static	struct	IntuiText	IText8 = {1,0,COMPLEMENT,1,1,&TOPAZ60,Plus,NULL};

static	struct	Gadget	Gadget8 = {NULL,18,-9,12,9,GRELBOTTOM,RELVERIFY|GADGIMMEDIATE|TOGGLESELECT|BOTTOMBORDER,BOOLGADGET,(APTR)&Border2,NULL,&IText8,NULL,NULL,CROSS_H,NULL};
static	struct	Gadget	Gadget7 = {&Gadget8,46,-9,12,9,GRELBOTTOM,RELVERIFY|BOTTOMBORDER,BOOLGADGET,(APTR)&Border2,NULL,&IText7,NULL,NULL,JUMP_W,NULL};
static	struct	Gadget	Gadget6 = {&Gadget7,32,-9,12,9,GRELBOTTOM,RELVERIFY|GADGIMMEDIATE|BOTTOMBORDER|TOGGLESELECT,BOOLGADGET,(APTR)&Border2,NULL,&IText6,NULL,NULL,DELAY_G,NULL};
static	struct	Gadget	Gadget5 = {&Gadget6,4,-9,12,9,GRELBOTTOM,RELVERIFY|GADGIMMEDIATE|TOGGLESELECT|BOTTOMBORDER,BOOLGADGET,(APTR)&Border2,NULL,&IText5,NULL,NULL,GRID_T,NULL};
static	struct	Gadget	Gadget4 = {&Gadget5,-15,-19,14,9,GRELBOTTOM|GRELRIGHT,RELVERIFY|RIGHTBORDER,BOOLGADGET,(APTR)&Border1,NULL,&IText1,NULL,NULL,Y_MINUS,NULL};
static	struct	Gadget	Gadget3 = {&Gadget4,-42,-9,12,9,GRELBOTTOM|GRELRIGHT,RELVERIFY|BOTTOMBORDER,BOOLGADGET,(APTR)&Border2,NULL,&IText2,NULL,NULL,X_MINUS,NULL};
static	struct	Gadget	Gadget2 = {&Gadget3,-29,-9,12,9,GRELBOTTOM|GRELRIGHT,RELVERIFY|BOTTOMBORDER,BOOLGADGET,(APTR)&Border2,NULL,&IText3,NULL,NULL,X_PLUS,NULL};
static	struct	Gadget	Gadget1 = {&Gadget2,-15,-29,14,9,GRELBOTTOM|GRELRIGHT,RELVERIFY|RIGHTBORDER,BOOLGADGET,(APTR)&Border1,NULL,&IText4,NULL,NULL,Y_PLUS,NULL};

static struct NewWindow nw =
{
	0,0,
	0,0,
	-1,-1,
	NEWSIZE
	| GADGETUP
	| GADGETDOWN
	| RAWKEY
	| CLOSEWINDOW,
	WINDOWSIZING
	| WINDOWDRAG
	| WINDOWDEPTH
	| WINDOWCLOSE
	| SIZEBRIGHT
	| SIMPLE_REFRESH
	| RMBTRAP
	| NOCAREREFRESH,
	&Gadget1,
	NULL,
	Title,
	NULL,
	NULL,
	176,30,
	-1,-1,
	WBENCHSCREEN
};

/**********************************************************************/

struct	MyToolPref
{
	short	LeftEdge;
	short	TopEdge;
	short	Width;
	short	Height;
	short	Grid;
	UBYTE	MagX;
	UBYTE	MagY;
	LONG	Delay;
	short	Crosshair;
};

static	char	MyPrefFile[]="ENV:MKSoft Lens";

VOID main(VOID)
{
struct	MyToolPref	MyPref;
struct	MyLens		MyData;
	BPTR		File;
struct	Process		*pr;
	APTR		oldwin;
struct	Screen		sc;
struct	Screen		*OpenScreen=NULL;

	pr=(struct Process *)FindTask(NULL);
	oldwin=pr->pr_WindowPtr;
	pr->pr_WindowPtr=(APTR)(-1);

	MyData.MagX=4;
	MyData.MagY=4;
	MyData.Grid=TRUE;
	MyData.Delay=DELAY_VALUE;
	MyData.Crosshair=FALSE;

	if (File=Open(MyPrefFile,MODE_OLDFILE))
	{
		if (sizeof(struct MyToolPref) ==
		    Read(File,(UBYTE *)&MyPref,sizeof(struct MyToolPref)))
		{
			nw.LeftEdge=MyPref.LeftEdge;
			nw.TopEdge=MyPref.TopEdge;
			nw.Width=MyPref.Width;
			nw.Height=MyPref.Height;
			MyData.MagX=MyPref.MagX;
			MyData.MagY=MyPref.MagY;
			MyData.Grid=MyPref.Grid;
			MyData.Delay=MyPref.Delay;
			MyData.Crosshair=MyPref.Crosshair;
		}
		Close(File);
	}

	if (MyData.Grid) Gadget5.Flags|=SELECTED;
	if (MyData.Crosshair) Gadget8.Flags|=SELECTED;
	if (!(MyData.Delay)) Gadget6.Flags|=SELECTED;

	if (IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",33L))
	{
		if (GfxBase=(struct GfxBase *)OpenLibrary("graphics.library",33L))
		{
			if (LayersBase=(struct LayersBase *)OpenLibrary("layers.library",33L))
			{
			short	JumpFlag=TRUE;

				Forbid();
				while (JumpFlag)
				{
					JumpFlag=FALSE;

					if (!OpenScreen) GetScreenData(&sc,sizeof(struct Screen),nw.Type=WBENCHSCREEN,nw.Screen=NULL);
					else GetScreenData(&sc,sizeof(struct Screen),nw.Type=((OpenScreen->Flags)&SCREENTYPE),nw.Screen=OpenScreen);

					nw.MinWidth=176+sc.WBorRight+sc.WBorLeft;
					nw.MinHeight=30+sc.WBorBottom+sc.WBorTop+sc.BarHeight-sc.BarVBorder;

					if (nw.Width<nw.MinWidth) nw.Width=nw.MinWidth;
					if (nw.Height<nw.MinHeight) nw.Height=nw.MinHeight;

					if (!(MyData.window=OpenWindow(&nw)))
					{
						nw.LeftEdge=0;
						nw.TopEdge=0;
						if (!(MyData.window=OpenWindow(&nw)))
						{
							nw.Width=nw.MinWidth;
							nw.Height=nw.MinHeight;
							MyData.window=OpenWindow(&nw);
						}
					}

					Permit();

					if (MyData.window)
					{
						if (SetUp_BitMaps(&MyData)) JumpFlag=MainLoop(&MyData);
						Clear_BitMaps(&MyData);

						if (JumpFlag)
						{
							Forbid();
							if (OpenScreen=MyData.window->WScreen->NextScreen)
							{
								ScreenToBack(MyData.window->WScreen);
							}
							else OpenScreen=IntuitionBase->FirstScreen;

							nw.LeftEdge=MyData.window->LeftEdge;
							nw.TopEdge=MyData.window->TopEdge;
							nw.Width=MyData.window->Width;
							nw.Height=MyData.window->Height;
						}
						else
						{
							/* Save preferences */
							MyPref.LeftEdge=MyData.window->LeftEdge;
							MyPref.TopEdge=MyData.window->TopEdge;
							MyPref.Width=MyData.window->Width;
							MyPref.Height=MyData.window->Height;
							MyPref.MagX=MyData.MagX;
							MyPref.MagY=MyData.MagY;
							MyPref.Grid=MyData.Grid;
							MyPref.Delay=MyData.Delay;
							MyPref.Crosshair=MyData.Crosshair;
							if (File=Open(MyPrefFile,MODE_NEWFILE))
							{
								Write(File,(UBYTE *)&MyPref,sizeof(struct MyToolPref));
								Close(File);
							}
						}

						CloseWindow(MyData.window);
					}
				}
				CloseLibrary((struct Library *)LayersBase);
			}
			CloseLibrary((struct Library *)GfxBase);
		}
		CloseLibrary((struct Library *)IntuitionBase);
	}

	pr->pr_WindowPtr=oldwin;
}
