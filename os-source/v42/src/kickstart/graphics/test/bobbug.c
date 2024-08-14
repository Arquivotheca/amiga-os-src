;/* Execute me to compile with SAS/C 6.2
SC PARM=R NOSTKCHK STRMER NOMINC LINK SMALLCODE SMALLDATA AFP NOICONS bobbug
quit
*/

#define	__USE_SYSBASE
#define	INTUI_V36_NAMES_ONLY

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gels.h>
#include <graphics/gfxmacros.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>

#include <string.h>


extern struct ExecBase		*SysBase;
extern struct DosLibrary	*DOSBase;

struct IntuitionBase			*IntuitionBase;
struct GfxBase					*GfxBase;


struct BobThing
{
	struct Bob		bt_Bob;
	struct VSprite	bt_VSprite;

	LONG				bt_Width;
	LONG				bt_Height;
	LONG				bt_Depth;
};
#define	size_BobThing	(sizeof(struct BobThing))

/* Declare all functions here! */
LONG	__stdargs	__main(	void	);
void	SetBobRect(	struct BobThing	*bt,	struct Rectangle	*rect );
struct BobThing *	GetTheBobThing(	LONG	width,
												LONG	height,
												LONG	depth	);
void FreeTheBobThing( struct BobThing *bt );
LONG	DoTheBugThing(	struct Window		*bigwin,
							struct Window		*smallwin,
							struct BobThing	*bt	);


LONG	__stdargs	__main(	void	)
{
	struct Screen		*scr;
	struct Window		*bigwin;
	struct Window		*smallwin;
	struct BobThing	*bobthing;
	LONG	exitval	=	100;

	if ( IntuitionBase = (struct IntuitionBase *)OpenLibrary( "intuition.library", 39 ) )
	{
		exitval	-=	10;

		if ( GfxBase = (struct GfxBase *)OpenLibrary( "graphics.library", 39 ) )
		{
			exitval	-=	10;

			if ( scr = OpenScreenTags( NULL,
											SA_LikeWorkbench,	TRUE,
											SA_Interleaved,	TRUE,
											SA_Title,			"Bob Bug Mania!  Watch the Big Window's title bar...",
											TAG_END	) )
			{
				exitval	-=	10;

				if ( bigwin = OpenWindowTags( NULL,
											WA_Height,			(scr->Height - scr->BarHeight - 1),
											WA_CustomScreen,	scr,
											WA_IDCMP,			IDCMP_CLOSEWINDOW,
											WA_Title,			"Big Window",
											WA_MinWidth,		64,
											WA_MinHeight,		32,
											WA_MaxWidth,		-1,
											WA_MaxHeight,		-1,
											WA_SizeGadget,		TRUE,
											WA_DragBar,			TRUE,
											WA_DepthGadget,	TRUE,
											WA_CloseGadget,	TRUE,
											WA_SmartRefresh,	TRUE,
											WA_AutoAdjust,		TRUE,
											WA_NewLookMenus,	TRUE,
											TAG_END ) )
				{
					exitval	-=	10;

					if ( smallwin = OpenWindowTags( NULL,
											WA_Left,				bigwin->LeftEdge + bigwin->BorderLeft + (bigwin->Width >> 1),
											WA_Top,				bigwin->TopEdge + bigwin->BorderTop + (bigwin->Height >> 1),
											WA_Width,			(bigwin->Width / 3),
											WA_Height,			(bigwin->Height / 3),
											WA_CustomScreen,	scr,
											WA_Title,			"Small Window",
											WA_MinWidth,		64,
											WA_MinHeight,		32,
											WA_MaxWidth,		-1,
											WA_MaxHeight,		-1,
											WA_SizeGadget,		TRUE,
											WA_DragBar,			TRUE,
											WA_DepthGadget,	TRUE,
											WA_SimpleRefresh,	TRUE,
											WA_AutoAdjust,		TRUE,
											WA_NewLookMenus,	TRUE,
											TAG_END ) )
					{
						exitval	-=	10;

						if ( bobthing = GetTheBobThing( (bigwin->Width >> 1), (bigwin->Height >> 1), scr->RastPort.BitMap->Depth ) )
						{
							exitval = DoTheBugThing( bigwin, smallwin, bobthing );

							WaitPort( bigwin->UserPort );

							FreeTheBobThing( bobthing );
						}
						CloseWindow( smallwin );
					}
					CloseWindow( bigwin );
				}
				CloseScreen( scr );
			}
			CloseLibrary( (struct Library *)GfxBase );
		}
		CloseLibrary( (struct Library *)IntuitionBase );
	}
	return( exitval );
}


