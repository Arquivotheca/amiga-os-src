/****************************
        FakeFreeAnim.c

  Simulated freeanim.library

        W.D.L 931013
*****************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/gfxbase.h>
#include <graphics/gfx.h>
#include <graphics/videocontrol.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>

#include <math.h>	// For min() & max()
#include <string.h>	// for setmem()
#include <stdlib.h>
#include <stdio.h>

#include "fakefreeanim.h"

#include "SimFreeAnim_rev.h"

#include "debugsoff.h"

// Uncomment to get debug output turned on
/**/
#define KPRINTF
//#include "debugson.h"

#define	SPR_LEFT	200
#define	SPR_TOP		10


#define	FFVER	VERSTAG " Simulated freeanim.library developer tool. Wayne D. Lutz"

UBYTE	* Version = FFVER;

STATIC struct FFAnimLibrary * FFAnimBase;

#define	CDIMAGE_WID	83
#define	CDIMAGE_HT	35

UWORD CDImageData[] = {

/* Plane # 0 */
	0x02ff,0xfe01,0x001f,0xffff,0xe020,0x0000,0x17ae,0xeeee,
	0x601e,0xeeee,0xeee4,0x0000,0x1fff,0xffff,0xf01f,0xffff,
	0xfffd,0x0000,0x72aa,0xaaaa,0xa81a,0xaaaa,0xaaaa,0x0000,
	0x2ddd,0xdddd,0xdc1d,0xdddd,0xdddd,0x8000,0xcaaa,0xaaaa,
	0xa81a,0xaaaa,0xaaaa,0xc000,0x7555,0x5555,0x5615,0x5555,
	0x5555,0x4000,0xaaaa,0xaaaa,0xaa1a,0xaaaa,0xaaaa,0x8000,
	0xc460,0x0004,0x4414,0x4000,0x00c4,0x4000,0xaac0,0x0001,
	0xab1a,0xa000,0x002a,0xa000,0x8080,0x0000,0x0010,0x0000,
	0x0020,0x0000,0x8000,0x0000,0x8010,0x0000,0x0000,0x0000,
	0x8000,0x0000,0x0010,0x0000,0x0010,0x0000,0x8000,0x0000,
	0x0010,0x0000,0x0010,0x0000,0x8000,0x0000,0x0010,0x0000,
	0x0010,0x0000,0x8000,0x0000,0x0010,0x0000,0x0010,0x0000,
	0x8000,0x0000,0x0000,0x0000,0x0010,0x0000,0x8000,0x0000,
	0x0000,0x0000,0x0010,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0010,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xbf00,0x0000,
	0x0030,0x0000,0x0000,0x0000,0x0000,0x0000,0x0030,0x0000,
	0x0080,0x0001,0x0000,0x0000,0x0040,0x0000,0x0040,0x0002,
	0x0000,0x0000,0x01a0,0x2000,0x003f,0xf80f,0x0110,0x0ffe,
	0x97c0,0x2000,0x8000,0x0000,0x0010,0x0000,0x0000,0x0000,
	0x8000,0x0000,0x0010,0x0000,0x0000,0x0000,0x4000,0x0000,
	0x0010,0x0000,0x0000,0x4000,0x4000,0x0000,0x0210,0x0000,
	0x0000,0xc000,0x0000,0x0000,0x0410,0x0000,0x0000,0x8000,
	0x1000,0x0000,0x0810,0x0000,0x0003,0x0000,0x0400,0x0000,
	0x2010,0x0000,0x000c,0x0000,0x0100,0x0001,0x0010,0x0000,
	0x0030,0x0000,

/* Plane # 1 */
	0x01ff,0xfffe,0x001f,0xffff,0xffc0,0x0000,0x0fd1,0x1111,
	0x8011,0x1111,0x1118,0x0000,0x3c00,0x0000,0x0010,0x0000,
	0x0002,0x0000,0x3d55,0x5555,0x5015,0x5555,0x5555,0x0000,
	0x7222,0x2222,0x2012,0x2222,0x2222,0x0000,0x7555,0x5555,
	0x5415,0x5555,0x5555,0x0000,0xcaaa,0xaaaa,0xa81a,0xaaaa,
	0xaaaa,0x8000,0xd555,0x5555,0x5415,0x5555,0x5555,0x4000,
	0xbb80,0x0003,0xba1b,0xa000,0x003b,0x8000,0xd500,0x0000,
	0x5415,0x4000,0x0015,0x4000,0xff00,0x0000,0xff1f,0xe000,
	0x001f,0xe000,0xff00,0x0000,0x7f1f,0xe000,0x001f,0xe000,
	0xff00,0x0000,0x7f1f,0xe000,0x000f,0xe000,0xff00,0x0000,
	0x001f,0xe000,0x000f,0xe000,0xff00,0x0000,0x001f,0xe000,
	0x000f,0xe000,0xff00,0x0000,0x001f,0xe000,0x000f,0xe000,
	0xff00,0x0000,0x001f,0xe000,0x000f,0xe000,0xff00,0x0000,
	0x001f,0xe000,0x000f,0xe000,0xff00,0x0000,0x001f,0xe000,
	0x001f,0xe000,0xff00,0x0000,0x001f,0xe000,0x000f,0xe000,
	0xff00,0x0000,0x001f,0xe000,0x001f,0xe000,0xff00,0x0000,
	0x001f,0xe000,0x001f,0xe000,0xff00,0x0000,0x7f1f,0xe000,
	0x001f,0xe000,0xff00,0x0000,0xff1f,0xe000,0x001f,0xe000,
	0xff00,0x0000,0xff1f,0xe000,0x003f,0xe000,0xff80,0x0001,
	0xff1f,0xe000,0x007f,0xc000,0xffc0,0x07ff,0xfe0f,0xe001,
	0x7fff,0xc000,0x7fff,0xffff,0xfe0f,0xffff,0xffff,0xc000,
	0x7fff,0xffff,0xfe0f,0xffff,0xffff,0xc000,0x3fff,0xffff,
	0xfe0f,0xffff,0xffff,0x8000,0x3fff,0xffff,0xfc0f,0xffff,
	0xffff,0x0000,0x3fff,0xffff,0xf80f,0xffff,0xffff,0x0000,
	0x0fff,0xffff,0xf00f,0xffff,0xfffc,0x0000,0x03ff,0xffff,
	0xc00f,0xffff,0xfff0,0x0000,0x00ff,0xfffe,0x000f,0xffff,
	0xffc0,0x0000,

};

/*
 * clipit - passed width and height of a display, and the text, std, and
 *          max overscan rectangles for the mode, clipit fills in the
 *          spos (screen pos) and dclip rectangles to use in centering.
 *          Centered around smallest containing user-editable oscan pref,
 *          with dclip confined to legal maxoscan limits.
 *          Screens which center such that their top is below text
 *          oscan top, will be moved up.
 *          If a non-null uclip is passed, that clip is used instead.
 */
STATIC VOID __asm __saveds
clipit( register __d1 SHORT wide, register __d2 SHORT high,
 register __a0 struct Rectangle *spos, register __a1 struct Rectangle *dclip,
 register __a2 struct Rectangle *txto, register __a3 struct Rectangle *stdo,
 register __a6 struct Rectangle *maxo )
{
struct  Rectangle *besto;
SHORT	minx, maxx, miny, maxy;
SHORT	txtw, txth, stdw, stdh, maxw, maxh, bestw, besth;

    /* get the txt, std and max widths and heights */
    txtw = txto->MaxX - txto->MinX + 1;
    txth = txto->MaxY - txto->MinY + 1;
    stdw = stdo->MaxX - stdo->MinX + 1;
    stdh = stdo->MaxY - stdo->MinY + 1;
    maxw = maxo->MaxX - maxo->MinX + 1;
    maxh = maxo->MaxY - maxo->MinY + 1;

    if((wide <= txtw)&&(high <= txth))
	{
	besto = txto;
	bestw = txtw;
	besth = txth;

	D(PRINTF("Best clip is txto\n"));
	}
    else
	{
	besto = stdo;
	bestw = stdw;
	besth = stdh;

	D(PRINTF("Best clip is stdo\n"));
	}

    D(PRINTF("TXTO: mnx=%ld mny=%ld mxx=%ld mxy=%ld  stdw=%ld stdh=%ld\n",
		txto->MinX,txto->MinY,txto->MaxX,txto->MaxY,txtw,txth));
    D(PRINTF("STDO: mnx=%ld mny=%ld mxx=%ld mxy=%ld  stdw=%ld stdh=%ld\n",
		stdo->MinX,stdo->MinY,stdo->MaxX,stdo->MaxY,stdw,stdh));
    D(PRINTF("MAXO: mnx=%ld mny=%ld mxx=%ld mxy=%ld  maxw=%ld maxh=%ld\n",
		maxo->MinX,maxo->MinY,maxo->MaxX,maxo->MaxY,maxw,maxh));

    /* CENTER the screen based on best oscan prefs
    * but confine dclip within max oscan limits
    *
    * FIX MinX first */
    spos->MinX = minx = besto->MinX - ((wide - bestw) >> 1);
    maxx = wide + minx - 1;
    if(maxx > maxo->MaxX)  maxx = maxo->MaxX;	/* too right */
    if(minx < maxo->MinX)  minx = maxo->MinX;	/* too left  */
    
    D(PRINTF("DCLIP: minx adjust from %ld to %ld\n",spos->MinX,minx));
    
    /* FIX MinY */
    spos->MinY = miny = besto->MinY - ((high - besth) >> 1);
    /* if lower than top of txto, move up */
    spos->MinY = miny = min(spos->MinY,txto->MinY);
    maxy = high + miny - 1;
    if(maxy > maxo->MaxY)  maxy = maxo->MaxY;	/* too down  */
    if(miny < maxo->MinY)  miny = maxo->MinY;	/* too up    */

    D(PRINTF("DCLIP: miny adjust from %ld to %ld\n",spos->MinY,miny));

    /* SET up dclip */
    dclip->MinX = minx;
    dclip->MinY = miny;
    dclip->MaxX = maxx;
    dclip->MaxY = maxy;

    D(PRINTF("CENTER: mnx=%ld mny=%ld maxx=%ld maxy=%ld\n",
	dclip->MinX,dclip->MinY,dclip->MaxX,dclip->MaxY));

} // clipit()