void	SetBobRect(	struct BobThing	*bt,	struct Rectangle	*rect )
{
	rect->MinX	=	bt->bt_VSprite.X;
	rect->MinY	=	bt->bt_VSprite.Y;
	rect->MaxX	=	(rect->MinX + bt->bt_Width) - 1;
	rect->MaxY	=	(rect->MinY + bt->bt_Height) - 1;
}


void	DrawPrettyRect(	struct RastPort	*rp,
								struct Rectangle	*rect,
								LONG					mode	)
{
	ULONG	what;
	UWORD	pattern[4];

	what	=	0xE38E38E3;
	pattern[0]	=	(what & 0xFFFF);	what	>>=	((mode == 0) ? 1 : 3);
	pattern[1]	=	(what & 0xFFFF);	what	>>=	((mode == 0) ? 1 : 3);
	pattern[2]	=	(what & 0xFFFF);	what	>>=	((mode == 0) ? 1 : 3);
	pattern[3]	=	(what & 0xFFFF);	what	>>=	((mode == 0) ? 1 : 3);

	SetDrMd( rp, JAM2 );
	SetAPen( rp, ((mode == 0) ? 0 : 1) );
	SetBPen( rp, ((mode == 0) ? 2 : 3) );
	SetOPen( rp, 1 );
	SetAfPt( rp, pattern, 2 );
	RectFill( rp, rect->MinX, rect->MinY, rect->MaxX, rect->MaxY );
	SetAfPt( rp, NULL, 0 );
}


struct BobThing *	GetTheBobThing(	LONG	width,
												LONG	height,
												LONG	depth	)
{
	struct BobThing	*bt;
	UBYTE					*planes;
	LONG					planesize, i;
	UWORD					*smask, *imask, x, y, z;

	struct BitMap		tempbm;
	struct RastPort	temprp;
	struct Rectangle	rect;

	if ( bt = AllocMem( size_BobThing, MEMF_CLEAR ) )
	{
		bt->bt_Width	=	width;
		bt->bt_Height	=	height;
		bt->bt_Depth	=	depth;

		if ( planes	= AllocRaster( width, (height * ((depth * 2) + 2)) ) )
		{
			planesize	=	RASSIZE( width, height );

			/* Do some initialization...
			 */
			bt->bt_Bob.BobVSprite		=	&(bt->bt_VSprite);
			bt->bt_VSprite.VSBob			=	&(bt->bt_Bob);
			bt->bt_VSprite.Flags			=	OVERLAY | SAVEBACK;
			bt->bt_VSprite.Height		=	height;
			bt->bt_VSprite.Width			=	(width + 15) >> 4;
			bt->bt_VSprite.Depth			=	depth;
			bt->bt_VSprite.PlanePick	=	~(-1L << depth);
			bt->bt_VSprite.PlaneOnOff	=	0x00;
			bt->bt_VSprite.ImageData	=	(UWORD *)planes;	planes	+=	(planesize * depth);
			bt->bt_Bob.SaveBuffer		=	(UWORD *)planes;	planes	+=	(planesize * depth);
			bt->bt_Bob.ImageShadow		=	(UWORD *)planes;
			bt->bt_VSprite.CollMask		=	(UWORD *)planes;

			planes	=	(UBYTE *)bt->bt_VSprite.ImageData;

			InitBitMap( &tempbm, depth, width, height );
			for ( i=0; i<depth; i++ )
			{	tempbm.Planes[ i ]	=	planes;	planes	+=	planesize;	}

			InitRastPort( &temprp );
			temprp.BitMap	=	&tempbm;

			/* Initialization of the shadowmask */
			SetRast( &temprp, 1 );

			imask	=	bt->bt_VSprite.ImageData;
			smask	=	bt->bt_Bob.ImageShadow;
			z		=	((width + 15) >> 4) * height;
			for ( x=0; x<depth; x++ )
			{
				for ( y=0; y<z; y++ )	{	*(smask+y) |= *imask++;	}
			}

			SetBobRect( bt, &rect );
			DrawPrettyRect( &temprp, &rect, 0 );
		}
		else
		{
			FreeMem( bt, size_BobThing );
			bt	=	NULL;
		}
	}
	return( bt );
}