STATIC VOID __asm __saveds
FreeSpriteContext( register __a1 SPRITECONTEXT * spc )
{
    struct GfxBase	* GfxBase;
    int			  i;

    if ( !spc )
	return;

    if(!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",39L)) ) {
	return;
    }

    for ( i = spc->numsprites; i < 4; i++ ) {
	if ( spc->spritenum[i] != -1 ) {
	    D(PRINTF("!Freeing spritenum[%ld]= %ld\n",i,spc->spritenum[i] );)
	    FreeSprite( spc->spritenum[i] );
	}
    }

    for ( i = 0; i < spc->numsprites; i++ ) {
	if ( spc->spritenum[i] != -1 ) {
	    D(PRINTF("Freeing spritenum[%ld]= %ld\n",i,spc->spritenum[i] );)
	    FreeSprite( spc->spritenum[i] );
	}
	if ( spc->extsprite[i] )
	    FreeSpriteData( spc->extsprite[i] );
    }

    FreeVec( spc );

    CloseLibrary( (struct Library *)GfxBase );

} // FreeSpriteContext()


STATIC int __asm __saveds
BitMap2Sprites( register __a1 struct BitMap * inbm, register __d1 ULONG modeID )
{
    struct GfxBase	* GfxBase;
    SPRITECONTEXT	* spc;
    struct BitMap	* tbm,* sprbm;
    int			  i,bmw,bmh,wid,sprwid,xoff,ret;
    struct TagItem	  gsti[] =
			{
			    {GSTAG_SPRITE_NUM,1},
			    {TAG_END,NULL}
			};

    struct TagItem	  asti[] =
			{
			    {SPRITEA_Width, NULL},
			    {SPRITEA_OutputHeight, NULL},
			    {SPRITEA_YReplication, NULL},
			    {SPRITEA_XReplication, NULL},
			    {TAG_END,NULL}
			};

    if(!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",39L)) ) {
	return( FALSE );
    }

    if ( !(spc = AllocVec( sizeof (*spc), MEMF_CLEAR )) ) {
	CloseLibrary( (struct Library *)GfxBase );
	return( FALSE );
    }

    ret = TRUE;
    tbm = NULL;

    for ( i = 0; i < MAX_SPRITES; i++ )
	spc->spritenum[i] = -1;

    bmw = GetBitMapAttr( inbm, BMA_WIDTH );
    bmh = GetBitMapAttr( inbm, BMA_HEIGHT );

    if ( bmw > MAX_SPRITE_WIDTH ) {
	if ( !(tbm = AllocBitMap( MAX_SPRITE_WIDTH, bmh, 2, BMF_CLEAR, NULL )) ) {
	    ret = FALSE;
	    goto exit;
	}
	wid = MAX_SPRITE_WIDTH;
    } else {
	wid = bmw;
    }


    if ( tbm ) {
	sprbm = tbm;
    } else {
	sprbm = inbm;
    }


    sprwid = wid;
    xoff = 0;

#ifdef	OUTT	// [
    asti[1].ti_Data = (modeID & LACE) ? bmh>>1 : bmh;
    asti[2].ti_Data = (modeID & LACE) ? -1 : 0;
#else		// ][
/*
    asti[1].ti_Data = (GfxBase->ChipRevBits0 & GFXF_AA_ALICE) ? bmh : (bmh>>1);
    asti[2].ti_Data = (GfxBase->ChipRevBits0 & GFXF_AA_ALICE) ? 0 : -1;
*/
    asti[1].ti_Data = bmh;
    asti[2].ti_Data = 0;
    asti[3].ti_Data = (GfxBase->ChipRevBits0 & GFXF_AA_ALICE) ? 0 : -1;
#endif		// ]

    for ( i = 0; (spc->width < bmw) && (i < 4); i++ ) {

	if ( tbm ) {
	    if ( wid < sprwid ) {
		// Clear the bitmap.
		BltBitMap ( tbm, 0, 0, tbm, 0, 0, sprwid,
		    bmh, 0, ~0, NULL);
	    }

	    BltBitMap( inbm, xoff, 0, tbm,
		0,0,wid,bmh,0xC0,0xFF,NULL);

	    WaitBlit();
	}

	asti[0].ti_Data = (GfxBase->ChipRevBits0 & GFXF_AA_ALICE) ? sprwid : sprwid>>1;
	if ( !(spc->extsprite[i] = AllocSpriteDataA( sprbm, asti)) )
	{
	    ret = FALSE;
	    break;
	}

	gsti[0].ti_Data = i+1;
	if ((spc->spritenum[i] = GetExtSpriteA(spc->extsprite[i],gsti)) == -1)
	{
	    D(PRINTF("GetExtSprite FAILED i= %ld, spc->numsprites= %ld\n",i,spc->numsprites);)
	    ret = FALSE;
	    break;
	} else {
	    spc->numsprites++;
	    D(PRINTF("Got spritenum= %ld, numsprites= %ld, j= %ld\n",
		spc->spritenum[i],spc->numsprites,i);)
	}

	xoff += wid;
	spc->width += wid;

	wid = bmw - spc->width;
	wid = (wid > MAX_SPRITE_WIDTH) ? MAX_SPRITE_WIDTH : wid;
    }

exit:

    if ( ret ) {
	FFAnimBase->spc = spc;
	// Get sprites 3&4 if not already gotten. This is because the
	// real library uses sprites 1-4.
	D(PRINTF("1\n");)
	for ( i = spc->numsprites; i < 4; i++ ) {
	    D(PRINTF("2\n");)
	    gsti[0].ti_Data = i+1;
	    D(PRINTF("3 i= %ld\n",i);)
	    spc->spritenum[i] = GetExtSpriteA(spc->extsprite[0],gsti);
	    D(PRINTF("spritenum[%ld]= %ld\n",i,spc->spritenum[i]);)
	}
    } else {
	FreeSpriteContext( spc );
    }

    if ( tbm )
	FreeBitMap( tbm );

    CloseLibrary( (struct Library *)GfxBase );

    return( ret );

} // BitMap2Sprites()


ULONG __asm OpenWorkBenchPatch(void)
{
    return(0);

} // OpenWorkBenchPatch()


VOID __saveds
LIBStopFakeAnim( VOID )
{
    struct IntuitionBase	* IntuitionBase;
    struct GfxBase		* GfxBase;

    if ( !(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39)) )
	return;

    if(!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",39L)) ) {
	CloseLibrary( (struct Library *)IntuitionBase );
	return;
    }

    D(PRINTF("StopFakeAnim() ENTERED\n");)

    if ( FFAnimBase->spc && FFAnimBase->screen ) {
	SPRITECONTEXT	  * spc = FFAnimBase->spc;
	int		    i,left,xoff;

	for ( left = SPR_LEFT; left > (FFAnimBase->screen->LeftEdge-(spc->width>>1)); left -= 2 ) {

	    xoff = 0;
	    WaitTOF();
	    for ( i = 0; (i < spc->numsprites); i++ ) {
		MoveSprite( NULL,(struct SimpleSprite *)spc->extsprite[i],left+xoff,SPR_TOP);
		xoff += SPRITE_SEPERATION;
	    }
	}
    }

    if ( FFAnimBase->spc ) {
	FreeSpriteContext( FFAnimBase->spc );
	FFAnimBase->spc = NULL;
    }

    if ( FFAnimBase->screen ) {
	APTR OWB;

        OWB = SetFunction((struct Library *)IntuitionBase, -0xd2, (APTR)OpenWorkBenchPatch);
	CloseScreen( FFAnimBase->screen );
        SetFunction((struct Library *)IntuitionBase, -0xd2, OWB);

	FFAnimBase->screen = NULL;
    }

    CloseLibrary( (struct Library *)GfxBase );

} // LIBStopFakeAnim()