void FreeTheBobThing( struct BobThing *bt )
{
	if ( bt )
	{
		/* throw it all away...
		 */
		if ( bt->bt_VSprite.ImageData )
			FreeRaster( (UBYTE *)bt->bt_VSprite.ImageData, bt->bt_Width, (bt->bt_Height * ((bt->bt_Depth * 2) + 2)) );

		FreeMem( bt, size_BobThing );
	}
}


UBYTE	*ScreenTitles[]	=
{
	"The bob is in the window... the bob is in the window...",
	"Drawing in bob's area...",
	"Removing the bob... should restore the background... no problem...",
	"Bob is now in the window, partially obscured...",
	"Removing the partially obscured bob...",
	"Now move the 'Small Window' to see the problem...",
	"Now move the 'Small Window' back over bob to see another problem...",
};

enum
{
	NOTE_BOBINWINDOW,
	NOTE_DRAWBOBAREA,
	NOTE_RESTOREBOBAREA,
	NOTE_OBSCUREDBOB,
	NOTE_REMOVEOBSCURED,
	NOTE_SEETHEBUG,
	NOTE_SEEANOTHER,
};

void	MakeNote(	struct Window	*win,	LONG	note	)
{
	SetWindowTitles( win, ScreenTitles[ note ], (UBYTE *)-1L );
	Delay( 250 );
}


LONG	DoTheBugThing(	struct Window		*bigwin,
							struct Window		*smallwin,
							struct BobThing	*bt	)
{
	struct ViewPort	*vp	=	&(bigwin->WScreen->ViewPort);
	struct RastPort	*rp;

	struct Rectangle	rect;
	struct RastPort	rpclone;
	struct GelsInfo	gelsinfo;
	struct VSprite		dummySpriteA, dummySpriteB;

	int gx,gy;

	rpclone	=	*(bigwin->RPort);
	rp			=	&rpclone;

	memset( &dummySpriteA, 0, sizeof( struct VSprite ) );
	memset( &dummySpriteB, 0, sizeof( struct VSprite ) );
	memset( &gelsinfo, 0, sizeof( struct GelsInfo ) );

	InitGels( &dummySpriteA, &dummySpriteB, &gelsinfo );
	rp->GelsInfo	=	&gelsinfo;

	/* First we will see how things work when
	 * the bob is not obscured...
	 */

	SetAPen(rp,-1);
	for(gx=0;gx<800;gx+=9)
		for(gy=0;gy<500;gy+=7)
			WritePixel(rp,gx,gy);

		AddBob( &bt->bt_Bob, rp );
	for(gx=0;gx<700;gx+=5)
		for(gy=0;gy<500;gy+=7)
	{
		bt->bt_VSprite.X	=	bigwin->BorderLeft+gx;
		bt->bt_VSprite.Y	=	bigwin->BorderTop+gy;
		/* Put the bob on the screen...
		 */
		SortGList( rp );
		DrawGList( rp, vp );


	}
		/* Remove the bob from the screen...
		 */
		RemBob( &bt->bt_Bob );
		SortGList( rp );
		DrawGList( rp, vp );

//	ChangeWindowBox( smallwin, bigwin->LeftEdge + bigwin->BorderLeft, bigwin->TopEdge + bigwin->BorderTop, smallwin->Width, smallwin->Height );


	return( 0 );
}