VOID __saveds 
PrintCount( VOID )
{
    struct IntuitionBase	* IntuitionBase;
    struct GfxBase		* GfxBase;
    struct IntuiText		  it;
    UBYTE			  buf[40];
    struct TextAttr		  Topaz =
				{
				    "topaz.font",
				    9,
				    NULL,	//FSF_BOLD,
				    0,
				};

    if ( !(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39)) )
	return;

    if(!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",39L)) ) {
	CloseLibrary( (struct Library *)IntuitionBase );
	return;
    }

    if ( FFAnimBase->screen ) {

	it.FrontPen = 1;
	it.BackPen = 0;
	it.DrawMode = JAM2;
	it.LeftEdge = it.TopEdge = 0;
	it.ITextFont = &Topaz;
	it.IText = buf;
	it.NextText = NULL;
/*
	sprintf( buf,"                              \0\n");
	PrintIText( &FFAnimBase->screen->RastPort, &it, 12, 12 );
*/
	sprintf( buf,"SimFreeAnim OpenCount= %d    \0\n",
	    FFAnimBase->opencount);
	PrintIText( &FFAnimBase->screen->RastPort, &it, 12, 12 );

/*	sprintf( buf,"                              \0\n");
	PrintIText( &FFAnimBase->screen->RastPort, &it, 12, 24 );
*/
	sprintf( buf,"SimFreeAnim CloseCount= %d    \0\n",
	    FFAnimBase->closecount);
	PrintIText( &FFAnimBase->screen->RastPort, &it, 12, 24 );

	sprintf( buf,"Simulating %ls Trademark\0\n",
	    (GfxBase->ChipRevBits0 & GFXF_AA_ALICE) ? "CD32" : "CDTV");
	PrintIText( &FFAnimBase->screen->RastPort, &it, 12, 36 );

    }

    CloseLibrary( (struct Library *)GfxBase );
    CloseLibrary( (struct Library *)IntuitionBase );

} // PrintCount()

__saveds
LIBStartFakeAnim( VOID )
{
    struct IntuitionBase	* IntuitionBase;
    struct GfxBase		* GfxBase;
    struct DisplayInfo		  disp;
    struct Rectangle		  txto,stdo,maxo,spos,dclip;
    struct ColorSpec		  initialcolors[] = 
				{
				    {0,0,0,0},
				    {1,0xFF,0xFF,0xFF},
				    {2,0,0,0},
				    {3,0,0,0},
				    {4,0,0,0},
				    {5,0,0,0},
				    {6,0,0,0},
				    {7,0,0,0},
				    {-1,0,0,0}
				};
    struct TagItem		  ti[] = 
				{
				    {SA_ShowTitle,FALSE},
				    {SA_Quiet,TRUE},
				    {SA_Colors,NULL},
				    {SA_Width,716},
				    {SA_Height,90},
				    {SA_Depth,3},
				    {SA_DisplayID,HIRESLACE_KEY},
				    {SA_Left,0},
				    {SA_VideoControl,NULL},
				    {SA_FullPalette,TRUE},
				    {TAG_DONE,NULL}
				};
    struct TagItem		vidti[] =
				{
				    {VTAG_SPRITERESN_SET,SPRITERESN_70NS},
				    {VTAG_BORDERSPRITE_SET,-1},
				    {TAG_END, NULL}
				};

    ULONG			colortable[] =
				{
				    3L<<16+0,
				    0x66666666,0x66666666,0x66666666,/* color #1 */
				    0x88888888,0x77777777,0x77777777,/* color #2 */
				    0xBBBBBBBB,0x99999999,0x99999999,/* color #3 */
				    0x0
			 	};



    if ( FFAnimBase->screen )
	return( TRUE );

    ti[2].ti_Data = (ULONG)initialcolors;

    D(PRINTF("StartFakeAnim() ENTERED\n");)
    if ( !(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39)) )
	return( FALSE );

    if(!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",39L)) ) {
	CloseLibrary( (struct Library *)IntuitionBase );
	return( FALSE );
    }

    if ( GetDisplayInfoData(NULL, (UBYTE *)&disp, sizeof(struct DisplayInfo), DTAG_DISP, LORES_KEY) ) {
	if ( disp.PropertyFlags & DIPF_IS_PAL ) {
	    ti[6].ti_Data |= PAL_MONITOR_ID;
	} else {
	    ti[6].ti_Data |= NTSC_MONITOR_ID;
	}
    }

    QueryOverscan( ti[6].ti_Data, &txto, OSCAN_TEXT );
    QueryOverscan( ti[6].ti_Data, &stdo, OSCAN_STANDARD );
    QueryOverscan( ti[6].ti_Data, &maxo, OSCAN_MAX );

    clipit( ti[3].ti_Data, ti[4].ti_Data, &spos, &dclip, &txto, &stdo,
	&maxo );

    ti[7].ti_Data = (LONG)spos.MinX;

//    ti[7].ti_Data = (LONG)(ti[3].ti_Data-(maxo.MaxX-maxo.MinX)) >> 1;

    D(PRINTF("MinX= %ld, Left= %ld\n",spos.MinX,ti[7].ti_Data);)

    ti[8].ti_Data = (LONG)vidti;

    FFAnimBase->screen = OpenScreenTagList( NULL, ti );

    D(PRINTF("StartFakeAnim() screen= 0x%lx\n",FFAnimBase->screen);)
    PrintCount();

    if ( FFAnimBase->screen ) {
	UBYTE		* data;
	struct BitMap	  bm;
	ULONG		  modeID;
	int		  i,w,h;

	w = (CDIMAGE_WID+15);
	w /= 8;
	w *= 8;

	h = CDIMAGE_HT;

	setmem( &bm, sizeof (bm) ,0 );
	InitBitMap( &bm, 2, w, h );

	D(PRINTF("sizeof (CDImageData)= %ld,(bm.BytesPerRow*bm.Rows)= %ld\n",
	    sizeof (CDImageData),(bm.BytesPerRow*bm.Rows));)

	if ( data = AllocVec( sizeof (CDImageData), MEMF_CHIP|MEMF_CLEAR ) ) {

	    CopyMem( CDImageData, data, sizeof (CDImageData) );

	    bm.Planes[0] = data;
	    bm.Planes[1] = data + (bm.BytesPerRow*bm.Rows);

	    D(PRINTF("Planes[0]= %ld, Planes[1]= %ld, diff= %ld\n",
		bm.Planes[0],bm.Planes[1],bm.Planes[1]-bm.Planes[0]);)

	    modeID = GetVPModeID( &FFAnimBase->screen->ViewPort );

	    if ( BitMap2Sprites( &bm, modeID ) && FFAnimBase->spc ) {
		SPRITECONTEXT	  * spc = FFAnimBase->spc;
		int		    xoff = 0,numcolors = 3;

		D(PRINTF("\nMAX_SPRITE_WIDTH= %ld, numsprites= %ld\n",
		    MAX_SPRITE_WIDTH,spc->numsprites);)

		for ( i = 0; (i < spc->numsprites); i++ ) {
		    colortable[0] = (numcolors<<16)+(((spc->spritenum[i]>>1)*4)+17);
		    LoadRGB32( &FFAnimBase->screen->ViewPort, colortable );
		    MoveSprite( NULL,(struct SimpleSprite *)spc->extsprite[i],SPR_LEFT+xoff,SPR_TOP);
		    xoff += SPRITE_SEPERATION;
		}
		for ( i = spc->numsprites; i < 4; i++ ) {
		    if ( spc->spritenum[i] != -1 ) {
			colortable[0] = (numcolors<<16)+(((spc->spritenum[i]>>1)*4)+17);
			LoadRGB32( &FFAnimBase->screen->ViewPort, colortable );
		    }
		}
	    }
	    FreeVec( data );
	}
    }

    CloseLibrary( (struct Library *)GfxBase );
    CloseLibrary( (struct Library *)IntuitionBase );

    return( FFAnimBase->screen ? TRUE : FALSE);

} // LibStartFakeAnim()


int __saveds __asm
__UserLibInit( register __a6 struct FFAnimLibrary *libbase )
{
    FFAnimBase = libbase;

    D(PRINTF("UserLibInit() ENTERED, NegSize= %ld, PosSize= %ld, sizeof (FFAnimLibrary)= %ld\n",
	libbase->ff_Lib.lib_NegSize,libbase->ff_Lib.lib_PosSize,sizeof (struct FFAnimLibrary));)

    if ( libbase->screen ) {
	libbase->opencount++;
	PrintCount();
    }

    return( 0 );  // 0 means successful

} //__UserLibInit()


void __saveds __asm
__UserLibCleanup( register __a6 struct FFAnimLibrary *libbase )
{
    D(PRINTF("UserLibCleanup() ENTERED\n");)

    if ( libbase->screen ) {
	libbase->closecount++;
	PrintCount();
    }

} //__UserLibCleanup()
